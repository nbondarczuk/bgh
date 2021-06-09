#if 0	/* just for version.sh */
static char *SCCS_VERSION = "4.1";
#endif


#define MAX_QUEUE 64

typedef struct Queue {
  int size;
  char *element[MAX_QUEUE];
} Queue;

void InitQueue(Queue *);
int AppendToQueue(Queue *, void *);
int IsEmptyQueue(Queue *);
void *FrontOfQueue(Queue *);
void RemoveFront(Queue *);
void DeleteQueue(Queue *);
int SizeOfQueue(Queue *);
