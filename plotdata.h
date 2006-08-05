
/* PlotDrop is free software, released under the GNU General Public License
 * See the COPYING file for licensing details.
 *
 * Copyright 2005 John Spray
 */


#ifndef PLOTDATA_H
#define PLOTDATA_H

typedef struct plotdata {
	char **paths;
	char **titles;
	int *series;
	int count;
	char const *xlabel;
	char const *ylabel;
	char const *title;
	enum {STYLE_POINTS = 0, STYLE_LINES, STYLE_LINESPOINTS} style;
	unsigned int zeroaxis;
	unsigned int errorbars;
	unsigned int grid;
	float xmin, xmax, ymin, ymax;
	unsigned int xminset, xmaxset, yminset, ymaxset;
	char *extra;
	unsigned int enhancedmode;
} plotdata;

typedef enum {
	FORMAT_PS = 0, FORMAT_COLORPS, FORMAT_EPS, FORMAT_COLOREPS,	FORMAT_PNG,
	FORMAT_SVG, FORMAT_PSLATEX, FORMAT_COLORPSLATEX, FORMAT_XFIG,
	FORMAT_COUNT
	} exportformat;

extern char* exportformat_names[];

#endif //PLOTDATA_H
