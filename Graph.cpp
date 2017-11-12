#include "Graph.hpp"
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

Graph* Graph::primBasic()
{
	cout << endl << "primBasic :" << endl;
	
	// ARBRE => On crée un arbre à partir du premier sommet (duquel on garde que le nom) ;
	cout << "Arbre (init) :" << endl;
	Graph* arbre = new Graph();
	arbre->addNode(vertices[0]->getName(), vertices[0]->getX(), vertices[0]->getY());
	arbre->showGraph();
	
	//ANCIEN => On garde tous les autres sommets pour en faire un arbre (on garde les arrêtes mêmes si elles sont sur l'arbre couvrant min) ;
	cout << "Ancien (init) :" << endl;
	Graph* ancien = new Graph();
	for (int i = 1 ; i < vertex_count ; i++)
		ancien->addNode(vertices[i]);
	ancien->showGraph();
	
	// Réitère jusqu'à plus rester de sommets
	while (arbre->vertex_count < vertex_count)
	{
		uint min = 9999;
		Node* from = NULL;
		Node* to = NULL;
		
		// On prend toutes les arêtes de ANCIEN pointant vers un sommet de ARBRE
		for (uint j = 0 ; j < ancien->vertex_count ; j++)
		{
			for (uint k = 0 ; k < arbre->vertex_count ; k++)
			{
				if (ancien->vertices[j]->isLinkedWith (arbre->vertices[k]))
				{
					cout << ancien->vertices[j]->getName() << " linked with " << arbre->vertices[k]->getName() << endl;
					uint weight = ancien->vertices[j]->getEdgeWeight(arbre->vertices[k]);
					
					// On en sélectionne la minimale
					if ( weight < min)
					{
						from = ancien->vertices[j];
						to = arbre->vertices[k];
						min = weight;
					}
				}
			}
		}
		
		if (from != NULL)
		{
			// Le sommet reliant est ajouté (sans arête du tout)
			arbre->addNode(from->getName(), from->getX(), from->getY());
			// On ajoute l'arête reliante aux deux sommets
			arbre->vertices[ arbre->vertex_count - 1 ]->addEdge(arbre->getNode(to->getName()), min);
		}
		ancien->removeNode (from);
		
		cout << "Arbre min :" << endl;
		arbre->showGraph();
		
		cout << "Ancien restant :" << endl;
		ancien->showGraph();
		cout << endl;
	}
	
	return arbre;
}

void swap (Node::Edge* T, int a, int b)
{
	Node::Edge tmp = T[a];
	T[a] = T[b];
	T[b] = tmp;
}

void quick_sort (Node::Edge* T, int g, int d)
{
	float v;
	int a = g; int b = d;
	if (g < d)
	{
		v = T[(g+d)/2].weight;
		while (a <= b)
		{
			while (T[a].weight < v) a++;
			while (T[b].weight > v) b--;
			if (a <= b)
			{
				swap(T, a, b);
				a++; b--;
			}
		}
		quick_sort(T, g, b);
		quick_sort(T, a, d);
	}
}

Graph* Graph::kruskalBasic()
{
	cout << endl << "kruskalBasic :" << endl;
	
	// ARBRE => On crée un arbre vide
	Graph* arbre = new Graph();
	
	// LISTE => Liste des arêtes
	list<Node::Edge> edge_list;
	
	// /!\ Attention toutes les aretes sont en doubles dans le tableau, et donc dans la liste
	Node::Edge tab [getEdgeCount()*2];
	int tab_size = 0;
	for (uint i = 0 ; i < vertex_count ; i++)
		for (uint j = 0 ; j < vertices[i]->getEdgeCount() ; j++)
			tab[tab_size++] = vertices[i]->getEdge(j);
			
	/*for (uint i = 0 ; i < getEdgeCount()*2 ; i++)
		cout << "Weight " << i << " : " << tab[i].weight << endl;*/
	
	// Les trie dans l'ordre croissant
	quick_sort(tab, 0, getEdgeCount()*2 - 1);
	
	for (uint i = 0 ; i < getEdgeCount()*2 ; i++)
		cout << "Weight " << i << " : " << tab[i].weight << endl;
	
	cout << "Max : " << edge_list.max_size () << endl;
	
	for (uint i = 0 ; i < getEdgeCount()*2 ; i++)
		edge_list.push_back (tab[i]);
		
	// 1ère arête
	// Ajoute les deux sommets à ARBRE (sans arête)
	Node* vertex = edge_list.front().vertex;
	arbre->addNode(vertex->getName (), vertex->getX(), vertex->getY());
	edge_list.pop_front();
	vertex = edge_list.front().vertex;
	arbre->addNode(vertex->getName (), vertex->getX(), vertex->getY());
	// Lie les deux par leur arête reliante
	arbre->vertices[0]->addEdge(arbre->vertices[1], edge_list.front().weight);
	// pop l'arête min de LISTE
	edge_list.pop_front();
	
	arbre->showGraph();
	
	// ===> Réitère jusqu'à plus rester de sommets
	while (arbre->vertex_count < vertex_count)
	{
		Node* node1 = arbre->getNode (edge_list.front().vertex->getName ());
		Node* vertex1 = edge_list.front().vertex;
		edge_list.pop_front();
		
		Node* node2 = arbre->getNode (edge_list.front().vertex->getName ());
		Node* vertex2 = edge_list.front().vertex;
		int weight = edge_list.front().weight;
		edge_list.pop_front();
		
		// Vérif si UN SEUL des deux sommets de l'arête fait partie de ARBRE* 
		// Si oui : Ajoute le nouveau sommet et l'arête reliante
		if ((node1 != NULL) && (node2 == NULL))
		{
			cout << "Trouve : " << node1->getName() << endl;
			arbre->addNode (vertex2->getName(), vertex2->getX(), vertex2->getY());
			arbre->getNode(node1->getName())->addEdge(arbre->getNode(vertex2->getName()), weight);
		}
		if ((node1 == NULL) && (node2 != NULL))
		{
			cout << "Trouve : " << node2->getName() << endl;
			arbre->addNode (vertex1->getName(), vertex1->getX(), vertex1->getY());
			arbre->getNode(node2->getName())->addEdge(arbre->getNode(vertex1->getName()), weight);
		}
		// Si non : Ajoute les deux sommets à ARBRE & l'arête
		if ((node1 == NULL) && (node2 == NULL))
		{
			cout << "Aucun trouve" << endl;
			arbre->addNode (vertex1->getName(), vertex1->getX(), vertex1->getY());
			arbre->addNode (vertex2->getName(), vertex2->getX(), vertex2->getY());
			arbre->getNode(vertex1->getName())->addEdge(arbre->getNode(vertex2->getName()), weight);
		}
		
		// Vérif si les deux sommets de l'arête font partie de ARBRE
		// Pas trop compris cette partie la
		// TODO : completer
		// Si non : Ajoute le sommet et l'arête puis continue
		if ((node1 != NULL) && (node2 != NULL))
		{
			cout << "2 noeuds trouves" << endl;
			Node* from = arbre->getNode(node1->getName());
			Node* to = arbre->getNode(node2->getName());
			// Si oui : Vérifie par un parcours en profondeur (par ses arêtes) 
				//si le sommet1 atteint sommet2 dans ARBRE
			if ((from->DepthFirstSeach (to)) || (to->DepthFirstSeach (from)))
				// Si trouvé : Ignore
				cout << "OK" << endl;
			else
				// Sinon : Ajoute l'arête
				from->addEdge(to, weight);
		}
		
		cout << "Arbre min : " << endl;
		arbre->showGraph();
	}
	
	
	while (!edge_list.empty())
	{
		cout << "Pop front : " << edge_list.front().weight << endl;
		edge_list.pop_front();
	}
	
	return arbre;
	
}

int Graph::getRoot(int* tab, int index)
{
    if (tab[index] == index)
        return tab[index];
    return getRoot(tab, tab[index]);
}

Graph* Graph::kruskalForest()
{
	cout << endl << "kruskalForest :" << endl;
	
	// ARBRE => On crée un arbre vide
	Graph* arbre = new Graph();
	
	// LISTE => Liste des arêtes
	list<Node::Edge> edge_list;
	
	// FATHERS => Tableau des pères
	int fathers [vertex_count];
	
	// DEPTH => Profondeur des sommets
	int depth [vertex_count];
	
	for (uint i = 0 ; i < vertex_count ; i++)
	{
		fathers[i] = i;
		depth[i] = 1;
	}
	
	Node::Edge tab [getEdgeCount()*2];
	int tab_size = 0;
	for (uint i = 0 ; i < vertex_count ; i++)
		for (uint j = 0 ; j < vertices[i]->getEdgeCount() ; j++)
		{
			bool present = false;
			// Verification de la presence de l'arete inverse
			for (uint k = 0 ; k < tab_size ; k++)
				if ( (tab[k].vertex->getName() == vertices[i]->getEdge(j).origin->getName()) 
						&& (tab[k].origin->getName() == vertices[i]->getEdge(j).vertex->getName()) )
							present = true;

			if (!present)
					tab[tab_size++] = vertices[i]->getEdge(j);
		}
			
			
	for (uint i = 0 ; i < getEdgeCount() ; i++)
		cout << "Weight " << i << " : " << tab[i].weight << endl;
	
	// Les trie dans l'ordre croissant
	quick_sort(tab, 0, getEdgeCount() - 1);
	
	for (uint i = 0 ; i < getEdgeCount() ; i++)
		cout << "Weight " << i << " : " << tab[i].weight << endl;
	
	for (uint i = 0 ; i < getEdgeCount() ; i ++)
		edge_list.push_back (tab[i]);
	
	// Ajoute dans l'arbre les sommets du graphe, sans aretes
	for (uint i = 0 ; i < vertex_count ; i++)
		arbre->addNode (getVertex(i)->getName(), getVertex(i)->getX(), getVertex(i)->getY());
	
	uint edge_count = 0;
	while (edge_count < vertex_count - 1)
	{
		Node* vertex1 = edge_list.front().origin;
		int weight1 = edge_list.front().weight;
		//edge_list.pop_front();
		
		Node* vertex2 = edge_list.front().vertex;
		int weight2 = edge_list.front().weight;
		edge_list.pop_front();
		
		int vertex1number = getNodeNumber (vertex1);
		int vertex2number = getNodeNumber (vertex2);
		int maxDepth = (depth[vertex2number] > depth[vertex1number]? vertex2number:vertex1number);
		int minDepth = (depth[vertex2number] <= depth[vertex1number]? vertex2number:vertex1number);
		Node* son = arbre->getNode((minDepth == vertex1number? vertex1:vertex2)->getName() );
		Node* father = arbre->getNode((maxDepth == vertex1number? vertex1:vertex2)->getName() );
		int son_root = arbre->getRoot (fathers, arbre->getNodeNumber (arbre->getNode(son->getName())));
		int father_root = arbre->getRoot (fathers, arbre->getNodeNumber (arbre->getNode(father->getName())));
		
		cout << weight1 << son->getName() << " root (son): " << arbre->getRoot (fathers, arbre->getNodeNumber (arbre->getNode(son->getName()))) << endl;
		cout << weight1 << father->getName() << " root (father): " << arbre->getRoot (fathers, arbre->getNodeNumber (arbre->getNode(father->getName()))) << endl;
		
		// Verification de la prescence de boucle
		if ( son_root != father_root )
		{
		
			arbre->getNode(father->getName())->addEdge(arbre->getNode(son->getName()), weight1 );
			edge_count++;
			
			if ( depth[father_root] >= depth[son_root])
				fathers[son_root] = father_root;
			else
			{
				int tmp = depth[father_root];
				depth[father_root] = depth[son_root];
				depth[son_root] = tmp;
				fathers[son_root] = father_root;
			}
			if (depth[son_root] + 1 > depth[father_root])
				depth[father_root] = depth[son_root] + 1;
			
		}
		else
			cout << "Boucle detectee" << endl;
		
		
		/*cout << "Arbre min : " << endl;
		arbre->showGraph();*/
		
		/*cout << "Tabs : " << endl;
		for (uint i = 0 ; i < vertex_count ; i++)
			cout << arbre->getVertex(i)->getName() << " : Pere : " << fathers[i]
				<< " : Depth : " << depth[i]
				<< " : Root : " << arbre->getRoot (fathers, i) << endl;
		cout << endl;*/
	}
	
	
	while (!edge_list.empty())
	{
		//cout << "Pop front : " << edge_list.front().weight << endl;
		edge_list.pop_front();
	}
	
	return arbre;
	
}
