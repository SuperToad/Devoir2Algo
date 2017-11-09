#include "Node.hpp"

#define MAX_CAPACITY 1024

class EdgeHeap{
	private:
		Edge* heap;
		int heap_size;
		int heap_capacity;
	public:
		EdgeHeap(Edge* _heap, int _heap_size);
		~EdgeHeap();
		
		void push(Edge edge);
		Edge pop();
		Edge head();
		void organize();
};

