
#include "Node.hpp"

Node::Node(string _name)
: name(_name), edge_capacity(0), edge_count(0)
{
	edges = NULL;
}

Node::~Node()
{
	delete[] edges;
	edges = NULL;
}

void Node::addEdge(Node* node, int _weight)
{
	if (node != NULL)
	{
		if (edges == NULL)
		{
			edge_capacity = EDGES_CAPACITY_DEFAULT;
			edges = new Edge[edge_capacity];
		}
		else if (edge_capacity <= edge_count+1)
		{
			Edge* new_edges = new Edge[edge_capacity*2];
			for (uint i = 0 ; i < edge_capacity ; i++)
				new_edges[i] = edges[i];
			edge_capacity *= 2;
			edges = new_edges;
		}
		Edge new_edge;
		new_edge.vertex = node;
		new_edge.weight = _weight;
		edges [edge_count++] = new_edge;
	}
}

bool Node::isLinkedWith(Node* vertex)
{
	for (int i = 0 ; i < edge_count ; i++)
	{
		if (edges[i].vertex->getName() == vertex->getName())
			return true;
	}
	
	return false;
}

int Node::getEdgeWeight(Node* vertex)
{
	for (int i = 0 ; i < edge_count ; i++)
	{
		if (edges[i].vertex->getName() == vertex->getName())
			return edges[i].weight;
	}
	
	return 9999;
}

void Node::showNode () 
{
	if (edge_count < 1)
		cout << name << "\t" << "-" << "\t" << "-" << endl;
	for (int i = 0 ; i < edge_count ; i++)
		cout << name << "\t" << edges[i].vertex->getName() << "\t" << edges[i].weight << endl;
}
