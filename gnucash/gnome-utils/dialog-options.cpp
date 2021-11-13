/********************************************************************\
 * dialog-options.cpp -- option handling                      *
 * Copyright (C) 1998-2000 Linas Vepstas                            *
 * Copyright (c) 2006 David Hampton <hampton@employees.org>         *
 * Copyright (c) 2011 Robert Fewell                                 *
 * Copyright 2020 John Ralls                                        *
 *                                                                  *
 * This program is free software; you can redistribute it and/or    *
 * modify it under the terms of the GNU General Public License as   *
 * published by the Free Software Foundation; either version 2 of   *
 * the License, or (at your option) any later version.              *
 *                                                                  *
 * This program is distributed in the hope that it will be useful,  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    *
 * GNU General Public License for more details.                     *
 *                                                                  *
 * You should have received a copy of the GNU General Public License*
 * along with this program; if not, contact:                        *
 *                                                                  *
 * Free Software Foundation           Voice:  +1-617-542-5942       *
 * 51 Franklin Street, Fifth Floor    Fax:    +1-617-542-2652       *
 * Boston, MA  02110-1301,  USA       gnu@gnu.org                   *
\********************************************************************/

extern "C"
{
#include <config.h> // Need this to include Account.h
}

#include <Account.h> // To include as C++ overriding later indirect includes
#include <libguile.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <glib/gi18n.h>

extern "C"
{
#include "swig-runtime.h"

#include "gnc-tree-model-budget.h" //FIXME?
#include "gnc-budget.h"
#include <qofbookslots.h>

#include "dialog-utils.h"
#include "gnc-engine-guile.h"
#include "glib-guile.h"
#include "gnc-account-sel.h"
#include "gnc-tree-view-account.h"
#include "gnc-commodity-edit.h"
#include "gnc-component-manager.h"
#include "gnc-general-select.h"
#include "gnc-currency-edit.h"
#include "gnc-date-edit.h"
#include "gnc-engine.h"
#include "gnc-prefs.h"
#include "gnc-gui-query.h"
#include "gnc-session.h"
#include "gnc-ui.h"
#include "gnc-guile-utils.h"
#include "guile-mappings.h"
#include "gnc-date-format.h"
#include "misc-gnome-utils.h"
}

#include <iostream>
#include <sstream>

#include "dialog-options.hpp"
#include <gnc-optiondb.hpp>
#include <gnc-optiondb-impl.hpp>

#define GNC_PREF_CLOCK_24H "clock-24h"


#include <gnc-option.hpp>
//#include <gnc-option-impl.hpp>
#include <gnc-optiondb.hpp>
#include <gnc-option-uitype.hpp>
#include <gnc-option-ui.hpp>

#define FUNC_NAME G_STRFUNC
/* TODO: clean up "register-stocks" junk
 */


/* This static indicates the debugging module that this .o belongs to.  */
static QofLogModule log_module = GNC_MOD_GUI;

static constexpr const char* DIALOG_OPTIONS_CM_CLASS{"dialog-options"};
static constexpr const char* GNC_PREFS_GROUP{"dialogs.options"};

/*
 * Point where preferences switch control method from a set of
 * notebook tabs to a list.
 */
#define MAX_TAB_COUNT 6

/* A pointer to the last selected filename */
#define LAST_SELECTION "last-selection"

struct gnc_option_win
{
    GtkWidget  * window;
    GtkWidget  * notebook;
    GtkWidget  * page_list_view;
    GtkWidget  * page_list;

    bool toplevel;

    GNCOptionWinCallback apply_cb;
    gpointer             apply_cb_data;

    GNCOptionWinCallback help_cb;
    gpointer             help_cb_data;

    GNCOptionWinCallback close_cb;
    gpointer             close_cb_data;

    /* Hold onto this for a complete reset */
    GncOptionDB *option_db;

    /* Hold on to this to unregister the right class */
    const char *component_class;

    /* widget being destroyed */
    bool destroyed;
};

enum page_tree
{
    PAGE_INDEX = 0,
    PAGE_NAME,
    NUM_COLUMNS
};

//Init the class static.
std::vector<WidgetCreateFunc> GncOptionUIFactory::s_registry{static_cast<size_t>(GncOptionUIType::MAX_VALUE)};

void
GncOptionUIFactory::set_func(GncOptionUIType type, WidgetCreateFunc func)
{
    s_registry[static_cast<size_t>(type)] = func;
}

GtkWidget*
GncOptionUIFactory::create(GncOption& option, GtkGrid* page, GtkLabel* name,
                     char* description, GtkWidget** enclosing, bool* packed)
{
    auto type{option.get_ui_type()};
    auto func{s_registry[static_cast<size_t>(type)]};
    if (func)
        return func(option, page, name, description, enclosing, packed);
    PERR("No function registered for type %d", static_cast<int>(type));
    return nullptr;
}

GncOptionGtkUIItem::GncOptionGtkUIItem(GtkWidget* widget,
                                       GncOptionUIType type) :
    GncOptionUIItem{type},
    m_widget{static_cast<GtkWidget*>(g_object_ref(widget))} {}

GncOptionGtkUIItem::GncOptionGtkUIItem(const GncOptionGtkUIItem& item) :
        GncOptionUIItem{item.get_ui_type()},
        m_widget{static_cast<GtkWidget*>(g_object_ref(item.get_widget()))} {}

GncOptionGtkUIItem::~GncOptionGtkUIItem()
{
    if (m_widget)
        g_object_unref(m_widget);
}

void
GncOptionGtkUIItem::set_selectable(bool selectable) const noexcept
{
    if (m_widget)
        gtk_widget_set_sensitive (m_widget, selectable);
}

void
GncOptionGtkUIItem::clear_ui_item()
{
    if (m_widget)
        g_object_unref(m_widget);
    m_widget = nullptr;
}

void
GncOptionGtkUIItem::set_widget(GtkWidget* widget)
{
    if (get_ui_type() == GncOptionUIType::INTERNAL)
    {
        std::string error{"INTERNAL option, setting the UI item forbidden."};
        throw std::logic_error(std::move(error));
    }

    if (m_widget)
        g_object_unref(m_widget);

    m_widget = static_cast<GtkWidget*>(g_object_ref(widget));
}


static GNCOptionWinCallback global_help_cb = NULL;
gpointer global_help_cb_data = NULL;

static void dialog_reset_cb(GtkWidget * w, gpointer data);
void dialog_list_select_cb (GtkTreeSelection *selection,
                                        gpointer data);
static void component_close_handler (gpointer data);

static void
section_reset_widgets(GncOptionSection* section)
{
}

GtkWidget* const
gnc_option_get_gtk_widget (const GncOption* option)
{
    if (!option) return nullptr;
    auto ui_item{dynamic_cast<const GncOptionGtkUIItem*>(option->get_ui_item())};
    if (ui_item)
        return ui_item->get_widget();

    return nullptr;
}

static void
dialog_changed_internal (GtkWidget *widget, bool sensitive)
{
    g_return_if_fail(widget);
    while (TRUE)
    {
        auto new_widget = gtk_widget_get_parent(widget);
        if (new_widget && GTK_IS_WIDGET(new_widget) &&
            GTK_IS_WINDOW(new_widget))
            widget = new_widget;
        else
            break;
    }

    /* find the ok and cancel buttons, we know where they will be so do it
       this way as opposed to using gtk_container_foreach, much less iteration */
    if (GTK_IS_CONTAINER(widget))
    {
        GList *children = gtk_container_get_children(GTK_CONTAINER(widget));
        for (GList *it = children; it; it = it->next)
        {
            if (GTK_IS_BOX (GTK_WIDGET(it->data)))
            {
                GList *children = gtk_container_get_children(GTK_CONTAINER(it->data));
                for (GList *it = children; it; it = it->next)
                {
                    if (GTK_IS_BUTTON_BOX (GTK_WIDGET(it->data)))
                    {
                        GList *children = gtk_container_get_children(GTK_CONTAINER(it->data));
                        for (GList *it = children; it; it = it->next)
                        {
                            GtkWidget* widget = GTK_WIDGET(it->data);
                            const gchar* name = gtk_widget_get_name(widget);

                            if (g_strcmp0 (name, "ok_button") == 0 ||
                                g_strcmp0 (name, "apply_button") == 0)
                                gtk_widget_set_sensitive (widget, sensitive);
                            else if (g_strcmp0 (name, "cancel_button") == 0)
                                gtk_button_set_label (GTK_BUTTON (widget),
                                                      sensitive ? _("_Cancel") :
                                                      _("_Close"));
                        }
                        g_list_free (children);
                        break; // Found the button-box, no need to continue.
                    }
                }
                g_list_free (children);
                break; // Found the box, no need to continue.
            }
        }
        g_list_free (children);
    }
}

void
gnc_options_dialog_changed (GNCOptionWin *win)
{
    if (!win) return;

    dialog_changed_internal (win->window, TRUE);
}

void
gnc_option_changed_widget_cb(GtkWidget *widget, GncOption* option)
{
    if (!option) return;
    const_cast<GncOptionUIItem*>(option->get_ui_item())->set_dirty(true);
    dialog_changed_internal(widget, true);
}

void
gnc_option_changed_option_cb(GtkWidget *dummy, GncOption* option)
{
    if (!option) return;
    auto widget{gnc_option_get_gtk_widget(option)};
    gnc_option_changed_widget_cb(widget, option);
}


// This do-nothing template is specialized for each GncOptionUIType.
template<GncOptionUIType type> GtkWidget*
create_option_widget(GncOption& option, GtkGrid*, GtkLabel*, char*, GtkWidget**,
                     bool*)
{
    return nullptr;
}

static void
gnc_option_set_ui_widget(GncOption& option, GtkGrid *page_box, gint grid_row)
{
    GtkWidget *enclosing = NULL;
    GtkWidget *value = NULL;
    bool packed = FALSE;
    char *name, *documentation;
    GtkLabel *name_label;

    ENTER("option %p(%s), box %p",
          &option, option.get_name().c_str(), page_box);
    auto type = option.get_ui_type();
    if (type == GncOptionUIType::INTERNAL)
    {
        LEAVE("internal type");
        return;
    }



    const char* raw_name = option.get_name().c_str();
    if (raw_name && *raw_name)
        name = _(raw_name);
    else
        name = nullptr;

    const char* raw_documentation = option.get_docstring().c_str();
    if (raw_documentation && *raw_documentation)
        documentation = _(raw_documentation);
    else
        documentation = nullptr;

    name_label = GTK_LABEL(gtk_label_new (name));
    auto widget = GncOptionUIFactory::create(option, page_box, name_label,
                                             documentation, &enclosing,
                                             &packed);
    /* Attach the name label to the first column of the grid and
       align to the end unless it's a check button, which has its own label. */
    if (!GTK_IS_CHECK_BUTTON(widget))
    {
        gtk_grid_attach (GTK_GRID(page_box), GTK_WIDGET(name_label),
                         0, grid_row, // left, top
                         1, 1);  // width, height
        gtk_widget_set_halign (GTK_WIDGET(name_label), GTK_ALIGN_END);
    }
    if (!packed && (enclosing != NULL))
    {
        /* Pack option widget into an extra eventbox because otherwise the
           "documentation" tooltip is not displayed. */
        GtkWidget *eventbox = gtk_event_box_new();

        gtk_container_add (GTK_CONTAINER (eventbox), enclosing);

        /* attach the option widget to the second column of the grid */
        gtk_grid_attach (GTK_GRID(page_box), eventbox,
                         1, grid_row, // left, top
                         1, 1);  // width, height

        gtk_widget_set_tooltip_text (eventbox, documentation);
    }

    if (value != NULL)
        gtk_widget_set_tooltip_text(value, documentation);

    LEAVE(" ");
}

static GtkBox*
create_content_box()
{
    auto content_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
    gtk_widget_set_name (content_box, "page-content-box");
    gtk_box_set_homogeneous (GTK_BOX (content_box), FALSE);

    gtk_container_set_border_width(GTK_CONTAINER(content_box), 12);
    return GTK_BOX(content_box);
}

static GtkGrid*
create_options_box(GtkBox* content_box)
{
    auto options_scrolled_win = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(content_box), options_scrolled_win,
                       TRUE, TRUE, 0);

    /* Build space for the content - the options box */
    auto options_box = gtk_grid_new(); // this will have two columns
    gtk_widget_set_name (options_box, "options-box");
    gtk_grid_set_row_homogeneous (GTK_GRID(options_box), FALSE);
    gtk_grid_set_column_homogeneous (GTK_GRID(options_box), FALSE);
    gtk_grid_set_row_spacing (GTK_GRID(options_box), 6);
    gtk_grid_set_column_spacing (GTK_GRID(options_box), 6);

    gtk_container_set_border_width(GTK_CONTAINER(options_box), 0);
    gtk_container_add (GTK_CONTAINER(options_scrolled_win),
                       GTK_WIDGET(options_box));
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(options_scrolled_win),
                                   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    return GTK_GRID(options_box);
}

static GtkButtonBox*
create_reset_button_box(GtkBox* page_content_box)
{
    auto buttonbox = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
    gtk_button_box_set_layout (GTK_BUTTON_BOX (buttonbox),
                               GTK_BUTTONBOX_EDGE);
    gtk_container_set_border_width(GTK_CONTAINER (buttonbox), 5);
    gtk_box_pack_end(GTK_BOX(page_content_box), buttonbox, FALSE, FALSE, 0);
    return GTK_BUTTON_BOX(buttonbox);
}

static int
setup_notebook_pages(GNCOptionWin* propertybox, GtkBox* page_content_box,
                     const char* name)
{
    auto page_count = gtk_notebook_page_num(GTK_NOTEBOOK(propertybox->notebook),
                                            GTK_WIDGET(page_content_box));

    if (propertybox->page_list_view)
    {
        /* Build the matching list item for selecting from large page sets */
        auto view = GTK_TREE_VIEW(propertybox->page_list_view);
        auto list = GTK_LIST_STORE(gtk_tree_view_get_model(view));

        PINFO("Page name is %s and page_count is %d", name, page_count);
        GtkTreeIter iter;
        gtk_list_store_append(list, &iter);
        gtk_list_store_set(list, &iter,
                           PAGE_NAME, _(name),
                           PAGE_INDEX, page_count,
                           -1);

        if (page_count > MAX_TAB_COUNT - 1)   /* Convert 1-based -> 0-based */
        {
            gtk_widget_show(propertybox->page_list);
            gtk_notebook_set_show_tabs(GTK_NOTEBOOK(propertybox->notebook), FALSE);
            gtk_notebook_set_show_border(GTK_NOTEBOOK(propertybox->notebook), FALSE);
        }
        else
            gtk_widget_hide(propertybox->page_list);

    }
    return page_count;
}

static int
gnc_options_dialog_append_page(GNCOptionWin * propertybox,
                               GncOptionSectionPtr& section)
{
    auto name = section->get_name().c_str();
    if (!name || *name == '\0')
        return -1;

    if (strncmp(name, "__", 2) == 0)
        return -1;

    auto page_label = gtk_label_new(_(name));
    PINFO("Page_label is %s", _(name));
    gtk_widget_show(page_label);

    /* Build this options page */
    auto page_content_box = create_content_box();
    auto options_box = create_options_box(page_content_box);

    /* Create all the options */
    size_t row = 0;
    section->foreach_option(
        [options_box, &row](GncOption& option) {
            g_object_set_data (G_OBJECT(options_box), "options-grid-row",
                               GINT_TO_POINTER(row));
            gnc_option_set_ui_widget(option, GTK_GRID(options_box), row);
            ++row;
        });

    /* Add a button box at the bottom of the page */
    auto buttonbox = create_reset_button_box(page_content_box);
    /* The reset button on each option page */
    auto reset_button = gtk_button_new_with_label (_("Reset defaults"));
    gtk_widget_set_tooltip_text(reset_button,
                                _("Reset all values to their defaults."));

    g_signal_connect(G_OBJECT(reset_button), "clicked",
                     G_CALLBACK(dialog_reset_cb), propertybox);
    g_object_set_data(G_OBJECT(reset_button), "section",
                      static_cast<void*>(section.get()));
    gtk_box_pack_end(GTK_BOX(buttonbox), reset_button, FALSE, FALSE, 0);
    gtk_widget_show_all(GTK_WIDGET(page_content_box));
    gtk_notebook_append_page(GTK_NOTEBOOK(propertybox->notebook),
                             GTK_WIDGET(page_content_box), page_label);

    /* Switch to selection from a list if the page count threshold is reached */
    return setup_notebook_pages(propertybox, page_content_box, name);
}

/********************************************************************\
 * gnc_options_dialog_build_contents                                *
 *   builds an options dialog given a property box and an options   *
 *   database and make the dialog visible                           *
 *                                                                  *
 * @param propertybox - gnome property box to use                    *
 * @param odb         - option database to use                       *
\********************************************************************/
void
gnc_options_dialog_build_contents (GNCOptionWin *propertybox,
                                   GncOptionDB  *odb)
{
    gnc_options_dialog_build_contents_full (propertybox, odb, true);
}

/********************************************************************\
 * gnc_options_dialog_build_contents_full                           *
 *   builds an options dialog given a property box and an options   *
 *   database and make the dialog visible depending on the          *
 *   show_dialog flag                                               *
 *                                                                  *
 * @param propertybox - gnome property box to use                    *
 * @param odb         - option database to use                       *
 * @param show_dialog - should dialog be made visible or not         *
\********************************************************************/
void
gnc_options_dialog_build_contents_full (GNCOptionWin *propertybox,
                                        GNCOptionDB  *odb, gboolean show_dialog)
{
    gint default_page = -1;

    gint page;

    g_return_if_fail (propertybox != NULL);
    g_return_if_fail (odb != NULL);

    propertybox->option_db = odb;

    auto num_sections = odb->num_sections();
    auto default_section = odb->get_default_section();

    PINFO("Default Section name is %s",
          default_section ? default_section->get_name().c_str() : "NULL");

    odb->foreach_section(
        [propertybox, default_section, &default_page]
        (GncOptionSectionPtr& section) {
            auto page = gnc_options_dialog_append_page(propertybox, section);
            if (default_section && section.get() == default_section)
                default_page = page;
        });

    /* call each option widget changed callbacks once at this point, now that
     * all options widgets exist.
     *
     * Note that this may be superfluous as each create_option_widget
     * specialization calls option.set_ui_item_from_option after creating the UI
     * item.
     */
    odb->foreach_section(
        [](GncOptionSectionPtr& section) {
            section->foreach_option(
                [](GncOption& option) {
                    option.set_ui_item_from_option();
                });
        });

    gtk_notebook_popup_enable(GTK_NOTEBOOK(propertybox->notebook));
    if (default_page >= 0)
    {
        /* Find the page list and set the selection to the default page */
        GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(propertybox->page_list_view));
        GtkTreeIter iter;
        GtkTreeModel *model;

        model = gtk_tree_view_get_model(GTK_TREE_VIEW(propertybox->page_list_view));
        gtk_tree_model_iter_nth_child(model, &iter, NULL, default_page);
        gtk_tree_selection_select_iter (selection, &iter);
        gtk_notebook_set_current_page(GTK_NOTEBOOK(propertybox->notebook), default_page);
    }
    dialog_changed_internal(propertybox->window, FALSE);
    if (show_dialog)
        gtk_widget_show(propertybox->window);
}

GtkWidget *
gnc_options_dialog_widget(GNCOptionWin * win)
{
    return win->window;
}

GtkWidget *
gnc_options_page_list(GNCOptionWin * win)
{
    return win->page_list;
}

GtkWidget *
gnc_options_dialog_notebook(GNCOptionWin * win)
{
    return win->notebook;
}

static void
gnc_options_dialog_help_button_cb(GtkWidget * widget, GNCOptionWin *win)
{
    if (win->help_cb)
        (win->help_cb)(win, win->help_cb_data);
}

static void
gnc_options_dialog_cancel_button_cb(GtkWidget * widget, GNCOptionWin *win)
{
    gnc_save_window_size (GNC_PREFS_GROUP, GTK_WINDOW(win->window));

    if (win->close_cb)
        (win->close_cb)(win, win->close_cb_data);
    else
        gtk_widget_hide(win->window);
}

static void
gnc_options_dialog_apply_button_cb(GtkWidget * widget, GNCOptionWin *win)
{
    GNCOptionWinCallback close_cb = win->close_cb;

    win->close_cb = NULL;
    if (win->apply_cb)
        win->apply_cb (win, win->apply_cb_data);
    win->close_cb = close_cb;
    gnc_save_window_size (GNC_PREFS_GROUP, GTK_WINDOW(win->window));
    dialog_changed_internal (win->window, FALSE);
}

static void
gnc_options_dialog_ok_button_cb(GtkWidget * widget, GNCOptionWin *win)
{
    GNCOptionWinCallback close_cb = win->close_cb;

    win->close_cb = NULL;
    if (win->apply_cb)
        win->apply_cb (win, win->apply_cb_data);
    win->close_cb = close_cb;

    gnc_save_window_size (GNC_PREFS_GROUP, GTK_WINDOW(win->window));

    if (win->close_cb)
        (win->close_cb)(win, win->close_cb_data);
    else
        gtk_widget_hide(win->window);
}

static void
gnc_options_dialog_destroy_cb (GtkWidget *object, GNCOptionWin *win)
{
    if (!win) return;

    if (win->destroyed == FALSE)
    {
        if (win->close_cb)
            (win->close_cb)(win, win->close_cb_data);
    }
}

static bool
gnc_options_dialog_window_key_press_cb(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    GNCOptionWin *win = static_cast<decltype(win)>(data);

    if (event->keyval == GDK_KEY_Escape)
    {
        component_close_handler (win);
        return TRUE;
    }
    else
        return FALSE;
}

static void
dialog_reset_cb(GtkWidget * w, gpointer data)
{
    GNCOptionWin *win = static_cast<decltype(win)>(data);
    gpointer val;
    bool dialog_changed = false;

    val = g_object_get_data(G_OBJECT(w), "section");
    g_return_if_fail (val);
    g_return_if_fail (win);

    auto section = static_cast<GncOptionSection*>(val);
    section->foreach_option(
        [&dialog_changed](GncOption& option) {
            if (option.is_changed())
            {
                option.reset_default_value();
                option.get_ui_item()->set_dirty(true);
                dialog_changed = true;
            }
            option.set_ui_item_from_option();
        });

    dialog_changed_internal (win->window, dialog_changed);
}

void
dialog_list_select_cb (GtkTreeSelection *selection,
                                   gpointer data)
{
    GNCOptionWin * win = static_cast<decltype(win)>(data);
    GtkTreeModel *list;
    GtkTreeIter iter;
    gint index = 0;

    if (!gtk_tree_selection_get_selected(selection, &list, &iter))
        return;
    gtk_tree_model_get(list, &iter,
                       PAGE_INDEX, &index,
                       -1);
    PINFO("Index is %d", index);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(win->notebook), index);
}

static void
component_close_handler (gpointer data)
{
    GNCOptionWin *win = static_cast<decltype(win)>(data);
    gnc_save_window_size (GNC_PREFS_GROUP, GTK_WINDOW(win->window));
    gnc_options_dialog_cancel_button_cb (NULL, win);
}

/* gnc_options_dialog_new:
 *
 *   - Opens the dialog-options glade file
 *   - Connects signals specified in the builder file
 *   - Sets the window's title
 *   - Initializes a new GtkNotebook, and adds it to the window
 *
 */
GNCOptionWin *
gnc_options_dialog_new(gchar *title, GtkWindow *parent)
{
    return gnc_options_dialog_new_modal(FALSE, title, NULL, parent);
}

/**
 *   - Opens the dialog-options glade file
 *   - Connects signals specified in the builder file
 *   - Sets the window's title
 *   - Initializes a new GtkNotebook, and adds it to the window
 *   - If modal TRUE, hides 'apply' button
 *   - If component_class is provided, it is used, otherwise,
 *     DIALOG_OPTIONS_CM_CLASS is used; this is used to distinguish the
 *     book-option dialog from report dialogs. The book-option dialog is a
 *     singleton, so if a dialog already exists it will be raised to the top of
 *     the window stack instead of creating a new dialog.
 */
GNCOptionWin *
gnc_options_dialog_new_modal(gboolean modal, gchar *title,
                             const char *component_class,
                             GtkWindow *parent)
{
    GNCOptionWin *retval;
    GtkBuilder   *builder;
    GtkWidget    *hbox;
    gint component_id;
    GtkWidget    *button;

    retval = g_new0(GNCOptionWin, 1);
    builder = gtk_builder_new();
    gnc_builder_add_from_file (builder, "dialog-options.glade", "gnucash_options_window");
    retval->window = GTK_WIDGET(gtk_builder_get_object (builder, "gnucash_options_window"));
    retval->page_list = GTK_WIDGET(gtk_builder_get_object (builder, "page_list_scroll"));
    retval->component_class = component_class ? component_class : DIALOG_OPTIONS_CM_CLASS;

    // Set the name for this dialog so it can be easily manipulated with css
    gtk_widget_set_name (GTK_WIDGET(retval->window), "gnc-id-options");

    /* Page List */
    {
        GtkTreeView *view;
        GtkListStore *store;
        GtkTreeSelection *selection;
        GtkCellRenderer *renderer;
        GtkTreeViewColumn *column;

        retval->page_list_view = GTK_WIDGET(gtk_builder_get_object (builder, "page_list_treeview"));

        view = GTK_TREE_VIEW(retval->page_list_view);

        store = gtk_list_store_new(NUM_COLUMNS, G_TYPE_INT, G_TYPE_STRING);
        gtk_tree_view_set_model(view, GTK_TREE_MODEL(store));
        g_object_unref(store);

        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes(_("Page"), renderer,
                 "text", PAGE_NAME, NULL);
        gtk_tree_view_append_column(view, column);

        gtk_tree_view_column_set_alignment(column, 0.5);

        selection = gtk_tree_view_get_selection(view);
        gtk_tree_selection_set_mode(selection, GTK_SELECTION_BROWSE);
        g_signal_connect (selection, "changed",
                          G_CALLBACK (dialog_list_select_cb), retval);
    }

    button = GTK_WIDGET(gtk_builder_get_object (builder, "helpbutton"));
        g_signal_connect(button, "clicked", G_CALLBACK(gnc_options_dialog_help_button_cb), retval);
    button = GTK_WIDGET(gtk_builder_get_object (builder, "cancelbutton"));
        g_signal_connect(button, "clicked", G_CALLBACK(gnc_options_dialog_cancel_button_cb), retval);
    button = GTK_WIDGET(gtk_builder_get_object (builder, "applybutton"));
        g_signal_connect(button, "clicked", G_CALLBACK(gnc_options_dialog_apply_button_cb), retval);
    button = GTK_WIDGET(gtk_builder_get_object (builder, "okbutton"));
        g_signal_connect(button, "clicked", G_CALLBACK(gnc_options_dialog_ok_button_cb), retval);

    gtk_builder_connect_signals_full (builder, gnc_builder_connect_full_func, retval);

    // when added to a page of the hierarchy assistant there will be no parent
    if (parent)
        gnc_restore_window_size (GNC_PREFS_GROUP, GTK_WINDOW(retval->window),
                                 parent);

    if (title)
        gtk_window_set_title(GTK_WINDOW(retval->window), title);

    /* modal */
    if (modal == TRUE)
    {
        GtkWidget *apply_button;

        apply_button = GTK_WIDGET(gtk_builder_get_object (builder, "applybutton"));
        gtk_widget_hide (apply_button);
    }

    /* glade doesn't support a notebook with zero pages */
    hbox = GTK_WIDGET(gtk_builder_get_object (builder, "notebook_placeholder"));
    retval->notebook = gtk_notebook_new();

    gtk_widget_set_vexpand (retval->notebook, TRUE);

    gtk_widget_show(retval->notebook);
    gtk_box_pack_start(GTK_BOX(hbox), retval->notebook, TRUE, TRUE, 5);

    component_id = gnc_register_gui_component (retval->component_class,
                                               NULL, component_close_handler,
                                               retval);
    gnc_gui_component_set_session (component_id, gnc_get_current_session());

    g_signal_connect (retval->window, "destroy",
                      G_CALLBACK(gnc_options_dialog_destroy_cb), retval);

    g_signal_connect (retval->window, "key_press_event",
                      G_CALLBACK(gnc_options_dialog_window_key_press_cb), retval);

    g_object_unref(G_OBJECT(builder));

    retval->destroyed = FALSE;
    return retval;
}

void
gnc_options_dialog_set_apply_cb(GNCOptionWin * win, GNCOptionWinCallback cb,
                                gpointer data)
{
    win->apply_cb = cb;
    win->apply_cb_data = data;
}

void
gnc_options_dialog_set_help_cb(GNCOptionWin * win, GNCOptionWinCallback cb,
                               gpointer data)
{
    win->help_cb = cb;
    win->help_cb_data = data;
}

void
gnc_options_dialog_set_close_cb(GNCOptionWin * win, GNCOptionWinCallback cb,
                                gpointer data)
{
    win->close_cb = cb;
    win->close_cb_data = data;
}

void
gnc_options_dialog_set_global_help_cb(GNCOptionWinCallback thunk,
                                      gpointer cb_data)
{
    global_help_cb = thunk;
    global_help_cb_data = cb_data;
}

/* This is for global program preferences. */
void
gnc_options_dialog_destroy(GNCOptionWin * win)
{
    if (!win) return;

    gnc_unregister_gui_component_by_data(win->component_class, win);

    win->destroyed = TRUE;
    gtk_widget_destroy(win->window);

    win->window = NULL;
    win->notebook = NULL;
    win->apply_cb = NULL;
    win->help_cb = NULL;
    win->component_class = NULL;

    g_free(win);
}

/* ****************************************************************/
/* Option Widgets                                      */
/* ***************************************************************/

static void
align_label (GtkLabel *name_label)
{
    /* some option widgets have a large vertical foot print so align
       the label name to the top and add a small top margin */
    gtk_widget_set_valign (GTK_WIDGET(name_label), GTK_ALIGN_START);
    gtk_widget_set_margin_top (GTK_WIDGET(name_label), 6);
}

class GncGtkBooleanUIItem : public GncOptionGtkUIItem
{
public:
    GncGtkBooleanUIItem(GtkWidget* widget) :
        GncOptionGtkUIItem{widget, GncOptionUIType::BOOLEAN} {}
    void set_ui_item_from_option(GncOption& option) noexcept override
    {
        auto widget{GTK_TOGGLE_BUTTON(get_widget())};
        gtk_toggle_button_set_active(widget, option.get_value<bool>());
    }
    void set_option_from_ui_item(GncOption& option) noexcept override
    {
        auto widget{GTK_TOGGLE_BUTTON(get_widget())};
        option.set_value(static_cast<bool>(gtk_toggle_button_get_active(widget)));
    }
};

template <> GtkWidget *
create_option_widget<GncOptionUIType::BOOLEAN> (GncOption& option,
                                                GtkGrid* page_box,
                                                GtkLabel* name_label,
                                                char* documentation,
                                                /* Return values */
                                                GtkWidget** enclosing,
                                                bool* packed)
{

    *enclosing = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_set_homogeneous (GTK_BOX (*enclosing), FALSE);
    auto widget =
        gtk_check_button_new_with_label (gtk_label_get_text(name_label));

    auto ui_item{std::make_unique<GncGtkBooleanUIItem>(widget)};

    g_signal_connect(G_OBJECT(widget), "toggled",
                     G_CALLBACK(gnc_option_changed_widget_cb), &option);

    option.set_ui_item(std::move(ui_item));
    option.set_ui_item_from_option();

    gtk_box_pack_start(GTK_BOX(*enclosing), widget, FALSE, FALSE, 0);
    gtk_widget_show_all(*enclosing);

    return widget;
}

class GncGtkStringUIItem : public GncOptionGtkUIItem
{
public:
    GncGtkStringUIItem(GtkWidget* widget) :
        GncOptionGtkUIItem{widget, GncOptionUIType::STRING} {}
    void set_ui_item_from_option(GncOption& option) noexcept override
    {
        auto widget{GTK_ENTRY(get_widget())};
        gtk_entry_set_text(widget, option.get_value<std::string>().c_str());
    }
    void set_option_from_ui_item(GncOption& option) noexcept override
    {
        auto widget{GTK_ENTRY(get_widget())};
        option.set_value(std::string{gtk_entry_get_text(widget)});
    }
};

template<> GtkWidget*
create_option_widget<GncOptionUIType::STRING> (GncOption& option,
                                               GtkGrid *page_box,
                                               GtkLabel *name_label,
                                               char *documentation,
                                               /* Return values */
                                               GtkWidget **enclosing,
                                               bool *packed)
{
    *enclosing = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_hexpand (GTK_WIDGET(*enclosing), TRUE);
    gtk_box_set_homogeneous (GTK_BOX (*enclosing), FALSE);
    auto widget = gtk_entry_new();
    if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
        gtk_entry_set_alignment (GTK_ENTRY(widget), 1.0);
    auto ui_item{std::make_unique<GncGtkStringUIItem>(widget)};

    g_signal_connect(G_OBJECT(widget), "changed",
                     G_CALLBACK(gnc_option_changed_widget_cb), &option);
    option.set_ui_item(std::move(ui_item));
    option.set_ui_item_from_option();

    gtk_box_pack_start(GTK_BOX(*enclosing), widget, TRUE, TRUE, 0);
    gtk_widget_show_all(*enclosing);
    return widget;
}

class GncGtkTextUIItem : public GncOptionGtkUIItem
{
public:
    GncGtkTextUIItem(GtkWidget* widget) :
        GncOptionGtkUIItem{widget, GncOptionUIType::TEXT} {}
    void set_ui_item_from_option(GncOption& option) noexcept override
    {
        auto widget{GTK_TEXT_VIEW(get_widget())};
        xxxgtk_textview_set_text(widget, option.get_value<std::string>().c_str());
    }
    void set_option_from_ui_item(GncOption& option) noexcept override
    {
        auto widget{GTK_TEXT_VIEW(get_widget())};
        option.set_value(std::string{xxxgtk_textview_get_text(widget)});
    }
};

template<> GtkWidget*
create_option_widget<GncOptionUIType::TEXT> (GncOption& option, GtkGrid *page_box,
                               GtkLabel *name_label, char *documentation,
                               /* Return values */
                               GtkWidget **enclosing, bool *packed)
{
    align_label (name_label);

    auto scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_NEVER,
                                   GTK_POLICY_AUTOMATIC);
    gtk_container_set_border_width(GTK_CONTAINER(scroll), 2);

    auto frame = gtk_frame_new(NULL);
    gtk_container_add(GTK_CONTAINER(frame), scroll);

    *enclosing = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_vexpand (GTK_WIDGET(*enclosing), TRUE);
    gtk_widget_set_hexpand (GTK_WIDGET(*enclosing), TRUE);
    gtk_box_set_homogeneous (GTK_BOX (*enclosing), FALSE);
    auto widget = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(widget), GTK_WRAP_WORD);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(widget), TRUE);
    gtk_text_view_set_accepts_tab (GTK_TEXT_VIEW(widget), FALSE);
    gtk_container_add (GTK_CONTAINER (scroll), widget);

    auto ui_item{std::make_unique<GncGtkTextUIItem>(widget)};
    auto text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
    g_signal_connect(G_OBJECT(text_buffer), "changed",
                     G_CALLBACK(gnc_option_changed_option_cb), &option);
    option.set_ui_item(std::move(ui_item));
    option.set_ui_item_from_option();

    gtk_box_pack_start(GTK_BOX(*enclosing), frame, TRUE, TRUE, 0);
    gtk_widget_show_all(*enclosing);
    return widget;
}

class GncGtkCurrencyUIItem : public GncOptionGtkUIItem
{
public:
    GncGtkCurrencyUIItem(GtkWidget* widget) :
        GncOptionGtkUIItem{widget, GncOptionUIType::CURRENCY} {}
    void set_ui_item_from_option(GncOption& option) noexcept override
    {
        auto widget{GNC_CURRENCY_EDIT(get_widget())};
        auto instance{option.get_value<const QofInstance*>()};
        if (instance)
            gnc_currency_edit_set_currency(widget, GNC_COMMODITY(instance));
    }
    void set_option_from_ui_item(GncOption& option) noexcept override
    {
        auto widget{GNC_CURRENCY_EDIT(get_widget())};
        auto currency = gnc_currency_edit_get_currency(widget);
        option.set_value(qof_instance_cast(currency));
    }
};

template<> GtkWidget*
create_option_widget<GncOptionUIType::CURRENCY> (GncOption& option, GtkGrid *page_box,
                                   GtkLabel *name_label, char *documentation,
                                   /* Return values */
                                   GtkWidget **enclosing, bool *packed)
{
    *enclosing = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_set_homogeneous (GTK_BOX (*enclosing), FALSE);
    auto widget = gnc_currency_edit_new();
    auto ui_item{std::make_unique<GncGtkCurrencyUIItem>(widget)};
    g_signal_connect(G_OBJECT(widget), "changed",
                     G_CALLBACK(gnc_option_changed_widget_cb), &option);
    option.set_ui_item(std::move(ui_item));
    option.set_ui_item_from_option();

    gtk_box_pack_start(GTK_BOX(*enclosing), widget, FALSE, FALSE, 0);
    gtk_widget_show_all(*enclosing);
    return widget;
}

class GncGtkCommodityUIItem : public GncOptionGtkUIItem
{
public:
    GncGtkCommodityUIItem(GtkWidget* widget) :
        GncOptionGtkUIItem{widget, GncOptionUIType::COMMODITY} {}
    void set_ui_item_from_option(GncOption& option) noexcept override
    {
        auto widget{GNC_GENERAL_SELECT(get_widget())};
        auto instance{option.get_value<const QofInstance*>()};
        if (instance)
            gnc_general_select_set_selected(widget, GNC_COMMODITY(instance));
    }
    void set_option_from_ui_item(GncOption& option) noexcept override
    {
        auto widget{GNC_GENERAL_SELECT(get_widget())};
        auto commodity{gnc_general_select_get_selected(widget)};
        option.set_value(qof_instance_cast(commodity));
    }
};

template<> GtkWidget*
create_option_widget<GncOptionUIType::COMMODITY> (GncOption& option, GtkGrid *page_box,
                                    GtkLabel *name_label, char *documentation,
                                    /* Return values */
                                    GtkWidget **enclosing, bool *packed)
{
    *enclosing = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_set_homogeneous (GTK_BOX (*enclosing), FALSE);
    auto widget = gnc_general_select_new(GNC_GENERAL_SELECT_TYPE_SELECT,
                                   gnc_commodity_edit_get_string,
                                   gnc_commodity_edit_new_select,
                                   NULL);

    auto ui_item{std::make_unique<GncGtkCommodityUIItem>(widget)};

    g_signal_connect(G_OBJECT(widget), "changed",
                     G_CALLBACK(gnc_option_changed_widget_cb), &option);
    option.set_ui_item(std::move(ui_item));
    option.set_ui_item_from_option();

    if (documentation != NULL)
        gtk_widget_set_tooltip_text(GNC_GENERAL_SELECT(widget)->entry,
                                    documentation);

    g_signal_connect(G_OBJECT(GNC_GENERAL_SELECT(widget)->entry), "changed",
                     G_CALLBACK(gnc_option_changed_widget_cb), &option);

    gtk_box_pack_start(GTK_BOX(*enclosing), widget, FALSE, FALSE, 0);
    gtk_widget_show_all(*enclosing);
    return widget;
}

static GtkWidget*
create_multichoice_widget(GncOption& option)
{
    auto num_values = option.num_permissible_values();

    g_return_val_if_fail(num_values >= 0, NULL);
    auto renderer = gtk_cell_renderer_text_new();
    auto store = gtk_list_store_new(1, G_TYPE_STRING);
    /* Add values to the list store, entry and tooltip */
    for (decltype(num_values) i = 0; i < num_values; i++)
    {
        GtkTreeIter iter;
        auto itemstring = option.permissible_value_name(i);
        gtk_list_store_append (store, &iter);
        gtk_list_store_set(store, &iter, 0,
                           (itemstring && *itemstring) ? _(itemstring) : "", -1);
    }
    /* Create the new Combo with tooltip and add the store */
    auto widget{GTK_WIDGET(gtk_combo_box_new_with_model(GTK_TREE_MODEL(store)))};
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT(widget), renderer, TRUE);
    gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT(widget),
                                   renderer, "text", 0);
    g_object_unref(store);

    g_signal_connect(G_OBJECT(widget), "changed",
                     G_CALLBACK(gnc_option_changed_widget_cb), &option);

    return widget;
}

class GncGtkMultichoiceUIItem : public GncOptionGtkUIItem
{
public:
    GncGtkMultichoiceUIItem(GtkWidget* widget) :
        GncOptionGtkUIItem{widget, GncOptionUIType::MULTICHOICE} {}
    void set_ui_item_from_option(GncOption& option) noexcept override
    {
        auto widget{GTK_COMBO_BOX(get_widget())};
        gtk_combo_box_set_active(widget, option.get_value<size_t>());
    }
    void set_option_from_ui_item(GncOption& option) noexcept override
    {
        auto widget{GTK_COMBO_BOX(get_widget())};
        option.set_value<size_t>(static_cast<size_t>(gtk_combo_box_get_active(widget)));
    }
};

template<> GtkWidget*
create_option_widget<GncOptionUIType::MULTICHOICE> (GncOption& option, GtkGrid *page_box,
                                      GtkLabel *name_label, char *documentation,
                                      /* Return values */
                                      GtkWidget **enclosing, bool *packed)
{
    *enclosing = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_set_homogeneous (GTK_BOX (*enclosing), FALSE);

    auto widget = create_multichoice_widget(option);
    auto ui_item{std::make_unique<GncGtkMultichoiceUIItem>(widget)};

    option.set_ui_item(std::move(ui_item));
    option.set_ui_item_from_option();

    gtk_box_pack_start(GTK_BOX(*enclosing), widget, FALSE, FALSE, 0);
    gtk_widget_show_all(*enclosing);
    return widget;
}


class GncDateEntry
{
public:
    GncDateEntry() = default;
    virtual ~GncDateEntry() = default;
    virtual void set_entry_from_option(GncOption& option) = 0;
    virtual void set_option_from_entry(GncOption& option) = 0;
    // Get the widget that has data
    virtual GtkWidget* get_entry() = 0;
    // Get the widget that gets put on the page
    virtual GtkWidget* get_widget() = 0;
    virtual void toggle_relative(bool) {} //BothDateEntry only
};


using GncDateEntryPtr = std::unique_ptr<GncDateEntry>;

class AbsoluteDateEntry : public GncDateEntry
{
public:
    AbsoluteDateEntry(GncOption& option);
    ~AbsoluteDateEntry() = default;
    void set_entry_from_option(GncOption& option) override;
    void set_option_from_entry(GncOption& option) override;
    GtkWidget* get_entry() override { return GTK_WIDGET(m_entry); }
    GtkWidget* get_widget() override { return GTK_WIDGET(m_entry); }
private:
    GNCDateEdit* m_entry;
};

AbsoluteDateEntry::AbsoluteDateEntry(GncOption& option) :
    m_entry{GNC_DATE_EDIT(gnc_date_edit_new(time(NULL), FALSE, FALSE))}
{
    auto entry = GNC_DATE_EDIT(m_entry)->date_entry;
    g_signal_connect(G_OBJECT(entry), "changed",
                     G_CALLBACK(gnc_option_changed_option_cb), &option);
}

void
AbsoluteDateEntry::set_entry_from_option(GncOption& option)
{
    gnc_date_edit_set_time(m_entry, option.get_value<time64>());
}

void
AbsoluteDateEntry::set_option_from_entry(GncOption& option)
{
    option.set_value<time64>(gnc_date_edit_get_date(m_entry));
}

class RelativeDateEntry : public GncDateEntry
{
public:
    RelativeDateEntry(GncOption& option);
    ~RelativeDateEntry() = default;
    void set_entry_from_option(GncOption& option) override;
    void set_option_from_entry(GncOption& option) override;
    GtkWidget* get_widget() override { return m_entry; }
    GtkWidget* get_entry() override { return m_entry; }
private:
    GtkWidget* m_entry;
};


RelativeDateEntry::RelativeDateEntry(GncOption& option)
{

    auto renderer = gtk_cell_renderer_text_new();
    auto store = gtk_list_store_new(1, G_TYPE_STRING);
    /* Add values to the list store, entry and tooltip */
    auto num = option.num_permissible_values();
    for (decltype(num) index = 0; index < num; ++index)
    {
        GtkTreeIter  iter;
        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter, 0,
                            option.permissible_value_name(index), -1);
    }

    /* Create the new Combo with tooltip and add the store */
    m_entry = GTK_WIDGET(gtk_combo_box_new_with_model(GTK_TREE_MODEL(store)));
    gtk_combo_box_set_active(GTK_COMBO_BOX(m_entry), 0);
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT(m_entry), renderer, TRUE);
    gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT(m_entry),
                                   renderer, "text", 0);

    g_object_unref(store);

    g_signal_connect(G_OBJECT(m_entry), "changed",
                     G_CALLBACK(gnc_option_changed_widget_cb), &option);
}

void
RelativeDateEntry::set_entry_from_option(GncOption& option)
{
    gtk_combo_box_set_active(GTK_COMBO_BOX(m_entry), option.get_value<size_t>());
}

void
RelativeDateEntry::set_option_from_entry(GncOption& option)
{
    option.set_value<size_t>(gtk_combo_box_get_active(GTK_COMBO_BOX(m_entry)));
}

using AbsoluteDateEntryPtr = std::unique_ptr<AbsoluteDateEntry>;
using RelativeDateEntryPtr = std::unique_ptr<RelativeDateEntry>;

class BothDateEntry : public GncDateEntry
{
public:
    BothDateEntry(GncOption& option);
    ~BothDateEntry() = default; //The GncOptionGtkUIItem owns the widget
    void set_entry_from_option(GncOption& option) override;
    void set_option_from_entry(GncOption& option) override;
    GtkWidget* get_widget() override { return m_widget; }
    GtkWidget* get_entry() override;
    void toggle_relative(bool use_absolute) override;
private:
    GtkWidget* m_widget;
    GtkWidget* m_abs_button;
    AbsoluteDateEntryPtr m_abs_entry;
    GtkWidget* m_rel_button;
    RelativeDateEntryPtr m_rel_entry;
    bool m_use_absolute = true;
};

static void date_set_absolute_cb(GtkWidget *widget, gpointer data1);
static void date_set_relative_cb(GtkWidget *widget, gpointer data1);

BothDateEntry::BothDateEntry(GncOption& option) :
    m_widget{gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5)},
    m_abs_button{gtk_radio_button_new(NULL)},
    m_abs_entry{std::make_unique<AbsoluteDateEntry>(option)},
    m_rel_button{
        gtk_radio_button_new_from_widget(GTK_RADIO_BUTTON(m_abs_button))},
    m_rel_entry{std::make_unique<RelativeDateEntry>(option)}
{
    gtk_box_set_homogeneous (GTK_BOX(m_widget), FALSE);
    g_signal_connect(G_OBJECT(m_abs_button), "toggled",
                     G_CALLBACK(date_set_absolute_cb), &option);
    g_signal_connect(G_OBJECT(m_rel_button), "toggled",
                     G_CALLBACK(date_set_relative_cb), &option);

    gtk_box_pack_start(GTK_BOX(m_widget),
                       m_abs_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(m_widget),
                       m_abs_entry->get_entry(), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(m_widget),
                       m_rel_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(m_widget),
                       m_rel_entry->get_entry(), FALSE, FALSE, 0);

}

GtkWidget*
BothDateEntry::get_entry()
{
    return m_use_absolute ? m_abs_entry->get_entry() : m_rel_entry->get_entry();
}

void
BothDateEntry::toggle_relative(bool use_absolute)
{
    m_use_absolute = use_absolute;
}

void
BothDateEntry::set_entry_from_option(GncOption& option)
{
    if (m_use_absolute)
        m_abs_entry->set_entry_from_option(option);
    else
        m_rel_entry->set_entry_from_option(option);
}

void
BothDateEntry::set_option_from_entry(GncOption& option)
{
    if (m_use_absolute)
        m_abs_entry->set_option_from_entry(option);
    else
        m_rel_entry->set_option_from_entry(option);
}


class GncOptionDateUIItem : public GncOptionGtkUIItem
{
public:
    GncOptionDateUIItem(GncDateEntryPtr entry, GncOptionUIType type) :
        GncOptionGtkUIItem{nullptr, type}, m_entry{std::move(entry)} { }
    ~GncOptionDateUIItem() = default;
    void clear_ui_item() override { m_entry = nullptr; }
    void set_ui_item_from_option(GncOption& option) noexcept override
    {
        if (m_entry)
            m_entry->set_entry_from_option(option);
    }
    void set_option_from_ui_item(GncOption& option) noexcept override
    {
        if (m_entry)
            m_entry->set_option_from_entry(option);
    }
    GtkWidget* const get_widget() const override
    {
        return m_entry->get_widget();
    }
    GncDateEntry* get_entry() { return m_entry.get(); }
private:
    GncDateEntryPtr m_entry;
};

static void
date_set_absolute_cb(GtkWidget *widget, gpointer data1)
{
    GncOption* option = static_cast<decltype(option)>(data1);
    auto ui_item = option->get_ui_item();
    if (auto date_ui = dynamic_cast<const GncOptionDateUIItem* const>(ui_item))
    {
        const_cast<GncOptionDateUIItem*>(date_ui)->get_entry()->toggle_relative(true);
        gnc_option_changed_option_cb(widget, option);
    }
}

static void
date_set_relative_cb(GtkWidget *widget, gpointer data1)
{
    GncOption* option = static_cast<decltype(option)>(data1);
    auto ui_item = option->get_ui_item();
    if (auto date_ui = dynamic_cast<const GncOptionDateUIItem* const>(ui_item))
    {
        const_cast<GncOptionDateUIItem*>(date_ui)->get_entry()->toggle_relative(false);
        gnc_option_changed_option_cb(widget, option);
    }
}

static GtkWidget*
create_date_option_widget(GncOption& option, GtkGrid *page_box,
                          GtkLabel *name_label, char *documentation,
                               /* Return values */
                          GtkWidget **enclosing, bool *packed)
{
    auto grid_row = GPOINTER_TO_INT(g_object_get_data
                                    (G_OBJECT(page_box), "options-grid-row"));
    auto type = option.get_ui_type();
    switch (type)
    {
        case GncOptionUIType::DATE_ABSOLUTE:
            option.set_ui_item(std::make_unique<GncOptionDateUIItem>(std::make_unique<AbsoluteDateEntry>(option), type));
            break;
        case GncOptionUIType::DATE_RELATIVE:
            option.set_ui_item(std::make_unique<GncOptionDateUIItem>(std::make_unique<RelativeDateEntry>(option), type));
            break;
        case GncOptionUIType::DATE_BOTH:
            option.set_ui_item(std::make_unique<GncOptionDateUIItem>(std::make_unique<BothDateEntry>(option), type));
            break;
        default:
            PERR("Attempted to create date option widget with wrong UI type %d",
                 static_cast<int>(type));
            return nullptr;
            break;
    }

    option.set_ui_item_from_option();
    auto widget{gnc_option_get_gtk_widget(&option)};
    if (type == GncOptionUIType::DATE_RELATIVE)
    {
        *enclosing = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
        gtk_box_set_homogeneous (GTK_BOX (*enclosing), FALSE);

        gtk_box_pack_start(GTK_BOX(*enclosing), widget, FALSE, FALSE, 0);
    }
    else
    {
        *enclosing = gtk_frame_new (NULL);
        g_object_set (G_OBJECT(widget), "margin", 3, NULL);

        gtk_container_add (GTK_CONTAINER(*enclosing), widget);
    }

    gtk_widget_set_halign (GTK_WIDGET(*enclosing), GTK_ALIGN_START);

    /* Pack option widget into an extra eventbox because otherwise the
       "documentation" tooltip is not displayed. */
    auto eventbox = gtk_event_box_new();
    gtk_container_add (GTK_CONTAINER (eventbox), *enclosing);

    gtk_grid_attach (GTK_GRID(page_box), eventbox, 1, grid_row, 1, 1);
    *packed = TRUE;

    gtk_widget_set_tooltip_text (eventbox, documentation);

    gtk_widget_show_all(*enclosing);
    return widget;
}

template<> GtkWidget*
create_option_widget<GncOptionUIType::DATE_ABSOLUTE>(GncOption& option,
                                                     GtkGrid *page_box,
                                                     GtkLabel *name_label,
                                                     char *documentation,
                                                     /* Return values */
                                                     GtkWidget **enclosing,
                                                     bool *packed)
{
    return create_date_option_widget(option, page_box, name_label,
                                     documentation, enclosing, packed);
}

template<> GtkWidget*
create_option_widget<GncOptionUIType::DATE_RELATIVE>(GncOption& option,
                                                     GtkGrid *page_box,
                                                     GtkLabel *name_label,
                                                     char *documentation,
                                                     /* Return values */
                                                     GtkWidget **enclosing,
                                                     bool *packed)
{
    return create_date_option_widget(option, page_box, name_label,
                                     documentation, enclosing, packed);
}

template<> GtkWidget*
create_option_widget<GncOptionUIType::DATE_BOTH>(GncOption& option,
                                                 GtkGrid *page_box,
                                                 GtkLabel *name_label,
                                                 char *documentation,
                                                 /* Return values */
                                                 GtkWidget **enclosing,
                                                 bool *packed)
{
    return create_date_option_widget(option, page_box, name_label,
                                     documentation, enclosing, packed);
}

using GncOptionAccountList = std::vector<const Account*>;

static void
account_select_all_cb(GtkWidget *widget, gpointer data)
{
    GncOption* option = static_cast<decltype(option)>(data);
    GncTreeViewAccount *tree_view;
    GtkTreeSelection *selection;

    tree_view = GNC_TREE_VIEW_ACCOUNT(gnc_option_get_gtk_widget (option));
    gtk_tree_view_expand_all(GTK_TREE_VIEW(tree_view));
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
    gtk_tree_selection_select_all(selection);
    gnc_option_changed_widget_cb(widget, option);
}

static void
account_clear_all_cb(GtkWidget *widget, gpointer data)
{
    GncOption* option = static_cast<decltype(option)>(data);
    GncTreeViewAccount *tree_view;
    GtkTreeSelection *selection;

    tree_view = GNC_TREE_VIEW_ACCOUNT(gnc_option_get_gtk_widget (option));
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
    gtk_tree_selection_unselect_all(selection);
    gnc_option_changed_widget_cb(widget, option);
}

static void
account_select_children_cb(GtkWidget *widget, gpointer data)
{
    GncOption* option = static_cast<decltype(option)>(data);
    GncTreeViewAccount *tree_view;
    GList *acct_list = NULL, *acct_iter = NULL;

    tree_view = GNC_TREE_VIEW_ACCOUNT(gnc_option_get_gtk_widget (option));
    acct_list = gnc_tree_view_account_get_selected_accounts (tree_view);

    for (acct_iter = acct_list; acct_iter; acct_iter = acct_iter->next)
        gnc_tree_view_account_select_subaccounts (tree_view, static_cast<Account*>(acct_iter->data));

    g_list_free (acct_list);
}

static void
account_set_default_cb(GtkWidget* widget, gpointer data)
{
    GncOption* option = static_cast<decltype(option)>(data);
    account_clear_all_cb(widget, data);
    option->set_value(option->get_default_value<GncOptionAccountList>());
    option->set_ui_item_from_option();
}

static void
show_hidden_toggled_cb(GtkWidget *widget, GncOption* option)
{
    if (option->get_ui_type() != GncOptionUIType::ACCOUNT_LIST &&
        option->get_ui_type() != GncOptionUIType::ACCOUNT_SEL)
        return;

    auto tree_view = GNC_TREE_VIEW_ACCOUNT(gnc_option_get_gtk_widget(option));
    AccountViewInfo avi;
    gnc_tree_view_account_get_view_info (tree_view, &avi);
    avi.show_hidden = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    gnc_tree_view_account_set_view_info (tree_view, &avi);
    gnc_option_changed_widget_cb(widget, option);
}

class GncGtkAccountListUIItem : public GncOptionGtkUIItem
{
public:
    GncGtkAccountListUIItem(GtkWidget* widget) :
        GncOptionGtkUIItem{widget, GncOptionUIType::ACCOUNT_LIST} {}
    void set_ui_item_from_option(GncOption& option) noexcept override
    {
        auto widget{GNC_TREE_VIEW_ACCOUNT(get_widget())};
        GList *acc_list = nullptr;
        const GncOptionAccountList& accounts =
            option.get_value<GncOptionAccountList>();
        for (auto account : accounts)
            acc_list = g_list_prepend(acc_list, static_cast<void*>(const_cast<Account*>(account)));
        acc_list = g_list_reverse(acc_list);
        gnc_tree_view_account_set_selected_accounts(widget, acc_list, TRUE);
        g_list_free(acc_list);
    }
    void set_option_from_ui_item(GncOption& option) noexcept override
    {
        auto widget{GNC_TREE_VIEW_ACCOUNT(get_widget())};
        auto acc_list = gnc_tree_view_account_get_selected_accounts(widget);
        GncOptionAccountList acc_vec;
        acc_vec.reserve(g_list_length(acc_list));
        for (auto node = acc_list; node; node = g_list_next(node))
            acc_vec.push_back(static_cast<const Account*>(node->data));
        g_list_free(acc_list);
        option.set_value(acc_vec);
    }
};

static GtkWidget*
create_account_widget(GncOption& option, char *name)
{
    bool multiple_selection;
    GtkWidget *scroll_win;
    GtkWidget *button;
    GtkWidget *frame;
    GtkWidget *tree;
    GtkWidget *vbox;
    GtkWidget *bbox;
    GList *acct_type_list;
    GtkTreeSelection *selection;

    multiple_selection = option.is_multiselect();
    acct_type_list = option.account_type_list();

    frame = gtk_frame_new(name);

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_set_homogeneous (GTK_BOX (vbox), FALSE);

    gtk_container_add(GTK_CONTAINER(frame), vbox);

    tree = GTK_WIDGET(gnc_tree_view_account_new (FALSE));
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW(tree), FALSE);
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(tree));
    if (multiple_selection)
        gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);
    else
        gtk_tree_selection_set_mode (selection, GTK_SELECTION_BROWSE);

    if (acct_type_list)
    {
        GList *node;
        AccountViewInfo avi;
        int i;

        gnc_tree_view_account_get_view_info (GNC_TREE_VIEW_ACCOUNT (tree), &avi);

        for (i = 0; i < NUM_ACCOUNT_TYPES; i++)
            avi.include_type[i] = FALSE;
        avi.show_hidden = FALSE;

        for (node = acct_type_list; node; node = node->next)
        {
            GNCAccountType type = static_cast<decltype(type)>(GPOINTER_TO_INT (node->data));
            avi.include_type[type] = TRUE;
        }

        gnc_tree_view_account_set_view_info (GNC_TREE_VIEW_ACCOUNT (tree), &avi);
        g_list_free (acct_type_list);
    }
    else
    {
        AccountViewInfo avi;
        int i;

        gnc_tree_view_account_get_view_info (GNC_TREE_VIEW_ACCOUNT (tree), &avi);

        for (i = 0; i < NUM_ACCOUNT_TYPES; i++)
            avi.include_type[i] = TRUE;
        avi.show_hidden = FALSE;
        gnc_tree_view_account_set_view_info (GNC_TREE_VIEW_ACCOUNT (tree), &avi);
    }

    scroll_win = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_win),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);

    gtk_box_pack_start(GTK_BOX(vbox), scroll_win, TRUE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(scroll_win), 5);
    gtk_container_add(GTK_CONTAINER(scroll_win), tree);

    bbox = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_SPREAD);
    gtk_box_pack_start(GTK_BOX(vbox), bbox, FALSE, FALSE, 10);

    if (multiple_selection)
    {
        button = gtk_button_new_with_label(_("Select All"));
        gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 0);
        gtk_widget_set_tooltip_text(button, _("Select all accounts."));

        g_signal_connect(G_OBJECT(button), "clicked",
                         G_CALLBACK(account_select_all_cb), &option);

        button = gtk_button_new_with_label(_("Clear All"));
        gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 0);
        gtk_widget_set_tooltip_text(button, _("Clear the selection and unselect all accounts."));

        g_signal_connect(G_OBJECT(button), "clicked",
                         G_CALLBACK(account_clear_all_cb), &option);

        button = gtk_button_new_with_label(_("Select Children"));
        gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 0);
        gtk_widget_set_tooltip_text(button, _("Select all descendents of selected account."));

        g_signal_connect(G_OBJECT(button), "clicked",
                         G_CALLBACK(account_select_children_cb), &option);
    }

    button = gtk_button_new_with_label(_("Select Default"));
    gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(button, _("Select the default account selection."));

    g_signal_connect(G_OBJECT(button), "clicked",
                     G_CALLBACK(account_set_default_cb), &option);

    gtk_widget_set_margin_start (GTK_WIDGET(bbox), 6);
    gtk_widget_set_margin_end (GTK_WIDGET(bbox), 6);

    if (multiple_selection)
    {
        /* Put the "Show hidden" checkbox on a separate line since
           the 4 buttons make the dialog too wide. */
        bbox = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
        gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_START);
        gtk_box_pack_start(GTK_BOX(vbox), bbox, FALSE, FALSE, 0);
    }

    button = gtk_check_button_new_with_label(_("Show Hidden Accounts"));
    gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(button, _("Show accounts that have been marked hidden."));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), FALSE);
    g_signal_connect(G_OBJECT(button), "toggled",
                     G_CALLBACK(show_hidden_toggled_cb), &option);

    option.set_ui_item(std::make_unique<GncGtkAccountListUIItem>(tree));
    option.set_ui_item_from_option();

    return frame;
}

template<> GtkWidget*
create_option_widget<GncOptionUIType::ACCOUNT_LIST>(GncOption& option,
                                                    GtkGrid *page_box,
                                                    GtkLabel *name_label,
                                                    char *documentation,
                                                    /* Return values */
                                                    GtkWidget **enclosing,
                                                    bool *packed)
{
    align_label (name_label);

    *enclosing = create_account_widget(option, NULL);
    gtk_widget_set_vexpand (GTK_WIDGET(*enclosing), TRUE);
    gtk_widget_set_hexpand (GTK_WIDGET(*enclosing), TRUE);

    gtk_widget_set_tooltip_text(*enclosing, documentation);

    auto  grid_row = GPOINTER_TO_INT(g_object_get_data
                                    (G_OBJECT(page_box), "options-grid-row"));
    gtk_grid_attach (GTK_GRID(page_box), *enclosing, 1, grid_row, 1, 1);
    *packed = TRUE;

    auto widget = gnc_option_get_gtk_widget(&option);
    auto selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
    g_signal_connect(G_OBJECT(selection), "changed",
                     G_CALLBACK(gnc_option_changed_widget_cb), &option);

    //  gtk_clist_set_row_height(GTK_CLIST(value), 0);
    //  gtk_widget_set_size_request(value, -1, GTK_CLIST(value)->row_height * 10);
    gtk_widget_show_all(*enclosing);
    return widget;
}

class GncGtkAccountSelUIItem : public GncOptionGtkUIItem
{
public:
    GncGtkAccountSelUIItem(GtkWidget* widget) :
        GncOptionGtkUIItem{widget, GncOptionUIType::ACCOUNT_SEL} {}
    void set_ui_item_from_option(GncOption& option) noexcept override
    {
        auto widget{GNC_ACCOUNT_SEL(get_widget())};
        auto instance{option.get_value<const QofInstance*>()};
        if (instance)
            gnc_account_sel_set_account(widget, GNC_ACCOUNT(instance), FALSE);
    }
    void set_option_from_ui_item(GncOption& option) noexcept override
    {
        auto widget{GNC_ACCOUNT_SEL(get_widget())};
        option.set_value(qof_instance_cast((gnc_account_sel_get_account(widget))));
    }
};

template<> GtkWidget*
create_option_widget<GncOptionUIType::ACCOUNT_SEL> (GncOption& option,
                                                    GtkGrid *page_box,
                                                    GtkLabel *name_label,
                                                    char *documentation,
                                                    /* Return values */
                                                    GtkWidget **enclosing,
                                                    bool *packed)
{
    auto acct_type_list = option.account_type_list();
    auto widget = gnc_account_sel_new ();
    gnc_account_sel_set_acct_filters(GNC_ACCOUNT_SEL(widget),
                                     acct_type_list, NULL);
    g_list_free(acct_type_list);
    g_signal_connect(widget, "account_sel_changed",
                     G_CALLBACK(gnc_option_changed_widget_cb), &option);


// gnc_account_sel doesn't emit a changed signal
    option.set_ui_item(std::make_unique<GncGtkAccountSelUIItem>(widget));
    option.set_ui_item_from_option();

    *enclosing = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_set_homogeneous (GTK_BOX (*enclosing), FALSE);
    gtk_box_pack_start(GTK_BOX(*enclosing), widget, FALSE, FALSE, 0);
    gtk_widget_show_all(*enclosing);
    return widget;
}

static void
list_changed_cb(GtkTreeSelection *selection, GncOption* option)
{
    GtkTreeView *view = GTK_TREE_VIEW(gnc_option_get_gtk_widget (option));
    gnc_option_changed_widget_cb(GTK_WIDGET(view), option);
}

static void
list_select_all_cb(GtkWidget *widget, gpointer data)
{
    GncOption* option = static_cast<decltype(option)>(data);
    GtkTreeView *view;
    GtkTreeSelection *selection;

    view = GTK_TREE_VIEW(gnc_option_get_gtk_widget(option));
    selection = gtk_tree_view_get_selection(view);
    gtk_tree_selection_select_all(selection);
    gnc_option_changed_widget_cb(GTK_WIDGET(view), option);
}

static void
list_clear_all_cb(GtkWidget *widget, gpointer data)
{
    GncOption* option = static_cast<decltype(option)>(data);
    GtkTreeView *view;
    GtkTreeSelection *selection;

    view = GTK_TREE_VIEW(gnc_option_get_gtk_widget(option));
    selection = gtk_tree_view_get_selection(view);
    gtk_tree_selection_unselect_all(selection);
    gnc_option_changed_widget_cb(GTK_WIDGET(view), option);
}

static void
list_set_default_cb(GtkWidget *widget, gpointer data)
{
    GncOption* option = static_cast<decltype(option)>(data);
    list_clear_all_cb(widget, data);
    option->set_value(option->get_default_value<GncMultichoiceOptionIndexVec>());
    option->set_ui_item_from_option();
}

class GncGtkListUIItem : public GncOptionGtkUIItem
{
public:
    GncGtkListUIItem(GtkWidget* widget) :
        GncOptionGtkUIItem{widget, GncOptionUIType::LIST} {}
    void set_ui_item_from_option(GncOption& option) noexcept override
    {
        auto widget{GTK_TREE_VIEW(get_widget())};
        auto selection{gtk_tree_view_get_selection(widget)};
        gtk_tree_selection_unselect_all(selection);
        for (auto index : option.get_value<GncMultichoiceOptionIndexVec>())
        {
            auto path{gtk_tree_path_new_from_indices(index, -1)};
            gtk_tree_selection_select_path(selection, path);
            gtk_tree_path_free(path);
        }
    }
    void set_option_from_ui_item(GncOption& option) noexcept override
    {
        auto widget{GTK_TREE_VIEW(get_widget())};
        auto selection{gtk_tree_view_get_selection(widget)};
        auto rows{option.num_permissible_values()};
        GncMultichoiceOptionIndexVec vec;
        for (size_t row = 0; row < rows; ++row)
        {
            auto path{gtk_tree_path_new_from_indices(row, -1)};
            auto selected{gtk_tree_selection_path_is_selected(selection, path)};
            gtk_tree_path_free(path);
            if (selected)
                vec.push_back(row);
        }
        option.set_value(vec);
    }
};

static GtkWidget *
create_list_widget(GncOption& option, char *name)
{
    auto frame = gtk_frame_new(name);
    auto hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_set_homogeneous (GTK_BOX (hbox), FALSE);
    gtk_container_add(GTK_CONTAINER(frame), hbox);

    auto store = gtk_list_store_new(1, G_TYPE_STRING);
    auto view = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(store)));
    g_object_unref(store);
    auto renderer = gtk_cell_renderer_text_new();
    auto column = gtk_tree_view_column_new_with_attributes("", renderer,
                                                           "text", 0,
                                                           NULL);
    gtk_tree_view_append_column(view, column);
    gtk_tree_view_set_headers_visible(view, FALSE);

    auto num_values = option.num_permissible_values();
    for (decltype(num_values) i = 0; i < num_values; i++)
    {
        GtkTreeIter iter;
        auto raw_string = option.permissible_value_name(i);
        auto string = (raw_string && *raw_string) ? _(raw_string) : "";
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, string ? string : "", -1);
    }

    gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(view), FALSE, FALSE, 0);

    auto selection = gtk_tree_view_get_selection(view);
    gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);
    g_signal_connect(selection, "changed",
                     G_CALLBACK(list_changed_cb), &option);

    auto bbox = gtk_button_box_new (GTK_ORIENTATION_VERTICAL);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_SPREAD);
    gtk_box_pack_end(GTK_BOX(hbox), bbox, FALSE, FALSE, 0);

    auto button = gtk_button_new_with_label(_("Select All"));
    gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(button, _("Select all entries."));

    g_signal_connect(G_OBJECT(button), "clicked",
                     G_CALLBACK(list_select_all_cb), &option);

    button = gtk_button_new_with_label(_("Clear All"));
    gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(button, _("Clear the selection and unselect all entries."));

    g_signal_connect(G_OBJECT(button), "clicked",
                     G_CALLBACK(list_clear_all_cb), &option);

    button = gtk_button_new_with_label(_("Select Default"));
    gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text(button, _("Select the default selection."));

    g_signal_connect(G_OBJECT(button), "clicked",
                     G_CALLBACK(list_set_default_cb), &option);

    g_object_set (G_OBJECT(hbox), "margin", 3, NULL);

    option.set_ui_item(std::make_unique<GncGtkListUIItem>(GTK_WIDGET(view)));
    option.set_ui_item_from_option();

    return frame;
}

template<> GtkWidget *
create_option_widget<GncOptionUIType::LIST> (GncOption& option,
                                             GtkGrid *page_box,
                                             GtkLabel *name_label,
                                             char *documentation,
                                             /* Return values */
                                             GtkWidget **enclosing,
                                             bool *packed)
{
    auto grid_row = GPOINTER_TO_INT(g_object_get_data
                                    (G_OBJECT(page_box), "options-grid-row"));

    *enclosing = create_list_widget(option, NULL);
    auto value = gnc_option_get_gtk_widget(&option);

    align_label (name_label);

    /* Pack option widget into an extra eventbox because otherwise the
       "documentation" tooltip is not displayed. */
    auto eventbox = gtk_event_box_new();
    gtk_container_add (GTK_CONTAINER (eventbox), *enclosing);

    gtk_grid_attach (GTK_GRID(page_box), eventbox, 1, grid_row, 1, 1);
    *packed = TRUE;

    gtk_widget_set_tooltip_text(eventbox, documentation);

    gtk_widget_show(*enclosing);
    return value;
}

class GncGtkNumberRangeUIItem : public GncOptionGtkUIItem
{
public:
    GncGtkNumberRangeUIItem(GtkWidget* widget) :
        GncOptionGtkUIItem{widget, GncOptionUIType::NUMBER_RANGE} {}
    void set_ui_item_from_option(GncOption& option) noexcept override
    {
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(get_widget()),
                                  option.get_value<double>());
    }
    void set_option_from_ui_item(GncOption& option) noexcept override
    {
        option.set_value<double>(gtk_spin_button_get_value(GTK_SPIN_BUTTON(get_widget())));
    }
};

/* New spin button configured with the values provided by the passed-in
 * GncOption, which had better contain a GncOptionRangeValue.
 *
 * Also used to set up the pixel spinner in the plot_size control.
 */
static GtkSpinButton*
create_range_spinner(GncOption& option)
{
    gdouble lower_bound = G_MINDOUBLE;
    gdouble upper_bound = G_MAXDOUBLE;
    gdouble step_size = 1.0;

    option.get_limits(upper_bound, lower_bound, step_size);
    auto adj = GTK_ADJUSTMENT(gtk_adjustment_new(lower_bound, lower_bound,
                                                 upper_bound, step_size,
                                                 step_size * 5.0,
                                                 0));

    size_t num_decimals = 0;
    for (auto steps = step_size; steps < 1; steps *= 10)
        ++num_decimals;
    auto widget = gtk_spin_button_new(adj, step_size, num_decimals);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(widget), TRUE);

    size_t num_digits = 0;
    for (auto bigger = MAX(ABS(lower_bound), ABS(upper_bound));
         bigger >= 1; bigger /= 10.0)
        ++num_digits;
    num_digits += num_decimals;
    gtk_entry_set_width_chars(GTK_ENTRY(widget), num_digits);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),
                              (upper_bound / 2)); //default
    return GTK_SPIN_BUTTON(widget);
}

template<> GtkWidget *
create_option_widget<GncOptionUIType::NUMBER_RANGE> (GncOption& option,
                                                     GtkGrid *page_box,
                                                     GtkLabel *name_label,
                                                     char *documentation,
                                                     /* Return values */
                                                     GtkWidget **enclosing,
                                                     bool *packed)
{
    GtkAdjustment *adj;
    size_t num_decimals = 0;

    *enclosing = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_set_homogeneous (GTK_BOX (*enclosing), FALSE);

    auto widget = create_range_spinner(option);
    option.set_ui_item(std::make_unique<GncGtkNumberRangeUIItem>(GTK_WIDGET(widget)));
    option.set_ui_item_from_option();

    g_signal_connect(G_OBJECT(widget), "changed",
                     G_CALLBACK(gnc_option_changed_widget_cb), &option);

    gtk_box_pack_start(GTK_BOX(*enclosing), GTK_WIDGET(widget),
                       FALSE, FALSE, 0);
    gtk_widget_show_all(*enclosing);
    return GTK_WIDGET(widget);
}

class GncGtkColorUIItem : public GncOptionGtkUIItem
{
public:
    GncGtkColorUIItem(GtkWidget* widget) :
        GncOptionGtkUIItem{widget, GncOptionUIType::COLOR} {}
    void set_ui_item_from_option(GncOption& option) noexcept override
    {
        GdkRGBA color;
        /* gdk_rgba_parse uses pango_color_parse for hex color strings instead
         * of pango_color_parse_with_alpha and that fails if the string length
         * is 8.
        */
        auto value{option.get_value<std::string>().substr(0,6)};
        auto rgba_str{g_strdup_printf("#%s", value.c_str())};
        if (gdk_rgba_parse(&color, rgba_str))
        {
            auto color_button = GTK_COLOR_CHOOSER(get_widget());
            gtk_color_chooser_set_rgba(color_button, &color);
        }
        g_free(rgba_str);
     }
    void set_option_from_ui_item(GncOption& option) noexcept override
    {
        GdkRGBA color;
        auto color_button = GTK_COLOR_CHOOSER(get_widget());
        gtk_color_chooser_get_rgba(color_button, &color);
        auto rgba_str = g_strdup_printf("%2x%2x%2x%2x",
                                        (uint8_t)(color.red * 255),
                                        (uint8_t)(color.green * 255),
                                        (uint8_t)(color.blue * 255),
                                        (uint8_t)(color.alpha * 255));
        auto rgb_str = g_strdup_printf("%2x%2x%2x",
                                       (uint8_t)(color.red * 255),
                                       (uint8_t)(color.green * 255),
                                       (uint8_t)(color.blue * 255));
// Hello World uses an old HTML4 attribute that doesn't understand alpha.
        option.set_value(std::string{rgb_str});
        g_free(rgba_str);
    }
};

template<> GtkWidget *
create_option_widget<GncOptionUIType::COLOR> (GncOption& option, GtkGrid *page_box,
                                GtkLabel *name_label, char *documentation,
                                /* Return values */
                                GtkWidget **enclosing, bool *packed)
{
    bool use_alpha;

    *enclosing = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_set_homogeneous (GTK_BOX (*enclosing), FALSE);

    auto widget = gtk_color_button_new();
    gtk_color_chooser_set_use_alpha(GTK_COLOR_CHOOSER(widget), TRUE);

    option.set_ui_item(std::make_unique<GncGtkColorUIItem>(widget));
    option.set_ui_item_from_option();

    g_signal_connect(G_OBJECT(widget), "color-set",
                     G_CALLBACK(gnc_option_changed_widget_cb), &option);

    gtk_box_pack_start(GTK_BOX(*enclosing), widget, FALSE, FALSE, 0);
    gtk_widget_show_all(*enclosing);
    return widget;
}

class GncGtkFontUIItem : public GncOptionGtkUIItem
{
public:
    GncGtkFontUIItem(GtkWidget* widget) :
        GncOptionGtkUIItem{widget, GncOptionUIType::FONT} {}
    void set_ui_item_from_option(GncOption& option) noexcept override
    {
        GtkFontButton *font_button = GTK_FONT_BUTTON(get_widget());
        gtk_font_button_set_font_name(font_button,
                                      option.get_value<std::string>().c_str());

    }
    void set_option_from_ui_item(GncOption& option) noexcept override
    {
        GtkFontButton *font_button = GTK_FONT_BUTTON(get_widget());
        option.set_value(std::string{gtk_font_button_get_font_name(font_button)});
    }
};

template<> GtkWidget *
create_option_widget<GncOptionUIType::FONT> (GncOption& option, GtkGrid *page_box,
                               GtkLabel *name_label, char *documentation,
                               /* Return values */
                               GtkWidget **enclosing, bool *packed)
{
    *enclosing = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_set_homogeneous (GTK_BOX (*enclosing), FALSE);
    auto widget = gtk_font_button_new();
    g_object_set(G_OBJECT(widget),
                 "use-font", TRUE,
                 "show-style", TRUE,
                 "show-size", TRUE,
                 (char *)NULL);

    option.set_ui_item(std::make_unique<GncGtkFontUIItem>(widget));
    option.set_ui_item_from_option();

    g_signal_connect(G_OBJECT(widget), "font-set",
                     G_CALLBACK(gnc_option_changed_widget_cb), &option);

    gtk_box_pack_start(GTK_BOX(*enclosing), widget, FALSE, FALSE, 0);
    gtk_widget_show_all(*enclosing);
    return widget;
}

static void
update_preview_cb (GtkFileChooser *chooser, void* data)
{
    g_return_if_fail(chooser != NULL);

    ENTER("chooser %p", chooser);
    auto filename = gtk_file_chooser_get_preview_filename(chooser);
    DEBUG("chooser preview name is %s.", filename ? filename : "(null)");
    if (filename == NULL)
    {
        filename = g_strdup(static_cast<const char*>(g_object_get_data(G_OBJECT(chooser), LAST_SELECTION)));
        DEBUG("using last selection of %s", filename ? filename : "(null)");
        if (filename == NULL)
        {
            LEAVE("no usable name");
            return;
        }
    }

    auto image = GTK_IMAGE(gtk_file_chooser_get_preview_widget(chooser));
    auto pixbuf = gdk_pixbuf_new_from_file_at_size(filename, 128, 128, NULL);
    g_free(filename);
    auto have_preview = (pixbuf != NULL);

    gtk_image_set_from_pixbuf(image, pixbuf);
    if (pixbuf)
        g_object_unref(pixbuf);

    gtk_file_chooser_set_preview_widget_active(chooser, have_preview);
    LEAVE("preview visible is %d", have_preview);
}

static void
change_image_cb (GtkFileChooser *chooser, void* data)
{
    auto filename{gtk_file_chooser_get_preview_filename(chooser)};
    if (!filename)
        return;
    g_object_set_data_full(G_OBJECT(chooser), LAST_SELECTION, filename, g_free);
}

class GncGtkPixmapUIItem : public GncOptionGtkUIItem
{
public:
    GncGtkPixmapUIItem(GtkWidget* widget) :
        GncOptionGtkUIItem{widget, GncOptionUIType::PIXMAP} {}
    void set_ui_item_from_option(GncOption& option) noexcept override
    {
        auto string{option.get_value<std::string>()};
        if (!string.empty())
        {
            DEBUG("string = %s", string.c_str());
            auto chooser{GTK_FILE_CHOOSER(get_widget())};
            gtk_file_chooser_select_filename(chooser, string.c_str());
            auto filename{gtk_file_chooser_get_filename(chooser)};
            g_object_set_data_full(G_OBJECT(chooser), LAST_SELECTION,
                                   g_strdup(string.c_str()), g_free);
            DEBUG("Set %s, retrieved %s", string.c_str(),
                  filename ? filename : "(null)");
            update_preview_cb(chooser, &option);
        }
    }
    void set_option_from_ui_item(GncOption& option) noexcept override
    {
        auto string = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(get_widget()));
        DEBUG("filename %s", string ? string : "(null)");
        if (string)
        {
            option.set_value(std::string{string});
            g_free(string);
        }
    }
};

template<> GtkWidget *
create_option_widget<GncOptionUIType::PIXMAP> (GncOption& option,
                                               GtkGrid *page_box,
                                               GtkLabel *name_label,
                                               char *documentation,
                                               /* Return values */
                                               GtkWidget **enclosing,
                                               bool *packed)
{

    *enclosing = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_set_homogeneous (GTK_BOX (*enclosing), FALSE);

    auto button = gtk_button_new_with_label(_("Clear"));
    gtk_widget_set_tooltip_text(button, _("Clear any selected image file."));

    auto widget = gtk_file_chooser_button_new(_("Select image"),
                                        GTK_FILE_CHOOSER_ACTION_OPEN);
    gtk_widget_set_tooltip_text(widget, _("Select an image file."));
    g_object_set(G_OBJECT(widget),
                 "width-chars", 30,
                 "preview-widget", gtk_image_new(),
                 (char *)NULL);
    g_signal_connect(G_OBJECT (widget), "selection-changed",
                     G_CALLBACK(gnc_option_changed_widget_cb), &option);
    g_signal_connect(G_OBJECT (widget), "selection-changed",
                     G_CALLBACK(change_image_cb), &option);
    g_signal_connect(G_OBJECT (widget), "update-preview",
                     G_CALLBACK(update_preview_cb), &option);
    g_signal_connect_swapped(G_OBJECT (button), "clicked",
                             G_CALLBACK(gtk_file_chooser_unselect_all), widget);

    option.set_ui_item(std::make_unique<GncGtkPixmapUIItem>(widget));
    option.set_ui_item_from_option();

    gtk_box_pack_start(GTK_BOX(*enclosing), widget, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(*enclosing), button, FALSE, FALSE, 0);

    gtk_widget_show(widget);
    gtk_widget_show(*enclosing);

    return widget;
}

static void
radiobutton_set_cb(GtkWidget *w, gpointer data)
{
    GncOption* option = static_cast<decltype(option)>(data);
    gpointer _current, _new_value;
    gint current, new_value;

    auto widget = gnc_option_get_gtk_widget(option);

    _current = g_object_get_data(G_OBJECT(widget), "gnc_radiobutton_index");
    current = GPOINTER_TO_INT (_current);

    _new_value = g_object_get_data (G_OBJECT(w), "gnc_radiobutton_index");
    new_value = GPOINTER_TO_INT (_new_value);

    if (current == new_value)
        return;

    g_object_set_data (G_OBJECT(widget), "gnc_radiobutton_index",
                       GINT_TO_POINTER(new_value));
    gnc_option_changed_widget_cb(widget, option);
}

class GncGtkRadioButtonUIItem : public GncOptionGtkUIItem
{
public:
    GncGtkRadioButtonUIItem(GtkWidget* widget) :
        GncOptionGtkUIItem{widget, GncOptionUIType::RADIOBUTTON} {}
    void set_ui_item_from_option(GncOption& option) noexcept override
    {
        auto index{option.get_value<size_t>()};
        auto list{gtk_container_get_children(GTK_CONTAINER(get_widget()))};
        auto box{GTK_WIDGET(list->data)};
        g_list_free(list);

        list = gtk_container_get_children(GTK_CONTAINER(box));
        auto node{g_list_nth(list, index)};
        GtkButton* button{};
        if (node)
        {
            button = GTK_BUTTON(node->data);
        }
        else
        {
            PERR("Invalid Radio Button Selection %lu", index);
            g_list_free(list);
            return;
        }
        g_list_free(list);
        auto val{g_object_get_data (G_OBJECT (button),
                                    "gnc_radiobutton_index")};
        g_return_if_fail (GPOINTER_TO_UINT (val) == index);

        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
    }
    void set_option_from_ui_item(GncOption& option) noexcept override
    {
        auto index{g_object_get_data(G_OBJECT(get_widget()),
                                     "gnc_radiobutton_index")};
        option.set_value<size_t>(GPOINTER_TO_INT(index));
    }
};

static GtkWidget *
create_radiobutton_widget(char *name, GncOption& option)
{
    GtkWidget *frame, *box;
    GtkWidget *widget = NULL;

    auto num_values{option.num_permissible_values()};

    g_return_val_if_fail(num_values >= 0, NULL);

    /* Create our button frame */
    frame = gtk_frame_new (name);

    /* Create the button box */
    box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_set_homogeneous (GTK_BOX (box), FALSE);
    gtk_container_add (GTK_CONTAINER (frame), box);

    /* Iterate over the options and create a radio button for each one */
    for (decltype(num_values) i = 0; i < num_values; i++)
    {
        auto label = option.permissible_value_name(i);

        widget =
            gtk_radio_button_new_with_label_from_widget (widget ?
                    GTK_RADIO_BUTTON (widget) :
                    NULL,
                    label && *label ? _(label) : "");
        g_object_set_data (G_OBJECT (widget), "gnc_radiobutton_index",
                           GINT_TO_POINTER (i));
        g_signal_connect(G_OBJECT(widget), "toggled",
                         G_CALLBACK(radiobutton_set_cb), &option);
        gtk_box_pack_start (GTK_BOX (box), widget, FALSE, FALSE, 0);
    }

    return frame;
}

template<> GtkWidget *
create_option_widget<GncOptionUIType::RADIOBUTTON> (GncOption& option, GtkGrid *page_box,
                                      GtkLabel *name_label, char *documentation,
                                      /* Return values */
                                      GtkWidget **enclosing, bool *packed)
{
    *enclosing = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_set_homogeneous (GTK_BOX (*enclosing), FALSE);

    align_label (name_label);

    auto widget = create_radiobutton_widget(NULL, option);

    option.set_ui_item(std::make_unique<GncGtkPixmapUIItem>(widget));
    option.set_ui_item_from_option();

    gtk_box_pack_start(GTK_BOX(*enclosing), widget, FALSE, FALSE, 0);
    gtk_widget_show_all(*enclosing);
    return widget;
}

class GncGtkDateFormatUIItem : public GncOptionGtkUIItem
{
public:
    GncGtkDateFormatUIItem(GtkWidget* widget) :
        GncOptionGtkUIItem{widget, GncOptionUIType::DATE_FORMAT} {}
    void set_ui_item_from_option(GncOption& option) noexcept override
    {
        auto widget{GNC_DATE_FORMAT(get_widget())};
        gnc_date_format_set_custom(widget,
                                   option.get_value<std::string>().c_str());
    }
    void set_option_from_ui_item(GncOption& option) noexcept override
    {
        auto widget{GNC_DATE_FORMAT(get_widget())};
        option.set_value(std::string{gnc_date_format_get_custom(widget)});
    }
};


template<> GtkWidget *
create_option_widget<GncOptionUIType::DATE_FORMAT> (GncOption& option,
                                                    GtkGrid *page_box,
                                                    GtkLabel *name_label,
                                                    char *documentation,
                                                    /* Return values */
                                                    GtkWidget **enclosing,
                                                    bool *packed)
{
    *enclosing = gnc_date_format_new_without_label ();
    align_label (name_label);

    option.set_ui_item(std::make_unique<GncGtkDateFormatUIItem>(*enclosing));
    option.set_ui_item_from_option();

    g_signal_connect(G_OBJECT(*enclosing), "format_changed",
                     G_CALLBACK(gnc_option_changed_widget_cb), &option);
    gtk_widget_show_all(*enclosing);
    return *enclosing;
}

static void
gnc_plot_size_option_set_select_method(GncOption& option, bool set_buttons)
{
    GList* widget_list;
    GtkWidget *px_widget, *p_widget;
    GtkWidget *widget;

    widget = gnc_option_get_gtk_widget(&option);

    widget_list = gtk_container_get_children(GTK_CONTAINER(widget));
    // px_button item 0
    px_widget = static_cast<decltype(px_widget)>(g_list_nth_data(widget_list, 1));
    // p_button item 2
    p_widget = static_cast<decltype(p_widget)>(g_list_nth_data(widget_list, 3));
    g_list_free(widget_list);

    if (set_buttons)
    {
        gtk_widget_set_sensitive(px_widget, TRUE);
        gtk_widget_set_sensitive(p_widget, FALSE);
    }
    else
    {
        gtk_widget_set_sensitive(p_widget, TRUE);
        gtk_widget_set_sensitive(px_widget, FALSE);
    }
}

static void
gnc_rd_option_px_set_cb(GtkWidget *widget, GncOption* option)
{
    option->set_alternate(true);
    gnc_option_changed_option_cb(widget, option);
}

static void
gnc_rd_option_p_set_cb(GtkWidget *widget, GncOption* option)
{
    option->set_alternate(false);
    gnc_option_changed_option_cb(widget, option);
}

class GncGtkPlotSizeUIItem : public GncOptionGtkUIItem
{
public:
    GncGtkPlotSizeUIItem(GtkWidget* widget) :
        GncOptionGtkUIItem{widget, GncOptionUIType::PLOT_SIZE} {}
    void set_ui_item_from_option(GncOption& option) noexcept override
    {
        auto widgets{gtk_container_get_children(GTK_CONTAINER(get_widget()))};
        GtkWidget *button{}, *spin{};
        if (option.is_alternate())
        {
            button = GTK_WIDGET(g_list_nth_data(widgets, 2));
            spin = GTK_WIDGET(g_list_nth_data(widgets, 3));
        }
        else
        {
            button = GTK_WIDGET(g_list_nth_data(widgets, 2));
            spin = GTK_WIDGET(g_list_nth_data(widgets, 3));
        }
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin),
                                  option.get_value<double>());
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
    }
    void set_option_from_ui_item(GncOption& option) noexcept override
    {
        auto widgets{gtk_container_get_children(GTK_CONTAINER(get_widget()))};
        auto px_button{GTK_BUTTON(g_list_nth_data(widgets, 0))};
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(px_button)))
        {
            option.set_alternate(false);
            auto spin{g_list_nth_data(widgets, 1)};
            option.set_value(gtk_spin_button_get_value(GTK_SPIN_BUTTON(get_widget())));
        }
        else
        {
            option.set_alternate(true);
            auto spin{g_list_nth_data(widgets, 3)};
            option.set_value(gtk_spin_button_get_value(GTK_SPIN_BUTTON(get_widget())));
        }
    }
};


template<> GtkWidget *
create_option_widget<GncOptionUIType::PLOT_SIZE> (GncOption& option,
                                                  GtkGrid *page_box,
                                                  GtkLabel *name_label,
                                                  char *documentation,
                                                  /* Return values */
                                                  GtkWidget **enclosing,
                                                  bool *packed)
{
    GtkWidget *value_percent;
    GtkWidget *px_butt, *p_butt;
    GtkWidget *hbox;
    GtkAdjustment *adj_percent;

    *enclosing = gtk_frame_new(NULL);
    gtk_widget_set_halign (GTK_WIDGET(*enclosing), GTK_ALIGN_START);

    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_set_homogeneous (GTK_BOX (hbox), FALSE);
    g_object_set (G_OBJECT(hbox), "margin", 3, NULL);

    gtk_container_add(GTK_CONTAINER(*enclosing), hbox);
    auto value_px = create_range_spinner(option);

    g_signal_connect(G_OBJECT(value_px), "changed",
                     G_CALLBACK(gnc_option_changed_widget_cb), &option);

    adj_percent = GTK_ADJUSTMENT(gtk_adjustment_new(1, 10, 100, 1, 5.0, 0));
    value_percent = gtk_spin_button_new(adj_percent, 1, 0);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(value_percent), TRUE);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(value_percent), 100); //default
    gtk_entry_set_width_chars(GTK_ENTRY(value_percent), 3);
    gtk_widget_set_sensitive(value_percent, FALSE);

    g_signal_connect(G_OBJECT(value_percent), "changed",
                     G_CALLBACK(gnc_option_changed_widget_cb), &option);

    px_butt = gtk_radio_button_new_with_label(NULL, _("Pixels"));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(px_butt), TRUE);

    g_signal_connect(G_OBJECT(px_butt), "toggled",
                         G_CALLBACK(gnc_rd_option_px_set_cb), &option);

    p_butt = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(px_butt), _("Percent"));
    g_signal_connect(G_OBJECT(p_butt), "toggled",
                         G_CALLBACK(gnc_rd_option_p_set_cb), &option);

    gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(px_butt), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(value_px), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(p_butt), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(value_percent),
                       FALSE, FALSE, 0);

    option.set_ui_item(std::make_unique<GncGtkPlotSizeUIItem>(static_cast<GtkWidget*>(hbox)));
    option.set_ui_item_from_option();

    gtk_widget_show_all(*enclosing);
    return hbox;
}

static GtkWidget *
create_budget_widget(GncOption& option)
{
    GtkTreeModel *tm;
    GtkComboBox *cb;
    GtkCellRenderer *cr;

    tm = gnc_tree_model_budget_new(gnc_get_current_book());
    cb = GTK_COMBO_BOX(gtk_combo_box_new_with_model(tm));
    g_object_unref(tm);
    cr = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(cb), cr, TRUE);

    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(cb), cr, "text",
                                   BUDGET_NAME_COLUMN, NULL);
    return GTK_WIDGET(cb);
}

class GncGtkBudgetUIItem : public GncOptionGtkUIItem
{
public:
    GncGtkBudgetUIItem(GtkWidget* widget) :
        GncOptionGtkUIItem{widget, GncOptionUIType::BUDGET} {}
    void set_ui_item_from_option(GncOption& option) noexcept override
    {
        GtkTreeIter iter;
        auto widget{GTK_COMBO_BOX(get_widget())};
        auto instance{option.get_value<const QofInstance*>()};
        if (instance)
        {
            auto tree_model{gtk_combo_box_get_model(widget)};
            if (gnc_tree_model_budget_get_iter_for_budget(tree_model, &iter,
                                                          GNC_BUDGET(instance)))
                gtk_combo_box_set_active_iter(widget, &iter);
        }
    }
    void set_option_from_ui_item(GncOption& option) noexcept override
    {
        GtkTreeIter iter;
        auto widget{GTK_COMBO_BOX(get_widget())};
        gtk_combo_box_get_active_iter(widget, &iter);
        auto tree_model{gtk_combo_box_get_model(widget)};
        auto budget{gnc_tree_model_budget_get_budget(tree_model, &iter)};
        option.set_value(qof_instance_cast(budget));
    }
};

template<> GtkWidget *
create_option_widget<GncOptionUIType::BUDGET> (GncOption& option,
                                               GtkGrid *page_box,
                                               GtkLabel *name_label,
                                               char *documentation,
                                               /* Return values */
                                               GtkWidget **enclosing,
                                               bool *packed)
{
    *enclosing = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_set_homogeneous (GTK_BOX (*enclosing), FALSE);

    auto widget{create_budget_widget(option)};

    option.set_ui_item(std::make_unique<GncGtkBudgetUIItem>(widget));
    option.set_ui_item_from_option();

    /* Maybe connect destroy handler for tree model here? */
    g_signal_connect(G_OBJECT(widget), "changed",
                     G_CALLBACK(gnc_option_changed_widget_cb), &option);

    gtk_box_pack_start(GTK_BOX(*enclosing), widget, FALSE, FALSE, 0);
    gtk_widget_show_all(*enclosing);
    return widget;
}

void
gnc_options_ui_initialize(void)
{
    GncOptionUIFactory::set_func(GncOptionUIType::BOOLEAN,
                                 create_option_widget<GncOptionUIType::BOOLEAN>);
    GncOptionUIFactory::set_func(GncOptionUIType::STRING,
                                 create_option_widget<GncOptionUIType::STRING>);
    GncOptionUIFactory::set_func(GncOptionUIType::TEXT,
                                 create_option_widget<GncOptionUIType::TEXT>);
    GncOptionUIFactory::set_func(GncOptionUIType::CURRENCY,
                                 create_option_widget<GncOptionUIType::CURRENCY>);
    GncOptionUIFactory::set_func(GncOptionUIType::COMMODITY,
                                 create_option_widget<GncOptionUIType::COMMODITY>);
    GncOptionUIFactory::set_func(GncOptionUIType::MULTICHOICE,
                                 create_option_widget<GncOptionUIType::MULTICHOICE>);
    GncOptionUIFactory::set_func(GncOptionUIType::DATE_ABSOLUTE,
                                 create_option_widget<GncOptionUIType::DATE_ABSOLUTE>);
    GncOptionUIFactory::set_func(GncOptionUIType::DATE_RELATIVE,
                                 create_option_widget<GncOptionUIType::DATE_RELATIVE>);
    GncOptionUIFactory::set_func(GncOptionUIType::DATE_BOTH,
                                 create_option_widget<GncOptionUIType::DATE_BOTH>);
    GncOptionUIFactory::set_func(GncOptionUIType::ACCOUNT_LIST,
                                 create_option_widget<GncOptionUIType::ACCOUNT_LIST>);
    GncOptionUIFactory::set_func(GncOptionUIType::ACCOUNT_SEL,
                                 create_option_widget<GncOptionUIType::ACCOUNT_SEL>);
    GncOptionUIFactory::set_func(GncOptionUIType::LIST,
                                 create_option_widget<GncOptionUIType::LIST>);
    GncOptionUIFactory::set_func(GncOptionUIType::NUMBER_RANGE,
                                 create_option_widget<GncOptionUIType::NUMBER_RANGE>);
    GncOptionUIFactory::set_func(GncOptionUIType::COLOR,
                                 create_option_widget<GncOptionUIType::COLOR>);
    GncOptionUIFactory::set_func(GncOptionUIType::FONT,
                                 create_option_widget<GncOptionUIType::FONT>);
    GncOptionUIFactory::set_func(GncOptionUIType::PLOT_SIZE,
                                 create_option_widget<GncOptionUIType::PLOT_SIZE>);
    GncOptionUIFactory::set_func(GncOptionUIType::BUDGET,
                                 create_option_widget<GncOptionUIType::BUDGET>);
    GncOptionUIFactory::set_func(GncOptionUIType::PIXMAP,
                                 create_option_widget<GncOptionUIType::PIXMAP>);
    GncOptionUIFactory::set_func(GncOptionUIType::RADIOBUTTON,
                                 create_option_widget<GncOptionUIType::RADIOBUTTON>);
    GncOptionUIFactory::set_func(GncOptionUIType::DATE_FORMAT,
                                 create_option_widget<GncOptionUIType::DATE_FORMAT>);


}

void
gnc_options_dialog_set_new_book_option_values (GncOptionDB *odb)
{
    if (!odb) return;
    auto num_split_action = gnc_prefs_get_bool(GNC_PREFS_GROUP_GENERAL,
                                               GNC_PREF_NUM_SOURCE);
    if (num_split_action)
    {
        auto option{odb->find_option(OPTION_SECTION_ACCOUNTS,
                                    OPTION_NAME_NUM_FIELD_SOURCE)};
        auto num_source_button{gnc_option_get_gtk_widget(option)};
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (num_source_button),
                                      num_split_action);
    }
}


static void
gnc_book_options_help_cb (GNCOptionWin *win, gpointer dat)
{
    gnc_gnome_help (GTK_WINDOW (win), HF_HELP, HL_BOOK_OPTIONS);
}

void
gnc_options_dialog_set_book_options_help_cb (GNCOptionWin *win)
{
    gnc_options_dialog_set_help_cb(win,
                                (GNCOptionWinCallback)gnc_book_options_help_cb,
                                nullptr);
}
