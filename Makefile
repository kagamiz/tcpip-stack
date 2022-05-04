CXX=g++
CFLAGS=-g -std=c++17 -Wall
TARGET:test.out
LIBS= -L ./CommandParser -lcli -lrt -lpthread
OBJS=graph.o \
	 topologies.o \
	 net.o \
	 color.o \
	 nwcli.o

test.out:testapp.o ${OBJS} CommandParser/libcli.a
	${CXX} ${CFLAGS} testapp.o ${OBJS} -o test.out ${LIBS}

testapp.o:testapp.cpp
	${CXX} ${CFLAGS} -c testapp.cpp -o testapp.o

graph.o:graph.cpp
	${CXX} ${CFLAGS} -c -I . graph.cpp -o graph.o

topologies.o:topologies.cpp
	${CXX} ${CFLAGS} -c -I . topologies.cpp -o topologies.o

net.o:net.cpp
	${CXX} ${CFLAGS} -c -I . net.cpp -o net.o

color.o:color.cpp
	${CXX} ${CFLAGS} -c -I . color.cpp -o color.o

nwcli.o:nwcli.cpp
	${CXX} ${CFLAGS} -c -I . -o nwcli.o nwcli.cpp

CommandParser/libcli.a:
	(cd CommandParser; make)

clean:
	rm -f *.o
	rm -f *.out
	(cd CommandParser; make clean)

all:
	make
	(cd CommandParser; make)
