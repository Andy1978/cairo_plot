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

class cairo_plot;

class marker
{
private:
  double x;
  double y;
  double diameter;
  double color[3];

public:
  marker (double _x, double _y, double _dia, double red, double green, double blue):
    x(_x), y(_y), diameter (_dia)
  {
    color[0] = red;
    color[1] = green;
    color[2] = blue;
  }
  friend cairo_plot;
};

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
  double xmin;
  double xmax;
  double ymin;
  double ymax;

  double zoom_max_x;
  double zoom_min_x;
  double zoom_max_y;
  double zoom_min_y;

  vector<marker*> plot_marker;

  void cairo_draw_label (double x, double y, int align, const char *str, double size);
  void cairo_draw_grid ();
  void cairo_draw_axes ();
  void cairo_draw ();

public:
  cairo_plot(int x, int y, int w, int h, const char *l=0);
  ~cairo_plot ();

  void add_point (double x, double y)
  {
    xdata.push_back (x);
    ydata.push_back (y);

    if (x > xmax)
      xmax = x;
    if (x < xmin)
      xmin = x;

    if (y > ymax)
      ymax = y;
    if (y < ymin)
      ymin = y;

    //cout << "xmin=" << xmin << " xmax=" << xmax << " ymin=" << ymin << " ymax=" << ymax << endl;
  }

  void add_marker (double x, double y, double dia, double red, double green, double blue)
  {
    plot_marker.push_back (new marker (x, y, dia, red, green, blue));
  }

  void clear ()
  {
    xdata.clear ();
    ydata.clear ();
    xmin = 1.0/0.0;
    ymin = 1.0/0.0;
    xmax = - xmin;
    ymax = - ymin;
    plot_marker.clear ();
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
    // in GNU Octave
    // n = round (3 * log10 (range / 7));
    // ret = (mod(n, 3).^2 + 1) .* 10.^floor(n/3);
    // Attention:
    // Octave mod (n, 3) is not equal to C++ n%3

    int n = round (3 * log (lim / 7.0) / log (10));
    double mod3 = n%3;
    if (mod3 < 0)
      mod3 += 3;

    return (pow (mod3, 2) + 1) * pow (10, floor (n / 3.0));
  }

  void zoom (double factor)
  {
    double new_w =  (xlim[1] - xlim[0]) * factor;
    double new_h =  (ylim[1] - ylim[0]) * factor;

    if (   new_w < zoom_max_x
           && new_w > zoom_min_x
           && new_h < zoom_max_y
           && new_h > zoom_min_y )
      {
        double mean_x = (xlim[0] + xlim[1]) / 2;
        double mean_y = (ylim[0] + ylim[1]) / 2;

        set_xlim (mean_x - new_w/2, mean_x + new_w/2);
        set_ylim (mean_y - new_h/2, mean_y + new_h/2);
        redraw ();
      }
  }

  void set_xlim (double x0, double x1)
  {
    cout << "set_xlim (" << x0 << ", " << x1 << ")" << endl;

    if (x1 > x0 + zoom_min_x)
      {
        xlim[0] = x0;
        xlim[1] = x1;

        if (xtickmode == AUTO)
          {
            double step = tick_from_lim (xlim[1] - xlim[0]);
            cout << "xstep=" << step << endl;
            set_xtick (ceil (xlim[0] / step) * step, step, floor (xlim[1] / step) * step);
          }
      }
  }

  void get_xlim (double &x0, double &x1)
  {
    x0 = xlim[0];
    x1 = xlim[1];
  }

  void set_ylim (double y0, double y1)
  {
    cout << "set_ylim (" << y0 << ", " << y1 << ")" << endl;

    if (y1 > y0 + zoom_min_y)
      {
        ylim[0] = y0;
        ylim[1] = y1;

        if (ytickmode == AUTO)
          {
            double step = tick_from_lim (ylim[1] - ylim[0]);
            set_ytick (ceil (ylim[0] / step) * step, step, floor (ylim[1] / step) * step);
          }
      }
  }

  void get_ylim (double &y0, double &y1)
  {
    y0 = ylim[0];
    y1 = ylim[1];
  }

  void update_limits ()
  {
    set_xlim (xmin, xmax);
    set_ylim (ymin, ymax * 1.1);
  }

  void load_csv (const char *fn, double FS);

  int handle (int event);
};

#endif
