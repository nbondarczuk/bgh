#include <string.h>
#include "types.h"
#include "gen.h"
#include "ing_timm.h"
#include "strutl.h"
#include "turnover.h"

/*
#define LOG(a, b) fprintf(stderr, a, b)
*/
#define LOG(a, b)   fovdPrintLog (LOG_DEBUG, a, b);

/*
 * Static functions
 */

static toenBool foenTurnoverItem_Gen(tostTurnover *ppstTurnover); 
static toenBool foenTurnoverItem_Next(tostTurnover *ppstTurnover);

/*
 * Static variables
 */

static struct s_TimmInter *spstTimmInter;
static struct s_group_22 *spstG22;

int foiTurnoverItems_Count(struct s_TimmInter *lpstTimmInter)
{
  struct s_group_22 *lpstG22;
  struct s_lin_seg *lpstLIN;
  int loiItems;
  
  /*
   * Count all items G22 where LIN type is TURNOV
   */

  loiItems = 0;
  lpstG22 = lpstTimmInter->timm->g_22;
  while (lpstG22) 
    {
      lpstLIN = lpstG22->lin;
      if (EQ(lpstLIN->v_7140, "TURNOV"))
        {
          loiItems++;
        }

      lpstG22 = lpstG22->g_22_next;
    }
  
  return loiItems;
}

void fovdTurnoverList_Init(struct s_TimmInter *lpstTimmInter)
{
  /*
   * Set up interchange
   */

  spstTimmInter = lpstTimmInter;

  /*
   * Set up G22 list
   */
  spstG22 = spstTimmInter->timm->g_22;
}

static toenBool foenTurnoverItem_Next(tostTurnover *ppstTurnover)
{
  struct s_lin_seg *lpstLIN;
  struct s_dtm_seg *lpstDTM;
  struct s_imd_seg *lpstIMD;
  struct s_group_3 *lpstG3;
  struct s_group_26 *lpstG26;
  struct s_rff_seg *lpstRFF;
  struct s_group_23 *lpstG23;
  struct s_moa_seg *lpstMOA;

  /*
   * Go to next element
   */

  if (spstG22 == NULL)
    {
      return FALSE;
    }
  
  while (spstG22)
    {
      lpstLIN = spstG22->lin;
      if (EQ(lpstLIN->v_7140, "TURNOV"))
        {
          break;
        }
      
      spstG22 = spstG22->g_22_next;
    }
  
  fovdPrintLog (LOG_DEBUG, "%s\n", lpstLIN->v_7140);

  if (spstG22 == NULL)
    {
      return FALSE;
    }
  
  /*
   * Load structure ppstTurnover
   */
  
  /*
   * Date
   */

  lpstDTM = spstG22->dtm;
  while (lpstDTM)
    {
      if (EQ(lpstDTM->v_2005, "601"))
        {
          break;
        }
      
      lpstDTM = lpstDTM->dtm_next;
    }
  
  if (lpstDTM == NULL)
    {
      return FALSE;
    }

  strcpy(ppstTurnover->sachzDate, lpstDTM->v_2380);
  LOG("DTM:%s\n", lpstDTM->v_2380);

  /*
   * Operation
   */
  lpstIMD = spstG22->imd;
  
  LOG("IMD:%s\n", lpstIMD->v_7009);
  if (EQ(lpstIMD->v_7009, "IVTRO"))
    {
      strcpy(ppstTurnover->sachzOperation, "");
    }
  else if (EQ(lpstIMD->v_7009, "PMTRO"))
    {
      lpstG26 = spstG22->g_26;
      if ((lpstRFF =  fpstFindItemReference(lpstG26, "RF")) == NULL)
        {
          strcpy(ppstTurnover->sachzOperation, "");      
        }
      else
        {
          strcpy(ppstTurnover->sachzOperation, lpstRFF->v_1154);      
          LOG("RFF:%s\n",  lpstRFF->v_1154);                
        }
    }
  else
    {
      return FALSE;
    }

  LOG("IMD:%s\n", lpstIMD->v_7009);

  /*
   * Invoice
   */

  lpstG26 = spstG22->g_26;
  if ((lpstRFF =  fpstFindItemReference(lpstG26, "IV")) == NULL)
    {
      return FALSE;
    }
  
  strcpy(ppstTurnover->sachzInvoice, lpstRFF->v_1154);      
  LOG("RFF:%s\n",  lpstRFF->v_1154);       

  /*
   * State
   */

  lpstG23 = spstG22->g_23;
  if ((lpstMOA = fpstFindPaymentSegment(lpstG23, "606")) == NULL)
    {
      return FALSE;
    }

  strcpy(ppstTurnover->sachzState, lpstMOA->v_5004);
  LOG("MOA:%s\n",  lpstMOA->v_5004);
  
  /*
   * Payment
   */

  lpstG23 = spstG22->g_23;
  if ((lpstMOA = fpstFindPaymentSegment(lpstG23, "607")) == NULL)
    {
      return FALSE;
    }

  strcpy(ppstTurnover->sachzPayment, lpstMOA->v_5004);
  LOG("MOA:%s\n",  lpstMOA->v_5004);
  
  /*
   * Saldo
   */

  lpstG23 = spstG22->g_23;
  if ((lpstMOA = fpstFindPaymentSegment(lpstG23, "608")) == NULL)
    {
      return FALSE;
    }

  strcpy(ppstTurnover->sachzSaldo, lpstMOA->v_5004);  
  LOG("MOA:%s\n",  lpstMOA->v_5004);
  
  ppstTurnover->spstG30 = spstG22->g_30;
  
  /*
   * Skip to the next item
   */
  
  spstG22 = spstG22->g_22_next;
  
  return TRUE;
}

void fovdTurnoverList_Free()
{
  spstG22 = NULL;
}


toenBool foenTurnoverList_Gen(struct s_TimmInter *spstTimmInter)
{
  int loiItems;
  toenBool loenStatus;
  static char lpchzItems[MAX_BUFFER];
  static tostTurnover lostTurnover;
  
  /*
   * Open output list, XARGS = number of elements in a list
   */

  sprintf(lpchzItems, "%d", loiItems = foiTurnoverItems_Count(spstTimmInter));
  fovdGen("TurnoverListStart", lpchzItems, EOL);
  
  /*
   * for each element of g22 element sequence 
   */

  fovdTurnoverList_Init(spstTimmInter);

  while ((loenStatus = foenTurnoverItem_Next(&lostTurnover)) != FALSE)
    {
      foenTurnoverItem_Gen(&lostTurnover);
    }

  fovdTurnoverList_Free();
  
  /*
   * Close output list
   */

  fovdGen("TurnoverListEnd", EOL);

  return TRUE;
}  

static toenBool foenTurnoverItem_Gen(tostTurnover *ppstTurnover) 
{
  static char lpchzDate[MAX_BUFFER];
  static char lpchzState[MAX_BUFFER];
  static char lpchzPayment[MAX_BUFFER];
  static char lpchzSaldo[MAX_BUFFER];
  static char lpchzTaxValue[2][MAX_BUFFER];
  struct s_moa_seg *lpstMOA;
  struct s_tax_seg *lpstTAX;
  struct s_group_30 *lpstG30;
  char *lachzTaxes[2] = {"Zredukowany", "Calkowity"};
  int i, n;
  
  /*
   * XARGS = tostTurnover
   */
  
  fovdPrintLog (LOG_DEBUG, "%s\n", ppstTurnover->sachzDate);
  strcpy(lpchzDate, ppstTurnover->sachzDate);
  fovdFormatDate(lpchzDate, YY_MM_DD);  

  fovdPrintLog (LOG_DEBUG, "%s\n", ppstTurnover->sachzState);
  strcpy(lpchzState, ppstTurnover->sachzState);
  fovdFormatMoney(lpchzState);  

  fovdPrintLog (LOG_DEBUG, "%s\n", ppstTurnover->sachzPayment);
  strcpy(lpchzPayment, ppstTurnover->sachzPayment);
  fovdFormatMoney(lpchzPayment);  

  fovdPrintLog (LOG_DEBUG, "%s\n", ppstTurnover->sachzSaldo);
  strcpy(lpchzSaldo, ppstTurnover->sachzSaldo);
  fovdFormatMoney(lpchzSaldo);  

  fovdGen("TurnoverItem", 
          lpchzDate,
          ppstTurnover->sachzOperation,
          ppstTurnover->sachzInvoice,
          lpchzState,
          lpchzPayment,
          lpchzSaldo,
          "PLN",
          EOL);

  return TRUE;
}





