# data file for the Fltk User Interface Designer (fluid)
version 1.0304
header_name {.h}
code_name {.cxx}
decl {\#include "cairo_plot.h"} {public local
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
    Fl_Box plot {
      xywh {15 15 765 555} box UP_BOX align 64
      class cairo_plot
    }
    Fl_Button {} {
      label {y = 0.5 * x}
      callback {plot->clear ();

for (int k=0;k<10;++k)
  plot->add_data(k, 0.5*k, 0);

plot->auto_zoom ();
plot->redraw ();}
      xywh {800 20 150 40}
    }
    Fl_Button {} {
      label {y = cos (x)}
      callback {plot->clear ();

for (int k=0;k<100;++k)
  {
    double p = k/10.0;
    plot->add_data(p, cos (p), 0);
  }
plot->auto_zoom ();
plot->redraw ();}
      xywh {800 65 150 40}
    }
    Fl_Button {} {
      label {load csv}
      callback {string fn = "Stahlwille_MANOSKOP_730_4_610315061_2.csv";
plot->load_csv (fn.c_str ());}
      xywh {800 110 150 40}
    }
    Fl_Input vi_xlabel {
      label xlabel
      callback {plot->set_xlabel (o->value());
plot->redraw ();}
      xywh {800 195 145 25} align 5 when 1
    }
    Fl_Input vi_ylabel {
      label ylabel
      callback {plot->set_ylabel (o->value());
plot->redraw ();}
      xywh {800 245 145 25} align 5 when 1
    }
  }
}
