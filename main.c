
/* PlotDrop is free software, released under the GNU General Public License
 * See the COPYING file for licensing details.
 *
 * Copyright 2005 John Spray
 */

#include <gio/gio.h>
#include <glib.h>

#include "droplist.h"
#include "gnuplot.h"


int main(int argc, char **argv)
{
	gtk_set_locale ();
	gtk_init (&argc,&argv);

	gchar *ver = gnuplot_get_version ();
	if (ver)
		fprintf (stdout, "Found gnuplot version '%s'\n", ver);
	else
		droplist_no_gnuplot ();

	droplist_init_gui ();

	if (argc > 1) {
		gchar *pwd = getcwd (NULL, 0); // g_get_current_dir?

		for (int i = 1; i < argc; ++i) {
			if (argv[i][0] == '/'/*g_path_is_absolute (argv[i])*/) {
				g_message ("filename absolute '%s'", argv[i]);
				droplist_add_file (argv[i]);
			} else {
				gchar *tmp = g_build_filename (pwd, argv[i], NULL);
				g_message ("built filename '%s'", tmp);
				droplist_add_file (tmp);
				g_free (tmp);
			}
		}

		g_free (pwd);
	}

	gtk_main ();
}
