#include "Graph.hpp"

int main()
{
	Graph* graph = new Graph();
	graph->addNode("kek");
	graph->addNode("ladylel");
	graph->addNode("sigurdowhen?!");
	graph->addEdge("kek","ladylel",999);
	graph->addEdge("kek","sigurdowhen?!",420);
	graph->showGraph();
	cout << "Nombre d'arrêtes : " << graph->getEdgeCount() << endl;
	delete graph;
	return 0;
}
