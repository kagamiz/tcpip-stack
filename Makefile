CXX=g++
CFLAGS=-g -std=c++17 -Wall
TARGET:test.out

OBJS=graph.o

test.out:testapp.o ${OBJS}
	${CXX} ${CFLAGS} testapp.o ${OBJS} -o test.out

testapp.o:testapp.cpp
	${CXX} ${CFLAGS} -c testapp.cpp -o testapp.o

graph.o:graph.cpp
	${CXX} ${CFLAGS} -c -I . graph.cpp -o graph.o

clean:
	rm *.o
	rm *.out
