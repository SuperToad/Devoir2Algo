#include "EdgeHeap.hpp"

// Fonctionne mais n'est pas la version implémentée, je la laisse quand même au cas où...
/*
int reorganiserTasDesc(uint i, uint n)
{	
	if (heap[i].weight < heap[n].weight)
	{
		echange(&heap[i],&heap[n]);
		n = (n-1)/2;
		i = (n-1)/2;
		reorganiserTasDesc(i,n);
	}
	return 1;
}
*/

// Échange simple entre deux arcs
void echange(Edge* a, Edge* b)
{
	Edge c = *a;
	*a = *b;
	*b = c;
}

// Retourne -1 si aucun enfant ; i*2 si premier et i*2+1 si deuxième.
int EdgeHeap::lowerChild(uint i, uint n)
{
	int result = -1;
	if (i*2+1 == n)
		result = i*2+1;
	else if (i*2+1 < n)
		result = ( heap[i*2+1].weight < heap[i*2+2].weight ? i*2+1 : i*2+2 );
	return result;
}

// Réorganise le tas de manière récursive dans le cas où 
//l'élément donné est plus grand que son + petit enfant
int EdgeHeap::organize(uint i, uint n)
{
	int bc = lowerChild(i,n);
	
	if (bc > 0)
	{
		if (heap[i].weight > heap[bc].weight)
		{
			echange(&heap[i],&heap[bc]);
			i = bc;
			organize(i,n);
		}
	}
	return 0;
}

// Ajoute un élément au tas et réorganise
int EdgeHeap::push(Edge to_add)
{
	int gud = 0;
	if (heap_size < heap_capacity)
	{
		gud = 1;
		heap[heap_size] = to_add;
	
		uint n = heap_size;
		uint i = (n-1)/2;
		while (i > 0)
		{
			organize(i,n);
			i = (i-1)/2;
		}
		organize(0,n);
		heap_size++;
	}
	return gud;
}

// Supprime l'élément à l'indice index et réorganise
int EdgeHeap::deleteElement(uint index)
{
	if (index > heap_size-1)
		return -1;
	if (heap_size == 1)
	{
		heap_size--;
		return 1;
	}

	echange(&heap[index], &heap[heap_size-1]);
	heap_size--;
	uint n = heap_size-1;
	uint i = index;
	while (i > 0)
	{
		organize(i, n);
		i = (i-1)/2;
	}
	organize(0,n);
	return 1;
}

// Affiche le tas
void EdgeHeap::displayHeap ()
{
	for (uint i = 0 ; i < heap_size ; i++)
	{
		cout << heap[i].weight << endl;
		if ((i == 0) || ((i+2) && !((i+2) & (i))))
			cout << endl;
	}
	cout << endl;
}
/*
void displayHeapTests (const Tas t)
{
	if (t.cursor >= 1)
		printf("%d : -\n", t.tab[0]);
	for (uint i = 1 ; i < t.cursor ; i++)
	{		
		printf("%d < %d %s \n",t.tab[i], t.tab[(i-1)/2], (t.tab[i] > t.tab[(i-1)/2]?"<----- X":""));
	}
	printf("\n\n");
}
*/

// Retire le premier élément du tas
Edge EdgeHeap::pop ()
{
	Edge result = heap[0];
	deleteElement(0);
	return result;
}

/*
int* heapSort(int n, Tas* t)
{
	int* result = malloc(n * sizeof(int));
	for (uint i = 0 ; i < n ; i++)
		result[i] = pop(t);
	return result;
}
*/

// Consutructeur
EdgeHeap::EdgeHeap(Edge* _heap, uint _heap_size)
:heap_capacity(MAX_CAPACITY)
{
	heap_size = 0;
	for (uint i = 0 ; i < heap_size ; i++)
		push(_heap[i]);
}

EdgeHeap::~EdgeHeap()
{
	delete[] heap;
		
}

// int main()
// {
// 	Edge* edges = new Edge[10];
// 	EdgeHeap* e_heap = new EdgeHeap(edges, 0);
// 	Node* nik = new Node();
// 	Edge e = {nik, 0};
// 	e_heap->push(e);
// 	e.weight = 3;
// 	e_heap->push(e);
// 	e.weight = 2;
// 	e_heap->push(e);
// 	e_heap->displayHeap();
// 	delete e_heap;
// 	cout << "HEEEEE" << endl;
// 	delete edges;
// 	return 0;
// }

/*
int main()
{
	srand(time(NULL));
	Tas t;
	t.cursor = 0;
	
	for (uint i = 0 ; i < 31 ; i++)
		if (!addElement(rand()%200-100,&t))
		{
			perror("Dépassement du tas.");
			break;
		}
	
	displayHeap(t);
	displayHeapTests(t);

	Tas t2;
	int lenght = t.cursor;
	int* sortedArray = heapSort(lenght, &t);
	buildHeap(sortedArray, lenght, &t2);
	free(sortedArray);

	displayHeap(t2);
	displayHeapTests(t2);
	

	return 0;
}
*/
