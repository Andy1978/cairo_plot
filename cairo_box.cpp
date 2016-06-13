/*

Copyright (C) 2015 Alluris GmbH & Co. KG <weber@alluris.de>

class cairo_box: Draw with cairo into a Fl_Box.

This file is part of TTT_certify.

TTT_certify is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TTT_certify is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  See ../COPYING
If not, see <http://www.gnu.org/licenses/>.

*/

#include <cairo-svg.h>
#include <cairo-ps.h>
#include <cairo-pdf.h>
#include <FL/fl_draw.H>
#include "cairo_box.h"
#include <cmath>

cairo_box::cairo_box (int x, int y, int w, int h, const char *l) : Fl_Box (x, y, w, h, l)
{
  surface  = NULL;
  cr       = NULL;
}

cairo_surface_t*  cairo_box::set_surface(int wo, int ho)
{
#ifdef WIN32
#warning win32 mode
  /* Get a Cairo surface for the current DC */
  HDC dc = fl_gc;                                     /* Exported by fltk */
  return cairo_win32_surface_create(dc);
#elif defined (__APPLE__)
#warning Apple Quartz mode
  /* Get a Cairo surface for the current CG context */
  CGContext *ctx = fl_gc;
  return cairo_quartz_surface_create_for_cg_context(ctx, wo, ho);
#else
  /* Get a Cairo surface for the current display */
  return cairo_xlib_surface_create(fl_display, fl_window, fl_visual->visual, wo, ho);
#endif
}

void cairo_box::draw(void)
{
  // using fltk functions, set up white background with thin black frame
  fl_color(FL_WHITE);
  fl_rectf(x(), y(), w(), h());
  fl_color(FL_BLACK);
  fl_rect(x(), y(), w(), h());

  //fl_push_clip(100,100,100,100);
  //fl_color(FL_BLACK);
  //fl_line(1,1,parent()->w(),parent()->h());

  // set up cairo structures
  surface = set_surface(parent()->w(), parent()->h());
  cr      = cairo_create(surface);
  cairo_set_source_rgb (cr, 0.0, 0.0, 0.0); // set drawing color to black
  cairo_new_path(cr);

  printf ("x()=%i, y()=%i, w()=%i, h()=%i\n", x(), y(), w(), h());
  cairo_draw ();

  // release the cairo context
  cairo_destroy(cr);
  cairo_surface_destroy(surface);
}
