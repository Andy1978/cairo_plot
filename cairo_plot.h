/*
 * simple plot using cairo
 */

#ifndef CAIRO_PLOT_H
#define CAIRO_PLOT_H

#include "cairo_box.h"

#include <iostream>
#include <vector>
#include <algorithm>    // std::min_element, std::max_element
#include <sstream>      // ostringstream
#include <cassert>
#include <cmath> //only for testing, FIXME
#include <fstream>

using namespace std;

enum mode {AUTO, MANUAL};

class cairo_plot : public cairo_box
{
private:
  double border;     // as fraction, default 10%
  double linewidth;  // in pixels, default 2px
  double gridlinewidth;  // in pixels, default 1px

  vector<double> xtick;
  vector<double> ytick;
  double xlim[2];
  double ylim[2];

  mode xtickmode;
  mode ytickmode;

  mode xlimmode;
  mode ylimmode;

  vector<double> xdata;
  vector<double> ydata;

  void cairo_draw_label (double x, double y, int align, const char *str, double size);
  void cairo_draw_grid ();
  void cairo_draw_axes ();
  void cairo_draw ();

public:
  cairo_plot(int x, int y, int w, int h, const char *l=0);

  void add_point (double x, double y)
  {
    xdata.push_back (x);
    ydata.push_back (y);
  }

  void clear_points ()
  {
    xdata.clear ();
    ydata.clear ();
  }

  void set_xtick (double start, double step, double stop)
  {
    xtick.clear ();
    for (double k=start; k<stop; k+=step)
      xtick.push_back (k);
    xtick.push_back (stop);
  }

  //~ double min_xtick ()
  //~ {
    //~ return *min_element (xtick.begin (), xtick.end());
  //~ }
//~ 
  //~ double max_xtick ()
  //~ {
    //~ return *max_element (xtick.begin (), xtick.end());
  //~ }

  void set_ytick (double start, double step, double stop)
  {
    ytick.clear ();
    for (double k=start; k<stop; k+=step)
      ytick.push_back (k);
    ytick.push_back (stop);
  }

  //~ double min_ytick ()
  //~ {
    //~ return *min_element (ytick.begin (), ytick.end());
  //~ }
//~ 
  //~ double max_ytick ()
  //~ {
    //~ return *max_element (ytick.begin (), ytick.end());
  //~ }


  double tick_from_lim (double lim)
  {
    double f = 5;
    return pow (f, ceil (log (lim / 10.0) / log(f)));
  }

  void set_xlim (double x0, double x1)
  {
    xlim[0] = x0;
    xlim[1] = x1;
    
    if (xtickmode == AUTO)
      {
        double step = tick_from_lim (xlim[1] - xlim[0]);
        set_xtick (ceil (xlim[0] / step) * step, step, floor (xlim[1] / step) * step);
      }
  }

  void set_ylim (double y0, double y1)
  {
    ylim[0] = y0;
    ylim[1] = y1;
    
    if (ytickmode == AUTO)
      {
        double step = tick_from_lim (ylim[1] - ylim[0]);
        set_ytick (ceil (ylim[0] / step) * step, step, floor (ylim[1] / step) * step);
      }
  }
  
  void load_csv (const char *fn, double FS);

  int handle (int event);
};

#endif
