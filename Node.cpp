
#include "Node.hpp"

Node::Node(string _name, int _x, int _y)
: name(_name), x(_x), y(_y), edge_capacity(0), edge_count(0)
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
		new_edge.origin = this;
		new_edge.vertex = node;
		new_edge.weight = _weight;
		edges [edge_count++] = new_edge;
	}
}

bool Node::isLinkedWith (Node* vertex)
{
	for (uint i = 0 ; i < edge_count ; i++)
	{
		if (edges[i].vertex->getName() == vertex->getName())
			return true;
	}
	
	return false;
}

bool Node::DepthFirstSeach (Node* vertex)
{
	if (getName() == vertex->getName())
			return true;
	
	for (uint i = 0 ; i < edge_count ; i++)
	{
		if (edges[i].vertex->DepthFirstSeach(vertex))
			return true;
	}
	
	return false;
}

int Node::getEdgeWeight (Node* vertex)
{
	int weight_min = 9999;
	for (uint i = 0 ; i < edge_count ; i++)
	{
		if (edges[i].vertex->getName() == vertex->getName())
			if (edges[i].weight < weight_min)
				weight_min = edges[i].weight;
	}
	
	return weight_min;
}

void Node::showNode () 
{
	if (edge_count < 1)
		cout << name << "\t" << "-" << "\t" << "-" << endl;
	for (uint i = 0 ; i < edge_count ; i++)
		cout << name << "\t" << edges[i].vertex->getName() << "\t" << edges[i].weight << endl;
}
