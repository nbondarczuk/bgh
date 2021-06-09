/**************************************************************************/
/*  MODULE : BALANCE data fetecher                                        */
/*                                                                        */
/*  AUTHOR : N.Bondarczuk                                                 */
/*                                                                        */
/*  CREATION DATE : 22.10.97                                              */
/*                                                                        */
/*  DESCRIPTION : Exports  functions accessing information from TIMM      */
/*                BAL SHEET message.                                      */ 
/**************************************************************************/
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "bgh.h"
#include "protos.h"
#include "parser.h"

#include "types.h"
#include "gen.h"
#include "inv_item.h"
#include "inv_types.h"
#include "inv_fetch.h"

#if 0	/* just for version.sh */
static char *SCCS_VERSION = "4.1";
#endif

/*
 * INTERNAL
 */

struct s_group_22 *findSegment(struct s_TimmInter *sum_ti, int n) 
{
  int i;
  struct s_group_22 *g_22;
  struct s_lin_seg *lin;
  
  g_22 = sum_ti->timm->g_22;         /* from the beginning */
  i = 0;
  while (g_22) 
    {
      lin = g_22->lin;
      if (lin && lin->v_1222)        /* is it correct TIMM structure */
        {                 
          if (EQ(lin->v_1222, "01")) /* we are on 01 level */
            {
              if (i == n)            /* that is this LIN block so let's break */ 
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

struct s_moa_seg *findMonetaryAmount(struct s_group_23 *g_23, char *amount_type) 
{
  struct s_moa_seg *moa;  
  
  while (g_23) 
    {
      moa = g_23->moa;
      if (EQ(moa->v_5025, "960")) 
        {
          return moa;
        }

      g_23 = g_23->g_23_next;
    }

  return NULL;
}

#define TRANSX_TAB_SIZE 17

char *tTransactions[TRANSX_TAB_SIZE] = 
{
  "PAYMN",
  "WROFF",
  "OINVC",
  "ADJST",
  "OVPMT",
  "CRDNO",
  "REFPM",
  "COACC",
  "INTRO",
  "INTRI",
  "PMTRO",
  "PMTRI",
  "CRMEM",
  "BOUNC",

  /*
   * New BGH trasactions
   */

  "COINV", /* Credit Invoice */
  "NLINV", /* Interest Note Last */
  "NPINV"  /* Interest Note Periodic */
};


int is_transaction(char *str) 
{
  int i;
  
  for (i = 0; i < TRANSX_TAB_SIZE; i++)
    {
      if (EQ(tTransactions[i], str))
        {
          return TRUE;
        }
    }

  return FALSE;
}


struct s_imd_seg *findTransactionInfo(struct s_imd_seg *imd) 
{
  while (imd) 
    {
      if (is_transaction(imd->v_7009)) 
        {
          return imd;
        }

      imd = imd->imd_next;
    }
  
  return NULL;
}

struct s_dtm_seg *findTransactionDate(struct s_dtm_seg *dtm) 
{
  while (dtm) 
    {
      if (EQ(dtm->v_2005, "900")) 
        {
          return dtm;
        }

      dtm = dtm->dtm_next;
  }
  
  return NULL;
}

/**************************************************************************/
/*  FUNCTION : fetchBalanceTransactionsNumber                             */
/*                                                                        */
/*  ARGUMENTS : IN sum_ti - BALANCE TIMM structure                        */
/*                                                                        */
/*  RETURN : number of subscribers in SUM SHEET msg or -1 value if error  */
/*           found in s_TimmInter structure                               */
/*                                                                        */
/*  DESCRIPTION : Returns the number of LIN blocks with nesting level 01  */
/*                in field LIN.V_1222.                                    */
/**************************************************************************/

int fetchBalanceTransactionsNumber(struct s_TimmInter *bal_ti) 
{
  int i;
  struct s_group_22 *g_22;             
  struct s_lin_seg *lin;
  
  g_22 = bal_ti->timm->g_22;
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
      
      g_22 = g_22->g_22_next;
    }
  
  return i;
}


/**************************************************************************/
/*  FUNCTION : fetchBalanceTransactionType                                */
/*                                                                        */
/*  ARGUMENTS : IN sum_ti - BALANCE TIMM structure                        */
/*                                                                        */
/**************************************************************************/

int fetchBalanceTransactionType(struct s_TimmInter *bal_ti, int index, char *ttype_buf, int ttype_buf_len) 
{
  struct s_group_22 *g_22;             
  struct s_imd_seg *imd;
  
  g_22 = findSegment(bal_ti, index);
  if (g_22) 
    {
      imd = findTransactionInfo(g_22->imd);
      if (imd) 
        {
          strncpy(ttype_buf, imd->v_7009, ttype_buf_len);
          return TRUE;
        }
    }

  return FALSE;
}

int fetchBalanceTransactionInvoice(struct s_TimmInter *bal_ti, int index, char *inv_buf, int inv_buf_len) 
{
  struct s_group_22 *g_22;             
  struct s_group_26 *g_26;             
  struct s_rff_seg *rff;
  
  g_22 = findSegment(bal_ti, index);
  if (g_22) 
    {
      g_26 = g_22->g_26;
      while (g_26)
        {
          rff = g_26->rff;
          if (EQ(rff->v_1153, "RF"))
            {
              strncpy(inv_buf, rff->v_1154, inv_buf_len);
              return TRUE;
            }

          g_26 = g_26->g_26_next;
        }
    }

  return FALSE;
}



/**************************************************************************/
/*  FUNCTION : fetchBalanceTransactionDate                                */
/*                                                                        */
/*  ARGUMENTS : IN sum_ti - BALANCE TIMM structure                        */
/*                                                                        */
/**************************************************************************/

int fetchBalanceTransactionDate(struct s_TimmInter *bal_ti, int index, char *tdate_buf, int tdate_buf_len) 
{
  struct s_group_22 *g_22;             
  struct s_dtm_seg *dtm;
  
  g_22 = findSegment(bal_ti, index);
  if (g_22) 
    {
      dtm = findTransactionDate(g_22->dtm);
      if (dtm && EQ(dtm->v_2005, "900")) 
        {
          strncpy(tdate_buf, dtm->v_2380, tdate_buf_len);
          return TRUE;
        }
    }
  
  return FALSE;
}


/**************************************************************************/
/*  FUNCTION : fetchBalanceTransactionCurrency                            */
/*                                                                        */
/*  ARGUMENTS : IN sum_ti - BALANCE TIMM structure                        */
/*                                                                        */
/**************************************************************************/

int fetchBalanceTransactionCurrency(struct s_TimmInter *bal_ti, int index, char *tcurrency_buf, int tcurrency_buf_len) 
{
  struct s_group_22 *g_22;
  struct s_group_23 *g_23;
  struct s_moa_seg *moa;  
  
  g_22 = findSegment(bal_ti, index);
  if (g_22) 
    {
      g_23 = g_22->g_23;
      moa = findMonetaryAmount(g_23, "960");
      if (moa) 
        {
          strncpy(tcurrency_buf, moa->v_6345, tcurrency_buf_len);
          return TRUE;
        }
    }

  return FALSE;
}

/**************************************************************************/
/*  FUNCTION : fetchBalanceTransactionAmount                              */
/*                                                                        */
/*  ARGUMENTS : IN sum_ti - BALANCE TIMM structure                        */
/*                                                                        */
/**************************************************************************/

int fetchBalanceTransactionAmount(struct s_TimmInter *bal_ti, int index, char *tamount_buf, int tamount_buf_len) 
{
  struct s_group_22 *g_22;
  struct s_group_23 *g_23;
  struct s_moa_seg *moa;
  
  g_22 = findSegment(bal_ti, index);
  if (g_22) 
    {
      g_23 = g_22->g_23;
      moa = findMonetaryAmount(g_23, "960");
      if (moa) 
        {
          strncpy(tamount_buf, moa->v_5004, tamount_buf_len);
          return TRUE;
        }
    }

  return FALSE;
}

int fetchSubscriberAccessPayment( struct s_TimmInter *sum_ti, 
                                  char *access_currency,  int access_currency_len, 
                                  char *access_payment,  int access_payment_len) 
{
  struct s_moa_seg *moa;  
  struct s_group_45 *g_45;
  
  g_45 = sum_ti->timm->g_45;
  while (g_45) 
    {
      moa = g_45->moa;
      if (EQ(moa->v_5025, "911")) 
        {
          strcpy(access_currency, moa->v_6345);
          strcpy(access_payment, moa->v_5004);
          return TRUE;
        }

      g_45 = g_45->g_45_next;
    }

  return FALSE;
}

int fetchSubscriberUsagePayment( struct s_TimmInter *sum_ti, 
                                 char *usage_currency,  int usage_currency_len, 
                                 char *usage_payment,  int usage_payment_len) 
{
  struct s_moa_seg *moa;  
  struct s_group_45 *g_45;
  
  g_45 = sum_ti->timm->g_45;
  while (g_45) 
    {
      moa = g_45->moa;
      if (EQ(moa->v_5025, "912")) 
        {
          strcpy(usage_currency, moa->v_6345);
          strcpy(usage_payment, moa->v_5004);
          return TRUE;
        }
      
      g_45 = g_45->g_45_next;
    }

  return FALSE;
}

int fetchSubscriberSubscriptionPayment( struct s_TimmInter *sum_ti, 
                                        char *services_currency,  int services_currency_len, 
                                        char *services_payment,  int services_payment_len) 
{
  struct s_moa_seg *moa;  
  struct s_group_45 *g_45;

  g_45 = sum_ti->timm->g_45;
  while (g_45) 
    {
      moa = g_45->moa;
      if (EQ(moa->v_5025, "910")) 
        {
          strcpy(services_currency, moa->v_6345);
          strcpy(services_payment, moa->v_5004);
          return TRUE;
        }

      g_45 = g_45->g_45_next;
    }
  
  return FALSE;
}

int fetchSubscriberOthersPayment( struct s_TimmInter *sum_ti, 
                                  char *others_currency,  int others_currency_len, 
                                  char *others_payment,  int others_payment_len) 
{
  struct s_moa_seg *moa;  
  struct s_group_45 *g_45;
  
  g_45 = sum_ti->timm->g_45;
  while (g_45) 
    {
      moa = g_45->moa;
      if (EQ(moa->v_5025, "910")) 
        {
          strcpy(others_currency, moa->v_6345);
          strcpy(others_payment, moa->v_5004);
          return TRUE;
        }

      g_45 = g_45->g_45_next;
    }
  
  return FALSE;
}

int fetchSubscriberSummaryPayment( struct s_TimmInter *sum_ti, 
                                   char *summary_currency,  int summary_currency_len, 
                                   char *summary_payment,  int summary_payment_len) 
{
  struct s_moa_seg *moa;  
  struct s_group_45 *g_45;
  
  g_45 = sum_ti->timm->g_45;
  while (g_45) 
    {
      moa = g_45->moa;
      if (EQ(moa->v_5025, "77")) 
        {
          strcpy(summary_currency, moa->v_6345);
          strcpy(summary_payment, moa->v_5004);
          return TRUE;
        }
      
      g_45 = g_45->g_45_next;
    }

  return FALSE;

}


int fetchAccountPreviousSaldo( struct s_TimmInter *bal_ti, 
                               char *currency,  int currency_len, 
                               char *payment,  int payment_len) 
{
  struct s_moa_seg *moa;  
  struct s_group_45 *g_45;

  g_45 = bal_ti->timm->g_45;
  while (g_45) 
    {
      moa = g_45->moa;
      if (EQ(moa->v_5025, "961")) 
        {
          strcpy(currency, moa->v_6345);
          strcpy(payment, moa->v_5004);
          return TRUE;
        }
      
      g_45 = g_45->g_45_next;
    }
  
  return FALSE;

}


int fetchAccountActualSaldo( struct s_TimmInter *bal_ti, 
                             char *currency, IN int currency_len, 
                             char *payment, IN int payment_len) 
{
  struct s_moa_seg *moa;  
  struct s_group_45 *g_45;

  g_45 = bal_ti->timm->g_45;
  while (g_45) 
    {
      moa = g_45->moa;
      if (EQ(moa->v_5025, "969")) 
        {
          strcpy(currency, moa->v_6345);
          strcpy(payment, moa->v_5004);
          return TRUE;
        }
      
      g_45 = g_45->g_45_next;
    }
  
  return FALSE;

}


int fetchBalanceInvoicePayment( struct s_TimmInter *bal_ti, 
                                char *currency,  int currency_len, 
                                char *payment,  int payment_len) 
{
  struct s_moa_seg *moa;  
  struct s_group_45 *g_45;
  
  g_45 = bal_ti->timm->g_45;
  while (g_45) 
    {
      moa = g_45->moa;
      if (EQ(moa->v_5025, "967")) 
        {
          strcpy(currency, moa->v_6345);
          strcpy(payment, moa->v_5004);
          return TRUE;
        }

      g_45 = g_45->g_45_next;
    }

  return FALSE;
}





