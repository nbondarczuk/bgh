#include <stdlib.h>
#include <string.h>
#include "queue.h"

#if 0	/* just for version.sh */
static char *SCCS_VERSION = "4.1";
#endif

void InitQueue(Queue *q)
{
  int i;

  q->size = 0;
  for (i = 0; i < MAX_QUEUE; i++)
    {
      q->element[i] = NULL;
    }
}

int AppendToQueue(Queue *q, void *e) 
{
  if (q->size < MAX_QUEUE)
    {
      q->element[(q->size)++] = e;
      return 1;
    }
  
  return 0;
}

int IsEmptyQueue(Queue *q) 
{
  return q->size == 0;
}

void *FrontOfQueue(Queue *q)
{
  if (q->size > 0)
    {
      return q->element[0];
    }

  return NULL;
}

void RemoveFront(Queue *q)
{
  int i;

  for (i = 0; i < q->size - 1; i++)
    {
      q->element[i] = q->element[i + 1];
    }
  
  q->size--;
}

int SizeOfQueue(Queue *q)
{
  return q->size;
}

void DeleteQueue(Queue *q)
{
}

