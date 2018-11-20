#include "cairo_plot.h"
#include <FL/names.h> //for fl_eventnames
#include <cmath>

cairo_plot::cairo_plot (int x, int y, int w, int h, const char *l)
  : cairo_box (x, y, w, h, l),
    border_left (0.10),
    border_right (0.10),
    border_top (0.10),
    border_bottom (0.10),
    linewidth (2),
    gridlinewidth (1),
    xtickmode (AUTO),
    ytickmode (AUTO),
    xlimmode (AUTO),
    ylimmode (AUTO),
    drag_mode (PAN),
    num_samples (0),
    x_chan (0),
    y_chan (1),
    zoom_max_w (50),
    zoom_min_w (0.02),
    zoom_max_h (500),
    zoom_min_h (0.05),
    plot_increment (1),
    decimation_factor (1)
{
  for (int k=0; k<4; ++k)
    zoom_rect[k] = 0;

  memset (data, 0, sizeof (float) * NUM_CHANNELS * MAX_NUM_SAMPLES);
  memset (data_dec_sum, 0, sizeof (float) * NUM_CHANNELS);

  clear ();

  set_xlim (0, 10);
  set_ylim (0, 5);

}

cairo_plot::~cairo_plot ()
{


}

void cairo_plot::cairo_draw_label (double x, double y, int align, const char *str, double size, double rot)
{
  cairo_save (cr);
  cairo_move_to (cr, x, y);

  //print_matrix ();
  cairo_matrix_t tmp;
  cairo_get_matrix (cr, &tmp);
  tmp.xx = 1;
  tmp.yy = 1;
  cairo_set_matrix (cr, &tmp);

  cairo_rotate (cr, rot);

  //print_matrix ();

  cairo_set_font_size (cr, size);
  cairo_text_extents_t extents;
  cairo_text_extents (cr, str, &extents);

  if (align == 0) //left
    {
      // FIXME: left aligned not yet tested
      //cairo_rel_move_to (cr, - (extents.width/2 + extents.x_bearing), extents.height/2 - extents.y_bearing);
    }
  else if (align == 1)  //top
    {
      cairo_rel_move_to (cr, - (extents.width/2 + extents.x_bearing), extents.height + size/3);
    }
  else if (align == 2) //right
    {
      cairo_rel_move_to (cr, - (extents.width + extents.x_bearing) - size/3, - extents.height/2 - extents.y_bearing);
    }

  cairo_show_text (cr, str);
  cairo_restore (cr);
}

void cairo_plot::cairo_draw_grid ()
{
  cairo_save (cr);

  for (vector<double>::iterator it = xtick.begin() ; it != xtick.end(); ++it)
    {
      cairo_move_to(cr, *it - xlim[0], 0);
      cairo_line_to(cr, *it - xlim[0], ylim[1] - ylim[0]);
    }

  for (vector<double>::iterator it = ytick.begin() ; it != ytick.end(); ++it)
    {
      cairo_move_to(cr, 0, *it - ylim[0]);
      cairo_line_to(cr, xlim[1] - xlim[0], *it - ylim[0]);
    }

  //static const double dashed[] = {4.0, 21.0, 2.0};
  //static int len_dashed  = sizeof(dashed) / sizeof(dashed[0]);
  static const double dash_len = 5;
  cairo_set_dash (cr, &dash_len, 1, 0);
  cairo_identity_matrix (cr);
  cairo_set_line_width (cr, 0.5);
  cairo_stroke (cr);
  cairo_restore (cr);
}

void cairo_plot::cairo_draw_axes ()
{
  cairo_save (cr);

  cairo_select_font_face (cr, "Helvetica", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);

  for (vector<double>::iterator it = xtick.begin() ; it != xtick.end(); ++it)
    {
      ostringstream tmp;
      tmp << *it;
      cairo_draw_label (*it - xlim[0], 0, 1, tmp.str().c_str (), 20, 0);
    }

  for (vector<double>::iterator it = ytick.begin() ; it != ytick.end(); ++it)
    {
      ostringstream tmp;
      tmp << *it;
      cairo_draw_label (0, *it - ylim[0], 2, tmp.str().c_str (), 20, 0);
    }

  cairo_identity_matrix (cr);
  cairo_restore (cr);
}

void cairo_plot::cairo_draw()
{
#ifdef DEBUG_DRAW_TIMING
  timespec ts1, ts2;
  clock_gettime(CLOCK_MONOTONIC, &ts1);
#endif

  //printf ("w=%i h=%i ----------------------------\n", w (), h ());
  cairo_identity_matrix (cr);
  // lower left
  cairo_translate(cr, x (), y () + h ());

  cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
  cairo_scale (cr, w (), -h ());

  //print_matrix ();

  // full plot space 0..1 in X and Y
  // here we could plot some legend

  // limit drawing
  cairo_rectangle (cr, 0, 0, 1, 1);
  cairo_clip (cr);

  // use 10% and 90% as 0..1
  cairo_translate(cr, border_left, border_bottom);
  cairo_scale (cr, 1 - (border_left + border_right), 1 - (border_top + border_bottom));

  // draw xlabel and ylabel
  if (xlabel.size() > 0 && xlabel.at(0) != '!')
    cairo_draw_label (0.5, -border_left/2, 1, xlabel.c_str (), 20, 0);
  if (ylabel.size() > 0 && ylabel.at(0) != '!')
    cairo_draw_label (-1.2 * border_bottom, 0.5, 1, ylabel.c_str (), 20, - M_PI/2);

  // now scale to xlim and ylim
  cairo_scale (cr, 1/(xlim[1] - xlim[0]), 1/(ylim[1] - ylim[0]));

  cairo_draw_grid ();
  cairo_draw_axes ();

  // Box um die Zeichenfläche
  cairo_save (cr);
  cairo_rectangle (cr, 0, 0, xlim[1] - xlim[0], ylim[1] - ylim[0]);
  cairo_set_line_width (cr, 2);
  cairo_matrix_t tmp;
  cairo_get_matrix (cr, &tmp);
  tmp.xx = 1;
  tmp.yy = 1;
  cairo_set_matrix (cr, &tmp);
  cairo_stroke (cr);
  cairo_restore (cr);

  // auf die Box clippen
  cairo_rectangle (cr, 0, 0, xlim[1] - xlim[0], ylim[1] - ylim[0]);
  cairo_clip (cr);

  cairo_save (cr);

  if (num_samples > plot_increment)
    {
      cairo_move_to (cr, data[x_chan][0] - xlim[0], data[y_chan][0] - ylim[0]);

      cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);

      int vis_cnt = 1;
      for (unsigned int k = 1; k < num_samples; k += plot_increment)
        {
          cairo_line_to(cr, data[x_chan][k] - xlim[0], data[y_chan][k] - ylim[0]);

          // zu zeichnende Datenpunkte zählen, die im xlim/ylim Fenster liegen
          if (   data[x_chan][k] > xlim[0]
                 && data[x_chan][k] < xlim[1]
                 && data[y_chan][k] > ylim[0]
                 && data[y_chan][k] < ylim[1])
            vis_cnt++;

        }

      plot_increment = (vis_cnt * plot_increment)/MAX_VISIBLE_POINTS + 1;

      //cout << "vis_cnt = " << vis_cnt << ", plot_increment = " << plot_increment << endl;

      //cairo_move_to(cr, 0.0, 0.0);
      //cairo_line_to(cr, 8, 5);

      // identity CTM so linewidth is in pixels
      cairo_identity_matrix (cr);
      cairo_set_line_width (cr, linewidth);
      cairo_stroke (cr);
    }

  cairo_restore (cr);
  // draw marker
  if (plot_marker.size () > 0)
    {
      cout << "there are " << plot_marker.size () << " markers..." << endl;
      for (vector<marker*>::iterator it = plot_marker.begin() ; it != plot_marker.end(); ++it)
        {
          cairo_set_source_rgb(cr, (*it)->color[0], (*it)->color[1], (*it)->color[2]);

          point_marker *m = dynamic_cast <point_marker*> (*it);
          line_marker *l = dynamic_cast <line_marker*> (*it);

          assert (m || l);
          if (m)
            {

              point_marker *m = dynamic_cast <point_marker*> (*it);


              double dx = m->diameter;
              double dy = dx;
              cairo_device_to_user_distance (cr, &dx, &dy);
              //cout << "dx=" << dx << " dy=" << dy << endl;

              cairo_move_to (cr, m->x - dx - xlim[0], m->y - ylim[0]);
              cairo_line_to (cr, m->x + dx - xlim[0], m->y - ylim[0]);
              cairo_move_to (cr, m->x - xlim[0], m->y - dy - ylim[0]);
              cairo_line_to (cr, m->x - xlim[0], m->y + dy - ylim[0]);

              //cairo_new_sub_path (cr);
              // das stellt verzerrt da
              //cairo_arc (cr, m->x - xlim[0], m->y - ylim[0], dx, 0, 2 * M_PI);

            }
          else if (l)
            {

              if (l->s == line_marker::style::horizontal)
                {
                  cout << "horizontal marker" << endl;
                  cairo_move_to (cr, 0, l->pos - ylim[0]);
                  cairo_line_to (cr, xlim[1] - xlim[0], l->pos - ylim[0]);
                }
              else if (l->s == line_marker::style::vertical)
                {
                  cout << "vertical marker" << endl;
                  cairo_move_to (cr, l->pos - xlim[0], 0);
                  cairo_line_to (cr, l->pos - xlim[0], ylim[1] - ylim[0]);
                }
            }

          cairo_save (cr);
          cairo_identity_matrix (cr);
          cairo_set_line_width (cr, (*it)->linewidth);
          cairo_stroke (cr);
          cairo_restore (cr);
        }

    }

  // draw zoom rect if wanted
  if (zoom_rect [2] != 0 && zoom_rect [3] != 0)
    {
      cairo_save (cr);
      cairo_set_source_rgb (cr, 0.0, 0.8, 0.0);
      cairo_rectangle (cr, zoom_rect[0], zoom_rect[1], zoom_rect[2], zoom_rect[3]);
      cairo_identity_matrix (cr);
      cairo_set_line_width (cr, 2);
      cairo_stroke (cr);
      cairo_restore (cr);
    }

  // Ausgabe Anzahl Punkte, ms,
  //int npoints = calc_visible_points();

#ifdef DEBUG_DRAW_TIMING
  clock_gettime(CLOCK_MONOTONIC, &ts2);

  double d = (ts2.tv_sec - ts1.tv_sec)*1.0e3 + (ts2.tv_nsec - ts1.tv_nsec)/1.0e6;
  cout << npoints << " " << d << " " << plot_increment << endl;

  /*
  * ich habe mir das Timing des Plots angesehen. Der Zeitversatz kommt wie vermutet dadurch,
  * dass die CPU (durch Xorg) ausgelastet ist und daher die Telegramme nicht mehr zeitnah verarbeiten kann.
  *
  * Das Zeichnen des Plots kann mit 1e5 Datenpunkte schon mal 600ms dauern.
  * Dabei ist für das Zeitverhalten wichtig, wie viele Datenpunkte im "Fenster"
  * angezeigt werden, d.h wenn man den Plot aus dem Sichtbereich rausschiebt, geht das schnell.

  * Möglichkeiten:
  *  Die Updaterate des Plots liegt momentan bei 10Hz / 100ms. Die könnte man langsamer
  *  machen, ggf. auch adaptiv d.h. je mehr Samples da sind, desto mehr "flackert" der Plot halt

  * nicht immer alles neu zeichnen sondern nur die Änderungen. Das ist sehr aufwendig mit clipping usw.,
  * kostet viel Zeit bei der Implementierung und dem testen

  * Die Datenpunkte in Abhängigkeit vom zoom level auswählen. Also wenn man
  * weit rein zoomt mehr Datenpunkte, wenn man rauszoomt weniger.

  */

#endif

}

void cairo_plot::pixel2data (int _x, int _y, double &data_x, double &data_y, bool only_scale)
{
  // Breite + Höhe der Achsen in Pixel
  double plot_width_px = w() * (1 - (border_left + border_right));
  double plot_height_px = h() * (1 - (border_top + border_bottom));

  // Umrechnungsfaktor Daten -> Pixel
  // FIXME: könnte man auch aus der cairo transform matrix rauslesen
  double fx =  plot_width_px / (xlim[1] - xlim[0]);
  double fy =  plot_height_px / (ylim[1] - ylim[0]);
  //cout << "fx=" << fx << " fy=" << fy << endl;

  // Umrechnung Pixel -> "Daten" Koy_chann
  if (only_scale)
    {
      data_x = _x / fx;
      data_y = _y / fy;
    }
  else
    {
      data_x = (_x - x() - border_left * w()) / fx;
      data_y = - (_y - y() - (1 - border_bottom) * h()) / fy;
    }
}

// Berücksichtigt immer nur das letzte sample
// clear = true setzt min/max zurück
void cairo_plot::update_stats (bool clear)
{
  for (int k=0; k < NUM_CHANNELS; ++k)
    {
      if (clear)
        {
          data_min[k] = 1.0/0.0;       // Inf
          data_max[k] = - data_min[k]; // -Inf
        }
      else if (num_samples > 0)
        {
          if (data[k][num_samples-1] > data_max[k])
            data_max[k] = data[k][num_samples-1];

          if (data[k][num_samples-1] < data_min[k])
            data_min[k] = data[k][num_samples-1];
        }
    }
}

void cairo_plot::downsample2 ()
{
  //printf ("downsample2, num_samples = %i\n", num_samples);

  if (num_samples >= 2)
    {
      //discard the last sample if odd
      if (num_samples % 2)
        num_samples--;

      for (unsigned int c = 0; c < NUM_CHANNELS; ++c)
        {
          for (unsigned int k = 0; k < num_samples/2; ++k)
            data[c][k] = (data[c][2*k] + data[c][2*k+1]) / 2.0;
        }
      num_samples /= 2;
    }
  //printf ("downsample2, num_samples = %i\n", num_samples);
}

#if NUM_CHANNELS == 3
void cairo_plot::add_data (float x0, float x1, float x2)
{
  static int cnt;
  cnt++;

  // check if the data buffer is full
  if ((num_samples + 1) > MAX_NUM_SAMPLES)
    {
      decimation_factor *= 2;
      downsample2 ();
      cnt = 1;
    }

  data_dec_sum[0] += x0;
  data_dec_sum[1] += x1;
  data_dec_sum[2] += x2;

  if (! (cnt % decimation_factor))
    {
      for (int k = 0; k < NUM_CHANNELS; ++k)
        {
          data[k][num_samples] = data_dec_sum[k] / decimation_factor;
          data_dec_sum[k] = 0;
        }
      num_samples++;
      update_stats ();
    }
}
#endif

int cairo_plot::add_point_marker (double _x, double _y, double _linewidth, double _dia, double red, double green, double blue)
{
  plot_marker.push_back (new point_marker (_x, _y, _linewidth, _dia, red, green, blue));
  return plot_marker.size() - 1;
}

int cairo_plot::add_line_marker (double _pos, line_marker::style _s, double _w, double red, double green, double blue)
{
  plot_marker.push_back (new line_marker (_pos, _s, _w, red, green, blue));
  return plot_marker.size() - 1;
}

void cairo_plot::clear ()
{
  num_samples = 0;
  update_stats (true);
  decimation_factor = 1;
  for (int k = 0; k < NUM_CHANNELS; ++k)
    data_dec_sum[k] = 0;

  for (vector<marker*>::iterator it = plot_marker.begin() ; it != plot_marker.end(); ++it)
    delete (*it);

  plot_marker.clear ();

  redraw ();
}

unsigned int cairo_plot::get_max_ticklen (double start, double step, double stop)
{
  unsigned int len = 0;
  for (double k=start; k<stop; k+=step)
    {
      if (fabs(k) < 10 * std::numeric_limits<double>::epsilon())
        k = 0;
      ostringstream tmp;
      tmp << k;
      if (tmp.str().size () > len)
        len = tmp.str().size ();
    }
  return len;
}


void cairo_plot::set_xtick (double start, double step, double stop)
{
  xtick.clear ();
  for (double k=start; k<stop; k+=step)
    {
      if (fabs(k) < 10 * std::numeric_limits<double>::epsilon())
        k = 0;
      xtick.push_back (k);
    }
  xtick.push_back (stop);
}

void cairo_plot::set_ytick (double start, double step, double stop)
{
  ytick.clear ();
  for (double k=start; k<stop; k+=step)
    {
      if (fabs(k) < 10 * std::numeric_limits<double>::epsilon())
        k = 0;
      ytick.push_back (k);
    }
  ytick.push_back (stop);
}

// Abstände der Gitternetzlinien aus dem anzuzeigenden Bereich
// berechnen
double cairo_plot::tick_from_lim (double r)
{
  // in GNU Octave
  // n = round (3 * log10 (r / 7));
  // ret = (mod(n, 3).^2 + 1) .* 10.^floor(n/3);

  // Attention:
  // Octave mod (n, 3) is not equal to C++ n%3

  int n = round (3 * log (r / 7.0) / log (10));
  double mod3 = n%3;
  if (mod3 < 0)
    mod3 += 3;

  return (pow (mod3, 2) + 1) * pow (10, floor (n / 3.0));
}

void cairo_plot::zoom (double factor)
{
  double new_w =  (xlim[1] - xlim[0]) * factor;
  double new_h =  (ylim[1] - ylim[0]) * factor;

  //printf ("new_w < zoom_max_w = %i\n", new_w < zoom_max_w);
  //printf ("new_w > zoom_min_w = %i\n", new_w > zoom_min_w);
  //printf ("new_h < zoom_max_h = %i\n", new_h < zoom_max_h);
  //printf ("new_h > zoom_min_h = %i\n", new_h > zoom_min_h);

  if (   new_w < zoom_max_w
         && new_w > zoom_min_w
         && new_h < zoom_max_h
         && new_h > zoom_min_h )
    {
      double mean_x = (xlim[0] + xlim[1]) / 2;
      double mean_y = (ylim[0] + ylim[1]) / 2;

      set_xlim (mean_x - new_w/2, mean_x + new_w/2);
      set_ylim (mean_y - new_h/2, mean_y + new_h/2);
      redraw ();
    }
}

// die minimale und maximale "Daten-Breite" des Plots
void cairo_plot::set_zoom_limits (double min_w, double max_w, double min_h, double max_h)
{
  //printf ("set_zoom_limits min_w=%f max_w=%f min_h=%f max_h=%f\n", min_w, max_w, min_h, max_h);
  zoom_min_w = min_w;
  zoom_max_w = max_w;
  zoom_min_h = min_h;
  zoom_max_h = max_h;
}

void cairo_plot::set_xlim (double x0, double x1)
{
  // cout << "set_xlim (" << x0 << ", " << x1 << ")" << endl;

  if (x1 > x0 + zoom_min_w)
    {
      xlim[0] = x0;
      xlim[1] = x1;

      if (xtickmode == AUTO)
        {
          double step  = tick_from_lim (xlim[1] - xlim[0]);
          double start = ceil (xlim[0] / step) * step;
          double stop  = floor (xlim[1] / step) * step;

          // Bei der Abszisse (y_chan nicht da Text horizontal)
          // kann es vorkommen, dass die Achsbeschriftung
          // zu breit wird und die Zahlen sich überlappen.
          // Hier berechnen, wie breit die zu erwartende Achsbeschriftung wird.

          // Anzahl Zeichen
          unsigned int len = get_max_ticklen (start, step, stop);

          int ngrid = round ((stop - start) / step + 1);

          // Die Textgröße ist moemntan fix im Code (cairo_draw_axes) auf 20 gesetzt
          // Ab wann man weniger Gitternetzlinien nehmen muss, hängt auch von der Breite
          // des Plots ab. Hier quick & dirty fix gemacht.

          double width_in_chars = 25;   // Anzahl Ziffern, die im Plot nebeneinander passen
          // ggf. an die Breite des Plots und Schriftgröße anzupassen
          double tmpw = ngrid * len;    // Anzahl Ziffern, die angezeigt werden sollen

          //printf ("len = %i, ngrid = %i, oldstep = %.3f, ", len, ngrid, step);

          double f = width_in_chars / tmpw;
          if (f < 1)
            {
              step = tick_from_lim ((xlim[1] - xlim[0]) / f);
              start = ceil (xlim[0] / step) * step;
              stop  = floor (xlim[1] / step) * step;
            }

          //printf ("f = %.2f, step_new = %.3f\n", f, step);

          set_xtick (start, step, stop);
        }
    }
}

void cairo_plot::get_xlim (double &x0, double &x1)
{
  x0 = xlim[0];
  x1 = xlim[1];
}

void cairo_plot::set_ylim (double y0, double y1)
{
  //cout << "set_ylim (" << y0 << ", " << y1 << ")" << endl;

  if (y1 > y0 + zoom_min_h)
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

void cairo_plot::get_ylim (double &y0, double &y1)
{
  y0 = ylim[0];
  y1 = ylim[1];
}

void cairo_plot::set_drag_mode (edrag_mode d)
{
  drag_mode = d;
}

void cairo_plot::set_xlabel (const string &s)
{
  xlabel = s;
}

void cairo_plot::set_ylabel (const string &s)
{
  ylabel = s;
}

void cairo_plot::set_xunit (const string &s)
{
  xunit = s;
}

void cairo_plot::set_yunit (const string &s)
{
  yunit = s;
}

void cairo_plot::auto_zoom ()
{
  set_xlim (data_min[x_chan], data_max[x_chan]);
  set_ylim (data_min[y_chan], data_max[y_chan] * 1.1);
  redraw ();
}

// alle Werte realtiv: 0.1 -> 10% der Höhe/Breite
void cairo_plot::set_border (double left, double top, double right, double bottom)
{
  border_left = left;
  border_top = top;
  border_right = right;
  border_bottom = bottom;
  auto_zoom ();
  redraw ();
}

void cairo_plot::set_x_chan (int chan)
{
  if (chan < NUM_CHANNELS )
    x_chan = chan;
}

void cairo_plot::set_y_chan (int chan)
{
  if (chan < NUM_CHANNELS)
    y_chan = chan;
}

void cairo_plot::load_csv (const char *fn)
{
  ifstream in (fn);
  if (in.is_open())
    {
      clear ();

      string line;
      int cnt = 0;
      while (std::getline (in, line))
        {
          //cout << "read line" << line << endl;

          stringstream ls (line);
          string cell;

          vector <double> values;

          while (std::getline (ls, cell, ';'))
            {
              values.push_back (stod (cell));
            }

          while (values.size () < NUM_CHANNELS)
            values.push_back (0);

          add_data (values[0], values[1], values[2]);
          cnt++;
        }

      auto_zoom ();

      //cout << "fail =" << in.fail() << endl;
      //cout << "bad =" << in.bad() << endl;
      //cout << "eof =" << in.eof() << endl;

      if (in.fail () && ! in.eof ())
        cerr << "Couldn't read double in line " << cnt << endl;

      cout << "read " << cnt << " lines..." << endl;

      in.close();
    }
  else
    cerr << "Unable to open file '" << fn << "'" << endl;

}

void cairo_plot::save_csv (const char *fn)
{
  ofstream out (fn);
  if (out.is_open())
    {
      for (unsigned int k=0; k < num_samples; ++k)
        {
          for (unsigned int j=0; j < NUM_CHANNELS; ++j)
            {
              out << data[j][k];
              if (j == (NUM_CHANNELS - 1))
                out << std::endl;
              else
                out << ";";
            }
        }

      cout << "cairo_plot::save_csv wrote " << num_samples << " lines into " << fn << endl;
      out.close();
    }
  else
    cerr << "Unable to open file '" << fn << "' for saving plot" << endl;

}

int cairo_plot::handle (int event)
{
  //cout << "event = " << fl_eventnames[event] << endl;
  static int last_x=0, last_y =0;
  switch (event)
    {
    case FL_PUSH:
      if (Fl::event_button () == FL_RIGHT_MOUSE)
        {
          auto_zoom ();
        }
      else
        {
          last_x = Fl::event_x ();
          last_y = Fl::event_y ();
          if (drag_mode == ZOOM)
            {
              // Umrechnung Pixel -> "Daten" Koy_chann Start Zoom
              pixel2data (last_x, last_y, zoom_rect[0], zoom_rect[1], false);
              //cout << "zoom push " << zoom_rect[0] << " " << zoom_rect[1] << endl;
            }
        }
      return 1;

    case FL_DRAG:
      if (Fl::event_button () == FL_LEFT_MOUSE)
        {

          if (drag_mode == PAN)
            {
              int dx = Fl::event_x () - last_x;
              int dy = Fl::event_y () - last_y;
              //cout << "dx=" << dx << " dy=" << dy << endl;

              last_x = Fl::event_x ();
              last_y = Fl::event_y ();

              double xs, ys;
              pixel2data (dx, dy, xs, ys, true);
              //cout << "xs = " << xs << " ys = " << ys << endl;

              set_xlim (xlim[0] - xs, xlim[1] - xs);
              set_ylim (ylim[0] + ys, ylim[1] + ys);
            }
          else if (drag_mode == ZOOM)
            {
              //cout << "ZOOM" << endl;

              // Umrechnung Pixel -> "Daten" Koy_chann
              pixel2data (Fl::event_x (), Fl::event_y (), zoom_rect[2], zoom_rect[3], false);
              zoom_rect[2] -= zoom_rect[0];
              zoom_rect[3] -= zoom_rect[1];
            }

          // Buttons verwenden when(), ich weiß nicht so recht wie wir das
          // hier sinnvoll einsetzen können. Siehe Fl_Button::handle(int event)
          // if (when() & FL_WHEN_CHANGED) do_callback();
          do_callback ();

          redraw ();
          return 1;
        }
      break;

    case FL_RELEASE:
      // Autozoom mit Doppelklick wegen touch vorerst entfernt
      //~ if (Fl::event_button () == FL_LEFT_MOUSE && Fl::event_clicks ())
      //~ {
      //~ //double click
      //~ update_limits ();
      //~ redraw ();
      //~ }

      if (drag_mode == ZOOM)
        {
          //cout << "zoom release x=" << zoom_rect[0] << " y=" << zoom_rect[1] << " w=" << zoom_rect[2] << " h=" << zoom_rect[3] << endl;

          double x1 = zoom_rect[0];
          double y1 = zoom_rect[1];
          double x2 = zoom_rect[0] + zoom_rect[2];
          double y2 = zoom_rect[1] + zoom_rect[3];

          if (x1 > x2)
            {
              double tmp = x1;
              x1 = x2;
              x2 = tmp;
            }

          if (y1 > y2)
            {
              double tmp = y1;
              y1 = y2;
              y2 = tmp;
            }

          //cout << "x1=" << x1 << " x2=" << x2 <<" y1=" << y1 <<" y2=" << y2 << endl;

          set_xlim (xlim[0] + x1, xlim[0] + x2);
          set_ylim (ylim[0] + y1, ylim[0] + y2);
          zoom_rect[2] = zoom_rect[3] = 0;
          redraw ();
        }
      return 1;

    case FL_MOUSEWHEEL:

#define SCALE_FACTOR 5

      double dw = Fl::event_dy ();

      if (dw < (- SCALE_FACTOR + 1))
        {
          dw = - SCALE_FACTOR + 1;
          //cout << "limit dw to " << dw << endl;
        }

      double zoom_factor = 1 + dw/SCALE_FACTOR;
      //cout << "zoom_factor=" << zoom_factor << endl;

      zoom (zoom_factor);

      return 1;
    }
  return 1;
}

// Datenpunkte berechnen, die im Fenster liegen
int cairo_plot::calc_visible_points ()
{
  int ret = 0;
  for (unsigned int k=0; k < num_samples; ++k)
    {
      if (   data[x_chan][k] > xlim[0]
             && data[x_chan][k] < xlim[1]
             && data[y_chan][k] > ylim[0]
             && data[y_chan][k] < ylim[1])
        ret++;
    }

  // quick&dirty mal adaptiv gemacht
  plot_increment = ret/1500;
  if (plot_increment < 1)
    plot_increment = 1;

  return ret;
}
