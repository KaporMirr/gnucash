/*
 * DO NOT EDIT THIS FILE - it is generated by Glade.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include <gnome.h>

#include "new-user-callbacks.h"
#include "new-user-interface.h"
#include "glade-support.h"

GtkWidget*
create_newUserDialog (void)
{
  GtkWidget *newUserDialog;
  GtkWidget *newUserDruid;
  GtkWidget *newUserStartPage;
  GdkColor newUserStartPage_bg_color = { 0, 6425, 6425, 28784 };
  GdkColor newUserStartPage_textbox_color = { 0, 65535, 65535, 65535 };
  GdkColor newUserStartPage_logo_bg_color = { 0, 65535, 65535, 65535 };
  GdkColor newUserStartPage_title_color = { 0, 65535, 65535, 65535 };
  GtkWidget *newAccountCurrencyChoosePage;
  GdkColor newAccountCurrencyChoosePage_bg_color = { 0, 6425, 6425, 28784 };
  GdkColor newAccountCurrencyChoosePage_logo_bg_color = { 0, 65535, 65535, 65535 };
  GdkColor newAccountCurrencyChoosePage_title_color = { 0, 65535, 65535, 65535 };
  GtkWidget *newAccountCurrencyChooser_vbox2;
  GtkWidget *newUserChooseCurrencyDescrip;
  GtkWidget *newAccountCurrencyChooser_vbox;
  GtkWidget *chooseAccountTypesPage;
  GdkColor chooseAccountTypesPage_bg_color = { 0, 6425, 6425, 28784 };
  GdkColor chooseAccountTypesPage_logo_bg_color = { 0, 65535, 65535, 65535 };
  GdkColor chooseAccountTypesPage_title_color = { 0, 65535, 65535, 65535 };
  GtkWidget *druid_vbox1;
  GtkWidget *pickAccountsDescriptionLabel;
  GtkWidget *frame5;
  GtkWidget *vbox7;
  GtkWidget *scrolledwindow1;
  GtkWidget *newAccountTypesList;
  GtkWidget *newAccountTypesList_TypeLabel;
  GtkWidget *newAccountTypesList_DescriptionLabel;
  GtkWidget *hbox3;
  GtkWidget *newAccountsTypeList_SelectAllButton;
  GtkWidget *newAccountsTypeList_ClearAllButton;
  GtkWidget *hbox1;
  GtkWidget *frame1;
  GtkWidget *scrolledwindow2;
  GtkWidget *viewport2;
  GtkWidget *newAccountTypesDescription;
  GtkWidget *frame2;
  GtkWidget *scrolledwindow3;
  GtkWidget *viewport1;
  GtkWidget *newAccountListTree;
  GtkWidget *finalAccountDruidPage;
  GdkColor finalAccountDruidPage_bg_color = { 0, 6425, 6425, 28784 };
  GdkColor finalAccountDruidPage_logo_bg_color = { 0, 65535, 65535, 65535 };
  GdkColor finalAccountDruidPage_title_color = { 0, 65535, 65535, 65535 };
  GtkWidget *druid_vbox3;
  GtkWidget *finalAccountLabel;
  GtkWidget *hbox4;
  GtkWidget *scrolledwindow4;
  GtkWidget *finalAccountCTree;
  GtkWidget *cTreeAccountNameLabel;
  GtkWidget *cTreeTypeLabel;
  GtkWidget *cTreeOpeningBalanceLabel;
  GtkWidget *vbox5;
  GtkWidget *placeholder;
  GtkWidget *frame3;
  GtkWidget *startBalanceBox;
  GtkWidget *newUserDruidFinishPage;
  GdkColor newUserDruidFinishPage_bg_color = { 0, 6425, 6425, 28784 };
  GdkColor newUserDruidFinishPage_textbox_color = { 0, 65535, 65535, 65535 };
  GdkColor newUserDruidFinishPage_logo_bg_color = { 0, 65535, 65535, 65535 };
  GdkColor newUserDruidFinishPage_title_color = { 0, 65535, 65535, 65535 };

  newUserDialog = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name (newUserDialog, "newUserDialog");
  gtk_object_set_data (GTK_OBJECT (newUserDialog), "newUserDialog", newUserDialog);
  gtk_widget_set_usize (newUserDialog, 540, 370);
  gtk_window_set_title (GTK_WINDOW (newUserDialog), _("New Account Hierarchy Setup"));
  gtk_window_set_position (GTK_WINDOW (newUserDialog), GTK_WIN_POS_MOUSE);
  gtk_window_set_default_size (GTK_WINDOW (newUserDialog), 640, 480);

  newUserDruid = gnome_druid_new ();
  gtk_widget_set_name (newUserDruid, "newUserDruid");
  gtk_widget_ref (newUserDruid);
  gtk_object_set_data_full (GTK_OBJECT (newUserDialog), "newUserDruid", newUserDruid,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (newUserDruid);
  gtk_container_add (GTK_CONTAINER (newUserDialog), newUserDruid);

  newUserStartPage = gnome_druid_page_start_new ();
  gtk_widget_set_name (newUserStartPage, "newUserStartPage");
  gtk_widget_ref (newUserStartPage);
  gtk_object_set_data_full (GTK_OBJECT (newUserDialog), "newUserStartPage", newUserStartPage,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (newUserStartPage);
  gnome_druid_append_page (GNOME_DRUID (newUserDruid), GNOME_DRUID_PAGE (newUserStartPage));
  gnome_druid_set_page (GNOME_DRUID (newUserDruid), GNOME_DRUID_PAGE (newUserStartPage));
  gnome_druid_page_start_set_bg_color (GNOME_DRUID_PAGE_START (newUserStartPage), &newUserStartPage_bg_color);
  gnome_druid_page_start_set_textbox_color (GNOME_DRUID_PAGE_START (newUserStartPage), &newUserStartPage_textbox_color);
  gnome_druid_page_start_set_logo_bg_color (GNOME_DRUID_PAGE_START (newUserStartPage), &newUserStartPage_logo_bg_color);
  gnome_druid_page_start_set_title_color (GNOME_DRUID_PAGE_START (newUserStartPage), &newUserStartPage_title_color);
  gnome_druid_page_start_set_title (GNOME_DRUID_PAGE_START (newUserStartPage), _("New Account Hierarchy Setup"));
  gnome_druid_page_start_set_text (GNOME_DRUID_PAGE_START (newUserStartPage), _("This druid will help you create a set of GnuCash \naccounts for your assets (such as investments, \nchecking or savings accounts), liabilities (such \nas loans) and different kinds of income and \nexpenses you might have. \n\nClick 'Cancel'  if you do not wish to create any \nnew accounts now."));

  newAccountCurrencyChoosePage = gnome_druid_page_standard_new_with_vals ("", NULL);
  gtk_widget_set_name (newAccountCurrencyChoosePage, "newAccountCurrencyChoosePage");
  gtk_widget_ref (newAccountCurrencyChoosePage);
  gtk_object_set_data_full (GTK_OBJECT (newUserDialog), "newAccountCurrencyChoosePage", newAccountCurrencyChoosePage,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show_all (newAccountCurrencyChoosePage);
  gnome_druid_append_page (GNOME_DRUID (newUserDruid), GNOME_DRUID_PAGE (newAccountCurrencyChoosePage));
  gnome_druid_page_standard_set_bg_color (GNOME_DRUID_PAGE_STANDARD (newAccountCurrencyChoosePage), &newAccountCurrencyChoosePage_bg_color);
  gnome_druid_page_standard_set_logo_bg_color (GNOME_DRUID_PAGE_STANDARD (newAccountCurrencyChoosePage), &newAccountCurrencyChoosePage_logo_bg_color);
  gnome_druid_page_standard_set_title_color (GNOME_DRUID_PAGE_STANDARD (newAccountCurrencyChoosePage), &newAccountCurrencyChoosePage_title_color);
  gnome_druid_page_standard_set_title (GNOME_DRUID_PAGE_STANDARD (newAccountCurrencyChoosePage), _("Choose Currency"));

  newAccountCurrencyChooser_vbox2 = GNOME_DRUID_PAGE_STANDARD (newAccountCurrencyChoosePage)->vbox;
  gtk_widget_set_name (newAccountCurrencyChooser_vbox2, "newAccountCurrencyChooser_vbox2");
  gtk_widget_ref (newAccountCurrencyChooser_vbox2);
  gtk_object_set_data_full (GTK_OBJECT (newUserDialog), "newAccountCurrencyChooser_vbox2", newAccountCurrencyChooser_vbox2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (newAccountCurrencyChooser_vbox2);
  gtk_container_set_border_width (GTK_CONTAINER (newAccountCurrencyChooser_vbox2), 20);

  newUserChooseCurrencyDescrip = gtk_label_new (_("Please choose the currency to use for new accounts."));
  gtk_widget_set_name (newUserChooseCurrencyDescrip, "newUserChooseCurrencyDescrip");
  gtk_widget_ref (newUserChooseCurrencyDescrip);
  gtk_object_set_data_full (GTK_OBJECT (newUserDialog), "newUserChooseCurrencyDescrip", newUserChooseCurrencyDescrip,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (newUserChooseCurrencyDescrip);
  gtk_box_pack_start (GTK_BOX (newAccountCurrencyChooser_vbox2), newUserChooseCurrencyDescrip, FALSE, FALSE, 0);

  newAccountCurrencyChooser_vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_set_name (newAccountCurrencyChooser_vbox, "newAccountCurrencyChooser_vbox");
  gtk_widget_ref (newAccountCurrencyChooser_vbox);
  gtk_object_set_data_full (GTK_OBJECT (newUserDialog), "newAccountCurrencyChooser_vbox", newAccountCurrencyChooser_vbox,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (newAccountCurrencyChooser_vbox);
  gtk_box_pack_start (GTK_BOX (newAccountCurrencyChooser_vbox2), newAccountCurrencyChooser_vbox, FALSE, FALSE, 0);

  chooseAccountTypesPage = gnome_druid_page_standard_new_with_vals ("", NULL);
  gtk_widget_set_name (chooseAccountTypesPage, "chooseAccountTypesPage");
  gtk_widget_ref (chooseAccountTypesPage);
  gtk_object_set_data_full (GTK_OBJECT (newUserDialog), "chooseAccountTypesPage", chooseAccountTypesPage,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show_all (chooseAccountTypesPage);
  gnome_druid_append_page (GNOME_DRUID (newUserDruid), GNOME_DRUID_PAGE (chooseAccountTypesPage));
  gnome_druid_page_standard_set_bg_color (GNOME_DRUID_PAGE_STANDARD (chooseAccountTypesPage), &chooseAccountTypesPage_bg_color);
  gnome_druid_page_standard_set_logo_bg_color (GNOME_DRUID_PAGE_STANDARD (chooseAccountTypesPage), &chooseAccountTypesPage_logo_bg_color);
  gnome_druid_page_standard_set_title_color (GNOME_DRUID_PAGE_STANDARD (chooseAccountTypesPage), &chooseAccountTypesPage_title_color);
  gnome_druid_page_standard_set_title (GNOME_DRUID_PAGE_STANDARD (chooseAccountTypesPage), _("Choose accounts to create"));

  druid_vbox1 = GNOME_DRUID_PAGE_STANDARD (chooseAccountTypesPage)->vbox;
  gtk_widget_set_name (druid_vbox1, "druid_vbox1");
  gtk_widget_ref (druid_vbox1);
  gtk_object_set_data_full (GTK_OBJECT (newUserDialog), "druid_vbox1", druid_vbox1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (druid_vbox1);
  gtk_container_set_border_width (GTK_CONTAINER (druid_vbox1), 5);

  pickAccountsDescriptionLabel = gtk_label_new (_("Select categories that correspond to the ways that you will use GnuCash.  \nEach category you select will cause several accounts to be created.  \nSelect the categories that are relevant to you.  You can always create \nadditional accounts by hand later."));
  gtk_widget_set_name (pickAccountsDescriptionLabel, "pickAccountsDescriptionLabel");
  gtk_widget_ref (pickAccountsDescriptionLabel);
  gtk_object_set_data_full (GTK_OBJECT (newUserDialog), "pickAccountsDescriptionLabel", pickAccountsDescriptionLabel,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (pickAccountsDescriptionLabel);
  gtk_box_pack_start (GTK_BOX (druid_vbox1), pickAccountsDescriptionLabel, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (pickAccountsDescriptionLabel), GTK_JUSTIFY_LEFT);

  frame5 = gtk_frame_new (NULL);
  gtk_widget_set_name (frame5, "frame5");
  gtk_widget_ref (frame5);
  gtk_object_set_data_full (GTK_OBJECT (newUserDialog), "frame5", frame5,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame5);
  gtk_box_pack_start (GTK_BOX (druid_vbox1), frame5, TRUE, TRUE, 0);

  vbox7 = gtk_vbox_new (FALSE, 0);
  gtk_widget_set_name (vbox7, "vbox7");
  gtk_widget_ref (vbox7);
  gtk_object_set_data_full (GTK_OBJECT (newUserDialog), "vbox7", vbox7,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox7);
  gtk_container_add (GTK_CONTAINER (frame5), vbox7);

  scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrolledwindow1, "scrolledwindow1");
  gtk_widget_ref (scrolledwindow1);
  gtk_object_set_data_full (GTK_OBJECT (newUserDialog), "scrolledwindow1", scrolledwindow1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrolledwindow1);
  gtk_box_pack_start (GTK_BOX (vbox7), scrolledwindow1, TRUE, TRUE, 0);

  newAccountTypesList = gtk_clist_new (2);
  gtk_widget_set_name (newAccountTypesList, "newAccountTypesList");
  gtk_widget_ref (newAccountTypesList);
  gtk_object_set_data_full (GTK_OBJECT (newUserDialog), "newAccountTypesList", newAccountTypesList,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (newAccountTypesList);
  gtk_container_add (GTK_CONTAINER (scrolledwindow1), newAccountTypesList);
  gtk_clist_set_column_width (GTK_CLIST (newAccountTypesList), 0, 144);
  gtk_clist_set_column_width (GTK_CLIST (newAccountTypesList), 1, 80);
  gtk_clist_set_selection_mode (GTK_CLIST (newAccountTypesList), GTK_SELECTION_MULTIPLE);
  gtk_clist_column_titles_show (GTK_CLIST (newAccountTypesList));

  newAccountTypesList_TypeLabel = gtk_label_new (_("Account Types"));
  gtk_widget_set_name (newAccountTypesList_TypeLabel, "newAccountTypesList_TypeLabel");
  gtk_widget_ref (newAccountTypesList_TypeLabel);
  gtk_object_set_data_full (GTK_OBJECT (newUserDialog), "newAccountTypesList_TypeLabel", newAccountTypesList_TypeLabel,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (newAccountTypesList_TypeLabel);
  gtk_clist_set_column_widget (GTK_CLIST (newAccountTypesList), 0, newAccountTypesList_TypeLabel);

  newAccountTypesList_DescriptionLabel = gtk_label_new (_("Description"));
  gtk_widget_set_name (newAccountTypesList_DescriptionLabel, "newAccountTypesList_DescriptionLabel");
  gtk_widget_ref (newAccountTypesList_DescriptionLabel);
  gtk_object_set_data_full (GTK_OBJECT (newUserDialog), "newAccountTypesList_DescriptionLabel", newAccountTypesList_DescriptionLabel,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (newAccountTypesList_DescriptionLabel);
  gtk_clist_set_column_widget (GTK_CLIST (newAccountTypesList), 1, newAccountTypesList_DescriptionLabel);

  hbox3 = gtk_hbox_new (FALSE, 0);
  gtk_widget_set_name (hbox3, "hbox3");
  gtk_widget_ref (hbox3);
  gtk_object_set_data_full (GTK_OBJECT (newUserDialog), "hbox3", hbox3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox3);
  gtk_box_pack_start (GTK_BOX (vbox7), hbox3, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox3), 5);

  newAccountsTypeList_SelectAllButton = gtk_button_new_with_label (_("Select All"));
  gtk_widget_set_name (newAccountsTypeList_SelectAllButton, "newAccountsTypeList_SelectAllButton");
  gtk_widget_ref (newAccountsTypeList_SelectAllButton);
  gtk_object_set_data_full (GTK_OBJECT (newUserDialog), "newAccountsTypeList_SelectAllButton", newAccountsTypeList_SelectAllButton,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (newAccountsTypeList_SelectAllButton);
  gtk_box_pack_start (GTK_BOX (hbox3), newAccountsTypeList_SelectAllButton, TRUE, FALSE, 0);

  newAccountsTypeList_ClearAllButton = gtk_button_new_with_label (_("Clear All"));
  gtk_widget_set_name (newAccountsTypeList_ClearAllButton, "newAccountsTypeList_ClearAllButton");
  gtk_widget_ref (newAccountsTypeList_ClearAllButton);
  gtk_object_set_data_full (GTK_OBJECT (newUserDialog), "newAccountsTypeList_ClearAllButton", newAccountsTypeList_ClearAllButton,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (newAccountsTypeList_ClearAllButton);
  gtk_box_pack_start (GTK_BOX (hbox3), newAccountsTypeList_ClearAllButton, TRUE, FALSE, 0);

  hbox1 = gtk_hbox_new (FALSE, 2);
  gtk_widget_set_name (hbox1, "hbox1");
  gtk_widget_ref (hbox1);
  gtk_object_set_data_full (GTK_OBJECT (newUserDialog), "hbox1", hbox1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (druid_vbox1), hbox1, TRUE, TRUE, 0);

  frame1 = gtk_frame_new (_("Detailed Description"));
  gtk_widget_set_name (frame1, "frame1");
  gtk_widget_ref (frame1);
  gtk_object_set_data_full (GTK_OBJECT (newUserDialog), "frame1", frame1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame1);
  gtk_box_pack_start (GTK_BOX (hbox1), frame1, TRUE, TRUE, 0);

  scrolledwindow2 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrolledwindow2, "scrolledwindow2");
  gtk_widget_ref (scrolledwindow2);
  gtk_object_set_data_full (GTK_OBJECT (newUserDialog), "scrolledwindow2", scrolledwindow2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrolledwindow2);
  gtk_container_add (GTK_CONTAINER (frame1), scrolledwindow2);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow2), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  viewport2 = gtk_viewport_new (NULL, NULL);
  gtk_widget_set_name (viewport2, "viewport2");
  gtk_widget_ref (viewport2);
  gtk_object_set_data_full (GTK_OBJECT (newUserDialog), "viewport2", viewport2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (viewport2);
  gtk_container_add (GTK_CONTAINER (scrolledwindow2), viewport2);

  newAccountTypesDescription = gtk_label_new ("");
  gtk_widget_set_name (newAccountTypesDescription, "newAccountTypesDescription");
  gtk_widget_ref (newAccountTypesDescription);
  gtk_object_set_data_full (GTK_OBJECT (newUserDialog), "newAccountTypesDescription", newAccountTypesDescription,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (newAccountTypesDescription);
  gtk_container_add (GTK_CONTAINER (viewport2), newAccountTypesDescription);
  gtk_label_set_justify (GTK_LABEL (newAccountTypesDescription), GTK_JUSTIFY_LEFT);
  gtk_label_set_line_wrap (GTK_LABEL (newAccountTypesDescription), TRUE);

  frame2 = gtk_frame_new (_("Accounts"));
  gtk_widget_set_name (frame2, "frame2");
  gtk_widget_ref (frame2);
  gtk_object_set_data_full (GTK_OBJECT (newUserDialog), "frame2", frame2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame2);
  gtk_box_pack_start (GTK_BOX (hbox1), frame2, TRUE, TRUE, 0);

  scrolledwindow3 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrolledwindow3, "scrolledwindow3");
  gtk_widget_ref (scrolledwindow3);
  gtk_object_set_data_full (GTK_OBJECT (newUserDialog), "scrolledwindow3", scrolledwindow3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrolledwindow3);
  gtk_container_add (GTK_CONTAINER (frame2), scrolledwindow3);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow3), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  viewport1 = gtk_viewport_new (NULL, NULL);
  gtk_widget_set_name (viewport1, "viewport1");
  gtk_widget_ref (viewport1);
  gtk_object_set_data_full (GTK_OBJECT (newUserDialog), "viewport1", viewport1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (viewport1);
  gtk_container_add (GTK_CONTAINER (scrolledwindow3), viewport1);

  newAccountListTree = gtk_tree_new ();
  gtk_widget_set_name (newAccountListTree, "newAccountListTree");
  gtk_widget_ref (newAccountListTree);
  gtk_object_set_data_full (GTK_OBJECT (newUserDialog), "newAccountListTree", newAccountListTree,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (newAccountListTree);
  gtk_container_add (GTK_CONTAINER (viewport1), newAccountListTree);

  finalAccountDruidPage = gnome_druid_page_standard_new_with_vals ("", NULL);
  gtk_widget_set_name (finalAccountDruidPage, "finalAccountDruidPage");
  gtk_widget_ref (finalAccountDruidPage);
  gtk_object_set_data_full (GTK_OBJECT (newUserDialog), "finalAccountDruidPage", finalAccountDruidPage,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show_all (finalAccountDruidPage);
  gnome_druid_append_page (GNOME_DRUID (newUserDruid), GNOME_DRUID_PAGE (finalAccountDruidPage));
  gnome_druid_page_standard_set_bg_color (GNOME_DRUID_PAGE_STANDARD (finalAccountDruidPage), &finalAccountDruidPage_bg_color);
  gnome_druid_page_standard_set_logo_bg_color (GNOME_DRUID_PAGE_STANDARD (finalAccountDruidPage), &finalAccountDruidPage_logo_bg_color);
  gnome_druid_page_standard_set_title_color (GNOME_DRUID_PAGE_STANDARD (finalAccountDruidPage), &finalAccountDruidPage_title_color);
  gnome_druid_page_standard_set_title (GNOME_DRUID_PAGE_STANDARD (finalAccountDruidPage), _("Enter opening balances"));

  druid_vbox3 = GNOME_DRUID_PAGE_STANDARD (finalAccountDruidPage)->vbox;
  gtk_widget_set_name (druid_vbox3, "druid_vbox3");
  gtk_widget_ref (druid_vbox3);
  gtk_object_set_data_full (GTK_OBJECT (newUserDialog), "druid_vbox3", druid_vbox3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (druid_vbox3);
  gtk_container_set_border_width (GTK_CONTAINER (druid_vbox3), 5);

  finalAccountLabel = gtk_label_new (_("If you would like an account to have an opening balance, click on the account\nand enter the starting balance in the box on the right. All accounts except Equity\naccounts may have an opening balance."));
  gtk_widget_set_name (finalAccountLabel, "finalAccountLabel");
  gtk_widget_ref (finalAccountLabel);
  gtk_object_set_data_full (GTK_OBJECT (newUserDialog), "finalAccountLabel", finalAccountLabel,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (finalAccountLabel);
  gtk_box_pack_start (GTK_BOX (druid_vbox3), finalAccountLabel, FALSE, FALSE, 5);
  gtk_label_set_justify (GTK_LABEL (finalAccountLabel), GTK_JUSTIFY_LEFT);
  gtk_misc_set_padding (GTK_MISC (finalAccountLabel), 1, 1);

  hbox4 = gtk_hbox_new (FALSE, 2);
  gtk_widget_set_name (hbox4, "hbox4");
  gtk_widget_ref (hbox4);
  gtk_object_set_data_full (GTK_OBJECT (newUserDialog), "hbox4", hbox4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox4);
  gtk_box_pack_start (GTK_BOX (druid_vbox3), hbox4, TRUE, TRUE, 0);

  scrolledwindow4 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrolledwindow4, "scrolledwindow4");
  gtk_widget_ref (scrolledwindow4);
  gtk_object_set_data_full (GTK_OBJECT (newUserDialog), "scrolledwindow4", scrolledwindow4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrolledwindow4);
  gtk_box_pack_start (GTK_BOX (hbox4), scrolledwindow4, TRUE, TRUE, 0);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow4), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  finalAccountCTree = gtk_ctree_new (3, 0);
  gtk_widget_set_name (finalAccountCTree, "finalAccountCTree");
  gtk_widget_ref (finalAccountCTree);
  gtk_object_set_data_full (GTK_OBJECT (newUserDialog), "finalAccountCTree", finalAccountCTree,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (finalAccountCTree);
  gtk_container_add (GTK_CONTAINER (scrolledwindow4), finalAccountCTree);
  gtk_clist_set_column_width (GTK_CLIST (finalAccountCTree), 0, 80);
  gtk_clist_set_column_width (GTK_CLIST (finalAccountCTree), 1, 80);
  gtk_clist_set_column_width (GTK_CLIST (finalAccountCTree), 2, 80);
  gtk_clist_column_titles_show (GTK_CLIST (finalAccountCTree));

  cTreeAccountNameLabel = gtk_label_new (_("Account Name"));
  gtk_widget_set_name (cTreeAccountNameLabel, "cTreeAccountNameLabel");
  gtk_widget_ref (cTreeAccountNameLabel);
  gtk_object_set_data_full (GTK_OBJECT (newUserDialog), "cTreeAccountNameLabel", cTreeAccountNameLabel,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (cTreeAccountNameLabel);
  gtk_clist_set_column_widget (GTK_CLIST (finalAccountCTree), 0, cTreeAccountNameLabel);

  cTreeTypeLabel = gtk_label_new (_("Type"));
  gtk_widget_set_name (cTreeTypeLabel, "cTreeTypeLabel");
  gtk_widget_ref (cTreeTypeLabel);
  gtk_object_set_data_full (GTK_OBJECT (newUserDialog), "cTreeTypeLabel", cTreeTypeLabel,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (cTreeTypeLabel);
  gtk_clist_set_column_widget (GTK_CLIST (finalAccountCTree), 1, cTreeTypeLabel);

  cTreeOpeningBalanceLabel = gtk_label_new (_("Opening Balance"));
  gtk_widget_set_name (cTreeOpeningBalanceLabel, "cTreeOpeningBalanceLabel");
  gtk_widget_ref (cTreeOpeningBalanceLabel);
  gtk_object_set_data_full (GTK_OBJECT (newUserDialog), "cTreeOpeningBalanceLabel", cTreeOpeningBalanceLabel,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (cTreeOpeningBalanceLabel);
  gtk_clist_set_column_widget (GTK_CLIST (finalAccountCTree), 2, cTreeOpeningBalanceLabel);

  vbox5 = gtk_vbox_new (FALSE, 0);
  gtk_widget_set_name (vbox5, "vbox5");
  gtk_widget_ref (vbox5);
  gtk_object_set_data_full (GTK_OBJECT (newUserDialog), "vbox5", vbox5,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox5);
  gtk_box_pack_start (GTK_BOX (hbox4), vbox5, FALSE, TRUE, 0);

  placeholder = gtk_check_button_new_with_label (_("Placeholder"));
  gtk_widget_set_name (placeholder, "placeholder");
  gtk_widget_ref (placeholder);
  gtk_object_set_data_full (GTK_OBJECT (newUserDialog), "placeholder", placeholder,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (placeholder);
  gtk_box_pack_start (GTK_BOX (vbox5), placeholder, FALSE, FALSE, 2);

  frame3 = gtk_frame_new (_("Opening Balance"));
  gtk_widget_set_name (frame3, "frame3");
  gtk_widget_ref (frame3);
  gtk_object_set_data_full (GTK_OBJECT (newUserDialog), "frame3", frame3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame3);
  gtk_box_pack_start (GTK_BOX (vbox5), frame3, FALSE, FALSE, 0);

  startBalanceBox = gtk_vbox_new (FALSE, 0);
  gtk_widget_set_name (startBalanceBox, "startBalanceBox");
  gtk_widget_ref (startBalanceBox);
  gtk_object_set_data_full (GTK_OBJECT (newUserDialog), "startBalanceBox", startBalanceBox,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (startBalanceBox);
  gtk_container_add (GTK_CONTAINER (frame3), startBalanceBox);
  gtk_container_set_border_width (GTK_CONTAINER (startBalanceBox), 3);

  newUserDruidFinishPage = gnome_druid_page_finish_new ();
  gtk_widget_set_name (newUserDruidFinishPage, "newUserDruidFinishPage");
  gtk_widget_ref (newUserDruidFinishPage);
  gtk_object_set_data_full (GTK_OBJECT (newUserDialog), "newUserDruidFinishPage", newUserDruidFinishPage,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (newUserDruidFinishPage);
  gnome_druid_append_page (GNOME_DRUID (newUserDruid), GNOME_DRUID_PAGE (newUserDruidFinishPage));
  gnome_druid_page_finish_set_bg_color (GNOME_DRUID_PAGE_FINISH (newUserDruidFinishPage), &newUserDruidFinishPage_bg_color);
  gnome_druid_page_finish_set_textbox_color (GNOME_DRUID_PAGE_FINISH (newUserDruidFinishPage), &newUserDruidFinishPage_textbox_color);
  gnome_druid_page_finish_set_logo_bg_color (GNOME_DRUID_PAGE_FINISH (newUserDruidFinishPage), &newUserDruidFinishPage_logo_bg_color);
  gnome_druid_page_finish_set_title_color (GNOME_DRUID_PAGE_FINISH (newUserDruidFinishPage), &newUserDruidFinishPage_title_color);
  gnome_druid_page_finish_set_title (GNOME_DRUID_PAGE_FINISH (newUserDruidFinishPage), _("Finish Account Setup"));
  gnome_druid_page_finish_set_text (GNOME_DRUID_PAGE_FINISH (newUserDruidFinishPage), _("Press `Finish' to create your new accounts.\n\nPress `Back' to review your selections.\n\nPress `Cancel' to close this dialog without creating any new accounts."));

  gtk_signal_connect (GTK_OBJECT (newUserDruid), "cancel",
                      GTK_SIGNAL_FUNC (on_accountChooseDruidPage_cancel),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (newUserStartPage), "next",
                      GTK_SIGNAL_FUNC (on_newUserStartPage_next),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (newAccountCurrencyChoosePage), "prepare",
                      GTK_SIGNAL_FUNC (on_newAccountCurrencyChoosePage_prepare),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (newAccountCurrencyChoosePage), "next",
                      GTK_SIGNAL_FUNC (on_newAccountCurrencyChoosePage_next),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (chooseAccountTypesPage), "next",
                      GTK_SIGNAL_FUNC (on_chooseAccountTypesPage_next),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (chooseAccountTypesPage), "prepare",
                      GTK_SIGNAL_FUNC (on_chooseAccountTypesPage_prepare),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (newAccountTypesList), "select_row",
                      GTK_SIGNAL_FUNC (on_newAccountTypesList_select_row),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (newAccountTypesList), "unselect_row",
                      GTK_SIGNAL_FUNC (on_newAccountTypesList_unselect_row),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (newAccountsTypeList_SelectAllButton), "clicked",
                      GTK_SIGNAL_FUNC (on_newAccountsTypeList_SelectAllButton_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (newAccountsTypeList_ClearAllButton), "clicked",
                      GTK_SIGNAL_FUNC (on_newAccountsTypeList_ClearAllButton_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (finalAccountDruidPage), "prepare",
                      GTK_SIGNAL_FUNC (on_finalAccountDruidPage_prepare),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (finalAccountDruidPage), "next",
                      GTK_SIGNAL_FUNC (on_finalAccountDruidPage_next),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (finalAccountCTree), "tree_select_row",
                      GTK_SIGNAL_FUNC (on_finalAccountCTree_tree_select_row),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (finalAccountCTree), "tree_unselect_row",
                      GTK_SIGNAL_FUNC (on_finalAccountCTree_tree_unselect_row),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (placeholder), "toggled",
                      GTK_SIGNAL_FUNC (on_finalAccountCTree_tree_placeholder_toggled),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (newUserDruidFinishPage), "finish",
                      GTK_SIGNAL_FUNC (on_newUserDruidFinishPage_finish),
                      NULL);

  return newUserDialog;
}

GtkWidget*
create_addAccountCancelDialog (void)
{
  GtkWidget *addAccountCancelDialog;
  GtkWidget *newAccountCancelDialog;
  GtkWidget *vbox1;
  GtkWidget *newAccountCancelDialog_ShouldRunAgainLable;
  GtkWidget *newAccountCancelDialog_RunAgainToggle;
  GtkWidget *dialog_action_area1;
  GtkWidget *newAccountCancelDialog_OKButton;

  addAccountCancelDialog = gnome_dialog_new (_("Canceling"), NULL);
  gtk_widget_set_name (addAccountCancelDialog, "addAccountCancelDialog");
  gtk_object_set_data (GTK_OBJECT (addAccountCancelDialog), "addAccountCancelDialog", addAccountCancelDialog);
  GTK_WINDOW (addAccountCancelDialog)->type = GTK_WINDOW_DIALOG;
  gtk_window_set_position (GTK_WINDOW (addAccountCancelDialog), GTK_WIN_POS_MOUSE);
  gtk_window_set_modal (GTK_WINDOW (addAccountCancelDialog), TRUE);
  gtk_window_set_policy (GTK_WINDOW (addAccountCancelDialog), FALSE, FALSE, FALSE);

  newAccountCancelDialog = GNOME_DIALOG (addAccountCancelDialog)->vbox;
  gtk_widget_set_name (newAccountCancelDialog, "newAccountCancelDialog");
  gtk_object_set_data (GTK_OBJECT (addAccountCancelDialog), "newAccountCancelDialog", newAccountCancelDialog);
  gtk_widget_show (newAccountCancelDialog);

  vbox1 = gtk_vbox_new (FALSE, 5);
  gtk_widget_set_name (vbox1, "vbox1");
  gtk_widget_ref (vbox1);
  gtk_object_set_data_full (GTK_OBJECT (addAccountCancelDialog), "vbox1", vbox1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox1);
  gtk_box_pack_start (GTK_BOX (newAccountCancelDialog), vbox1, TRUE, TRUE, 0);

  newAccountCancelDialog_ShouldRunAgainLable = gtk_label_new (_("Uncheck if you do not want this dialog to run if you start GnuCash up again. "));
  gtk_widget_set_name (newAccountCancelDialog_ShouldRunAgainLable, "newAccountCancelDialog_ShouldRunAgainLable");
  gtk_widget_ref (newAccountCancelDialog_ShouldRunAgainLable);
  gtk_object_set_data_full (GTK_OBJECT (addAccountCancelDialog), "newAccountCancelDialog_ShouldRunAgainLable", newAccountCancelDialog_ShouldRunAgainLable,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (newAccountCancelDialog_ShouldRunAgainLable);
  gtk_box_pack_start (GTK_BOX (vbox1), newAccountCancelDialog_ShouldRunAgainLable, FALSE, FALSE, 0);
  gtk_label_set_line_wrap (GTK_LABEL (newAccountCancelDialog_ShouldRunAgainLable), TRUE);

  newAccountCancelDialog_RunAgainToggle = gtk_check_button_new_with_label (_("Run dialog for new user again?"));
  gtk_widget_set_name (newAccountCancelDialog_RunAgainToggle, "newAccountCancelDialog_RunAgainToggle");
  gtk_widget_ref (newAccountCancelDialog_RunAgainToggle);
  gtk_object_set_data_full (GTK_OBJECT (addAccountCancelDialog), "newAccountCancelDialog_RunAgainToggle", newAccountCancelDialog_RunAgainToggle,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (newAccountCancelDialog_RunAgainToggle);
  gtk_box_pack_start (GTK_BOX (vbox1), newAccountCancelDialog_RunAgainToggle, FALSE, FALSE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (newAccountCancelDialog_RunAgainToggle), TRUE);

  dialog_action_area1 = GNOME_DIALOG (addAccountCancelDialog)->action_area;
  gtk_widget_set_name (dialog_action_area1, "dialog_action_area1");
  gtk_object_set_data (GTK_OBJECT (addAccountCancelDialog), "dialog_action_area1", dialog_action_area1);
  gtk_widget_show (dialog_action_area1);
  gtk_button_box_set_spacing (GTK_BUTTON_BOX (dialog_action_area1), 8);

  gnome_dialog_append_button (GNOME_DIALOG (addAccountCancelDialog), GNOME_STOCK_BUTTON_OK);
  newAccountCancelDialog_OKButton = GTK_WIDGET (g_list_last (GNOME_DIALOG (addAccountCancelDialog)->buttons)->data);
  gtk_widget_set_name (newAccountCancelDialog_OKButton, "newAccountCancelDialog_OKButton");
  gtk_widget_ref (newAccountCancelDialog_OKButton);
  gtk_object_set_data_full (GTK_OBJECT (addAccountCancelDialog), "newAccountCancelDialog_OKButton", newAccountCancelDialog_OKButton,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (newAccountCancelDialog_OKButton);
  GTK_WIDGET_SET_FLAGS (newAccountCancelDialog_OKButton, GTK_CAN_DEFAULT);

  gtk_signal_connect (GTK_OBJECT (newAccountCancelDialog_OKButton), "clicked",
                      GTK_SIGNAL_FUNC (on_newAccountCancelDialog_OKButton_clicked),
                      NULL);

  return addAccountCancelDialog;
}

GtkWidget*
create_newUserChoiceWindow (void)
{
  GtkWidget *newUserChoiceWindow;
  GtkWidget *dialog_vbox1;
  GtkWidget *frame4;
  GtkWidget *vbox6;
  GSList *new_user_group_group = NULL;
  GtkWidget *new_accounts_button;
  GtkWidget *import_qif_button;
  GtkWidget *tutorial_button;
  GtkWidget *dialog_action_area2;
  GtkWidget *button1;
  GtkWidget *button3;

  newUserChoiceWindow = gnome_dialog_new (_("Welcome to GnuCash 1.6!"), NULL);
  gtk_widget_set_name (newUserChoiceWindow, "newUserChoiceWindow");
  gtk_object_set_data (GTK_OBJECT (newUserChoiceWindow), "newUserChoiceWindow", newUserChoiceWindow);
  gtk_window_set_policy (GTK_WINDOW (newUserChoiceWindow), FALSE, FALSE, FALSE);

  dialog_vbox1 = GNOME_DIALOG (newUserChoiceWindow)->vbox;
  gtk_widget_set_name (dialog_vbox1, "dialog_vbox1");
  gtk_object_set_data (GTK_OBJECT (newUserChoiceWindow), "dialog_vbox1", dialog_vbox1);
  gtk_widget_show (dialog_vbox1);

  frame4 = gtk_frame_new (NULL);
  gtk_widget_set_name (frame4, "frame4");
  gtk_widget_ref (frame4);
  gtk_object_set_data_full (GTK_OBJECT (newUserChoiceWindow), "frame4", frame4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame4);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), frame4, FALSE, FALSE, 0);

  vbox6 = gtk_vbox_new (FALSE, 0);
  gtk_widget_set_name (vbox6, "vbox6");
  gtk_widget_ref (vbox6);
  gtk_object_set_data_full (GTK_OBJECT (newUserChoiceWindow), "vbox6", vbox6,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox6);
  gtk_container_add (GTK_CONTAINER (frame4), vbox6);
  gtk_container_set_border_width (GTK_CONTAINER (vbox6), 3);

  new_accounts_button = gtk_radio_button_new_with_label (new_user_group_group, _("Create a new set of accounts"));
  new_user_group_group = gtk_radio_button_group (GTK_RADIO_BUTTON (new_accounts_button));
  gtk_widget_set_name (new_accounts_button, "new_accounts_button");
  gtk_widget_ref (new_accounts_button);
  gtk_object_set_data_full (GTK_OBJECT (newUserChoiceWindow), "new_accounts_button", new_accounts_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (new_accounts_button);
  gtk_box_pack_start (GTK_BOX (vbox6), new_accounts_button, FALSE, FALSE, 0);

  import_qif_button = gtk_radio_button_new_with_label (new_user_group_group, _("Import my QIF files"));
  new_user_group_group = gtk_radio_button_group (GTK_RADIO_BUTTON (import_qif_button));
  gtk_widget_set_name (import_qif_button, "import_qif_button");
  gtk_widget_ref (import_qif_button);
  gtk_object_set_data_full (GTK_OBJECT (newUserChoiceWindow), "import_qif_button", import_qif_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (import_qif_button);
  gtk_box_pack_start (GTK_BOX (vbox6), import_qif_button, FALSE, FALSE, 0);

  tutorial_button = gtk_radio_button_new_with_label (new_user_group_group, _("Open the new user tutorial"));
  new_user_group_group = gtk_radio_button_group (GTK_RADIO_BUTTON (tutorial_button));
  gtk_widget_set_name (tutorial_button, "tutorial_button");
  gtk_widget_ref (tutorial_button);
  gtk_object_set_data_full (GTK_OBJECT (newUserChoiceWindow), "tutorial_button", tutorial_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (tutorial_button);
  gtk_box_pack_start (GTK_BOX (vbox6), tutorial_button, FALSE, FALSE, 0);

  dialog_action_area2 = GNOME_DIALOG (newUserChoiceWindow)->action_area;
  gtk_widget_set_name (dialog_action_area2, "dialog_action_area2");
  gtk_object_set_data (GTK_OBJECT (newUserChoiceWindow), "dialog_action_area2", dialog_action_area2);
  gtk_widget_show (dialog_action_area2);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area2), GTK_BUTTONBOX_END);
  gtk_button_box_set_spacing (GTK_BUTTON_BOX (dialog_action_area2), 8);

  gnome_dialog_append_button (GNOME_DIALOG (newUserChoiceWindow), GNOME_STOCK_BUTTON_OK);
  button1 = GTK_WIDGET (g_list_last (GNOME_DIALOG (newUserChoiceWindow)->buttons)->data);
  gtk_widget_set_name (button1, "button1");
  gtk_widget_ref (button1);
  gtk_object_set_data_full (GTK_OBJECT (newUserChoiceWindow), "button1", button1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (button1);
  GTK_WIDGET_SET_FLAGS (button1, GTK_CAN_DEFAULT);

  gnome_dialog_append_button (GNOME_DIALOG (newUserChoiceWindow), GNOME_STOCK_BUTTON_CANCEL);
  button3 = GTK_WIDGET (g_list_last (GNOME_DIALOG (newUserChoiceWindow)->buttons)->data);
  gtk_widget_set_name (button3, "button3");
  gtk_widget_ref (button3);
  gtk_object_set_data_full (GTK_OBJECT (newUserChoiceWindow), "button3", button3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (button3);
  GTK_WIDGET_SET_FLAGS (button3, GTK_CAN_DEFAULT);

  return newUserChoiceWindow;
}
