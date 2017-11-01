
all: Node.o Graph.o devoir2.o Node.hpp Graph.hpp
	g++ -Wall -o devoir2 devoir2.o Node.o Graph.o

devoir2: devoir2.cpp Graph.o Node.o
	g++ -Wall -c devoir2.cpp

Graph: Graph.cpp Graph.hpp
	g++ -Wall -c Graph.cpp

Node: Node.hpp Node.cpp
	g++ -Wall -c Node.cpp

clean:
	rm -f % *.o *.exe

