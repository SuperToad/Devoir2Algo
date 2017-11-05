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
		int x;
		int y;
	public:
		 Node(string _name = "", int _x = 0, int _y = 0);
		~Node();
		 bool isLinkedWith(Node* vertex); // Renseigne la possibilité d'une liaison avec @vertex
		 bool DepthFirstSeach(Node* vertex); // Renvoir si il existe un chemin vers @vertex
		 int getEdgeWeight(Node* vertex); // Renvoie le poids du noeud en commun avec @vertex, s'il existe)
		 void addEdge(Node* node, int weight); // Ajoute un enfant au noeud
		 void showNode ();
		  
		 inline string getName() {return name;}
		 inline int  getX() {return x;}
		 inline int  getY() {return y;}
		 inline uint getEdgeCount() {return edge_count;}
		 inline Edge getEdge(int i) {return edges[i];}
};

