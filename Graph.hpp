#ifndef GRAPH_H
#define GRAPH_H

#include "Node.hpp"

#define DEFAULT_MAX_NODE 100

class Graph
{
	private:
		Node** vertices;
		uint vertex_count;	
		uint vertex_capacity;
		
		void addNode(Node* node);
		void removeNode(Node* node);
	public:
		Graph();
		~Graph();
		void addNode(string name, int x, int y);
		void addEdge(string name1, string name2, int weight);
		void showGraph();
		Node* getNode(string name);
		int getEdgeCount();
		int getNodeNumber(Node* node);
		int getNodeNumber(string name);
		inline Node* getVertex(int i) {return vertices[i];}
		inline uint getVertexCount() {return vertex_count;}
		int getRoot(int* tab, int index);
		
		// -- PRIM
		Graph* primBasic();
		Graph* primHeap();
		
		// -- KRUSKAL
		bool sortEdges(const Node::Edge& first, const Node::Edge& second);
		Graph* kruskalBasic();
		Graph* kruskalForest();
};

#endif
