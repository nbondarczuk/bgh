/**************************************************************************/
/*  MODULE : SUM SHEET data fetecher                                      */
/*                                                                        */
/*  AUTHOR : N.Bondarczuk                                                 */
/*                                                                        */
/*  CREATION DATE : 22.10.97                                              */
/*                                                                        */
/*  DESCRIPTION : Exports  functions accessing information from TIMM      */
/*                SUM SHEET message.                                      */ 
/**************************************************************************/
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "bgh.h"
#include "protos.h"
#include "parser.h"

#include "types.h"
#include "gen.h"
#include "inv_types.h"
#include "inv_item.h"
#include "inv_fetch.h"
#include "sum_fetch_acc.h"


#if 0	/* just for version.sh */
static char *SCCS_VERSION = "4.1";
#endif

#define EMPTY(s) (s[0] == '\0')

static char *findSumAmount(struct s_group_45 *, char *);

/*
 * MODULE LOCAL : Lookup in the sequence of group 45 blocks for a proper MOA block
 */

static char *findSumAmount(struct s_group_45 *ppstG45, char *pasnzId) 
{
  struct s_moa_seg *lpstMOA;
  struct s_group_45 *lpstG45;

  fovdPrintLog (LOG_DEBUG, "findSumAmount: Look for %s\n", pasnzId);   
  
  lpstG45 = ppstG45;
  while (lpstG45) 
    {
      lpstMOA = lpstG45->moa;
      if (lpstMOA) 
        {
          fovdPrintLog (LOG_DEBUG, "findSumAmount: MOA+%s\n", lpstMOA->v_5025);   
          if (EQ(lpstMOA->v_5025, pasnzId)) 
            {
              return lpstMOA->v_5004;
            }
        }
      
      lpstG45 = lpstG45->g_45_next;
    }
  
  return NULL;
}

static char *findSumCurrency(struct s_group_45 *g_45, char *amount_id) 
{
  struct s_moa_seg *moa;
  
  while (g_45) 
    {
      moa = g_45->moa;
      if (moa) 
        {
          if (EQ(moa->v_5025, amount_id)) 
            {
              return moa->v_6345;
            }
        }

      g_45 = g_45 -> g_45_next;
    }
  
  return NULL;
}


struct s_group_22 *findSubscriberSegment(struct s_TimmInter *sum_ti, int n) 
{
  int i;
  struct s_group_22 *g_22;
  struct s_lin_seg *lin;
  
  g_22 = sum_ti->timm->g_22;              /* from the beginning */
  i = 0;
  while (g_22) 
    {
      lin = g_22->lin;
      if (lin && lin->v_1222)             /* is it correct TIMM structure */
        {                 
          if (EQ(lin->v_1222, "01"))      /* we are on 01 level */
            {    
              if (i == n)                 /* that is this LIN block so let's break */ 
                {                                           
                  break;
                }
             
              i++;
            }
        }
      else
        {
          return NULL;
        }

      g_22 = g_22->g_22_next;
    }

  return g_22;
}

struct s_group_22 *findSubscriberContractSegment(struct s_TimmInter *sum_ti, int sn, int cn) 
{
  struct s_group_22 *g_22;
  struct s_lin_seg *lin;
  int i;

  g_22 = findSubscriberSegment(sum_ti, sn);

  i = 0;
  while (g_22) 
    {
      lin = g_22->lin;
      if (lin) 
        {
          if (lin->v_1222 && EQ(lin->v_1222, "02") &&  strlen(lin->v_7140) == 0)
            {
              if (cn == i)
                {
                  return g_22;
                }
              else
                {
                  i++;
                }
            }
        }    

      g_22 = g_22->g_22_next;
    }
  
  return NULL;
}

int fetchSubscriberAccountPayment(struct s_TimmInter *sum_ti, int poiSubscriber,
                                  char *pachzAccount, int poiAccountBufLen,
                                  char *pachzNC, int poiNCBufLen,
                                  char *pachzAmount, int poiAmountBufLen, 
                                  char *pachzCurrency, int poiCurrencyBufLen)
{
  struct s_group_22 *g_22;
  struct s_group_23 *g_23;
  struct s_group_26 *g_26;
  struct s_rff_seg *rff;
  struct s_moa_seg *moa;
  int loiFound;
  
  g_22 = findSubscriberSegment(sum_ti, poiSubscriber);
  if (g_22 == NULL)
    {
      return FALSE;
    }

  /*
   * SUBSCRIBER ACCOUNT
   */

  g_26 = g_22->g_26;
  loiFound = FALSE;
  while (g_26)
    {
      rff = g_26->rff;
      if (EQ(rff->v_1153, "IT"))
        {
          strncpy(pachzAccount, rff->v_1154, poiAccountBufLen);
          loiFound = TRUE;
          break;
        }
      g_26 = g_26->g_26_next;
    }

  if (loiFound == FALSE)
    {
      return FALSE;
    }

  /*
   * NUMBER OF CONTRACTS
   */

  g_26 = g_22->g_26;
  loiFound = FALSE;
  while (g_26)
    {
      rff = g_26->rff;
      if (EQ(rff->v_1153, "NC"))
        {
          strncpy(pachzNC, rff->v_1154, poiNCBufLen);
          loiFound = TRUE;
          break;
        }
      g_26 = g_26->g_26_next;
    }

  if (loiFound == FALSE)
    {
      return FALSE;
    }

  /*
   * SUMMARY CONTRACT PAYMENT
   */

  g_23 = g_22->g_23;
  loiFound = FALSE;
  while (g_23)
    {
      moa = g_23->moa;
      if (EQ(moa->v_5025, "930"))
        {
          strncpy(pachzAmount, moa->v_5004, poiAmountBufLen);
          strncpy(pachzCurrency, moa->v_6345, poiCurrencyBufLen);
          loiFound = TRUE;
          break;
        }
      g_23 = g_23->g_23_next;
    }

  if (loiFound == FALSE)
    {
      return FALSE;
    }
  
  return TRUE;
}

int findSubscriberContractsNumber(struct s_TimmInter *sum_ti, int sn) 
{
  struct s_group_22 *g_22;
  struct s_lin_seg *lin;
  int i;
  
  i = 0;
  g_22 = findSubscriberSegment(sum_ti, sn);
  while (g_22) 
    {
      lin = g_22->lin;
      if (lin) 
        {
          if (strlen(lin->v_7140)) 
            {  
              i++;
            }
        }    
      
      g_22 = g_22->g_22_next;
    }
    
  return i;
}

/**************************************************************************/
/*  FUNCTION : fetchSubscribersNumber                                     */
/*                                                                        */
/*  ARGUMENTS : IN sum_ti - SUMMARY SHEET TIMM structure                  */
/*                                                                        */
/*  RETURN : number of subscribers in SUM SHEET msg or -1 value if error  */
/*           found in s_TimmInter structure                               */
/*                                                                        */
/*  DESCRIPTION : Returns the number of LIN blocks with nesting level 01  */
/*                in field LIN.V_1222.                                    */
/**************************************************************************/

int fetchSubscribersNumber(struct s_TimmInter *sum_ti) 
{
  int i;
  struct s_group_22 *g_22;             
  struct s_lin_seg *lin;

  g_22 = sum_ti->timm->g_22;
  if (g_22) 
    {
      i = 0;
      while (g_22) 
        {
          lin = g_22->lin;
          if (lin && lin->v_1222) 
            {
              if (EQ(lin->v_1222, "01"))
                {
                  i++;
                }
            }
          else
            {
              return -1;
            }

          g_22 = g_22->g_22_next;
        }  
    }
  else
    {
      return -1;
    }

  return i;
}



/**************************************************************************/
/*  FUNCTION : fetchSubscriberCustomerCode                                */
/*                                                                        */
/*  ARGUMENTS : IN sum_ti - SUMMARY SHEET TIMM structure                  */
/*              IN i  - index of subscriber being found                   */
/*              OUT customer code buffer                                  */
/*                                                                        */
/*  RETURN : status flag equal TRUE iff OK, else FALSE                    */
/*                                                                        */
/*  DESCRIPTION : Returns the value from LIN blocks from group 26         */
/*                block RFF filed V_1153 = "IT"  with nesting             */
/*                level 01 (subscriber level) in field LIN.V_1222.        */
/*                of a customer identified by a LIN block index.          */
/*                This value means : CUSTOMER CODE                        */
/**************************************************************************/

int fetchSubscriberCustomerCode(struct s_TimmInter *sum_ti, int index, char *cc_buf, int cc_buf_len) 
{
  int found, n, i;
  struct s_group_22 *g_22;
  struct s_group_26 *g_26;
  struct s_group_22 *findSubscriberSegment(IN struct s_TimmInter *, IN int);  
  struct s_rff_seg *rff;

  n = fetchSubscribersNumber(sum_ti);
  for (i = 0; i < n; i++) 
    {
      g_22 = findSubscriberSegment(sum_ti, index);  
      g_26 = g_22->g_26;
      found = FALSE;
      while (g_26) 
        {
          rff = g_26->rff;
          if (rff) 
            {
              if (EQ(rff->v_1153, "IT")) 
                {
                  found = TRUE;
                  break;
                }
            }
          else
            {
              return FALSE;
            }

          g_26 = g_26->g_26_next;
        }

      if (found) 
        {
          strncpy(cc_buf, rff->v_1154, cc_buf_len);
          return TRUE;
        }
    }

  return FALSE;
};


/**************************************************************************/
/*  FUNCTION : fetchSubscriberContractsNumber                             */
/*                                                                        */
/*  ARGUMENTS : IN sum_ti - SUMMARY SHEET TIMM structure                  */
/*              IN subscriber_no - number of subscriber being found       */
/*                                                                        */
/*  RETURN : number of constracts in SUM SHEET msg                        */
/*                                                                        */
/*  DESCRIPTION : Returns the value from LIN blocks from group 26         */
/*                block RFF filed V_1153 = "NC"  with nesting             */
/*                level 01 (subscriber level) in field LIN.V_1222.        */
/*                of a customer identified by a given code.               */
/*                This value means : NUMBER OF SUBSCRIBER CONTRACTS       */
/**************************************************************************/



int fetchSubscriberContractsNumber(IN struct s_TimmInter *sum_ti, IN char *cc_buf, IN int cc_buf_len) 
{
  int found, n, i;
  struct s_group_22 *g_22;
  struct s_group_26 *g_26;
  struct s_group_22 *findSubscriberSegment(IN struct s_TimmInter *, IN int);  
  struct s_rff_seg *rff;
  
  n = fetchSubscribersNumber(sum_ti);
  for (i = 0; i < n; i++) 
    {
      g_22 = findSubscriberSegment(sum_ti, i);  
      if (g_22) 
        {
          g_26 = g_22->g_26;
          found = FALSE;
          while (g_26) 
            {
              rff = g_26->rff;
              if (rff) 
                {
                  if (EQ(rff->v_1153, "IT") && EQ(cc_buf, rff->v_1154)) 
                    {
                      found = TRUE;
                      break;
                    }
                }
              else
                {
                  return FALSE;
                }

              g_26 = g_26->g_26_next;
            }
        }
      else 
        {
          return FALSE;
        }

      if (found) 
        {
          g_26 = g_22->g_26;
          while (g_26) 
            {
              rff = g_26->rff;
              if (rff) 
                {
                  if (EQ(rff->v_1153, "NC")) 
                    {
                      return atoi(rff->v_1154);
                    }
                }

              g_26 = g_26->g_26_next;
            }
        }
      else
        {
          return FALSE;
        }
    }
  
  return FALSE;
}


/**************************************************************************/
/*  FUNCTION : fetchContractId                                            */
/*                                                                        */
/*  ARGUMENTS : IN sum_ti - SUMMARY SHEET TIMM structure                  */
/*              OUT contract_id - buffer with contract id                 */
/*                                                                        */
/*  RETURN : TRUE iff everything OK                                       */
/*           FALSE iff error                                              */
/*                                                                        */
/*  DESCRIPTION : Returns the contract id from the field IMD.V_7008       */
/*                of IMD block with IMD.V_7009 = "CO". LIN blocks are     */
/*                are referenced by number in the sequence of all LIN     */
/*                blocks with nesting level 01.                           */
/*                We use for that function findSubscriberSegment(ti, sn)  */
/*                where ti - SUM SHEET, sn - sequential block number      */
/*                with description of the subscriber.                     */
/**************************************************************************/

int fetchContractId(IN struct s_TimmInter *sum_ti, IN int sn, IN int cn, 
                    OUT char *contract_id_buf, IN int contract_id_buf_len) 
{
  int n, i;
  struct s_group_22 *g_22;
  struct s_lin_seg *lin;
  struct s_imd_seg *imd;

  g_22 = findSubscriberContractSegment(sum_ti, sn, cn);
  if (g_22) 
    {
      /* find LIN segment with empty v_7140 field */
      lin = g_22->lin;
      if (lin) 
        {
          if (lin->v_7140 == NULL) 
            {
              imd = g_22->imd;    /* that's it - block LIN with costs for a contract */
              while (imd)         /* in the sequence of IMD blocks we look for a IMD block with CO in field v_7009 */
                {
                  if (EQ(imd->v_7009, "CO")) 
                    {
                      strncpy(contract_id_buf, imd->v_7008, contract_id_buf_len);
                      return TRUE;
                    }
                  
                  imd = imd->imd_next;
                }
            }
        }
    }


  return FALSE;
};

/**************************************************************************/
/*  FUNCTION : fetchContractsMarket                                       */
/*                                                                        */
/*  ARGUMENTS : IN sum_ti - SUMMARY SHEET TIMM structure                  */
/*              OUT contract_id - buffer with contract id                 */
/*                                                                        */
/*  RETURN : TRUE iff everything OK                                       */
/*           FALSE iff error                                              */
/*                                                                        */
/*  DESCRIPTION : Returns the contract id from the field IMD.V_7008       */
/*                of IMD block with IMD.V_7009 = "MRKT". LIN blocks are   */
/*                are referenced by number in the sequence of all LIN     */
/*                blocks with nesting level 01.                           */
/**************************************************************************/

int fetchContractMarket(struct s_TimmInter *sum_ti, int sn, int cn, 
                        char *market_buf, int market_buf_len,
                        char *market_code_buf, int market_code_buf_len) 
{
  int n, i;
  struct s_group_22 *g_22;
  struct s_lin_seg *lin;
  struct s_imd_seg *imd;
  
  g_22 = findSubscriberContractSegment(sum_ti, sn, cn);
  if (g_22) 
    {
      /* 
       * find LIN segment with empty v_7140 field 
       */

      lin = g_22->lin;
      if (lin) 
        {
          if (EMPTY(lin->v_7140)) 
            {
              imd = g_22->imd;    /* that's it - block LIN with costs for a contract */
              while (imd)         /* in the sequence of IMD blocks we look for a IMD block with MRKT in field v_7009 */  
                {       
                  if (EQ(imd->v_7009, "MRKT")) 
                    {
                      strncpy(market_buf, imd->v_7008a, market_buf_len);
                      strncpy(market_code_buf, imd->v_7008, market_code_buf_len);
                      return TRUE;
                    }
                  
                  imd = imd->imd_next;
                }
              
              fovdPrintLog (LOG_DEBUG, "No IMD block with tag MRKT found\n");     
            }
        }
      else
        {
          fovdPrintLog (LOG_DEBUG, "LIN block is empty\n");     
        }
    }
  else
    {
      fovdPrintLog (LOG_DEBUG, "G22 not found\n");     
    }

  return FALSE;
};

/**************************************************************************/
/*  FUNCTION : fetchContractStorageMedium                                 */
/*                                                                        */
/*  ARGUMENTS : IN sum_ti - SUMMARY SHEET TIMM structure                  */
/*              OUT contract_id - buffer with contract id                 */
/*                                                                        */
/*  RETURN : TRUE iff everything OK                                       */
/*           FALSE iff error                                              */
/*                                                                        */
/*  DESCRIPTION : Returns the contract market from the field IMD.V_7008   */
/*                of IMD block with IMD.V_7009 = "SMNUM". LIN blocks are  */
/*                are referenced by number in the sequence of all LIN     */
/*                blocks with nesting level 01.                           */
/**************************************************************************/

int fetchContractStorageMedium(IN struct s_TimmInter *sum_ti, IN int sn, IN int cn, 
                               OUT char *dn_buf, IN int dn_buf_len) 
{
  int n, i;
  struct s_group_22 *g_22;
  struct s_lin_seg *lin;
  struct s_imd_seg *imd;
  
  g_22 = findSubscriberContractSegment(sum_ti, sn, cn);
  if (g_22) 
    {
      lin = g_22->lin;
      if (lin) 
        {
          if (EMPTY(lin->v_7140)) 
            {
              imd = g_22->imd;
              if (imd) 
                {
                  while (imd) 
                    { 
                      if (EQ(imd->v_7009, "SMNUM")) 
                        {
                          strncpy(dn_buf, imd->v_7008a, dn_buf_len);
                          return TRUE;
                        }

                      imd = imd->imd_next;
                    }

                  return FALSE; 
                }
              else 
                {
                  return FALSE;
                }
            }
        }
    }
  else 
    {
      return FALSE;
    }

  return FALSE;
};

/**************************************************************************/
/*  FUNCTION : fetchContractDirectoryNumber                               */
/*                                                                        */
/*  ARGUMENTS : IN sum_ti - SUMMARY SHEET TIMM structure                  */
/*              OUT contract_id - buffer with contract id                 */
/*                                                                        */
/*  RETURN : TRUE iff everything OK                                       */
/*           FALSE iff error                                              */
/*                                                                        */
/*  DESCRIPTION : Returns the value from the field IMD.V_7008             */
/*                of IMD block with IMD.V_7009 = "DNNUM". LIN blocks are  */
/*                are referenced by number in the sequence of all LIN     */
/*                blocks with nesting level 01.                           */
/**************************************************************************/



int fetchContractDirectoryNumber(IN struct s_TimmInter *sum_ti, IN int sn, IN int cn, 
                                 OUT char *dn_buf, IN int dn_buf_len) 
{
  int n, i;
  struct s_group_22 *g_22;
  struct s_lin_seg *lin;
  struct s_imd_seg *imd;
  
  g_22 = findSubscriberContractSegment(sum_ti, sn, cn);
  if (g_22) 
    {
      lin = g_22->lin;
      if (lin) 
        {
          if (EMPTY(lin->v_7140)) 
            {
              imd = g_22->imd;
              if (imd) 
                {
                  while (imd) 
                    { 
                      if (EQ(imd->v_7009, "DNNUM")) 
                        {
                          strncpy(dn_buf, imd->v_7008a, dn_buf_len);
                          return TRUE;
                        }

                      imd = imd->imd_next;
                    }

                  return FALSE; 
                }
              else 
                {
                  return FALSE;
                }
            }
        }
    }
  else 
    {
      return FALSE;
    }

  return FALSE;
};

/**************************************************************************/
/*  FUNCTION : fetchContractAmount                                        */
/*                                                                        */
/*  ARGUMENTS : IN sum_ti - SUMMARY SHEET TIMM structure                  */
/*              OUT contract_id - buffer with contract id                 */
/*                                                                        */
/*  RETURN : TRUE iff everything OK                                       */
/*           FALSE iff error                                              */
/*                                                                        */
/*  DESCRIPTION : Returns the payment value from the field MOA.V_5004     */
/*                of MOA block with MOA.V_5025 = "931" and                */
/*                MOA.V_4405 = "9". LIN blocks                            */
/*                are referenced by number in the sequence of all LIN     */
/*                blocks with nesting level 01.                           */
/**************************************************************************/

int fetchContractAmount(struct s_TimmInter *sum_ti, int sn, int cn, char *amount_buf, int amount_buf_len, char *currency_buf, int currency_buf_len) 
{
  int found;
  struct s_group_22 *g_22;
  struct s_moa_seg *moa;
  struct s_group_23 *g_23;
  
  g_22 = findSubscriberContractSegment(sum_ti, sn, cn);
  if (g_22) 
    {
      g_23 = g_22->g_23;
      found = FALSE;
      while (g_23) 
        {
          moa = g_23->moa;
          if (moa) 
            {
              if (moa->v_5025 && EQ(moa->v_5025, "931")) 
                {
                  strncpy(amount_buf, moa->v_5004, amount_buf_len);
                  strncpy(currency_buf, moa->v_6345, currency_buf_len);
                  return  TRUE;
                }
            }

          g_23 = g_23->g_23_next;
        }
    }
  else
    {
      return FALSE;
    }

  return FALSE;
};

/**************************************************************************/
/*  FUNCTION : fetchAccountAccessPayment                                  */
/*                                                                        */
/*  ARGUMENTS : IN sum_ti - SUMMARY SHEET TIMM structure                  */
/*              OUT summary access currency                               */
/*              OUT summary access payment                                */
/*                                                                        */
/*  RETURN : TRUE iff everything OK                                       */
/*           FALSE iff error                                              */
/*                                                                        */
/*  DESCRIPTION :                                                         */
/**************************************************************************/

#define FIELD_V5025_LEN 3
#define FIELD_V4405_LEN 3
#define FIELD_V6345_LEN 3
#define ALL_CUSTOMERS_ACCESS_AMOUNT_STR "911"
#define INFO_AMOUNT_STR "9"

int fetchAccountAccessPayment(struct s_TimmInter *sum_ti, char *ac_buf, int ac_buf_len, char *ap_buf, int ap_buf_len) 
{
  char *pAmount, *pCurrency;
  struct s_group_45 *g_45;
  
  g_45 = sum_ti->timm->g_45;

  ac_buf[0] = '\0';
  ap_buf[0] = '\0';
  
  pCurrency = findSumCurrency(g_45, ALL_CUSTOMERS_ACCESS_AMOUNT_STR); 
  if (pCurrency)
    {
      strncpy(ac_buf, pCurrency, ac_buf_len);
    }
  else
    {
      return FALSE;
    }

  pAmount = findSumAmount(g_45, ALL_CUSTOMERS_ACCESS_AMOUNT_STR);
  if (pAmount)
    {
      strncpy(ap_buf, pAmount, ap_buf_len);
    }
  else
    {
      return FALSE;
    }

  return TRUE;
}


/**************************************************************************/
/*  FUNCTION : fetchAccountUsagePayment                                   */
/*                                                                        */
/*  ARGUMENTS : IN sum_ti - SUMMARY SHEET TIMM structure                  */
/*              OUT summary usage currency                                */
/*              OUT summary usage payment                                 */
/*                                                                        */
/*  RETURN : TRUE iff everything OK                                       */
/*           FALSE iff error                                              */
/*                                                                        */
/*  DESCRIPTION :                                                         */
/**************************************************************************/

#define ALL_CUSTOMERS_USAGE_AMOUNT_STR "912"

int fetchAccountUsagePayment(struct s_TimmInter *sum_ti, char *uc_buf, int uc_buf_len, char *up_buf, int up_buf_len) 
{
  char *pAmount, *pCurrency;
  struct s_group_45 *g_45;
  
  g_45 = sum_ti->timm->g_45;

  uc_buf[0] = '\0';
  up_buf[0] = '\0';
  
  pCurrency = findSumCurrency(g_45, ALL_CUSTOMERS_USAGE_AMOUNT_STR); 
  if (pCurrency)
    {
      strncpy(uc_buf, pCurrency, uc_buf_len);
    }
  else
    {
      return FALSE;
    }

  pAmount = findSumAmount(g_45, ALL_CUSTOMERS_USAGE_AMOUNT_STR);
  if (pAmount)
    {
      strncpy(up_buf, pAmount, up_buf_len);
    }
  else
    {
      return FALSE;
    }

  return TRUE;
}


/**************************************************************************/
/*  FUNCTION : fetchAccountServicesPayment                                */
/*                                                                        */
/*  ARGUMENTS : IN sum_ti - SUMMARY SHEET TIMM structure                  */
/*              OUT summary services currency                             */
/*              OUT summary services payment                              */
/*                                                                        */
/*  RETURN : TRUE iff everything OK                                       */
/*           FALSE iff error                                              */
/*                                                                        */
/*  DESCRIPTION :                                                         */
/**************************************************************************/

#define ALL_CUSTOMERS_SUBSCRIPTION_AMOUNT_STR "910"

int fetchAccountSubscriptionPayment(struct s_TimmInter *sum_ti, char *sc_buf, int sc_buf_len, char *sp_buf, int sp_buf_len) 
{
  char *pAmount, *pCurrency;
  struct s_group_45 *g_45;
  
  g_45 = sum_ti->timm->g_45;

  sc_buf[0] = '\0';
  sp_buf[0] = '\0';
  
  pCurrency = findSumCurrency(g_45, ALL_CUSTOMERS_SUBSCRIPTION_AMOUNT_STR); 
  if (pCurrency)
    {
      strncpy(sc_buf, pCurrency, sc_buf_len);
    }
  else
    {
      return FALSE;
    }

  pAmount = findSumAmount(g_45, ALL_CUSTOMERS_SUBSCRIPTION_AMOUNT_STR);
  if (pAmount)
    {      
      strncpy(sp_buf, pAmount, sp_buf_len);
    }
  else
    {
      return FALSE;
    }

  return TRUE;
}

/**************************************************************************/
/*  FUNCTION : fetchAccountAccessPayment                                  */
/*                                                                        */
/*  ARGUMENTS : IN sum_ti - SUMMARY SHEET TIMM structure                  */
/*              OUT summary others currency                               */
/*              OUT summary others payment                                */
/*                                                                        */
/*  RETURN : TRUE iff everything OK                                       */
/*           FALSE iff error                                              */
/*                                                                        */
/*  DESCRIPTION :                                                         */
/**************************************************************************/

int fetchAccountOthersPayment(struct s_TimmInter *sum_ti, char *oc_buf, int oc_buf_len, char *op_buf, int op_buf_len) 
{
  char *pAmount, *pCurrency;
  struct s_group_45 *g_45;
  
  g_45 = sum_ti->timm->g_45;

  oc_buf[0] = '\0';
  op_buf[0] = '\0';
  
  pCurrency = findSumCurrency(g_45, "913"); 
  if (pCurrency != NULL)
    {
      strncpy(oc_buf, pCurrency, oc_buf_len);
    }
  else
    {
      return FALSE;
    }

  pAmount = findSumAmount(g_45, "913");
  if (pAmount != NULL)
    {
      strncpy(op_buf, pAmount, op_buf_len);
    }
  else
    {
      return FALSE;
    }

  return TRUE;
}

/**************************************************************************/
/*  FUNCTION : fetchAccountSummaryPayment                                 */
/*                                                                        */
/*  ARGUMENTS : IN sum_ti - SUMMARY SHEET TIMM structure                  */
/*              OUT summary payment currency                              */
/*              OUT summary payment amount                                */
/*                                                                        */
/*  RETURN : TRUE iff everything OK                                       */
/*           FALSE iff error                                              */
/*                                                                        */
/*  DESCRIPTION :                                                         */
/**************************************************************************/

#define ALL_CUSTOMERS_SUMMARY_AMOUNT_STR "77"

int fetchAccountSummaryPayment(struct s_TimmInter *sum_ti, char *sc_buf, int sc_buf_len, char *sp_buf, int sp_buf_len) 
{
  char *pAmount, *pCurrency;
  struct s_group_45 *g_45;
  
  g_45 = sum_ti->timm->g_45;

  sc_buf[0] = '\0';
  sp_buf[0] = '\0';

  pCurrency = findSumCurrency(g_45, ALL_CUSTOMERS_SUMMARY_AMOUNT_STR); 
  if (pCurrency)
    {
      strncpy(sc_buf, pCurrency, sc_buf_len);
    }
  else
    {
      return FALSE;
    }

  pAmount = findSumAmount(g_45, ALL_CUSTOMERS_SUMMARY_AMOUNT_STR);
  if (pAmount)
    {
      strncpy(sp_buf, pAmount, sp_buf_len);
    }
  else
    {
      return FALSE;
    }

  return TRUE;
}





