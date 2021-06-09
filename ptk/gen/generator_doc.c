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

extern tostStream *fpstStream_New(toenDocType poenDoc, toenInvType poenInv, int poiBillMediumIndex);

extern toenBool foenStream_Next(tostStream *ppstStream, int poiCustomerId, toenBool poenInitializeLineList);

extern toenBool foenStream_Print(tostStream *ppstStream, char *ppsnzLine);

extern toenBool foenStream_Flush(tostStream *ppstStream);

extern int foiStream_SavePoint(tostStream *ppstStream);

extern int foiStream_RollBackWork(tostStream *ppstStream);

/**********************************************************************************************************************
 *
 * fpstDocGen_New
 *
 **********************************************************************************************************************
 */

tostDocGen *fpstDocGen_New(toenDocType poenDoc)
{
  tostDocGen *lpstGen;
  toenStreamType loenStream;

  if ((lpstGen = (tostDocGen *)calloc(1, sizeof(tostDocGen))) == NULL)
    {
      return NULL;
    }

  lpstGen->soenGen = GEN_DOC_TYPE;
  
  if ((lpstGen->spstStream = (tostStream *)fpstStream_New(poenDoc, INV_UNDEFINED_TYPE, INV_UNDEFINED_TYPE)) == NULL)
    {
      return NULL;
    }
  
  return lpstGen;
}

/**********************************************************************************************************************
 *
 * foenDocGen_Next
 *
 **********************************************************************************************************************
 */

toenBool foenDocGen_Next(tostDocGen *ppstGen, int poiCustomerId)
{
  toenBool loenStatus;
  
  if ((loenStatus = foenStream_Next(ppstGen->spstStream, poiCustomerId, TRUE)) == FALSE)
    {
      return FALSE;
    }

  return TRUE;
}

/**********************************************************************************************************************
 *
 * foenDocGen_Print
 *
 **********************************************************************************************************************
 */

toenBool foenDocGen_Print(tostDocGen *ppstGen, char *ppsnzLine)
{
  toenBool loenStatus;
  
  if ((loenStatus = foenStream_Print(ppstGen->spstStream, ppsnzLine)) == FALSE)
    {
      return FALSE;
    }

  return TRUE;
}

/**********************************************************************************************************************
 *
 * foenDocGen_Flush
 *
 **********************************************************************************************************************
 */

toenBool foenDocGen_Flush(tostDocGen *ppstGen)
{
  toenBool loenStatus;

  if ((loenStatus = foenStream_Flush(ppstGen->spstStream)) == FALSE)
    {
      return FALSE;
    }

  return TRUE;
}

/**********************************************************************************************************************
 *
 * Function name:	foiDocGen_SavePoint
 *
 * Function call:	rc = foiDocGen_SavePoint()
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
 * Arguments:	ppstGen, tostDocGen * - the invoice generator structure keeping it's state info
 *
 **********************************************************************************************************************
 */

int foiDocGen_SavePoint(tostDocGen *ppstGen)
{
  int rc = 0;

  if ((rc = foiStream_SavePoint(ppstGen->spstStream)) < 0)
    {
      rc = -1;
    }

  return rc;
}

/**********************************************************************************************************************
 *
 * Function name:	foiDocGen_RollBackWork
 *
 * Function call:	rc = foiDocGen_RollBackWork()
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
 * Arguments:	ppstGen, tostDocGen * - the invoice generator structure keeping it's state info
 *
 **********************************************************************************************************************
 */

int foiDocGen_RollBackWork(tostDocGen *ppstGen)
{
  int rc = 0;
  
  if ((rc = foiStream_RollBackWork(ppstGen->spstStream)) < 0)
    {
      rc = -1;
    }
  
  return rc;
}


