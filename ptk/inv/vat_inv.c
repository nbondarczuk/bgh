#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "bgh.h"
#include "protos.h"
#include "parser.h"
#include "types.h"
#include "gen.h"
#include "vat_inv.h"
#include "strutl.h"
#include "money.h"
#include "bgh_transx.h"
#include "timm.h"

#ifdef _MACHINE_ROUNDING_
#define ROUND(v) foflRound(v)
#else
#define ROUND(v) v
#endif

/*
 * global variables
 */

double goflInvoicePayment = 0.0;
double goflDiscountAmount = 0.0;

/*
 * extern functions
 */

extern double foflMoney_Round(double poflVal, 
                              double *ppflRest); 

toenBool foenTimm_GetDiscAmount(stTIMM *ppstTimm,
                                double *ppflDiscAmount);
                                
/*
 * extern variables
 */

extern toenBool goenCustomerIsExempt;

extern long goilCustomerId;

extern toenBool goenIsDocumentValid;

/*
 * static functions
 */

static toenBool foenVATInv_Init(tostVATInv *ppstVATInv);

static toenBool foenVATInv_Save(tostVATInv *ppstVATInv);

/*
 * static variables
 */

static char szTemp[128];

/********************************************************************************************************************
 *
 * fpstVATInv_New
 *
 * DESCRIPTION:
 * Creates object containing info for processing tax in BGH for VAT invoice.
 *
 ********************************************************************************************************************
 */

tostVATInv *fpstVATInv_New(struct s_TimmInter *ppstInv)
{
  tostVATInv *lpstVATInv;  
  toenBool loenStatus;
  struct s_group_22 *g_22;
  struct s_moa_seg *moa;
  int loiCat, n;

  fovdPushFunctionName ("fpstVATInv_New");

  /*
   * Alloc memory
   */

  if ((lpstVATInv = (tostVATInv *)calloc(1, sizeof(tostVATInv))) == NULL)
    {
      fovdPrintLog(LOG_DEBUG, "fpstVATInv_New: Can't calloc memory");
      fovdPopFunctionName();
      return NULL;
    }

  /*
   * Init
   */

  if ((loenStatus = foenVATInv_Init(lpstVATInv)) == FALSE)
    {
      fovdPrintLog(LOG_DEBUG, "fpstVATInv_New: Can't init struct");
      fovdPopFunctionName ();
      return NULL;
    }

  lpstVATInv->spstTimmInv = ppstInv;

  /*
   * Process each G22 item classifying and adding it's values to the class found
   */

  g_22 = ppstInv->timm->g_22;
  while (g_22)
    {
      if ((loiCat = foiServiceCat_Classify(g_22)) == VAT_INV_UNKNOWN_CAT)
        {
          fovdPrintLog(LOG_DEBUG, "fpstVATInv_New: Can't classify service of G22");
          fovdPopFunctionName ();
          return NULL;
        }
      
      if ((loenStatus = foenServiceCat_Add(&(lpstVATInv->sastCatTab[loiCat]), 
                                           g_22, 
                                           &(lpstVATInv->sostSumList))) == FALSE)
        {
          fovdPrintLog(LOG_DEBUG, "fpstVATInv_New: Can't add service to category: %d", loiCat);
          fovdPopFunctionName ();
          return NULL;
        }
      
      g_22 = g_22->g_22_next;
    }

  /*
   * Invoice Amount
   */

  if ((moa = fpstFindMainPaymentSegment(ppstInv->timm->g_45, 
                                        "77", 
                                        "19")) == NULL)
    {
      fovdPopFunctionName ();
      return NULL;
    }

  n = sscanf(moa->v_5004, "%lf", &(lpstVATInv->soflBCH_TotalAmount));

  /*
   * Taxable Amount
   */

  if ((moa = fpstFindTaxPaymentSegment(ppstInv->timm->g_47, 
                                       "124", 
                                       MOA_TYPE_EXACT)) == NULL)
    {
      fovdPopFunctionName ();
      return NULL;
    }

  n = sscanf(moa->v_5004, "%lf", &(lpstVATInv->soflBCH_VATAmount));

  /*
   * To Pay Amount
   */

  if ((moa = fpstFindMainPaymentSegment(ppstInv->timm->g_45, 
                                        "178", 
                                        MOA_TYPE_TO_PAY)) == NULL)
    {
      fovdPopFunctionName ();
      return NULL;
    }

  n = sscanf(moa->v_5004, "%lf", &(lpstVATInv->soflBCH_PaymentAmount));

  /*
   * BCH rounding difference
   */

  if ((moa = fpstFindMainPaymentSegment(ppstInv->timm->g_45, 
                                        "980", 
                                        MOA_TYPE_EXACT)) == NULL)
    {
      fovdPopFunctionName ();
      return NULL;
    }

  n = sscanf(moa->v_5004, "%lf", &(lpstVATInv->soflBCH_RoundingDiff));

  /*
   * Basic discount amount
   */

  if (FALSE == foenTimm_GetDiscAmount(ppstInv,
                                      &(lpstVATInv->soflDiscountAmount)))
    {
      fovdPopFunctionName ();
      return NULL;
    }
  
  fovdPopFunctionName ();  
  return lpstVATInv;
}

/********************************************************************************************************************
 *
 * fpstVATInv_Init
 *
 ********************************************************************************************************************
 */

static toenBool foenVATInv_Init(tostVATInv *ppstVATInv)
{
  int i;
  toenBool loenStatus;

  fovdPushFunctionName ("fpstVATInv_Init");

  for (i = 0; i < VAT_INV_CAT_SIZE; i++)
    {
      if ((loenStatus = foenServiceCat_Init(&(ppstVATInv->sastCatTab[i]), i)) == FALSE)
        {
          fovdPopFunctionName ();
          return FALSE;
        }
    }

  if ((loenStatus = foenVATSumList_Init(&(ppstVATInv->sostSumList))) == FALSE)
    {
      fovdPopFunctionName ();
      return FALSE;
    }

  ppstVATInv->soflTotalNetAmount = 0.0;
  ppstVATInv->soflTotalBrutAmount = 0.0;
  ppstVATInv->soflVATAmount = 0.0;
  ppstVATInv->soflVATNetAmount = 0.0;
  ppstVATInv->soflVATBrutAmount = 0.0;
  ppstVATInv->soflNoVATAmount = 0.0;
  ppstVATInv->soflTotalNetAmount = 0.0;
  ppstVATInv->soflTotalBrutAmount = 0.0;

  ppstVATInv->soflBCH_PaymentAmount = 0.0;
  ppstVATInv->soflBCH_VATAmount = 0.0;
  ppstVATInv->soflBCH_TotalAmount = 0.0;
  ppstVATInv->soflBCH_RoundingDiff = 0.0;
  
  fovdPopFunctionName ();
  return TRUE;
}

/********************************************************************************************************************
 *
 * fpstVATInv_Delete
 *
 ********************************************************************************************************************
 */

toenBool foenVATInv_Delete(tostVATInv *ppstVATInv)
{
  int i;
  toenBool loenStatus;
  
  for (i = 0; i < VAT_INV_CAT_SIZE; i++)
    {
      if ((loenStatus = foenServiceCat_Delete(&(ppstVATInv->sastCatTab[i]))) == FALSE)
        {
          return FALSE;
        }
    }
  
  if ((loenStatus = foenVATSumList_Delete(&(ppstVATInv->sostSumList))) == FALSE)
    {
      return FALSE;
    }

  free(ppstVATInv);

  return TRUE;
}

/********************************************************************************************************************
 *
 * fpstVATInv_Gen
 *
 ********************************************************************************************************************
 */

toenBool foenVATInv_Gen(tostVATInv *ppstVATInv)
{
  int i, loiItems, rc, n;
  toenBool loenStatus;
  double loflRoundDiff;                        /* dounding difference between BGH amount and BCH amount of invoice */
  double loflRound;                            /* variable for macro ROUND */  
  char lasnzType[16];
  int loiCustomerId;
  double loflDiscAmountGranted;
  char lasnzItemsNo            [TMP_BUFFER_SIZE];
  char lasnzCurrency           [TMP_BUFFER_SIZE];
  static char lasnzTotalNet    [MAX_BUFFER_SIZE];
  static char lasnzTotalBrut   [MAX_BUFFER_SIZE];
  static char lasnzVATBrutSum  [MAX_BUFFER_SIZE];
  static char lasnzVATNetSum   [MAX_BUFFER_SIZE];
  static char lasnzVATSum      [MAX_BUFFER_SIZE];
  static char lasnzPayment     [MAX_BUFFER_SIZE];
  static char lasnzPaymentWords[MAX_BUFFER_SIZE];
  static char due_date         [MAX_BUFFER_SIZE];
  static char bank_name        [MAX_BUFFER_SIZE];
  static char account          [MAX_BUFFER_SIZE];
  static char lasnzInvoiceNo   [MAX_BUFFER_SIZE];
  static char lasnzNet         [MAX_BUFFER_SIZE];
  double loflRest;

  fovdPushFunctionName ("fpstVATInv_Gen");

  /*
   * Init
   */
  
  INIT_STATIC_BUF(lasnzTotalNet);
  INIT_STATIC_BUF(lasnzTotalBrut);
  INIT_STATIC_BUF(lasnzVATNetSum);
  INIT_STATIC_BUF(lasnzVATBrutSum);
  INIT_STATIC_BUF(lasnzVATSum);
  INIT_STATIC_BUF(lasnzPayment);
  INIT_STATIC_BUF(lasnzPaymentWords);
  INIT_STATIC_BUF(due_date);
  INIT_STATIC_BUF(bank_name);
  INIT_STATIC_BUF(account);
  INIT_STATIC_BUF(lasnzInvoiceNo);

  /*
   * Init
   */
  
  strncpy(lasnzCurrency, LOCAL_CURRENCY_CODE, TMP_BUFFER_SIZE);

  loiCustomerId = atol(ppstVATInv->spstTimmInv->unb->v_0010);

  /*
   * Check discount model
   */

  ppstVATInv->soflDiscountAmount = foflMoney_Round((-1) * ppstVATInv->soflDiscountAmount, 
                                                   &loflRound);

  goflDiscountAmount = ppstVATInv->soflDiscountAmount;
    
  /*
   * VAT item list
   */
  
  for (i = 0, loiItems = 0; i < VAT_INV_CAT_SIZE; i++)
    {
      if ((loenStatus = foenServiceCat_IsVATItem(&(ppstVATInv->sastCatTab[i]))) == TRUE && 
          (loenStatus = foenServiceCat_IsEmpty(&(ppstVATInv->sastCatTab[i]))) == FALSE)
        {
          loiItems++;
        }
    }

  if (ppstVATInv->soflDiscountAmount > 0.001)
    {
      loiItems++;
    }
  
  if (loiItems > 0)
    {
      sprintf(lasnzItemsNo, "%d", loiItems);
      fovdGen("InvoiceVATItemListStart", lasnzItemsNo, EOL);
      
      /*
       * Print summary amounts for each category
       */

      for (i = 0; i < VAT_INV_CAT_SIZE; i++)
        {
          if ((loenStatus = foenServiceCat_IsVATItem(&(ppstVATInv->sastCatTab[i]))) == TRUE)
            {
              if ((loenStatus = foenServiceCat_Gen(&(ppstVATInv->sastCatTab[i]))) == FALSE)
                {
                  fovdPopFunctionName ();
                  return FALSE;
                }
            }
        }

      /* 
       * It's time to check the discound model usage that is assigned only to
       * the local services - this must be genearated before the lst of taxes
       * is printed becouse the summary discount value is labanced here
       */

      if (ppstVATInv->soflDiscountAmount > 0.001)
        {          
          if ((loenStatus = foenVATSumList_GenDisc(&(ppstVATInv->sostSumList), 
                                                   ppstVATInv->soflDiscountAmount)) == FALSE)
            {
              fovdPopFunctionName ();
              return FALSE;              
            }
        }

      fovdGen("InvoiceVATItemListEnd", EOL);
    }

  /*
   * VAT Rates Sum list
   */

  if (goenCustomerIsExempt == FALSE || ppstVATInv->soflVATAmount > 0.0)
    {
      if ((loenStatus = foenVATSumList_Gen(&(ppstVATInv->sostSumList))) == FALSE)
        {
          fovdPopFunctionName ();
          return FALSE;
        }
    }

  /*
   * VAT summary
   */

  sprintf(lasnzVATNetSum, "%lf", ppstVATInv->soflVATNetAmount);
  fovdFormatMoney(lasnzVATNetSum);

  sprintf(lasnzVATSum, "%lf", ppstVATInv->soflVATAmount);
  fovdFormatMoney(lasnzVATSum);
  
  sprintf(lasnzVATBrutSum, "%lf", ppstVATInv->soflVATBrutAmount);
  fovdFormatMoney(lasnzVATBrutSum);

  /*
   * No VAT item list
   */

  for (i = 0, loiItems = 0; i < VAT_INV_CAT_SIZE; i++)
    {
      if (FALSE == foenServiceCat_IsVATItem(&(ppstVATInv->sastCatTab[i]))
          && FALSE == foenServiceCat_IsEmpty(&(ppstVATInv->sastCatTab[i])))
        {
          loiItems++;
        }
    }

  /*
   * This allows layout to guess what type of invoice we have Roaming or Local
   */

  if (loiItems > 0)
    {
      lasnzType[0] = 'R';
      lasnzType[1] = '\0';
    }
  else
    {
      lasnzType[0] = 'L';
      lasnzType[1] = '\0';
    }

  fovdGen("InvoiceVATSum", 
          lasnzVATNetSum, 
          lasnzVATSum, 
          lasnzVATBrutSum, 
          lasnzCurrency, 
          lasnzType, 
          EOL);
  
  fovdPrintLog (LOG_TIMM, 
                "VAT value: %s, %s, %s\n", 
                lasnzVATNetSum, 
                lasnzVATSum, 
                lasnzVATBrutSum);     
  
  if (loiItems > 0)
    {      
      sprintf(lasnzItemsNo, "%d", loiItems);
      fovdGen("InvoiceNoVATItemListStart", lasnzItemsNo, EOL);      
      
      for (i = 0; i < VAT_INV_CAT_SIZE; i++)
        {
          if ((loenStatus = foenServiceCat_IsVATItem(&(ppstVATInv->sastCatTab[i]))) == FALSE)
            {
              if ((loenStatus = foenServiceCat_Gen(&(ppstVATInv->sastCatTab[i]))) == FALSE)
                {
                  fovdPopFunctionName ();
                  return FALSE;
                }
            }
        }

      fovdGen("InvoiceNoVATItemListEnd", EOL);
    }
  
  /*
   * Total Net Sum
   */
  
  sprintf(lasnzTotalNet, "%lf", ppstVATInv->soflTotalNetAmount);  
  fovdFormatMoney(lasnzTotalNet);        
  fovdGen("InvoiceTotalNetSum", lasnzTotalNet, lasnzCurrency, EOL);
  
  /*
   * Total Brut Sum
   */
  
  sprintf(lasnzTotalBrut, "%lf", ppstVATInv->soflTotalBrutAmount);
  fovdFormatMoney(lasnzTotalBrut);        
  fovdGen("InvoiceTotalBrutSum", lasnzTotalBrut, lasnzCurrency, EOL);      
  
  /*
   * Payment: must update ORDERHDR_ALL when there is difference between
   * BCH Sum and BGH Sum
   */

  goflInvoicePayment = ppstVATInv->soflTotalBrutAmount;

  /*
   * Production of the amount to be payed by the customer
   */

  sprintf(lasnzPayment, "%lf", goflInvoicePayment);
  fovdFormatMoney(lasnzPayment);      
  if ((rc = moa2str_pl(lasnzPayment, 
                       MAX_BUFFER_SIZE, 
                       lasnzPaymentWords, 
                       MAX_BUFFER_SIZE)) != TRUE)
    {
      fovdPopFunctionName ();
      return FALSE;
    }
  
  fovdGen("InvoicePaymentWord", 
          lasnzPaymentWords, 
          lasnzPayment, 
          lasnzCurrency,          
          EOL);

  fovdPrintLog (LOG_CUSTOMER, "BGH Amount: %.02lf, Original BCH Amount: %.02lf\n", 
                ppstVATInv->soflTotalBrutAmount,
                ppstVATInv->soflBCH_TotalAmount); 
  
  /*
   * Payment conditions
   */

  fovdPrintLog (LOG_DEBUG, "Getting due date\n");     
  rc = fetchInvoiceDueDate(ppstVATInv->spstTimmInv, due_date, MAX_BUFFER);
  if (rc == FALSE)
    {
      fovdPopFunctionName ();
      return FALSE;
    }
  
  fovdPrintLog (LOG_DEBUG, "Getting invoice bank\n");     
  rc = fetchInvoiceBank(ppstVATInv->spstTimmInv, bank_name, MAX_BUFFER, account, MAX_BUFFER);
  if (rc == FALSE)
    {
      fovdPopFunctionName ();
      return FALSE;
    }
  
  fovdFormatDate(due_date, YY_MM_DD);
  fovdGen( "InvoicePayment", due_date, bank_name, account, EOL);
  
  fovdGen( "InvoiceEnd", EOL);

  fovdPopFunctionName ();
  return TRUE;
}

/********************************************************************************************************************
 *
 * fpstVATInv_Print
 *
 ********************************************************************************************************************
 */

toenBool foenVATInv_Print(tostVATInv *ppstVATInv)
{
  int i;
  toenBool loenStatus;

  for (i = 0; i < VAT_INV_CAT_SIZE; i++)
    {
      printf("Service Category: %s\n", gasnzServiceStr[i]);
      if ((loenStatus = foenServiceCat_Print(&(ppstVATInv->sastCatTab[i]))) == FALSE)
        {
          return FALSE;
        }

      printf("\n");
    }

  if ((loenStatus = foenVATSumList_Print(&(ppstVATInv->sostSumList))) == FALSE)
    {
      return FALSE;
    }

  return TRUE;
}

/********************************************************************************************************************
 *
 * fpstVATInv_Process
 *
 * DESCRIPTION:
 * Computes Net sum, Brut sum, VAT sum of VAT services from each service category shown on the invoice.
 * Checks partial sums for each VAT level using info got during creation of this object.
 * If necessary, updates ORDERHDR_ALL with correct VAT invoice value.
 *
 ********************************************************************************************************************
 */

toenBool foenVATInv_Process(tostVATInv *ppstVATInv, 
                            struct s_TimmInter *ppstInv)
{
  int i, n;
  toenBool loenStatus;
  double loflNet = 0.0;
  double loflVAT = 0.0;
  double loflBrut = 0.0;  
  double loflRound = 0.0;
  double loflRoamingAmount = 0.0;
  double loflRoundDiff = 0.0;
  double loflRest = 0.0;
  double loflLeftAmount =0.0;

  fovdPushFunctionName ("foenVATInv_Process");

  /*
   * Get exact VAT summary
   */

  loflNet = 0.0;
  loflVAT = 0.0;
  loflBrut = 0.0;  

  if ((loenStatus = foenVATSumList_Sum(&(ppstVATInv->sostSumList), 
                                       &loflNet, 
                                       &loflVAT, 
                                       &loflBrut)) == FALSE)
    {
      fovdPopFunctionName ();
      return FALSE;
}

  /*
   * Print ready rounding for VAT items
   */
  
  ppstVATInv->soflVATAmount = foflMoney_Round(loflVAT, 
                                              &loflRound);
  
  ppstVATInv->soflVATNetAmount = foflMoney_Round(loflNet, 
                                                 &loflRound);
  
  ppstVATInv->soflVATBrutAmount = foflMoney_Round(loflBrut, 
                                                  &loflRound);
  
  ppstVATInv->soflTotalNetAmount = foflMoney_Round(loflNet,
                                                   &loflRound);
  
  ppstVATInv->soflTotalBrutAmount = foflMoney_Round(loflBrut, 
                                                    &loflRound);
  /*
   * Adjust discount for VAT summary
   * Discount Model application is the trigger for amounts
   * adjustments
   */
  
  if (ppstVATInv->soflDiscountAmount != 0.0)
    {
      /* adjust net amount using Gross and VAT amount */

      ppstVATInv->soflVATNetAmount = ppstVATInv->soflVATBrutAmount - ppstVATInv->soflVATAmount;      
      ppstVATInv->soflTotalNetAmount = ppstVATInv->soflVATBrutAmount - ppstVATInv->soflVATAmount;      

      /* get net amount to be printed after rounding */

      loflLeftAmount = 0.0;
      for (i = 0; i < VAT_INV_CAT_SIZE; i++)
        {
          if (TRUE == foenServiceCat_IsVATItem(&(ppstVATInv->sastCatTab[i])))
            {
              loflNet = 0.0;
              loflVAT = 0.0; 
              loflBrut = 0.0;
              
              if (FALSE == foenServiceCat_Sum(&(ppstVATInv->sastCatTab[i]),
                                              &loflNet, 
                                              &loflVAT, 
                                              &loflBrut))
                {
                  fovdPopFunctionName ();
                  return FALSE;
                }

              loflLeftAmount += loflNet;
            }          
        }
      
      ppstVATInv->soflDiscountAmount = ppstVATInv->soflVATNetAmount - loflLeftAmount;
  
        /*
         * Rounding of VAT amount, and brut amount for each tax rate using discount amount
         */
      
      if (FALSE == foenVATSumList_Round(&(ppstVATInv->sostSumList),                                         
                                             ppstVATInv->soflDiscountAmount))
        {
          fovdPopFunctionName ();
          return FALSE;
        }
    }
    
  /*
   * VPLMN payment sum - no VAT items
   */
  
  loflRoamingAmount = 0.0;
  for (i = 0, ppstVATInv->soflNoVATAmount = 0.0; i < VAT_INV_CAT_SIZE; i++)
    {
      if ((loenStatus = foenServiceCat_IsVATItem(&(ppstVATInv->sastCatTab[i]))) == FALSE)
        {          
          if ((loenStatus = foenServiceCat_Round(&(ppstVATInv->sastCatTab[i]), 
                                                 &loflRound)) == FALSE)
            {
              fovdPopFunctionName ();
              return FALSE;
            }
          
          loflNet = 0.0;
          loflVAT = 0.0;
          loflBrut = 0.0;  
          
          if ((loenStatus = foenServiceCat_Sum(&(ppstVATInv->sastCatTab[i]), 
                                               &loflNet, 
                                               &loflVAT, 
                                               &loflBrut)) == FALSE)
            {
              fovdPopFunctionName ();
              return FALSE;
            }
          
          ppstVATInv->soflNoVATAmount += loflBrut;
          ppstVATInv->soflTotalNetAmount += loflBrut;
          ppstVATInv->soflTotalBrutAmount += loflBrut;          
          loflRoamingAmount += loflBrut;
        }
    }
  
  loflRoamingAmount = loflRoamingAmount;

  ppstVATInv->soflNoVATAmount = foflMoney_Round(ppstVATInv->soflNoVATAmount, 
                                                &loflRound);
  
  ppstVATInv->soflTotalNetAmount = foflMoney_Round(ppstVATInv->soflTotalNetAmount, 
                                                   &loflRound);

  ppstVATInv->soflTotalBrutAmount = foflMoney_Round(ppstVATInv->soflTotalBrutAmount, 
                                                    &loflRound); 

  fovdPopFunctionName ();
  return TRUE;
}

/********************************************************************************************************************
 *
 * foenGenInvoiceItemList
 *
 ********************************************************************************************************************
 */

toenBool foenGenInvoiceItemList(struct s_TimmInter *ppstInv)
{
  tostVATInv *lpstVATInv;
  toenBool loenStatus;

  fovdPushFunctionName ("foenGenInoiceItemList");

  if (NULL == (lpstVATInv = fpstVATInv_New(ppstInv)))
    {
      sprintf (szTemp, "foenGenInvoiceItemList: Can't create list of categories\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);  
      fovdPopFunctionName ();
      return FALSE;
    }
  
  if (FALSE == foenVATInv_Process(lpstVATInv, ppstInv))
    {
      sprintf (szTemp, "foenGenInvoiceItemList: Can't process list of categories\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName ();
      return FALSE;
    }
  
  if (FALSE == foenVATInv_Gen(lpstVATInv))
    {
      sprintf (szTemp, "foenGenInvoiceItemList: Can't generate output\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName ();
      return FALSE;
    }

  if (lpstVATInv->soflDiscountAmount != 0.0)
    {
      if (FALSE == foenVATInv_Save(lpstVATInv))
        {
          sprintf (szTemp, "foenGenInvoiceItemList: Can't save invoice strucure\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          fovdPopFunctionName ();
          return FALSE;
        }
    }

  if (FALSE == foenVATInv_Delete(lpstVATInv))
    {
      sprintf (szTemp, "foenGenInvoiceItemList: Can't delete list\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName ();
      return FALSE;
    }

  fovdPopFunctionName ();
  return TRUE;
}

/********************************************************************************************************************
 *
 * foenVATInv_Save
 *
 ********************************************************************************************************************
 */

static toenBool foenVATInv_Save(tostVATInv *ppstVATInv)
{
  int i;
  int rc = 0;
  toenBool loenStatus;
  struct tostOtEntrySeq *lpstSeq = NULL, *lpstTmp;

  /* load ORDERTARILER, match with INV */

  fovdPrintLog (LOG_DEBUG, "Getting OT entries\n");     

  if ((rc = foiTransx_GetOtEntries(&lpstSeq,
                                   ppstVATInv->spstTimmInv)) < 0)
    {
      return FALSE;
    }
   
  /*  save to DB with OTSEQ set */

  fovdPrintLog (LOG_DEBUG, "Inserting OT entries\n");     

  i = 0;
  while(lpstSeq != NULL)
    {
      if ((rc = foiVatInv_InsertOtEntry(lpstSeq)) < 0)
        {
          fovdPrintLog (LOG_CUSTOMER, "Can't insert OT entry no: %d\n", i);     
          return FALSE;
        }
      else
        {
          fovdPrintLog (LOG_DEBUG, "Inserted OT entry no: %d\n", i);     
          i++;          
        }

      lpstTmp = lpstSeq;
      lpstSeq = lpstSeq->spstNext;
      free(lpstTmp);
    }

  /* save summary info in DB */

  fovdPrintLog (LOG_DEBUG, "Saving inv summary info\n");     

  if (FALSE == foenVATSumList_Save(&(ppstVATInv->sostSumList)))
    {
      return FALSE;
    }

  return TRUE;
}
