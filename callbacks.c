/* See LICENSE file for license and copyright information */

#include "callbacks.h"
#include "datastructures.h"
#include "session.h"
#include "shortcuts.h"
#include <string.h>

#include "internal.h"
#if GTK_MAJOR_VERSION == 2
#include "gtk2-compat.h"
#endif

/* callback implementation */
bool
girara_callback_view_key_press_event(GtkWidget* UNUSED(widget), GdkEventKey* event, girara_session_t* session)
{
  g_return_val_if_fail(session != NULL, FALSE);

  GIRARA_LIST_FOREACH(session->bindings.shortcuts, girara_shortcut_t*, iter, shortcut)
    if (session->buffer.command) {
      break;
    }
    if (
       event->keyval == shortcut->key
       && (CLEAN(event->state) == shortcut->mask || (shortcut->key >= 0x21
       && shortcut->key <= 0x7E && CLEAN(event->state) == GDK_SHIFT_MASK))
       && (session->modes.current_mode & shortcut->mode || shortcut->mode == 0)
       && shortcut->function
      )
    {
      int t = (session->buffer.n > 0) ? session->buffer.n : 1;
      for (int i = 0; i < t; i++) {
        if (!shortcut->function(session, &(shortcut->argument), session->buffer.n)) {
          break;
        }
      }

      if (session->global.buffer) {
        g_string_free(session->global.buffer, TRUE);
        session->global.buffer = NULL;
      }

      session->buffer.n = 0;

      if (session->events.buffer_changed) {
        session->events.buffer_changed(session);
      }

      girara_list_iterator_free(iter);
      return TRUE;
    }
  GIRARA_LIST_FOREACH_END(session->bindings.shortcuts, girara_shortcut_t*, iter, shortcut);

  /* update buffer */
  if (event->keyval >= 0x21 && event->keyval <= 0x7E) {
    /* overall buffer */
    if (!session->global.buffer) {
      session->global.buffer = g_string_new("");
    }

    session->global.buffer = g_string_append_c(session->global.buffer, event->keyval);

    if (!session->buffer.command && event->keyval >= 0x30 && event->keyval <= 0x39) {
      if (((session->buffer.n * 10) + (event->keyval - '0')) < INT_MAX) {
        session->buffer.n = (session->buffer.n * 10) + (event->keyval - '0');
      }
    } else {
      if (!session->buffer.command) {
        session->buffer.command = g_string_new("");
      }

      session->buffer.command = g_string_append_c(session->buffer.command, event->keyval);
    }

    if (session->events.buffer_changed) {
      session->events.buffer_changed(session);
    }
  }

  /* check for buffer command */
  if (session->buffer.command) {
    bool matching_command = FALSE;

    GIRARA_LIST_FOREACH(session->bindings.shortcuts, girara_shortcut_t*, iter, shortcut)
      if (shortcut->buffered_command) {
        /* buffer could match a command */
        if (!strncmp(session->buffer.command->str, shortcut->buffered_command, session->buffer.command->len)) {
          /* command matches buffer exactly */
          if (!strcmp(session->buffer.command->str, shortcut->buffered_command)) {
            g_string_free(session->buffer.command, TRUE);
            g_string_free(session->global.buffer,  TRUE);
            session->buffer.command = NULL;
            session->global.buffer  = NULL;

            if (session->events.buffer_changed) {
              session->events.buffer_changed(session);
            }

            int t = (session->buffer.n > 0) ? session->buffer.n : 1;
            for (int i = 0; i < t; i++) {
              if (!shortcut->function(session, &(shortcut->argument), session->buffer.n)) {
                break;
              }
            }

            session->buffer.n = 0;
            girara_list_iterator_free(iter);
            return TRUE;
          }

          matching_command = TRUE;
        }
      }
    GIRARA_LIST_FOREACH_END(session->bindings.shortcuts, girara_shortcut_t*, iter, shortcut);

    /* free buffer if buffer will never match a command */
    if (!matching_command) {
      g_string_free(session->buffer.command, TRUE);
      g_string_free(session->global.buffer,  TRUE);
      session->buffer.command = NULL;
      session->global.buffer  = NULL;
      session->buffer.n       = 0;

      if (session->events.buffer_changed) {
        session->events.buffer_changed(session);
      }
    }
  }

  return FALSE;
}

bool
girara_callback_inputbar_activate(GtkEntry* entry, girara_session_t* session)
{
  g_return_val_if_fail(session != NULL, FALSE);

  gchar *input  = gtk_editable_get_chars(GTK_EDITABLE(entry), 1, -1);
  if (!input) {
    return false;
  }

  if (strlen(input) == 0) {
    return false;
  }

  gchar** argv = NULL;
  gint    argc = 0;

  if (g_shell_parse_argv(input, &argc, &argv, NULL) == FALSE) {
    g_free(input);
    return false;
  }

  gchar *cmd = argv[0];

  /* special commands */
  char *identifier_s = gtk_editable_get_chars(GTK_EDITABLE(entry), 0, 1);
  char identifier    = identifier_s[0];
  g_free(identifier_s);

  GIRARA_LIST_FOREACH(session->bindings.special_commands, girara_special_command_t*, iter, special_command)
    if (special_command->identifier == identifier) {
      if (special_command->always != true) {
        special_command->function(session, input, &(special_command->argument));
      }

      g_free(input);
      g_strfreev(argv);

      girara_isc_abort(session, NULL, 0);

      girara_list_iterator_free(iter);
      return true;
    }
  GIRARA_LIST_FOREACH_END(session->bindings.special_commands, girara_special_command_t*, iter, special_command);

  /* search commands */
  GIRARA_LIST_FOREACH(session->bindings.commands, girara_command_t*, iter, command)
    if ((g_strcmp0(cmd, command->command) == 0) ||
       (g_strcmp0(cmd, command->abbr)    == 0))
    {
      girara_list_t* argument_list = girara_list_new();
      if (argument_list == NULL) {
        g_free(input);
        g_strfreev(argv);
        girara_list_iterator_free(iter);
        return false;
      }

      girara_list_set_free_function(argument_list, g_free);

      for(int i = 1; i < argc; i++) {
        char* argument = g_strdup(argv[i]);
        if (argument != NULL) {
          girara_list_append(argument_list, (void*) argument);
        } else {
          girara_list_free(argument_list);
          g_free(input);
          g_strfreev(argv);
          girara_list_iterator_free(iter);
          return false;
        }
      }

      command->function(session, argument_list);

      girara_list_free(argument_list);
      g_free(input);
      g_strfreev(argv);

      girara_isc_abort(session, NULL, 0);

      gtk_widget_hide(GTK_WIDGET(session->gtk.inputbar));
      girara_list_iterator_free(iter);
      return true;
    }
  GIRARA_LIST_FOREACH_END(session->bindings.commands, girara_command_t*, iter, command);

  /* no known command */
  girara_isc_abort(session, NULL, 0);

  return false;
}

bool
girara_callback_inputbar_key_press_event(GtkWidget* entry, GdkEventKey* event, girara_session_t* session)
{
  g_return_val_if_fail(session != NULL, false);

  GIRARA_LIST_FOREACH(session->bindings.inputbar_shortcuts, girara_inputbar_shortcut_t*, iter, inputbar_shortcut)
    if (inputbar_shortcut->key == event->keyval
     && inputbar_shortcut->mask == CLEAN(event->state))
    {
      if (inputbar_shortcut->function != NULL) {
        inputbar_shortcut->function(session, &(inputbar_shortcut->argument), 0);
      }

      girara_list_iterator_free(iter);
      return true;
    }
  GIRARA_LIST_FOREACH_END(session->bindings.inputbar_shortcuts, girara_inputbar_shortcut_t*, iter, inputbar_shortcut);

  /* special commands */
  char *identifier_s = gtk_editable_get_chars(GTK_EDITABLE(entry), 0, 1);
  char identifier    = identifier_s[0];
  g_free(identifier_s);

  GIRARA_LIST_FOREACH(session->bindings.special_commands, girara_special_command_t*, iter, special_command)
    if ((special_command->identifier == identifier) &&
       (special_command->always == true))
    {
      gchar *input  = gtk_editable_get_chars(GTK_EDITABLE(entry), 1, -1);
      special_command->function(session, input, &(special_command->argument));
      g_free(input);
      girara_list_iterator_free(iter);
      return false;
    }
  GIRARA_LIST_FOREACH_END(session->bindings.special_commands, girara_special_command_t*, iter, special_command);

  if ((session->gtk.results != NULL) &&
     (gtk_widget_get_visible(GTK_WIDGET(session->gtk.results)) == TRUE) &&
     (event->keyval == GDK_KEY_space))
  {
    gtk_widget_hide(GTK_WIDGET(session->gtk.results));
  }

  return false;
}