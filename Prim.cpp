#include "Graph.hpp"
#include "EdgeHeap.hpp"

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
					//cout << ancien->vertices[j]->getName() << " linked with " << arbre->vertices[k]->getName() << endl;
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
			arbre->addEdge(arbre->vertices[ arbre->vertex_count - 1 ]->getName(), arbre->getNode(to->getName())->getName(), min);
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

Graph* Graph::primHeap()
{
    int vertex_index = 0;
    cout << endl << "primHeap :" << endl;
   
    // ARBRE => On crée un arbre à partir du premier sommet (duquel on garde que le nom) ;
    cout << "Arbre (init) :" << endl;
    Graph* arbre = new Graph();
    arbre->addNode(vertices[vertex_index]->getName(), vertices[vertex_index]->getX(),
                                                      vertices[vertex_index]->getY());
    arbre->showGraph();
   
    //ANCIEN => On garde tous les autres sommets pour en faire un arbre (on garde les arrêtes mêmes si elles sont sur l'arbre couvrant min) ;
    cout << "Ancien (init) :" << endl;
    Graph* ancien = new Graph();
    for (int i = 1 ; i < vertex_count ; i++)
        ancien->addNode(vertices[i]);
    ancien->showGraph();
   
    // Initialisation du tas_min d'arcs
    cout << "EdgeHeap (init) :" << endl;
    EdgeHeap* e_heap = new EdgeHeap();
    
    int edge_count = 0;
    while (edge_count < vertex_count-1)
    {
        //cout << vertex_index << endl;
        // Ajout de toutes les aretes du dernier sommet ajouté
        // Seulement si le sommet associé n'est PAS dans ARBRE
        if (vertex_index >= 0)
            for (uint i = 0 ; i < vertices[vertex_index]->getEdgeCount() ; i++)
                if (arbre->getNodeNumber(vertices[vertex_index]->getEdge(i).vertex->getName()) < 0)
                {
                    //cout << "EdgeHeap push : " << vertices[vertex_index]->getEdge(i).weight <<endl;
                    e_heap->push(vertices[vertex_index]->getEdge(i));
                }
        cout << "EdgeHeap :" << endl;
        e_heap->displayHeap();
        // Sortie du sommet de e_heap
        Node::Edge to_add = e_heap->pop();
        // Test si le vertex associé est déjà présent dans ARBRE
        vertex_index = arbre->getNodeNumber(to_add.vertex->getName());

        if (vertex_index < 0)
        {
            vertex_index = getNodeNumber(to_add.vertex->getName());

            arbre->addNode(vertices[vertex_index]->getName(),
                           vertices[vertex_index]->getX(),
                           vertices[vertex_index]->getY());
           /* cout << vertices[vertex_index]->getName() << "["
                 << vertices[vertex_index]->getX() << ","
                 << vertices[vertex_index]->getY() << "] ajouté" << endl;
            cout << "vertex_index" << endl;
            cout << to_add.origin->getName() << endl;
            cout     << to_add.vertex->getName()<< endl;
            cout     << to_add.weight << "]" << endl;*/
            arbre->addEdge(to_add.origin->getName(), to_add.vertex->getName(), to_add.weight);

            edge_count++;

            ancien->removeNode(vertices[vertex_index]);
        }
        else
            vertex_index = -1;
        cout << "Arbre min :" << endl;
        arbre->showGraph();
        cout << endl;
    }
 
    //cout << "Arbre min :" << endl;
    //arbre->showGraph();
 
    return arbre;
}
