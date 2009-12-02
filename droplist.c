
/* PlotDrop is free software, released under the GNU General Public License
 * See the COPYING file for licensing details.
 *
 * Copyright 2005 John Spray
 */


// For exit ()
#include <stdlib.h>

#include <gio/gio.h>
#include <glib.h>
#include <glade/glade.h>
#include <string.h>
#include "droplist.h"
#include "gnuplot.h"

static struct {
	GtkListStore *store;
	GtkWidget *view;
	GtkWidget *plotmenu;
	GtkWidget *plotbutton;
	GtkWidget *clearmenu;
	GtkWidget *deletemenu;
	GtkWidget *clearbutton;
	GtkWidget *deletebutton;
	GtkWidget *exportbutton;
	GtkWidget *renamebutton;
	GtkWidget *stylecombo;
	GtkWidget *zeroaxischeck;
	GtkWidget *errorbarscheck;
	GtkWidget *gridcheck;
	GtkWidget *enhancedmodecheck;
	GtkWidget *logscaleycheck;
	GtkWidget *logscalexcheck;
	GtkWidget *titleentry;
	GtkWidget *xlabelentry;
	GtkWidget *ylabelentry;
	GtkAdjustment *xmin;
	GtkAdjustment *xmax;
	GtkAdjustment *ymin;
	GtkAdjustment *ymax;
	GtkWidget *xminspin;
	GtkWidget *xmaxspin;
	GtkWidget *yminspin;
	GtkWidget *ymaxspin;
	GtkWidget *xminset;
	GtkWidget *xmaxset;
	GtkWidget *yminset;
	GtkWidget *ymaxset;
	GtkWidget *extra;

	GtkWidget *dropherebox;
	GtkWidget *filebox;

	gchar *exportpath;
} droplist;

enum
{
  COL_FILENAME = 0,
  COL_PATH,
  COL_TITLE,
  COL_SERIES,
  NUM_COLS
};

enum
{
	DRAG_TEXT,
	DRAG_URILIST
};

static GtkTargetEntry drag_types[] = {
		//{"text/*", 0, DRAG_TEXT },
		//{"STRING", 0, DRAG_TEXT },
		{"text/uri-list", 0, DRAG_URILIST }
};
static int const n_drag_types = sizeof (drag_types) / sizeof (drag_types[0]);

void droplist_init_gui ()
{
	GladeXML *xml;

	xml = glade_xml_new ("droplist.glade", NULL, NULL);
	if (!xml) {
		xml = glade_xml_new (DATADIR "/droplist.glade", NULL, NULL);
		if (!xml)
			g_error ("Couldn't load glade file!\n");
	}

	GtkWidget *window;
	window = glade_xml_get_widget (xml, "DropList");
	g_signal_connect (G_OBJECT (window), "delete-event",
		G_CALLBACK (droplist_exit), NULL);

	GdkPixbuf *icon = gdk_pixbuf_new_from_file ("plotdrop.png", NULL);
	if (icon) {
		gtk_window_set_icon (GTK_WINDOW (window), icon);
		g_object_unref (G_OBJECT (icon));
	} else {
		icon = gdk_pixbuf_new_from_file (DATADIR "/plotdrop.png", NULL);
		if (icon) {
			gtk_window_set_icon (GTK_WINDOW (window), icon);
			g_object_unref (G_OBJECT (icon));
		}
	}

	GtkListStore *liststore = gtk_list_store_new (NUM_COLS,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	droplist.store = liststore;

	GtkCellRenderer *cellRO = gtk_cell_renderer_text_new ();
	//g_object_set (G_OBJECT (cellRO), "ellipsize", TRUE, NULL);
	GtkCellRenderer *cell_title = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (cell_title), "editable", TRUE, NULL);
	GtkCellRenderer *cell_series = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (cell_series), "editable", TRUE, NULL);
	//g_object_set (G_OBJECT (cellRW), "ellipsize", TRUE, NULL);
	g_signal_connect (G_OBJECT (cell_title), "edited",
		G_CALLBACK (droplist_title_edited), NULL);
	g_signal_connect (G_OBJECT (cell_series), "edited",
		G_CALLBACK (droplist_series_edited), NULL);

	GtkWidget *treeview;
	treeview = glade_xml_get_widget (xml, "FileView");
	gtk_tree_view_set_model (GTK_TREE_VIEW (treeview),
	                         GTK_TREE_MODEL (liststore));

	gtk_tree_view_insert_column_with_attributes (
		GTK_TREE_VIEW (treeview),
		-1,
		"Filename",
		cellRO,
		"text", COL_FILENAME,
		NULL);
	gtk_tree_view_column_set_resizable (
		gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), 0), TRUE);
	//gtk_tree_view_column_set_sizing (gtk_tree_view_get_column (treeview, 0), GTK_TREE_VIEW_COLUMN_AUTOSIZE);


	gtk_tree_view_insert_column_with_attributes (
		GTK_TREE_VIEW (treeview),
		-1,
		"Display title",
		cell_title,
		"markup", COL_TITLE,
		NULL);
	gtk_tree_view_column_set_resizable (
		gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), 1), TRUE);
	//gtk_tree_view_column_set_sizing (gtk_tree_view_get_column (treeview, 1), GTK_TREE_VIEW_COLUMN_AUTOSIZE);

	gtk_tree_view_insert_column_with_attributes (
		GTK_TREE_VIEW (treeview),
		-1,
		"Series",
		cell_series,
		"text", COL_SERIES,
		NULL);
	gtk_tree_view_column_set_resizable (
		gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), 2), TRUE);

	droplist.view = treeview;

	gtk_drag_dest_set (
		treeview,
		GTK_DEST_DEFAULT_ALL,
		drag_types,
		n_drag_types,
		GDK_ACTION_COPY | GDK_ACTION_LINK | GDK_ACTION_MOVE);

	g_signal_connect (G_OBJECT (treeview), "drag-data-received",
		G_CALLBACK (droplist_drag_data_received), NULL);
	g_signal_connect (G_OBJECT (treeview), "row-activated",
		G_CALLBACK (droplist_row_activated), NULL);

	GtkTreeSelection *sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
	g_signal_connect (G_OBJECT (sel), "changed",
		G_CALLBACK (droplist_selection_changed), NULL);

	droplist.dropherebox = glade_xml_get_widget (xml, "DropFilesHereBox");
	droplist.filebox = glade_xml_get_widget (xml, "FileListBox");
	GtkWidget *dropfileshere = glade_xml_get_widget (xml, "DropFilesHere");

	gtk_drag_dest_set (
		dropfileshere,
		GTK_DEST_DEFAULT_ALL,
		drag_types,
		n_drag_types,
		GDK_ACTION_COPY | GDK_ACTION_LINK | GDK_ACTION_MOVE);
	g_signal_connect (G_OBJECT (dropfileshere), "drag-data-received",
		G_CALLBACK (droplist_drag_data_received), NULL);

	droplist.stylecombo = glade_xml_get_widget (xml, "Style");
	gtk_combo_box_set_active (GTK_COMBO_BOX (droplist.stylecombo), STYLE_POINTS);

	droplist.zeroaxischeck = glade_xml_get_widget (xml, "ShowZeroAxes");
	droplist.errorbarscheck = glade_xml_get_widget (xml, "ShowErrorbars");
	droplist.gridcheck = glade_xml_get_widget (xml, "ShowGrid");
	droplist.enhancedmodecheck = glade_xml_get_widget (xml, "EnableSuperSubScripts");
	droplist.logscaleycheck = glade_xml_get_widget (xml, "EnableLogscaleY");
	droplist.logscalexcheck = glade_xml_get_widget (xml, "EnableLogscaleX");
	

	droplist.xlabelentry = glade_xml_get_widget (xml, "XLabel");
	droplist.ylabelentry = glade_xml_get_widget (xml, "YLabel");
	droplist.titleentry = glade_xml_get_widget (xml, "Title");

	droplist.xminset = glade_xml_get_widget (xml, "XMinCheck");
	g_signal_connect (G_OBJECT (droplist.xminset), "toggled",
		G_CALLBACK (droplist_update_limit_sensitivities), NULL);
	droplist.xmaxset = glade_xml_get_widget (xml, "XMaxCheck");
	g_signal_connect (G_OBJECT (droplist.xmaxset), "toggled",
		G_CALLBACK (droplist_update_limit_sensitivities), NULL);
	droplist.yminset = glade_xml_get_widget (xml, "YMinCheck");
	g_signal_connect (G_OBJECT (droplist.yminset), "toggled",
		G_CALLBACK (droplist_update_limit_sensitivities), NULL);
	droplist.ymaxset = glade_xml_get_widget (xml, "YMaxCheck");
	g_signal_connect (G_OBJECT (droplist.ymaxset), "toggled",
		G_CALLBACK (droplist_update_limit_sensitivities), NULL);

	droplist.xminspin = glade_xml_get_widget (xml, "XMin");
	droplist.xmin = gtk_spin_button_get_adjustment
		(GTK_SPIN_BUTTON (droplist.xminspin));
	droplist.xmaxspin = glade_xml_get_widget (xml, "XMax");
	droplist.xmax = gtk_spin_button_get_adjustment
		(GTK_SPIN_BUTTON (droplist.xmaxspin));
	droplist.yminspin = glade_xml_get_widget (xml, "YMin");
	droplist.ymin = gtk_spin_button_get_adjustment
		(GTK_SPIN_BUTTON (droplist.yminspin));
	droplist.ymaxspin = glade_xml_get_widget (xml, "YMax");
	droplist.ymax = gtk_spin_button_get_adjustment
		(GTK_SPIN_BUTTON (droplist.ymaxspin));

	droplist.extra = glade_xml_get_widget (xml, "Extra");
	GtkWidget *extra_infobutton = glade_xml_get_widget (xml, "ExtraInfo");
	g_signal_connect (G_OBJECT (extra_infobutton), "clicked",
		G_CALLBACK (droplist_extra_help), NULL);

	GtkWidget *addfile = glade_xml_get_widget (xml, "AddFile");
	g_signal_connect (G_OBJECT (addfile), "activate",
		G_CALLBACK (droplist_add_file_UI), NULL);

	droplist.deletemenu = glade_xml_get_widget (xml, "Remove");
	g_signal_connect (G_OBJECT (droplist.deletemenu), "activate",
		G_CALLBACK (droplist_delete), NULL);
	droplist.deletebutton = glade_xml_get_widget (xml, "RemoveButton");
	g_signal_connect (G_OBJECT (droplist.deletebutton), "clicked",
		G_CALLBACK (droplist_delete), NULL);

	droplist.clearmenu = glade_xml_get_widget (xml, "Clear");
	g_signal_connect (G_OBJECT (droplist.clearmenu), "activate",
		G_CALLBACK (droplist_clear), NULL);
	droplist.clearbutton = glade_xml_get_widget (xml, "ClearButton");
	g_signal_connect (G_OBJECT (droplist.clearbutton), "clicked",
		G_CALLBACK (droplist_clear), NULL);

	droplist.plotmenu = glade_xml_get_widget (xml, "Plot");
	g_signal_connect (G_OBJECT (droplist.plotmenu), "activate",
		G_CALLBACK (droplist_plot), NULL);
	droplist.plotbutton = glade_xml_get_widget (xml, "PlotButton");
	g_signal_connect (G_OBJECT (droplist.plotbutton), "clicked",
		G_CALLBACK (droplist_plot), NULL);

	droplist.exportbutton = glade_xml_get_widget (xml, "ExportPS");
	g_signal_connect (G_OBJECT (droplist.exportbutton), "activate",
		G_CALLBACK (droplist_export), NULL);

	droplist.renamebutton = glade_xml_get_widget (xml, "Rename");
	g_signal_connect (G_OBJECT (droplist.renamebutton), "activate",
		G_CALLBACK (droplist_rename), NULL);

#if GTK_MINOR_VERSION < 6
	GtkWidget *helpmenu = glade_xml_get_widget (xml, "HelpMenu");
	gtk_widget_hide (helpmenu);
#endif
	GtkWidget *aboutbutton = glade_xml_get_widget (xml, "About");
	g_signal_connect (G_OBJECT (aboutbutton), "activate",
		G_CALLBACK (droplist_about), NULL);

	GtkWidget *quitbutton = glade_xml_get_widget (xml, "Quit");
	g_signal_connect (G_OBJECT (quitbutton), "activate",
		G_CALLBACK (droplist_exit), NULL);

	droplist_clear ();
	droplist_update_limit_sensitivities ();
	droplist_selection_changed (sel, NULL);
}


void droplist_add_file (char const *filename)
{
	gtk_widget_hide_all (droplist.dropherebox);
	gtk_widget_show_all (droplist.filebox);

	GtkTreeIter it;
	gtk_list_store_append (droplist.store, &it);

	gchar *basename = g_path_get_basename (filename);
	/*gchar *tmp = basename;
	basename = g_filename_to_utf8 ();*/
	gtk_list_store_set (droplist.store, &it,
		COL_FILENAME, basename,
		COL_PATH, filename,
		COL_TITLE, basename,
		COL_SERIES, "2",
		-1);

	g_free (basename);

	droplist_update_sensitivities ();
}


void droplist_clear ()
{
	gtk_list_store_clear (droplist.store);
	droplist_update_sensitivities ();

	/*GtkTreeIter it;
	gtk_list_store_append (droplist.store, &it);
	gtk_list_store_set (droplist.store, &it,
		COL_FILENAME, "",
		COL_PATH, "",
		COL_TITLE, "<i>Drop files here</i>",
		COL_SERIES, "",
		-1);*/

	/*GtkTreeIter it;
	gtk_list_store_append (droplist.store, &it);
	gtk_list_store_set (droplist.store, &it, COL_FILENAME, "<i>Drag files here</i>", -1);*/
}


void droplist_getdata (plotdata *data)
{
	GtkTreeModel *model = GTK_TREE_MODEL (droplist.store);

	int n_rows = gtk_tree_model_iter_n_children (model, NULL);
	char **paths = (char**) g_malloc (n_rows * sizeof (char*));
	char **titles = (char**) g_malloc (n_rows * sizeof (char*));
	int *series = (int*) g_malloc (n_rows * sizeof (int));

	GtkTreeIter it;
	if (!gtk_tree_model_get_iter_first (model, &it))
		return;

	int i = 0;
	do {
		gtk_tree_model_get (model, &it, COL_PATH, &paths[i], -1);
		gtk_tree_model_get (model, &it, COL_TITLE, &titles[i], -1);
		gchar *seriestmp;
		gtk_tree_model_get (model, &it, COL_SERIES, &seriestmp, -1);
		//g_print ("'%s'\n", seriestmp);
		sscanf (seriestmp, "%d", &series[i]);
		g_free (seriestmp);
		i++;
	} while (gtk_tree_model_iter_next (model, &it));


	// Files
	data->paths = paths;
	data->titles = titles;
	data->count = n_rows;
	data->series = series;

	// Apearance
	data->style = gtk_combo_box_get_active (GTK_COMBO_BOX (droplist.stylecombo));
	data->zeroaxis = gtk_toggle_button_get_active (
		GTK_TOGGLE_BUTTON (droplist.zeroaxischeck));
	data->errorbars = gtk_toggle_button_get_active (
		GTK_TOGGLE_BUTTON (droplist.errorbarscheck));
	data->grid = gtk_toggle_button_get_active (
		GTK_TOGGLE_BUTTON (droplist.gridcheck));
	data->enhancedmode = gtk_toggle_button_get_active (
		GTK_TOGGLE_BUTTON (droplist.enhancedmodecheck));
	data->logscaley = gtk_toggle_button_get_active (
		GTK_TOGGLE_BUTTON (droplist.logscaleycheck));
	data->logscalex = gtk_toggle_button_get_active (
		GTK_TOGGLE_BUTTON (droplist.logscalexcheck));

	// Captions
	data->xlabel = gtk_entry_get_text (GTK_ENTRY (droplist.xlabelentry));
	data->ylabel = gtk_entry_get_text (GTK_ENTRY (droplist.ylabelentry));
	data->title = gtk_entry_get_text (GTK_ENTRY (droplist.titleentry));

	// Limits
	data->xmin = gtk_adjustment_get_value (droplist.xmin);
	data->xmax = gtk_adjustment_get_value (droplist.xmax);
	data->ymin = gtk_adjustment_get_value (droplist.ymin);
	data->ymax = gtk_adjustment_get_value (droplist.ymax);
	data->xminset = gtk_toggle_button_get_active
		(GTK_TOGGLE_BUTTON (droplist.xminset));
	data->xmaxset = gtk_toggle_button_get_active
		(GTK_TOGGLE_BUTTON (droplist.xmaxset));
	data->yminset = gtk_toggle_button_get_active
		(GTK_TOGGLE_BUTTON (droplist.yminset));
	data->ymaxset = gtk_toggle_button_get_active
		(GTK_TOGGLE_BUTTON (droplist.ymaxset));

	// Arbitrary commands
	GtkTextIter start;
	GtkTextIter end;
	GtkTextBuffer *buf = gtk_text_view_get_buffer (GTK_TEXT_VIEW (droplist.extra));
	gtk_text_buffer_get_start_iter (buf, &start);
	gtk_text_buffer_get_end_iter (buf, &end);
	data->extra = gtk_text_buffer_get_text (buf, &start, &end, TRUE);

	// File me!
	//data->seriescount = 2;
}


void droplist_freedata (plotdata *data)
{
	for (int i = 0; i < data->count; ++i) {
		g_free (data->paths[i]);
		g_free (data->titles[i]);
	}

	g_free (data->paths);
	g_free (data->titles);
	g_free (data->extra);
	g_free (data->series);
}


void droplist_handle_plot (
	plotdata const *data,
	gboolean export,
	gchar *outfile,
	exportformat format)
{
	char *retval = gnuplot_plot (data, export, outfile, format);
	if (retval) {
		gchar *errorstring = g_convert (retval, -1, "UTF-8", "ISO-8859-1", NULL, NULL, NULL);
		GtkWidget *dialog = gtk_message_dialog_new_with_markup (
			NULL, 0, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
			"<b><big>Plotting error!</big></b>\n\n"
			"The following error(s) were experienced when running gnuplot:\n");
		gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

		GtkWidget *vbox = GTK_DIALOG(dialog)->vbox;

		GtkTextBuffer *buffer = gtk_text_buffer_new (NULL);
		gtk_text_buffer_set_text (buffer, errorstring, -1);
		g_free (errorstring);

		// The output probably won't fit in the window, and we don't want
		// to wrap it.
		GtkWidget *scrolled = gtk_scrolled_window_new (NULL, NULL);
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
		gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled),
			GTK_SHADOW_IN);
		gtk_box_pack_start_defaults (GTK_BOX (vbox), scrolled);

		GtkWidget *textview = gtk_text_view_new_with_buffer (buffer);
		gtk_text_view_set_editable (GTK_TEXT_VIEW (textview), FALSE);
		// Use a monospace font, because GNUPlot formats output accordingly
		PangoFontDescription *font_desc;
		font_desc = pango_font_description_from_string("Monospace");
		gtk_widget_modify_font(textview, font_desc);
		pango_font_description_free(font_desc);
		gtk_container_add (GTK_CONTAINER (scrolled), textview);
		gtk_widget_show_all (scrolled);

		gtk_widget_set_size_request (GTK_WIDGET (dialog), 500, 300);
		gtk_window_set_resizable (GTK_WINDOW (dialog), TRUE);

		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (GTK_WIDGET (dialog));
	}
}


void droplist_plot ()
{
	plotdata data;

	droplist_getdata (&data);
	droplist_handle_plot (&data, NULL, NULL, 0);
	droplist_freedata (&data);
}


void droplist_export ()
{
	// Save the user's format preference in between calls
	static int formatrow = 0;

	GtkWidget *chooser = gtk_file_chooser_dialog_new (
		"Plot to File",
		NULL,
		GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
		NULL);
	gtk_dialog_set_default_response (GTK_DIALOG (chooser), GTK_RESPONSE_ACCEPT);

	#if GTK_MINOR_VERSION >= 8
		gtk_file_chooser_set_do_overwrite_confirmation (
			GTK_FILE_CHOOSER (chooser), TRUE);
	#endif

	if (droplist.exportpath) {
		g_message ("Using exportpath '%s'\n", droplist.exportpath);
		gtk_file_chooser_set_current_folder (
			GTK_FILE_CHOOSER (chooser), droplist.exportpath);
	} else {
		// Go to the folder of the first file in the list
		GtkTreeIter it;
		GtkTreeModel *model = GTK_TREE_MODEL (droplist.store);
		if (!gtk_tree_model_get_iter_first (model, &it))
			return;
		gchar *path;
		gchar *pathdir;
		gtk_tree_model_get (model, &it, COL_PATH, &path, -1);
		pathdir = g_path_get_dirname (path);
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (chooser), pathdir);
		g_free (pathdir);
	}


	static char* format_names[] = {
		"Postscript", "Color Postscript",
		"Encapsulated Postscript (EPS)", "Color EPS",
		"Portable Network Graphics (PNG)",
		"Scalable Vector Graphcs (SVG)",
		"LaTeX-embedded Postscript (pslatex)",
		"Color LaTeX-embedded Postscript (pslatex)",
		"XFig"};
	static char* format_extensions[] = {
		".ps", ".ps", ".eps", ".eps", ".png", ".svg", ".tex", ".tex", ".fig"};

	GtkWidget *formatcombo = gtk_combo_box_new_text ();
	gtk_combo_box_append_text (GTK_COMBO_BOX (formatcombo), "Automatic from file name");
	for (int i = 0; i < FORMAT_COUNT; ++i)
		gtk_combo_box_append_text (GTK_COMBO_BOX (formatcombo), format_names[i]);
	gtk_combo_box_set_active (GTK_COMBO_BOX (formatcombo), formatrow);

	GtkWidget *formatbox = gtk_hbox_new (FALSE, 12);
	GtkWidget *formatlabel = gtk_label_new ("Format:");
	gtk_box_pack_start (GTK_BOX (formatbox), formatlabel, FALSE, FALSE, 0);
	gtk_box_pack_start_defaults (GTK_BOX (formatbox), formatcombo);

	gtk_file_chooser_set_extra_widget (GTK_FILE_CHOOSER (chooser), formatbox);
	gtk_widget_show_all (formatbox);

	if (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_ACCEPT) {
		gchar *filename = g_strdup (
			gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser)));

		formatrow = gtk_combo_box_get_active (GTK_COMBO_BOX (formatcombo));
		exportformat format;
		gboolean guessformat = FALSE;
		if (formatrow != 0) {
			format = formatrow - 1;
		} else {
			format = FORMAT_PS;
			guessformat = TRUE;
		}

		gchar *basename = g_path_get_basename (filename);
		gchar *extension = strrchr (basename, '.');
		if (extension == NULL) {
			// Append appropriate extension because the filename has no extension
			char *strtmp;
			strtmp = (char*)filename;
	    filename = g_strconcat (filename, format_extensions[format], NULL);
		  g_free (strtmp);
		} else if (guessformat) {
			// Guess at what format to used based on the extension the user gave the file
			for (int i = 0; i < FORMAT_COUNT; ++i) {
				if (!strcmp (format_extensions[i], extension)) {
					format = i;
					break;
				}
			}
			// Falling through to the default format set above.
		}
		g_free (basename);

		plotdata data;
		droplist_getdata (&data);
		droplist_handle_plot (&data, TRUE, filename, format);
		droplist_freedata (&data);

		if (droplist.exportpath)
			g_free (droplist.exportpath);
		droplist.exportpath = g_path_get_dirname (filename);

		g_free (filename);
	}

	gtk_widget_destroy (chooser);
}


gint droplist_exit (GtkWidget *widget, gpointer data)
{
	if (droplist_really_quit ()) {
		gtk_main_quit ();
	}

	return TRUE;
}


void droplist_drag_data_received (GtkWidget *widget,
                                  GdkDragContext *drag_context,
                                  gint x,
                                  gint y,
                                  GtkSelectionData *data,
                                  guint info,
                                  guint time,
                                  gpointer user_data)
{
gchar **uriList;
gchar *listItem;

uriList = g_uri_list_extract_uris (data->data);

for (listItem = uriList; *listItem != NULL; listItem++)
{
	fprintf(stderr, "%s\n", listItem);

}
g_strfreev (uriList);

/*	GList *list = NULL;
	GSList *file_list = NULL;
	GList *p = NULL;

	list = gnome_vfs_uri_list_parse (data->data);
	p = list;

	while (p != NULL)
	{
		file_list = g_slist_prepend (file_list,
			gnome_vfs_uri_to_string ((const GnomeVFSURI*)(p->data),
			GNOME_VFS_URI_HIDE_NONE));

		gchar *uripath = gnome_vfs_uri_to_string ((const GnomeVFSURI*)(p->data), 0);
		gchar *filename = g_filename_from_uri (uripath, NULL, NULL);
		droplist_add_file (filename);
		g_free (filename);
		g_free (uripath);

		p = g_list_next (p);
	}

	gnome_vfs_uri_list_free (list);

	file_list = g_slist_reverse (file_list);
*/
}


void droplist_selection_changed (GtkTreeSelection *sel, gpointer data)
{
	// Update sensitivity of delete & rename buttons
	gboolean enable = gtk_tree_selection_get_selected (sel, NULL, NULL);
	gtk_widget_set_sensitive (droplist.deletemenu, enable);
	gtk_widget_set_sensitive (droplist.deletebutton, enable);
	gtk_widget_set_sensitive (droplist.renamebutton, enable);
}

void droplist_update_sensitivities ()
{
	gboolean enable = gtk_tree_model_iter_n_children
		(GTK_TREE_MODEL (droplist.store), NULL);
	gtk_widget_set_sensitive (droplist.plotmenu, enable);
	gtk_widget_set_sensitive (droplist.plotbutton, enable);
	gtk_widget_set_sensitive (droplist.exportbutton, enable);
	gtk_widget_set_sensitive (droplist.clearbutton, enable);
	gtk_widget_set_sensitive (droplist.clearmenu, enable);
}

void droplist_delete ()
{
	GtkTreeModel *model;
	GtkTreeIter it;
	if (!gtk_tree_selection_get_selected (gtk_tree_view_get_selection (GTK_TREE_VIEW (droplist.view)), &model, &it))
		return;

	gtk_list_store_remove (droplist.store, &it);

	// Broken - we need to use gtk_tree_row_reference stuff to modify
	/*GList *selected = gtk_tree_selection_get_selected_rows (
		gtk_tree_view_get_selection (GTK_TREE_VIEW (droplist.view)));

	while (selected) {
		gtk_tree_model_get_iter (gtk_tree_view_get_model (GTK_TREE_VIEW (droplist.view)), &it, selected->data);
		//gtk_list_store_remove (droplist.store, &it);
		selected = selected->next;
	}

	g_list_foreach (list, gtk_tree_path_free, NULL);
	g_list_free (list);*/

	droplist_update_sensitivities ();
}


void droplist_rename ()
{
	GtkTreeModel *model;
	GtkTreeIter it;

	if (!gtk_tree_selection_get_selected (gtk_tree_view_get_selection (GTK_TREE_VIEW (droplist.view)), &model, &it))
		return;

	GtkTreePath *path = gtk_tree_model_get_path (model, &it);
	gtk_tree_view_set_cursor (GTK_TREE_VIEW (droplist.view), path,
		gtk_tree_view_get_column (GTK_TREE_VIEW (droplist.view), 1), TRUE);
	gtk_tree_path_free (path);
}


void droplist_title_edited (GtkCellRendererText *cell,
                           gchar               *path_string,
                           gchar               *new_text,
                           gpointer             user_data)
{
	GtkTreeIter it;
	if (gtk_tree_model_get_iter_from_string (
	    GTK_TREE_MODEL (droplist.store), &it, path_string))
		gtk_list_store_set (droplist.store, &it, COL_TITLE, new_text, -1);
}


void droplist_series_edited (GtkCellRendererText *cell,
                           gchar               *path_string,
                           gchar               *new_text,
                           gpointer             user_data)
{

	int scratch;
	// Make sure it's an integer
	if (sscanf (new_text, "%d", &scratch) != 1)
		return;
	// Field 1 is always the x axis
	if (scratch < 2)
		return;

	// Enter it into the treemodel
	GtkTreeIter it;
	if (gtk_tree_model_get_iter_from_string (
	    GTK_TREE_MODEL (droplist.store), &it, path_string))
		gtk_list_store_set (droplist.store, &it, COL_SERIES, new_text, -1);
}


void droplist_row_activated (GtkTreeView *treeview,
                             GtkTreePath *arg1,
                             GtkTreeViewColumn *arg2,
                             gpointer user_data)
{
	GtkTreeModel *model = GTK_TREE_MODEL (droplist.store);
	GtkTreeIter it;

	if (!gtk_tree_selection_get_selected (
	    gtk_tree_view_get_selection (treeview), &model, &it))
		return;

	gchar *path;
	gtk_tree_model_get (model, &it, COL_PATH, &path, -1);
	path = g_strdup_printf ("file://%s", path);

	//gnome_vfs_url_show (path);

	g_free (path);
}

void droplist_update_limit_sensitivities ()
{
	gtk_widget_set_sensitive (droplist.xminspin,
		gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (droplist.xminset)));
	gtk_widget_set_sensitive (droplist.xmaxspin,
		gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (droplist.xmaxset)));
	gtk_widget_set_sensitive (droplist.yminspin,
		gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (droplist.yminset)));
	gtk_widget_set_sensitive (droplist.ymaxspin,
		gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (droplist.ymaxset)));
}


void droplist_extra_help (GtkWidget *widg, gpointer data)
{
	GtkWidget *dialog = gtk_message_dialog_new_with_markup (
		NULL,
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_INFO,
		GTK_BUTTONS_CLOSE,
		"<b><big>Using extra commands</big></b>\n\n"
		"The commands entered here are executed by gnuplot after plotdrop "
		"sets up the environment according to the requested settings, but "
		"before the plot command itself.  For example, if you select the "
		"\"Lines\" appearance and put \"set style data points\" here, then "
		"the resulting plot will be with points rather than lines."
		);

	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
}


void droplist_no_gnuplot ()
{
	GtkWidget *dialog = gtk_message_dialog_new_with_markup (
		NULL,
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_ERROR,
		GTK_BUTTONS_NONE,
		"<b><big>Gnuplot not found!</big></b>\n\n"
		"PlotDrop cannot find gnuplot.  Please make sure that you have gnuplot "
		"installed, and that it is in a directory referred to in the PATH "
		"environment variable.  PlotDrop will not work.  Do you want to quit "
		"now, or continue anyway?"
		);

	gtk_dialog_add_buttons (GTK_DIALOG (dialog),
		"Continue Anyway", GTK_RESPONSE_OK,
		GTK_STOCK_QUIT, GTK_RESPONSE_CANCEL,
		NULL);

	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_CANCEL);


	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_CANCEL) {
		exit (-1);
	}
	gtk_widget_destroy (dialog);
}

gboolean droplist_really_quit ()
{
	if (!gtk_tree_model_iter_n_children
	    (GTK_TREE_MODEL (droplist.store), NULL))
		return TRUE;


	GtkWidget *dialog = gtk_message_dialog_new_with_markup (
		NULL,
		GTK_DIALOG_MODAL,
		GTK_MESSAGE_QUESTION,
		GTK_BUTTONS_NONE,
		"<b><big>Really quit?</big></b>\n\n"
		"Are you certain you wish to quit?  If you quit now, you will lose "
		"the contents of the file list and all parameters."
		);

	gtk_dialog_add_buttons (GTK_DIALOG (dialog),
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_QUIT, GTK_RESPONSE_OK,
		NULL);

	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_CANCEL);

	gboolean retval;
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK)
		retval = TRUE;
	else
		retval = FALSE;

	gtk_widget_destroy (dialog);

	return retval;
}


void droplist_about ()
{
#if GTK_MINOR_VERSION >= 6
	GtkWidget *aboutdialog = gtk_about_dialog_new ();

	gtk_about_dialog_set_name (GTK_ABOUT_DIALOG (aboutdialog),
		"PlotDrop");

	gtk_about_dialog_set_comments (GTK_ABOUT_DIALOG (aboutdialog),
		"A GNOME frontend to gnuplot, for simple graph plotting");

	gtk_about_dialog_set_copyright (GTK_ABOUT_DIALOG (aboutdialog),
		"Copyright © 2005 John Spray");

	gtk_about_dialog_set_website (GTK_ABOUT_DIALOG (aboutdialog),
		"http://icculus.org/~jcspray/plotdrop/");
	gtk_about_dialog_set_website_label (GTK_ABOUT_DIALOG (aboutdialog),
		"http://icculus.org/~jcspray/plotdrop/");

	gchar *extrastring = g_strdup_printf (
		"%s\nUsing gnuplot version %s",
		gtk_about_dialog_get_comments (GTK_ABOUT_DIALOG (aboutdialog)),
		gnuplot_get_version ());
	gtk_about_dialog_set_comments (
		GTK_ABOUT_DIALOG (aboutdialog), extrastring);

	gtk_about_dialog_set_version (
		GTK_ABOUT_DIALOG (aboutdialog), VERSION);

	gtk_dialog_run (GTK_DIALOG (aboutdialog));

	gtk_widget_destroy (aboutdialog);
#endif
}


void droplist_add_file_UI ()
{
	GtkWidget *chooser = gtk_file_chooser_dialog_new (
		"Add data file",
		NULL,
		GTK_FILE_CHOOSER_ACTION_OPEN,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_ADD, GTK_RESPONSE_ACCEPT,
		NULL);
	gtk_dialog_set_default_response (GTK_DIALOG (chooser), GTK_RESPONSE_ACCEPT);
	gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (chooser), TRUE);

	// Go to the folder of the first file in the list
	GtkTreeIter it;
	GtkTreeModel *model = GTK_TREE_MODEL (droplist.store);
	if (gtk_tree_model_get_iter_first (model, &it)) {
		gchar *path;
		gchar *pathdir;
		gtk_tree_model_get (model, &it, COL_PATH, &path, -1);
		pathdir = g_path_get_dirname (path);
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (chooser), pathdir);
		g_free (pathdir);
	}

	if (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_ACCEPT) {
		GSList *files = gtk_file_chooser_get_filenames (GTK_FILE_CHOOSER (chooser));
		GSList *it = files;
		while (it != NULL) {
			gchar *filename = (gchar*)it->data;
			droplist_add_file (filename);
			g_free (filename);
			it = it->next;
		}
		g_slist_free (files);
	}

	gtk_widget_destroy (chooser);

}
