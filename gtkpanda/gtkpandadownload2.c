/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GTK+ Team and others 1997-1999.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include "gtkpandaintl.h"
#include "gtkpandadownload2.h"

static void gtk_panda_download2_class_init    (GtkPandaDownload2Class *klass);
static void gtk_panda_download2_init          (GtkPandaDownload2      *download);

GType
gtk_panda_download2_get_type (void)
{
  static GType type = 0;

  if (!type)
    {
      static const GTypeInfo info =
      {
        sizeof (GtkPandaDownload2Class),
        NULL, /* base_init */
        NULL, /* base_finalize */
        (GClassInitFunc) gtk_panda_download2_class_init,
        NULL, /* class_finalize */
        NULL, /* class_data */
        sizeof (GtkPandaDownload2),
        0,
        (GInstanceInitFunc) gtk_panda_download2_init
      };

      type = g_type_register_static( GTK_TYPE_WIDGET,
                                     "GtkPandaDownload2",
                                     &info,
                                     0);
    }

  return type;
}

static void
gtk_panda_download2_class_init (GtkPandaDownload2Class *class)
{
}

static void
gtk_panda_download2_init (GtkPandaDownload2 *download)
{
  gtk_widget_set_has_window(GTK_WIDGET(download),FALSE);
}

GtkWidget*
gtk_panda_download2_new (void)
{
  GtkWidget *download2;

  download2 = g_object_new (GTK_PANDA_TYPE_DOWNLOAD2,NULL);
  return download2;
}
