#if 0	/* just for version.sh */
static char *SCCS_VERSION = "4.1";
#endif

#define MAX_SET 64

typedef struct Set {
  int size;
  char *element[MAX_SET];
} Set;

void InitSet(Set *);
int Insert(Set *, char *);
int Contains(Set *, char *);
int EmptySet(Set *);
char *First(Set *);
void RemoveFirst(Set *);
void DumpSet(Set *); 
