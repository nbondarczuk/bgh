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

extern stBGHGLOB	stBgh;

/*
 * external functions
 */

extern tostImageStream *fpstImageStream_New();

extern toenBool foenImageStream_Next(tostImageStream *ppstStream, 
				     int poiCustomerId, 
				     toenBool poenInitializeLineList);

extern tostStreamTab *fpstStreamTab_New(toenDocType poenDoc, 
					toenInvType poenInv);

extern tostStream *fpstStreamTab_Next(tostStreamTab *ppstTab, 
				      int poiBillMedium);

extern tostStream *fpstStream_New(toenDocType poenDoc, 
				  toenInvType poenInv, 
				  int poiBillMedium);

extern toenBool foenStream_Next(tostStream *ppstStream, 
				int poiCustomerId, 
				toenBool poenInitializeLineList);

extern toenBool foenStream_Print(tostStream *ppstStream, 
				 char *ppsnzLine);

extern toenBool foenStream_Flush(tostStream *ppstStream);

extern toenBool foenImageStream_Insert(tostImageStream *ppstStream, 
				       int poiImgTypeId);

extern toenBool foenItbLineFormat(int poiBillMedium, 
				  char **papsnzArgv, 
				  int poiArgs, 
                                  char *pasnzLine, 
				  int poiMaxLineLen);

extern tostSingleStream *fpstSingleStream_New(toenDocType poenDoc, 
					      toenInvType poenInv, 
					      int poiBillMedium);

extern toenBool foenSingleStream_Next(tostSingleStream *ppstStream, 
				      int poiCustomerId, 
				      char *ppsnzCustCode,
				      toenBool poenInitializeLineList);

extern toenBool foenSingleStream_Print(tostSingleStream *ppstStream, 
				       char *ppsnzLine);

extern toenBool foenSingleStream_Flush(tostSingleStream *ppstStream);

extern int foiStream_SavePoint(tostStream *ppstStream);

extern int foiStream_RollBackWork(tostStream *ppstStream);

extern int foiStreamSeq_Delete(tostStreamSeq **pppstSeq);

extern int foiStreamSeq_AddUnique(tostStreamSeq **pppstSeq, 
				  tostStream *ppstStream);


/*
 * static variables
 */

static char szTemp[PATH_MAX];

toenDocType daenDiskType[DISK_TYPES_NO] =
{
  DOC_ITBEXCEL_TYPE,
  DOC_ITBFIXED_TYPE
};


/**********************************************************************************************************************
 *
 * fpstInvGen_New  
 *
 **********************************************************************************************************************
 */

tostInvGen *fpstInvGen_New()
{
  tostInvGen *lpstGen;
  int i;

  fovdPrintLog (LOG_DEBUG, "fpstInvGen_Init: Allocating memory\n");
  if ((lpstGen = (tostInvGen *)calloc(1, sizeof(tostInvGen))) == NULL)
    {
      sprintf (szTemp, "Can't init stream table\n");
      macErrorMessage (GEN_MALLOC, WARNING, szTemp);               
      return NULL;
    }
  else
    {
      fovdPrintLog (LOG_DEBUG, "fpstInvGen_Init: Allocated bytes: %d\n", sizeof(tostInvGen));
    }

  lpstGen->soenGen = GEN_INV_TYPE;

  /*
   * Al invoice types must be handled
   */

  fovdPrintLog (LOG_DEBUG, "fpstInvGen_Init: Creating streams: %d\n", INV_TYPES_NO);
  for (i = 0; i < INV_TYPES_NO; i++)
    {
      fovdPrintLog (LOG_DEBUG, "fpstInvGen_Init: Creating stream no: %d\n", i);
      if ((lpstGen->sapstStreamTab[i] = fpstStreamTab_New(DOC_INV_TYPE, i)) == NULL)
        {
          sprintf (szTemp, "Can't init stream table\n");
          macErrorMessage (GEN_NEW_STREAM, WARNING, szTemp);               
          return NULL;
        }
      else
        {
          fovdPrintLog (LOG_DEBUG, "fpstInvGen_Init: Created stream no: %d\n", i);
        }
    }

  lpstGen->spstStream = NULL;

  /*
   * Create image stream if BI writing requested
   */

  if (stBgh.bWriteBillImage == TRUE)
    {
      fovdPrintLog (LOG_DEBUG, "fpstInvGen_Init: Creating image stream\n");
      if ((lpstGen->spstImage = fpstImageStream_New()) == NULL)
        {
          sprintf (szTemp, "Can't init image stream\n");
          macErrorMessage (GEN_NEW_STREAM, WARNING, szTemp);               
          return NULL;
        }
    }

  /*
   * For each disk type screate one stream
   */

  fovdPrintLog (LOG_DEBUG, "fpstInvGen_Init: Creating disk streams: %d\n", DISK_TYPES_NO);
  for (i = 0; i < DISK_TYPES_NO; i++)
    {
      fovdPrintLog (LOG_DEBUG, "fpstInvGen_Init: Creating disk stream: %d\n", i);
      if ((lpstGen->sapstItbStream[i] = fpstSingleStream_New(daenDiskType[i], 
                                                             INV_UNDEFINED_TYPE, 
                                                             UNDEFINED_BILL_MEDIUM)) == NULL)
        {
          sprintf (szTemp, "Can't create disk stream: %d\n", i);
          macErrorMessage (GEN_NEW_STREAM, WARNING, szTemp);
          return NULL;
        }
      else
        {
          fovdPrintLog (LOG_DEBUG, "fpstInvGen_Init: Created disk stream: %d\n", i);
        }
    }
  
  lpstGen->spstItbStream = NULL;
  
  lpstGen->spstUsedStreams = NULL;

  return lpstGen;
}

/**********************************************************************************************************************
 *
 * foenInvGen_Next
 *
 **********************************************************************************************************************
 */

toenBool foenInvGen_Next(tostInvGen *ppstGen, 
                         toenInvType poenInv, 
                         int poiBillMedium, 
                         int poiCustomerId, 
                         char *ppsnzCustCode,
                         toenBool poenInitializeLineList)
{
  int rc = 0;
  toenBool loenStatus;
  tostStreamSeq *lpstSeq = NULL;

  ASSERT(ppstGen != NULL);
  
  fovdPrintLog (LOG_DEBUG, "foenInvGen_Next: Finding stream\n");

  /*
   * Set up dynamic stream data
   */
  
  ppstGen->soiBillMedium = poiBillMedium;
  ppstGen->soiCustomerId = poiCustomerId;
  strcpy(ppstGen->sasnzCustCode, ppsnzCustCode);

  /*
   * Select and init next ITB disk stream
   */

  if ((ppstGen->spstStream = fpstStreamTab_Next(ppstGen->sapstStreamTab[poenInv], poiBillMedium)) == NULL)
    {
      sprintf (szTemp, "Can't select stream ptr from stream table\n");
      macErrorMessage (GEN_NEXT_DOC, WARNING, szTemp);
      return FALSE;
    }
  
  if ((loenStatus = foenStream_Next(ppstGen->spstStream, poiCustomerId, poenInitializeLineList)) == FALSE)
    {
      sprintf (szTemp, "Can't create next document with stream found int the stream table\n");
      macErrorMessage (GEN_NEXT_DOC, WARNING, szTemp);      
      return FALSE;
    }

  /*
   * Register usage o this stream in the list of used streams
   */

  if ((rc = foiStreamSeq_AddUnique(&(ppstGen->spstUsedStreams), ppstGen->spstStream)) < 0)
    {
      sprintf (szTemp, "Can't add stream to the lit of used streams\n");
      macErrorMessage (GEN_NEXT_DOC, WARNING, szTemp);      
      return FALSE;
    }
        
  /*
   * Handle bill image
   */

  if (stBgh.bWriteBillImage == TRUE)
    {
      if ((loenStatus = foenImageStream_Next(ppstGen->spstImage, poiCustomerId, poenInitializeLineList)) == FALSE)
        {
          sprintf (szTemp, "Can't create next document with stream found int the stream table\n");
          macErrorMessage (GEN_NEXT_DOC, WARNING, szTemp);      
          return FALSE;
        }

/*      if (poenInv == INV_DEFAULT_TYPE)
        {
          ppstGen->soiImgTypeId = 1;
        }
  */
      if (poenInv == INV_ENCLOSURE_TYPE)
        {
          ppstGen->soiImgTypeId = 3;
        }     
      else
        {
          ppstGen->soiImgTypeId = 1;
        }
    }

  /*
   * Select and init next ITB disk stream
   */

  switch (poiBillMedium)
    {
    case DISK_EXCEL_BILL_MEDIUM:
      fovdPrintLog (LOG_TIMM, "Using EXCEL type ITB for CID: %d\n", poiCustomerId);
      ppstGen->spstItbStream = ppstGen->sapstItbStream[DISK_EXCEL_FORMAT];
      break;

    case DISK_FIXED_BILL_MEDIUM:
      fovdPrintLog (LOG_TIMM, "Using FIXED type ITB for CID: %d\n", poiCustomerId);
      ppstGen->spstItbStream = ppstGen->sapstItbStream[DISK_FIXED_FORMAT];
      break;

    default:
      ppstGen->spstItbStream = NULL;
    }

  if (ppstGen->spstItbStream != NULL)
    {   
      if ((loenStatus = foenSingleStream_Next(ppstGen->spstItbStream, poiCustomerId, ppstGen->sasnzCustCode,
     poenInitializeLineList )) == FALSE)
        {
          sprintf (szTemp, "Can't create next document with stream found int the stream table\n");
          macErrorMessage (GEN_NEXT_DOC, WARNING, szTemp);      
          return FALSE;
        }
    }

  return TRUE;
}

/**********************************************************************************************************************
 *
 * foenInvGen_Print
 *
 **********************************************************************************************************************
 */

toenBool foenInvGen_Print(tostInvGen *ppstGen, char *ppsnzLine, char **papsnzArgv, int poiArgs)
{
  toenBool loenStatus;
  char lasnzLine[MAX_GEN_LINE_LEN];

  ASSERT(ppstGen != NULL);

  fovdPushFunctionName ("foenInvGen_Print");

  /*
   * Print image line - invoice copy
   */

  if (stBgh.bWriteBillImage == TRUE && ppsnzLine != NULL)
    {
      if ((loenStatus = foenImageStream_Print(ppstGen->spstImage, ppsnzLine)) == FALSE)
        {
          sprintf (szTemp, "Can't print line in the image store\n");
          macErrorMessage (GEN_PRINT, WARNING, szTemp);      
          fovdPopFunctionName ();  
          return FALSE;
        }
    }
  
  /*
   * What s to be printed: DOC or ITB or nothing
   */

  if (ppsnzLine == NULL)
    {
      /*
       * Disk stream is empty
       */

      if (ppstGen->spstItbStream != NULL)
        {
          /*
           * Print line to ITB file if stream created and fovdGenItb interface function used
           */
          
          if ((loenStatus = foenItbLineFormat(ppstGen->soiBillMedium, papsnzArgv, poiArgs, 
                                              lasnzLine, MAX_GEN_LINE_LEN)) == FALSE)
            {
              sprintf (szTemp, "Can't format itb line\n");
              macErrorMessage (GEN_FORMAT_ITB_LINE, WARNING, szTemp);
              fovdPopFunctionName ();  
              return FALSE;
            }
          
          if ((loenStatus = foenSingleStream_Print(ppstGen->spstItbStream, lasnzLine)) == FALSE)
            {
              sprintf (szTemp, "Can't print line in the itb store\n");
              macErrorMessage (GEN_PRINT, WARNING, szTemp);      
              fovdPopFunctionName ();  
              return FALSE;
            }
        }
    }
  else
    {
      /*
       * Print line in the main stream
       */
      
      if ((loenStatus = foenStream_Print(ppstGen->spstStream, ppsnzLine)) == FALSE)
        {
          sprintf (szTemp, "Can't print line in the main document store\n");
          macErrorMessage (GEN_PRINT, WARNING, szTemp);            
          fovdPopFunctionName ();  
          return FALSE;
        }
    }
  
  fovdPopFunctionName ();  
  return TRUE;
}

/**********************************************************************************************************************
 *
 * foenInvGen_Flush
 *
 **********************************************************************************************************************
 */

toenBool foenInvGen_Flush(tostInvGen *ppstGen)
{
  toenBool loenStatus;

  ASSERT(ppstGen != NULL);

  /*
   * Insert to BILL_IMAGES may fail then this action must stay at the beginning
   */

  if (stBgh.bWriteBillImage == TRUE)
    {
      if ((loenStatus = foenImageStream_Insert(ppstGen->spstImage, ppstGen->soiImgTypeId)) == FALSE)
        {
          sprintf (szTemp, "Can't insert image of the document to table BILL_IMAGES\n");
          macErrorMessage (GEN_INSERT_IMAGE, WARNING, szTemp);            
          return FALSE;
        }
      
      if ((loenStatus = foenImageStream_Flush(ppstGen->spstImage)) == FALSE)
        {
          sprintf (szTemp, "Can't flush image of the document\n");
          macErrorMessage (GEN_FLUSH_IMAGE, WARNING, szTemp);            
          return FALSE;
        }
    }

  /*
   * If failure during printing disk file then stop processing
   */

  if (ppstGen->spstItbStream != NULL)
    {
      if ((loenStatus = foenSingleStream_Flush(ppstGen->spstItbStream)) == FALSE)
        {
          sprintf (szTemp, "Can't flush itb\n");
          macErrorMessage (GEN_FLUSH_ITB, WARNING, szTemp);            
          return FALSE;
        }
    }

  /*
   * Write document in the file
   */

  if ((loenStatus = foenStream_Flush(ppstGen->spstStream)) == FALSE)
    {
      sprintf (szTemp, "Can't flush main document\n");
      macErrorMessage (GEN_FLUSH_DOCUMENT, WARNING, szTemp);            
      return FALSE;
    }

  return TRUE;
}

/**********************************************************************************************************************
 *
 * Function name:	foiInvGen_SavePoint
 *
 * Function call:	rc = foiInvGen_SavePoint()
 *
 * Returns:	int
 *
 *              Value on success:
 *
 *                      0
 *
 *		Value on failure:
 *
 *                      -1 - can not save the point on some stream
 *                      -2 -  can not clean the sequence of used streams
 *
 * Arguments:	ppstGen, tostInvGen * - the invoice generator structure keeping it's state info
 *
 **********************************************************************************************************************
 */

int foiInvGen_SavePoint(tostInvGen *ppstGen)
{
  int rc = 0;
  tostStream *lpstStream = NULL;
  tostStreamSeq *lpstSeq = NULL;

  ASSERT(ppstGen != NULL);

  /*
   * Save point in each used streams
   */
  
  lpstSeq = ppstGen->spstUsedStreams;
  while (lpstSeq != NULL && rc == 0)
    {
      lpstStream = lpstSeq->spstStream;
      if ((rc = foiStream_SavePoint(lpstStream)) < 0)
	{
	  rc = -1;
	}

      lpstSeq = lpstSeq->spstNext;
    }

  /*
   * free the list
   */

  if (rc == 0)
    {
      if ((rc = foiStreamSeq_Delete(&(ppstGen->spstUsedStreams))) < 0)
	{
	  rc = -2;
	}
    }

  return rc;
}

/**********************************************************************************************************************
 *
 * Function name:	foiInvGen_RollBackWork
 *
 * Function call:	rc = foiInvGen_RollBackWork()
 *
 * Returns:	int
 *
 *              Value on success:
 *
 *                      0
 *
 *		Value on failure:
 *
 *                      -1 - can not roll back work on some stream
 *                      -2 - can not clean the sequence of used streams
 *
 * Arguments:	ppstGen, tostInvGen * - the invoice generator structure keeping it's state info
 *
 **********************************************************************************************************************
 */

int foiInvGen_RollBackWork(tostInvGen *ppstGen)
{
  int rc = 0;
  tostStream *lpstStream;
  tostStreamSeq *lpstSeq, *lpstTmp;

  ASSERT(ppstGen != NULL);

  fovdPrintLog (LOG_DEBUG, "foiInvGen_RollBackWork: Roll back of the invoice file operation\n");  

  /*
   * Save point in each used streams
   */

  lpstSeq = ppstGen->spstUsedStreams;
  while (lpstSeq != NULL && rc == 0)
    {
      lpstStream = lpstSeq->spstStream;
      if ((rc = foiStream_RollBackWork(lpstStream)) < 0)
	{
	  rc = -1;
	}
      
      lpstSeq = lpstSeq->spstNext;
    }
  
  /*
   * free the list
   */

  if (rc == 0)
    {
      if ((rc = foiStreamSeq_Delete(&(ppstGen->spstUsedStreams))) < 0)
	{
	  rc = -2;
	}
    }

  return rc;
}


