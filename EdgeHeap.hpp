#ifndef EDGEHEAP_H
#define EDGEHEAP_H

#include "Node.hpp"

typedef unsigned int uint;
typedef Node::Edge Edge;

#define MAX_CAPACITY 1024

class EdgeHeap{
	private:
		Edge* heap;
		uint heap_size;
		uint heap_capacity;
		
		int lowerChild(uint i, uint n);
		int deleteElement(uint index);
	public:
		EdgeHeap(Edge* _heap = NULL, uint _heap_size = 0);
		~EdgeHeap();
		
		int push(Edge edge);
		Edge pop();
		Edge head();
		int organize(uint i, uint n);
		void displayHeap();

		inline bool isEmpty() {return (heap_size == 0);}
};

#endif