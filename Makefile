ifdef SystemRoot
   RM = del /Q
   LIB = -lgdi32
else
   ifeq ($(shell uname), Linux)
      RM = rm -f
      LIB = -L/usr/X11R6/lib -lX11 -lXext
   endif
endif


all: Node.o EdgeHeap.o Graph.o Image.o ez-draw++.o ez-draw.o devoir2.o Prim.o Kruskal.o ez-draw.h ez-draw++.h Node.hpp Graph.hpp Image.h
	g++ -Wall -o devoir2 devoir2.o ez-draw++.o ez-draw.o Node.o Graph.o EdgeHeap.o Image.o Prim.o Kruskal.o $(LIB)

devoir2: devoir2.cpp Graph.o Node.o
	g++ -g -Wall -c devoir2.cpp

ez-draw++: ez-draw++.cpp ez-draw++.h
	g++ -Wall -c ez-draw++.cpp

ez-draw: ez-draw.h ez-draw.c
	gcc -Wall -c ez-draw.c
	
Image: Image.cpp Image.h
	g++ -Wall -c Image.cpp

Graph: Graph.cpp Prim.cpp Kruskal.cpp Graph.hpp
	g++ -Wall -c Graph.cpp Prim.cpp Kruskal.cpp

EdgeHeap: EdgeHeap.cpp EdgeHeap.hpp
	g++ -Wall -c EdgeHeap.cpp

Node: Node.hpp Node.cpp
	g++ -Wall -c Node.cpp

clean:
	$(RM) *.o *.exe
