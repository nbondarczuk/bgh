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
#include <varargs.h>

#include "types.h"
#include "bgh.h"
#include "line_list.h"
#include "stream.h"
#include "generator.h"
#include "generator_inv.h"

/*
 * external variables
 */

extern stBGHGLOB	stBgh;

/*
 * external functions
 */

extern tostInvGen *fpstInvGen_New();

extern toenBool foenInvGen_Print(tostInvGen *ppstGen, char *ppsnzLine, char **papsnzArgv, int poiArgs);

extern int foiInvGen_RollBackWork(tostInvGen *ppstGen);

extern int foiInvGen_SavePoint(tostInvGen *ppstGen);

extern tostDocGen *fpstDocGen_New(toenDocType poenDoc);

extern int foiDocGen_RollBackWork(tostDocGen *ppstGen);

extern int foiDocGen_SavePoint(tostDocGen *ppstGen);

/*
 * static variables
 */

static void *dpvGen = NULL;
static char szTemp[PATH_MAX];

/**********************************************************************************************************************
 *
 * foenGen_Init
 *
 **********************************************************************************************************************
 */

toenBool foenGen_Init()
{
  toenGenMode loenMode;
  toenDocType loenDoc;
  toenGenType loenGen = GEN_DOC_TYPE; 

  fovdPushFunctionName ("foenGen_Init");

  fovdPrintLog (LOG_DEBUG, "foenGen_Init: Initializing generator\n");

  /*
   * Check whole environment
   */

  if (stBgh.lCustSetSize > 1)
    {
      fovdPrintLog (LOG_DEBUG, "foenGen_Init: Mode selected: GEN_MULTI_MODE\n");
      loenMode = GEN_MULTI_MODE;
    }
  else
    {
      fovdPrintLog (LOG_DEBUG, "foenGen_Init: Mode selected: GEN_SINGLE_MODE\n");
      loenMode = GEN_SINGLE_MODE;
    }

  switch (stBgh.enTimmProcessing)
    {
    case INV_TYPE: loenDoc = DOC_INV_TYPE; loenGen = GEN_INV_TYPE; break;         
    case DNL_DWH:  loenDoc = DOC_DNL_TYPE; break;                    
    case WLL_DWH:  loenDoc = DOC_WLL_TYPE; break;
    case INH_INP:  loenDoc = DOC_INP_TYPE; break;
    case INH_INL:  loenDoc = DOC_INL_TYPE; break;
    default:
      sprintf (szTemp, "Incorrect processing mode selected\n");
      macErrorMessage (GEN_INCORRECT_DOC_TYPE, WARNING, szTemp);
      fovdPopFunctionName ();
      return FALSE;
    }  

  fovdPrintLog (LOG_DEBUG, "foenGen_Init: doc: %d, Gen: %d\n", loenDoc, loenGen);

  /*
   * Create virtual generator
   */
  
  if (loenGen == GEN_INV_TYPE)
    {
      fovdPrintLog (LOG_DEBUG, "foenGen_Init: Creating virtual generator for invoice\n");
      dpvGen = (void *)fpstInvGen_New();      
    }
  else
    {
      fovdPrintLog (LOG_DEBUG, "foenGen_Init: Creating virtual generator for documents\n");
      dpvGen = (void *)fpstDocGen_New(loenDoc);
    }
  
  if (dpvGen == NULL)
    {
      sprintf (szTemp, "Can't create document generator\n");
      macErrorMessage (GEN_CREATE_GENERATOR, WARNING, szTemp);
      fovdPopFunctionName ();
      return FALSE;
    }

  return TRUE;
}

/**********************************************************************************************************************
 *
 * foenGen_NextDoc
 *
 **********************************************************************************************************************
 */

toenBool foenGen_NextDoc(toenDocType poenDoc, 
                         toenInvType poenInv, 
                         int poiBillMedium, 
                         int poiCustomerId, 
                         char *ppsnzCustCode, 
                         toenBool poenInitializeLineList)
{
  toenBool loenStatus;
  
  fovdPrintLog (LOG_DEBUG, "foenGen_NextDoc: Doc: %d, Inv: %d, BM:%d, CustId: %d, CustCode: %s\n", 
                poenDoc, 
		poenInv, 
		poiBillMedium, 
		poiCustomerId, 
		ppsnzCustCode);
  
  switch (*(toenGenType *)dpvGen)
    {
    case GEN_INV_TYPE:
      fovdPrintLog (LOG_DEBUG, "foenGen_NextDoc: Creating INVOICE\n");
      loenStatus = foenInvGen_Next((tostInvGen *)dpvGen, 
				   poenInv, 
				   poiBillMedium, 
				   poiCustomerId, 
				   ppsnzCustCode,
                                   poenInitializeLineList);
      break;
      
    case GEN_DOC_TYPE:
      fovdPrintLog (LOG_DEBUG, "foenGen_NextDoc: Creating DOCUMENT\n");
      loenStatus = foenDocGen_Next((tostDocGen *)dpvGen, poiCustomerId);
      break;
      
    default:
      ASSERT(FALSE);
    }
  
  return loenStatus;
}

/**********************************************************************************************************************
 *
 * fovdGen
 *
 **********************************************************************************************************************
 */

void fovdGen(va_alist)
  va_dcl
{
  toenBool loenStatus;
  char *lapsnzArg[MAX_POSTSCRIPT_ARG];
  char *lpsnzTag, *lpsnzStr;
  char lasnzBuf[MAX_GEN_ARG_LEN];
  char lasnzLine[MAX_GEN_LINE_LEN];
  int loiArgNo = 0, i;
  va_list args;
  
  fovdPushFunctionName ("fovdGen");
  
  /*
   * Fill table with arguments
   */

  va_start(args);
  lpsnzTag = va_arg(args, char *);

  i = 0;
  while ((lpsnzStr = va_arg(args, char *)) != (char *)EOL) 
    {
      ASSERT(i < MAX_POSTSCRIPT_ARG);
      lapsnzArg[i++] = lpsnzStr;
    }

  va_end(args);

  /*
   * Init output line
   */

  memset(lasnzLine, 0, MAX_GEN_LINE_LEN);

  /*
   * Get all arguments back and create output line
   */

  /*
  fovdPrintLog (LOG_DEBUG, "fovdGen: Tag: %s, Args: %d\n", lpsnzTag, i);
  */
  for (i--;i >= 0; i--)
    {
      memset(lasnzBuf, 0, MAX_GEN_ARG_LEN);
      fovdFormatPSArg(lasnzBuf, MAX_GEN_ARG_LEN, lapsnzArg[i]);
      strncat(lasnzLine, lasnzBuf, MAX_GEN_LINE_LEN);
    }
  
  strncat(lasnzLine, lpsnzTag, MAX_GEN_LINE_LEN);  
  strncat(lasnzLine, "\n", MAX_GEN_LINE_LEN);  
  /*
  fovdPrintLog (LOG_DEBUG, "fovdGen: Arg: %s\n", lasnzLine);
  */
  switch (*(toenGenType *)dpvGen)
    {
    case GEN_INV_TYPE:
      loenStatus = foenInvGen_Print((tostInvGen *)dpvGen, lasnzLine, NULL, 0);
      break;
      
    case GEN_DOC_TYPE:
      loenStatus = foenDocGen_Print((tostDocGen *)dpvGen, lasnzLine);
      break;
      
    default:
      ASSERT(FALSE);
    }
  
  fovdPopFunctionName ();  
}

/**********************************************************************************************************************
 *
 * fovdGenItb
 *
 **********************************************************************************************************************
 */

void fovdGenItb(va_alist)
     va_dcl
{
  toenBool loenStatus;
  char *lapsnzArg[MAX_POSTSCRIPT_ARG];
  char *lpsnzTag, *lpsnzStr;
  char lasnzBuf[MAX_GEN_ARG_LEN];
  char lasnzLine[MAX_GEN_LINE_LEN];
  int loiArgNo = 0, i;
  va_list args;
  
  fovdPushFunctionName ("fovdGenItb");

  /*
   * Fill table with arguments
   */

  va_start(args);
  lpsnzTag = va_arg(args, char *);
  
  i = 0;
  fovdPrintLog (LOG_DEBUG, "fovdGenItb: Line: ");
  while ((lpsnzStr = va_arg(args, char *)) != (char *)EOL) 
    {
      ASSERT(i < MAX_POSTSCRIPT_ARG);
      lapsnzArg[i++] = lpsnzStr;
      fovdPrintLog (LOG_DEBUG, "%s ", lpsnzStr);
    }

  fovdPrintLog (LOG_DEBUG, "\n");

  va_end(args);
  
  switch (*(toenGenType *)dpvGen)
    {
    case GEN_INV_TYPE:
      loenStatus = foenInvGen_Print((tostInvGen *)dpvGen, NULL, lapsnzArg, i);
      break;
      
    default:
      ASSERT(FALSE);
    }
  
  fovdPopFunctionName ();  
}

/**********************************************************************************************************************
 *
 * foenGen_Flush
 *
 **********************************************************************************************************************
 */

toenBool foenGen_FlushAll()
{
  toenBool loenStatus;

  fovdPrintLog (LOG_DEBUG, "foenGen_FlushAll: Printing document file\n");
  switch (*(toenGenType *)dpvGen)
    {
    case GEN_INV_TYPE:
      fovdPrintLog (LOG_DEBUG, "foenGen_FlushAll: Printing INVOICE\n");
      loenStatus = foenInvGen_Flush((tostInvGen *)dpvGen);
      break;
      
    case GEN_DOC_TYPE:
      fovdPrintLog (LOG_DEBUG, "foenGen_FlushAll: Printing DOCUMENT\n");
      loenStatus = foenDocGen_Flush((tostDocGen *)dpvGen);
      break;
      
    default:
      ASSERT(FALSE);
    }
  
  return loenStatus;
}

/**********************************************************************************************************************
 *
 * Function name:	foiGen_SavePoint
 *
 * Function call:	rc = foiGen_SavePoint()
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
 * Arguments:	void
 *
 * Description: The processed document is correct but the point must be saved for later use in roll back
 *              function when some error during DB operation has happened. In multi document mode this is
 *              only storing the file offset. 
 *
 **********************************************************************************************************************
 */

int foiGen_SavePoint()
{
  int rc = 0;
  
  switch (*(toenGenType *)dpvGen)
    {
    case GEN_INV_TYPE:
      rc = foiInvGen_SavePoint((tostInvGen *)dpvGen);
      break;
      
    case GEN_DOC_TYPE:
      rc = foiDocGen_SavePoint((tostDocGen *)dpvGen);
      break;
      
    default:
      ASSERT(FALSE);
    }
  
  if (rc < 0)
    {
      rc = -1;
    }

  return rc;
}

/**********************************************************************************************************************
 *
 * Function name:	foiGen_RollBackWork
 *
 * Function call:	rc = foiGen_RollBackWork()
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
 * Arguments:	void
 *
 * Description: Some error may happen during time interval between flushing the document and the moment
 *              when DB operations re commited including the commit operation. The file operations must
 *              be rolled back to the state before the document being produced. In single document mode 
 *              this is file removal. In multi document mode this is truncate to the file offset. 
 *
 **********************************************************************************************************************
 */

int foiGen_RollBackWork()
{
  int rc = 0;

  fovdPrintLog (LOG_DEBUG, "foiGen_RollBackWork: Roll back of the file operation\n");  
  switch (*(toenGenType *)dpvGen)
    {
    case GEN_INV_TYPE:
      rc = foiInvGen_RollBackWork((tostInvGen *)dpvGen);
      break;
      
    case GEN_DOC_TYPE:
      rc = foiDocGen_RollBackWork((tostDocGen *)dpvGen);
      break;
      
    default:
      ASSERT(FALSE);
    }

  if (rc < 0)
    {
      rc = -1;
    }
  
  return rc;
}








