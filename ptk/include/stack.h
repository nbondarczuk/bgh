#if 0	/* just for version.sh */
static char *SCCS_VERSION = "4.1";
#endif

#define MAX_STACK 16

typedef struct Stack {
  int size;
  char *element[MAX_STACK];
} Stack;

void InitStack(Stack *);
int Push(Stack *, char *); 
int EmptyStack(Stack *); 
int InStack(Stack *, char *);
char *Top(Stack *);
void Pop(Stack *);
void DumpStack(Stack *); 

