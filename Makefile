LIBS=xcb xcb-keysyms xcb-icccm
CXXFLAGS=-std=c++11 -Wall -O${OPTLEVEL}
CXXFLAGS+=$(shell pkg-config --cflags ${LIBS})
LDFLAGS=$(shell pkg-config --libs ${LIBS})

OPTLEVEL=3

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

debug: CXXFLAGS+=-g -D_GLIBCXX_DEBUG
debug: OPTLEVEL=0
debug: all

%.hpp.gch: %.hpp
	rm -f $(<:%.hpp=%.o)
	${CXX} ${CXXFLAGS} -c $<

clean:
	rm -f ${EXE} ${HPPOBJS} ${CPPOBJS}

.PHONY: clean
