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
#include <gtk/gtk.h>
#include "inkblot.h"

void inkblot_rescan_devices ( GtkWidget *widget, gpointer data ) {
	inkblot_scan_devices();
}

void inkblot_about_dialog ( GtkWidget *widget, gpointer data ) {
	GdkPixbuf *pixbuf;
        static GtkWidget *dialog = NULL;

        /* Don't create more than one about box */
        if (dialog != NULL) {
                g_assert (GTK_WIDGET_REALIZED (dialog));
                gdk_window_show (dialog->window);
                gdk_window_raise (dialog->window);
        } else {
		const gchar *authors[]={"Mike Newman <mikegtn@gnome.org>",
					"Thierry Merle <thierry.merle@free.fr>",
					NULL};
		pixbuf = gdk_pixbuf_new_from_file (PACKAGE_DATA_DIR 
				"/inkblot/pixmaps/printer_multi_48.png", NULL);
		dialog = gnome_about_new (
                        "Inkblot", VERSION,
                        "© 2004-2007 Mike Newman",
                        _("Reports on your printer ink levels.\n"),
                        authors,
                        NULL,
                        NULL, pixbuf);

                g_signal_connect (G_OBJECT (dialog), "destroy",
                        G_CALLBACK (gtk_widget_destroyed), &dialog);

                g_object_unref (G_OBJECT(pixbuf));
                gtk_widget_show (dialog);
        }
}

void inkblot_exit ( GtkWidget *widget, gpointer data ) {
	exit (0);
}

gboolean inkblot_tray_menu (GtkWidget *widget, GdkEventButton *event, gpointer data) {

        GtkWidget *status_menu;
        GtkWidget *item, *image;

        status_menu = gtk_menu_new();

	item = gtk_image_menu_item_new_with_label (_("Rescan Printer Devices"));
        image = gtk_image_new_from_stock ("gtk-refresh",GTK_ICON_SIZE_MENU);
        gtk_container_add (GTK_CONTAINER (status_menu), item);
        gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item), image);
        g_signal_connect (G_OBJECT (item), "activate",
                          G_CALLBACK (inkblot_rescan_devices),
                          NULL);


        item = gtk_image_menu_item_new_with_label (_("About Inkblot"));
        image = gtk_image_new_from_stock ("gnome-stock-about",GTK_ICON_SIZE_MENU);
        gtk_container_add (GTK_CONTAINER (status_menu), item);
        gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item), image);
        g_signal_connect (G_OBJECT (item), "activate",
                          G_CALLBACK (inkblot_about_dialog),
                          NULL);

        item = gtk_image_menu_item_new_with_label (_("Exit Inkblot"));
        image = gtk_image_new_from_stock ("gtk-quit",
					   GTK_ICON_SIZE_MENU);
	gtk_container_add (GTK_CONTAINER (status_menu), item);
        gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item), image);
        g_signal_connect (G_OBJECT (item), "activate",
                          G_CALLBACK (inkblot_exit),
                          NULL);

        gtk_widget_show_all (status_menu);

        gtk_menu_popup (GTK_MENU(status_menu), NULL, NULL,
	NULL, NULL,event->button, event->time);

        return TRUE;
}

