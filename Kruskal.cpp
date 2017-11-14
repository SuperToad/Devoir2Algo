#include "Graph.hpp"
#include <algorithm>
#include <functional>
#include <iostream>
#include <iterator>
#include <list>

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
			
	/*for (uint i = 0 ; i < getEdgeCount()*2 ; i++)
		cout << "Weight " << i << " : " << tab[i].weight << endl;*/
	
	// Les trie dans l'ordre croissant
	quick_sort(tab, 0, getEdgeCount() - 1);
	
	for (uint i = 0 ; i < getEdgeCount() ; i++)
		cout << "Weight " << i << " : " << tab[i].weight << endl;
	
	for (uint i = 0 ; i < getEdgeCount() ; i++)
		edge_list.push_back (tab[i]);
	
	// ===> Réitère jusqu'à plus rester de sommets
	while (arbre->getEdgeCount() < vertex_count -1 && !edge_list.empty())
	{
		Node* node1 = arbre->getNode (edge_list.front().origin->getName ());
		Node* vertex1 = edge_list.front().origin;
		
		Node* node2 = arbre->getNode (edge_list.front().vertex->getName ());
		Node* vertex2 = edge_list.front().vertex;
		int weight = edge_list.front().weight;
		edge_list.pop_front();
		
		// Vérif si UN SEUL des deux sommets de l'arête fait partie de ARBRE* 
		// Si oui : Ajoute le nouveau sommet et l'arête reliante
		if ((node1 != NULL) && (node2 == NULL))
		{
			//cout << "Trouve : " << node1->getName() << endl;
			arbre->addNode (vertex2->getName(), vertex2->getX(), vertex2->getY());
			//arbre->getNode(node1->getName())->addEdge(arbre->getNode(vertex2->getName()), weight);
			arbre->addEdge (node1->getName(), vertex2->getName(), weight);
		}
		else if ((node1 == NULL) && (node2 != NULL))
		{
			//cout << "Trouve : " << node2->getName() << endl;
			arbre->addNode (vertex1->getName(), vertex1->getX(), vertex1->getY());
			//arbre->getNode(node2->getName())->addEdge(arbre->getNode(vertex1->getName()), weight);
			arbre->addEdge (node2->getName(), vertex1->getName(), weight);
		}
		// Si non : Ajoute les deux sommets à ARBRE & l'arête
		else if ((node1 == NULL) && (node2 == NULL))
		{
			//cout << "Aucun trouve" << endl;
			arbre->addNode (vertex1->getName(), vertex1->getX(), vertex1->getY());
			arbre->addNode (vertex2->getName(), vertex2->getX(), vertex2->getY());
			//arbre->getNode(vertex1->getName())->addEdge(arbre->getNode(vertex2->getName()), weight);
			arbre->addEdge (vertex1->getName(), vertex2->getName(), weight);
		}
		
		// Vérif si les deux sommets de l'arête font partie de ARBRE
		// Si non : Ajoute le sommet et l'arête puis continue
		else if ((node1 != NULL) && (node2 != NULL))
		{
			//cout << "2 noeuds trouves" << endl;
			Node* from = arbre->getNode(node1->getName());
			Node* to = arbre->getNode(node2->getName());
			// Si oui : Vérifie par un parcours en profondeur (par ses arêtes) 
				//si le sommet1 atteint sommet2 dans ARBRE
			//cout << "Avant test" << endl;
			if (from->DepthFirstSeach (to,NULL)) //|| (to->DepthFirstSeach (from)))
				// Si trouvé : Ignore
				;//cout << "OK" << endl;
			else
			{
				// Sinon : Ajoute l'arête
				//cout << "Ajout" << endl;
				//from->addEdge(to, weight);
				arbre->addEdge (vertex1->getName(), vertex2->getName(), weight);
				
			}
		}
		
		cout << "Arbre min : " << endl;
		arbre->showGraph();
	}
	
	//cout << "Fini !" << endl;
	
	while (!edge_list.empty())
	{
		//cout << "Pop front : " << edge_list.front().weight << endl;
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
		int weight = edge_list.front().weight;
		//edge_list.pop_front();
		
		Node* vertex2 = edge_list.front().vertex;
		//int weight2 = edge_list.front().weight;
		edge_list.pop_front();
		
		int vertex1number = getNodeNumber (vertex1);
		int vertex2number = getNodeNumber (vertex2);
		int maxDepth = (depth[vertex2number] > depth[vertex1number]? vertex2number:vertex1number);
		int minDepth = (depth[vertex2number] <= depth[vertex1number]? vertex2number:vertex1number);
		Node* son = arbre->getNode((minDepth == vertex1number? vertex1:vertex2)->getName() );
		Node* father = arbre->getNode((maxDepth == vertex1number? vertex1:vertex2)->getName() );
		int son_root = arbre->getRoot (fathers, arbre->getNodeNumber (arbre->getNode(son->getName())));
		int father_root = arbre->getRoot (fathers, arbre->getNodeNumber (arbre->getNode(father->getName())));
		
		//cout << weight << son->getName() << " root (son): " << arbre->getRoot (fathers, arbre->getNodeNumber (arbre->getNode(son->getName()))) << endl;
		//cout << weight << father->getName() << " root (father): " << arbre->getRoot (fathers, arbre->getNodeNumber (arbre->getNode(father->getName()))) << endl;
		
		// Verification de la prescence de boucle
		if ( son_root != father_root )
		{
			//arbre->getNode(father->getName())->addEdge(arbre->getNode(son->getName()), weight );
			arbre->addEdge(father->getName(), son->getName(), weight );
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
			;//cout << "Boucle detectee" << endl;
		
		
		cout << "Arbre min : " << endl;
		arbre->showGraph();
		
		cout << "Tabs : " << endl;
		for (uint i = 0 ; i < vertex_count ; i++)
			cout << arbre->getVertex(i)->getName() << " : Pere : " << fathers[i]
				<< " : Depth : " << depth[i]
				<< " : Root : " << arbre->getRoot (fathers, i) << endl;
		cout << endl;
	}
	
	
	while (!edge_list.empty())
	{
		//cout << "Pop front : " << edge_list.front().weight << endl;
		edge_list.pop_front();
	}
	
	return arbre;
	
}

int getUltrametrique(int* array, int vertex_count, int x, int y)
{
	return array[y*vertex_count + x];
}

int setUltrametrique(int* array, int vertex_count, int x, int y, int valeur)
{
	array[y*vertex_count + x] = valeur;
}

Graph* Graph::kruskalForestUltrametrique(int* ultrametrics)
{
	cout << endl << "kruskalForestUltrametrique :" << endl;
	
	// ARBRE => On crée un arbre vide
	Graph* arbre = new Graph();
	
	// LISTE => Liste des arêtes
	list<Node::Edge> edge_list;
	
	// FATHERS => Tableau des pères
	int fathers [vertex_count];
	
	// DEPTH => Profondeur des sommets
	int depth [vertex_count];
	
	// ULTRAMETRIQUES => Tableau des pères
	//ultrametrics = new int[vertex_count*vertex_count];
	for (uint i = 0 ; i < vertex_count*vertex_count ; i++)
		ultrametrics[i] = 0;
	
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
		int weight = edge_list.front().weight;
		//edge_list.pop_front();
		
		Node* vertex2 = edge_list.front().vertex;
		//int weight2 = edge_list.front().weight;
		edge_list.pop_front();
		
		int vertex1number = getNodeNumber (vertex1);
		int vertex2number = getNodeNumber (vertex2);
		int maxDepth = (depth[vertex2number] > depth[vertex1number]? vertex2number:vertex1number);
		int minDepth = (depth[vertex2number] <= depth[vertex1number]? vertex2number:vertex1number);
		Node* son = arbre->getNode((minDepth == vertex1number? vertex1:vertex2)->getName() );
		Node* father = arbre->getNode((maxDepth == vertex1number? vertex1:vertex2)->getName() );
		int son_root = arbre->getRoot (fathers, arbre->getNodeNumber (arbre->getNode(son->getName())));
		int father_root = arbre->getRoot (fathers, arbre->getNodeNumber (arbre->getNode(father->getName())));
		
		//cout << weight << son->getName() << " root (son): " << arbre->getRoot (fathers, arbre->getNodeNumber (arbre->getNode(son->getName()))) << endl;
		//cout << weight << father->getName() << " root (father): " << arbre->getRoot (fathers, arbre->getNodeNumber (arbre->getNode(father->getName()))) << endl;
		
		// Verification de la prescence de boucle
		if ( son_root != father_root )
		{

			// Ultrametrie
			for (uint i = 0 ; i < vertex_count ; ++i)
				if (getRoot(fathers,i) == son_root)
				{
					for (uint j = 0 ; j < vertex_count ; ++j)
						if (getRoot(fathers,j) == father_root)
						{
							//cout << "lol" << endl;
							if (getUltrametrique(ultrametrics, vertex_count, i, j) == 0)
							{
								setUltrametrique(ultrametrics, vertex_count, i, j, weight);
								setUltrametrique(ultrametrics, vertex_count, j, i, weight);
							}
						}
					//fathers[i] = father_root;
				}
			//arbre->getNode(father->getName())->addEdge(arbre->getNode(son->getName()), weight );
			arbre->addEdge(father->getName(), son->getName(), weight );
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
			;//cout << "Boucle detectee" << endl;
		
		
		/*cout << "Arbre min : " << endl;
		arbre->showGraph();
		
		cout << "Tabs : " << endl;
		for (uint i = 0 ; i < vertex_count ; i++)
			cout << arbre->getVertex(i)->getName() << " : Pere : " << fathers[i]
				<< " : Depth : " << depth[i]
				<< " : Root : " << arbre->getRoot (fathers, i) << endl;
		cout << endl;*/
	}
	
	cout << "Ultrametriques : " << endl;
	for (uint i = 0 ; i < vertex_count*vertex_count ; i++)
	{
		cout << ultrametrics[i] << "\t";
		if ((i+1)%vertex_count == 0) cout << endl;
	}
	cout << endl;
	
	while (!edge_list.empty())
	{
		//cout << "Pop front : " << edge_list.front().weight << endl;
		edge_list.pop_front();
	}
	
	return arbre;
	
}
