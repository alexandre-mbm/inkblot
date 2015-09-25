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
#  include <config.h>
#endif
#include <inklevel.h>
#include <gconf/gconf-client.h>
#include <glib.h>
#include "inkblot.h"

GtkWidget	*conf_dialog;

gint inkblot_scan_devices ( void ) {

	GConfClient	*client = gconf_client_get_default ();
	GList		*printers = NULL;
        gchar           *message, *bus_d, *port_d;
        gint            buttons;
        gint            type;
        gint            response;
	gint 		bus = 0, port = 0, result, num_printers = 0;
	struct 		ink_level device;
	InkblotPrinter 	*pref_device;

	if (conf_dialog != NULL) return;

	for (bus=0; bus <3; bus++) {
		for (port=0; port<17; port++) {
			result = get_ink_level (bus, "", port, &device);
			if (result == OK) {
				InkblotPrinter *dev = g_new0(InkblotPrinter,1);
				dev->model = g_strdup(device.model);
				dev->bus = bus;
				dev->port = port;
				printers = g_list_prepend (printers, dev);
				num_printers++;
			} else {
				printf("Result was %d\n", result);
			}
		}
	}
	if (num_printers < 1) {
		if (! gconf_client_get_int (client,INKBLOT_CONFIG_KEY, NULL)) {
			inkblot_error_dialog (INKBLOT_ERROR_NO_CONFIG);
			exit (1);
		}
		inkblot_error_dialog (INKBLOT_ERROR_NO_PRINTER);
		return;
	}

	/* FIXME: temp restrict to first printer. need a UI for selection */
	num_printers = 1;

	if (num_printers > 1) {
				g_print(_("Choose Printer Not Implemented"));
	} else {
		pref_device = g_list_first(printers)->data;
	}

	gconf_client_set_int (client, INKBLOT_BUS_KEY,pref_device->bus,NULL);
	gconf_client_set_int (client, INKBLOT_CONFIG_KEY,1,NULL);

	if (pref_device->bus == 1) {
		bus_d = g_strdup (_("parallel port"));
	} else {
		bus_d = g_strdup (_("USB"));
	}
	port_d = g_strdup_printf ("lp%d",pref_device->port);
	message = g_strdup_printf (_("Found <i>%s</i> %s printer on %s\n\n"
				   "Inkblot will report the ink level on "
				   "this device."),
				   pref_device->model,bus_d,port_d);
	g_free(port_d);
	g_free(bus_d);
	g_free(pref_device);
	conf_dialog = gtk_message_dialog_new (NULL, 0, GTK_MESSAGE_INFO,
                                         GTK_BUTTONS_OK,
					 "<b><big>%s</big></b>\n\n%s",
                                         _("Printer Detection Complete"), message);
        gtk_label_set_use_markup 
                       (GTK_LABEL(GTK_MESSAGE_DIALOG(conf_dialog)->label),TRUE);
	gtk_window_set_title (GTK_WINDOW(conf_dialog),_("Inkblot Setup"));
        response = gtk_dialog_run (GTK_DIALOG(conf_dialog));
	gtk_widget_destroy (conf_dialog);
        g_free(message);
	conf_dialog = NULL;
}
