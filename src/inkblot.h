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
#include <gnome.h>

#ifndef INKBLOT_H
#define INKBLOT_H

/* Inkblot - GConf Key Names */
#define INKBLOT_PORT_KEY "/apps/inkblot/port"
#define INKBLOT_BUS_KEY "/apps/inkblot/bus"
#define INKBLOT_CONFIG_KEY "/apps/inkblot/first-run"

/* Inkblot - Types and Enums */
typedef enum {
	INKBLOT_ERROR_OK,
	INKBLOT_ERROR_GLADE,
	INKBLOT_ERROR_NO_CONFIG,
	INKBLOT_ERROR_NO_PRINTER,
	INKBLOT_ERROR_INVALID_RESPONSE,
	INKBLOT_ERROR_UNKNOWN
} InkblotError;

typedef enum {
	INKBLOT_OK,
	INKBLOT_WARN,
	INKBLOT_LOW,
	INKBLOT_EMPTY,
	INKBLOT_ERROR,
	INKBLOT_QUERY
} InkblotStatus;

typedef struct {
	gchar *model;
	gint bus;
	gint port;
} InkblotPrinter;

/* Inkblot - Function Definitions */
GdkPixbuf*	inkblot_tray_icon_display ( InkblotStatus );
gboolean	inkblot_tray_menu (GtkWidget *, GdkEventButton *, gpointer);
gboolean 	inkblot_check_status_event ( gpointer );
void 		inkblot_dialog_response (GtkWidget *, gint, gpointer);
gint 		inkblot_scan_devices ( void );
InkblotStatus 	inkblot_get_overall_status ( void );
void 		inkblot_tray_icon_clicked ( GtkWidget *, GdkEventButton *, 
					    gpointer );
void 		inkblot_toggle_dialog ( void );
static gint 	inkblot_save_session (GnomeClient *, gint, GnomeRestartStyle, 
				      gint, GnomeInteractStyle,gint,gpointer);
gint		inkblot_error_dialog (InkblotError);

/* Inkblot: Globals */
guint timer_id;

#endif
