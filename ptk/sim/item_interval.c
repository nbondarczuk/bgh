#include <string.h>
#include <stdlib.h>

#include "bgh.h"
#include "protos.h"
#include "types.h"
#include "parser.h"
#include "strutl.h"
#include "item_list.h"
#include "timm.h"

static char szTemp[128];

/**************************************************************************************************
 *                                                                                          
 * METHOD : MakeRecord
 *                                                                                          
 * DESCRIPTION : 
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenItemInterval_FillRecord(tostItemInterval *ppstItem, tostIntervalRecord *ppstRecord)
{
  fovdPrintLog (LOG_DEBUG, "foenItemInterval_FillRecord: Interval: [%s, %s]\n", ppstItem->sasnzEndDate, ppstItem->sasnzStartDate);
  strcpy(ppstRecord->sasnzStartDate, ppstItem->sasnzStartDate);
  strcpy(ppstRecord->sasnzEndDate, ppstItem->sasnzEndDate);

  ppstRecord->sasnzState[0] = ppstItem->sochState;
  ppstRecord->sasnzState[1] = '\0';
  
  return TRUE;
}

/**************************************************************************************************
 *                                                                                          
 * METHOD : Gen
 *                                                                                          
 * DESCRIPTION : 
 *                                                                                          
 **************************************************************************************************
 */

char gochItemInterval_State;

toenBool foenItemInterval_Gen(tostItemInterval *ppstItem)
{
  toenBool loenStatus;
  static tostIntervalRecord lostIntervalRecord;

  loenStatus = foenItemInterval_FillRecord(ppstItem, &lostIntervalRecord);
  if (loenStatus == FALSE)
    {
      sprintf (szTemp, "fpstItemInterval_Gen: Can't fill Interval record\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName();
      return FALSE;
    }

  fovdPrintLog (LOG_DEBUG, "foenItemInterval_Gen: Interval: [%s, %s]\n", lostIntervalRecord.sasnzEndDate, lostIntervalRecord.sasnzStartDate);
  fovdFormatDate(lostIntervalRecord.sasnzEndDate, YY_MM_DD);
  fovdFormatDate(lostIntervalRecord.sasnzStartDate, YY_MM_DD);

  fovdGen("SimIntervalItem", 
          lostIntervalRecord.sasnzState, lostIntervalRecord.sasnzEndDate, lostIntervalRecord.sasnzStartDate, 
          EOL);
  
  return TRUE;
}

/**************************************************************************************************
 *                                                                                          
 * METHOD : New
 *                                                                                          
 * DESCRIPTION : 
 *                                                                                          
 **************************************************************************************************
 */

tostItemInterval *fpstItemInterval_New(struct s_group_22 *ppstG22, struct s_group_22 *ppstPrevG22)
{
  tostItemInterval *lpstItem;
  struct s_imd_seg *lpstImd;
  struct s_dtm_seg *lpstDtm;
  struct s_moa_seg *lpstMoa;
  double loflMoa;
  int n;

  fovdPushFunctionName("foenItemInterval_New");

  lpstItem = (tostItemInterval *)calloc(1, sizeof(tostItemInterval));
  if (lpstItem == NULL)
    {
      fovdPopFunctionName();
      return NULL;
    }
  
  /*
   * Type of payment for prev. block on level 04 used in printing block
   */

  lpstItem->sochAccessSubtype = ppstPrevG22->pia->v_7140[2];
  
  /*
   * MOA+125
   */

  fovdPrintLog (LOG_DEBUG, "fpstItemInterval_New: Loading MOA\n");
  lpstMoa = fpstFindPaymentSegment(ppstPrevG22->g_23, "125", "9");
  if (lpstMoa == NULL)
    {
      fovdPopFunctionName();
      return NULL;
    }
  
  sscanf(lpstMoa->v_5004, "%lf", &loflMoa);  
  if ((int)loflMoa > 0)
    {
      lpstItem->sochSign = '+';
    }
  else if ((int)loflMoa < 0)
    {
      lpstItem->sochSign = '-';
    }
  else
    {
      lpstItem->sochSign = '0';
    }

  /*
   * Fill item
   */

  lpstImd = ppstG22->imd;
  if (EQ(lpstImd->v_7009, "STATE"))
    {
      fovdPrintLog (LOG_DEBUG, "fpstItemInterval_New: Found block G22 with type : STATE\n");
      lpstItem->sochState = lpstImd->v_7008a[0];
    }
  else
    {
      sprintf (szTemp, "fpstItemInterval_New: Can't find STATE blocks\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName();
      return NULL;
    }
  
          
  n = 0;
  lpstDtm = ppstG22->dtm;
  while (lpstDtm)
    {
      if (EQ(lpstDtm->v_2005, "901"))
        {          
          strcpy(lpstItem->sasnzStartDate, lpstDtm->v_2380);
          fovdPrintLog (LOG_DEBUG, "fpstItemInterval_New: Found block DTM with type: 901, %s\n", lpstItem->sasnzStartDate);
          n++;
        }
      
      if (EQ(lpstDtm->v_2005, "902"))
        {
          strcpy(lpstItem->sasnzEndDate, lpstDtm->v_2380);
          fovdPrintLog (LOG_DEBUG, "fpstItemInterval_New: Found block DTM with type: 902, %s\n", lpstItem->sasnzEndDate);
          n++;
        }
      
      lpstDtm = lpstDtm->dtm_next;
    }

  fovdPrintLog (LOG_DEBUG, "fpstItemInterval_New: Found number of dates : %d\n", n);

  if (n != 2)
    {
      sprintf (szTemp, "fpstItemInterval_New: Number of dates found: %d\n", n);
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName();
      return NULL;
    }          
  
  fovdPopFunctionName();
  return lpstItem;
}


/**************************************************************************************************
 *                                                                                          
 * METHOD : Delete
 *                                                                                          
 * DESCRIPTION : 
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenItemInterval_Delete(tostItemInterval *ppstItem)
{
  fovdPushFunctionName("foenItemInterval_Delete");  

  if (ppstItem != NULL)
    {
      free(ppstItem);
    }

  fovdPopFunctionName();
  return TRUE;
}

/**************************************************************************************************
 *                                                                                          
 * METHOD : Empty
 *                                                                                          
 * DESCRIPTION : 
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenItemInterval_Empty(tostItemInterval *ppstItem)
{
  if (ppstItem->sochAccessSubtype == 'A')
    {
      return TRUE;
    }

  if (ppstItem->sochAccessSubtype == 'C' && ppstItem->sochState == 'a')
    {
      return TRUE;
    }

  if (ppstItem->sochAccessSubtype == 'P' && (ppstItem->sochState == 's' || ppstItem->sochState == 'd'))
    {
      return TRUE;
    }
  
  /*
 
  if (EQ(ppstItem->sasnzStartDate, ppstItem->sasnzEndDate) && (ppstItem->sochState != 'a'))
    {
      return TRUE;
    }
 
  */

  return FALSE;
}

/**************************************************************************************************
 *                                                                                          
 * METHOD : Merge
 *                                                                                          
 * DESCRIPTION : 
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenItemInterval_Merge(tostItemInterval *ppstItemA, tostItemInterval *ppstItemB)
{
  return FALSE;
}

/**************************************************************************************************
 *                                                                                          
 * METHOD : Merge
 *                                                                                          
 * DESCRIPTION : 
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenItemInterval_Match(tostItemInterval *ppstItemA, tostItemInterval *ppstItemB)
{
  return FALSE;
}
