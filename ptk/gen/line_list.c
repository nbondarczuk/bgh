#include <stdlib.h>
#include <stdio.h>
#include <strings.h>

#include "types.h"
#include "bgh.h"
#include "line_list.h"


/* External globals             */

/*
 * external functions
 */

#ifdef _USE_FAST_WRITE_
extern tostIOVecList *fpstIOVecList_New();
extern toenBool foenIOVecList_Write(tostIOVecList *ppstList, int pofilFile);
extern toenBool foenIOVecList_Delete(tostIOVecList *ppstList);
extern toenBool foenIOVecList_NextSlot(tostIOVecList *ppstList, int poiLen, char *ppsnVal);
#endif

/**********************************************************************************************************************
 *
 * fpstLineList_New
 *
 **********************************************************************************************************************
 */

tostLineList *fpstLineList_New()
{
  tostLineList *lpstList;

  if ((lpstList = (tostLineList *)calloc(1, sizeof(tostLineList))) == NULL)
    {
      return NULL;
    }

  lpstList->soiMemoryUsage = 0;
  lpstList->soiLen = 0;
  lpstList->spstFirst = lpstList->spstLast = NULL;
#ifdef _USE_FAST_WRITE_
  if ((lpstList->spstIOVecList = fpstIOVecList_New()) == NULL)
    {
      return NULL;
    }
#endif

  return lpstList;
}

/**********************************************************************************************************************
 *
 * foenLineList_Init
 *
 **********************************************************************************************************************
 */

toenBool foenLineList_Init(tostLineList *ppstList)
{
    ppstList->soiMemoryUsage = 0;
    ppstList->soiLen = 0;
    ppstList->spstFirst = ppstList->spstLast = NULL;
#ifdef _USE_FAST_WRITE_
    ppstList->spstIOVecList = NULL;
#endif
    return TRUE;
}

/**********************************************************************************************************************
 *
 * foenLineList_Write
 *
 **********************************************************************************************************************
 */

toenBool foenLineList_Write(tostLineList *ppstList, int pofilFile)
{
  tostLineNode *lpstNode = ppstList->spstFirst;
  int rc;
  toenBool loenStatus;

#ifdef _USE_FAST_WRITE_
  if ((loenStatus = foenIOVecList_Write(ppstList->spstIOVecList, pofilFile)) == FALSE)
    {
      return FALSE;
    }
#else
  fovdPrintLog (LOG_DEBUG, "foenLineList_Write: List size: %d\n", ppstList->soiLen);
  while (lpstNode != NULL)
    {
      fovdPrintLog (LOG_DEBUG, "foenLineList_Write: Line: %s\n", lpstNode->spstBuf->spsnVal);
      if ((rc = write(pofilFile, lpstNode->spstBuf->spsnVal, lpstNode->spstBuf->soiLen)) == -1)
        {
          return FALSE;
        }
      
      lpstNode = lpstNode->spstNext;
    }
#endif

  return TRUE;
}

/**********************************************************************************************************************
 *
 * foenLineList_Delete
 *
 **********************************************************************************************************************
 */

toenBool foenLineList_Delete(struct tostLineList *ppstList)
{
  char szTemp[128];
  tostLineNode *lpstNode = ppstList->spstFirst, *lpstTmp;
  toenBool loenStatus;
  
#ifdef _USE_FAST_WRITE_
  if ((loenStatus = foenIOVecList_Delete(ppstList->spstIOVecList)) == FALSE)
    {
      return FALSE;
    }
#endif  
  
  while (lpstNode != NULL)
    {
      /*
       * Save pointer to the node
       */
      
      lpstTmp = lpstNode;
      
      /*
       * Go to the next node
       */
      
      lpstNode = lpstNode->spstNext;
      
      /*
       * Free whole node
       */
      
      free(lpstTmp->spstBuf->spsnVal);      
      free(lpstTmp->spstBuf);
      free(lpstTmp);
    }
  
  if ((loenStatus = foenLineList_Init(ppstList)) == FALSE)
    {
      sprintf (szTemp, "foenLineList_Delete, Can't init line list \n");
      macErrorMessage (STREAM_INIT_LINE_LIST, WARNING, szTemp);      
      return FALSE;
    }

    return TRUE;
}       

/**********************************************************************************************************************
 *
 * foenLineList_Append
 *
 **********************************************************************************************************************
 */

toenBool foenLineList_Append(tostLineList *ppstList, char *ppsnzLine)
{
  tostLineNode *lpstNode;
  tostBuf *lpstBuf;
  toenBool loenStatus;

  /*
   * Create buffer for a string
   */
  
  if ((lpstBuf = (tostBuf *)calloc(1, sizeof(tostBuf))) == NULL)
    {
      return FALSE;
    }
  
  lpstBuf->soiLen = strlen(ppsnzLine);
  
  if ((lpstBuf->spsnVal = (char *)calloc(1, lpstBuf->soiLen)) == NULL)
    {
      return FALSE;
    }
  
  memcpy(lpstBuf->spsnVal, ppsnzLine, lpstBuf->soiLen);

  /*
   * Get buffer container
   */
  
  if ((lpstNode = (tostLineNode *)calloc(1, sizeof(tostLineNode))) == NULL)
    {
      return FALSE;
    }

  lpstNode->spstBuf = lpstBuf;
  lpstNode->spstNext = NULL;

  /*
   * Append new node
   */

  if (ppstList->spstFirst == NULL)
    {
      fovdPrintLog (LOG_DEBUG, "foenLineList_Append: First item\n");
      ppstList->spstFirst = ppstList->spstLast = lpstNode;
    }
  else
    {
      fovdPrintLog (LOG_DEBUG, "foenLineList_Append: Next item, list size: %d\n", ppstList->soiLen);
      ppstList->spstLast->spstNext = lpstNode;
      ppstList->spstLast = lpstNode;
    }
  
  ppstList->soiLen++;
  ppstList->soiMemoryUsage += lpstBuf->soiLen;

#ifdef _USE_FAST_WRITE_

  /*
   * Fill IOV list with buffer pointer
   */
  
  if ((loenStatus = foenIOVecList_NextSlot(ppstList->spstIOVecList, lpstBuf->soiLen, lpstBuf->spsnVal)) == FALSE)
    {
      return FALSE;
    }

#endif

  return TRUE;
}

/**********************************************************************************************************************
 *
 * foenLineList_FillBuf
 *
 **********************************************************************************************************************
 */

toenBool foenLineList_FillBuf(tostLineList *ppstList, tostBuf *ppstBuf)
{
  tostLineNode *lpstNode = ppstList->spstFirst;
  int rc, i;

  ppstBuf->soiLen = ppstList->soiMemoryUsage + 1;
  if ((ppstBuf->spsnVal = calloc(1, ppstBuf->soiLen * sizeof(char))) == NULL)
    {
      return FALSE;
    }

  fovdPrintLog (LOG_DEBUG, "foenLineList_FillBuf: Memory allocated: %d\n", ppstBuf->soiLen);
  
  i = 0;
  while (lpstNode != NULL)
    {
      /*
       * Add next line to the line list
       */

      fovdPrintLog (LOG_DEBUG, "foenLineList_FillBuf: Index: %d, Len: %d\n", i, lpstNode->spstBuf->soiLen); 
      memcpy(&(ppstBuf->spsnVal[i]), lpstNode->spstBuf->spsnVal, lpstNode->spstBuf->soiLen); 
      i += lpstNode->spstBuf->soiLen;       
      ASSERT(i < ppstBuf->soiLen);
      
      /*
       * Get next node
       */
      
      lpstNode = lpstNode->spstNext;
    }

  fovdPrintLog (LOG_DEBUG, "foenLineList_FillBuf: Memory filled with: %d bytes\n", i);

  return TRUE;
}
