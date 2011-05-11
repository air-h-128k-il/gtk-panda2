/* pandaTable 
 * Copyright 2011 NaCl
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
 *
 */

/*
 * Modified by the GTK+ Team and others 1997-1999.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/. 
 */

#ifndef __GTK_PANDA_TABLE_H__
#define __GTK_PANDA_TABLE_H__

#include <gtk/gtktreeview.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define GTK_PANDA_TYPE_TABLE			(gtk_panda_table_get_type ())
#define GTK_PANDA_TABLE(obj)			GTK_CHECK_CAST (obj, gtk_panda_table_get_type (), GtkPandaTable)
#define GTK_PANDA_TABLE_CLASS(klass)	GTK_CHECK_CLASS_CAST (klass, gtk_panda_table_get_type (), GtkPandaTableClass)
#define GTK_IS_PANDA_TABLE(obj)			GTK_CHECK_TYPE (obj, gtk_panda_table_get_type ())

#define GTK_PANDA_TABLE_MAX_COLS 100

enum GtkPandaTableRendererType {
  GTK_PANDA_TABLE_RENDERER_TEXT = 0,
  GTK_PANDA_TABLE_RENDERER_LABEL,
  GTK_PANDA_TABLE_RENDERER_CHECK,
  GTK_PANDA_TABLE_RENDERER_ICON,
  GTK_PANDA_TABLE_RENDERER_COLOR
};

typedef struct _GtkPandaTable		GtkPandaTable;
typedef struct _GtkPandaTableClass	GtkPandaTableClass;

/* you should access only the entry and list fields directly */
struct _GtkPandaTable {
  GtkTreeView tree;

  gint columns;
  gchar *types;
  gchar *titles;
  gchar *widths;
  int renderer_types[GTK_PANDA_TABLE_MAX_COLS];
  GType model_types[GTK_PANDA_TABLE_MAX_COLS];
  int color_column;
};

struct _GtkPandaTableClass {
  GtkTreeViewClass parent_class;

  void (*cell_edited) (GtkPandaTable *table,int row,int column,gchar *data);
};

GType gtk_panda_table_get_type (void);
GtkWidget *gtk_panda_table_new ();

void gtk_panda_table_set_columns(GtkPandaTable *table, 
  gint n_columns);
void gtk_panda_table_set_types(GtkPandaTable *table,
  const gchar *types);
void gtk_panda_table_set_titles (GtkPandaTable *table,
  const gchar *titles);
void gtk_panda_table_set_column_widths (GtkPandaTable *table,
  const gchar *widths);

void gtk_panda_table_moveto (GtkPandaTable *table,
  gint      row,
  gint      column,
  gfloat    row_align,
  gfloat    col_align);

gint gtk_panda_table_get_n_rows(GtkPandaTable *table);
gint gtk_panda_table_get_columns(GtkPandaTable *table);
gint gtk_panda_table_get_column_type(GtkPandaTable *table,gint column);

GtkVisibility gtk_panda_table_row_is_visible (GtkPandaTable *table, gint row);

void gtk_panda_table_clear (GtkPandaTable *table);
void gtk_panda_table_append  (GtkPandaTable *table,
  gchar       *text[]);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GTK_PANDA_TABLE_H__ */