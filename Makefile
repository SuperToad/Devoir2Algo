all: Node.o Graph.o Image.o ez-draw++.o ez-draw.o devoir2.o ez-draw.h ez-draw++.h Node.hpp Graph.hpp Image.h
	g++ -Wall -o devoir2 devoir2.o ez-draw++.o ez-draw.o Node.o Graph.o Image.o -L/usr/X11R6/lib -lX11 -lXext

devoir2: devoir2.cpp Graph.o Node.o 
	g++ -Wall -c devoir2.cpp

ez-draw++: ez-draw++.cpp ez-draw++.h
	g++ -Wall -c ez-draw++.cpp

ez-draw: ez-draw.h ez-draw.c
	gcc -Wall -c ez-draw.c
	
Image: Image.cpp Image.h
	g++ -Wall -c Image.cpp

Graph: Graph.cpp Graph.hpp
	g++ -Wall -c Graph.cpp

Node: Node.hpp Node.cpp
	g++ -Wall -c Node.cpp

clean:
	rm -f % *.o *.exe
