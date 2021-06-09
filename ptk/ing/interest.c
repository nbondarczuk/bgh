#include <string.h>
#include "types.h"
#include "gen.h"
#include "ing_timm.h"
#include "strutl.h"
#include "interest.h"

/*
#define LOG(a, b) fprintf(stderr, a, b)
*/

#define LOG(a, b)   fovdPrintLog (LOG_DEBUG, a, b);

/*
 * Static functions
 */

static toenBool foenInterestItem_Gen(tostInterest *ppstInterest);
static toenBool foenInterestItem_Next(tostInterest *ppstInterest);

/*
 * Static variables
 */

static struct s_TimmInter *spstTimmInter;
static struct s_group_22 *spstG22;

int foiInterestItems_Count(struct s_TimmInter *lpstTimmInter)
{
  struct s_group_22 *lpstG22;
  struct s_lin_seg *lpstLIN;
  int loiItems;
  
  /*
   * Count all items G22 where LIN type is INTER
   */
  
  loiItems = 0;
  lpstG22 = lpstTimmInter->timm->g_22;
  while (lpstG22) 
    {
      lpstLIN = lpstG22->lin;
      if (EQ(lpstLIN->v_7140, "INTER"))
        {
          loiItems++;
        }

      lpstG22 = lpstG22->g_22_next;
    }
  
  return loiItems;
}

void fovdInterestList_Init(struct s_TimmInter *lpstTimmInter)
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

static toenBool foenInterestItem_Next(tostInterest *ppstInterest)
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
      
      /*
       * Find next INTER type LIN block in g22 list
       */

      if (EQ(lpstLIN->v_7140, "INTER"))
        {
          /*
           * OK, INTER block found
           */
          
          break;
        }
      
      spstG22 = spstG22->g_22_next;
    }

  if (spstG22 == NULL)
    {
      return FALSE;
    }

  /*
   * Load structure ppstInterest
   */

  /*
   * Date
   */
  
  lpstDTM = spstG22->dtm;
  while (lpstDTM)
    {
      if (EQ(lpstDTM->v_2005, "600"))
        {
          break;
        }
      
      lpstDTM = lpstDTM->dtm_next;
    }
  
  if (lpstDTM == NULL)
    {
      return FALSE;
    }

  strcpy(ppstInterest->sachzDate, lpstDTM->v_2380);  
  LOG("DTM:%s\n", lpstDTM->v_2380);
  
  /*
   * Operation type from IMD block
   */

  lpstIMD = spstG22->imd;
  if (lpstIMD == NULL)
    {
      return FALSE;
    }

  LOG("IMD:%s\n", lpstIMD->v_7009);  
  if (EQ(lpstIMD->v_7009, "IVTRO"))
    {
      ppstInterest->soenIntSeqEnd = FALSE;
      lpstG26 = spstG22->g_26;
      if ((lpstRFF =  fpstFindItemReference(lpstG26, "RF")) == NULL)
        {
          return FALSE;
        }
      
      strcpy(ppstInterest->sachzOperation, lpstRFF->v_1154);      
      LOG("RFF:%s\n",  lpstRFF->v_1154);      
    }
  else if (EQ(lpstIMD->v_7009, "PMTRO"))
    {
      ppstInterest->soenIntSeqEnd = FALSE;
      lpstG26 = spstG22->g_26;
      if ((lpstRFF =  fpstFindItemReference(lpstG26, "RF")) == NULL)
        {
          return FALSE;
        }
     
      strcpy(ppstInterest->sachzOperation,  lpstRFF->v_1154);      
      LOG("RFF:%s\n",  lpstRFF->v_1154);      
    }
  else if(EQ(lpstIMD->v_7009, "STATE"))
    {
      ppstInterest->soenIntSeqEnd = TRUE;
      lpstG26 = spstG22->g_26;
      if ((lpstRFF =  fpstFindItemReference(lpstG26, "RF")) == NULL)
        {
          return FALSE;
        }
     
      strcpy(ppstInterest->sachzOperation,  lpstRFF->v_1154);      
      LOG("RFF:%s\n",  lpstRFF->v_1154);      
    }
  else
    {
      return FALSE;
    }


  /*
   * State
   */

  lpstG23 = spstG22->g_23;
  if ((lpstMOA = fpstFindPaymentSegment(lpstG23, "501")) == NULL)
    {      
      return FALSE;
    }

  strcpy(ppstInterest->sachzState, lpstMOA->v_5004);
  LOG("MOA:%s\n",  lpstMOA->v_5004);
  
  /*
   * Payment
   */

  lpstG23 = spstG22->g_23;
  if ((lpstMOA = fpstFindPaymentSegment(lpstG23, "502")) == NULL)
    {
      return FALSE;
    }

  strcpy(ppstInterest->sachzPayment, lpstMOA->v_5004);
  LOG("MOA:%s\n",  lpstMOA->v_5004);
  
  /*
   * Saldo
   */

  lpstG23 = spstG22->g_23;
  if ((lpstMOA = fpstFindPaymentSegment(lpstG23, "503")) == NULL)
    {
      return FALSE;
    }

  strcpy(ppstInterest->sachzSaldo, lpstMOA->v_5004);  
  LOG("MOA:%s\n",  lpstMOA->v_5004);
  
  /*
   * Delay
   */

  lpstG23 = spstG22->g_23;
  if ((lpstMOA = fpstFindPaymentSegment(lpstG23, "504")) == NULL)
    {
      return FALSE;
    }

  strcpy(ppstInterest->sachzDelay, lpstMOA->v_5004);
  LOG("MOA:%s\n",  lpstMOA->v_5004);
  
  /*
   * Interest
   */
  
  lpstG23 = spstG22->g_23;
  if ((lpstMOA = fpstFindPaymentSegment(lpstG23, "505")) == NULL)
    {
      return FALSE;
    }
  
  strcpy(ppstInterest->sachzInterest, lpstMOA->v_5004);
  LOG("MOA:%s\n",  lpstMOA->v_5004);

  /*
   * Skip actual item
   */

  spstG22 = spstG22->g_22_next;

  return TRUE;
}

void fovdInterestList_Free()
{
  spstG22 = NULL;
}

/*
 * Table Interest
 */

toenBool foenInterestList_Gen(struct s_TimmInter *spstTimmInter)
{
  int loiItems;
  static char lpchzItems[MAX_BUFFER];
  static tostInterest loenInterest;
  toenBool loenStatus;

  /*
   * Open output list, XARGS = number of elements in a list
   */
  
  sprintf(lpchzItems, "%d", loiItems = foiInterestItems_Count(spstTimmInter));
  fovdGen("InterestListStart", lpchzItems, EOL);
  
  /*
   * for each element of g22 element sequence 
   */

  fovdInterestList_Init(spstTimmInter);

  while ((loenStatus = foenInterestItem_Next(&loenInterest)) != FALSE)
    {
      foenInterestItem_Gen(&loenInterest);
    }

  fovdInterestList_Free();

  /*
   * Close output list
   */

  fovdGen("InterestListEnd", EOL);

  return TRUE;
}

static toenBool foenInterestItem_Gen(tostInterest *ppstInterest)
{
  static char lpchzDate[MAX_BUFFER];
  static char lpchzState[MAX_BUFFER];
  static char lpchzPayment[MAX_BUFFER];
  static char lpchzSaldo[MAX_BUFFER];
  static char lpchzInterest[MAX_BUFFER];
  
  /*
   * XARGS = tostInterest
   */
  
  strcpy(lpchzDate, ppstInterest->sachzDate);
  fovdFormatDate(lpchzDate, YY_MM_DD);  
  
  strcpy(lpchzState, ppstInterest->sachzState);
  fovdFormatMoney(lpchzState);  

  strcpy(lpchzPayment, ppstInterest->sachzPayment);
  fovdFormatMoney(lpchzPayment);  

  strcpy(lpchzSaldo, ppstInterest->sachzSaldo);
  fovdFormatMoney(lpchzSaldo);  

  strcpy(lpchzInterest, ppstInterest->sachzInterest);
  fovdFormatMoney(lpchzInterest);  

  fovdGen("InterestItem", 
          lpchzDate,
          ppstInterest->sachzOperation,
          lpchzState,
          lpchzPayment,
          lpchzSaldo,
          ppstInterest->sachzDelay,
          lpchzInterest,
          "PLN",
          EOL);  

  if (ppstInterest->soenIntSeqEnd == TRUE)
    {
      fovdGen("InterestItemEnd", EOL);
    }

  return TRUE;
}


