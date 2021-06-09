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
extern long golLastCustId; /* last customer id */

/*
 * external functions
 */

extern tostBuf *fpstFileMap(char *spsnzHeaderPath);
extern char *fpsnzBillMediumDesPadded(int poiBillMedium);
extern toenStreamType foenMapDocInv2Stream(toenDocType poenDoc, toenInvType poenInv);

/*
 * static variables
 */

static tostBuf *dpstBuf = NULL;
static char szTemp[PATH_MAX];

/**********************************************************************************************************************
 *
 * fpstMultiStream_New
 *
 **********************************************************************************************************************
 */

tostMultiStream *fpstMultiStream_New(toenDocType poenDoc, 
				     toenInvType poenInv, 
				     int poiBillMediumIndex)
{
  char lasnzPath[MAX_PATH_NAME_LEN];
  char *lpsnzDes;
  tostMultiStream *lpstStream;
  toenStreamType loenStream;
  
  /*
   * Map Doc to Stream
   */

  if ((loenStream = foenMapDocInv2Stream(poenDoc, poenInv)) == STREAM_UNDEFINED_TYPE)
    {
      return NULL;
    }

  /*
   * Get header file if necessary
   */
  
  if (dpstBuf == NULL)
    {
      strcpy(lasnzPath, stBgh.szLayoutDir);
      strcat(lasnzPath, "header_" GEN_VER "_" BGH_VER ".ps" );  
      if ((dpstBuf = fpstFileMap(lasnzPath)) == NULL)
        {
          sprintf (szTemp, "Can't map file: %s\n", lasnzPath);
          macErrorMessage (STREAM_MAP_FILE, WARNING, szTemp);      
          return NULL;
        }
    }

  /*
   * Alloc memory 
   */
  
  if ((lpstStream = (tostMultiStream *)calloc(1, sizeof(tostMultiStream))) == NULL)
    {
      sprintf (szTemp, "Can't malloc memory\n");
      macErrorMessage (STREAM_MALLOC, WARNING, szTemp);      
      return NULL;
    }

  /*
   * Fill the structure
   */

  lpstStream->soilFileOffset = 0;
  lpstStream->soenStream = loenStream;
  lpstStream->soiBillMediumIndex = poiBillMediumIndex;
  strcpy(lpstStream->sasnzPathName, stBgh.szPrintDir);
  lpstStream->spszFileName = NULL;
  lpstStream->soiPID = getpid();
  lpstStream->soiSeqNo = 1;
  lpstStream->soiDocNo = 1;
  lpstStream->soiMaxDocNo = stBgh.lCustSetSize;
  if ((lpstStream->spstLineList = fpstLineList_New()) == NULL)
    {
      sprintf (szTemp, "Can't create line list for stream\n");
      macErrorMessage (STREAM_CREATE_LINE_LIST, WARNING, szTemp);      
      return NULL;
    }
  
  return lpstStream;
}

/**********************************************************************************************************************
 *
 * foenMultiStream_Next
 *
 **********************************************************************************************************************
 */

toenBool foenMultiStream_Next(tostMultiStream *ppstStream, 
			      int poiCustomerId, 
			      toenBool poenInitializeLineList)
{
  toenBool loenStatus;
  
  ASSERT(ppstStream != NULL);

  fovdPrintLog (LOG_DEBUG, "foenMultiStream_Next: Next customer: %ld\n", poiCustomerId);  

  ppstStream->soiCustomerId = poiCustomerId;
  if (TRUE == poenInitializeLineList)
    {
      
      /* 
       * Initializing the list without freeing it probably is a mistake
       * if ((loenStatus = foenLineList_Init(ppstStream->spstLineList)) == FALSE)
       */
      
      if ((loenStatus = foenLineList_Delete(ppstStream->spstLineList)) == FALSE)
	{
	  sprintf (szTemp, "Can't init line list for customer: %d\n", poiCustomerId);
	  macErrorMessage (STREAM_INIT_LINE_LIST, WARNING, szTemp);      
	  return FALSE;
	}
    }  
  
  return TRUE;
}

/**********************************************************************************************************************
 *
 * foenMultiStream_Print
 *
 **********************************************************************************************************************
 */

toenBool foenMultiStream_Print(tostMultiStream *ppstStream, char *ppsnzLine)
{
  toenBool loenStatus;
  
  if ((loenStatus = foenLineList_Append(ppstStream->spstLineList, ppsnzLine)) == FALSE)
    {
      sprintf (szTemp, "Can't append line to the line list: %\n", ppsnzLine);
      macErrorMessage (STREAM_APPEND_LINE_LIST, WARNING, szTemp);             
      return FALSE;
    }
  
  return TRUE;
}

/**********************************************************************************************************************
 *
 * foenMultiStream_Flush
 *
 **********************************************************************************************************************
 */

toenBool foenMultiStream_Flush(tostMultiStream *ppstStream)
{ 
  static char lasnzFile[MAX_PATH_NAME_LEN];
  char *lpsnzDes, *lpsnzType;
  toenBool loenStatus;
  int rc, i;

  ASSERT(ppstStream != NULL);
 
  fovdPrintLog (LOG_DEBUG, "foenMultiStream_Flush: Flushing document for: %d\n", ppstStream->soiCustomerId);
  
  /*
   * Get the document type desc.
   */
  
  lpsnzType = dasnzStreamTypePrefix[ppstStream->soenStream];
  
  /*
   * If first document in the file then file name must be created
   * from scratch.
   */

  if (ppstStream->soiDocNo == 1)
    {
      /*
       * Create type dependent file name
       */

      if (ppstStream->spszFileName != NULL)
	{
	  free(ppstStream->spszFileName);
	}
      
      switch (ppstStream->soenStream)
        {
        case STREAM_INV_TYPE:
        case STREAM_INV_MINUS_TYPE:
        case STREAM_INV_MINUSVAT_TYPE:      
        case STREAM_ENCLOSURE_TYPE:
	case STREAM_PAYMENT_TYPE:
          
	  fovdPrintLog (LOG_DEBUG, "foenMultiStream_Flush, BillMedium index %d\n", ppstStream->soiBillMediumIndex); 
	  
          if ((lpsnzDes = fpsnzBillMediumDesPadded(ppstStream->soiBillMediumIndex )) == NULL)
            {
              sprintf (szTemp, "Bill medium index: %d out of array\n", ppstStream->soiBillMediumIndex);
              macErrorMessage (STREAM_BAD_BILL_MEDIUM, WARNING, szTemp);                       
              return FALSE;
            }
          
          sprintf(lasnzFile, 
                  "%s/%s.%s.%06d.%08d.ps", 
                  ppstStream->sasnzPathName, 
                  lpsnzDes, 
                  lpsnzType, 
                  ppstStream->soiPID,
                  ppstStream->soiSeqNo);        
          break;
          
        case STREAM_WLL_TYPE:
        case STREAM_DNL_TYPE:
        case STREAM_INL_TYPE:
        case STREAM_INP_TYPE:

          sprintf(lasnzFile, 
                  "%s/%s.%06d.%08d.ps", 
                  ppstStream->sasnzPathName,                   
                  lpsnzType, 
                  ppstStream->soiPID,
                  ppstStream->soiSeqNo);        
          break;
          
        default:
          ASSERT(FALSE);
        }

      ppstStream->spszFileName = strdup(lasnzFile);

      /*
       * One open action for one file
       */
      
      if ((ppstStream->sofilFile = open(lasnzFile, O_WRONLY | O_CREAT, STREAM_FILE_ACCESS_RIGHTS)) == NULL)
        {
          sprintf (szTemp, "Can't open output file: %\n", lasnzFile);
          macErrorMessage (STREAM_OPEN_OUTPUT_FILE, WARNING, szTemp);                
          return FALSE;
        }
      
      /*
       * Add header file
       */
      
      if ((rc = write(ppstStream->sofilFile, dpstBuf->spsnVal, dpstBuf->soiLen)) == -1)
        {
          sprintf (szTemp, "Can't write header to the output file: %\n", lasnzFile);
          macErrorMessage (STREAM_WRITE_OUTPUT_FILE, WARNING, szTemp);           
          return FALSE;
        }      
    }

  /*
   * Write all lines from the queue
   */
  
  if ((loenStatus = foenLineList_Write(ppstStream->spstLineList, ppstStream->sofilFile)) == FALSE)
    {
      sprintf (szTemp, "Can't write document to the output file: %\n", lasnzFile);
      macErrorMessage (STREAM_WRITE_OUTPUT_FILE, WARNING, szTemp);       
      return FALSE;
    }
  
  /*
   * Clean module memory
   */
  
  if ((loenStatus = foenLineList_Delete(ppstStream->spstLineList)) == FALSE)
    {
      sprintf (szTemp, "Can't delete the line list for the output file: %\n", lasnzFile);
      macErrorMessage (STREAM_DELETE_LINE_LIST, WARNING, szTemp);
      return FALSE;
    }       

  fovdPrintLog (LOG_DEBUG, "foenMultiStream_Flush: Flushed document for: %d\n", ppstStream->soiCustomerId);

  return TRUE;
}

/**********************************************************************************************************************
 *
 * Function name:	foiMultiStream_SavePoint
 *
 * Function call:	rc = foiMultiStream_SavePoint()
 *
 * Returns:	int
 *
 *              Value on success:
 *
 *                      0
 *
 *		Value on failure:
 *
 *                      -1 - can't do lseek on output stream file
 *
 * Exceptions:  WARNING, STREAM_SAVEPOINT_LSEEK_FILE
 *
 * Arguments:	ppstStream, tostMultiStream * - the stream state container
 *
 **********************************************************************************************************************
 */

int foiMultiStream_SavePoint(tostMultiStream *ppstStream)
{
  int rc = 0;
  off_t loilOffset = -1; 

  ASSERT(ppstStream != NULL);

  /*
   * Store point where the document is started
   */
  
  if ((loilOffset = lseek(ppstStream->sofilFile, (off_t)0L, SEEK_CUR)) == (off_t) -1)
    {
      sprintf (szTemp, "Can't lseek output file, error: %s\n", strerror(errno));
      macErrorMessage (STREAM_SAVEPOINT_LSEEK_FILE, WARNING, szTemp);
      rc = -1;
    }
  
  if (rc == 0)
    {
      fovdPrintLog (LOG_DEBUG, "foiMultiStream_SavePoint: customer: %d, offset: %ld\n", ppstStream->soiCustomerId, loilOffset);  
      ppstStream->soilFileOffset = loilOffset;

      /*
       * Control document number
       */

      if (ppstStream->soiDocNo >= ppstStream->soiMaxDocNo)
	{
	  /*
	   * This is the last document in the file
	   */
	  
	  if ((rc = close(ppstStream->sofilFile)) == -1)
	    {
	      sprintf (szTemp, "Can't close the output file: %\n", ppstStream->spszFileName);  
	      macErrorMessage (STREAM_CLOSE_OUTPUT_FILE, WARNING, szTemp);          
	      rc = -2;
	    }      
	  
	  /*
	   * Next file
	   */
	  
	  if (rc == 0)
	    {
	      ppstStream->soiDocNo = 1;
	      ppstStream->soiSeqNo++;
	      ppstStream->soilFileOffset = 0L;
	    }
	}
      else
	{
	  ppstStream->soiDocNo++;
	}
    }
  
  return rc;
}

/**********************************************************************************************************************
 *
 * Function name:	foiMultiStream_RollBackWork
 *
 * Function call:	rc = foiMultiStream_RollBackWork()
 *
 * Returns:	int
 *
 *              Value on success:
 *
 *                      0
 *
 *		Value on failure:
 *
 *                      -1 - can't unlink file with one first document
 *                      -2 - can't do truncate on file descriptior of the open output file
 *
 * Exceptions:  WARNING, STREAM_ROLLBACK_UNLINK_FILE
 *              WARNING, STREAM_ROLLBACK_TRUNCATE_FILE
 *
 * Arguments:	ppstStream, tostMultiStream * - the stream state container
 * 
 *
 **********************************************************************************************************************
 */

int foiMultiStream_RollBackWork(tostMultiStream *ppstStream)
{
  int rc = 0;

  fovdPrintLog (LOG_DEBUG, "foiMultiStream_RollBackWork: Roll back of operation for: cust: %d, type: %d, docno: %d, seqno: %d, off: %ld\n", 
		ppstStream->soiCustomerId, 
		ppstStream->soenStream,
		ppstStream->soiDocNo,
		ppstStream->soiSeqNo,
		ppstStream->soilFileOffset);

  fovdPrintLog (LOG_MAX, "File operation roll back\n");
  
  ASSERT(ppstStream != NULL);
  
  /*
   * Only the last one procesed
   */
  
  if (ppstStream->soiCustomerId != golLastCustId)
    {
      fovdPrintLog (LOG_DEBUG, "foiMultiStream_RollBackWork: Not current customer: %ld\n", ppstStream->soiCustomerId);  
      return rc;
    }
  else
    {
      fovdPrintLog (LOG_DEBUG, "foiMultiStream_RollBackWork: customer: %d\n", ppstStream->soiCustomerId);  
    }

  /*
   * Which document in the file, fist one must cause the file removal
   */

  if (ppstStream->soiDocNo == 1)
    {
      fovdPrintLog (LOG_DEBUG, "foiMultiStream_RollBackWork: Removing file: %s\n", ppstStream->spszFileName);  

      ASSERT (ppstStream->soilFileOffset == 0);

      if ((rc = unlink(ppstStream->spszFileName)) == -1)
	{
	  sprintf (szTemp, "Can't unlink output file with one first document, file: %s, error: %s\n", ppstStream->spszFileName, strerror(errno));
	  macErrorMessage (STREAM_ROLLBACK_UNLINK_FILE, WARNING, szTemp);
	  rc = -1;
	}
      else
	{
	  fovdPrintLog (LOG_MAX, "Successfully unlinked file: %s\n", ppstStream->spszFileName);
	}
    }
  else 
    {
      fovdPrintLog (LOG_DEBUG, "foiMultiStream_RollBackWork: Truncating document no: %d from file: %s to offset: %ld\n", 
		    ppstStream->soiDocNo, 
		    ppstStream->spszFileName,
		    ppstStream->soilFileOffset);  
      
      ASSERT (ppstStream->soilFileOffset > 0);
      
      /*
       * Restore file size to state before the creation of the current document
       */
      
      if ((rc = ftruncate(ppstStream->sofilFile, ppstStream->soilFileOffset)) == -1)
	{
	  sprintf (szTemp, "Can't truncate output file: %s, error: %s\n", ppstStream->spszFileName, strerror(errno));
	  macErrorMessage (STREAM_ROLLBACK_TRUNCATE_FILE, WARNING, szTemp);
	  rc = -2;
	}
      else
	{
	  fovdPrintLog (LOG_MAX, "Successfully truncated file: %s\n", ppstStream->spszFileName);
	}
    }
  
  return rc;
}


