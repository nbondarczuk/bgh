#if 0	/* just for version.sh */
static char *SCCS_VERSION = "4.1";
#endif


typedef struct Node
{
  struct Node *next;
  void *element;
  size_t size;
} Node;

typedef struct List
{
  struct Node *first, *last;
} List;

void List_Print(List *);
