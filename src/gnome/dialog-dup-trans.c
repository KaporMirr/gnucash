/********************************************************************\
 * dialog-dup-trans.c -- duplicate transaction dialog               *
 * Copyright (C) 2001 Gnumatic, Inc.                                *
 * Author: Dave Peticolas <dave@krondo.com>                         *
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
 * 59 Temple Place - Suite 330        Fax:    +1-617-542-2652       *
 * Boston, MA  02111-1307,  USA       gnu@gnu.org                   *
\********************************************************************/

#include "config.h"

#include <gnome.h>
#include <time.h>

#include "glade-gnc-dialogs.h"
#include "gnc-dateedit.h"
#include "gnc-engine-util.h"
#include "gnc-ui.h"
#include "messages.h"


/* This static indicates the debugging module that this .o belongs to.  */
/* static short module = MOD_GUI; */

typedef struct
{
  GtkWidget * dialog;

  GtkWidget * date_edit;
  GtkWidget * num_edit;
} DupTransDialog;


/* Parses the string value and returns true if it is a
 * number. In that case, *num is set to the value parsed.
 * Copied from numcell.c */
static gboolean
parse_num (const char *string, long int *num)
{
  long int number;

  if (string == NULL)
    return FALSE;

  if (!gnc_strisnum(string))
    return FALSE;

  number = strtol(string, NULL, 10);

  if ((number == LONG_MIN) || (number == LONG_MAX))
    return FALSE;

  if (num != NULL)
    *num = number;

  return TRUE;
}

static void
gnc_dup_trans_dialog_create (GtkWidget * parent, DupTransDialog *dt_dialog,
                             time_t date, const char *num_str)
{
  GtkWidget *dialog;
  GtkObject *dtdo;

  dialog = create_Duplicate_Transaction_Dialog ();
  dt_dialog->dialog = dialog;
  dtdo = GTK_OBJECT (dialog);

  /* parent */
  if (parent != NULL)
    gnome_dialog_set_parent (GNOME_DIALOG (dialog), GTK_WINDOW (parent));

  /* default to ok */
  gnome_dialog_set_default (GNOME_DIALOG(dialog), 0);

  /* date widget */
  {
    GtkWidget *date_edit;
    GtkWidget *hbox;

    date_edit = gnc_date_edit_new (date, FALSE, FALSE);
    hbox = gtk_object_get_data (dtdo, "date_hbox");
    gtk_widget_show (date_edit);

    gtk_box_pack_end (GTK_BOX (hbox), date_edit, TRUE, TRUE, 0);
    dt_dialog->date_edit = date_edit;
  }

  {
    GtkWidget *num_spin;
    long int num;

    num_spin = gtk_object_get_data (dtdo, "num_spin");
    dt_dialog->num_edit = num_spin;

    gnome_dialog_editable_enters (GNOME_DIALOG (dialog),
                                  GTK_EDITABLE (num_spin));

    if (num_str && parse_num (num_str, &num))
      gtk_spin_button_set_value (GTK_SPIN_BUTTON (num_spin), num + 1);
    else
      gtk_entry_set_text (GTK_ENTRY (num_spin), "");
  }
}

/********************************************************************\
 * gnc_dup_trans_dialog                                             *
 *   opens up a window to do an automatic transfer between accounts *
 *                                                                  * 
 * Args:   parent  - the parent of the window to be created         *
 *         date    - the initial date to use, and the output        *
 *                   parameter for the new date                     *
 *         num     - input num field                                *
 *         out_num - output num field, g_newed string               *
 * Return: TRUE if user closes dialog with 'OK'                     *
\********************************************************************/
gboolean
gnc_dup_trans_dialog (GtkWidget * parent, time_t *date_p,
                      const char *num, char **out_num)
{
  DupTransDialog *dt_dialog;
  GNCDateEdit *gde;
  GtkWidget *entry;
  gboolean ok;
  gint result;

  if (!date_p || !out_num)
    return FALSE;

  dt_dialog = g_new0 (DupTransDialog, 1);

  gnc_dup_trans_dialog_create (parent, dt_dialog, *date_p, num);

  gde = GNC_DATE_EDIT (dt_dialog->date_edit);
  entry = gde->date_entry;

  gtk_widget_grab_focus (entry);

  result = gnome_dialog_run_and_close (GNOME_DIALOG (dt_dialog->dialog));

  if (result == 0)
  {
    *date_p = gnc_date_edit_get_date (GNC_DATE_EDIT (dt_dialog->date_edit));
    *out_num = g_strdup (gtk_entry_get_text (GTK_ENTRY (dt_dialog->num_edit)));
    ok = TRUE;
  }
  else
    ok = FALSE;

  g_free (dt_dialog);

  return ok;
}