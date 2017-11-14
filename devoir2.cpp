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
		drawText(EZ_BL, 2, getHeight()-2, "O : Prim    P : Prim(Tas)\nJ : Kruskal K : Kruskal(Forets)\nU : Ultrametriques\nR : Reset\nQ : Quit");
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
				sendExpose();
				break;
			case XK_p  :
				drawGraph = initGraph->primHeap();
				sendExpose();
				break;
			case XK_j  :
				drawGraph = initGraph->kruskalBasic();
				sendExpose();
				break;
			case XK_k  :
				drawGraph = initGraph->kruskalForest();
				sendExpose();
				break;
			case XK_u  :
				drawGraph = initGraph->kruskalForestUltrametrique();
				sendExpose();
				break;
			case XK_r  :
				drawGraph = initGraph;
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
	/*Graph* graph = new Graph();
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
	
	graph->primHeap();
	graph->kruskalForest();*/
	
	// Test de creation d image A UTILISER SI IMAGE NON TROUVEE
	/*GrayImage img(600, 800);
	img.clear(250);
	img.rectangle(5, 6, 10, 40, 0);
	img.fillRectangle(55, 56, 100, 200, 10);
	img.fillRectangle(305, 250, 150, 200, 10);

	// Test d ecriture
	ofstream writeTest("image.pgm");
	img.writePGM(writeTest);*/
	
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
	
	//ofstream writeTestlol("testW.pgm");
	//image->writePGM(writeTestlol);
	
	cout << " Image : Width : " << image->getWidth() << ", Height : " << image->getHeight() << endl;
	
	/*for(ushort i = 0; i < image->getWidth(); i++)
		for(ushort j = 0; j < image->getHeight(); j++)
			if ((int)image->pixel (i, j) < 50)
				cout << "Pixel at " << i << ", " << j << " is at " << (int)image->pixel (i, j) << endl;*/
	
	Graph* new_graph = generate_graph (image);
	
	// GUI tests
	MyWindow win(image->getWidth(), image->getHeight(), "Algo Prim et Kuskal", new_graph);
	ezDraw.mainLoop();
	
	delete new_graph;
	return 0;
}
