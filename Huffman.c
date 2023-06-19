#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#define MAX_CHARS   256

typedef struct couple
{
    unsigned char ch;
    unsigned int counter;
} couple;

typedef struct Node
{
    couple data;
    struct Node *left, *right;

} Node;

Node * pop(Node *all[], int size);
void insert(Node *all[], Node * elm, int i);
void buildHeap(Node *all[], int size);
void heapify(Node *all[], int i, int size);
void extractFromTree(Node *root, int ply, char code[], char *codes[]);
int isLeaf(Node * n);
Node * buildTree(Node *all[], int size);
int numberOfNodes(Node *all[], int size);

int numberOfNodes(Node *all[], int size)
{
    int i;
    for (i = 0; i < size; i++)
    {
        if (!all[i])
            break;
    }
    return i;
}

Node * buildTree(Node *all[], int size)
{
    while (numberOfNodes(all, size) > 1)
    {
        Node * f = pop(all, size);
        Node * s = pop(all, size);
        Node * t = (Node *)malloc(sizeof(Node));
        t->data.ch = 0;
        t->data.counter = s->data.counter+f->data.counter;
        t->left = f;
        t->right = s;
        insert(all, t, numberOfNodes(all, size));
    }
    return pop(all, size);
}


int isLeaf(Node * n)
{
    return !n->left && !n->right;
}
 
void heapify(Node *all[], int i, int size)
{
    int s = i;
    int l = 2*i+1;
    int r = 2*i+2;

    if (l < size && all[l]->data.counter < all[s]->data.counter) s = l;
    if (r < size && all[r]->data.counter < all[s]->data.counter) s = r;
    if (s != i)
    {
        Node * tmp = all[i];
        all[i] = all[s];
        all[s] = tmp;
        heapify(all, s, size);
    }
}

void buildHeap(Node *all[], int size)
{
    int idx = (size/2)-1;
    for (int i = idx; i >= 0; i--)
        heapify(all, i, size);
}

void insert(Node *all[], Node * elm, int i)
{
    int curr = i;
    int parent = (curr-1)/2;
    all[curr] = elm;
    while (all[curr]->data.counter < all[parent]->data.counter)
    {
        Node * tmp = all[curr];
        all[curr] = all[parent];
        all[parent] = tmp;
        curr = parent;
        parent = (curr-1)/2;
    }
}

Node * pop(Node *all[], int size)
{
    if (!all[0]->data.counter) return NULL;
    Node * ret = all[0];
    if (all[size-1])
    {
        all[0] = all[size-1];
        all[size-1] = NULL;
        heapify(all, 0, size-1);
        return ret;
    }
    for (int i = 0; i < size-1; i++)
    {
        if (!all[i+1])
        {
            all[0] = all[i];
            all[i] = NULL;
            heapify(all, 0, i);
            break;
        }

    }
    return ret;
}

void extractFromTree(Node *root, int ply, char code[], char *codes[])
{
    if (isLeaf(root)) {
        code[ply] = 0;
        codes[root->data.ch] = strdup(code);
    } else {
        code[ply] = '0';
        extractFromTree (root->left, ply+1, code, codes);

        code[ply] = '1';
        extractFromTree (root->right, ply+1, code, codes);
    }
}

void writeBinFile(char *codes[], FILE *oldf, FILE *newf)
{
    unsigned char c;
    c = fgetc(oldf);
    unsigned char toFill = 0;
    int bitCounter = 0;
    while (!feof(oldf))
    {
        char *code;
        for (code = codes[c]; *code; code++)
        {
            toFill <<= 1;
            if (*code == '1') 
                toFill |= 1;
            bitCounter++;
            if (bitCounter == 8)
            {
            	fputc(toFill, newf);
                toFill = 0;
                bitCounter = 0;
            }
        }
        c = fgetc(oldf);
    }
    if (bitCounter != 0)
    {
        toFill <<= (8-bitCounter);
        fputc(toFill, newf);
    }

}

void writeCompressionExtras(couple all[], int size, FILE *newf, int sizeToWrite)
{
	unsigned char c1 = 31;
	unsigned char c2 = 139;
	fputc(c1, newf);		
	fputc(c2, newf);
	fputc(sizeToWrite, newf);
	for (int i = 0; i < size; i++)
	{
		if (all[i].counter)
		{
			fputc(all[i].ch, newf);
			fwrite(&(all[i].counter), sizeToWrite, 1, newf);
		}
	}
	fputc(c2, newf);
	
}

void constructFile(Node * root, couple all[], FILE * oldf, FILE * newf)
{
	unsigned char c = fgetc(oldf);
	Node * tmpRoot = root;
	while (!feof(oldf))
	{
		for (int i = 0; i < 8; i++)
		{
			if (isLeaf(tmpRoot))
			{
				if (all[tmpRoot->data.ch].counter)
				{
					fputc(tmpRoot->data.ch, newf);
					all[tmpRoot->data.ch].counter--;
					tmpRoot = root;
				}
				else break;
			}
			if (c & 128)
				tmpRoot = tmpRoot->right;
			else
				tmpRoot = tmpRoot->left;
			c <<= 1;
		}
		c = fgetc(oldf);
	}
}

void freeTree(Node * root)
{
	if (!root) return;
	freeTree(root->right);
	freeTree(root->left);
	free(root);
}

int main(int argc, char **argv)
{
    int compress = 0;
    int decompress = 0;
    int maxFreq = 255;
    int sizeOfMaxFreq = sizeof(char);
    if (argc != 3)
    {
        fprintf(stderr, "Wrong number of arguments!\n");
        exit(1);
    }

    if (strcmp(argv[1], "-c") == 0 || strcmp(argv[1], "-C") == 0) compress = 1;
    else if (strcmp(argv[1], "-d") == 0 || strcmp(argv[1], "-D") == 0) decompress = 1;
    else
    {
        fprintf(stderr, "No known option %s\n", argv[1]);
     	exit(2);
    }

    if (compress)
    {
    	if (strstr(argv[2], ".huf"))
		{
			fprintf(stderr, "File is already compressed\n");
			exit(5);
		}
		
        FILE *f = fopen(argv[2], "r");
        if (!f)
        {
            fprintf(stderr, "File not found!\n");
            exit(3);
        }
        couple all[MAX_CHARS] = {{0,0}};
        unsigned char c;
        c = fgetc(f);
        int count = 0;
        while (!feof(f))
        {
            if (!all[c].counter) count++;
            all[c].counter++;
            all[c].ch = c;
            
            if (all[c].counter > maxFreq)
            {
            	maxFreq = USHRT_MAX;
            	sizeOfMaxFreq = sizeof(short);
            }
            if (all[c].counter > maxFreq)
          	{
           		maxFreq = UINT_MAX;
           		sizeOfMaxFreq = sizeof(int);
           	}
           	
            c = fgetc(f);
        }
        if (!count)
        {
        	fprintf(stderr, "File is empty\n");
        	exit(6);
        }
        rewind(f);
        Node *nodes[count];
        int j = 0;
        for (int i = 0; i < MAX_CHARS; i++)
        {
            if (all[i].counter)
            {
                Node *t = (Node*)malloc(sizeof(Node));
                t->data = all[i];
                t->left=t->right=NULL;
                nodes[j++] = t;
                if (j == count) break;
            }
        }
        buildHeap(nodes, sizeof(nodes)/sizeof(nodes[0]));
        Node * root = buildTree(nodes, sizeof(nodes)/sizeof(nodes[0]));

        char binCode[MAX_CHARS];
        char *binCodes[MAX_CHARS];

        extractFromTree(root, 0, binCode, binCodes);
        freeTree(root);
		char *nfName = argv[2];
		strcat(nfName, ".huf");
		FILE * newf = fopen(nfName, "wb");
		writeCompressionExtras(all, MAX_CHARS, newf, sizeOfMaxFreq);
		writeBinFile(binCodes, f, newf);
		fclose(f);
		fclose(newf);
    }
    else if (decompress)
    {
        FILE *f = fopen(argv[2], "rb");
        if (!f)
        {
            fprintf(stderr, "File not found!\n");
            exit(3);
        }
       	couple all[MAX_CHARS] = {{0,0}};
        unsigned char c = fgetc(f);
      	if (c != 31)
      	{
      		fprintf(stderr, "Wrong file format\n");
      		exit(4);
      	}
        c = fgetc(f);
      	if (c != 139)
      	{
      		fprintf(stderr, "Wrong file format\n");
      		exit(4);
      	}
      	sizeOfMaxFreq = (int)fgetc(f);
        int count = 0;
        int counter = 0;
        c = fgetc(f);
		while(c != 139)
		{
		    count++;
        	all[c].ch = c;
			fread(&counter, sizeOfMaxFreq, 1, f);
			all[c].counter = counter;
			c = fgetc(f);
        }
        
        Node *nodes[count];
        int j = 0;
        for (int i = 0; i < MAX_CHARS; i++)
        {
            if (all[i].counter)
            {
                Node *t = (Node*)malloc(sizeof(Node));
                t->data = all[i];
                t->left=t->right=NULL;
                nodes[j++] = t;
                if (j == count) break;
            }
        }
        buildHeap(nodes, sizeof(nodes)/sizeof(nodes[0]));
        Node * root = buildTree(nodes, sizeof(nodes)/sizeof(nodes[0]));
        char *nfName = argv[2];
        nfName[strlen(nfName)-4] = 0;
        FILE * newf = fopen(nfName, "w");
        constructFile(root, all, f, newf);
        freeTree(root);
        fclose(f);
        fclose(newf);
    }
}
