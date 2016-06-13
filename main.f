# data file for the Fltk User Interface Designer (fluid)
version 1.0304
header_name {.h}
code_name {.cxx}
decl {\#include "cairo_plot.h"} {public local
}

decl {\#include "cairo_star.h"} {public local
}

decl {\#include <fstream>} {public local
}

decl {\#include <string>} {public local
}

Function {} {open
} {
  Fl_Window mainwin {
    label {start test} open
    xywh {2615 492 960 575} type Double resizable visible
  } {
    Fl_Box plot {selected
      xywh {15 15 620 555} box UP_BOX align 64
      class cairo_plot
    }
    Fl_Box {} {
      xywh {645 305 300 255} box UP_BOX align 64
      class cairo_star
    }
    Fl_Button {} {
      label {y = 0.5 * x}
      callback {plot->clear_points ();

for (int k=0;k<10;++k)
  plot->add_point(k, 0.5*k);

plot->redraw ();}
      xywh {645 20 195 40}
    }
    Fl_Button {} {
      label {y = cos (x)}
      callback {plot->clear_points ();

for (int k=0;k<100;++k)
  {
    double p = k/10.0;
    plot->add_point(p, cos (p));
  }
plot->redraw ();}
      xywh {645 65 195 40}
    }
    Fl_Button {} {
      label {load file}
      callback {string fn = "Stahlwille_MANOSKOP_730_4_610315061_2.csv";
plot->load_csv (fn.c_str (), 900);}
      xywh {645 110 150 35}
    }
  }
}
