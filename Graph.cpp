#include "Graph.hpp"

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

void Graph::addNode (string name)
{
	if (vertex_count < vertex_capacity)
		vertices[vertex_count++] = new Node (name);
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

Graph* Graph::primBasic()
{
	cout << endl << "primBasic :" << endl;
	
	// ARBRE => On crée un arbre à partir du premier sommet (duquel on garde que le nom) ;
	cout << "Arbre (init) :" << endl;
	Graph* arbre = new Graph();
	arbre->addNode(vertices[0]->getName());
	arbre->showGraph();
	
	//ANCIEN => On garde tous les autres sommets pour en faire un arbre (on garde les arrêtes mêmes si elles sont sur l'arbre couvrant min) ;
	cout << "Ancien (init) :" << endl;
	Graph* ancien = new Graph();
	for (int i = 1 ; i < vertex_count ; i++)
		ancien->addNode(vertices[i]);
	ancien->showGraph();
	
	// Réitère jusqu'à plus rester de sommets
	for (uint i = 0 ; i < vertex_count - 1 ; i++)
	{
		uint min = 9999;
		Node* node = NULL;
		
		// On prend toutes les arêtes de ANCIEN pointant vers un sommet de ARBRE
		for (uint j = 0 ; j < arbre->vertex_count ; j++)
			if (ancien->vertices[0]->isLinkedWith (arbre->vertices[j]))
			{
				cout << ancien->vertices[0]->getName() << " linked with " << arbre->vertices[j]->getName() << endl;
				uint weight = ancien->vertices[0]->getEdgeWeight(arbre->vertices[j]);
				
				// On en sélectionne la minimale
				if ( weight < min)
				{
					node = arbre->vertices[j];
					min = weight;
				}
			}
		
		if (node != NULL)
		{
			// Le sommet reliant est ajouté (sans arête du tout)
			arbre->addNode(node->getName());
			// On ajoute l'arête reliante aux deux sommets
			arbre->vertices[ arbre->vertex_count - 1 ]->addEdge(ancien->vertices[0], min);
		}
		ancien->removeNode (ancien->vertices[0]);
		
		cout << "Arbre min :" << endl;
		arbre->showGraph();
		
		cout << "Ancien restant :" << endl;
		ancien->showGraph();
		cout << endl;
	}
}

