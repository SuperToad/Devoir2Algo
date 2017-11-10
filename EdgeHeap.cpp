

void echange(int* a, int* b)
{
	*a = *b+*a;
	*b = *a-*b;
	*a = *a-*b;
}

int reorganiserTasDesc(uint i, uint n, Tas* t)
{	
	if (t->tab[i] < t->tab[n])
	{
		echange(&(t->tab[i]),&(t->tab[n]));
		n = (n-1)/2;
		i = (n-1)/2;
		reorganiserTasDesc(i,n,t);
	}
	return 1;
}

// Retourne -1 si aucun enfant ; i*2 si premier et i*2+1 si deuxième.
int biggerChild(uint i, uint n, Tas* t)
{
	int result = -1;
	if (i*2+1 == n)
		result = i*2+1;
	else if (i*2+1 < n)
		result = ( t->tab[i*2+1] > t->tab[i*2+2] ? i*2+1 : i*2+2 );
	return result;
}

int EdgeHeap::organize(uint i, uint n)
{
	int bc = biggerChild(i,n);
	
	if (bc > 0)
	{
		if (heap[i] < heap[bc])
		{
			echange(&(t->tab[i]),&(t->tab[bc]));
			i = bc;
			reorganiserTas(i,n,t);
		}
	}
	return 0;
}

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
			reorganiserTas(i,n);
			i = (i-1)/2;
		}
		reorganiserTas(0,n);
		heap_size++;
	}
	return gud;
}

int EdgeHeap::deleteElement(int index)
{
	if (index > heap_size-1)
		return -1;

	echange(&head[index], &heap[heap_size-1]);
	heap_size--;
	uint n = heap_size-1;
	uint i = index;
	while (i > 0)
	{
		reorganiserTas(i, n);
		i = (i-1)/2;
	}
	reorganiserTas(0,n);
	return 1;
}

void EdgeHeap::displayHeap ()
{
	for (uint i = 0 ; i < heap_size ; i++)
	{
		cout << heap[i] << endl;
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
EdgeHeap::EdgeHeap(Edge* _heap, int _heap_size)
:head_capacity(MAX_CAPACITY)
{
	heap_size = 0;
	for (uint i = 0 ; i < heap_size ; i++)
		push(_heap[i]);
}
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
