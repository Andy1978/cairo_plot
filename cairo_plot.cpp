#include "cairo_plot.h"
#include <FL/names.h> //for fl_eventnames
#include <cmath>

cairo_plot::cairo_plot (int x, int y, int w, int h, const char *l)
  : cairo_box (x, y, w, h, l),
    border (0.1),
    linewidth (2),
    gridlinewidth (1),
    xtickmode (AUTO),
    ytickmode (AUTO),
    xlimmode (AUTO),
    ylimmode (AUTO)
{

  //dummy plot
  set_xtick (0, 2, 8);
  set_ytick (0, 1, 6);
  for (double k=0; k<6; k+=0.1)
    add_point (k, 3+sin(k)*2);
}

void cairo_plot::cairo_draw_label (double x, double y, int align, const char *str, double size)
{
  cairo_save (cr);

  size=20;
  double dx = size;
  double dy = size;

  cairo_move_to (cr, x, y);

  print_matrix ();
  cairo_matrix_t tmp;
  cairo_get_matrix (cr, &tmp);
  tmp.xx = 1;
  tmp.yy = 1;
  cairo_set_matrix (cr, &tmp);
  print_matrix ();

  //cairo_scale (cr, 1, -1);

  //~ cairo_device_to_user_distance (cr, &dx, &dy);
//~
  //~ printf ("size=%f, dx=%f, dy=%f align=%i\n", size, dx, dy, align);
  //~ if (dx > -dy)
  //~ cairo_set_font_size (cr, dx);
  //~ else
  //~ cairo_set_font_size (cr, -dy);

  cairo_set_font_size (cr, size);
  cairo_text_extents_t extents;
  cairo_text_extents (cr, str, &extents);

  if (align == 0) //left
    {
      // FIXME: not tested
      //cairo_rel_move_to (cr, - (extents.width/2 + extents.x_bearing), extents.height/2 - extents.y_bearing);
    }
  else if (align == 1)  //top
    {
      cairo_rel_move_to (cr, - (extents.width/2 + extents.x_bearing), extents.height + dy/3);
    }
  else if (align == 2) //right
    {
      cairo_rel_move_to (cr, - (extents.width + extents.x_bearing) - dx/3, - extents.height/2 - extents.y_bearing);
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

  cairo_select_font_face (cr, "Georgia", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);

  for (vector<double>::iterator it = xtick.begin() ; it != xtick.end(); ++it)
    {
      ostringstream tmp;
      tmp << *it;
      cairo_draw_label (*it - xlim[0], 0, 1, tmp.str().c_str (), 15);
    }

  for (vector<double>::iterator it = ytick.begin() ; it != ytick.end(); ++it)
    {
      ostringstream tmp;
      tmp << *it;
      cairo_draw_label (0, *it - ylim[0], 2, tmp.str().c_str (), 15);
    }

  cairo_identity_matrix (cr);
  cairo_restore (cr);
}

void cairo_plot::cairo_draw()
{
  printf ("w=%i h=%i ----------------------------\n", w (), h ());
  cairo_identity_matrix (cr);
  // lower left
  cairo_translate(cr, x (), y () + h ());

  cairo_set_source_rgb(cr, 0.0, 0.0, 0.5);
  cairo_scale (cr, w (), -h ());

  print_matrix ();

  // full plot space 0..1 in X and Y
  // here we could plot some legend

  // limit drawing
  cairo_rectangle (cr, 0, 0, 1, 1);
  cairo_clip (cr);

  // use 10% and 90% as 0..1
  cairo_translate(cr, border, border);
  cairo_scale (cr, 1 - 2 * border, 1 - 2 * border);

  // now scale to xlim and ylim
  cairo_scale (cr, 1/(xlim[1] - xlim[0]), 1/(ylim[1] - ylim[0]));

  cairo_draw_grid ();
  cairo_draw_axes ();

  // Box um die Zeichenfläche
  cairo_save (cr);
  cairo_rectangle (cr, 0, 0, xlim[1] - xlim[0], ylim[1] - ylim[0]);
  cairo_set_line_width (cr, 3);
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

  // draw data
  assert (xdata.size () == ydata.size ());
  vector<double>::iterator xit = xdata.begin();
  vector<double>::iterator yit = ydata.begin();
  cairo_move_to (cr, *xit++ - xlim[0], *yit++ - ylim[0]);

  for (; xit != xdata.end(); xit++, yit++)
    {
      cairo_line_to(cr, *xit - xlim[0], *yit - ylim[0]);
    }

  //cairo_move_to(cr, 0.0, 0.0);
  //cairo_line_to(cr, 8, 5);

  // identity CTM so linewidth is in pixels
  cairo_identity_matrix (cr);
  cairo_set_line_width (cr, linewidth);
  cairo_stroke (cr);

}

void cairo_plot::load_csv (const char *fn, double FS)
{
  double value;
  int cnt = 0;
  ifstream in (fn);
  if (in.is_open())
    {
      clear_points ();
      while (! in.fail ())
        {
          in >> value;
          //cout << value << endl;;
          add_point (cnt/FS, value);
          cnt++;
        }

      //set_xtick (0, 2, trunc (cnt/FS) + 1);
      set_xtick (4, 4, 20);
      set_ytick (5, 10, 55);

      xlim[0] = 3;
      xlim[1] = 21;

      ylim[0] = 3;
      ylim[1] = 55;


      //cout << "fail =" << in.fail() << endl;
      //cout << "bad =" << in.bad() << endl;
      //cout << "eof =" << in.eof() << endl;

      if (in.fail () && ! in.eof ())
        cerr << "Couldn't read double in line " << cnt << endl;
      cout << "read " << cnt << " values..." << endl;
      //while ( getline (in, line) )
      //{
      //  cout << line << '\n';
      //}
      in.close();
      redraw ();
    }
  else
    cout << "Unable to open file '" << fn << "'" << endl;
}

int cairo_plot::handle (int event)
{
  cout << "event = " << fl_eventnames[event] << endl;
  static int last_x=0, last_y =0;
  switch (event)
    {
    case FL_PUSH:
      last_x = Fl::event_x ();
      last_y = Fl::event_y ();
      cout << "last_x=" << last_x << " last_y" << last_y << endl;
      return 1;

    case FL_DRAG:
      if (Fl::event_button () == 1) //PAN
        {
          int dx = Fl::event_x () - last_x;
          int dy = Fl::event_y () - last_y;
          last_x = Fl::event_x ();
          last_y = Fl::event_y ();

          cout << "dx=" << dx << " dy=" << dy << endl;

          // FIXME: könnte man auch aus der cairo transform matrix rauslesen
          double fx = w() * (1 - 2 * border) / (xlim[1] - xlim[0]);
          double fy = h() * (1 - 2 * border) / (ylim[1] - ylim[0]);
          cout << "fx=" << fx << " fy=" << fy << endl;

          set_xlim (xlim[0] - dx/fx, xlim[1] - dx/fx);
          set_ylim (ylim[0] + dy/fy, ylim[1] + dy/fy);

          redraw ();
          return 1;
        }
      break;
    case FL_MOUSEWHEEL:
      cout << "event_dy " << Fl::event_dy () << endl;

      double xw = (xlim[1] - xlim[0]) * Fl::event_dy ();
      set_xlim (xlim[0] - xw/10, xlim[1] + xw/10);

      double yw = (ylim[1] - ylim[0]) * Fl::event_dy ();
      set_ylim (ylim[0] - yw/10, ylim[1] + yw/10);

      redraw ();
      return 1;
    }

}

