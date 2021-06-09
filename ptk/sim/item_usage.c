#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "types.h"
#include "gen.h"
#include "shdes2des.h"
#include "item_list.h"
#include "money.h"
#include "call_category.h"
#include "timm.h"

static char szTemp[128];
static int doiCalls[2] = {0, 0};
static double doflMoa[2] = {0.0, 0.0};

#define HPLMN_TYPE_INDEX 0
#define VPLMN_TYPE_INDEX 1

/*
  ================================================================================================
  =                                                                                              =
  =                                                                                              =
  = DATATYPE: ItemUsage                                                                          =
  =                                                                                              =
  =                                                                                              = 
  ================================================================================================
*/

int foiItemUsage_Compare(tostItemUsage *ppstItemA, tostItemUsage *ppstItemB)
{
  toenPLMNType loenTypeA, loenTypeB;
  int rc;

  fovdPushFunctionName("foiItemUsage_Compare");

  if (ppstItemA->sochType == 'R' || ppstItemA->sochType == 'r' || ppstItemA->sochType == 'm')
    {
      loenTypeA = VPLMN_TYPE;
    }
  else
    {
      loenTypeA = HPLMN_TYPE;
    }
  
  if (ppstItemB->sochType == 'R' || ppstItemB->sochType == 'r' || ppstItemB->sochType == 'm')
    {
      loenTypeB = VPLMN_TYPE;
    }
  else
    {
      loenTypeB = HPLMN_TYPE;
    }

  if (loenTypeA == VPLMN_TYPE && loenTypeB == HPLMN_TYPE)
    {
      rc = 1;
    }
  else if (loenTypeA == HPLMN_TYPE && loenTypeB == VPLMN_TYPE)
    {
      rc = -1;
    }
  else
    {
      rc = 0;
    }

  fovdPopFunctionName();  
  return rc;
}

/**************************************************************************************************
 *                                                                                          
 * METHOD : MakeRecord
 *                                                                                          
 * DESCRIPTION : 
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenItemUsage_FillRecord(tostItemUsage *ppstItem, tostUsageRecord *ppstRecord) 
{
  char *lpsnzDes;
  char *ptr;

  fovdPushFunctionName("foenItemUsage_FillRecord");

  memset((void *)ppstRecord, '\0', sizeof(tostUsageRecord));

  /*
   * Tariff Model
   */

#ifdef _LONG_DES_
  lpsnzDes = fpsnzMapTMShdes2Des(ppstItem->sasnzTMShdes);
  if (lpsnzDes != NULL)
    {
      strncpy(ppstRecord->sasnzTariffModel, lpsnzDes, MAX_BUFFER);
    }
#else
  strncpy(ppstRecord->sasnzTariffModel, ppstItem->sasnzTMShdes, MAX_BUFFER);
#endif

  fovdPrintLog (LOG_DEBUG, "foenItemUsage_FillRecord: TM: %s\n", ppstRecord->sasnzTariffModel);

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

  fovdPrintLog (LOG_DEBUG, "foenItemUsage_FillRecord: SP: %s\n", ppstRecord->sasnzServicePackage);

  /*
   * Service
   */

  /*
#ifdef _LONG_DES_
  */

  lpsnzDes = fpsnzMapSNShdes2Des(ppstItem->sasnzSNShdes);
  if (lpsnzDes != NULL)
    {
      strncpy(ppstRecord->sasnzService, lpsnzDes, MAX_BUFFER);  
    }

  /*
#else  
  strncpy(ppstRecord->sasnzService, ppstItem->sasnzSNShdes, MAX_BUFFER);  
#endif
  */
  
  fovdPrintLog (LOG_DEBUG, "foenItemUsage_FillRecord: SN: %s\n", ppstRecord->sasnzService);

  /*
   * Rating Type and TAP records
   */

  fovdPrintLog (LOG_DEBUG, "foenItemUsage_FillRecord: Rating: %c\n", ppstItem->sochRatingModel);
  fovdPrintLog (LOG_DEBUG, "foenItemUsage_FillRecord: VPLMN: %s\n", ppstItem->sasnzVPLMN);
  if (ppstItem->sochRatingModel == 'm' 
      || ppstItem->sochRatingModel == 'R' 
      || ppstItem->sochRatingModel == 'r')
    {
      ptr = fpsnzMapVPLMNShdes2Country(ppstItem->sasnzVPLMN);
      if (ptr == NULL)
        {
          ptr = ppstItem->sasnzVPLMN;
        }
      else
        {
          strncpy(ppstRecord->sasnzVPLMN, ptr, MAX_BUFFER); 
        }
    }

  switch(ppstItem->sochRatingModel)
    {
    case 'm':
      strcpy(ppstRecord->sasnzUsageType, "INROAMING"); 
      break;

    case 'R':
      strcpy(ppstRecord->sasnzUsageType, "SURCHARGE"); 
      break;

    case 'r':
      strcpy(ppstRecord->sasnzUsageType, "OUTROAMING"); 
      break;
      
    default:
      strcpy(ppstRecord->sasnzUsageType, "NORMAL"); 
      fovdPrintLog (LOG_DEBUG, "foenItemUsage_FillRecord: ZN\n");
      lpsnzDes = fpsnzMapTZShdes2Des(ppstItem->sasnzZNShdes);
      if (lpsnzDes != NULL)
        {
          strncpy(ppstRecord->sasnzTariffZone, lpsnzDes, MAX_BUFFER);
        }
      
      fovdPrintLog (LOG_DEBUG, "foenItemUsage_FillRecord: TT\n");
      lpsnzDes = fpsnzMapTTShdes2Des(ppstItem->sasnzTTShdes);
      if (lpsnzDes != NULL)
        {
          strncpy(ppstRecord->sasnzTariffTime, lpsnzDes, MAX_BUFFER);      
        }
    }

  /*
   * Type of connection
   */
  
  fovdPrintLog (LOG_DEBUG, "foenItemUsage_FillRecord: Type: %c\n", ppstItem->sochType);
  ppstRecord->sasnzConnectionType[0] = ppstItem->sochType;
  ppstRecord->sasnzConnectionType[1] = '\0';

  /*
   * Monetary amounts
   */
  
  fovdPrintLog (LOG_DEBUG, "foenItemUsage_FillRecord: Moa\n");
  sprintf(ppstRecord->sasnzMonetaryAmount, "%.2lf", ppstItem->soflMoa);  
  strncpy(ppstRecord->sasnzCurrency, ppstItem->sasnzCurrency, MAX_BUFFER);
  sprintf(ppstRecord->sasnzQuantity, "%d", ppstItem->soiCalls);  

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

tostItemUsage *fpstItemUsage_New(struct s_group_22 *ppstG22)
{
  tostItemUsage *lpstItem;
  char lasnzArticleNo[FIELD_SIZE];
  char lasnzItemId[FIELD_SIZE];
  char lasnzField[FIELD_SIZE];
  int n, i, j, loiFieldNo;
  struct s_qty_seg *lpstQty;
  struct s_moa_seg *lpstMoa;
  struct s_pri_seg *lpstPri;

  fovdPushFunctionName("fpstItemUsage_New");
  
  lpstItem = (tostItemUsage *)calloc(1, sizeof(tostItemUsage));
  if (lpstItem == NULL)
    {
      fovdPopFunctionName();
      return NULL;
    }

  /*
   * LIN
   */

  fovdPrintLog (LOG_DEBUG, "fpstItemUsage_New: Loading LIN segment\n");
  strncpy(lasnzArticleNo, ppstG22->lin->v_7140, 75);
  n = strlen(lasnzArticleNo);
  for (i = 0, j = 0, loiFieldNo = 0; i <= n; i++)
    {      
      if (lasnzArticleNo[i] == '.' || lasnzArticleNo[i] == '\0')
        {
          lasnzField[j] = '\0';
          fovdPrintLog (LOG_DEBUG, "fpstItemUsage_New: Field[%d]: %s\n", loiFieldNo, lasnzField);
          j = 0;
          switch (loiFieldNo++)
            {
            case 0: strncpy(lpstItem->sasnzTMShdes, lasnzField, SHDES_SIZE); break; 
            case 1: lpstItem->soilTMVersion = atoi(lasnzField); break;
            case 2: strncpy(lpstItem->sasnzSPShdes, lasnzField, SHDES_SIZE); break; 
            case 3: strncpy(lpstItem->sasnzSNShdes, lasnzField, SHDES_SIZE); break; 
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

  fovdPrintLog (LOG_DEBUG, "fpstItemUsage_New: Loading PIA segment\n");
  strncpy(lasnzItemId, ppstG22->pia->v_7140, 75);
  n = strlen(lasnzItemId);
  for (i = 0, loiFieldNo = 0; i <= n; i++)
    {      
      if (lasnzItemId[i] == '.' || lasnzItemId[i] == '\0')
        {
          lasnzField[j] = '\0';
          fovdPrintLog (LOG_DEBUG, "fpstItemUsage_New: Field[%d]: %s\n", loiFieldNo, lasnzField);
          j = 0;
          switch (loiFieldNo++)
            {
            case 0: lpstItem->sochType = lasnzField[0]; break;
            case 1: lpstItem->sochRatingModel = lasnzField[0]; break;
            case 2:
              if (lpstItem->sochRatingModel == 'R' 
		  || lpstItem->sochRatingModel == 'r' 
		  || lpstItem->sochRatingModel == 'm')
                {
                  strncpy(lpstItem->sasnzVPLMN, lasnzField, SHDES_SIZE);
                }
              else
                {
                  lpstItem->sasnzVPLMN[0] = '\0';
                }
              break;
            case 3: lpstItem->soilEGLPVersion = atol(lasnzField); break;
            case 4: strncpy(lpstItem->sasnzTTShdes, lasnzField, SHDES_SIZE); break;
            case 5: strncpy(lpstItem->sasnzZNShdes, lasnzField, SHDES_SIZE); break;
            }
        }
      else
        {
          lasnzField[j++] = lasnzItemId[i];
        }
    }

  /*
   * MOA+125
   */

  fovdPrintLog (LOG_DEBUG, "fpstItemUsage_New: Loading MOA+125 segment\n");
  lpstMoa = fpstFindPaymentSegment(ppstG22->g_23, "125", "9");
  if (lpstMoa == NULL)
    {
      fovdPopFunctionName();
      return NULL;
    }

  sscanf(lpstMoa->v_5004, "%lf", &(lpstItem->soflMoa));
  strncpy(lpstItem->sasnzCurrency, lpstMoa->v_6345, 3);
  fovdPrintLog (LOG_DEBUG, "fpstItemUsage_New: MOA+125: %lf %s\n", 
		lpstItem->soflMoa, 
		lpstItem->sasnzCurrency);

  /*
   * QTY+107
   */
  
  fovdPrintLog (LOG_DEBUG, "fpstItemUsage_New: Loading QTY+107 segment\n");
  lpstQty = fpstFindQuantity(ppstG22->qty, "107", "UNI");
  if (lpstQty == NULL)
    {
      fovdPrintLog (LOG_DEBUG, "fpstItemUsage_New: Loading QTY+108 segment\n");
      lpstQty = fpstFindQuantity(ppstG22->qty, "108", "UNI");
      if (lpstQty == NULL)
        {
          fovdPopFunctionName();
          return NULL;
        }
    }
  
  sscanf(lpstQty->v_6060, "%d", &(lpstItem->soiCalls));
  fovdPrintLog (LOG_DEBUG, "fpstItemUsage_New: Value QTY: %d\n", lpstItem->soiCalls);

  /*
   * G22 for future processing
   */

  lpstItem->spstG22 = ppstG22;

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

toenBool foenItemUsage_Delete(tostItemUsage *ppstItem)
{
  fovdPushFunctionName("foenItemUsage_Delete");

  free(ppstItem);

  fovdPopFunctionName();
  return TRUE;
}

/**************************************************************************************************
 *                                                                                          
 * METHOD : Merge
 *                                                                                          
 * DESCRIPTION : 
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenItemUsage_Merge(tostItemUsage *ppstItemA, tostItemUsage *ppstItemB)
{
  fovdPushFunctionName("foenItemUsage_Merge");

  ppstItemA->soiCalls += ppstItemB->soiCalls;
  ppstItemA->soflMoa += ppstItemB->soflMoa;

  fovdPopFunctionName();
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

toenBool foenItemUsage_Match(tostItemUsage *ppstItemA, tostItemUsage *ppstItemB)
{
  fovdPushFunctionName("foenItemUsage_Match");

  if (NOT(EQ(ppstItemA->sasnzTMShdes, ppstItemB->sasnzTMShdes)))
    {
      fovdPopFunctionName();
      return FALSE;
    }

  if (NOT(EQ(ppstItemA->sasnzSPShdes, ppstItemB->sasnzSPShdes)))
    {
      fovdPopFunctionName();
      return FALSE;
    }

  if (NOT(EQ(ppstItemA->sasnzSNShdes, ppstItemB->sasnzSNShdes)))
    {
      fovdPopFunctionName();
      return FALSE;
    }

  if (ppstItemA->sochType != ppstItemB->sochType)
    {
      fovdPopFunctionName();
      return FALSE;
    }

  if (ppstItemA->sochRatingModel != ppstItemB->sochRatingModel)
    {
      fovdPopFunctionName();
      return FALSE;
    }

  if (NOT(EQ(ppstItemA->sasnzTTShdes, ppstItemB->sasnzTTShdes)))
    {
      fovdPopFunctionName();
      return FALSE;
    }

  if (NOT(EQ(ppstItemA->sasnzZNShdes, ppstItemB->sasnzZNShdes)))
    {
      fovdPopFunctionName();
      return FALSE;
    }
  
  fovdPopFunctionName();  
  return TRUE;
}



/**************************************************************************************************
 *                                                                                          
 * METHOD : SummaryGen
 *                                                                                          
 * DESCRIPTION : 
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenItemUsage_SummaryGen(tostItemUsage *ppstItem, toenPLMNType poenType)
{
  int i;
  char *lpsnzTag;
  static char lasnzCalls[MAX_BUFFER];
  static char lasnzMonetaryAmount[MAX_BUFFER];

  fovdPushFunctionName("foenItemUsage_SummaryGen");
  
  /*
   * Find index of value and tag for printing summary
   */

  if (poenType == HPLMN_TYPE)
    {
      i = HPLMN_TYPE_INDEX;
      lpsnzTag = "SimSummaryCountryItemUsage";
    }
  else
    {
      i = VPLMN_TYPE_INDEX;
      lpsnzTag = "SimSummaryRoamingItemUsage";
    }
  
  if (doflMoa[i] == 0.0 || doiCalls[i] == 0)
    {
      fovdPopFunctionName();
      return TRUE;      
    }

  /*
   * Format values
   */
  
  sprintf(lasnzCalls, "%d", doiCalls[i]);

  sprintf(lasnzMonetaryAmount, "%.2lf", doflMoa[i]);  
  fovdFormatMoney(lasnzMonetaryAmount);

  fovdGen(lpsnzTag, lasnzCalls, lasnzMonetaryAmount, "PLN", EOL);

  /*
   * Flush item
   */

  doiCalls[i] = 0;
  doflMoa[i] = 0.0;

  fovdPopFunctionName();
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

toenBool foenItemUsage_Gen(tostItemUsage *ppstItem, toenPLMNType poenType)
{
  tostUsageRecord lostUsageRecord;
  toenBool loenStatus;
  toenPLMNType loenType;
  int i;

  fovdPushFunctionName("foenItemUsage_Gen");

  fovdPrintLog (LOG_DEBUG, "foenItemUsage_Gen: Printing item\n");

  /*
  if (ppstItem->sochType == 'R' || ppstItem->sochType == 'r' || ppstItem->sochType == 'm')
  */

  if (ppstItem->sasnzVPLMN[0] != '\0')
    {
      loenType = VPLMN_TYPE;
    }
  else
    {
      loenType = HPLMN_TYPE;
    }

  if (loenType == poenType)
    {
      if (loenType == HPLMN_TYPE)
        {
          i = HPLMN_TYPE_INDEX;
        }
      else
        {
          i = VPLMN_TYPE_INDEX;
        }
      
      /*
       * Don't include ROAMING SURCHARGE and INTERCONNECT part of a call
       */

      if (ppstItem->sochRatingModel != 'R' && ppstItem->sochType != 'I')
        {
          doiCalls[i] += ppstItem->soiCalls;
        }

      doflMoa[i] += ppstItem->soflMoa;
      
      fovdPrintLog (LOG_DEBUG, "foenItemUsage_Gen: Filling record with item of type: %s\n", "Usage");
      loenStatus = foenItemUsage_FillRecord(ppstItem, &lostUsageRecord);
      if (loenStatus == FALSE)
        {
          fovdPopFunctionName();
          return FALSE;
        }
      
      fovdPrintLog (LOG_DEBUG, "foenItemUsage_Gen: Formating record\n");
      fovdFormatMoney(lostUsageRecord.sasnzMonetaryAmount);  
      
      fovdPrintLog (LOG_DEBUG, "foenItemUsage_Gen: Printing record\n");
      fovdGen("SimSummaryItemUsage", 
              lostUsageRecord.sasnzUsageType, 
              lostUsageRecord.sasnzConnectionType, 
              lostUsageRecord.sasnzVPLMN, 
              lostUsageRecord.sasnzTariffTime, 
              lostUsageRecord.sasnzTariffZone, 
              lostUsageRecord.sasnzTariffModel, 
              lostUsageRecord.sasnzServicePackage,
              lostUsageRecord.sasnzService,
              lostUsageRecord.sasnzQuantity,
              lostUsageRecord.sasnzMonetaryAmount, 
              lostUsageRecord.sasnzCurrency,                
              lostUsageRecord.sasnzTime,                
              lostUsageRecord.sasnzUnitPrice,
              lostUsageRecord.sasnzFullPrice,
              EOL);
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

toenBool foenItemUsage_Empty(tostItemUsage *ppstItem)
{
  fovdPushFunctionName("foenItemUsage_Empty");
  
  if (foenMoney_Eq(ppstItem->soflMoa, 0.0) == TRUE)
    {
      fovdPopFunctionName();
      return TRUE;
    }

  fovdPopFunctionName();
  return FALSE;
}























