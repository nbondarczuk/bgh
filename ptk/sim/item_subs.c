#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "parser.h"
#include "types.h"
#include "gen.h"
#include "shdes2des.h"
#include "item_list.h"
#include "money.h"
#include "timm.h"

/*
  ================================================================================================
  =                                                                                              =
  =                                                                                              =
  = DATATYPE: ItemSubs                                                                         =
  =                                                                                              =
  =                                                                                              = 
  ================================================================================================
*/

/**************************************************************************************************
 *                                                                                          
 * METHOD : MakeRecord
 *                                                                                          
 * DESCRIPTION : 
 *                                                                                          
 **************************************************************************************************
 */

/*
typedef struct tostSubsRecord 
{
  char sasnzTariffModel[MAX_BUFFER];
  char sasnzServicePackage[MAX_BUFFER];  
  char sasnzService[MAX_BUFFER];
  char sasnzQuantity[MAX_BUFFER];
  char sasnzMonetaryAmount[MAX_BUFFER];
  char sasnzCurrency[MAX_BUFFER];
  char sasnzPrice[MAX_BUFFER];
} 
tostSubsRecord;

typedef struct tostItemSubs
{
  struct s_group_22 *spstG22;

  char sasnzTMShdes[SHDES_SIZE];
  long soilTMVersion;
  char sasnzSPShdes[SHDES_SIZE];
  char sasnzSNShdes[SHDES_SIZE];
  char sasnzSNDes[DES_SIZE];
  int  soiQuantity;
  double soflMoa;
  char sasnzCurrency[4];
  double soflPrice;

} tostItemSubs;
*/

toenBool foenItemSubs_FillRecord(tostItemSubs *ppstItem, tostSubsRecord *ppstRecord) 
{
  char *lpsnzDes;

  memset((void *)ppstRecord, '\0', sizeof(tostSubsRecord));

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

  /*
  strncpy(ppstRecord->sasnzService, ppstItem->sasnzSNShdes, MAX_BUFFER);  
  */

  strncpy(ppstRecord->sasnzService, ppstItem->sasnzSNDes, MAX_BUFFER);  

  fovdPrintLog (LOG_DEBUG, "SN: %s\n", ppstRecord->sasnzService);


  sprintf(ppstRecord->sasnzMonetaryAmount, "%.2lf", ppstItem->soflMoa);  
  strncpy(ppstRecord->sasnzCurrency, ppstItem->sasnzCurrency, MAX_BUFFER);
  sprintf(ppstRecord->sasnzQuantity, "%d", ppstItem->soiQuantity);
  sprintf(ppstRecord->sasnzPrice, "%.2lf", ppstItem->soflPrice);

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

tostItemSubs *fpstItemSubs_New(struct s_group_22 *ppstG22)
{
  tostItemSubs *lpstItem;
  char lasnzArticleNo[FIELD_SIZE];
  char lasnzItemId[FIELD_SIZE];
  char lasnzField[FIELD_SIZE];
  int n, i, j, loiFieldNo;
  struct s_qty_seg *lpstQty;
  struct s_moa_seg *lpstMoa;
  struct s_pri_seg *lpstPri;
  struct s_imd_seg *lpstImd;  

  lpstItem = (tostItemSubs *)calloc(1, sizeof(tostItemSubs));
  if (lpstItem == NULL)
    {
      return NULL;
    }

  lpstItem->spstG22 = ppstG22;

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

  fovdPrintLog (LOG_DEBUG, "Loading LIN\n");
  strncpy(lasnzArticleNo, ppstG22->lin->v_7140, DES_SIZE);
  n = strlen(lasnzArticleNo);
  for (i = 0, j = 0, loiFieldNo = 0; i <= n; i++)
    {      
      if (lasnzArticleNo[i] == '.' || lasnzArticleNo[i] == '\0')
        {
          lasnzField[j] = '\0';
          fovdPrintLog (LOG_DEBUG, "Field[%d]: %s\n", loiFieldNo, lasnzField);
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
   * MOA+125
   */

  fovdPrintLog (LOG_DEBUG, "Loading MOA\n");
  lpstMoa = fpstFindPaymentSegment(ppstG22->g_23, "125", "9");
  if (lpstMoa == NULL)
    {
      return NULL;
    }

  sscanf(lpstMoa->v_5004, "%lf", &(lpstItem->soflMoa));
  strcpy(lpstItem->sasnzCurrency, lpstMoa->v_6345);

  /*
   * QTY+107
   */

  fovdPrintLog (LOG_DEBUG, "Loading QTY\n");
  lpstQty = fpstFindQuantity(ppstG22->qty, "107", "UNI");
  if (lpstQty == NULL)
    {
      return NULL;
    }
  
  sscanf(lpstQty->v_6060, "%d", &(lpstItem->soiQuantity));


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

toenBool foenItemSubs_Delete(tostItemSubs *ppstItem)
{
  free(ppstItem);
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

toenBool foenItemSubs_Merge(tostItemSubs *ppstItemA, tostItemSubs *ppstItemB)
{
  return FALSE;
}

/**************************************************************************************************
 *                                                                                          
 * METHOD : Match
 *                                                                                          
 * DESCRIPTION : 
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenItemSubs_Match(tostItemSubs *ppstItemA, tostItemSubs *ppstItemB)
{
  return FALSE;
}


/**************************************************************************************************
 *                                                                                          
 * METHOD : Gen
 *                                                                                          
 * DESCRIPTION : 
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenItemSubs_Gen(tostItemSubs *ppstItem)
{
  tostSubsRecord lostSubsRecord;
  toenBool loenStatus;

  loenStatus = foenItemSubs_FillRecord(ppstItem, &lostSubsRecord);
  if (loenStatus == FALSE)
    {
      fovdPopFunctionName();
      return FALSE;
    }
  
  fovdFormatMoney(lostSubsRecord.sasnzMonetaryAmount);  
  fovdGen( "SimSummaryItemService", 
           lostSubsRecord.sasnzTariffModel, 
           lostSubsRecord.sasnzServicePackage,
           lostSubsRecord.sasnzService,
           lostSubsRecord.sasnzMonetaryAmount, 
           lostSubsRecord.sasnzCurrency,
           EOL);
  
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

toenBool foenItemSubs_Empty(tostItemSubs *ppstItem)
{
  if (foenMoney_Eq(ppstItem->soflMoa, 0.0) == TRUE)
    {
      return TRUE;
    }

  return FALSE;
}


