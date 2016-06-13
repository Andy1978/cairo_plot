#ifndef CAIRO_STAR_H
#define CAIRO_STAR_H

#include "cairo_box.h"

class cairo_star : public cairo_box
{
private:

public:
  cairo_star(int x, int y, int w, int h, const char *l=0);

  void cairo_draw();
  void star(double radius);
};

#endif
