/**************************************************************************************************
 *                                                                                          
 * MODULE: SIM_ENC_GEN
 *                                                                                          
 * AUTHOR: N.Bondarczuk                                                
 * 
 * CREATION DATE : 05.11.97                                              
 *
 * DESCRIPTION: Contains functions generating invoice enclosure with information about
 *              all contracted SIM numbers. Produces itemized list of calls done from
 *              this SIM number. Creates roaming raport for a contract with this SIM 
 *              number.   
 *
 **************************************************************************************************
 */

#include <stdio.h>
#include "bgh.h"
#include "protos.h"
#include "parser.h"

#include "types.h"
#include "gen.h"
#include "strutl.h"
#include "inv_gen.h"
#include "inv_types.h"
#include "inv_item.h"
#include "inv_fetch.h"
#include "sum_fetch_acc.h"
#include "sum_fetch_sim.h"
#include "roa_fetch_sim.h"
#include "sim_enc_gen.h"
#include "dealer.h"
#include "item_list.h"
#include "call_list.h"
#include "legend.h"
#include "con_serv.h"
#include "workparval.h"

#if 0   /* just for version.sh */
static char *SCCS_VERSION = "2.3.1";
#endif

/*
 * 28.01.98 N.Bondarczuk : Suppress zero bill item generatior policy added
 * 10.02.98 N.Bondarczuk : Use threshold value to suppress calls in SIM enclosure
 * 12.02.98 N.Bondarczuk : Create legend section with codes and descriptions of call types
 * 19.02.98 N.Bondarczuk : Don't suppress printing sum if sum is zero and number of items in a list is not eq. zero
 * 09.03.98 N.Bondarczuk : Dealer comission handling in version 1.3
 * 25.06.98 N.Bondarczuk : Major change in functions greating categories lists
 */

extern stBGHGLOB        stBgh;                  /* structure with globals for BGH */

extern char gachzInvoiceType[MAX_BUFFER];

extern stServiceName *pstServiceName;

extern long glSNCount;

struct s_TimmInter *dpstSumSheet, *dpstRoamingSheet;

static char szTemp[128];

static TYPEID doenTypeId;

static double doflBCHSumUsage;

static long doilCoId;

/*
 * local functions
 */

static toenBool foenGenHeader();

static toenBool foenGenSimList();

static toenBool foenGenSimListEntry(int poiSubNo);

static toenBool foenGenSimSummaryItemList(struct tostCoInfo *ppstCoInfo,
                                          struct tostItemCallList *ppstCallList_Local, 
                                          struct tostItemCallList *ppstCallList_Roaming); 

static toenBool foenGenSimSummaryPaymentType(struct tostCoInfo *ppstCoInfo,
                                             enum toenItemType poenType);

static toenBool foenGenSimRoamingList(char *);

static toenBool foenGenSimRoamingList(char *poszSim);

static toenBool foenFlushSimCallList(struct tostItemCallList *ppstList, 
				     char *ppszNumber);

/**************************************************************************************************
 *                                                                                          
 * FUNCTION : foenGenSimList             
 *                                                                                          
 **************************************************************************************************
 */

extern void timm_test_print(struct s_TimmInter *);

toenBool foenGenSimEnclosure(TYPEID toenType, 
                             struct s_TimmInter *ppstSumSheet , 
                             struct s_TimmInter *ppstRoamingSheet) 
{
  toenBool loenStatus;
  int i;
  
  fovdPushFunctionName("foenGenSimEnclosure");
  
  doenTypeId = toenType;
  dpstSumSheet = ppstSumSheet;
  dpstRoamingSheet = ppstRoamingSheet;
  
  stBgh.soSubscriberCallDetailListPrinted = FALSE; 
  
  fovdGen("SimEnclosureStart", EOL);
  fovdGen("SimType", "0", EOL);

  fovdPrintLog (LOG_DEBUG, "Starting document creation for SIM enclosure\n");
  
  loenStatus = foenGenHeader();
  if (loenStatus == FALSE)
    {
      sprintf (szTemp, "foenGenSimEnclosure: ERROR in function foenGenHeader\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName();
      return FALSE;
    }  
  
  loenStatus = foenLegend_Init();
  if (loenStatus == FALSE)
    {
      sprintf (szTemp, "foenGenSimEnclosure: ERROR in function foenLegend_Init\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName();
      return FALSE;
    }  
  
  fovdGen("SimListStart", EOL);
  
  loenStatus = foenGenSimList();
  if (loenStatus == FALSE)
    {
      sprintf (szTemp, "foenGenSimEnclosure: ERROR in function foenGenSimList\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName();
      return FALSE;
    }

  fovdGen("SimListEnd", EOL);
  
  loenStatus = foenLegend_Gen();
  if (loenStatus == NULL)
    {
      sprintf (szTemp, "foenGenSimEnclosure: Can't create legend\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName();
      return FALSE;      
    }
  
  fovdGen("SimEnclosureEnd", EOL);
  fovdPrintLog (LOG_DEBUG, "Document generated for SIM enclosure\n");
  
  fovdPopFunctionName();
  return TRUE;
}

/**************************************************************************************************
 *                                                                                          
 * FUNCTION : foenGenSimList             
 *                                                                                          
 **************************************************************************************************
 */

static toenBool foenGenSimList() 
{
  int loiSub, loiSubNo;
  toenBool loenState;

  fovdPushFunctionName("foenGenSimList");
  
  fovdPrintLog (LOG_DEBUG, "SIM Item List Start\n");
  
  loiSubNo = foiCountSubscribers(dpstSumSheet); 
  fovdPrintLog (LOG_DEBUG, "Subscribers no: %d\n", loiSubNo);
  for (loiSub = 0; loiSub < loiSubNo; loiSub++)
    {
      fovdPrintLog (LOG_DEBUG, "Processing subscriber no: %d\n", loiSub);

      loenState = foenGenSimListEntry(loiSub);
      if (loenState == FALSE)
        {
          sprintf (szTemp, "ERROR in function foenGenSimList: Can't create info for subscriber\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          fovdPopFunctionName();
          return FALSE;
        }
    }

  fovdPrintLog (LOG_DEBUG, "SIM Item List End\n");
  
  fovdPopFunctionName();
  return TRUE;
}


/**************************************************************************************************
 *                                                                                          
 * FUNCTION : foenGenSimListEntry             
 *                                                                                          
 **************************************************************************************************
 */

static toenBool foenGenSimListEntry(int poiSubNo) 
{
  int loiCoNo;
  int loiCo;
  toenBool loenStatus;
  int iRet;
  struct tostItemCallList *lpstCallList_Local;
  struct tostItemCallList *lpstCallList_Roaming;
  int loiCoId = -1;
  char lachzAccountNo [MAX_BUFFER];
  struct tostCoInfo lostCoInfo;

  fovdPushFunctionName("foenGenSimListEntry");

  /*
   * CONTRACTS
   */
  
  loiCoNo = foiCountSubscriberContracts(dpstSumSheet, poiSubNo);
  fovdPrintLog (LOG_DEBUG, "foenGenSimListEntry: Subscriber %d has %d contracts\n", 
                poiSubNo, 
                loiCoNo);
  
  iRet = fetchSubscriberCustomerCode(dpstSumSheet, 
                                     poiSubNo, 
                                     lachzAccountNo, 
                                     MAX_BUFFER);
  if (iRet == FALSE)
    {
      sprintf (szTemp, "ERROR in function fetchSubscriberCustomerCode\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName ();
      return FALSE;
    }
  
  for (loiCo = 0; loiCo < loiCoNo; loiCo++) 
    {
      fovdPrintLog (LOG_DEBUG, "foenGenSimListEntry: co: %d\n", loiCo);
      
      loenStatus = foenFetchCoInfo(dpstSumSheet,
                                   poiSubNo,
                                   loiCo,
                                   &lostCoInfo);
      if (loenStatus == FALSE)
        {
          sprintf (szTemp, "ERROR in function foenFetchCoInfo\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          fovdPopFunctionName();
          return FALSE;
        }
      else
        {
          (void)sscanf(lostCoInfo.saszCoNo, "%d", &loiCoId);
        }

      fovdPrintLog (LOG_CUSTOMER, "Processing CO: %s, SIM: %s, DN: %s, MOA: %s\n", 
                    lostCoInfo.saszCoNo,
                    lostCoInfo.saszSmNo,
                    lostCoInfo.saszDnNo,
                    lostCoInfo.saszSumAmt);
      
      /*
       * Create header section
       */

      stBgh.soContractCallDetailListExist = FALSE;
      fovdGen("SimPhoneNumber", 
              lostCoInfo.saszMarketCode,
              lostCoInfo.saszMarket,
              lachzAccountNo, 
              lostCoInfo.saszSmNo,
              lostCoInfo.saszDnNo,
              EOL);       
      
      fovdFormatMoney(lostCoInfo.saszSumAmt);  
      fovdGen("SimTotalSum", 
              lostCoInfo.saszSumAmt, 
              lostCoInfo.saszCurrency, 
              EOL);
      
      /*
       * Prepare call lists
       */
            
      lpstCallList_Local = fpstItemCallList_New(lostCoInfo.g_99,
                                                HPLMN_TYPE, 
                                                lostCoInfo.saszDnNo);
      
      lpstCallList_Roaming = fpstItemCallList_New(lostCoInfo.g_99,
                                                  VPLMN_TYPE, 
                                                  lostCoInfo.saszDnNo);

      /*
       * Contract item list
       */
      
      fovdGen("SimSummaryItemListStart", EOL);

      loenStatus = foenGenSimSummaryItemList(&lostCoInfo, 
                                             lpstCallList_Local, 
                                             lpstCallList_Roaming);
      if (loenStatus == FALSE)
        {
          sprintf (szTemp, "foenGenSimSummaryItemList: Can't create list of items\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          fovdErrorPopFunctionName();
          return FALSE;
        }
      
      fovdGen("SimSummaryItemListEnd", EOL);
      
      /*
       * WORKPARVAL info for the contract
       */
      
      if ((iRet = foiParVal_SimGen(dpstSumSheet, 
                                   loiCoId, 
                                   lostCoInfo.saszDnNo)) < 0)
        {
          sprintf (szTemp, "foenGenSimSummaryItemList: Can't get WORKPARVAL for co_id: %d\n", loiCoId);
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          fovdErrorPopFunctionName();
          return FALSE;
        }
      
      /*
       * No printing of this info if document is a part of Invoice 
       */
      
      if (doenTypeId == ENC_TYPE || doenTypeId == SUM_TYPE)
        {
          /*
           * VISITED VPLMN LIST
           */
          
          loenStatus = foenGenSimRoamingList(lostCoInfo.saszSmNo);
          if (loenStatus == FALSE)
            {
              sprintf (szTemp, "foenGenSimRoamingList: Can't create VPLMN list\n");
              macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
              fovdPopFunctionName();
              return FALSE;
            }
          
          /*
           * Print and destroy call lists
           */

          /*
           * CALL LIST
           */
          
          if (lpstCallList_Local != NULL)
            {
              loenStatus = foenFlushSimCallList(lpstCallList_Local, 
                                                lostCoInfo.saszDnNo);
              if (loenStatus == FALSE)                
                {
                  fovdPopFunctionName();
                  return FALSE;
                }
            }
          
          /*
           * ROAMING CALL LIST
           */
          
          if (lpstCallList_Roaming != NULL)
            {
              loenStatus = foenFlushSimCallList(lpstCallList_Roaming, 
                                                lostCoInfo.saszDnNo); 
              if (loenStatus == FALSE)
                {
                  fovdPopFunctionName();
                  return FALSE;
                }
            }
        }      
      
      /*
       * Do we handle dealer comission ?
       */
          
      if ((doenTypeId == INV_TYPE || doenTypeId == SUM_TYPE)  
          && stBgh.bInsertComission == TRUE)
        {
          /*
           * AIR TIME for dealers
           */
          
          loenStatus = foenHandleDealerComission(dpstSumSheet, 
                                                 poiSubNo, 
                                                 loiCo);
          if (loenStatus == FALSE)
            {
              sprintf (szTemp, "foenHandleDealerComission: Can't create comission info\n");
              macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);   
              fovdPopFunctionName();
              return FALSE;
            }
        }
    }
  
  fovdPopFunctionName();
  return TRUE;
}

/**************************************************************************************************
 *                                                                                          
 * FUNCTION: foenGenSimSummaryItemList             
 *                                                                                          
 **************************************************************************************************
 */

#define CATEGORIES 4

static toenBool foenGenSimSummaryItemList(struct tostCoInfo *ppstCoInfo,
                                          struct tostItemCallList *ppstCallList_Local, 
                                          struct tostItemCallList *ppstCallList_Roaming) 
{
  toenBool loenStatus;
  struct tostItemList *lpstList;
  struct tostItem *lpstItem;
  struct s_group_22 *g_22;
  int i;
  int loiItemsNo;  
  char lasnzItemsNo[16];
  double loflBCHSumUsage; 
  double loflBCHSumSubscription;
  double loflBCHSumAccess;
  double loflBCHSumOCC;
  long loilCoId;
  struct tostConServTab *lpstTab;
  enum toenItemType laenType[CATEGORIES] = 
  {
    SUBS_TYPE, 
    ACCESS_TYPE, 
    OCC_TYPE, 
    USAGE_TYPE
  };
  char *lasnzCategory[CATEGORIES] = 
  {
    "Subs", 
    "Access", 
    "OCC", 
    "Usage" 
  }; 
  char *lasnzStartTag[CATEGORIES] = 
  {
    "SimSummaryItemServiceStart", 
    "SimSummaryItemAccessStart", 
    "SimSummaryItemOtherStart", 
    "SimSummaryItemUsageStart"
  };
  char *lasnzEndTag[CATEGORIES] = 
  {
    "SimSummaryItemServiceEnd", 
    "SimSummaryItemAccessEnd", 
    "SimSummaryItemOtherEnd", 
    "SimSummaryItemUsageEnd"
  };
  
  fovdPushFunctionName("foenGenSimSummaryItemList");
  
  loenStatus = foenFetchContractInfo(ppstCoInfo->g_22,
                                     &loilCoId, 
                                     &loflBCHSumUsage, 
                                     &loflBCHSumSubscription, 
                                     &loflBCHSumAccess, 
                                     &loflBCHSumOCC);
  if (loenStatus == FALSE)
    {
      sprintf (szTemp, "foenGenSimSummaryItemList: Can't get contract info\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);   
      fovdPopFunctionName();
      return FALSE;
    }          

  for (i = 0; i < CATEGORIES; i++)
    {
      /*
       * List lostAccessList will be used to keep Access items and to merge them
       */
      
      if (laenType[i] == USAGE_TYPE)
        {
          lpstTab = fpstConServTab_New(ppstCoInfo->g_22, 
                                       ppstCallList_Local, 
                                       ppstCallList_Roaming);
          if (lpstTab == NULL)
            {
              sprintf (szTemp, "foenGenSimSummaryItemList: Can't create category table\n");
              macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);   
              fovdPopFunctionName();
              return FALSE;
            }          
          
          /*
           * List of categories
           */
          
          if (foiConServTab_Size(lpstTab) > 0)
            {
              sprintf(lasnzItemsNo, "%d", foiConServTab_Size(lpstTab));
              
              fovdGen(lasnzStartTag[i],  lasnzItemsNo, EOL);
              
              loenStatus = foenConServTab_Gen(lpstTab);
              if (loenStatus == FALSE)
                {
                  sprintf (szTemp, "foenGenSimSummaryItemList: Can't generate category table\n");
                  macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);   
                  fovdPopFunctionName();
                  return FALSE;
                }
              
              fovdGen(lasnzEndTag[i], EOL);
            }

          loenStatus = foenConServTab_Delete(lpstTab);
          if (loenStatus == FALSE)
            {
              sprintf (szTemp, "foenGenSimSummaryItemList: Can't delete category table\n");
              macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);   
              fovdPopFunctionName();
              return FALSE;
            }
          
          /*
           * Summary payment
           */
          
          loenStatus = foenGenSimSummaryPaymentType(ppstCoInfo,
                                                    laenType[i]);
          if (loenStatus == FALSE)
            {
              sprintf (szTemp, "foenGenSimSummaryItemList: Can't create summary payment info\n");
              macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);   
              fovdPopFunctionName();
              return FALSE;
            }      
          
          continue;
        }

      fovdPrintLog (LOG_DEBUG, 
                    "Processing SIM: %s with charge category: %s\n", 
                    ppstCoInfo->saszSmNo, 
                    lasnzCategory[i]);  
      
      g_22 = fpstFindCoServCat(ppstCoInfo->g_22,
                               laenType[i]);
      if (g_22 == NULL)
        {
          sprintf (szTemp, "foenGenSimSummaryItemList: Can't find category\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);   
          fovdPopFunctionName();
          return FALSE;
        }

      fovdPrintLog (LOG_DEBUG, "Creating items for cat: %s\n", 
                    lasnzCategory[i]);
      
      lpstList = fpstItemList_New(g_22, 
                                  laenType[i], 
                                  NULL);
      if (lpstList == NULL)
        {
          sprintf (szTemp, "foenGenSimSummaryItemList: Can't create list for category\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);   
          fovdPopFunctionName();
          return FALSE;
        }

      loenStatus = foenItemList_Init(lpstList, 
                                     ppstCoInfo->g_22);
      if (loenStatus == FALSE)
        {
          sprintf (szTemp, "foenGenSimSummaryItemList: Can't init list for category\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);   
          fovdPopFunctionName();
          return FALSE;
        }

      /*
       * Print the number of items
       */
      
      fovdPrintLog (LOG_DEBUG, "Counting items for cat: %s\n", 
                    lasnzCategory[i]);
      
      loiItemsNo = foiItemList_Count(lpstList);
      if (loiItemsNo == 0)
        {
          fovdPrintLog (LOG_DEBUG, "Zero size of list for category: %s\n", 
                        lasnzCategory[i]);
          
          /*
           * Summary payment
           */
          
          loenStatus = foenGenSimSummaryPaymentType(ppstCoInfo,
                                                    laenType[i]);
          if (loenStatus == FALSE)
            {
              sprintf (szTemp, "foenGenSimSummaryItemList: Can't create summary payment info\n");
              macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);   
              fovdPopFunctionName();
              return FALSE;
            }      
          
          loenStatus = foenItemList_Delete(lpstList);
          if (loenStatus == FALSE)
            {
              sprintf (szTemp, "foenGenSimSummaryItemList: Can't delete the category list\n");
              macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);                 
              fovdPopFunctionName();
              return FALSE;
            }
                    
          continue;
        }

      sprintf(lasnzItemsNo, "%d", loiItemsNo);
      fovdGen(lasnzStartTag[i],  lasnzItemsNo, EOL);
      fovdPrintLog (LOG_DEBUG, "Number of items in the list : %d\n", loiItemsNo);
      
      /*
       * Walk the list
       */
      
      fovdPrintLog (LOG_DEBUG, "Printing the list for cat: %s\n",                    
                    lasnzCategory[i]);
      
      loenStatus = foenItemList_Gen(lpstList);
      if (loenStatus == FALSE)
        {
          sprintf (szTemp, "foenGenSimSummaryItemList: Can't print the category list\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);   
          fovdPopFunctionName();
          return FALSE;
        }
      
      fovdGen(lasnzEndTag[i], EOL);

      /*
       * Clean dynamic data structure allocated from the heap
       */
      
      fovdPrintLog (LOG_DEBUG, "Deleting the list of items for cat: %s\n", 
                    lasnzCategory[i]);
      
      loenStatus = foenItemList_Delete(lpstList);
      if (loenStatus == FALSE)
        {
          sprintf (szTemp, "foenGenSimSummaryItemList: Can't delete category list\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);   
          fovdPopFunctionName();
          return FALSE;
        }

      /*
       * Summary payment
       */

      loenStatus = foenGenSimSummaryPaymentType(ppstCoInfo, 
                                                laenType[i]);
      if (loenStatus == FALSE)
        {
          sprintf (szTemp, "foenGenSimSummaryItemList: Can't create summary payment info\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);   
          fovdPopFunctionName();
          return FALSE;
        }      
    }
  
  fovdPopFunctionName();
  return TRUE;
}


/**************************************************************************************************
 *                                                                                          
 * FUNCTION: foenGenSimSummaryPaymentType            
 *                                                                                          
 **************************************************************************************************
 */

static toenBool foenGenSimSummaryPaymentType(struct tostCoInfo *ppstCoInfo,
                                             enum toenItemType poenType)
{
  toenBool loenState;
  char *ppchzTag;
  struct tostPayment sostPayment;
  int n;
  double loflAmount;
  
  fovdPushFunctionName("foenGenSimSummaryPaymentType");
  
  loenState = foenFetchCoServCatPayment(dpstSumSheet, 
                                        ppstCoInfo, 
                                        poenType, 
                                        &sostPayment);
  if (loenState == FALSE)
    {
      sprintf (szTemp, "foenGenSimSummaryPaymentType: Can't fetch co serv cat payment\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName();
      return FALSE;
    }
  
  (void)sscanf(sostPayment.sasnzMonetaryAmount, "%lf", &loflAmount);    
  fovdPrintLog (LOG_DEBUG, "Summary Payment : %lf\n", loflAmount);
  if (loflAmount == 0.0)
    {
      fovdPrintLog (LOG_DEBUG, "Suppressed Summary Payment : %lf\n", loflAmount);
      fovdPopFunctionName();
      return TRUE;
    }
  
  switch (poenType) 
    {
    case USAGE_TYPE:
      ppchzTag = "SimSummaryUsage";
      break;

    case ACCESS_TYPE:
      ppchzTag = "SimSummaryAccess";
      break;

    case SUBS_TYPE:
      ppchzTag = "SimSummaryService";
      break;

    case OCC_TYPE:
      ppchzTag = "SimSummaryOther";
      break;
    }

  fovdFormatMoney(sostPayment.sasnzMonetaryAmount);  
  fovdPrintLog (LOG_DEBUG, "Summary Payment [%s]: %s\n", 
                ppchzTag, 
                sostPayment.sasnzMonetaryAmount);

  fovdGen(ppchzTag, 
          sostPayment.sasnzMonetaryAmount, 
          sostPayment.sasnzCurrency, 
          EOL);
    
  fovdPopFunctionName();
  return TRUE;
}

/**************************************************************************************************
 *                                                                                          
 * FUNCTION: foenGenSimRoamingList            
 *                                                                                          
 **************************************************************************************************
 */

static toenBool foenGenSimRoamingList(char *poszSim) 
{ 
  int n, i;
  toenBool loenStatus;
  struct tostVPLMN sostVPLMN;
  char lpchzItemCount[16];
  
  fovdPushFunctionName("foenGenSimRoamingList");

  /*
   * May be we have no roaming sheet ?
   */

  if (dpstRoamingSheet == NULL)
    {
      fovdPopFunctionName();
      return TRUE;
    }

  n = foiCountRoamingContractVPLMNs(dpstRoamingSheet, poszSim, MAX_BUFFER);
  fovdPrintLog (LOG_DEBUG, "VPLMNS = %d\n", n);
  if (n == 0)
    {
      fovdPopFunctionName();
      return TRUE;
    }
  
  sprintf(lpchzItemCount, "%d", n);
  
  fovdGen("SimRoamingListStart", 
          lpchzItemCount, 
          EOL);

  /*
   * There are some roamings
   */

  for (i = 0; i < n; i++)
    {
      fovdPrintLog (LOG_DEBUG, "VPLMN[%d]\n", i);
      loenStatus =  foenFetchRoamingContractVPLMN(dpstRoamingSheet, 
                                                  poszSim, 
                                                  MAX_BUFFER, 
                                                  i, 
                                                  &sostVPLMN);
      if (loenStatus == TRUE)
        {
          fovdPrintLog (LOG_DEBUG, "VPLMN : NAME %s\n",
                        sostVPLMN.laszName);

          fovdFormatMoney(sostVPLMN.laszNetInboundAmount);
          fovdFormatMoney(sostVPLMN.laszNetOutboundAmount);
          fovdFormatMoney(sostVPLMN.laszChargedTaxAmount);
          fovdFormatMoney(sostVPLMN.laszUsageAmount);
          fovdFormatMoney(sostVPLMN.laszSurchargeAmount);
          
          fovdGen("SimRoamingCharge",
                  sostVPLMN.laszName, 
                  sostVPLMN.laszNetInboundAmount, sostVPLMN.laszNetInboundCurrency, 
                  sostVPLMN.laszNetOutboundAmount, sostVPLMN.laszNetOutboundCurrency,
                  sostVPLMN.laszChargedTaxAmount, sostVPLMN.laszChargedTaxCurrency, 
                  sostVPLMN.laszUsageAmount, sostVPLMN.laszUsageCurrency,
                  sostVPLMN.laszSurchargeAmount, sostVPLMN.laszSurchargeCurrency,
                  EOL);   
        }
    }
  
  fovdGen("SimRoamingListEnd", EOL);
  
  fovdPopFunctionName();
  return TRUE; 
}

/**************************************************************************************************
 *                                                                                          
 * FUNCTION: foenGenHeader             
 *                                                                                          
 **************************************************************************************************
 */

static toenBool foenGenHeader() 
{
  toenBool loenState;
  static char loszDate[MAX_BUFFER];
  static char loszAccountNo[MAX_BUFFER];
  static char loszInvoiceNo[MAX_BUFFER];
  static char loszPeriodBegin[MAX_BUFFER];
  static char loszPeriodEnd[MAX_BUFFER];

  fovdPushFunctionName("foenGenHeader");

  /*
   * DATE
   */

  if (NOT(loenState = foenFetchInvoiceDate(dpstSumSheet, loszDate, MAX_BUFFER)))
    {
      sprintf (szTemp, "ERROR in function foenFetchInvoiceDate.\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName();
      return FALSE;
    }
  
  fovdFormatDate(loszDate, YY_MM_DD);
  
  fovdPrintLog (LOG_DEBUG, "SimEnclosureHeder : DATE = %s\n", loszDate);
  
  fovdGen("SimEnclosureHeader", 
          loszDate, 
          EOL);
  
  /*
   * ACCOUNT NO
   */

  if (NOT(loenState = foenFetchInvoiceCustomerAccountNo(dpstSumSheet, loszAccountNo, MAX_BUFFER)))
    {
      sprintf (szTemp, "ERROR in function foenFetchInvoiceCustomerAccount.\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName();
      return FALSE;
    }

  fovdPrintLog (LOG_DEBUG, "SimCustomerAccount : ACCOUNT = %s\n", 
                loszAccountNo);
  
  fovdGen("SimCustomerAccount", 
          loszAccountNo, 
          EOL);

  /*
   * INVOICE NO, BEGIN, END
   */
  
  if (NOT(loenState = foenFetchInvoiceNo(dpstSumSheet, loszInvoiceNo, MAX_BUFFER)))
    {
      sprintf (szTemp, "ERROR in function foenFetchInvoiceNo.\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName();
      return FALSE;
    }

  if (NOT(loenState = foenFetchInvoicePeriodBegin(dpstSumSheet, loszPeriodBegin, MAX_BUFFER)))
    {
      sprintf (szTemp, "ERROR in function foenFetchPeriodBegin.\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName();
      return FALSE;
    }

  if (NOT(loenState = fetchInvoicePeriodEnd(dpstSumSheet, loszPeriodEnd, MAX_BUFFER)))
    {
      sprintf (szTemp, "ERROR in function foenFetchPeriodEnd.\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName();
      return FALSE;
    }

  fovdFormatDate(loszPeriodBegin, YY_MM_DD);
  fovdFormatDate(loszPeriodEnd, YY_MM_DD);
  fovdGen("SimTitle", 
          loszInvoiceNo, 
          loszPeriodBegin, 
          loszPeriodEnd, 
          EOL);

  fovdPrintLog (LOG_DEBUG, "SimTitle: INVOICE = %s, BEGIN = %s, END = %s\n", 
                loszInvoiceNo, 
                loszPeriodBegin, 
                loszPeriodEnd);
  
  fovdPopFunctionName();
  return TRUE;
}

/**************************************************************************************************
 *                                                                                          
 * FUNCTION: foenFlushSimCallList
 *                                                                                          
 **************************************************************************************************
 */

static toenBool foenFlushSimCallList(struct tostItemCallList *ppstList, 
				     char *ppszNumber)
{
  toenBool loenStatus = TRUE;
  
  loenStatus = foenItemCallList_Gen(ppstList, ppszNumber);
  if (loenStatus == TRUE)
    {
      loenStatus = foenItemCallList_Delete(ppstList);
    }
  
  return loenStatus;
}



