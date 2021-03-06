 /* Copyright 1997 Paolo Molaro
 * Copyright 2001 AXE, Inc.
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
 * Based on gtk/gtktreeview.c
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

#include <glib-object.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "gtkpandaintl.h"
#include "gtkpandaclist.h"
#include "pandamarshal.h"

enum {
PROP_0,
PROP_SHOW_TITLES,
PROP_COLUMNS,
PROP_COLUMN_WIDTHS,
PROP_SELECTION_MODE
};

enum
{
  SELECT_ROW,
  UNSELECT_ROW,
  LAST_SIGNAL 
}; 

#define GTK_PANDA_TYPE_SELECTION_MODE (gtk_panda_type_selection_mode_get_type())
static GType
gtk_panda_type_selection_mode_get_type (void)
{
  static GType type = 0;
  static const GEnumValue data[] = {
    {GTK_SELECTION_SINGLE, "GTK_SELECTION_SINGLE", "single"},
    {GTK_SELECTION_MULTIPLE, "GTK_SELECTION_MULTIPLE", "multi"},
    {0, NULL, NULL},
  };

  if (!type) {
    type =
        g_enum_register_static ("GtkPandaTypeSelectionMode", data);
  }
  return type;
}

static GtkWidgetClass *parent_class = NULL;
static guint clist_signals [LAST_SIGNAL] = { 0 };

static void gtk_panda_clist_class_init (GtkPandaCListClass *klass);
static void gtk_panda_clist_init (GtkPandaCList *clist);

static gboolean gtk_panda_clist_button_press (GtkWidget *widget, GdkEventButton *event);
static gboolean gtk_panda_clist_key_press (GtkWidget *widget, GdkEventKey *event);
static void selection_changed (GtkTreeSelection *selection, gpointer user_data);

static void  gtk_panda_clist_set_property(
  GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void  gtk_panda_clist_get_property(
  GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);

static void
gtk_panda_clist_class_init ( GtkPandaCListClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class;

  widget_class = (GtkWidgetClass *) klass;
  widget_class->button_press_event = gtk_panda_clist_button_press;
  widget_class->key_press_event = gtk_panda_clist_key_press;

  parent_class = g_type_class_ref(GTK_TYPE_TREE_VIEW);

  clist_signals[SELECT_ROW] =
  g_signal_new ("select_row",
        G_TYPE_FROM_CLASS (gobject_class),
        G_SIGNAL_RUN_FIRST,
        G_STRUCT_OFFSET (GtkPandaCListClass, select_row),
        NULL, NULL,
        panda_marshal_VOID__INT_INT,
        G_TYPE_NONE, 2,
        G_TYPE_INT,
        G_TYPE_INT);

  clist_signals[UNSELECT_ROW] =
  g_signal_new ("unselect_row",
        G_TYPE_FROM_CLASS (gobject_class),
        G_SIGNAL_RUN_FIRST,
        G_STRUCT_OFFSET (GtkPandaCListClass, unselect_row),
        NULL, NULL,
        panda_marshal_VOID__INT_INT,
        G_TYPE_NONE, 2,
        G_TYPE_INT,
        G_TYPE_INT);

  gobject_class->set_property = gtk_panda_clist_set_property; 
  gobject_class->get_property = gtk_panda_clist_get_property; 

  g_object_class_install_property (gobject_class,
    PROP_SHOW_TITLES,
    g_param_spec_boolean ("show_titles",
                          _("Show header title"),
                          _("Whether show header title"),
                          TRUE,
                          G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
    PROP_COLUMNS,
    g_param_spec_int ("columns",
                          _("Number of columns"),
                          _("Number of columns"),
                          0,
                          100,
                          0,
                          G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
    PROP_COLUMN_WIDTHS,
    g_param_spec_string ("column_widths",
                          _("The list of column width"),
                          _("The list of column width(comma separated string)"),
                          "80,80,80,80,80,80,80,80,80,80,80,80,80,80,80",
                          G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
    PROP_SELECTION_MODE,
    g_param_spec_enum ("selection_mode",
                      _("Selection Mode"),
                      _("The mode of selection"),
                      GTK_PANDA_TYPE_SELECTION_MODE,
                      GTK_SELECTION_SINGLE,
                      G_PARAM_READWRITE));
}

static void
gtk_panda_clist_init ( GtkPandaCList * clist)
{
  GtkTreeSelection *selection;
  
  clist->show_titles = TRUE;
  clist->column_widths = g_strdup("");
  clist->selection_mode = GTK_SELECTION_SINGLE;
  clist->prev_selected_num = 0;
  clist->nrows = 0;
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(clist));
  gtk_panda_clist_set_columns(clist,0);
  g_signal_connect (G_OBJECT(selection), "changed",
    G_CALLBACK(selection_changed), (gpointer)clist);
  gtk_tree_view_set_enable_search(GTK_TREE_VIEW(clist), FALSE );
  gtk_tree_view_set_rubber_banding(GTK_TREE_VIEW(clist), FALSE );
  gtk_widget_set_can_focus(GTK_WIDGET(clist),TRUE);
}

/* global function  */

GType
gtk_panda_clist_get_type (void)
{
  static GType clist_type = 0;

  if (!clist_type)
    {
      static const GTypeInfo clist_info =
      {
        sizeof (GtkPandaCListClass),
        NULL, /* base_init */
        NULL, /* base_finalize */
        (GClassInitFunc) gtk_panda_clist_class_init,           
        NULL, /* class_finalize */
        NULL, /* class_data */
        sizeof (GtkPandaCList),
        0,
        (GInstanceInitFunc) gtk_panda_clist_init
      };

      clist_type = g_type_register_static( GTK_TYPE_TREE_VIEW,
                                           "GtkPandaCList",
                                           &clist_info,
                                           0);
    }
  return clist_type;
}

GtkWidget *
gtk_panda_clist_new ()
{
  return (GtkWidget*)g_object_new(GTK_PANDA_TYPE_CLIST, NULL);
}

#define CLIST_MAX_COLUMNS 100

static void
_gtk_panda_clist_set_columns (
  GtkPandaCList *clist,
  gint new_columns) 
{
  GtkListStore *store;
  GList *list;
  GType types[CLIST_MAX_COLUMNS];
  int i,j;

  g_return_if_fail(clist != NULL);
  g_return_if_fail(new_columns >= 0);

  if (new_columns <=0 ) {
    new_columns = 1;
  }
  if (new_columns > CLIST_MAX_COLUMNS) {
    new_columns = CLIST_MAX_COLUMNS;
  }

  list = gtk_tree_view_get_columns(GTK_TREE_VIEW(clist));
  for (i=0; i < g_list_length(list); i++) {
    gtk_tree_view_remove_column(GTK_TREE_VIEW(clist),g_list_nth_data(list,i));
  }
  gtk_tree_view_set_model(GTK_TREE_VIEW(clist), NULL);
  g_list_free(list);

  clist->columns = new_columns;

  for (i=0; i < new_columns; i++) {
    gtk_tree_view_insert_column_with_attributes (
      GTK_TREE_VIEW(clist),
      -1,
      "",
      gtk_cell_renderer_text_new (),
      "text", i,
      "foreground",new_columns,
      "cell-background",new_columns+1,
      NULL);
  }
  j = new_columns + 2;
  for (i = 0; i < j; i++) {
    types[i] =  G_TYPE_STRING;
  }
  store = gtk_list_store_newv(j,types);
  gtk_tree_view_set_model(GTK_TREE_VIEW(clist), GTK_TREE_MODEL(store));
}

static void
monsia3_gtk_panda_clist_set_columns (
  GtkPandaCList *clist,
  gint new_columns) 
{
  GtkTreeViewColumn *column;
  GtkListStore *store;
  GType *types;
  int i;
  int columns;

  g_return_if_fail(clist != NULL);
  g_return_if_fail(new_columns >= 0);

  if (new_columns == 0) {
    GList *list;
    list = gtk_tree_view_get_columns(GTK_TREE_VIEW(clist));
    for (i=0; i < g_list_length(list); i++) {
      gtk_tree_view_remove_column(GTK_TREE_VIEW(clist),g_list_nth_data(list,i));
    }
    gtk_tree_view_set_model(GTK_TREE_VIEW(clist), NULL);
    g_list_free(list);
    clist->columns = 0;
    return;
  }

  columns = gtk_panda_clist_get_columns(clist);
  if (columns == new_columns) {
    return;
  } else if (columns > new_columns){
    for (i = columns - 1; i >= new_columns; i-- ){ 
      column = gtk_tree_view_get_column(GTK_TREE_VIEW(clist), i);
      if (column != NULL) {
        gtk_tree_view_remove_column(GTK_TREE_VIEW(clist),column);
      }
    }
  } else {
    for (i = columns ; i < new_columns; i++ ){ 
      gtk_tree_view_insert_column_with_attributes (
        GTK_TREE_VIEW(clist),
        -1,
        "",
        gtk_cell_renderer_text_new (),
        "text", i,
        NULL);
    }
  }

  clist->columns = new_columns;
  types = g_new0(GType, new_columns);
  for (i = 0; i < new_columns; i++) {
    types[i] =  G_TYPE_STRING;
  }
  store = gtk_list_store_newv(new_columns, types);
  gtk_tree_view_set_model(GTK_TREE_VIEW(clist), GTK_TREE_MODEL(store));
}

void
gtk_panda_clist_set_columns (
  GtkPandaCList *clist,
  gint new_columns) 
{
  if (!getenv("MONSIA3_GTKPANDA")) {
    _gtk_panda_clist_set_columns (clist,new_columns);
  } else {
    monsia3_gtk_panda_clist_set_columns (clist,new_columns);
  }
}

void 
gtk_panda_clist_set_column_widths (
  GtkPandaCList *clist,
  const gchar *new_column_widths)
{
  gchar **widths;
  int i;

  g_return_if_fail(clist != NULL);
  g_return_if_fail(new_column_widths != NULL);

  if (clist->column_widths != NULL) {
    g_free(clist->column_widths);
  }
  clist->column_widths = g_strdup(new_column_widths);

  widths = g_strsplit(new_column_widths,",",100);
  for(i = 0; widths[i] != NULL; i++) {
    gtk_panda_clist_set_column_width(clist,i,atoi(widths[i])); 
  }
  g_strfreev(widths);
}

void 
gtk_panda_clist_set_column_width (
  GtkPandaCList *clist,
  gint      column,
  gint      width)
{
  GtkTreeViewColumn *col;

  g_return_if_fail(clist != NULL);
  col = gtk_tree_view_get_column(GTK_TREE_VIEW(clist), column);
  if (col == NULL) {
    return;
  }
  gtk_tree_view_column_set_sizing(col, 
    GTK_TREE_VIEW_COLUMN_AUTOSIZE);
  gtk_tree_view_column_set_min_width(col, width);
  gtk_tree_view_column_set_resizable(col, TRUE);
}

void
gtk_panda_clist_set_rows(
  GtkPandaCList *clist,
  int new)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  int i;
  
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(clist));
  if (new > clist->nrows) {
    for(i=clist->nrows;i<new;i++) {
      gtk_list_store_append(GTK_LIST_STORE(model),&iter);
    }
  } else if (new < clist->nrows) {
    if (!gtk_tree_model_get_iter_first(model, &iter)) {
      return;
    }
    for(i=0;i<clist->nrows;i++) {
      if (i < new) {
        if (!gtk_tree_model_iter_next(model,&iter)) {
          return;
        }
      } else {
        gtk_list_store_remove(GTK_LIST_STORE(model),&iter);
      }
    }
  }
  clist->nrows = new;
}

gint
gtk_panda_clist_get_rows(
  GtkPandaCList *clist)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  gint nrows;

  g_return_val_if_fail (clist != NULL, 0);
  g_return_val_if_fail (GTK_IS_PANDA_CLIST (clist), 0);

  nrows = 0; 
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(clist));
  if (gtk_tree_model_get_iter_first(model, &iter)) {
    nrows = 1;
    while(gtk_tree_model_iter_next(model, &iter)) {
      nrows++;
    }
  }
  return nrows;
}

void
gtk_panda_clist_set_row(
  GtkPandaCList *clist,
  int row,
  gchar **rdata)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  gchar path[16];
  int i;

  g_return_if_fail(clist != NULL);
  g_return_if_fail(GTK_IS_PANDA_CLIST(clist));

  sprintf(path,"%d",row);
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(clist));
  if (gtk_tree_model_get_iter_from_string(model,&iter,path)) {
    for(i=0;i<gtk_panda_clist_get_columns(clist)-2;i++) {
      gtk_list_store_set(GTK_LIST_STORE(model),&iter,i,rdata[i],-1);
    }
  }
}

gint
gtk_panda_clist_get_columns(
  GtkPandaCList *clist)
{
  GtkTreeModel *model;

  g_return_val_if_fail(clist != NULL, 0);
  g_return_val_if_fail(GTK_IS_PANDA_CLIST(clist), 0);

  model = gtk_tree_view_get_model(GTK_TREE_VIEW(clist));
  if (model == NULL) {
    return 0;
  }
  return gtk_tree_model_get_n_columns(model);
}

void 
gtk_panda_clist_moveto (
  GtkPandaCList *clist,
  gint      row,
  gint      column,
  gfloat    row_align,
  gfloat    col_align)
{
  GtkTreePath *path;
  GtkTreeViewColumn *col;

  g_return_if_fail (clist != NULL);
  g_return_if_fail (GTK_IS_PANDA_CLIST (clist));

  path = gtk_tree_path_new_from_indices(row, -1);
  col = gtk_tree_view_get_column(GTK_TREE_VIEW(clist), column);

  gtk_tree_view_scroll_to_cell(
	GTK_TREE_VIEW(clist), path, col, TRUE, row_align, col_align);
  gtk_tree_path_free(path);
}

gboolean
gtk_panda_clist_row_is_selected (GtkPandaCList *clist,
  gint      row)
{
  GtkTreeSelection *select;
  GtkTreePath *path;

  select = gtk_tree_view_get_selection (GTK_TREE_VIEW (clist));
  path = gtk_tree_path_new_from_indices(row, -1);
  return gtk_tree_selection_path_is_selected(select, path);
}

gboolean
gtk_panda_clist_row_is_visible (GtkPandaCList *clist,
  gint      row)
{
  GtkTreePath *start;
  GtkTreePath *end;
  gint *indices;
  gboolean ret; 

  ret = FALSE;
  if (gtk_tree_view_get_visible_range(GTK_TREE_VIEW(clist), &start, &end)) {
    indices = gtk_tree_path_get_indices(start);
    if (indices != NULL && indices[0] <= row) {
      ret = TRUE;
    }
    gtk_tree_path_free(start);
    gtk_tree_path_free(end);
  }

  return ret;
}

GtkSelectionMode
gtk_panda_clist_get_selection_mode (
  GtkPandaCList *clist)
{
  GtkTreeSelection *select;

  g_return_val_if_fail (clist != NULL,GTK_SELECTION_SINGLE);
  g_return_val_if_fail (GTK_IS_PANDA_CLIST (clist),GTK_SELECTION_SINGLE);

  select = gtk_tree_view_get_selection (GTK_TREE_VIEW (clist));
  clist->selection_mode = gtk_tree_selection_get_mode (select);
  return clist->selection_mode;
}

void 
gtk_panda_clist_set_selection_mode (
  GtkPandaCList *clist,
  GtkSelectionMode mode)
{
  GtkTreeSelection *select;

  g_return_if_fail (clist != NULL);
  g_return_if_fail (GTK_IS_PANDA_CLIST (clist));

  select = gtk_tree_view_get_selection (GTK_TREE_VIEW (clist));
  gtk_tree_selection_set_mode (select, mode);
  clist->selection_mode = mode;
}

void 
gtk_panda_clist_select_row (
  GtkPandaCList *clist,
  gint      row,
  gint      column)
{
  GtkTreeSelection *select;
  GtkTreePath *path;

  g_return_if_fail (clist != NULL);
  g_return_if_fail (GTK_IS_PANDA_CLIST (clist));

  select = gtk_tree_view_get_selection (GTK_TREE_VIEW (clist));
  path = gtk_tree_path_new_from_indices(row, -1);
  gtk_tree_selection_select_path(select, path);
}

void 
gtk_panda_clist_unselect_row (
  GtkPandaCList *clist,
  gint      row,
  gint      column)
{
  GtkTreeSelection *select;
  GtkTreePath *path;

  g_return_if_fail (clist != NULL);
  g_return_if_fail (GTK_IS_PANDA_CLIST (clist));

  select = gtk_tree_view_get_selection(GTK_TREE_VIEW(clist));
  path = gtk_tree_path_new_from_indices(row, -1);
  gtk_tree_selection_unselect_path(select, path);
}

void 
gtk_panda_clist_set_show_titles (
  GtkPandaCList *clist,
  gboolean show_titles)
{
  g_return_if_fail (clist != NULL);
  g_return_if_fail (GTK_IS_PANDA_CLIST (clist));

  clist->show_titles = show_titles;
  gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(clist),show_titles);
}

void
gtk_panda_clist_set_fgcolor(
  GtkPandaCList *clist,
  int row,
  gchar *color)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  gchar path[16];

  g_return_if_fail(clist != NULL);
  g_return_if_fail(GTK_IS_PANDA_CLIST(clist));

  sprintf(path,"%d",row);
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(clist));
  gtk_tree_model_get_iter_from_string(model,&iter,path);

  if (color == NULL || strlen(color) == 0) {
    gtk_list_store_set(GTK_LIST_STORE(model),&iter,clist->columns,"black",-1);
  } else {
    gtk_list_store_set(GTK_LIST_STORE(model),&iter,clist->columns,color,-1);
  }
}

void
gtk_panda_clist_set_bgcolor(
  GtkPandaCList *clist,
  int row,
  gchar *color)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  gchar path[16];

  g_return_if_fail(clist != NULL);
  g_return_if_fail(GTK_IS_PANDA_CLIST(clist));

  sprintf(path,"%d",row);
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(clist));
  gtk_tree_model_get_iter_from_string(model,&iter,path);

  if (color == NULL || strlen(color) == 0) {
    gtk_list_store_set(GTK_LIST_STORE(model),&iter,clist->columns+1,"white",-1);
  } else {
    gtk_list_store_set(GTK_LIST_STORE(model),&iter,clist->columns+1,color,-1);
  }
}

/* static function  */

static gboolean
gtk_panda_clist_button_press(
  GtkWidget *widget,
  GdkEventButton *event)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  GtkTreeSelection *select;
  GtkTreePath *path;
  gboolean get_path_result;
  gint bx, by;

  if (event->button != 1) {
    return FALSE;
  }

  model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
  if (!gtk_tree_model_get_iter_first(model, &iter))
    return FALSE;

  if (event->window != 
    gtk_tree_view_get_bin_window(GTK_TREE_VIEW(widget))) {
    bx = -1;
    by = -1;
  } else {
    bx = (gint)event->x;
    by = (gint)event->y;
  }

  get_path_result = gtk_tree_view_get_path_at_pos(
    GTK_TREE_VIEW(widget),
    bx, 
    by,
    &path,
    NULL, 
    NULL, 
    NULL);

  if (get_path_result) {
    select = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
    if (gtk_tree_selection_path_is_selected(select, path)) {
      gtk_tree_selection_unselect_path(select, path);
    } else  {
      gtk_tree_selection_select_path(select, path);
    }
    gtk_tree_path_free(path);
    return FALSE;
  } else {
    return ((GtkWidgetClass *)parent_class)->button_press_event(widget, event);
  }
}

static gboolean
gtk_panda_clist_key_press(
  GtkWidget   *widget,
  GdkEventKey *event)
{
  GtkTreeView *tree;
  GtkTreeViewColumn *column;
  GtkTreePath *path;
  GtkTreeSelection *select;

  g_return_val_if_fail (widget != NULL, TRUE);
  g_return_val_if_fail (GTK_IS_TREE_VIEW (widget), TRUE);

  tree = GTK_TREE_VIEW(widget);

  switch (event->keyval) {
  case GDK_KEY_space:
  case GDK_KEY_Return:
  case GDK_KEY_KP_Space:
  case GDK_KEY_KP_Enter:
    gtk_tree_view_get_cursor(tree, &path, &column);
    if (path != NULL) {
      select = gtk_tree_view_get_selection(tree);
      if (gtk_tree_selection_get_mode(select) == GTK_SELECTION_MULTIPLE) {
        if (gtk_tree_selection_path_is_selected(select, path)) {
          gtk_tree_selection_unselect_path(select, path);
        } else {
          gtk_tree_selection_select_path(select, path);
        }
      } else {
        gtk_tree_selection_select_path(select, path);
      }
    }
    break;
  case GDK_KEY_Tab:
  case GDK_KEY_ISO_Left_Tab:
    if (event->state & GDK_SHIFT_MASK) {
      return gtk_widget_child_focus(widget,GTK_DIR_TAB_BACKWARD);
    } else {
      return gtk_widget_child_focus(widget,GTK_DIR_TAB_FORWARD);
    }
    break;
  default:
    event->state |= GDK_CONTROL_MASK;
    if (GTK_WIDGET_CLASS (parent_class)->key_press_event &&
        GTK_WIDGET_CLASS (parent_class)->key_press_event (widget, event))
    break;
  }
  return TRUE;
}

static void 
selection_changed(
  GtkTreeSelection *selection,
  gpointer user_data)
{
  GtkPandaCList *clist;
  gboolean emit_select;
  int num;
  GList * selected;

  emit_select = FALSE;
  clist = (GtkPandaCList *)user_data;
  selected = gtk_tree_selection_get_selected_rows(selection,NULL);

  if (selected != NULL) {
    num = g_list_length(selected);
    if (clist->prev_selected_num <= num) {
      emit_select = TRUE;
    }
    clist->prev_selected_num = num;
    g_list_free(selected);
  }
  if (emit_select) {
    // now don't return ROW, COLUMN.
    g_signal_emit (clist, clist_signals[SELECT_ROW], 0, 0, 0);
  } else {
    // now don't return ROW, COLUMN.
    g_signal_emit (clist, clist_signals[UNSELECT_ROW], 0, 0, 0);
  }
}

static 
void 
gtk_panda_clist_set_property (
  GObject         *object,
  guint            prop_id,
  const GValue    *value,
  GParamSpec      *pspec)
{
  GtkPandaCList *clist;

  g_return_if_fail(GTK_IS_PANDA_CLIST(object));
  clist = GTK_PANDA_CLIST (object);

  switch (prop_id)
    {
    case PROP_SHOW_TITLES:
      gtk_panda_clist_set_show_titles(clist,g_value_get_boolean(value));
      break;
    case PROP_COLUMNS:
      gtk_panda_clist_set_columns(clist,g_value_get_int(value));
	  break;
    case PROP_COLUMN_WIDTHS:
      gtk_panda_clist_set_column_widths(clist,g_value_get_string(value));
      break;
    case PROP_SELECTION_MODE:
      gtk_panda_clist_set_selection_mode(clist,g_value_get_enum(value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void 
gtk_panda_clist_get_property (
  GObject         *object,
  guint            prop_id,
  GValue          *value,
  GParamSpec      *pspec)
{
  GtkPandaCList *clist;

  g_return_if_fail(GTK_IS_PANDA_CLIST(object));
  clist = GTK_PANDA_CLIST (object);

  switch (prop_id)
    {
    case PROP_SHOW_TITLES:
      g_value_set_boolean (value, clist->show_titles);
      break;
    case PROP_COLUMNS:
      g_value_set_int (value, clist->columns);
      break;
    case PROP_COLUMN_WIDTHS:
      g_value_set_string (value, g_strdup(clist->column_widths));
      break;
    case PROP_SELECTION_MODE:
      g_value_set_enum (value, gtk_panda_clist_get_selection_mode(clist));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}
