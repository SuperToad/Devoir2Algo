#include "Graph.hpp"
#include "Image.h"
#include "ez-draw++.h"
#include <sstream>
#include <string>

#include <stdio.h>
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <cmath>

#define NB_SOMMETS_GENERES 15

typedef unsigned short int ushort;
typedef unsigned int uint;
typedef unsigned char ubyte;

using namespace std;

EZDraw ezDraw;

class MyWindow : public EZWindow 
{
public:
	MyWindow(int w,int h,const char *name, Graph* graph)
	: EZWindow(w,h,name), initGraph (graph), drawGraph (graph)
	{
		selected1 = -1;
		selected2 = -1;
		ultrametrics = new int[graph->getVertexCount()*graph->getVertexCount()];
		isNotReset = false;
		// redGraph = new Graph();
	}

	~MyWindow()
	{
		if (ultrametrics != NULL)
			delete ultrametrics;
	}
	
	Graph* initGraph;
	Graph* drawGraph;
	// Graph* redGraph;
	int* ultrametrics;
	int selected1;
	int selected2;
	bool isNotReset;

	void write_ultrametrics (int ultrametrics[], int vertex_count)
	{
		cout << "Ultrametriques : " << endl;
		
		string data;
		ofstream outfile;
		outfile.exceptions ( std::ofstream::failbit | std::ofstream::badbit );
		
		try {
			outfile.open("ultrametrics.txt");
			
			if (!outfile)
				throw std::runtime_error("Erreur d'ouverture du fichier d'ecriture");

			cout << "Ecriture du fichier..." << endl;

			outfile << '\t';
			for (uint i = 0 ; i < vertex_count ; i++)
				outfile << drawGraph->getVertex(i)->getName() << '\t';

			outfile << endl << drawGraph->getVertex(0)->getName() << '\t';	
			for (uint i = 0 ; i < vertex_count*vertex_count ; i++)
			{
				outfile << ultrametrics[i] << "\t";
				if ((i+1)%vertex_count == 0 && (i+1 < vertex_count*vertex_count)) 
					outfile << endl << drawGraph->getVertex((i+1)/vertex_count)->getName() << '\t';
			}
			
			outfile.close();
			
		}catch (exception &e) {
	        cerr << e.what() << std::endl;
	    }
		
		cout << "Voir ultrametrics.txt pour les resultats" << endl;
	}

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

	// Trace l'arête entre les deux noeuds passés en commentaire et utilise le poids en paramètre
	void trace_simple_edge(Node* from, Node* to, int weight)
	{
		setColor (ez_green);
		ostringstream oss;
		oss << weight;
		drawText(EZ_TL, (from->getX() + to->getX())/2-10,
			(from->getY() + to->getY())/2-10, oss.str());
		drawLine(from->getX(),from->getY(),to->getX(),to->getY());
	}
	// Permet de juste passer les coordonnées dans le tableau de mesures ultramétriques
	void trace_simple_edge(int node1, int node2, int weight)
	{
		Node* from = drawGraph->getVertex(node1);
		Node* to = drawGraph->getVertex(node2);
		trace_simple_edge(from,to,weight);
	}
	
	void trace_graph ()
	{
		int weight = 0;
		for(int i = 0; i < drawGraph->getVertexCount(); i ++)
		{
			Node* vertex = drawGraph->getVertex(i);
			setColor (ez_black);
			drawText(EZ_TL, vertex->getX(), vertex->getY(), vertex->getName());
			for (int j = 0 ; j < vertex->getEdgeCount() ; j++)
			{
				Node* to = vertex->getEdge(j).vertex;
				ostringstream oss;
				oss << vertex->getEdge(j).weight;
				setColor (ez_red);
				drawText(EZ_TL, (vertex->getX() + to->getX())/2,
					(vertex->getY() + to->getY())/2 - 20, oss.str());
					
				setColor (ez_grey);
				drawLine(vertex->getX(), vertex->getY(), to->getX(), to->getY());
				
				weight += vertex->getEdge(j).weight;
			}
			
		}

		if (selected1 != -1 && selected2 != -1 && ultrametrics[0] == 0 && isNotReset)
			trace_simple_edge(selected1,selected2, ultrametrics[
								selected1+selected2*drawGraph->getVertexCount()]);

		setColor (ez_red);
		ostringstream oss;
		oss << "Weight : " << weight/2;
		drawText(EZ_TR, getWidth()-2, 1, oss.str());
	}

	void expose()
	{
		setColor (ez_black);
		setFont(1);
		trace_graph ();
		setColor (ez_blue);
		drawText(EZ_BL, 2, getHeight()-2, "O : Prim    P : Prim(Tas)\nJ : Kruskal K : Kruskal(Forets)\nU : Ultrametriques\nR : Reset    E : Reset selection\nQ : Quit");
	}
	
	// Bouton de la souris enfonce :
	void buttonPress(int mouse_x,int mouse_y,int button)
	{
		cout << "ButtonPress      win = " << this << " mouse_x = " << mouse_x << " mouse_y = " << mouse_y << " button = " << button << endl ;
		
		int min_dist = 999999;
		int index = -1;
		for(int i = 0; i < drawGraph->getVertexCount(); i ++)
		{
			double x1 = mouse_x;
			double y1 = mouse_y;
			double x2 = drawGraph->getVertex(i)->getX ();
			double y2 = drawGraph->getVertex(i)->getY ();
			
			double dist = sqrt((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1));
			//cout << dist << endl;
			if (dist < min_dist)
			{
				min_dist = dist;
				index = i;
			}
		}
		if (selected1 == -1)
			selected1 = index;
		else if (selected2 == -1)
			selected2 = index;
		sendExpose();
		// redGraph->addNode (node->getName(), node->getX (), node->getY ());
		
		// for(int i = 0; i < redGraph->getVertexCount(); i ++)
		// 	redGraph->addEdge( node->getName(), redGraph->getVertex(i)->getName(), 0 );
			
		// if (redGraph->getVertexCount() > 2)
		// {
		// 	drawGraph = redGraph;
		// 	sendExpose();
		// }
	}

	void keyPress(KeySym keysym)
	{
		switch (keysym)
		{
			case XK_Escape:
			case XK_q  :
				ezDraw.quit();
				break;
			case XK_o :
				drawGraph = initGraph->primBasic();
				isNotReset = true;
				sendExpose();
				break;
			case XK_p  :
				drawGraph = initGraph->primHeap();
				isNotReset = true;
				sendExpose();
				break;
			case XK_j  :
				drawGraph = initGraph->kruskalBasic();
				isNotReset = true;
				sendExpose();
				break;
			case XK_k  :
				drawGraph = initGraph->kruskalForest();
				isNotReset = true;
				sendExpose();
				break;
			case XK_u  :
				drawGraph = initGraph->kruskalForestUltrametrique(ultrametrics);
				write_ultrametrics (ultrametrics, drawGraph->getVertexCount());
				isNotReset = true;
				sendExpose();
				break;
			case XK_e  :
				selected1 = -1;
				selected2 = -1;
				sendExpose();
				break;
			case XK_r  :
				drawGraph = initGraph;
				selected1 = -1;
				selected2 = -1;
				isNotReset = false;
				sendExpose();
				break;
		}
	}
};

Graph* generate_graph (GrayImage *image)
{
	Graph* graph = new Graph();
	
	char name_c = 'A';
	
	while (graph->getVertexCount() < NB_SOMMETS_GENERES)
	{
		int randx = EZDraw::random (image->getWidth ());
		int randy = EZDraw::random (image->getHeight ());
		
		if ((int)image->pixel (randx, randy) < 50)
		{
			bool too_close = false;
			
			for(int i = 0; i < graph->getVertexCount(); i++)
			{
				double x1 = graph->getVertex(i)->getX ();
				double y1 = graph->getVertex(i)->getY ();
				double x2 = randx;
				double y2 = randy;
				
				double dist = sqrt((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1));
				//cout << dist << endl;
				if (dist < 30)
					too_close = true;
			}
			if (!too_close)
			{
				stringstream name;
				name << name_c;
				graph->addNode(name.str(), randx, randy);
				
				name_c++;
			}
		}
		
	}
	
	for(int i = 0; i < graph->getVertexCount(); i++)
		for(int j = i; j < graph->getVertexCount(); j++)
			if ( graph->getVertex(i)->getName() != graph->getVertex(j)->getName() )
			{
				double x1 = graph->getVertex(i)->getX ();
				double y1 = graph->getVertex(i)->getY ();
				double x2 = graph->getVertex(j)->getX ();
				double y2 = graph->getVertex(j)->getY ();
				
				double dist = sqrt((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1));
				graph->addEdge(graph->getVertex(i)->getName(),
					graph->getVertex(j)->getName(), dist);
			}
	
	return graph;
}

int main()
{
	
	// Test de lecture
	ifstream readTest("image.pgm");
	GrayImage *image = NULL;
	
	if (readTest == NULL)
	{
		image = new GrayImage (600, 800);
		image->clear(250);
		image->rectangle(5, 6, 10, 40, 0);
		image->fillRectangle(55, 56, 100, 200, 10);
		image->fillRectangle(305, 250, 150, 200, 10);
		
		ofstream writeTest("image.pgm");
		image->writePGM(writeTest);
	}
	else 
		image = image->readPGM(readTest);
	
	cout << " Image : Width : " << image->getWidth() << ", Height : " << image->getHeight() << endl;
	
	Graph* new_graph = generate_graph (image);
	
	// GUI
	MyWindow win(image->getWidth(), image->getHeight(), "Algo Prim et Kuskal", new_graph);
	ezDraw.mainLoop();
	
	delete new_graph;
	return 0;
}
