/*
 * Copyright (C) 2004 Mike Newman <mikegtn@gnome.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <gnome.h>
#include <inklevel.h>
#include <glade/glade-xml.h>
#include <gconf/gconf-client.h>
#include "inkblot.h"
#include "eggtrayicon.h"

GtkWidget	*icon_box;
GtkWidget	*icon;
GtkWidget	*dialog;
GtkWidget	*icon_pix;
GtkTooltips	*tooltips = NULL;
gint		visible;
gint		pref_bus;
gint		pref_port;
InkblotStatus	state;
static void	inkblot_assign_tooltip (InkblotStatus);

gint inkblot_check_prefs ( void ) {

  GConfClient *client = gconf_client_get_default ();
  if (! gconf_client_get_int (client, INKBLOT_CONFIG_KEY, NULL)) {
    inkblot_scan_devices();
  }
  pref_bus = gconf_client_get_int (client, INKBLOT_BUS_KEY, NULL);
  pref_port = gconf_client_get_int (client, INKBLOT_PORT_KEY, NULL);
  return 0;
}

int main (int argc, char *argv[]) {

  GnomeClient *client;
  GdkPixbuf *pixbuf = NULL;
#ifdef ENABLE_NLS
  bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
#endif

  gnome_program_init (PACKAGE, VERSION, LIBGNOMEUI_MODULE,
		      argc, argv,
		      GNOME_PARAM_APP_DATADIR, PACKAGE_DATA_DIR,
		      NULL);
  client = gnome_master_client ();
  tooltips = gtk_tooltips_new();
  gtk_tooltips_enable(tooltips);
  inkblot_check_prefs();
  g_signal_connect (client, "save-yourself", 
		    G_CALLBACK(inkblot_save_session), (gpointer) argv[0]);
  icon = GTK_WIDGET(egg_tray_icon_new ("inkblot tray icon"));
  icon_box = gtk_event_box_new ();
  state = inkblot_get_overall_status();
  g_signal_connect (G_OBJECT(icon_box), "button_press_event", 
		    G_CALLBACK(inkblot_tray_icon_clicked), NULL);
  gtk_container_add (GTK_CONTAINER(icon), icon_box);
  pixbuf = inkblot_tray_icon_display(state);
  icon_pix = gtk_image_new_from_pixbuf(pixbuf);
  gdk_pixbuf_unref(pixbuf);
  gtk_container_add(GTK_CONTAINER(icon_box),icon_pix); 
  gtk_widget_show_all(icon);
  timer_id = g_timeout_add(3000,inkblot_check_status_event,NULL);
  gtk_main ();
  return 0;
}

gboolean inkblot_check_status_event ( gpointer data ) {
  GdkPixbuf *buf = NULL;
  gint oldstate = state;
  state = inkblot_get_overall_status();
  if (state != oldstate) {
    buf = inkblot_tray_icon_display(state);
    gtk_image_set_from_pixbuf(GTK_IMAGE(icon_pix),buf);
    gdk_pixbuf_unref(buf);
  }
  return TRUE;
}

void inkblot_assign_tooltip (InkblotStatus state) {
  gchar *tip;
  switch (state) {
  case INKBLOT_OK:
    tip = g_strdup (_("Ink level: high"));
    break;
  case INKBLOT_WARN:
    tip = g_strdup (_("Ink level: medium"));
    break;
  case INKBLOT_LOW:
    tip = g_strdup (_("Ink level: low"));
    break;
  case INKBLOT_EMPTY:
    tip = g_strdup (_("Printer off or ink empty"));
    break;
  default:
    tip = g_strdup (_("Error: problem with printer"));
  }
  gtk_tooltips_set_tip(tooltips, GTK_WIDGET(icon_box), tip, NULL);
  g_free(tip);
}
	
GdkPixbuf * inkblot_tray_icon_display ( InkblotStatus state ) {
  GdkPixbuf *pixbuf;
  GError *error = NULL;
  gchar *pixbuf_file = NULL;
  switch (state) {
  case INKBLOT_OK:
    pixbuf_file = g_strdup(PACKAGE_DATA_DIR
			   "/inkblot/pixmaps/printer_green_16.png");
    break;
  case INKBLOT_WARN:
    pixbuf_file = g_strdup(PACKAGE_DATA_DIR
			   "/inkblot/pixmaps/printer_amber_16.png");
    break;
  case INKBLOT_LOW:
    pixbuf_file = g_strdup(PACKAGE_DATA_DIR
			   "/inkblot/pixmaps/printer_red_16.png");
    break;
  case INKBLOT_EMPTY:
    pixbuf_file = g_strdup(PACKAGE_DATA_DIR
			   "/inkblot/pixmaps/printer_black_16.png");
    break;
  default:
    pixbuf_file = g_strdup(PACKAGE_DATA_DIR
			   "/inkblot/pixmaps/printer_error_16.png");
  }
  pixbuf = gdk_pixbuf_new_from_file (pixbuf_file,&error);
  inkblot_assign_tooltip(state);
  g_free (pixbuf_file);
  return pixbuf;
};

void inkblot_tray_icon_clicked ( GtkWidget *event_box, GdkEventButton *event, gpointer data ) {

  if (event->type != GDK_BUTTON_PRESS) {
    return;
  }

  if (event->button == 1 && state != INKBLOT_ERROR) {
    inkblot_toggle_dialog();
  }

  if (event->button == 3) {
    inkblot_tray_menu (event_box, event, data);
  }
}

void inkblot_toggle_dialog ( void ) {
  GladeXML *glade;
  struct ink_level level;
  gchar *black=NULL;
  gchar *color=NULL;
  gchar *cyan=NULL;
  gchar *magenta=NULL;
  gchar *yellow=NULL;
  GtkWidget *b_label;
  GtkWidget *c_label;
  GtkWidget *cy_label;
  GtkWidget *mg_label;
  GtkWidget *yl_label;
  GtkWidget *b_prog;
  GtkWidget *c_prog;
  GtkWidget *cy_prog;
  GtkWidget *mg_prog;
  GtkWidget *yl_prog;
  GtkWidget *expander;
  int i, colorLevel;

  if (visible) {
    inkblot_dialog_response (dialog, -1, NULL);
    return;
  }

  glade = glade_xml_new (PACKAGE_DATA_DIR 
			 "/inkblot/inkblot.glade", NULL, NULL);

  if (glade==NULL) {
    inkblot_error_dialog (INKBLOT_ERROR_GLADE);
    exit (1);
  }

  dialog = glade_xml_get_widget (glade,"inkblot_dialog");
  expander = glade_xml_get_widget (glade,"expander");
  b_label = glade_xml_get_widget (glade,"black_label");
  c_label = glade_xml_get_widget (glade,"color_label");
  cy_label = glade_xml_get_widget (glade,"cy_label");
  mg_label = glade_xml_get_widget (glade,"mg_label");
  yl_label = glade_xml_get_widget (glade,"yl_label");
  b_prog = glade_xml_get_widget (glade,"black_progress");
  c_prog = glade_xml_get_widget (glade,"color_progress");
  cy_prog = glade_xml_get_widget (glade,"cy_prog");
  mg_prog = glade_xml_get_widget (glade,"mg_prog");
  yl_prog = glade_xml_get_widget (glade,"yl_prog");
  get_ink_level (pref_bus, "",pref_port, &level);

  /* First check if the ink level request succeeded */
  if(level.status != RESPONSE_VALID) {
    inkblot_error_dialog ( INKBLOT_ERROR_INVALID_RESPONSE );
    exit(1);
    }

    /* Parse all present cartridges and update labels consequently */
    i=0;
    while( (i<MAX_CARTRIDGE_TYPES) &&
	   (level.levels[i][INDEX_TYPE] != CARTRIDGE_NOT_PRESENT)) {
      switch(level.levels[i][INDEX_TYPE])
	{
	case CARTRIDGE_BLACK:
	case CARTRIDGE_PHOTO: /* Photo seems to be a gray cartridge */
	  black = g_strdup_printf("%d%%",level.levels[i][INDEX_LEVEL]);
	  gtk_label_set_text(GTK_LABEL(b_label),black);
	  gtk_progress_set_value(GTK_PROGRESS(b_prog),
				 level.levels[i][INDEX_LEVEL]);
	  break;
	case CARTRIDGE_COLOR:
	  color = g_strdup_printf("%d%%",level.levels[i][INDEX_LEVEL]);
	  gtk_label_set_text(GTK_LABEL(c_label),color);
	  gtk_progress_set_value(GTK_PROGRESS(c_prog),
				 level.levels[i][INDEX_LEVEL]);
	  break;
	case CARTRIDGE_CYAN:
	case CARTRIDGE_PHOTOCYAN: /* simplification, but a printer can
				     have CYAN and PHOTOCYAN... */
	  cyan = g_strdup_printf("%d%%",level.levels[i][INDEX_LEVEL]);
	  gtk_label_set_text(GTK_LABEL(cy_label),cyan);
	  gtk_progress_set_value(GTK_PROGRESS(cy_prog),
				 level.levels[i][INDEX_LEVEL]);
	  break;
	case CARTRIDGE_MAGENTA:
	case CARTRIDGE_PHOTOMAGENTA:
	  magenta = g_strdup_printf("%d%%",level.levels[i][INDEX_LEVEL]);
	  gtk_label_set_text(GTK_LABEL(mg_label),magenta);
	  gtk_progress_set_value(GTK_PROGRESS(mg_prog),
				 level.levels[i][INDEX_LEVEL]);
	  break;
	case CARTRIDGE_YELLOW:
	case CARTRIDGE_PHOTOYELLOW:
	  yellow = g_strdup_printf("%d%%",level.levels[i][INDEX_LEVEL]);
	  gtk_label_set_text(GTK_LABEL(yl_label),yellow);
	  gtk_progress_set_value(GTK_PROGRESS(yl_prog),
				 level.levels[i][INDEX_LEVEL]);
	  break;
	case CARTRIDGE_RED:
	case CARTRIDGE_GREEN:
	  /* FIXME: not yet supported */
	  break;
	}
      i++;
    }
    /* if no colour level reported, but individual reservoirs
     * available, report an average */
    if (color == NULL) {
      colorLevel = (gtk_progress_get_value(GTK_PROGRESS(cy_prog)) +
		    gtk_progress_get_value(GTK_PROGRESS(mg_prog)) +
		    gtk_progress_get_value(GTK_PROGRESS(yl_prog))  )/3;
      color = g_strdup_printf("%d%%",colorLevel);
      gtk_label_set_text(GTK_LABEL(c_label),color);
      gtk_progress_set_value(GTK_PROGRESS(c_prog), colorLevel);
    }
  gtk_window_set_title(GTK_WINDOW(dialog),level.model);
  if ( (cyan != NULL) || (magenta != NULL) || (yellow != NULL)) {
    gtk_widget_set_sensitive(expander,TRUE);
  } else {
    gtk_widget_set_sensitive(expander,FALSE);
  }
  gtk_widget_show_all(dialog);
  g_signal_connect(G_OBJECT(dialog),"response",
		   G_CALLBACK(inkblot_dialog_response),NULL);
  visible = 1;
  gtk_dialog_run(GTK_DIALOG(dialog));
}

void inkblot_dialog_response ( GtkWidget *widget, gint response, gpointer data) {
  gtk_widget_destroy(widget);
  visible = 0;
}

InkblotStatus inkblot_get_overall_status ( void ) {

  struct ink_level level;
  gint color=0;
  gint black=0;
  gint cyan=0;
  gint magenta=0;
  gint yellow=0;
  gint total=0;
  gint result=0;
  int i;

  result = get_ink_level (pref_bus, "",pref_port, &level);
  if ((result < 0) || (level.status == RESPONSE_INVALID)) return INKBLOT_ERROR;

  /* Parse all present cartridges and update labels consequently */
  i=0;
  while( (i<MAX_CARTRIDGE_TYPES) &&
	 (level.levels[i][INDEX_TYPE] != CARTRIDGE_NOT_PRESENT)) {
    switch(level.levels[i][INDEX_TYPE])
      {
      case CARTRIDGE_BLACK:
      case CARTRIDGE_PHOTO: /* Photo seems to be a gray cartridge */
	black = level.levels[i][INDEX_LEVEL];
	break;
      case CARTRIDGE_COLOR:
	color = level.levels[i][INDEX_LEVEL];
	break;
      case CARTRIDGE_CYAN:
      case CARTRIDGE_PHOTOCYAN: /* simplification, but a printer can
				   have CYAN and PHOTOCYAN... */
	cyan = level.levels[i][INDEX_LEVEL];
	break;
      case CARTRIDGE_MAGENTA:
      case CARTRIDGE_PHOTOMAGENTA:
	magenta = level.levels[i][INDEX_LEVEL];
	break;
      case CARTRIDGE_YELLOW:
      case CARTRIDGE_PHOTOYELLOW:
	yellow = level.levels[i][INDEX_LEVEL];
	break;
      case CARTRIDGE_RED:
      case CARTRIDGE_GREEN:
	/* FIXME: not yet supported */
	break;
      }
    i++;
  }
  /* if no colour level reported, but individual reservoirs
   * available, report an average */
  if (color == 0 && (cyan || magenta || yellow)) {
    color = ( cyan + magenta + yellow ) / 3;
  }
  total = (color+black)/2;
  if (total>66) return INKBLOT_OK;
  if (total>33) return INKBLOT_WARN;
  if (total>0) return INKBLOT_LOW;
  if (total==0) return INKBLOT_EMPTY;
  return INKBLOT_ERROR;
}

static int inkblot_save_session(GnomeClient        *client,
				gint                phase,
				GnomeRestartStyle   save_style,
				gint                shutdown,
				GnomeInteractStyle  interact_style,
				gint                fast,
				gpointer            client_data)  {
                                                                                
  gchar *argv[]= { NULL };
  argv[0] = (gchar*) client_data;
  gnome_client_set_clone_command (client, 1, argv);
  gnome_client_set_restart_command (client, 1, argv);
  return TRUE;
}                                                                               

gint inkblot_error_dialog ( InkblotError err ) {
  GtkWidget       *dialog;
  gchar           *message;
  gint            buttons;
  gint            type;
  gint            response;

  switch (err) {
  case INKBLOT_ERROR_NO_CONFIG:
    message = g_strdup (_("No supported printers found.\n\n"
			  "Please check your printer and cables "
			  "and restart Inkblot."));
    buttons = GTK_BUTTONS_CLOSE;
    type = GTK_MESSAGE_WARNING;
    break;
  case INKBLOT_ERROR_NO_PRINTER:
    message = g_strdup (_("No supported printers found.\n\n"
			  "Please check your printer and cables "
			  "and scan for printers again."));
    buttons = GTK_BUTTONS_CLOSE;
    type = GTK_MESSAGE_WARNING;
    break;
  case INKBLOT_ERROR_GLADE:
    message = g_strdup (_("Inkblot was not installed correctly\n"
			  "and the inkblot.glade file could not "
			  "be found."));
    type = GTK_MESSAGE_ERROR;
    buttons = GTK_BUTTONS_CLOSE;
    break;
  case INKBLOT_ERROR_INVALID_RESPONSE:
    message = g_strdup (_("Inkblot got an invalid response from "
			  "the printer.\n\n"
			  "Please check your printer and check the "
			  "libinklevel homepage if your printer is "
			  "supported."));
    type = GTK_MESSAGE_ERROR;
    buttons = GTK_BUTTONS_CLOSE;
  default:
    message = g_strdup (_("An unexpected error occurred."));
    buttons = GTK_BUTTONS_CLOSE;
    type = GTK_MESSAGE_ERROR;
  }
	
  dialog = gtk_message_dialog_new (NULL, 0, type,
				   buttons,"<b>%s</b>\n\n%s",
				   _("Inkblot Error"), message);
  gtk_label_set_use_markup 
    (GTK_LABEL(GTK_MESSAGE_DIALOG(dialog)->label),TRUE);
  response = gtk_dialog_run (GTK_DIALOG(dialog));
  g_free(message);

  if (response != GTK_RESPONSE_OK) {
    gtk_widget_destroy (dialog);
  }

  return 0;

}
