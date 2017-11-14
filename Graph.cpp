#include "Graph.hpp"
#include "Prim.hpp"
#include "Kruskal.hpp"

#include <algorithm>
#include <functional>
#include <iostream>
#include <iterator>
#include <list>


Graph::Graph()
{
	vertex_capacity = DEFAULT_MAX_NODE;
	vertex_count = 0;
	vertices = new Node*[vertex_capacity];
}

Graph::~Graph()
{
	delete[] vertices;
}

void Graph::addNode (string name, int x, int y)
{
	if (vertex_count < vertex_capacity)
		vertices[vertex_count++] = new Node (name, x, y);
}

void Graph::addEdge(string name1, string name2, int weight)
{
	Node* from = getNode (name1);
	Node* to = getNode (name2);
	
	if ( (from != NULL) && (to != NULL) )
	{
		// On admet un graph non orienté
		from->addEdge (to, weight);
		to->addEdge(from, weight);
	}
}

Node* Graph::getNode(string name)
{
	Node* result = NULL;
	
	for (int i = 0; (i < vertex_count) && (result == NULL); i++)
		if (vertices[i]->getName() == name)
			result =  vertices[i];
	
	return result;
}

void Graph::showGraph()
{
	bool shownNodes[vertex_count];
	for (uint i = 0 ; i < vertex_count ; i++)
		vertices[i]->showNode();
}

int Graph::getEdgeCount()
{
	int sum = 0;
	for (uint i = 0 ; i < vertex_count ; i++)
		sum += vertices[i]->getEdgeCount();
	return sum/2;
}

void Graph::addNode(Node* node)
{
	if (vertex_count+1 < vertex_capacity)
		vertices[vertex_count++] = node;
}

void Graph::removeNode(Node* node)
{
	int toremove = getNodeNumber(node);
	for (uint i = toremove ; i < vertex_count-1 ; i++)
		vertices[i] = vertices[i+1];
	vertex_count--;
}

int Graph::getNodeNumber(Node* node)
{
	int number = -1;
	for (uint i = 0 ; i < vertex_count && number == -1 ; i++)
		number = (vertices[i] == node)?i:number;
	return number;
}

int Graph::getNodeNumber(string name)
{
    int result = -1;
    
    for (int i = 0; (i < vertex_count) && (result == -1); i++)
        if (vertices[i]->getName() == name)
            result =  i;
    
    return result;
}


