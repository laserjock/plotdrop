
/* PlotDrop is free software, released under the GNU General Public License
 * See the COPYING file for licensing details.
 *
 * Copyright 2005 John Spray
 */


#include <gtk/gtk.h>

#include "plotdata.h"

void droplist_init_gui ();
void droplist_add_file (char const *filename);
void droplist_plot ();
void droplist_handle_plot ();
void droplist_export ();
gint droplist_exit (GtkWidget *widget, gpointer data);

void droplist_update_limit_sensitivities ();

void droplist_drag_data_received (GtkWidget *widget,
                                  GdkDragContext *drag_context,
                                  gint x,
                                  gint y,
                                  GtkSelectionData *data,
                                  guint info,
                                  guint time,
                                  gpointer user_data);

void droplist_title_edited (GtkCellRendererText *cell,
                           gchar               *path_string,
                           gchar               *new_text,
                           gpointer             user_data);

void droplist_row_activated (GtkTreeView *treeview,
                             GtkTreePath *arg1,
                             GtkTreeViewColumn *arg2,
                             gpointer user_data);

void droplist_series_edited (GtkCellRendererText *cell,
                           gchar               *path_string,
                           gchar               *new_text,
                           gpointer             user_data);

void droplist_clear ();
void droplist_selection_changed (GtkTreeSelection *sel, gpointer data);
void droplist_delete ();
void droplist_rename ();
void droplist_extra_help (GtkWidget *widg, gpointer data);
void droplist_no_gnuplot ();
gboolean droplist_really_quit ();
void droplist_update_sensitivities ();
void droplist_about ();
void droplist_add_file_UI ();

void droplist_getdata (plotdata *data);
void droplist_freedata (plotdata *data);
