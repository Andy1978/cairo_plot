.PHONY:clean style all

TARGETS = main.cpp main.h main Stahlwille_MANOSKOP_730_4_610315061_2.csv
CPPFLAGS = -Wall -Wextra -ggdb `fltk-config --use-cairo --cxxflags` -D USE_X11 -D FLTK_HAVE_CAIRO
#LDFLAGS = `fltk-config --use-cairo --ldflags` -lusb-1.0 -lsqlite3 -lcairo -lconfuse
LDFLAGS = `fltk-config --use-cairo --ldflags`

all: $(TARGETS)

main.cpp main.h: main.f
	fluid -o .cpp -c $<

%.o:%.cpp %.h
	g++ $(CPPFLAGS) -c $<

main: main.o cairo_box.o cairo_plot.o cairo_star.o
	g++ $(CPPFLAGS) $^ -o $@ $(LDFLAGS)

style:
	find . \( -name "*.m" -or -name "*.c" -or -name "*.cpp" -or -name "*.h" -or -name "Makefile" \) -exec sed -i 's/[[:space:]]*$$//' {} \;
	find . \( -name "*.c" -or -name "*.cpp" -or -name "*.h" \) -exec astyle --style=gnu -s2 -n {} \;

Stahlwille_MANOSKOP_730_4_610315061_2.csv: Stahlwille_MANOSKOP_730_4_610315061_2.tar.gz
	tar xzf $^
	touch $@

clean:
	find . -name "octave-workspace" -exec rm {} \;
	rm -f $(TARGETS)
	rm -f *.log *.o

