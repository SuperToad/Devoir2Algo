#include <string>
#include <iostream>

using namespace std;
typedef unsigned int uint;

const uint EDGES_CAPACITY_DEFAULT = 5;

class Node {
	public:		
		typedef struct{
			Node* vertex;
			int weight;
		} Edge;

	private:
		string name;		// Valeur du noeud
		Edge* edges;	// Liste des arrètes
		uint edge_capacity;	// Capacité du tableau de pointeurs edges
		uint edge_count;		// Nombre d'enfants total dans edges
	public:
		 Node(string _name = "");
		~Node();
		 bool isLinkedWith(Node* vertex); // Renseigne la possibilité d'une liaison avec @vertex
		 void addEdge(Node* node, int weight); // Ajoute un enfant au noeud
		 void showNode ();
		  
		 inline string getName() {return name;}
		 inline uint getEdgeCount() {return edge_count;}
};
