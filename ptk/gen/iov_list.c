#include <stdlib.h>
#include <stdio.h>
#include <strings.h>

#include "types.h"
#include "bgh.h"
#include "line_list.h"

/**********************************************************************************************************************
 *
 * fpstIOVecList_New
 *
 **********************************************************************************************************************
 */

tostIOVecList *fpstIOVecList_New()
{
  struct tostIOVecList *lpstList;

  if ((lpstList = (struct tostIOVecList*)malloc(sizeof(struct tostIOVecList))) == FALSE)
    {
      return NULL;
    }

  lpstList->soiLen = 0;
  lpstList->spstFirst = lpstList->spstLast = NULL;
  
  return NULL;
}

/**********************************************************************************************************************
 *
 * foenIOVecList_Write
 *
 **********************************************************************************************************************
 */

toenBool foenIOVecList_Write(tostIOVecList *ppstList, int pofilFile)
{
  tostIOVecNode *lpstNode;
  int rc;

  lpstNode = ppstList->spstFirst;
  while (lpstNode != NULL)
    {
      if ((rc = writev(pofilFile, lpstNode->sastIOVecArr, lpstNode->soiFreeSlot)) == -1)
        {
          return FALSE;
        }

      lpstNode = lpstNode->spstNext;
    }

  return TRUE;
}

/**********************************************************************************************************************
 *
 * foenIOVecList_Delete
 *
 **********************************************************************************************************************
 */

toenBool foenIOVecList_Delete(tostIOVecList *ppstList)
{
  tostIOVecNode *lpstNode, *lpstTmp;
  int rc;

  lpstNode = ppstList->spstFirst;
  while (lpstNode != NULL)
    {
      lpstTmp = lpstNode;
      lpstNode = lpstNode->spstNext;
      free(lpstTmp);
    }

  return TRUE;
}

/**********************************************************************************************************************
 *
 * foenIOVecList_NextSlot
 *
 **********************************************************************************************************************
 */

toenBool foenIOVecList_NextSlot(tostIOVecList *ppstList, int poiLen, char *ppsnVal)
{
  tostIOVecNode *lpstNode;
  toenBool loenNew = FALSE;
  struct iovec *lpstIOVec;

  /*
   * Create or find next node with a free slot in the IOV table
   */

  if (ppstList->spstFirst == NULL)
    {
      loenNew = TRUE;
    }

  if (ppstList->spstLast->soiFreeSlot == IOV_MAX)
    {
      loenNew = TRUE;
    }
  
  /*
   * IOV node pointer must be filled
   */

  if (loenNew == TRUE)
    {
      /*
       * Create new IOV node
       */
      
      if ((lpstNode = (tostIOVecNode *)calloc(1, sizeof(tostIOVecNode))) == NULL)
        {
          return FALSE;
        }
      
      lpstNode->soiFreeSlot = 0;
      lpstNode->spstNext = NULL;
      
      /*
       * Place the node on the list
       */

      if (ppstList->spstFirst == NULL)
        {
          /*
           * This is the fist node of the list
           */

          ppstList->spstFirst = ppstList->spstLast = lpstNode;
        }
      else
        {
          /*
           * Some nodes were created so place the node on the end of the list
           */

          ppstList->spstLast->spstNext = lpstNode;
          ppstList->spstLast = lpstNode;
        }
    }
  else
    {
      /*
       * Use last IOV node of the list
       */

      lpstNode = ppstList->spstLast;
    }
  
  /*
   * Use founeded or created table referenced with pointer lpstNode
   */
  
  lpstIOVec = &(lpstNode->sastIOVecArr[lpstNode->soiFreeSlot++]);
  lpstIOVec->iov_len = poiLen;     
  lpstIOVec->iov_base = ppsnVal;
  
  return FALSE;
}
