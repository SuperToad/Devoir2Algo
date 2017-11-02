
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
		int getNodeNumber(Node* node);
	public:
		Graph();
		~Graph();
		void addNode(string name);
		void addEdge(string name1, string name2, int weight);
		void showGraph();
		Node* getNode(string name);
		int getEdgeCount();
		
		// -- PRIM
		Graph* primBasic();
		//Graph* primHeap();
		
		// -- KRUSKAL
		bool sortEdges(const Node::Edge& first, const Node::Edge& second);
		Graph* kruskalBasic();
		// Graph* kruskalForest();
};
