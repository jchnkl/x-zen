LIBS=xcb
CXXFLAGS=$(shell pkg-config --cflags ${LIBS}) -std=c++11 -Wall -O0 -g
LDFLAGS=$(shell pkg-config --libs ${LIBS})

SRCS=main.cpp

INCS=x/event.hpp \
		 x/interface.hpp \
		 x/connection.hpp \
		 x/drawable.hpp \
		 x/window.hpp \
		 zen/client.hpp

OBJS=$(SRCS:%.cpp=%.o)

EXE=x:zen

all: ${INCS} ${OBJS}
	${CXX} ${CXXFLAGS} ${LDFLAGS} ${INCS} ${OBJS} -o ${EXE}

clean:
	rm -f ${EXE} ${OBJS}

.PHONY: clean
