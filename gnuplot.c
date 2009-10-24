
/* PlotDrop is free software, released under the GNU General Public License
 * See the COPYING file for licensing details.
 *
 * Copyright 2005 John Spray
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <locale.h>
#include <unistd.h>
#include <glib.h>

#include "gnuplot.h"

// Nasty, we have two arrays because not all formats support 'enhanced'
// Recall that 'enhanced' means doing things like superscripts and subscripts
static char* exportformats[] = {"postscript", "postscript color", "postscript eps", "postscript eps color", "png", "svg", "pslatex", "pslatex color", "fig"};
static char* exportformats_enhanced[] = {"postscript enhanced", "postscript enhanced color", "postscript enhanced eps", "postscript enhanced eps color", "png", "svg enhanced", "pslatex", "pslatex color", "fig"};

// This should be in stdio.h, I don't know why it's not.
extern FILE *popen (__const char *__command, __const char *__modes);
extern int pclose (FILE *__stream);

static gchar *gnuplot_prepstring (gchar const *in) {
		gchar *encoded = g_convert_with_fallback (
			in, -1, "ISO-8859-1", "UTF-8", "[]", NULL, NULL, NULL);
		gchar *const escaped = g_strescape (encoded, "");
		g_free (encoded);
		return escaped;
}

char *gnuplot_plot (
	plotdata const *data,
	unsigned int export,
	char *outfile,
	exportformat format)
{
	if (data->paths[0] == NULL || data->count < 1)
		return "Plotdrop: No data series found";

	char scriptname[256];
	snprintf (scriptname, 255, "/tmp/plotdrop_%d.gnu", getpid ());
	FILE *plotscript = fopen (scriptname, "w");

	if (!plotscript) {
		return "Plotdrop: Could not open temporary script file";
	}

	// Set encoding before selecting terminal
	fprintf (plotscript, "set encoding iso_8859_1\n");

	// Set which terminal to use for output
	if (export) {
		if (data->enhancedmode &&
		    !strcmp (exportformats_enhanced[format], exportformats[format]))
			fprintf (stderr, "gnuplot_export: Warning - enhanced mode was requested, "
			                 "but the selected format does not support it\n");
			//FIXME: tell the user


		fprintf (plotscript, "set term %s\nset output '%s'\n",
		         data->enhancedmode ?
		         exportformats_enhanced[format] : exportformats[format],
		         outfile);
	} else {
		fprintf (plotscript, "set term x11 %s\n",
			data->enhancedmode ? "enhanced" : "");
	}

	gnuplot_compose_script (plotscript, data);
	fprintf (stderr, "\n\nRunning GNUPlot with the following commands:\n");
	gnuplot_compose_script (stderr, data);

	fprintf (plotscript, "quit\n");
	fflush (plotscript);
	fclose (plotscript);

	// Compose the gnuplot command line
	char command[512];
	if (export)
		snprintf (command, 511, "gnuplot %s 2>&1", scriptname);
	else
		snprintf (command, 511, "gnuplot -persist %s 2>&1", scriptname);

	// Run gnuplot and get the return value
	FILE *instance = popen (command, "r");
	int error = pclose (instance);

	// In case of error:
	// We actually run gnuplot again in this case of an error, because
	// we only want to do the (potentially blocking) fread when there's
	// an error, and we only know if there's an error once we've closed
	// the original pipe.
	static char *log = NULL;
	if (error) {
		fprintf (stderr, "\nGNUPlot returned %d\n", error);
		if (!log)
			log = malloc (sizeof (char) * 2048);
		instance = popen (command, "r");
		// Get stdout and stderr all together
		int size = fread (log, 1, 2047, instance);
		pclose (instance);
		log[size] = '\0';
		if (size > 0)
			fprintf (stderr, "\n\nGNUPlot log:%d\n\"\"\"\n%s\n\n\"\"\"\n", size, log);
	}

	// Delete the plot script
	unlink (scriptname);

	if (error)
		return log;
	else
		return NULL;
}


void gnuplot_compose_script (FILE *plotscript, plotdata const *data)
{
	// GNUPlot wants values formatted with periods
	setlocale (LC_ALL, "C");

	gchar *escaped = NULL;
	if (data->xlabel) {
		escaped = gnuplot_prepstring (data->xlabel);
		fprintf (plotscript, "set xlabel \"%s\"\n", escaped);
		g_free (escaped);
	}

	if (data->ylabel) {
		escaped = gnuplot_prepstring (data->ylabel);
		fprintf (plotscript, "set ylabel \"%s\"\n", escaped);
		g_free (escaped);
	}

	if (data->title) {
		escaped = gnuplot_prepstring (data->title);
		fprintf (plotscript, "set title \"%s\"\n", escaped);
		g_free (escaped);
	}
	
	// Logscales
	if (data->logscaley)
		fprintf (plotscript, "set logscale y\n");
	if (data->logscalex)
		fprintf (plotscript, "set logscale x\n");

	if (data->zeroaxis)
		fprintf (plotscript, "set zeroaxis\n");

	if (data->grid)
		fprintf (plotscript, "set grid\n");

	if (data->extra)
		fprintf (plotscript, "%s\n", data->extra);
	
	char *plotcommand = "plot ";
	fprintf (plotscript, "%s", plotcommand);

	// X Limits
	fprintf (plotscript, "[");
	if (data->xminset)
		fprintf (plotscript, "%f", data->xmin);
	fprintf (plotscript, ":");
	if (data->xmaxset)
		fprintf (plotscript, "%f", data->xmax);
	fprintf (plotscript, "]");

	// Y Limits
	fprintf (plotscript, "[");
	if (data->yminset)
		fprintf (plotscript, "%f", data->ymin);
	fprintf (plotscript, ":");
	if (data->ymaxset)
		fprintf (plotscript, "%f", data->ymax);
	fprintf (plotscript, "]");
	
	// For cycling through point types "with points pt %d"
	int ptcounter = 0;

	for (int i = 0; i < data->count; ++i ) {
		char *path = data->paths[i];
		char *title = data->titles[i];
		int series = data->series[i];

		// Series separator
		if (i > 0)
			fprintf (plotscript, ",");

		// Filename
		if (path)
			fprintf (plotscript, " '%s' ", path);

		// 'Using' clause
		if (data->errorbars) {
			fprintf (plotscript, " using 1:%d:%d ", series, series + 1);
		} else {
			fprintf (plotscript, " using 1:%d ", series);
		}

		if (data->errorbars) {
			if (data->style == STYLE_POINTS)
				fprintf (plotscript, "with errorbars ");
			else if (data->style == STYLE_LINES || data->style == STYLE_LINESPOINTS)
				fprintf (plotscript, "with errorlines ");
		} else {
			switch (data->style) {
				case STYLE_POINTS:
					fprintf (plotscript, "with points ");
					break;
				case STYLE_LINES:
					fprintf (plotscript, "with lines ");
					break;
				case STYLE_LINESPOINTS:
					fprintf (plotscript, "with linespoints ");
					break;
				case STYLE_UNIQUE:
					fprintf (plotscript, "smooth unique ");
					break;
				case STYLE_FREQUENCY:
					fprintf (plotscript, "smooth frequency ");
					break;
				case STYLE_CSPLINES:
					fprintf (plotscript, "smooth csplines ");
					break;
				case STYLE_ACSPLINES:
					fprintf (plotscript, "smooth acsplines ");
					break;
				case STYLE_BEZIER:
					fprintf (plotscript, "smooth bezier ");
					break;
				case STYLE_SBEZIER:
					fprintf (plotscript, "smooth sbezier ");
					break;
			}
		}

		if (data->style == STYLE_POINTS
		    || data->style == STYLE_LINESPOINTS
		    || data->errorbars) {
		  // First use hollow shapes (4, 6, 8, 10, 12)
		  // Then use filled shapes (5, 7, 9, 11, 13)
		  // Then give up on being picky
			if (ptcounter < 5)
				fprintf (plotscript, "pt %d ", 4 + (ptcounter % 5) * 2);
			else if (ptcounter < 10)
				fprintf (plotscript, "pt %d ", 4 + (ptcounter % 5) * 2 + 1);
			else
				fprintf (plotscript, "pt %d ", ptcounter % 14);
			++ptcounter;
		}

		// Series label
		if (title) {
			title = gnuplot_prepstring (title);
			fprintf (plotscript, "title \"%s\"", title);
			g_free (title);
		}
	}

	fprintf (plotscript, "\n");

	setlocale (LC_ALL, "");
}

char* gnuplot_get_version ()
{
	FILE *prog = popen ("gnuplot --version", "r");
	if (!prog) {
		fprintf (stderr, "gnuplot_get_version: error opening pipe!\n");
		return NULL;		
	}

	char outp[255];
	int size = fread (outp, 1, 128, prog);
	outp[size] = '\0';

	int status = pclose (prog);
	if (status) {
		fprintf (stderr, "gnuplot_get_version: gnuplot not found!\n");
		return NULL;
	}		

	static char *version = NULL;
	if (!version)
		version = malloc (128);
	sscanf (outp, "gnuplot %s\n", version);

	if (strlen (version) == 0)
		return NULL;
	else
		return version;
}

