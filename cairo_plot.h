/*
 * Plot using cairo
 */

#ifndef CAIRO_PLOT_H
#define CAIRO_PLOT_H

#include "cairo_box.h"

#include <iostream>
#include <vector>
#include <algorithm>    // std::min_element, std::max_element
#include <sstream>      // ostringstream
#include <cassert>
#include <cmath>        // only for testing, FIXME
#include <fstream>

//#define DEBUG_DRAW_TIMING

// Wieviele Punkte werden maximal simultan im Fenster angezeigt?
// Wird dieser Wert überschritten, erhöht sich plot_increment
// und in Folge dessen wird nur jeder n-te Wert angezeigt

#define MAX_VISIBLE_POINTS 5000

// entspricht 33.3 min bei 100Hz
#define MAX_NUM_SAMPLES 200000

//#define MAX_NUM_SAMPLES 100
//#define MAX_NUM_SAMPLES 6

#define NUM_CHANNELS 3

using namespace std;

enum mode {AUTO, MANUAL};
enum edrag_mode {PAN, ZOOM};

class cairo_plot;

class marker
{
public:

  double color[3];
  double linewidth;

  marker (double _linewidth, double red, double green, double blue)
    : linewidth(_linewidth)
  {
    color[0] = red;
    color[1] = green;
    color[2] = blue;
  }

  virtual ~marker ()
  { }

};

class point_marker: public marker
{
public:

  double x;
  double y;
  double diameter;

  point_marker (double _x, double _y, double _linewidth, double _dia, double red, double green, double blue):
    marker(_linewidth, red, green, blue),
    x(_x), y(_y), diameter (_dia)
  { }

};

class line_marker: public marker
{
public:
  enum style {horizontal, vertical};

  double pos;
  style s;

  line_marker (double _pos, style _s, double _linewidth, double red, double green, double blue):
    marker(_linewidth, red, green, blue),
    pos(_pos), s(_s)
  { }

};

class cairo_plot : public cairo_box
{
private:
  double border_left;       // as fraction, default 10%
  double border_right;      // as fraction, default 10%
  double border_top;        // as fraction, default 10%
  double border_bottom;     // as fraction, default 10%
  double linewidth;         // in pixels, default 2px
  double gridlinewidth;     // in pixels, default 1px

  string xlabel;
  string xunit;

  string ylabel;
  string yunit;

  vector<double> xtick;
  vector<double> ytick;
  double xlim[2];
  double ylim[2];

  mode xtickmode;
  mode ytickmode;

  mode xlimmode;
  mode ylimmode;

  edrag_mode drag_mode;

  // die eigentlichen Samples
  unsigned int num_samples;
  float data[NUM_CHANNELS][MAX_NUM_SAMPLES];
  float data_dec_sum[NUM_CHANNELS];

  // Berechnete Statistiken min/max
  float data_min[NUM_CHANNELS];
  float data_max[NUM_CHANNELS];

  // welcher Kanal wird für Abszisse, welcher für y_chan verwendet?
  int x_chan;
  int y_chan;

  double zoom_max_w;
  double zoom_min_w;
  double zoom_max_h;
  double zoom_min_h;

  double zoom_rect[4];  //x,y,w,h

  vector<marker*> plot_marker;

  void cairo_draw_label (double x, double y, int align, const char *str, double size, double rot);
  void cairo_draw_grid ();
  void cairo_draw_axes ();
  void cairo_draw ();

  void pixel2data (int _x, int _y, double &data_x, double &data_y, bool only_scale);

  void update_stats (bool clear = false);

  int calc_visible_points ();

  // jeder wievielte Punkt wird gezeichnet?
  // Dient der Reduzierung der CPU Last bei großer Datenmenge
  unsigned int plot_increment;

  // Dezimierungsfaktor, wenn die Anzahl der sample MAX_NUM_SAMPLES überschreitet
  int decimation_factor;

public:
  cairo_plot(int x, int y, int w, int h, const char *l=0);
  ~cairo_plot ();

  void downsample2 ();

#if NUM_CHANNELS == 3
  void add_data (float x0, float x1, float x2);
#endif

  int add_point_marker (double _x, double _y, double _linewidth, double _dia, double red, double green, double blue);
  int add_line_marker (double _pos, line_marker::style _s, double _w, double red, double green, double blue);

  void clear ();
  unsigned int get_max_ticklen (double start, double step, double stop);

  void set_xtick (double start, double step, double stop);
  void set_ytick (double start, double step, double stop);

  // Abstände der Gitternetzlinien aus dem anzuzeigenden Bereich
  // berechnen
  double tick_from_lim (double r);
  void zoom (double factor);

  // die minimale und maximale "Daten-Breite" des Plots
  void set_zoom_limits (double min_w, double max_w, double min_h, double max_h);

  void set_xlim (double x0, double x1);
  void get_xlim (double &x0, double &x1);

  void set_ylim (double y0, double y1);
  void get_ylim (double &y0, double &y1);

  void set_drag_mode (edrag_mode d);

  // ein ! am Anfang macht den Label unsichtbar
  void set_xlabel (const string &s);
  void set_ylabel (const string &s);

  // ein ! am Anfang macht die Einheit unsichtbar
  // FIXME: Wird momentan nur intern gespeichert, noch nicht gezeichnet
  // macht aber für save_measurement trotzdem schon Sinn
  void set_xunit (const string &s);
  void set_yunit (const string &s);

  void auto_zoom ();

  // alle Werte realtiv: 0.1 -> 10% der Höhe/Breite
  void set_border (double left, double top, double right, double bottom);

  void set_x_chan (int chan);
  void set_y_chan (int chan);

  void load_csv (const char *fn);
  void save_csv (const char *fn);

  int handle (int event);
};

#endif
