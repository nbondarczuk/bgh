#if 0	/* just for version.sh */
static char *SCCS_VERSION = "4.1";
#endif

typedef struct PTree {
  char *value;
  Ptree *next[10];
} PTree;

void initPTreeNode(PTree *);
char *findInPTree(PTree *, char *);
PTree *insertToPtree(PTree *, char *, char *);

