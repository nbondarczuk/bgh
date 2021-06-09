/**************************************************************************************************
 *                                                                                          
 * MODULE: SUM_FETCH_SIM
 *                                                                                          
 * AUTHOR: N.Bondarczuk                                                
 * 
 * CREATION DATE : 06.11.97                                              
 *
 * DESCRIPTION: Contains functions used by the module SIM_ENC_GEN in order to generate invoice 
 *              enclosure. The most important key information for access to call descriptions
 *              is SIM number. It is given by one function  and afterward user by other access functions.
 *
 **************************************************************************************************
 */

#include <stdio.h>
#include <string.h>
#include "bgh.h"
#include "protos.h"
#include "parser.h"

#include "types.h"
#include "gen.h"
#include "sum_fetch_sim.h"
#include "fnmatch.h"
#include "unit_price.h"
#include "timm.h"

#if 0   /* just for version.sh */
static char *SCCS_VERSION = "4.2.5";
#endif

/*
 * 28.01.98 N.Bondarczuk : Mapping shdes to des in interfece structures connecting FETCH and GEN modules 
 * 28.01.98 N.Bondarczuk : Additional 3 fields for CallItem
 * 28.01.98 N.Bondarczuk : Suppress zero bill item generatior policy added
 * 04.02.98 N.Bondarczuk : Error found in serching call items for a contract, v.1.2 -> v.1.2.1
 * 04.02.98 N.Bondarczuk : Added handling of treshold value for XCD records, v.1.2.1 -> v.1.2.2
 * 16.02.98 N.Bondarczuk : Use rounded call volume before discounting/price plans for X009 call volume
 *                         if FUM applied (X021 = F, P), v.1.2.3 -> v.1.2.4
 * 20.02.98 N.Bondarczuk : Recount unit price if Flat rating used
 * 23.07.99 L.browski    : unconditional generating detailed list of calls 
 */


extern stBGHGLOB       stBgh;                  /* structure with globals for BGH */

static char szTemp[64];

toenBool soenIsChargeType(struct s_lin_seg *, toenRecordType);

struct s_group_2 *lpstFindInvoiceeBlock(struct s_group_2 *);

struct s_moa_seg *lpstFindPaymentSegment(struct s_group_23 *, char *);

struct s_group_22 *lpstFindChargeSegment(struct s_group_22 *, toenItemType); 

struct s_group_22 *lpstFindSimSegment(struct s_group_22 *, char *);

struct s_group_22 *lpstFindSubscriberSegment(struct s_group_22 *, int);  

struct s_group_22 *lpstFindSubscriberContractSegment(struct s_group_22 *, int);

struct s_imd_seg *lpstFindItemDescription(struct s_imd_seg *, char *);

struct s_rff_seg *lpstFindReference(struct s_group_3 *, char*);

char *lpchzGetField(int, char *);

struct s_qty_seg *lpstFindQuantity(struct s_qty_seg *, char *, char *);

struct s_pia_seg *lpstFindProductId(struct s_pia_seg *, toenRecordType);

double foflFindThresholdValue(struct s_group_22 *);

/**************************************************************************************************
 *                                                                                          
 * FUNCTION : foenFetchInvoiceDate             
 *                                                                                          
 * LOCAL FUNCTIONS USED:                                                                    
 *                                                                                          
 * EXTERNAL FUNCTIONS USED:                                                                 
 * MODULE: SUM_FETCH                                                                        
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenFetchInvoiceDate(struct s_TimmInter *ppstSumSheet, char *poszDate, int poiDateBufferLength) 
{
  struct s_dtm_seg *dtm;  

  fovdPushFunctionName("foenFetchInvoiceDate");

  dtm = ppstSumSheet->timm->dtm;  
  while (dtm) 
    {
      if (EQ(dtm->v_2005, "3")) 
        {
          strncpy(poszDate, dtm->v_2380, poiDateBufferLength);
          fovdPopFunctionName();
          return TRUE;
        }
      else
        {
          dtm = dtm->dtm_next;
        }
    }

  fovdPopFunctionName();
  return FALSE;
}

/**************************************************************************************************
 *                                                                                          
 * FUNCTION : foenFetchInvoiceCustomerAccountNo             
 *                                                                                          
 * LOCAL FUNCTIONS USED:                                                                    
 * struct s_group_2 *lpstFindInvoiceeBlock(struct s_group_2 *);
 *                                                                                          
 * EXTERNAL FUNCTIONS USED:                                                                 
 * MODULE: SUM_FETCH                                                                        
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenFetchInvoiceCustomerAccountNo(struct s_TimmInter *ppstSumSheet, 
					   char *poszAccountNo, 
					   int poiAccountNoBufferLength) 
{
  struct s_group_2 *g_2;
  struct s_rff_seg *rff;
  struct s_group_2 *lpstFindInvoiceeBlock(struct s_group_2 *);

  fovdPushFunctionName("foenFetchInvoiceCustomerAccountNo");

  g_2 = lpstFindInvoiceeBlock(ppstSumSheet->timm->g_2);
  if (g_2 == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }
  
  rff = lpstFindReference(g_2->g_3, "IT");
  if (rff == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }
  
  strncpy(poszAccountNo, rff->v_1154, poiAccountNoBufferLength);

  fovdPopFunctionName();
  return TRUE;
}

/**************************************************************************************************
 *                                                                                          
 * FUNCTION : foenFetchInvoiceNo             
 *                                                                                          
 * LOCAL FUNCTIONS USED:                                                                    
 * struct s_group_2 *lpstFindInvoiceeBlock(struct s_group_2 *);
 *                                                                                          
 * EXTERNAL FUNCTIONS USED:                                                                 
 * MODULE: SUM_FETCH                                                                        
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenFetchInvoiceNo(struct s_TimmInter *ppstSumSheet, 
			    char *poszInvoiceNo, 
			    int poiInvoiceNoBufferLength) 
{
  struct s_group_2 *g_2;
  struct s_rff_seg *rff;
  struct s_group_2 *lpstFindInvoiceeBlock(struct s_group_2 *);

  fovdPushFunctionName("foenFetchInvoiceNo");

  g_2 = lpstFindInvoiceeBlock(ppstSumSheet->timm->g_2);
  if (g_2 == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }
  
  rff = lpstFindReference(g_2->g_3, "IV");
  if (rff == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }
  
  strncpy(poszInvoiceNo, rff->v_1154, poiInvoiceNoBufferLength);

  fovdPopFunctionName();
  return TRUE;
}

/**************************************************************************************************
 *                                                                                          
 * FUNCTION : foenFetchInvoicePeriodBegin             
 *                                                                                          
 * LOCAL FUNCTIONS USED:                                                                    
 *                                                                                          
 * EXTERNAL FUNCTIONS USED:                                                                 
 * MODULE: SUM_FETCH                                                                        
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenFetchInvoicePeriodBegin(struct s_TimmInter *ppstSumSheet, 
				     char *poszPeriodBegin, 
				     int poiBufferLen) 
{
  struct s_dtm_seg *dtm;

  fovdPushFunctionName("foenFetchInvoicePeriodBegin");

  dtm = ppstSumSheet->timm->dtm;
  while (dtm)
    {
      if (EQ(dtm->v_2005, "167")) 
        {
          strncpy(poszPeriodBegin, dtm->v_2380, poiBufferLen);
          fovdPopFunctionName();
          return TRUE;
        }
      
      dtm = dtm->dtm_next;
    }

  fovdPopFunctionName();
  return FALSE;
}


/**************************************************************************************************
 *                                                                                          
 * FUNCTION : foenFetchInvoicePeriodEnd             
 *                                                                                          
 * LOCAL FUNCTIONS USED:                                                                    
 *                                                                                          
 * EXTERNAL FUNCTIONS USED:                                                                 
 * MODULE: SUM_FETCH                                                                        
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenFetchInvicePeriodEnd(struct s_TimmInter *ppstSumSheet, 
				  char *poszPeriodEnd, 
				  int poiBufferLen) 
{
  struct s_dtm_seg *dtm;

  fovdPushFunctionName("foenFetchInvoicePeriodEnd");

  dtm = ppstSumSheet->timm->dtm;
  while (dtm)
    {
      if (EQ(dtm->v_2005, "168")) 
        {
          strncpy(poszPeriodEnd, dtm->v_2380, poiBufferLen);
          fovdPopFunctionName();
          return TRUE;
        }
      
      dtm = dtm->dtm_next;
    }
  
  fovdPopFunctionName();
  return FALSE;
}

/**************************************************************************************************
 *                                                                                          
 * FUNCTION :  foiCountSubscribers             
 *                                                                                          
 * LOCAL FUNCTIONS USED:                                                                    
 *                                                                                          
 * EXTERNAL FUNCTIONS USED:                                                                 
 * MODULE: SUM_FETCH                                                                        
 *                                                                                          
 **************************************************************************************************
 */

int foiCountSubscribers(struct s_TimmInter *spstSumSheet) 
{
  int loiSubscribersNo;
  struct s_group_22 *g_22;             
  struct s_lin_seg *lin;

  fovdPushFunctionName("foiCountSubsribers");

  g_22 = spstSumSheet->timm->g_22;
  loiSubscribersNo = 0;
  while (g_22) 
    {
      lin = g_22->lin;
      if (lin && lin->v_1222 && EQ(lin->v_1222, "01"))
        {
          loiSubscribersNo++;
        }
      g_22 = g_22->g_22_next;
    }  

  fovdPopFunctionName();
  return loiSubscribersNo;
}



/**************************************************************************************************
 *                                                                                          
 * FUNCTION :  foiCountSubscriberContracts             
 *                                                                                          
 * LOCAL FUNCTIONS USED:                                                                    
 * struct s_group_22 *lpstFindSubscriberSegment(struct s_group_22 *, int);  
 *                                                                                          
 * EXTERNAL FUNCTIONS USED:                                                                 
 * MODULE: SUM_FETCH                                                                        
 *                                                                                          
 **************************************************************************************************
 */

int foiCountSubscriberContracts(struct s_TimmInter *spstSumSheet, 
                                int poiSubscriberNo) 
{
  struct s_group_22 *g_22;
  struct s_lin_seg *lin;  
  int loiContractsNo;
  
  fovdPushFunctionName("foiCountScubsriberContracts");

  g_22 = lpstFindSubscriberSegment(spstSumSheet->timm->g_22, poiSubscriberNo);  
  if (g_22 == NULL)
    {
      return 0;
    }

  loiContractsNo = 0;
  g_22 = g_22->g_22_next;
  while (g_22) 
    {
      lin = g_22->lin;
      if (lin && lin->v_1222 && EQ(lin->v_1222, "01")) 
        {
          break;
        }

      if (lin && lin->v_1222 && EQ(lin->v_1222, "02") && lin->v_7140 && NOT(EQ(lin->v_7140, "O")))
        {          
          loiContractsNo++;
        }

      g_22 = g_22->g_22_next;
    }  
  
  fovdPopFunctionName();
  return loiContractsNo;
}


toenBool foenFetchCoInfo(struct s_TimmInter *ppstSumSheet,
                         int poiSubNo,
                         int poiCoNo,
                         struct tostCoInfo *ppstCoInfo)
{
  struct s_group_22 *g_22;
  struct s_group_23 *g_23;
  struct s_group_99 *g_99;  
  struct s_imd_seg *imd;  
  struct s_moa_seg *moa;  
  
  g_22 = lpstFindSubscriberSegment(ppstSumSheet->timm->g_22, poiSubNo);  
  if (g_22 == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }
  
  g_22 = lpstFindSubscriberContractSegment(g_22->g_22_next, poiCoNo);
  if (g_22 == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }

  memset(ppstCoInfo, 0x00, sizeof(struct tostCoInfo));

  ppstCoInfo->g_22 = g_22;

  /* Market */
  /* Market Code */
  imd = lpstFindItemDescription(g_22->imd, "MRKT");
  if (imd == NULL) 
    {
      fovdPopFunctionName();
      return FALSE;
    }
  else
    {
      strncpy(ppstCoInfo->saszMarketCode, 
              imd->v_7008, 
              MAX_BUFFER - 1);
      
      strncpy(ppstCoInfo->saszMarket, 
              imd->v_7008a, 
              MAX_BUFFER - 1);
    }
  
  /* Contract Number */
  imd = lpstFindItemDescription(g_22->imd, "CO");
  if (imd == NULL) 
    {
      fovdPopFunctionName();
      return FALSE;
    }
  else
    {
      strncpy(ppstCoInfo->saszCoNo, 
              imd->v_7008a, 
              MAX_BUFFER - 1);
    }

  /* Storage Medium Number */
  imd = lpstFindItemDescription(g_22->imd, "SMNUM");
  if (imd == NULL) 
    {
      fovdPopFunctionName();
      return FALSE;
    }
  else
    {
      strncpy(ppstCoInfo->saszSmNo, 
              imd->v_7008a, 
              MAX_BUFFER - 1);
    }

  /* Directory Number */
  imd = lpstFindItemDescription(g_22->imd, "DNNUM");
  if (imd == NULL) 
    {
      fovdPopFunctionName();
      return FALSE;
    }
  else
    {
      strncpy(ppstCoInfo->saszDnNo, 
              imd->v_7008a, 
              MAX_BUFFER - 1);
    }

  /* Summary contract charges */
  g_23 = g_22->g_23;
  moa = lpstFindPaymentSegment(g_23, "931");
  if (moa == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }
  else
    {
      strncpy(ppstCoInfo->saszSumAmt,
              moa->v_5004, 
              MAX_BUFFER - 1);
      
      strncpy(ppstCoInfo->saszCurrency, 
              moa->v_6345, 
              MAX_BUFFER - 1);
    }

  /*
   * G22 segment on level 03 with 
   * - IMD block empty 
   * - MOA+903 in group G23
   * - RFF+IC in group G23
   * - G99 not empty
   * Let's find any block with G99 not equal NULL
   */

  g_99 = NULL;
  g_22 = g_22->g_22_next;
  while (g_22)
    {
      if (EQ(g_22->lin->v_1222, "02") || EQ(g_22->lin->v_1222, "01"))
        {
          /* Next contract or customer */          
          break;
        }     
      else if (g_22->g_99 != NULL)
        {
          stBgh.sofoThresholdValue = foflFindThresholdValue(g_22);          
          ppstCoInfo->g_99 = g_22->g_99;
          break;
        }
      else
        {}
      
      g_22 = g_22->g_22_next;
    }

  

  return TRUE;
}


/**************************************************************************************************
 *                                                                                          
 * FUNCTION :  foenFetchContractSim
 *                                                                                          
 * LOCAL FUNCTIONS USED:                                                                    
 * struct s_group_22 *lpstFindSubscriberSegment(struct s_group_22 *, int);  
 * struct s_group_22 *lpstFindSubscriberContractSegment(struct s_group_22 *, int);
 * struct s_lin_seg *lpstFindItemDescription(struct s_lin_seg *, char *);
 *                                                                                         
 * EXTERNAL FUNCTIONS USED:                                                                 
 * MODULE: SUM_FETCH                                                                        
 *                                                                                          
 **************************************************************************************************
 */



toenBool foenFetchContractSim(struct s_TimmInter *ppstSumSheet, 
                              int poiSubscriberNo, 
                              int poiContractNo, 
                              char *loszSim, 
                              int loszSimBufferLength) 
{
  struct s_group_22 *g_22;
  struct s_lin_seg *lin;  
  struct s_imd_seg *imd;  
  int loiContractsNo;

  fovdPushFunctionName("foenFetchcontractSim");

  g_22 = lpstFindSubscriberSegment(ppstSumSheet->timm->g_22, poiSubscriberNo);  
  if (g_22 == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }

  g_22 = lpstFindSubscriberContractSegment(g_22->g_22_next, poiContractNo);
  if (g_22 == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }

  imd = lpstFindItemDescription(g_22->imd, "SMNUM");
  if (imd == NULL) 
    {
      fovdPopFunctionName();
      return FALSE;
    }

  strncpy(loszSim, imd->v_7008a, loszSimBufferLength);

  fovdPopFunctionName();
  return TRUE;
}

toenBool foenFetchContractMarket(struct s_TimmInter *ppstSumSheet, 
                                 int poiSubscriberNo, 
                                 int poiContractNo, 
                                 char *loszMarket, 
                                 int loszMarketBufferLength) 
{
  struct s_group_22 *g_22;
  struct s_lin_seg *lin;  
  struct s_imd_seg *imd;  
  int loiContractsNo;

  fovdPushFunctionName("foenFetchContractMarket");

  g_22 = lpstFindSubscriberSegment(ppstSumSheet->timm->g_22, poiSubscriberNo);  
  if (g_22 == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }

  g_22 = lpstFindSubscriberContractSegment(g_22->g_22_next, poiContractNo);
  if (g_22 == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }
  
  imd = lpstFindItemDescription(g_22->imd, "MRKT");
  if (imd == NULL) 
    {
      fovdPopFunctionName();
      return FALSE;
    }

  strncpy(loszMarket, imd->v_7008a, loszMarketBufferLength);
  
  fovdPopFunctionName();
  return TRUE;
}

/**************************************************************************************************
 *                                                                                          
 * FUNCTION :  foenFetchContractNumber
 *                                                                                          
 * LOCAL FUNCTIONS USED:                                                                    
 * struct s_group_22 *lpstFindSubscriberSegment(struct s_group_22 *, int);  
 * struct s_group_22 *lpstFindSubscriberContractSegment(struct s_group_22 *, int);
 * struct s_lin_seg *lpstFindItemDescription(struct s_lin_seg *, char *);
 *                                                                                         
 * EXTERNAL FUNCTIONS USED:                                                                 
 * MODULE: SUM_FETCH                                                                        
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenFetchContractNumber(struct s_TimmInter *ppstSumSheet, 
                                 int poiSubscriberNo, 
                                 int poiContractNo, 
                                 char *loszNumber, 
                                 int loszNumberBufferLength,
                                 int *ppiCoId) 
{
  struct s_group_22 *g_22;
  struct s_lin_seg *lin;  
  struct s_imd_seg *imd;  
  int loiContractsNo;
  int rc = 0;

  fovdPushFunctionName("foenFetchContractNumber");

  g_22 = lpstFindSubscriberSegment(ppstSumSheet->timm->g_22, poiSubscriberNo);  
  if (g_22 == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }
  
  g_22 = lpstFindSubscriberContractSegment(g_22->g_22_next, poiContractNo);
  if (g_22 == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }

  imd = lpstFindItemDescription(g_22->imd, "DNNUM");
  if (imd == NULL) 
    {
      fovdPopFunctionName();
      return FALSE;
    }

  strncpy(loszNumber, imd->v_7008a, loszNumberBufferLength);

  imd = lpstFindItemDescription(g_22->imd, "CO");
  if (imd == NULL) 
    {
      fovdPopFunctionName();
      return FALSE;
    }
  
  if ((rc = sscanf(imd->v_7008a, "%d", ppiCoId)) <= 0)
    {
      fovdPopFunctionName();
      return FALSE;
    }
  
  fovdPopFunctionName();
  return TRUE;
}

/**************************************************************************************************
 *                                                                                          
 * FUNCTION : toenBool foenFetchSimPayment
 *                                                                                          
 * LOCAL FUNCTIONS USED:                                                                    
 * struct s_group_22 *lpstFindPaymentSegment(struct s_moa_seg *, char *);
 * struct s_group_22 *lpstFindSimSegment(struct s_group_22 *, char *);
 *                                                                                          
 * EXTERNAL FUNCTIONS USED:                                                                 
 * MODULE: SUM_FETCH                                                                        
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenFetchSimPayment(struct s_TimmInter *ppstSumSheet, char *poszSim, tostPayment *postPayment) 
{
  struct s_group_22 *g_22;
  struct s_group_23 *g_23;  
  struct s_moa_seg *moa;

  fovdPushFunctionName("foenFetchSimPayment");

  g_22 = lpstFindSimSegment(ppstSumSheet->timm->g_22, poszSim);
  if (g_22 == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }

  g_23 = g_22->g_23;
  moa = lpstFindPaymentSegment(g_23, "931");
  if (moa == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }

  strncpy(postPayment->sasnzMonetaryAmount, moa->v_5004, MAX_BUFFER);
  strncpy(postPayment->sasnzCurrency, moa->v_6345, MAX_BUFFER);

  fovdPopFunctionName();
  return TRUE;
}


/**************************************************************************************************
 *                                                                                          
 * FUNCTION :  foenFetchSimSummaryRecord
 *                                                                                          
 * LOCAL FUNCTIONS USED:                                                                    
 * struct s_group_22 *lpstFindPaymentSegment(struct s_moa_seg *, char *);
 * struct s_group_22 *lpstFindChargeSegment(struct s_group_22, toenRecordType); 
 * struct s_moa_seg *lpstFindPaymentSegment(struct s_moa_seg *, char *);
 *
 * EXTERNAL FUNCTIONS USED:                                                                 
 * MODULE:
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenFetchCoServCatPayment(struct s_TimmInter *ppstSumSheet, 
                                   struct tostCoInfo *ppstCoInfo,
                                   enum toenItemType poenType, 
                                   struct tostPayment *ppstPayment) 
{
  struct s_group_22 *g_22;
  struct s_moa_seg *moa;  

  g_22 = lpstFindChargeSegment(ppstCoInfo->g_22->g_22_next, poenType);
  if (g_22 == NULL)
    {
      return FALSE;
    }
  else
    {      
      moa = lpstFindPaymentSegment(g_22->g_23, "932");
      if (moa == NULL)
        {
          return FALSE;
        }
      else
        {
          strncpy(ppstPayment->sasnzMonetaryAmount, 
                  moa->v_5004, 
                  MAX_BUFFER - 1);
          
          strncpy(ppstPayment->sasnzCurrency, 
                  moa->v_6345, 
                  MAX_BUFFER - 1);
        }
    }

  return TRUE;
}


/**************************************************************************************************
 *                                                                                          
 * FUNCTION: fpstFindSimCallList 
 *                                                                                          
 * DESCRIPTION:
 *
 * For a given SIM no find a contract seq. Starting from first G22 item on the contract level 
 * of SUM-SHEET try to firn G22 item wth not NULL G99 group. If found G99 group check if 
 * treshold value is gr than 0.0. If it is then return NULL meaning - no call list found.
 *                                                                                          
 **************************************************************************************************
 */

struct s_group_99 *fpstFindSimCallList(struct s_TimmInter *ppstSumSheet, char *ppsnzSim) 
{
  struct s_group_22 *lpstG22;
  struct s_group_23 *lpstG23;
  struct s_group_99 *lpstG99, *lpstG99Seq;  
  struct s_lin_seg *lpstLin;
/*  double loflThresholdValue; */

  fovdPushFunctionName ("fpstFindSimCallList");

  /*
   * G22 segment on level 02
   */

  if ((lpstG22 = lpstFindSimSegment(ppstSumSheet->timm->g_22, ppsnzSim)) == NULL)
    {
      fovdPopFunctionName ();
      return FALSE;
    }
  
  /*
   * G22 segment on level 03 with 
   * - IMD block empty 
   * - MOA+903 in group G23
   * - RFF+IC in group G23
   * - G99 not empty
   * Let's find any block with G99 not equal NULL
   */

  lpstG99 = NULL;
  lpstG22 = lpstG22->g_22_next;
  while (lpstG22)
    {
      lpstLin = lpstG22->lin;
      
      if (EQ(lpstLin->v_1222, "02") || EQ(lpstLin->v_1222, "01"))
        {
          /*
           * Next contract or customer
           */
          
          break;
        }

      if (lpstG22->g_99 != NULL)
        {
          stBgh.sofoThresholdValue = foflFindThresholdValue(lpstG22);
/*
		  if (loflThresholdValue != 0.0)
            {
              lpstG99 = NULL;
            }
          else
            {
              lpstG99 = lpstG22->g_99;
            }
*/
          
          lpstG99 = lpstG22->g_99;
          
          break;
        }
      
      lpstG22 = lpstG22->g_22_next;
    }
  
  fovdPopFunctionName ();
  return lpstG99;
}


/**************************************************************************************************
 *                                                                                          
 * FUNCTION : foiCountSims
 *                                                                                          
 * LOCAL FUNCTIONS USED:                                                                    
 *                                                                                          
 * EXTERNAL FUNCTIONS USED:                                                                 
 * MODULE:
 *                                                                                          
 **************************************************************************************************
 */


int foiCountSims(struct s_TimmInter *ppstSumSheet)  
{
  struct s_group_22 *lpstG22;  
  struct s_lin_seg *lpstLin;
  struct s_imd_seg *lpstImd;
  int loiSimsNo;
  
  loiSimsNo = 0;
  lpstG22 = ppstSumSheet->timm->g_22;
  while (lpstG22)
    {
      lpstLin = lpstG22->lin;
      if (EQ(lpstLin->v_1222, "02"))
        {
          lpstImd = lpstFindItemDescription(lpstG22->imd, "SMNUM");          
          if (lpstImd != NULL)
            {
              loiSimsNo++;
            }
        }  
      lpstG22 = lpstG22->g_22_next;
    }
  
  return loiSimsNo;
}

/**************************************************************************************************
 *                                                                                          
 * FUNCTION :
 *                                                                                          
 * LOCAL FUNCTIONS USED:                                                                    
 *                                                                                          
 * EXTERNAL FUNCTIONS USED:                                                                 
 * MODULE:
 *                                                                                          
 **************************************************************************************************
 */

struct s_group_2 *lpstFindInvoiceeBlock(struct s_group_2 *ppstG2) 
{
  struct s_group_2 *lpstG2;
  struct s_nad_seg *nad;

  lpstG2 = ppstG2;
  while (lpstG2) 
    {
      nad = lpstG2->nad;
      if (EQ(nad->v_3035, "IV"))
        {
          return lpstG2;
        }
      lpstG2 = lpstG2->g_2_next;
    }

  return NULL;
}

struct s_moa_seg *lpstFindPaymentSegment(struct s_group_23 *ppstG23, char *ppchzType) 
{
  struct s_moa_seg *lpstMoa;
  struct s_group_23 *lpstG23;
  
  lpstG23 = ppstG23;
  while (lpstG23)
    {
      lpstMoa = lpstG23->moa;
      if (EQ(lpstMoa->v_5025, ppchzType))
        {
          return lpstMoa;
        }
      lpstG23 = lpstG23->g_23_next;
    }

  return NULL;
}

char *spszPatternTab[] = {"U", "A", "S", "O"};

struct s_group_22 *lpstFindChargeSegment(struct s_group_22 *ppstG22, toenItemType poenType) 
{
  struct s_group_22 *lpstG22;
  struct s_lin_seg *lpstLin;
  struct s_imd_seg *lpstImd;
  char lochPattern;
  
  switch (poenType)
    {
    case ACCESS_TYPE: lochPattern = 'A'; break;
    case USAGE_TYPE: lochPattern = 'U'; break;
    case SUBS_TYPE: lochPattern = 'S'; break;
    case OCC_TYPE: lochPattern = 'O'; break;
    }

  lpstG22 = ppstG22;
  while (lpstG22)
    {
      lpstLin = lpstG22->lin;
      if (lpstLin && lpstLin->v_1222) 
        {
          if (EQ(lpstLin->v_1222, "02") || EQ(lpstLin->v_1222, "01"))
            {
              break;
            }

          if (EQ(lpstLin->v_1222, "03"))
            {
              lpstImd = lpstG22->imd;
              while (lpstImd)
                {
                  if (EQ(lpstImd->v_7009, "CT") && lpstImd->v_7008a[0] == lochPattern)
                    {
                      return lpstG22;
                    }
                  
                  lpstImd = lpstImd->imd_next;
                }
            }
        }
      
      lpstG22 = lpstG22->g_22_next;
    }

  return NULL;
}

struct s_group_22 *lpstFindSimSegment(struct s_group_22 *ppstG22, char *ppszSim)
{
  struct s_group_22 *lpstG22;
  struct s_lin_seg *lpstLin;
  struct s_imd_seg *lpstImd;

  lpstG22 = ppstG22;
  while (lpstG22)
    {
      lpstLin = lpstG22->lin;
      if (lpstLin && lpstLin->v_1222 && EQ(lpstLin->v_1222, "02") && lpstLin->v_7140[0] == '\0')
        {
          lpstImd = lpstFindItemDescription(lpstG22->imd, "SMNUM");
          if (lpstImd != NULL && EQ(lpstImd->v_7008a, ppszSim))
            {
              return lpstG22;
            }
        }
      lpstG22 = lpstG22->g_22_next;
    }
  
  return NULL;
}

struct s_group_22 *fpstFindCoServCat(struct s_group_22 *g_22,
                                     enum toenItemType poenType)
{
  struct s_imd_seg *imd;
  char pochType;
  
  switch (poenType)
    {
    case ACCESS_TYPE: pochType = 'A'; break;
    case USAGE_TYPE:  pochType = 'U'; break;
    case SUBS_TYPE:   pochType = 'S'; break;
    case OCC_TYPE:    pochType = 'O'; break;
    }

  while (g_22)
    {      
      if (g_22->lin->v_1222 && EQ(g_22->lin->v_1222, "03") && g_22->lin->v_7140[0] == '\0')
        {
          imd = lpstFindItemDescription(g_22->imd, "CT");
          if (imd->v_7008a[0] == pochType)
            {
              return g_22;
            }
        }
      
      g_22 = g_22->g_22_next;
    }
  
  return NULL;
}

struct s_group_22 *lpstFindSubscriberSegment(struct s_group_22 *ppstG22, int poiSegmentNo)
{
  struct s_group_22 *lpstG22;
  struct s_lin_seg *lpstLin;
  struct s_imd_seg *lpstImd;
  int loiSegmentNo;

  lpstG22 = ppstG22;
  loiSegmentNo = 0;
  while (lpstG22)
    {
      lpstLin = lpstG22->lin;
      if (lpstLin && lpstLin->v_1222 && EQ(lpstLin->v_1222, "01"))
        {
          if (poiSegmentNo == loiSegmentNo)
            {
              return lpstG22;
            }
          else
            {
              loiSegmentNo++;
            }
        }

      lpstG22 = lpstG22->g_22_next;      
    }

  return NULL; 
}

struct s_group_22 *lpstFindSubscriberContractSegment(struct s_group_22 *ppstG22, int poiContractNo)
{
  struct s_group_22 *lpstG22;
  struct s_lin_seg *lpstLin;
  struct s_imd_seg *lpstImd;
  int loiContractNo;

  lpstG22 = ppstG22;
  loiContractNo = 0;
  while (lpstG22)
    {
      lpstLin = lpstG22->lin;
      if (lpstLin && lpstLin->v_1222 && EQ(lpstLin->v_1222, "02") && NOT(EQ(lpstLin->v_7140, "O")))
        {
          if (poiContractNo == loiContractNo)
            {
              return lpstG22;
            }
          loiContractNo++;
        }
      if (lpstLin && lpstLin->v_1222 && EQ(lpstLin->v_1222, "01"))
        {
          return NULL;
        }

      lpstG22 = lpstG22->g_22_next;      
    }

  return NULL; 
}



struct s_imd_seg *lpstFindItemDescription(struct s_imd_seg *ppstImd, char *ppszDescription)
{
  struct s_imd_seg *lpstImd;

  lpstImd = ppstImd;
  while (lpstImd) 
    {
      if (EQ(lpstImd->v_7009, ppszDescription))
        {
          return lpstImd;
        }

      lpstImd = lpstImd->imd_next;
    }

  return NULL;

}



struct s_rff_seg *lpstFindReference(struct s_group_3 *ppstG3, char *ppszType) 
{
  struct s_group_3 *lpstG3;  
  struct s_rff_seg *lpstRff;

  lpstG3 = ppstG3;
  while (lpstG3) 
    {
      lpstRff = lpstG3->rff;
      if (EQ(lpstRff->v_1153, ppszType))
        {
          return lpstRff;
        }

      lpstG3 = lpstG3->g_3_next;
    }

  return NULL;
}


char *lpchzGetField(int poiFieldNo, char *pochzField)
{
  static char lpchzOutput[MAX_BUFFER];
  char *lpchzToken;
  char lpchzField[MAX_BUFFER];
  int loiTokenNo;
  int loiFound;
  
  strncpy(lpchzField, pochzField, MAX_BUFFER);
  loiTokenNo = 0;

  loiFound = TRUE;
  lpchzToken = strtok(lpchzField, ".");
  if (poiFieldNo == 0)
    {
      fovdPrintLog (LOG_DEBUG, "FIELD %d = %s in %s\n", poiFieldNo, lpchzToken, pochzField);
      strncpy(lpchzOutput, lpchzToken, MAX_BUFFER);
      return lpchzOutput;
    }

  while (loiTokenNo < poiFieldNo)
    {
      lpchzToken = strtok(NULL, ".");
      if (lpchzToken == NULL)
        {
          lpchzOutput[0] = '\0';
          loiFound = FALSE;
          break;
        }
      loiTokenNo++;
    }
  
  if (loiFound)
    {
      fovdPrintLog (LOG_DEBUG, "FIELD %d = %s in %s\n", poiFieldNo, lpchzToken, pochzField);
      strncpy(lpchzOutput, lpchzToken, MAX_BUFFER);
    }

  return lpchzOutput;
}

struct s_qty_seg *lpstFindQuantity(struct s_qty_seg *ppstQty, char *ppchzDetails, char *ppchzUnitType) 
{
  struct s_qty_seg *lpstQty;
  fovdPushFunctionName ("lpstFindQuantity");

  lpstQty = ppstQty;
  while (lpstQty) 
    {
      if (EQ(lpstQty->v_6063, ppchzDetails) && EQ(lpstQty->v_6411, ppchzUnitType))
        {
          fovdPopFunctionName ();
          return lpstQty;
        }
      lpstQty = lpstQty->qty_next;
    }

  fovdPopFunctionName ();
  return NULL;
}


struct s_qty_seg *fpstFindQuantitySegment(struct s_qty_seg *ppstQty, char *ppchzType) 
{
  struct s_qty_seg *lpstQty;
  fovdPushFunctionName ("lpstFindQuantitySegment");

  lpstQty = ppstQty;
  while (lpstQty) 
    {
      if (EQ(lpstQty->v_6063, ppchzType))
        {
          fovdPopFunctionName ();
          return lpstQty;
        }

      lpstQty = lpstQty->qty_next;
    }

  fovdPopFunctionName ();
  return NULL;
}

struct s_pia_seg *lpstFindProductId(struct s_pia_seg *pia, 
				    enum toenRecordType poenRecordType) 
{
  return pia;
};

struct s_xcd_seg *lpstFindXCDSegment(struct s_group_99 *ppstG99, 
				     int poiIndex)
{
  struct s_xcd_seg *lpstXcd;
  struct s_group_99 *lpstG99;
  int loiIndex;

  loiIndex = 0;
  lpstG99 = ppstG99;
  while (lpstG99) 
    {
      if (loiIndex == poiIndex) 
        {
          return lpstG99->xcd;
        }
      else
        {
          loiIndex++;
        }
      lpstG99 = lpstG99->g_99_next;
    }

  return NULL;
}

char *lpchzFindCountryOfVPLMN(char *ppstVPLMN) 
{
  return ppstVPLMN;
};


/**************************************************************************************************
 *                                                                                          
 * FUNCTION :  foenFetchContractNumber
 *                                                                                          
 * LOCAL FUNCTIONS USED:                                                                    
 * struct s_group_22 *lpstFindSubscriberSegment(struct s_group_22 *, int);  
 * struct s_group_22 *lpstFindSubscriberContractSegment(struct s_group_22 *, int);
 * struct s_lin_seg *lpstFindItemDescription(struct s_lin_seg *, char *);
 *                                                                                         
 * EXTERNAL FUNCTIONS USED:                                                                 
 * MODULE: SUM_FETCH                                                                        
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenFetchContractData(struct s_TimmInter *ppstSumSheet, 
			       int poiSubscriberNo, 
			       int poiContractNo, 
                               tostContractData *ppstContractData)
{
  struct s_group_22 *g_22;
  struct s_lin_seg *lin;  
  struct s_imd_seg *imd;  
  int loiContractsNo;

  fovdPushFunctionName("foenFetchContractData");

  g_22 = lpstFindSubscriberSegment(ppstSumSheet->timm->g_22, poiSubscriberNo);  
  if (g_22 == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }
  
  g_22 = lpstFindSubscriberContractSegment(g_22->g_22_next, poiContractNo);
  if (g_22 == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }

  imd = lpstFindItemDescription(g_22->imd, "CO");
  if (imd == NULL) 
    {
      fovdPopFunctionName();
      return FALSE;
    }

  strncpy(ppstContractData->sachzContractNo, imd->v_7008a, MAX_BUFFER);

  fovdPopFunctionName();
  return TRUE;
}


toenBool foenFetchAirTime(struct s_TimmInter *ppstSumSheet, 
                          char *pachzSim,  
                          char *pachzAirTimeValue, int loiAirTimeBufLen)
{
  struct s_group_22 *lpstG22;
  struct s_group_99 *lpstG99;  
  struct s_moa_seg *lpstMoa;
  struct s_lin_seg *lpstLin;
  struct s_xcd_seg *lpstXcd;
  struct s_group_23 *lpstG23;
  float lofThreshold;
  double lofAirTimeValue, lofSumAirTimeValue ;
  int n;
  toenBool foenIsSpecialNumber(char *);
  
  lpstG22 = lpstFindSimSegment(ppstSumSheet->timm->g_22, pachzSim);
  if (lpstG22 == NULL)
    {
      fovdPopFunctionName ();
      return FALSE;
    }  
  fovdPrintLog (LOG_DEBUG, "G22 segment found\n");
  
    /*
   * G22 segment on level 03 with 
   * - IMD block empty 
   * - MOA+903 in group G23
   * - RFF+IC in group G23
   * - G99 not empty
   * Let's find any block with G99 not equal NULL
   */

  lpstG99 = NULL;
  lpstG22 = lpstG22->g_22_next;
  while (lpstG22)
    {
      lpstLin = lpstG22->lin;

      /*
       * Next contract or customer
       */
      if (EQ(lpstLin->v_1222, "02") || EQ(lpstLin->v_1222, "01"))
        {
          lpstG99 = NULL;
          break;
        }

      /*
       * Not broken loop so we are in a contract
       */
      if (lpstG22->g_99)
        {
          lpstG99 = lpstG22->g_99;
          break;
        }
      
      lpstG22 = lpstG22->g_22_next;
    }
  fovdPrintLog (LOG_DEBUG, "Usage g22 segment found\n");

  /*
   * XCD seq was found
   */

  if (lpstG99 != NULL)
    {
      lofSumAirTimeValue = 0.0;
      while (lpstG99) 
        {
          lpstXcd = lpstG99->xcd;
          if (lpstXcd->v_X019[0] == 'A' &&   /* AIR */
              lpstXcd->v_X026[0] == 'O' &&   /* OUTBOUND */
              lpstXcd->v_X028[0] == 'H' &&   /* HOME */
              EQ(lpstXcd->v_6411, "Sec") &&  /* measured in Sec -> SERVICE = TELEF */
              /*
              foenIsSpecialNumber(lpstXcd->v_X043) == FALSE) /* special number 
              */
              lpstXcd->v_X022[0] != 'S')      /* special number */
            {
              sscanf(lpstXcd->v_5004, "%lf", &lofAirTimeValue);              
              fovdPrintLog (LOG_TIMM, "Adding air time value [%s, %s, %s]: %.02lf\n", 
                            lpstXcd->v_X001, lpstXcd->v_X002, lpstXcd->v_X004,
                            lofAirTimeValue);
              lofSumAirTimeValue = lofSumAirTimeValue + lofAirTimeValue;
            }
          lpstG99 = lpstG99->g_99_next;
        }
    }
  
  sprintf(pachzAirTimeValue, "%.2lf", lofSumAirTimeValue);
  fovdPrintLog (LOG_CUSTOMER, "Air time summary value: %.2lf\n", lofSumAirTimeValue);

  fovdPopFunctionName ();
  return TRUE;

}

extern stPN *pstPN;
extern long glPNCount;

toenBool foenIsSpecialNumber(char *lpchzDigits)
{
  int i;

  for (i = 0; i < glPNCount; i++)
    {
      if (EQ(pstPN[i].szDigits, lpchzDigits))
        {
          return TRUE;
        }
    }
  
  return FALSE;
}

/**************************************************************************************************
 * 
 * FUNCTION: foenIsCustomerOCC
 *
 * DESCRIPTION: Check if OCC is located on subscriber level of a G22 block list in some
 *              block on the level 02 and with produt id 'O'
 *
 **************************************************************************************************/

extern int foiScanMoa(char *, int);

toenBool foenIsCustomerOCC(struct s_TimmInter *ppstSumSheet, 
                           char *ppsnzFE, 
                           char *ppsnzNetAmount)
{
  struct s_group_22 *lpstG22;  
  struct s_lin_seg *lpstLin;
  struct s_moa_seg *lpstMoa;
  struct s_imd_seg *lpstImd;
  int loiVal, loiItemVal;
  toenBool loenStatus;

  fovdPushFunctionName("foenIsCustomerOCC");
  
  loiVal = foiScanMoa(ppsnzNetAmount, 2);
  
  loenStatus = FALSE;
  lpstG22 = ppstSumSheet->timm->g_22;
  while (lpstG22)
    {
      lpstLin = lpstG22->lin;

      /*
       * Intem of G22 on level 02 with prodict id O like OCC
       */

      if (EQ(lpstLin->v_1222, "02") && EQ(lpstLin->v_7140, "O"))
        {
          lpstImd = lpstFindItemDescription(lpstG22->imd, "FE");
          if (lpstImd == NULL || NOT(EQ(lpstImd->v_7008a, ppsnzFE)))
            {
              lpstG22 = lpstG22->g_22_next;
              continue;
            }

          /*
           * Net amount of OCC item on subsciber level
           */

          lpstMoa = fpstFindPaymentSegment(lpstG22->g_23, "125", "5");
          if (lpstMoa != NULL)
            {
              loiItemVal = foiScanMoa(lpstMoa->v_5004, 2);
              if (loiVal == loiItemVal)
                {
                  /*
                   * Item found
                   */

                  loenStatus = TRUE;
                  break;
                }
            }
        }  

      lpstG22 = lpstG22->g_22_next;
    }

  fovdPopFunctionName();  
  return loenStatus;
}

double foflFindThresholdValue(struct s_group_22 *ppstG22)
{
  struct s_moa_seg *lpstMoa;
  double loflThresholdValue;
  int n;

  fovdPushFunctionName("foflFindThresholdValue");

  /*
   * Try to find MOA+901 in G23
   */
  
  if ((lpstMoa = fpstFindPaymentSegment(ppstG22->g_23, "901", "9")) == NULL)
    {
      fovdPopFunctionName();  
      return 0.0;
    }
  
  if ((n = sscanf(lpstMoa->v_5004, "%lf", &loflThresholdValue)) != 1)
    {
      fovdPopFunctionName();  
      return 0.0;
    }
  
  fovdPopFunctionName();  
  return loflThresholdValue;
}

toenBool foenFetchContractInfo(struct s_group_22 *ppstG22, 
                               long *ppilCoId, 
                               double *ppflBCHSumUsage, 
                               double *ppflBCHSumSubscription,
                               double *ppflBCHSumAccess, 
                               double *ppflBCHSumOCC)
{
  struct s_group_22 *lpstG22;
  struct s_imd_seg *lpstImd;
  struct s_moa_seg *lpstMoa;
  struct s_lin_seg *lpstLin;
  toenBool loenStatus, loenCoIdFound = FALSE;
  int n, loiAmountsCat = 0;
  
  fovdPushFunctionName("foenFetchContractInfo");

  fovdPrintLog (LOG_DEBUG, "foenFetchContractInfo: Loading G22\n");
  lpstG22 = ppstG22;  
  while (lpstG22) 
    {
      lpstLin = lpstG22->lin;
      
      /*
       * Fill List info fields with values found in the G22 item list
       */
      
      if (EQ(lpstLin->v_1222, "02") && loenCoIdFound == FALSE)
        {
          /*
           * Contract level
           */
          
          fovdPrintLog (LOG_DEBUG, "foenFetchContractInfo: Loading G22 level 02\n");
          
          lpstImd = fpstFindItemDescription(lpstG22->imd, "CO");
          if (lpstImd != NULL)
            {
              n = sscanf(lpstImd->v_7008a, "%ld", ppilCoId);
              ASSERT(n == 1);
              loenCoIdFound = FALSE;
              fovdPrintLog (LOG_TIMM, "Contract : %ld\n", *ppilCoId);
            }
        }
      
      else if (EQ(lpstLin->v_1222, "03") & loiAmountsCat < 4)
        {
          /*
           * Item level: Usage, Access, OCC, Subscription
           */
      
          fovdPrintLog (LOG_DEBUG, "foenFetchContractInfo: Loading G22 level 03\n");
          lpstImd = fpstFindItemDescription(lpstG22->imd, "CT");         
          lpstMoa = fpstFindPaymentSegment(lpstG22->g_23, "932", "9"); 
          if (lpstImd != NULL && lpstMoa != NULL)
            {
              switch(lpstImd->v_7008a[0])
                {
                case 'O': /* OCC */
                  n = sscanf(lpstMoa->v_5004, "%lf", ppflBCHSumOCC);
                  fovdPrintLog (LOG_TIMM, "OCC      : %8.02lf\n", *ppflBCHSumOCC);
                  break;
                  
                case 'U': /* Usage */
                  n = sscanf(lpstMoa->v_5004, "%lf", ppflBCHSumUsage);
                  fovdPrintLog (LOG_TIMM, "Usage    : %8.02lf\n", *ppflBCHSumUsage);
                  break;
              
                case 'S': /* Subscription */
                  n = sscanf(lpstMoa->v_5004, "%lf", ppflBCHSumSubscription);
                  fovdPrintLog (LOG_TIMM, "Subscr.  : %8.02lf\n", *ppflBCHSumSubscription);
                  break;
                  
                case 'A': /* Access */
                  n = sscanf(lpstMoa->v_5004, "%lf", ppflBCHSumAccess);
                  fovdPrintLog (LOG_TIMM, "Access   : %8.02lf\n", *ppflBCHSumAccess);
                  break;
                }
              
              ASSERT(n == 1);
              loiAmountsCat ++;
            }
        }
      
      else if (EQ(lpstLin->v_1222, "03") || EQ(lpstLin->v_1222, "02"))
        {
          break;
        }
      
      lpstG22 = lpstG22->g_22_next;
    }

  fovdPopFunctionName();
  return TRUE;  
}
