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
  = DATATYPE: ItemOCC                                                                         =
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
typedef struct tostOCCRecord 
{
  char sasnzTariffModel[MAX_BUFFER];
  char sasnzServicePackage[MAX_BUFFER];  
  char sasnzService[MAX_BUFFER];
  char sasnzQuantity[MAX_BUFFER];
  char sasnzPrice[MAX_BUFFER];
  char sasnzMonetaryAmount[MAX_BUFFER];
  char sasnzCurrency[MAX_BUFFER];
} 
tostOCCRecord;

typedef struct tostItemOCC
{
  struct s_group_22 *spstG22;
  
  char sasnzTMShdes[SHDES_SIZE];
  long soilTMVersion;
  char sasnzSPShdes[SHDES_SIZE];
  char sasnzSNShdes[SHDES_SIZE];
  char sasnzSNDes[DES_SIZE];
  char sasnzFE[DES_SIZE];  
  int  soiQuantity;
  double soflMoa;
  char sasnzCurrency[4];
  double soflPrice;

} tostItemOCC;
*/

toenBool foenItemOCC_FillRecord(tostItemOCC *ppstItem, tostOCCRecord *ppstRecord) 
{
  char *lpsnzDes;

  fovdPrintLog (LOG_DEBUG, "foenItemOCC_FillRecord: Filling\n");
  memset(ppstRecord, 0, sizeof(tostOCCRecord));

  if (ppstItem->sasnzFE[0] == '\0')
    {

      fovdPrintLog (LOG_DEBUG, "foenItemOCC_FillRecord: Normal OCC\n");

      /*
       * Service Name
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
          strncpy(ppstRecord->sasnzServicePackage, lpsnzDes , MAX_BUFFER);
        }
#else
      strncpy(ppstRecord->sasnzServicePackage, ppstItem->sasnzSPShdes , MAX_BUFFER);
#endif
      fovdPrintLog (LOG_DEBUG, "SP: %s\n", ppstRecord->sasnzServicePackage);
      
      /*
       * Service Name
       */

#ifdef _LONG_DES_
      lpsnzDes = fpsnzMapSNShdes2Des(ppstItem->sasnzSNShdes);
      if (lpsnzDes != NULL)
        {
          strncpy(ppstRecord->sasnzService, lpsnzDes, MAX_BUFFER);  
        }
#else
      strncpy(ppstRecord->sasnzService, ppstItem->sasnzSNShdes, MAX_BUFFER);  
#endif
      fovdPrintLog (LOG_DEBUG, "SN: %s\n", ppstRecord->sasnzService);
    }
  else
    {
      fovdPrintLog (LOG_DEBUG, "foenItemOCC_FillRecord: Special OCC\n");
      strncpy(ppstRecord->sasnzService, ppstItem->sasnzFE, MAX_BUFFER);  
      fovdPrintLog (LOG_DEBUG, "foenItemOCC_FillRecord: FE: %s\n", ppstRecord->sasnzService);
    }

  sprintf(ppstRecord->sasnzMonetaryAmount, "%.2lf", ppstItem->soflMoa);  
  strncpy(ppstRecord->sasnzCurrency, ppstItem->sasnzCurrency, MAX_BUFFER);
  sprintf(ppstRecord->sasnzQuantity, "%d", ppstItem->soiQuantity);
  /*
  sprintf(ppstRecord->sasnzPrice, "%.2lf", ppstItem->soflPrice);
  */
  
  fovdPrintLog (LOG_DEBUG, "foenItemOCC_FillRecord: OCC: %.2lf %d\n", ppstItem->soflMoa, ppstItem->soiQuantity);
  
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


tostItemOCC *fpstItemOCC_New(struct s_group_22 *ppstG22)
{
  tostItemOCC *lpstItem;
  char lasnzArticleNo[FIELD_SIZE];
  char lasnzItemId[FIELD_SIZE];
  char lasnzField[FIELD_SIZE];
  int n, i, j, loiFieldNo;
  struct s_qty_seg *lpstQty;
  struct s_moa_seg *lpstMoa;
  struct s_pri_seg *lpstPri;
  struct s_imd_seg *lpstImd;

  lpstItem = (tostItemOCC *)calloc(1, sizeof(tostItemOCC));
  if (lpstItem == NULL)
    {
      return NULL;
    }

  lpstItem->spstG22 = ppstG22;

  /*
   * LIN
   */

  fovdPrintLog (LOG_DEBUG, "fpstItemOCC_New: Loading LIN\n");
  strncpy(lasnzArticleNo, ppstG22->lin->v_7140, DES_SIZE);
  if (lasnzArticleNo[0] == 'O' && lasnzArticleNo[1] == '\0')
    {
      fovdPrintLog (LOG_DEBUG, "fpstItemOCC_New: Loading IMD with info for OCC\n");
      lpstImd = fpstFindItemDescription(ppstG22->imd, "FE");
      if (lpstImd == NULL)
        {
          return NULL;
        }
      
      strncpy(lpstItem->sasnzFE, lpstImd->v_7008a, DES_SIZE);
    }
  else
    {
      fovdPrintLog (LOG_DEBUG, "fpstItemOCC_New: Loading VAS OCC\n");
      n = strlen(lasnzArticleNo);
      for (i = 0, j = 0, loiFieldNo = 0; i <= n; i++)
        {      
          if (lasnzArticleNo[i] == '.' || lasnzArticleNo[i] == '\0')
            {
              lasnzField[j] = '\0';
              fovdPrintLog (LOG_DEBUG, "fpstItemOCC_New: Field[%d]: %s\n", loiFieldNo, lasnzField);
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
    }

  /*
   * MOA+125
   */

  fovdPrintLog (LOG_DEBUG, "fpstItemOCC_New: Loading MOA\n");
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

  fovdPrintLog (LOG_DEBUG, "fpstItemOCC_New: Loading QTY\n");
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

toenBool foenItemOCC_Delete(tostItemOCC *ppstItem)
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

toenBool foenItemOCC_Merge(tostItemOCC *ppstItemA, tostItemOCC *ppstItemB)
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

toenBool foenItemOCC_Match(tostItemOCC *ppstItemA, tostItemOCC *ppstItemB)
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

toenBool foenItemOCC_Gen(tostItemOCC *ppstItem)
{
  tostOCCRecord lostOCCRecord;
  toenBool loenStatus;

  fovdPrintLog (LOG_DEBUG, "foenItemOCC_Gen: Format item\n");
  loenStatus = foenItemOCC_FillRecord(ppstItem, &lostOCCRecord);
  if (loenStatus == FALSE)
    {
      fovdPopFunctionName();
      return FALSE;
    }
  
  fovdPrintLog (LOG_DEBUG, "foenItemOCC_Gen: Format money\n");
  fovdFormatMoney(lostOCCRecord.sasnzMonetaryAmount);  

  fovdPrintLog (LOG_DEBUG, "foenItemOCC_Gen: Printing\n");
  fovdGen("SimSummaryItemOther", 
          lostOCCRecord.sasnzTariffModel, 
          lostOCCRecord.sasnzServicePackage,
          lostOCCRecord.sasnzService,
          lostOCCRecord.sasnzMonetaryAmount, 
          lostOCCRecord.sasnzCurrency,
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

toenBool foenItemOCC_Empty(tostItemOCC *ppstItem)
{
  if (foenMoney_Eq(ppstItem->soflMoa, 0.0) == TRUE)
    {
      return TRUE;
    }

  return FALSE;
}


