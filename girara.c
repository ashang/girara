/* See LICENSE file for license and copyright information */

#include <stdlib.h>

#include "girara.h"

girara_session_t*
girara_session_create()
{
  girara_session_t* session = malloc(sizeof(girara_session_t));
  if(!session)
    return NULL;

  /* init values */
  session->gtk.window            = NULL;
  session->gtk.box               = NULL;
  session->gtk.view              = NULL;
  session->gtk.statusbar         = NULL;
  session->gtk.statusbar_entries = NULL;
  session->gtk.inputbar          = NULL;
  session->gtk.embed             = 0;

  session->style.font            = NULL;

  session->bindings.mouse_events       = NULL;
  session->bindings.commands           = NULL;
  session->bindings.shortcuts          = NULL;
  session->bindings.inputbar_shortcuts = NULL;

  session->elements.statusbar_items    = NULL;

  session->settings.settings                        = NULL;
  session->settings.font                            = "monospace normal 9";
  session->settings.default_background              = "#000000";
  session->settings.default_foreground              = "#DDDDDD";
  session->settings.inputbar_background             = "#141414";
  session->settings.inputbar_foreground             = "#9FBC00";
  session->settings.statusbar_background            = "#000000";
  session->settings.statusbar_foreground            = "#FFFFFF";
  session->settings.completion_foreground           = "#DDDDDD";
  session->settings.completion_background           = "#232323";
  session->settings.completion_group_foreground     = "#DEDEDE";
  session->settings.completion_group_background     = "#000000";
  session->settings.completion_highlight_foreground = "#232323";
  session->settings.completion_highlight_background = "#9FBC00";
  session->settings.notification_error_background   = "#FF1212";
  session->settings.notification_error_foreground   = "#FFFFFF";
  session->settings.notification_warning_background = "#FFF712";
  session->settings.notification_warning_foreground = "#000000";

  session->settings.width  = 800;
  session->settings.height = 600;

  /* add default settings */
  girara_setting_add(session, "font",                     &(session->settings.font),                            STRING, TRUE, NULL, NULL);
  girara_setting_add(session, "default-fg",               &(session->settings.default_foreground),              STRING, TRUE, NULL, NULL);
  girara_setting_add(session, "default-bg",               &(session->settings.default_background),              STRING, TRUE, NULL, NULL);
  girara_setting_add(session, "inputbar-fg",              &(session->settings.inputbar_foreground),             STRING, TRUE, NULL, NULL);
  girara_setting_add(session, "inputbar-bg",              &(session->settings.inputbar_background),             STRING, TRUE, NULL, NULL);
  girara_setting_add(session, "statusbar-fg",             &(session->settings.statusbar_foreground),            STRING, TRUE, NULL, NULL);
  girara_setting_add(session, "statusbar-bg",             &(session->settings.statusbar_background),            STRING, TRUE, NULL, NULL);
  girara_setting_add(session, "completion-fg",            &(session->settings.completion_foreground),           STRING, TRUE, NULL, NULL);
  girara_setting_add(session, "completion-bg",            &(session->settings.completion_background),           STRING, TRUE, NULL, NULL);
  girara_setting_add(session, "completion-group-fg",      &(session->settings.completion_group_foreground),     STRING, TRUE, NULL, NULL);
  girara_setting_add(session, "completion-group-bg",      &(session->settings.completion_group_background),     STRING, TRUE, NULL, NULL);
  girara_setting_add(session, "completion-highlight-fg",  &(session->settings.completion_highlight_foreground), STRING, TRUE, NULL, NULL);
  girara_setting_add(session, "completion-highlight-bg",  &(session->settings.completion_highlight_background), STRING, TRUE, NULL, NULL);
  girara_setting_add(session, "notification-error-fg",    &(session->settings.notification_error_foreground),   STRING, TRUE, NULL, NULL);
  girara_setting_add(session, "notification-error-bg",    &(session->settings.notification_error_background),   STRING, TRUE, NULL, NULL);
  girara_setting_add(session, "notification-warning-fg",  &(session->settings.notification_warning_foreground), STRING, TRUE, NULL, NULL);
  girara_setting_add(session, "notification-warning-bg",  &(session->settings.notification_warning_background), STRING, TRUE, NULL, NULL);
  girara_setting_add(session, "width",                    &(session->settings.width),                           INT,    TRUE, NULL, NULL);
  girara_setting_add(session, "height",                   &(session->settings.height),                          INT,    TRUE, NULL, NULL);

  return session;
}

gboolean
girara_session_init(girara_session_t* session)
{
  if(session->gtk.embed)
    session->gtk.window = gtk_plug_new(session->gtk.embed);
  else
    session->gtk.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  session->gtk.box               = GTK_BOX(gtk_vbox_new(FALSE, 0));
  session->gtk.view              = gtk_scrolled_window_new(NULL, NULL);
  session->gtk.statusbar         = gtk_event_box_new();
  session->gtk.statusbar_entries = GTK_BOX(gtk_hbox_new(FALSE, 0));
  session->gtk.inputbar          = GTK_ENTRY(gtk_entry_new());

  /* window */
  GdkGeometry hints = {1, 1};
  gtk_window_set_geometry_hints(GTK_WINDOW(session->gtk.window), NULL, &hints, GDK_HINT_MIN_SIZE);

  /* box */
  gtk_box_set_spacing(session->gtk.box, 0);
  gtk_container_add(GTK_CONTAINER(session->gtk.window), GTK_WIDGET(session->gtk.box));

  /* statusbar */
  gtk_container_add(GTK_CONTAINER(session->gtk.statusbar), GTK_WIDGET(session->gtk.statusbar_entries));

  /* inputbar */
  gtk_entry_set_inner_border(session->gtk.inputbar, NULL);
  gtk_entry_set_has_frame(session->gtk.inputbar, FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(session->gtk.inputbar), TRUE);

  /* packing */
  gtk_box_pack_start(session->gtk.box, GTK_WIDGET(session->gtk.view),       TRUE,  TRUE, 0);
  gtk_box_pack_start(session->gtk.box, GTK_WIDGET(session->gtk.statusbar), FALSE, FALSE, 0);
  gtk_box_pack_end(  session->gtk.box, GTK_WIDGET(session->gtk.inputbar),  FALSE, FALSE, 0);

  /* parse color values */
  gdk_color_parse(session->settings.default_foreground,              &(session->style.default_foreground));
  gdk_color_parse(session->settings.default_background,              &(session->style.default_background));
  gdk_color_parse(session->settings.inputbar_foreground,             &(session->style.inputbar_foreground));
  gdk_color_parse(session->settings.inputbar_background,             &(session->style.inputbar_background));
  gdk_color_parse(session->settings.statusbar_foreground,            &(session->style.statusbar_foreground));
  gdk_color_parse(session->settings.statusbar_background,            &(session->style.statusbar_background));
  gdk_color_parse(session->settings.completion_foreground,           &(session->style.completion_foreground));
  gdk_color_parse(session->settings.completion_background,           &(session->style.completion_background));
  gdk_color_parse(session->settings.completion_group_foreground,     &(session->style.completion_group_foreground));
  gdk_color_parse(session->settings.completion_group_background,     &(session->style.completion_group_background));
  gdk_color_parse(session->settings.completion_highlight_foreground, &(session->style.completion_highlight_foreground));
  gdk_color_parse(session->settings.completion_highlight_background, &(session->style.completion_highlight_background));
  gdk_color_parse(session->settings.notification_error_foreground,   &(session->style.notification_error_foreground));
  gdk_color_parse(session->settings.notification_error_background,   &(session->style.notification_error_background));
  gdk_color_parse(session->settings.notification_warning_foreground, &(session->style.notification_warning_foreground));
  gdk_color_parse(session->settings.notification_warning_background, &(session->style.notification_warning_background));

  session->style.font = pango_font_description_from_string(session->settings.font);

  /* statusbar */
  gtk_widget_modify_bg(GTK_WIDGET(session->gtk.statusbar), GTK_STATE_NORMAL, &(session->style.statusbar_background));

  /* inputbar */
  gtk_widget_modify_base(GTK_WIDGET(session->gtk.inputbar), GTK_STATE_NORMAL, &(session->style.inputbar_background));
  gtk_widget_modify_text(GTK_WIDGET(session->gtk.inputbar), GTK_STATE_NORMAL, &(session->style.inputbar_foreground));
  gtk_widget_modify_font(GTK_WIDGET(session->gtk.inputbar),                     session->style.font);

  /* set window size */
  gtk_window_set_default_size(GTK_WINDOW(session->gtk.window), session->settings.width, session->settings.height);

  gtk_widget_show_all(GTK_WIDGET(session->gtk.window));

  return 0;
}

gboolean
girara_session_destroy(girara_session_t* session)
{
  if(!session)
    return FALSE;

  /* clena up style */
  pango_font_description_free(session->style.font);

  /* clean up settings */
  girara_setting_t* setting = session->settings.settings;
  while(setting)
  {
    girara_setting_t* tmp = setting->next;

    if(setting->name)
      free(setting->name);
    if(setting->description)
      free(setting->description);

    free(setting);

    setting = tmp;
  }

  /* clean up statusbar items */
  girara_statusbar_item_t* item = session->elements.statusbar_items;
  while(item)
  {
    girara_statusbar_item_t* tmp = item->next;

    free(item);

    item = tmp;
  }

  free(session);
  return TRUE;
}

gboolean
girara_setting_add(girara_session_t* session, char* name, void* value, girara_setting_type_t type, gboolean init_only, char* description, girara_setting_callback_t callback)
{
  if(!session)
    return FALSE;

  /* search for existing setting */
  girara_setting_t* tmp = session->settings.settings;
  while(tmp && tmp->next)
  {
    if(!g_strcmp0(name, tmp->name))
      return FALSE;

    tmp = tmp->next;
  }

  /* add new setting */
  girara_setting_t* setting = malloc(sizeof(girara_setting_t));
  if(!setting)
    return FALSE;

  setting->name        = name ? g_strdup(name) : NULL;
  setting->value       = value;
  setting->type        = type;
  setting->init_only   = init_only;
  setting->description = description ? g_strdup(description) : NULL;
  setting->callback    = callback;
  setting->next        = NULL;

  if(tmp)
    tmp->next = setting;
  else
    session->settings.settings = setting;

  return TRUE;
}

gboolean
girara_setting_set(girara_session_t* session, char* name, void* value)
{
  if(!session)
    return FALSE;

  girara_setting_t* setting = session->settings.settings;
  while(setting)
  {
    setting = setting->next;
  }

  return TRUE;
}

gboolean
girara_shortcut_add(girara_session_t* session, int modifier, int key, char* buffer, girara_shortcut_function_t function, girara_mode_t mode, girara_argument_t argument)
{
  return TRUE;
}

gboolean
girara_inputbar_command_add(girara_session_t* session, char* command , char* abbreviation, girara_command_function_t function, girara_completion_function_t completion, char* description)
{
  return TRUE;
}

gboolean
girara_inputbar_shortcut_add(girara_session_t* session, int modifier, int key, girara_shortcut_function_t function, girara_argument_t argument)
{
  return TRUE;
}

gboolean
girara_inputbar_special_command_add(girara_session_t* session, char identifier, girara_inputbar_special_function_t function, gboolean always, girara_argument_t argument)
{
  return TRUE;
}

gboolean
girara_mouse_event_add(girara_session_t* session, int mask, int button, girara_shortcut_function_t function, girara_mode_t mode, girara_argument_t argument)
{
  return TRUE;
}

girara_statusbar_item_t*
girara_statusbar_item_add(girara_session_t* session, gboolean expand, gboolean fill, gboolean left, girara_statusbar_event_t callback)
{
  if(!session)
    return NULL;

  girara_statusbar_item_t* item = malloc(sizeof(girara_statusbar_item_t));
  if(!item)
    return NULL;

  item->text = GTK_LABEL(gtk_label_new(NULL));
  item->next = NULL;

  /* set style */
  gtk_widget_modify_fg(GTK_WIDGET(item->text),     GTK_STATE_NORMAL, &(session->style.statusbar_foreground));
  gtk_widget_modify_font(GTK_WIDGET(item->text),   session->style.font);

  /* set properties */
  gtk_misc_set_alignment(GTK_MISC(item->text),     left ? 0.0 : 1.0, 0.0);
  gtk_misc_set_padding(GTK_MISC(item->text),       2.0, 4.0);
  gtk_label_set_use_markup(item->text,             TRUE);

  if(callback)
    g_signal_connect(G_OBJECT(item->text), "event", G_CALLBACK(callback), NULL);

  /* add it to the list */
  gtk_box_pack_start(session->gtk.statusbar_entries, GTK_WIDGET(item->text), expand, fill, 2);
  gtk_widget_show_all(GTK_WIDGET(item->text));

  if(session->elements.statusbar_items)
    item->next = session->elements.statusbar_items;

  session->elements.statusbar_items = item;

  return item;
}

gboolean
girara_statusbar_item_set_text(girara_session_t* session, girara_statusbar_item_t* item, char* text)
{
  if(!session || !item)
    return FALSE;

  char* escaped_text = g_markup_escape_text(text, -1);
  gtk_label_set_markup((GtkLabel*) item->text, escaped_text);
  g_free(escaped_text);

  return TRUE;
}

gboolean
girara_set_view(girara_session_t* session, GtkWidget* widget)
{
  return TRUE;
}