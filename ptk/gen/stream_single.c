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

/*
 * external functions
 */

extern tostBuf *fpstFileMap(char *spsnzHeaderPath);

extern toenStreamType foenMapDocInv2Stream(toenDocType poenDoc, toenInvType poenInv);

extern char *fpsnzBillMediumDesPadded(int poiBillMedium);

/*
 * static variables
 */

static tostBuf *dpstBuf = NULL;

static char szTemp[PATH_MAX];

static char *doszExcelHeaderLine = 
"Numer telefonu;Data;Godzina;Rodzaj uslugi;Opis/kierunek;Wybrany numer;Czas polaczenia w minutach;Oplata PTK;Oplata TPSA;Oplaty razem;\n";


/**********************************************************************************************************************
 *
 * fpstSingleStream_New
 *
 **********************************************************************************************************************
 */

tostSingleStream *fpstSingleStream_New(toenDocType poenDoc, toenInvType poenInv, int poiBillMediumIndex)
{
  char lasnzPath[MAX_PATH_NAME_LEN];
  char *lpsnzDes;
  tostSingleStream *lpstStream;
  toenStreamType loenStream;

  fovdPrintLog (LOG_DEBUG, "fpstSingleStream_New: Creating single stream\n");

  /*
   * Map Doc to Stream
   */

  if ((loenStream = foenMapDocInv2Stream(poenDoc, poenInv)) == STREAM_UNDEFINED_TYPE)
    {
      sprintf (szTemp, "Can't map type: Doc: %d, Inv: %d\n", poenDoc, poenInv);
      macErrorMessage (STREAM_MAP_TYPE, WARNING, szTemp);
      return NULL;
    }
  else
    {
      fovdPrintLog (LOG_DEBUG, "fpstSingleStream_New: Doc: %d, Inv: %d -> Stream: %d\n", poenDoc, poenInv, loenStream);
    }

  /*
   * Get header file if necessary
   */
  
  if (dpstBuf == NULL)
    {
      strcpy(lasnzPath, stBgh.szLayoutDir);
      strcat(lasnzPath, "header_" GEN_VER "_" BGH_VER ".ps");  
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
  
  if ((lpstStream = (tostSingleStream *)calloc(1, sizeof(tostSingleStream))) == NULL)
    {
      sprintf (szTemp, "Can't malloc memory\n");
      macErrorMessage (STREAM_MALLOC, WARNING, szTemp);      
      return NULL;
    }
  
  /*
   * Fill the structure
   */

  lpstStream->soenStream = loenStream;  
  lpstStream->soiBillMediumIndex = poiBillMediumIndex;
  strcpy(lpstStream->sasnzPathName, stBgh.szPrintDir);
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
 * foenSingleStream_Next
 *
 **********************************************************************************************************************
 */

toenBool foenSingleStream_Next(tostSingleStream *ppstStream, 
			       int poiCustomerId, 
			       char *ppsnzCustCode, 
                               toenBool poenInitializeLineList)
{
  toenBool loenStatus;
  
  /*
   * Customer id for next document
   */
  
  ppstStream->soiCustomerId = poiCustomerId;
  if (ppsnzCustCode != NULL)
    {
      strcpy(ppstStream->sasnzCustCode, ppsnzCustCode);
    }
  else
    {
      memset(ppstStream->sasnzCustCode, 0x00, MAX_CUSTCODE_LEN);
    }
  
  /*
   * List must be deleted before next customer processing is started, except the dunning the same customer
   */

  if(TRUE == poenInitializeLineList)
  {
      /* Initializing the list without freeing it probably is a mistake
	 if ((loenStatus = foenLineList_Init(ppstStream->spstLineList)) == FALSE)
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
 * foenSingleStream_Print
 *
 **********************************************************************************************************************
 */

toenBool foenSingleStream_Print(tostSingleStream *ppstStream, char *ppsnzLine)
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
 * foenSingleStream_Flush
 *
 **********************************************************************************************************************
 */

toenBool foenSingleStream_Flush(tostSingleStream *ppstStream)
{
  char lasnzFile[MAX_PATH_NAME_LEN];
  char *lpsnzDes, *lpsnzType;
  toenBool loenStatus;
  int rc, i;
  toenBool loenAddHeader = TRUE;
  toenBool loenAddExcelHeader = FALSE;

  fovdPrintLog (LOG_DEBUG, "foenSingleStream_Flush: Starting\n");

  /*
   * Get the document type desc.
   */

  lpsnzType = dasnzStreamTypePrefix[ppstStream->soenStream];
  fovdPrintLog (LOG_DEBUG, "foenSingleStream_Flush: File name prefix: %s\n", lpsnzType);

  /*
   * Create type dependent file name
   */

  fovdPrintLog (LOG_DEBUG, "foenSingleStream_Flush: Stream type: %d\n", ppstStream->soenStream);
  switch (ppstStream->soenStream)
    {
    case STREAM_INV_TYPE:
    case STREAM_INV_MINUS_TYPE:
    case STREAM_INV_MINUSVAT_TYPE:      
    case STREAM_ENCLOSURE_TYPE:
    case STREAM_PAYMENT_TYPE:

      if ((lpsnzDes = fpsnzBillMediumDesPadded(ppstStream->soiBillMediumIndex)) == NULL)
        {
          sprintf (szTemp, "Bill medium index: %d out of array\n", ppstStream->soiBillMediumIndex);
          macErrorMessage (STREAM_BAD_BILL_MEDIUM, WARNING, szTemp);                       
          return FALSE;
        }

      sprintf(lasnzFile, 
              "%s/%s.%s.%08d.ps", 
              ppstStream->sasnzPathName, lpsnzDes, lpsnzType, ppstStream->soiCustomerId);
      break;

    case STREAM_WLL_TYPE:
    case STREAM_DNL_TYPE:
    case STREAM_INL_TYPE:
    case STREAM_INP_TYPE:    
      sprintf(lasnzFile, 
              "%s/%s.%08d.ps", 
              ppstStream->sasnzPathName, lpsnzType, ppstStream->soiCustomerId);
      break;
      
    case STREAM_ITBEXECEL_TYPE:
	loenAddExcelHeader = TRUE;
    case STREAM_ITBFIXED_TYPE:
      
      loenAddHeader = FALSE;
      sprintf(lasnzFile, 
              "%s/%s.DYSK", 
              ppstStream->sasnzPathName, ppstStream->sasnzCustCode);
      break;
      
    default:
      ASSERT(FALSE);
    }

  fovdPrintLog (LOG_DEBUG, "foenSingleStream_Flush: File name: %s\n", lasnzFile);

  /*
   * Open output file
   */

  fovdPrintLog (LOG_DEBUG, "fpstSingleStream_Flush: File is: %s\n", lasnzFile);  
  if ((ppstStream->sofilFile = open(lasnzFile, O_TRUNC | O_CREAT | O_WRONLY, STREAM_FILE_ACCESS_RIGHTS)) == -1)
    {
      sprintf (szTemp, "Can't open output file: %s, errno = %d\n", lasnzFile, errno);
      macErrorMessage (STREAM_OPEN_OUTPUT_FILE, WARNING, szTemp);      
      return FALSE;
    }

  /*
   * Add header file
   */

  if (loenAddHeader == TRUE)
    {
      fovdPrintLog (LOG_DEBUG, "fpstSingleStream_Flush: Adding header\n");
      if ((rc = write(ppstStream->sofilFile, dpstBuf->spsnVal, dpstBuf->soiLen)) == -1)
        {
          sprintf (szTemp, "Can't write header to the output file: %\n", lasnzFile);
          macErrorMessage (STREAM_WRITE_OUTPUT_FILE, WARNING, szTemp); 
          return FALSE;
        }
    }

  /*
   * Add excel header 
   */

  if (loenAddExcelHeader == TRUE)
    {
      fovdPrintLog (LOG_DEBUG, "fpstSingleStream_Flush: Adding excel  header\n");
      if ((rc = write(ppstStream->sofilFile, doszExcelHeaderLine, strlen(doszExcelHeaderLine))) == -1)
        {
          sprintf (szTemp, "Can't write excel header to the output file: %\n", lasnzFile);
          macErrorMessage (STREAM_WRITE_OUTPUT_FILE, WARNING, szTemp); 
          return FALSE;
        }
    }
  
  /*
   * Write all lines from the queue
   */
  
  fovdPrintLog (LOG_DEBUG, "fpstSingleStream_Flush: Writing document\n");
  if ((loenStatus = foenLineList_Write(ppstStream->spstLineList, ppstStream->sofilFile)) == FALSE)
    {
      sprintf (szTemp, "Can't write document to the output file: %\n", lasnzFile);
      macErrorMessage (STREAM_WRITE_OUTPUT_FILE, WARNING, szTemp); 
      return FALSE;
    }

  /*
   * close output file
   */

  if ((rc = close(ppstStream->sofilFile)) == -1)
    {
      sprintf (szTemp, "Can't close the output file: %\n", lasnzFile);
      macErrorMessage (STREAM_CLOSE_OUTPUT_FILE, WARNING, szTemp); 
      return FALSE;
    }
  
  /*
   * Clean module memory
   */

  fovdPrintLog (LOG_DEBUG, "fpstSingleStream_Flush: Deleting line list\n");
  if ((loenStatus = foenLineList_Delete(ppstStream->spstLineList)) == FALSE)
    {
      sprintf (szTemp, "Can't delete the line list for the output file: %\n", lasnzFile);
      macErrorMessage (STREAM_DELETE_LINE_LIST, WARNING, szTemp);       
      return FALSE;
    } 
  
  return TRUE;
}

/**********************************************************************************************************************
 *
 * Function name:	foiingleStream_SavePoint
 *
 * Function call:	rc = foiSingleStream_SavePoint()
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
 * Arguments:	ppstStream, tostSingleStream * - the stream state container
 *
 **********************************************************************************************************************
 */

int foiSingleStream_SavePoint(tostSingleStream *ppstStream)
{
  return 0;
}

/**********************************************************************************************************************
 *
 * Function name:	foiSingleStream_RollBackWork
 *
 * Function call:	rc = foiSingleStream_RollBackWork()
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
 * Arguments:	ppstStream, tostSingleStream * - the stream state container
 * 
 *
 **********************************************************************************************************************
 */

int foiSingleStream_RollBackWork(tostSingleStream *ppstStream)
{
  return 0;
}


