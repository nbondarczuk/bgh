#include <string.h>
#include <time.h>
#include <stdlib.h>

#include "parser.h"
#include "types.h"
#include "gen.h"
#include "strutl.h"
#include "item_list.h"
#include "shdes2des.h"
#include "item_interval.h"
#include "date.h"
#include "money.h"
#include "timm.h"

extern stBGHGLOB stBgh;

/*
  ================================================================================================
  =                                                                                              =
  =                                                                                              =
  = DATATYPE: ItemAccess                                                                         =
  =                                                                                              =
  =                                                                                              = 
  ================================================================================================
*/

/**************************************************************************************************
 *                                                                                          
 * METHOD : FillRecord
 *                                                                                          
 * DESCRIPTION : 
 *                                                                                          
 **************************************************************************************************
 */

extern char *fpsnzFormatAccessDays(char, int);

toenBool foenItemAccess_FillRecord(tostItemAccess *ppstItem, tostAccessRecord *ppstRecord) 
{
  char *lpsnzDes;
  char *lpsnzStr;

  memset((void *)ppstRecord, '\0', sizeof(tostAccessRecord));

  /*
   * Tariff Model
   */

  lpsnzDes = fpsnzMapTMShdes2Des(ppstItem->sasnzTMShdes);
  if (lpsnzDes != NULL)
    {
      strncpy(ppstRecord->sasnzTariffModel, lpsnzDes, MAX_BUFFER);
    }

  fovdPrintLog (LOG_DEBUG, "TM: %s\n", ppstRecord->sasnzTariffModel);

  /*
   * Service Package
   */

#ifdef _LONG_DES_
  lpsnzDes = fpsnzMapSPShdes2Des(ppstItem->sasnzSPShdes);
  if (lpsnzDes != NULL)
    {
      strncpy(ppstRecord->sasnzServicePackage, lpsnzDes, MAX_BUFFER);
    }
#else
  strncpy(ppstRecord->sasnzServicePackage, ppstItem->sasnzSPShdes, MAX_BUFFER);
#endif
  
  fovdPrintLog (LOG_DEBUG, "SP: %s\n", ppstRecord->sasnzServicePackage);

  /*
   * Service
   */

  /*
  lpsnzDes = fpsnzMapSNShdes2Des(ppstItem->sasnzSNShdes);
  if (lpsnzDes != NULL)
    {
      strncpy(ppstRecord->sasnzService, lpsnzDes, MAX_BUFFER);  
    }
  */

  strncpy(ppstRecord->sasnzService, ppstItem->sasnzSNDes, MAX_BUFFER);  
  
  ppstRecord->sasnzAccessType[0] = ppstItem->sochChargeType;
  ppstRecord->sasnzAccessType[1] = '\0';
  ppstRecord->sasnzChargeType[0] = ppstItem->sochChargeSubtype;
  ppstRecord->sasnzChargeType[1] = '\0';

  lpsnzStr = fpsnzFormatAccessDays(ppstItem->sochChargeSubtype, ppstItem->soiDays);
  strncpy(ppstRecord->sasnzDays, lpsnzStr, MAX_BUFFER);

  strncpy(ppstRecord->sasnzCurrency, ppstItem->sasnzCurrency, MAX_BUFFER);
  sprintf(ppstRecord->sasnzMonetaryAmount, "%.2lf", ppstItem->soflMoa);  
  sprintf(ppstRecord->sasnzQuantity, "%d", ppstItem->soiQuantity);
  sprintf(ppstRecord->sasnzPrice, "%.2lf", ppstItem->soflPrice);

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

extern char gochItemInterval_State;
extern struct s_TimmInter *dpstSumSheet, *dpstRoamingSheet;

toenBool foenItemAccess_Gen(tostItemAccess *ppstItem)
{
  tostAccessRecord lostAccessRecord;
  toenBool loenStatus;
  int loiItemsNo;
  char lasnzItemsNo[16];
  static char lasnzStartDate[16];
  static char lasnzEndDate[16];
  static char lasnzDate[16];
  char *lpsnzStr;
  int loiDays;

  fovdPushFunctionName("foenItemAccess_Gen");

  loenStatus = foenItemAccess_FillRecord(ppstItem, &lostAccessRecord);
  if (loenStatus == FALSE)
    {
      fovdPopFunctionName();
      return FALSE;
    }

  if (ppstItem->sochChargeSubtype == 'A')
    {
      loenStatus = foenFetchInvicePeriodEnd(dpstSumSheet, lasnzDate, 16);
      if (loenStatus == FALSE)
        {
          fovdPopFunctionName();
          return FALSE;
        }

      fovdPrintLog (LOG_DEBUG, "foenItemAccess_Gen: Getting next day for: %s\n", lasnzDate);
      memset(lasnzStartDate, 0, 16);
      if ((loenStatus = foenDate_AddDay(lasnzDate, lasnzStartDate)) == FALSE)
        {
          fovdPopFunctionName();
          return FALSE;
        }
      
      fovdPrintLog (LOG_DEBUG, "foenItemAccess_Gen: Getting next month for: %s\n", lasnzDate);
      memset(lasnzEndDate, 0, 16);
      if ((loiDays = foiDate_AddMonth(lasnzDate, lasnzEndDate)) < 0)
        {
          fovdPrintLog (LOG_DEBUG, "foenItemAccess_Gen: Got number of days: %d\n", loiDays);
          fovdPopFunctionName();
          return FALSE;
        }

      lpsnzStr = fpsnzFormatAccessDays(ppstItem->sochChargeSubtype, loiDays);
      strncpy(lostAccessRecord.sasnzDays, lpsnzStr, MAX_BUFFER);           
    }

  fovdFormatMoney(lostAccessRecord.sasnzMonetaryAmount);    

  fovdPrintLog (LOG_DEBUG, "Printing list: Usage\n");
  fovdGen("SimSummaryItemAccess", 
          lostAccessRecord.sasnzAccessType, 
          lostAccessRecord.sasnzChargeType, 
          lostAccessRecord.sasnzTariffModel, 
          lostAccessRecord.sasnzServicePackage,
          lostAccessRecord.sasnzService,
          lostAccessRecord.sasnzMonetaryAmount, 
          lostAccessRecord.sasnzCurrency,
          lostAccessRecord.sasnzDays,
          EOL);      

  if (ppstItem->sochChargeSubtype == 'C' || ppstItem->sochChargeSubtype == 'P')
    {
      if (ppstItem->spstListInterval != NULL)
        {
          loiItemsNo = foiItemList_Count(ppstItem->spstListInterval);
          if (loiItemsNo > 0)
            {      
              sprintf(lasnzItemsNo, "%d", loiItemsNo);
              fovdGen("IntervalListStart", lasnzItemsNo, EOL);          
              loenStatus = foenItemList_Gen(ppstItem->spstListInterval);
              if (loenStatus == FALSE)
                {
                  fovdPopFunctionName();
                  return FALSE;
                }
          
              fovdGen("IntervalListEnd", EOL);
            }
        }
    }
  else /* Advance Payment */
    { 
      fovdPrintLog (LOG_DEBUG, "foenItemAccess_Gen: Interval: [%s, %s]\n", lasnzStartDate, lasnzEndDate);
      fovdFormatDate(lasnzEndDate, YYYY_MM_DD);
      fovdFormatDate(lasnzStartDate, YY_MM_DD);

      fovdGen("IntervalListStart", "1", EOL);          
      fovdGen("SimIntervalItem", "a", lasnzEndDate, lasnzStartDate, EOL);
      fovdGen("IntervalListEnd", EOL);
    }

  fovdPopFunctionName();
  return TRUE;
}

/**************************************************************************************************
 *                                                                                          
 * METHOD : New
 *                                                                                          
 * DESCRIPTION : Copy EDI G22 group info to ItemAccess structure
 *                                                                                          
 **************************************************************************************************
 */

tostItemAccess *fpstItemAccess_New(struct s_group_22 *ppstG22)
{
  tostItemAccess *lpstItem;
  char lasnzArticleNo[75];
  char lasnzItemId[75];
  char lasnzField[FIELD_SIZE];
  int n, i, j, loiFieldNo;
  struct s_qty_seg *lpstQty;
  struct s_moa_seg *lpstMoa;
  struct s_pri_seg *lpstPri;
  struct s_imd_seg *lpstImd;

  lpstItem = (tostItemAccess *)calloc(1, sizeof(tostItemAccess));
  if (lpstItem == NULL)
    {
      return NULL;
    }

  fovdPrintLog (LOG_DEBUG, "fpstItemAccess_New: New G22 item type: Access\n");

  /*
   * IMD
   */
  
  lpstImd = fpstFindItemDescription(ppstG22->imd, "SN");
  if (lpstImd != NULL)
    {
      strncpy(lpstItem->sasnzSNDes, lpstImd->v_7008a, DES_SIZE);
    }
  else
    {
      memset(lpstItem->sasnzSNDes, '\0', DES_SIZE);
    }

  /*
   * LIN
   */

  fovdPrintLog (LOG_DEBUG, "fpstItemAccess_New: Loading LIN\n");
  strncpy(lasnzArticleNo, ppstG22->lin->v_7140, 75);
  n = strlen(lasnzArticleNo);
  for (i = 0, j = 0, loiFieldNo = 0; i <= n; i++)
    {      
      if (lasnzArticleNo[i] == '.' || lasnzArticleNo[i] == '\0')
        {
          lasnzField[j] = '\0';
          fovdPrintLog (LOG_DEBUG, "fpstItemAccess_New: Field[%d]: %s\n", loiFieldNo, lasnzField);
          j = 0;
          switch (loiFieldNo++)
            {
            case 0: strncpy(lpstItem->sasnzTMShdes, lasnzField, 6); break; 
            case 1: lpstItem->soilTMVersion = atoi(lasnzField); break;
            case 2: strncpy(lpstItem->sasnzSPShdes, lasnzField, 6); break; 
            case 3: strncpy(lpstItem->sasnzSNShdes, lasnzField, 6); break; 
            }
        }
      else
        {
          lasnzField[j++] = lasnzArticleNo[i];
        }
    }

  /*
   * PIA
   */

  fovdPrintLog (LOG_DEBUG, "fpstItemAccess_New: Loading PIA\n");
  strncpy(lasnzItemId, ppstG22->pia->v_7140, 75);
  lpstItem->sochChargeType = lasnzItemId[0];     
  lpstItem->sochChargeSubtype = lasnzItemId[2];
  if (strlen(lasnzItemId) > 3)
    {
      lpstItem->sochOverwriteInd = lasnzItemId[4];
    }
  else
    {
      lpstItem->sochOverwriteInd = '\0';
    }

  /*
   * QTY+107
   */

  fovdPrintLog (LOG_DEBUG, "fpstItemAccess_New: Loading QTY\n");
  lpstQty = fpstFindQuantity(ppstG22->qty, "107", "UNI");
  if (lpstQty == NULL)
    {
      return NULL;
    }
  
  sscanf(lpstQty->v_6060, "%d", &(lpstItem->soiQuantity));

  /*
   * QTY+109
   */

  fovdPrintLog (LOG_DEBUG, "fpstItemAccess_New: Loading QTY\n");
  lpstQty = fpstFindQuantity(ppstG22->qty, "109", "DAY");
  if (lpstQty != NULL)
    {
      sscanf(lpstQty->v_6060, "%d", &(lpstItem->soiDays));
    }
  

  /*
   * MOA+125
   */

  fovdPrintLog (LOG_DEBUG, "fpstItemAccess_New: Loading MOA\n");
  lpstMoa = fpstFindPaymentSegment(ppstG22->g_23, "125", "9");
  if (lpstMoa == NULL)
    {
      return NULL;
    }

  sscanf(lpstMoa->v_5004, "%lf", &(lpstItem->soflMoa));
  strncpy(lpstItem->sasnzCurrency, lpstMoa->v_6345, 4);

  /*
   * PRI
   */
  /*
  lpstPri = fpstFindPriceSegment(ppstG22->g_25);
  if (lpstPri == NULL)
    {
      return NULL;
    }

  sscanf(lpstPri->v_5118, "%lf", &(lpstItem->soflPrice));
  */

  /*
   * Create the list of access intervals
   */

  fovdPrintLog (LOG_DEBUG, "fpstItemAccess_New: Creating new interval list\n");  
  if ((lpstItem->spstListInterval = fpstItemList_New(ppstG22, INTERVAL_TYPE, ppstG22)) == NULL)
    {
      return NULL;
    }
  
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

toenBool foenItemAccess_Delete(tostItemAccess *ppstItem)
{
  toenBool loenStatus = TRUE;

  fovdPushFunctionName("foenItemAccess_Delete");

  if (ppstItem != NULL)
    {
      if (ppstItem->spstListInterval != NULL)
        {
          loenStatus = foenItemList_Delete(ppstItem->spstListInterval);
        }
      
      free(ppstItem);
    }

  fovdPopFunctionName();
  return loenStatus;
}


/**************************************************************************************************
 *                                                                                          
 * METHOD : Merge
 *                                                                                          
 * DESCRIPTION : 
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenItemAccess_Merge(tostItemAccess *ppstItemA, tostItemAccess *ppstItemB)
{
  ppstItemA->soiDays += ppstItemB->soiDays;
  ppstItemA->soflMoa += ppstItemB->soflMoa;

  return TRUE;
}

/**************************************************************************************************
 *                                                                                          
 * METHOD : Match
 *                                                                                          
 * DESCRIPTION : 
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenItemAccess_Match(tostItemAccess *ppstItemA, tostItemAccess *ppstItemB)
{
  if (ppstItemA->sochChargeType != ppstItemB->sochChargeType)
    {
      return FALSE;
    }

  if (ppstItemA->sochChargeSubtype != ppstItemB->sochChargeSubtype)
    {
      return FALSE;
    }

  if (ppstItemA->sochOverwriteInd != ppstItemB->sochOverwriteInd)
    {
      return FALSE;
    }

  if (NOT(EQ(ppstItemA->sasnzTMShdes, ppstItemB->sasnzTMShdes)))
    {
      return FALSE;
    }
  
  if (NOT(EQ(ppstItemA->sasnzSPShdes, ppstItemB->sasnzSPShdes)))
    {
      return FALSE;
    }

  if (NOT(EQ(ppstItemA->sasnzSNShdes, ppstItemB->sasnzSNShdes)))
    {
      return FALSE;
    }

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

toenBool foenItemAccess_Empty(tostItemAccess *ppstItem)
{
  return FALSE;
}
