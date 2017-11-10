#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "Node.hpp"

typedef unsigned int uint;

#define MAX_CAPACITY 1024

class EdgeHeap{
	private:
		Edge* heap;
		int heap_size;
		int heap_capacity;
		
		void deleteElement(int index);
	public:
		EdgeHeap(Edge* _heap, int _heap_size);
		~EdgeHeap();
		
		void push(Edge edge);
		Edge pop();
		Edge head();
		void organize(uint i, uint n);
		void displayHeap();
};

