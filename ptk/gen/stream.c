#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "types.h"
#include "bgh.h"
#include "line_list.h"
#include "stream.h"
#include "generator.h"

/*
 * external variables
 */

extern stBGHGLOB        stBgh;

extern int goilBillMediumSize;

extern tostBillMedium *gpstBillMedium;

/*
 * external functions
 */

extern tostMultiStream *fpstMultiStream_New(toenDocType poenDoc, 
                                            toenInvType poenInv, 
                                            int poiBillMediumIndex);

extern toenBool foenMultiStream_Next(tostMultiStream *ppstStream, 
                                     int poiCustomerId, 
                                     toenBool poenInitializeLineList);

extern toenBool foenMultiStream_Print(tostMultiStream *ppstStream, 
                                      char *ppsnzLine);

extern toenBool foenMultiStream_Flush(tostMultiStream *ppstStream);

extern tostSingleStream *fpstSingleStream_New(toenDocType poenDoc, 
                                              toenInvType poenInv, 
                                              int poiBillMediumIndex);

extern toenBool foenSingleStream_Next(tostSingleStream *ppstStream, 
                                      int poiCustomerId, 
                                      char *ppsnzCustCode,
                                      toenBool poenInitializeLineList);

extern toenBool foenSingleStream_Print(tostSingleStream *ppstStream, 
                                       char *ppsnzLine);

extern toenBool foenSingleStream_Flush(tostSingleStream *ppstStream);

extern int foiMapBillMediumId2Index(int poiId);

extern int foiMultiStream_SavePoint(tostMultiStream *ppstStream);

extern int foiMultiStream_RollBackWork(tostMultiStream *ppstStream);

extern int foiSingleStream_SavePoint(tostSingleStream *ppstStream);      

extern int foiSingleStream_RollBackWork(tostSingleStream *ppstStream);      

/**********************************************************************************************************************
 *
 * fpstStream_New
 *
 **********************************************************************************************************************
 */

tostStream *fpstStream_New(toenDocType poenDoc, toenInvType poenInv, int poiBillMediumIndex)
{
  tostStream *lpstStream = NULL;

  if (stBgh.lCustSetSize > 0)
    {
      lpstStream = (tostStream *)fpstMultiStream_New(poenDoc, poenInv, poiBillMediumIndex);
    }
  else
    {
      lpstStream = (tostStream *)fpstSingleStream_New(poenDoc, poenInv, poiBillMediumIndex);
    }
  
  return lpstStream;
}

/**********************************************************************************************************************
 *
 * foenStream_Next
 *
 **********************************************************************************************************************
 */

toenBool foenStream_Next(tostStream *ppstStream, int poiCustomerId, toenBool poenInitializeLineList)
{
  toenBool loenStatus;
  
  ASSERT(ppstStream != NULL);

  if (stBgh.lCustSetSize > 0)
    {
      loenStatus = foenMultiStream_Next((tostMultiStream *)ppstStream, poiCustomerId, poenInitializeLineList);
    }
  else
    {
      loenStatus = foenSingleStream_Next((tostSingleStream *)ppstStream, poiCustomerId, NULL,  poenInitializeLineList);
    }
      
  return loenStatus;
}

/**********************************************************************************************************************
 *
 * foenStream_Print
 *
 **********************************************************************************************************************
 */

toenBool foenStream_Print(tostStream *ppstStream, char *ppsnzLine)
{
  toenBool loenStatus;

  ASSERT(ppstStream != NULL);

  if (stBgh.lCustSetSize > 0)
    {
      loenStatus = foenMultiStream_Print((tostMultiStream *)ppstStream, ppsnzLine);
    }
  else
    {
      loenStatus = foenSingleStream_Print((tostSingleStream *)ppstStream, ppsnzLine);
    }
  
  return loenStatus;
}

/**********************************************************************************************************************
 *
 * foenStream_Flush
 *
 **********************************************************************************************************************
 */

toenBool foenStream_Flush(tostStream *ppstStream)
{
  toenBool loenStatus;

  ASSERT(ppstStream != NULL);

  if (stBgh.lCustSetSize > 0)
    {
      loenStatus = foenMultiStream_Flush((tostMultiStream *)ppstStream);
    }
  else
    {
      loenStatus = foenSingleStream_Flush((tostSingleStream *)ppstStream);      
    }
      
  return loenStatus;
}

/**********************************************************************************************************************
 *
 * fpstStreamTab_New
 *
 **********************************************************************************************************************
 */

tostStreamTab *fpstStreamTab_New(toenDocType poenDoc, toenInvType poenInv)
{
  tostStreamTab *lpstTab;
  int i;
  
  if ((lpstTab = (tostStreamTab *)calloc(1, sizeof(tostStreamTab))) == NULL)
    {
      return NULL;
    }
  else
    {
      fovdPrintLog (LOG_DEBUG, "fpstStreamTab_New: Allocated bytes: %d\n", sizeof(tostStreamTab));
    }

  lpstTab->soiTabSize = goilBillMediumSize;
  if ((lpstTab->sapstStream = (tostStream **)calloc(goilBillMediumSize, sizeof(tostStream *))) == NULL)
    {
      return NULL;
    }
  else
    {
      fovdPrintLog (LOG_DEBUG, "fpstStreamTab_New: Allocated bytes: %d\n", sizeof(tostStream *) * goilBillMediumSize);
    }

  fovdPrintLog (LOG_DEBUG, "fpstStreamTab_New: Filling stream table of size: %d\n", goilBillMediumSize);
  for (i = 0; i < goilBillMediumSize; i++)
    {
      fovdPrintLog (LOG_DEBUG, "fpstStreamTab_New: Creating stream for BM: %d\n", i);
      if ((lpstTab->sapstStream[i] =  (tostStream *)fpstStream_New(poenDoc, poenInv, i)) == NULL)
        {
          return NULL;
        }
      else
        {
          fovdPrintLog (LOG_DEBUG, "fpstStreamTab_New: Created stream for BM: %d\n", i);
        }      
    }

  return lpstTab;
}

/**********************************************************************************************************************
 *
 * fpstStreamTab_Next
 *
 **********************************************************************************************************************
 */

tostStream *fpstStreamTab_Next(tostStreamTab *ppstTab, int poiBillMedium)
{
  int loiIndex;
/*!@#*/
/*  printf("BillMedium %d pointer %ld\n", poiBillMedium, (long)ppstTab->sapstStream[poiBillMedium]);
  printf("BillMedium %d pointer-1 %ld\n", poiBillMedium, (long)ppstTab->sapstStream[poiBillMedium - 1]);
*/

  if(( loiIndex = foiMapBillMediumId2Index(poiBillMedium)) == -1)
    {
      return NULL;
    }
  
  return ppstTab->sapstStream[loiIndex ];
}

/**********************************************************************************************************************
 *
 * Function name:	foiStream_SavePoint
 *
 * Function call:	rc = foiStream_SavePoint()
 *
 * Returns:	int
 *
 *              Value on success:
 *
 *                      0
 *
 *		Value on failure:
 *
 *                      -1
 *
 * Arguments:	ppstStream, tostStream * - the stream state container
 *
 **********************************************************************************************************************
 */

int foiStream_SavePoint(tostStream *ppstStream)
{
  int rc = 0;

  ASSERT(ppstStream != NULL);

  if (stBgh.lCustSetSize > 0)
    {
      rc = foiMultiStream_SavePoint((tostMultiStream *)ppstStream);
    }
  else
    {
      rc = foiSingleStream_SavePoint((tostSingleStream *)ppstStream);      
    }

  if (rc < 0)
    {
      rc = -1;
    }

  return rc;
}

/**********************************************************************************************************************
 *
 * Function name:	foiStream_RollBackWork
 *
 * Function call:	rc = foiStream_RollBackWork()
 *
 * Returns:	int
 *
 *              Value on success:
 *
 *                      0
 *
 *		Value on failure:
 *
 *                      -1
 *
 * Arguments:	ppstStream, tostStream * - the stream state container
 * 
 *
 **********************************************************************************************************************
 */

int foiStream_RollBackWork(tostStream *ppstStream)
{
  int rc = 0;

  fovdPrintLog (LOG_DEBUG, "foiGen_RollBackWork: Roll back of the stream operation\n");  

  ASSERT(ppstStream != NULL);

  if (stBgh.lCustSetSize > 0)
    {
      rc = foiMultiStream_RollBackWork((tostMultiStream *)ppstStream);
    }
  else
    {
      rc = foiSingleStream_RollBackWork((tostSingleStream *)ppstStream);      
    }

  if (rc < 0)
    {
      rc = -1;
    }

  return rc;
}

/**********************************************************************************************************************
 *
 * Function name:	foiStreamSeq_Delete
 *
 * Function call:	rc = foiStreamSeq_Delete(&lpstSeq)
 *
 * Returns:	int
 *
 *              Value on success:
 *
 *                      0
 *
 *		Value on failure:
 *
 *                      void
 *
 * Arguments:	pppstSeq, tostStreamSeq ** - pointer to the stream sequence
 * 
 *
 **********************************************************************************************************************
 */

int foiStreamSeq_Delete(tostStreamSeq **pppstSeq) 
{
  int rc = 0;
  tostStreamSeq *lpstSeq, *lpstTmp;

  lpstSeq = *pppstSeq;
  while (lpstSeq != NULL)
    {
      lpstTmp = lpstSeq;
      lpstSeq = lpstSeq->spstNext;
      free(lpstTmp);
    }
  
  *pppstSeq = NULL;
  
  return rc;
}

/**********************************************************************************************************************
 *
 * Function name:	foiStreamSeq_AddUnique
 *
 * Function call:	rc = foiStreamSeq_AddUnique(&lpstSeq, lpstStream)
 *
 * Returns:	int
 *
 *              Value on success:
 *
 *                      0
 *
 *		Value on failure:
 *
 *                      -1 - can not allocate memory 
 *
 * Arguments:	pppstSeq, tostStreamSeq ** - pointer to the stream sequence
 * 
 *
 **********************************************************************************************************************
 */

int foiStreamSeq_AddUnique(tostStreamSeq **pppstSeq, tostStream *ppstStream) 
{
  int rc = 0;
  tostStreamSeq *lpstNew, *lpstSeq, *lpstPrev;
  
  /*
   * Create new element
   */
  
  if ((lpstNew = (tostStreamSeq *)calloc(1, sizeof(tostStreamSeq))) == NULL)
    {
      return -1;
    }
  
  lpstNew->spstStream = ppstStream;
  lpstNew->spstNext = NULL;

  /*
   * Add to the list
   */

  if (*pppstSeq == NULL) /* first element on the list */
    {
      *pppstSeq = lpstNew;
    }
  else /* not the firt element on the list */
    {
      /*
       * Find the lst element of the list checking the uniquness of the list
       * new element can not be added twice
       */

      lpstSeq = *pppstSeq;
      lpstPrev = NULL;
      while (lpstSeq != NULL)
	{
	  if (lpstSeq->spstStream == ppstStream)
	    {
	      lpstPrev = NULL;
	      break;
	    }
	  
	  lpstPrev = lpstSeq;
	  lpstSeq = lpstSeq->spstNext;
	} 
      
      if (lpstPrev != NULL) /* new tream is added to the end of the list */
	{
	  lpstPrev->spstNext = lpstNew;
	}
      else /* unlikely - the stream was previously used */
	{
	  free(lpstNew);
	}
    }

  return rc;
}



