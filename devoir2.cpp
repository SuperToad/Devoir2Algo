#include "Graph.hpp"
#include "ez-draw++.h"
#include <sstream>
#include <string>

using namespace std;

EZDraw ezDraw;

class MyWindow : public EZWindow 
{
public:
	MyWindow(int w,int h,const char *name, Graph* graph)
	: EZWindow(w,h,name), initGraph (graph), drawGraph (graph)
	{}
	
	Graph* initGraph;
	Graph* drawGraph;

	void trace_graph_basic ()
	{
		for(int i = 0; i < drawGraph->getVertexCount(); i ++)
		{
			Node* vertex = drawGraph->getVertex(i);
			if (vertex->getEdgeCount() < 1)
				drawText(EZ_TL, i*100 + 10, i*150 + 10, vertex->getName());
			else
				for (int j = 0 ; j < vertex->getEdgeCount() ; j++)
				{
					ostringstream oss;
					oss << drawGraph->getVertex(i)->getName() << " -> " <<
						drawGraph->getVertex(i)->getEdge(j).vertex->getName() << " : " << drawGraph->getVertex(i)->getEdge(j).weight;
					drawText(EZ_TL, i*50 + 10, i*50 + j*20 + 10, oss.str());
					
				}
		}
	}
	
	void trace_graph ()
	{
		int weight = 0;
		for(int i = 0; i < drawGraph->getVertexCount(); i ++)
		{
			Node* vertex = drawGraph->getVertex(i);
			drawText(EZ_TL, vertex->getX(), vertex->getY(), vertex->getName());
			for (int j = 0 ; j < vertex->getEdgeCount() ; j++)
			{
				Node* to = vertex->getEdge(j).vertex;
				ostringstream oss;
				oss << vertex->getEdge(j).weight;
				drawText(EZ_TL, (vertex->getX() + to->getX())/2,
					(vertex->getY() + to->getY())/2 - 20, oss.str());
				
				drawLine(vertex->getX(), vertex->getY(), to->getX(), to->getY());
				
				weight += vertex->getEdge(j).weight;
			}
			
		}
		setColor (ez_red);
		ostringstream oss;
		oss << "Weight : " << weight;
		drawText(EZ_TR, getWidth()-2,             1, oss.str());
	}

	void expose()
	{
		setColor (ez_black);
		setFont(1);
		/*for(int i = 0; i <= 3; i++)
		{
			setFont(i); // Choix d'une des 4 polices de caracteres differente a chaque tour de boucle.
			ostringstream oss; // On cree un std::ostringstream.
			oss << "Fonte numero " << i ; // On ecrit dedans.
			drawText(
				EZ_TC, // Texte aligne en haut et centre horizontalement.
				getWidth()/2, 90 + 25 * i, // Position de l'ecriture.
				oss.str() // On recupere le contenu de oss sous forme d'une std::string qu'on
				);          // transmet a drawText().
		}

		setFont(0); // Choix de la police de caracteres par defaut (0).
		setColor(ez_red);
		drawText(EZ_TL,            2,             1, "Top\nLeft");
		drawText(EZ_TC, getWidth()/2,             1, "Top\nCenter");
		drawText(EZ_TR, getWidth()-2,             1, "Top\nRight");
		drawText(EZ_ML,            2, getHeight()/2, "Middle\nLeft");
		drawText(EZ_MR, getWidth()-2, getHeight()/2, "Middle\nRight");
		drawText(EZ_BL,            2, getHeight()-2, "Bottom\nLeft");
		drawText(EZ_BC, getWidth()/2, getHeight()-2, "Bottom\nCenter");
		drawText(EZ_BR, getWidth()-2, getHeight()-2, "Bottom\nRight");
		// Les fonctions membres getWidth() et getHeight() permettent d'obtenir la largeur
		// et hauteur actuelle de la fenetre courrante.*/
		trace_graph ();
		setColor (ez_blue);
		drawText(EZ_BL, 2, getHeight()-2, "P : Prim\nK : Kruskal\nR : Reset\nQ : Quit");
	}

	void keyPress(KeySym keysym)
	{
		switch (keysym)
		{
			case XK_Escape:
			case XK_q  :
				ezDraw.quit();
				break;
			case XK_p  :
				drawGraph = initGraph->primBasic();
				sendExpose();
				break;
			case XK_k  :
				drawGraph = initGraph->kruskalForest();
				sendExpose();
				break;
			case XK_r  :
				drawGraph = initGraph;
				sendExpose();
				break;
		}
	}
};

int main()
{
	Graph* graph = new Graph();
	graph->addNode("kek", 50, 100);
	graph->addNode("ladylel", 500, 400);
	graph->addNode("sigurdowhen?!", 40, 450);
	graph->addNode("bbq", 100, 300);
	
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
	graph->kruskalForest();
	
	// GUI tests
	MyWindow win(800, 600, "Algo Prim et Kuskal", graph);
	ezDraw.mainLoop();
	
	delete graph;
	return 0;
}
