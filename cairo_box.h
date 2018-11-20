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

/*
 This class was inspired from David M. Allens' CairoBox
 https://blog.as.uky.edu/allen/?page_id=677
 Original licence: "Feel free to use this code in any way, but at your own risk."
*/

#ifndef CAIROBOX
#define CAIROBOX

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <cairo.h>

#ifdef WIN32
#  include <cairo-win32.h>
#elif defined (__APPLE__)
#  include <cairo-quartz.h>
#else
#  include <cairo-xlib.h>
#endif

#include <iostream>
#include <cstdio>

class cairo_box : public Fl_Box
{
private:
  void draw(void);
  cairo_surface_t*  surface;
  cairo_surface_t*  set_surface(int wo, int ho);
protected:
  cairo_t*          cr;
public:
  cairo_box(int x, int y, int w, int h, const char *l=0);

  void print_matrix ()
  {
    cairo_matrix_t matrix;
    cairo_get_matrix (cr, &matrix);
    printf ("\n");
    printf ("| xx=%8.3f xy=%8.3f x0=%8.3f |\n", matrix.xx, matrix.xy, matrix.x0);
    printf ("| xy=%8.3f yy=%8.3f y0=%8.3f |\n", matrix.yx, matrix.yy, matrix.y0);
    printf ("|    %8.3f    %8.3f    %8.3f |\n", 0.0, 0.0, 1.0);
  }
  virtual void resize (int x, int y, int w, int h);
  virtual void cairo_draw() = 0;
};

#endif

