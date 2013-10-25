LIBS=xcb xcb-keysyms xcb-icccm
CXXFLAGS=-std=c++11 -Wall -O0 -g \
				 $(shell pkg-config --cflags ${LIBS})
LDFLAGS=$(shell pkg-config --libs ${LIBS})

CPPSRCS=main.cpp

HPPSRCS=generic_accessor.hpp \
				x/event.hpp \
				x/cursor.hpp \
				x/request.hpp \
				x/requests.hpp \
				x/interface.hpp \
				x/connection.hpp \
				x/drawable.hpp \
				x/window.hpp \
				zen/interface.hpp \
				zen/algorithm.hpp \
				zen/event.hpp \
				zen/pointer.hpp \
				zen/client.hpp \
				zen/client_snap.hpp \
				zen/client_wm_size_hints.hpp \
				zen/client_manager.hpp

CPPOBJS=$(CPPSRCS:%.cpp=%.o)
HPPOBJS=$(HPPSRCS:%.hpp=%.hpp.gch)

EXE=x:zen

all: ${HPPOBJS} ${CPPOBJS}
	${CXX} ${CXXFLAGS} ${LDFLAGS} ${CPPOBJS} -o ${EXE}

%.hpp.gch: %.hpp
	rm -f $(<:%.hpp=%.o)
	${CXX} ${CXXFLAGS} -c $<

clean:
	rm -f ${EXE} ${HPPOBJS} ${CPPOBJS}

.PHONY: clean
