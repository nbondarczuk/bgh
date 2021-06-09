#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "gen.h"
#include "inv_item.h"
#include "occ_queue.h"

/*
typedef struct tostOCCNode
{
  tostInvItem *spstInvItem;
  struct tostOCCNode *spstNext;

} tostOCCNode;

typedef struct tostOCCQueue
{
  tostOCCNode *spstRoot;

} tostOCCQueue;
*/

toenBool foenOCCQueue_Init(tostOCCQueue *ppstQueue)
{
  ppstQueue->spstRoot = NULL;

  return TRUE;
}

toenBool foenOCCQueue_Delete(tostOCCQueue *ppstQueue)
{
  return TRUE;
}

toenBool foenOCCQueue_Enqueue(tostOCCQueue *ppstQueue, tostInvItem *ppstInvItem)
{
  tostOCCNode *lpstNode;
  tostInvItem *lpstItem;

  lpstNode = (tostOCCNode *)malloc(sizeof(tostOCCNode));
  if (lpstNode == NULL)
    {
      return FALSE;
    }

  lpstItem = (tostInvItem *)malloc(sizeof(tostInvItem));
  if (lpstItem == NULL)
    {
      return FALSE;
    }
  
  memcpy((void *)lpstItem, (void *)ppstInvItem, sizeof(tostInvItem));
  lpstNode->spstInvItem = lpstItem;
  lpstNode->spstNext = ppstQueue->spstRoot;
  ppstQueue->spstRoot = lpstNode;

  return TRUE;
}

tostInvItem *fpstOCCQueue_Dequeue(tostOCCQueue *ppstQueue)
{
  tostInvItem *lpstItem;  
  tostOCCNode *lpstNode, *lpstTmpNode;

  lpstNode = ppstQueue->spstRoot;
  if (lpstNode == NULL)
    {
      return NULL;
    }
  
  lpstTmpNode = lpstNode;
  lpstItem = lpstNode->spstInvItem;
  ppstQueue->spstRoot = lpstNode->spstNext;
  free(lpstTmpNode);

  return lpstItem;
}
