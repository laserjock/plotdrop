
/* PlotDrop is free software, released under the GNU General Public License
 * See the COPYING file for licensing details.
 *
 * Copyright 2005 John Spray
 */

#include <stdio.h>
#include "plotdata.h"

// Perform the plotting operation, either to the screen (data, 0, NULL, 0)
// or to a file (data, 1, ...)
char *gnuplot_plot (
	plotdata const *data,
	unsigned int export,
	char *outfile,
	exportformat format);

// Write a GNUPlot script to plot plotdata, to plotscript
void gnuplot_compose_script (FILE *plotscript, plotdata const *data);

// Return a string like "4.0" for "gnuplot 4.0 patchlevel 1"
char* gnuplot_get_version ();

