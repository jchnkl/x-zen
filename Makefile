LIBS=xcb xcb-keysyms
CXXFLAGS=-std=c++11 -Wall -O0 -g \
				 $(shell pkg-config --cflags ${LIBS})
LDFLAGS=$(shell pkg-config --libs ${LIBS})

CPPSRCS=main.cpp

HPPSRCS=x/event.hpp \
				x/request.hpp \
				x/requests.hpp \
				x/interface.hpp \
				x/connection.hpp \
				x/drawable.hpp \
				x/window.hpp \
				zen/pointer.hpp \
				zen/client.hpp \
				zen/client_manager.hpp

CPPOBJS=$(CPPSRCS:%.cpp=%.o)
HPPOBJS=$(HPPSRCS:%.hpp=%.hpp.gch)

EXE=x:zen

all: ${HPPOBJS} ${CPPOBJS}
	${CXX} ${CXXFLAGS} ${LDFLAGS} ${CPPOBJS} -o ${EXE}

%.hpp.gch: %.hpp
	rm -f ${CPPOBJS}
	${CXX} ${CXXFLAGS} -c $<

clean:
	rm -f ${EXE} ${HPPOBJS} ${CPPOBJS}

.PHONY: clean
