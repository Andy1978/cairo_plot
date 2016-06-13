#include "cairo_star.h"
#include <cmath>

cairo_star::cairo_star (int x, int y, int w, int h, const char *l)
  : cairo_box (x, y, w, h, l)
{

}

void cairo_star::star(double radius)
{
  double theta = 0.8*M_PI;
  cairo_save(cr);
  cairo_move_to(cr, 0.0, -radius);
  for(int i=0; i<5; i++)
    {
      cairo_rotate(cr, theta);
      cairo_line_to(cr, 0.0, -radius);
    }
  cairo_fill(cr);
  cairo_restore(cr);
}

void cairo_star::cairo_draw()
{
  double f = 1.0/(1.0+sin(0.3*M_PI));
  cairo_translate(cr, x()+w()/2, y()+f*h());
  double radius  = f*h();
  double srink[] = {1.0, 0.95, 0.85, 0.75};
  for(int i = 0; i<4; i++)
    {
      if(i % 2)
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.5);
      else
        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
      star(srink[i]*radius);
    }
}
