#include "Graph.hpp"
#include "ez-draw++.h"

EZDraw ezDraw;

int main()
{
	Graph* graph = new Graph();
	graph->addNode("kek");
	graph->addNode("ladylel");
	graph->addNode("sigurdowhen?!");
	graph->addNode("bbq");
	graph->addEdge("kek","ladylel",999);
	graph->addEdge("kek","sigurdowhen?!",420);
	graph->addEdge("ladylel","sigurdowhen?!",1000);
	graph->addEdge("ladylel","bbq",50);
	graph->addEdge("kek","kek",1000);
	graph->addEdge("kek","bbq",500);
	graph->addEdge("bbq","kek",250);
	graph->addEdge("bbq","sigurdowhen?!",100);
	graph->showGraph();
	cout << "Nombre d'arrêtes : " << graph->getEdgeCount() << endl;
	
	graph->primBasic();
	graph->kruskalBasic();
	
	// GUI tests
	EZWindow win(400, 300, "Demo++ 0 : Hello World");
	ezDraw.mainLoop();
	
	delete graph;
	return 0;
}
