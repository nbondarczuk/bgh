/**************************************************************************/
/*  MODULE : Enclosure Account Generator                                  */
/*                                                                        */
/*  AUTHOR : N.Bondarczuk                                                 */
/*                                                                        */
/*  CREATION DATE : 22.10.97                                              */
/*                                                                        */
/*  DESCRIPTION : Exports  function  creating tagged information          */
/*                necessary for creation of invoice account enclosure     */
/**************************************************************************/
#include <stdio.h>
#include <string.h>
#include "bgh.h"
#include "protos.h"
#include "parser.h"

#include "types.h"
#include "gen.h"
#include "strutl.h"
#include "set.h"
#include "stack.h"
#include "inv_item.h"
#include "inv_gen.h"
#include "inv_types.h"
#include "inv_fetch.h"
#include "num_pl.h"
#include "acc_enc_gen.h"
#include "occ_queue.h"
#include "vpn.h"
#include "workparval.h"
#include "env_gen.h"


#if 0	/* just for version.sh */
static char *SCCS_VERSION = "4.2";
#endif

/*
 * static functions
 */

static int foiAccountOCCItem_Gen(struct s_group_22 *g_22, double *ppflSumAmount);
static int foiAccountOCCList_Gen(struct s_TimmInter *, struct s_TimmInter *);

/*
 * static variables
 */

static char szTemp[128];
static struct s_TimmInter *spstInvoice, *spstSumSheet;
static char period_end[MAX_BUFFER];

/*
 * extern functions
 */

extern toenBool foenGenEnclosureAccountInvoiceHeader(); 
extern toenBool foenCheckNIP(char *);      
extern int fetchInvoiceItemsTypeNumber(struct s_TimmInter *, char);
extern toenBool foenFetchInvoiceItem(struct s_TimmInter *, int, tostInvItem *);
extern void fovdInvItem_Init(tostInvItem *);    
extern toenBool foenIsCustomerOCC(struct s_TimmInter *, char *, char *);

/*
 * extern variables
 */

extern stBGHGLOB	stBgh;			/* structure with globals for BGH */
extern float gofInvoiceType;
extern char gachzInvoiceType[MAX_BUFFER];
extern double goflInvoicePayment;
extern double goflDiscountAmount;

/**************************************************************************/
/*  FUNCTION : genEnclosureAccount                                        */
/*                                                                        */
/*  ARGUMENTS : IN sum_ti - SUMMARY SHEET TIMM structure                  */
/*              IN bal_ti - BALANCE SHEET TIMM structure                  */
/*              OUT out - output stream                                   */
/*                                                                        */
/*  RETURN : TRUE iff all functions evaluted to TRUE                      */
/*           FALSE iff first of the called functions evaluated as FALSE   */
/*                                                                        */
/*  DESCRIPTION : Creates in the output file al list of segments for      */
/*                invoice account enclosure. When error is fount in       */
/*                one of called functions return value is FALSE. No       */
/*                system services are called.                             */
/**************************************************************************/

toenBool genEnclosureAccount(TYPEID poenTypeId, 
                             struct s_TimmInter *inv_ti, 
                             struct s_TimmInter *sum_ti, 
                             struct s_TimmInter *bal_ti,
                             int poiAddrRuleNo) 
{
  int rc;
  static char str[200];
  toenBool loenStatus;
  toenMailingType loenMailing;

  spstInvoice = inv_ti;
  
  /*
   * ENVELOPE
   */

  if (poenTypeId == ENC_TYPE)
    {
      loenMailing = MAILING_ENC;
      
      if (FALSE == foenGenEnvelope(inv_ti,
                                   sum_ti, 
                                   loenMailing,
                                   poiAddrRuleNo))
        {
          sprintf (szTemp, "ERROR in function foenGenEnvelope.\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          fovdPopFunctionName ();
          return FALSE;
        }
    }
  
  fovdGen("AccountStart", EOL);

  fovdGen("AccountType", gachzInvoiceType, EOL);

  fovdPrintLog (LOG_DEBUG, "Account Header\n");     
  if ((rc = foenGenEnclosureAccountInvoiceHeader()) != TRUE) 
    {
      sprintf (szTemp, "ERROR creating account enclosure\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      return FALSE;
    }

  fovdPrintLog (LOG_DEBUG, "Account Balance\n");     
  if ((rc = genEnclosureAccountBalance(bal_ti)) != TRUE)
    {
      sprintf (szTemp, "ERROR creating account enclosure\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      return FALSE;
    }

  fovdPrintLog (LOG_DEBUG, "Account Summary\n");     
  if ((rc = genEnclosureAccountSummary(sum_ti, bal_ti)) != TRUE)
    {
      sprintf (szTemp, "ERROR creating account enclosure\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      return FALSE;
    }

  fovdPrintLog (LOG_DEBUG, "Account Sim List\n");     
  if ((rc = genEnclosureAccountSimList(sum_ti)) != TRUE)
    {
      sprintf (szTemp, "ERROR creating account enclosure\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      return FALSE;
    }

  fovdPrintLog (LOG_DEBUG, "VPN Contract  List\n");     
  if ((rc = foenVPNSubscriberList_Gen(sum_ti)) != TRUE)
    {
      sprintf (szTemp, "ERROR creating account enclosure\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      return FALSE;
    }

  fovdPrintLog (LOG_DEBUG, "Account OCC\n");     
  if ((rc = foiAccountOCCList_Gen(inv_ti, sum_ti)) != TRUE)
    {
      sprintf (szTemp, "ERROR creating account enclosure\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      return FALSE;
    }

  fovdPrintLog (LOG_DEBUG, "Account Trailer\n");     
  if ((rc = genEnclosureAccountTrailer(sum_ti)) != TRUE) 
    {
      sprintf (szTemp, "ERROR creating account enclosure\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      return FALSE;
    }

  if ((rc = foiParVal_AccGen(sum_ti)) < 0)
    {
      sprintf (szTemp, "ERROR creating WORKPARVAL info\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      return FALSE;
    }

  fovdGen("AccountEnd", EOL);

  return TRUE;
}

/**************************************************************************/
/*  FUNCTION : genEnclosureAccountHeader                                  */
/*                                                                        */
/*  ARGUMENTS : IN sum_ti - SUMMARY SHEET TIMM structure                  */
/*              OUT out - output stream                                   */
/*                                                                        */
/*  RETURN : TRUE iff all functions evaluted to TRUE                      */
/*           FALSE iff first of the called functions evaluated as FALSE   */
/*                                                                        */
/* DESCRIPTION : Generates tag strings in the output file with infornation*/
/*               from the header of invoice being produced.               */
/**************************************************************************/



int genEnclosureAccountHeader(struct s_TimmInter *ti) 
{
  char date[MAX_BUFFER];
  char address[4][MAX_BUFFER];
  char name[MAX_BUFFER];
  char customer_account[MAX_BUFFER];
  char customer_nip[MAX_BUFFER];
  char seller_nip[MAX_BUFFER];
  char period_begin[MAX_BUFFER];
  char period_end[MAX_BUFFER];
  char invoice_number[MAX_BUFFER];
  int rc, i, n;
  toenBool loenStatus;
  
  /*
   * THE BEGINNING
   */

  fovdGen("AccountStart", EOL); 
  if ((rc = fetchInvoiceDate(ti, date, MAX_BUFFER)) == FALSE)
    {
      return FALSE;
    }

  fovdFormatDate(date, YY_MM_DD);
  fovdGen("AccountHeader", date, EOL);

  /* 
   * CUSTOMER DATA
   */

  name[0] = EOS;
  if ((rc = fetchInvoiceCustomerName(ti, name, MAX_BUFFER)) == FALSE)
    {
      return FALSE;
    }

  fovdGen("AccountCustomerName", name, EOL);

  for (i = 0; i < 4; i++) 
    {
      address[i][0] = EOS;
    }

  if ((rc = fetchInvoiceCustomerAddress(ti, 
                                        address[0], MAX_BUFFER, address[1], MAX_BUFFER, 
                                        address[2], MAX_BUFFER, address[3], MAX_BUFFER)) == FALSE)
    {
      return FALSE;
    }

  fovdGen("AccountCustomerAddress", address[0], address[1], address[2], address[3], EOL);

  if ((rc = fetchInvoiceCustomerAccountNo(ti, customer_account, MAX_BUFFER)) == FALSE)
    {
      return FALSE;
    }

  fovdGen("AccountCustomerAccount", customer_account, EOL);
  
  if ((rc = fetchInvoiceCustomerAccountNo(ti, customer_account, MAX_BUFFER)) == FALSE)
    {
      return FALSE;
    }

  fovdGen("AccountCustomerCode", customer_account, EOL);

  if ((rc = fetchInvoiceCustomerNIP(ti, customer_nip, MAX_BUFFER)) == FALSE)
    {
      return FALSE;
    }
  
  fovdPrintLog (LOG_DEBUG, "CUSTOMER NIP : %s\n", customer_nip); 

  loenStatus = foenCheckNIP(customer_nip);      
  if (loenStatus == TRUE)
    {
      /*
       * NIP not zero
       */

      fovdGen("AccountCustomerNIP", customer_nip, EOL);
      fovdPrintLog (LOG_DEBUG, "CUSTOMER NIP VALIDATED: %s\n", customer_nip); 
    }
  else
    {
      /*
       * NIP zero
       */

      fovdPrintLog (LOG_DEBUG, "CUSTOMER NIP SUPPRESSED : %s\n", customer_nip); 
    }
  

  /*
   * SELLER DATA
   */

  if ((rc = fetchInvoiceSellerName(ti, name, MAX_BUFFER)) == FALSE)
    {
      return FALSE;
    }

  fovdGen("AccountSellerName", name, EOL);

  for (i = 0; i < 4; i++) 
    {
      address[i][0] = '\0';
    }

  if ((rc = fetchInvoiceSellerAddress(ti, 
                                      address[0], MAX_BUFFER, address[1], MAX_BUFFER, 
                                      address[2], MAX_BUFFER, address[3], MAX_BUFFER)) == FALSE)
    {
      return FALSE;
    }

  fovdGen("AccountSellerAddress", address[0], address[1], address[2], address[3], EOL);

  if ((rc = fetchInvoiceSellerNIP(ti, seller_nip, MAX_BUFFER)) == FALSE)
    {
      return FALSE;
    }

  fovdGen("AccountSellerNIP", seller_nip, EOL);

  /*
   * TITLE
   */
  
  if ((rc = fetchInvoiceNumber(ti, invoice_number, MAX_BUFFER)) == FALSE)
    {
      return FALSE;
    }

  if ((rc = fetchInvoicePeriodBegin(ti, period_begin, MAX_BUFFER)) == FALSE)
    {
      return FALSE;
    }

  if ((rc = fetchInvoicePeriodEnd(ti, period_end, MAX_BUFFER)) == FALSE)
    {
      return FALSE;
    }

  fovdFormatDate(period_begin, YY_MM_DD);
  fovdFormatDate(period_end, YY_MM_DD);
  fovdGen("AccountTitle", invoice_number, period_begin, period_end, EOL);

  return TRUE;
}

/**************************************************************************/
/*  FUNCTION : genEnclosureAccountBalance                                 */
/*                                                                        */
/*                                                                        */
/*                                                                        */
/**************************************************************************/

int genEnclosureAccountBalance(struct s_TimmInter *bal_ti) 
{
  int iTns, i, iRet, iRc;
  char transaction_type[MAX_BUFFER], date[MAX_BUFFER], currency[MAX_BUFFER], amount[MAX_BUFFER];
  static char payment[MAX_BUFFER], invoice[MAX_BUFFER];

  if ((iRc = fetchAccountPreviousSaldo(bal_ti, currency, MAX_BUFFER, payment, MAX_BUFFER)) == FALSE) 
    {
      return FALSE;
    }

  fovdFormatMoney(payment);  
  fovdGen("AccountPreviousSaldo", payment, currency, EOL);  
  fovdPrintLog (LOG_DEBUG, "Prev Saldo : %s\n", payment);     

  fovdGen("AccountTransactionListStart", EOL); 
  fovdPrintLog (LOG_DEBUG, "Transaction list end\n");   
  
  iTns = fetchBalanceTransactionsNumber(bal_ti);
  fovdPrintLog (LOG_DEBUG, "Items : %d\n", iTns);   
  if (iTns > 0) 
    {
      for (i = 0; i < iTns; i++) 
        {
          fovdPrintLog (LOG_DEBUG, "Item : %d\n", i);   
          if ((iRet = fetchBalanceTransactionType(bal_ti, i, transaction_type, MAX_BUFFER)) != TRUE)
            {
              return FALSE;
            }

          fovdPrintLog (LOG_DEBUG, "Transaction type : %s\n", transaction_type);   

          /*
           * OVPMT transactions should not be printed
           */

          if (EQ(transaction_type, "OVPMT"))
            {
              continue;
            }

          /*
           * Special handling of transactions that may be connected with invoices
           */

          if (EQ(transaction_type, "OINVC") || EQ(transaction_type, "OVPMT"))
            {
              if ((iRet = fetchBalanceTransactionInvoice(bal_ti, i, invoice, MAX_BUFFER)) != TRUE)
                {
                  invoice[0] = '\0';
                }
              
              fovdPrintLog (LOG_DEBUG, "Invoice   : %s\n", invoice);   
            }
          else
            {
              invoice[0] = '\0';
            }

          if ((iRet = fetchBalanceTransactionDate(bal_ti, i, date, MAX_BUFFER)) != TRUE)
            {
              return FALSE;
            }

          fovdPrintLog (LOG_DEBUG, "Date     : %s\n", date);   

          if ((iRet = fetchBalanceTransactionCurrency(bal_ti, i, currency, MAX_BUFFER)) != TRUE)
            {
              return FALSE;
            }

          fovdPrintLog (LOG_DEBUG, "Currency : %s\n", currency);   

          if ((iRet = fetchBalanceTransactionAmount(bal_ti, i, amount, MAX_BUFFER)) != TRUE)
            {
              return FALSE; 
            }

          fovdPrintLog (LOG_DEBUG, "Amount   : %s\n", amount);             

          fovdFormatMoney(amount);  
          fovdFormatDate(date, YY_MM_DD);
          fovdGen("AccountTransactionItem", transaction_type, invoice, date, amount, currency, EOL );  
          fovdPrintLog (LOG_DEBUG, "Transaction Item : %s, %s, %s\n", transaction_type, date, amount);   
        }
    }
  
  fovdPrintLog (LOG_DEBUG, "Transaction list end\n");   
  fovdGen("AccountTransactionListEnd", EOL); 

  return TRUE;
}

/**************************************************************************/
/*  FUNCTION :                                                            */
/*                                                                        */
/*                                                                        */
/*                                                                        */
/**************************************************************************/

int genEnclosureAccountSummary(struct s_TimmInter *sum_ti, struct s_TimmInter *bal_ti) 
{
  static char currency[MAX_BUFFER], payment[MAX_BUFFER];
  int iRc, n;
  double loflPayment, loflBalance;

  fovdPushFunctionName ("genEnclosureAccountSummary");

  /*
   * OCC
   */

  fovdPrintLog (LOG_DEBUG, "Payment OCC\n"); 
  if ((iRc = fetchAccountOthersPayment(sum_ti, currency, MAX_BUFFER, payment, MAX_BUFFER)) == FALSE) 
    {
      sprintf (szTemp, "Can't get OCC charges from SUM-SHEET\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);      
      fovdPopFunctionName ();
      return FALSE; 
    }
  
  fovdFormatMoney(payment);  
  fovdGen("AccountOthersPayment", payment, currency, EOL);  


  /*
   * Access
   */

  fovdPrintLog (LOG_DEBUG, "Payment Access\n"); 
  if ((iRc = fetchAccountAccessPayment(sum_ti, currency, MAX_BUFFER, payment, MAX_BUFFER)) == FALSE) 
    {
      sprintf (szTemp, "Can't get ACC charges from SUM-SHEET\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);      
      fovdPopFunctionName ();
      return FALSE;   
    }

  fovdFormatMoney(payment);  
  fovdGen("AccountAccessPayment", payment, currency, EOL);


  /*
   * Usage
   */

  fovdPrintLog (LOG_DEBUG, "Payment Usage\n"); 
  if ((iRc = fetchAccountUsagePayment(sum_ti, currency, MAX_BUFFER, payment, MAX_BUFFER)) == FALSE) 
    {
      sprintf (szTemp, "Can't get USG charges from SUM-SHEET\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);      
      fovdPopFunctionName ();
      return FALSE;
    }

  fovdFormatMoney(payment);  
  fovdGen("AccountUsagePayment", payment, currency, EOL);


  /*
   * Subs
   */

  fovdPrintLog (LOG_DEBUG, "Payment Subs\n"); 
  if ((iRc = fetchAccountSubscriptionPayment(sum_ti, currency, MAX_BUFFER, payment, MAX_BUFFER)) == FALSE) 
    {
      sprintf (szTemp, "Can't get SUB charges from SUM-SHEET\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);      
      fovdPopFunctionName ();
      return FALSE; 
    }

  fovdFormatMoney(payment);  
  fovdGen("AccountServicePayment", payment, currency, EOL);
  
  /*
   * Summary
   */

  fovdPrintLog (LOG_DEBUG, "Payment Sumary\n"); 
  if ((iRc = fetchAccountSummaryPayment(sum_ti, currency, MAX_BUFFER, payment, MAX_BUFFER)) == FALSE) 
    {
      sprintf (szTemp, "Can't get sum of charges from SUM-SHEET\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);      
      fovdPopFunctionName ();
      return FALSE; 
    }

  fovdFormatMoney(payment);  
  fovdGen("AccountSummaryPayment", payment, currency, EOL);  
  
  /*
   * Invoice value
   */

  /*
  fovdPrintLog (LOG_DEBUG, "Payment Invoice\n"); 
  if ((iRc = fetchBalanceInvoicePayment(bal_ti, currency, MAX_BUFFER, payment, MAX_BUFFER)) == FALSE) 
    {
      fovdPopFunctionName ();
      return FALSE; 
    }
  
  n = sscanf(payment, "%lf", &loflPayment);
  ASSERT(n == 1);
  */

  loflPayment = goflInvoicePayment;
  sprintf(payment, "%lf", goflInvoicePayment);
  fovdFormatMoney(payment);  
  fovdGen("AccountInvoicePayment", payment, currency, EOL);  

  /*
   * Saldo - must be eq to BCH balance at the virtual start date val. + invoice amount
   */

  fovdPrintLog (LOG_DEBUG, "Actul saldo\n"); 
  if ((iRc = fetchAccountActualSaldo(bal_ti, currency, MAX_BUFFER, payment, MAX_BUFFER)) == FALSE) 
    {
      sprintf (szTemp, "Can't get actual saldo in BAL-SHEET\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);      
      fovdPopFunctionName ();
      return FALSE;
    }

  n = sscanf(payment, "%lf", &loflBalance);
  ASSERT(n == 1);  
  loflBalance = foflRound(loflBalance) + foflRound(loflPayment);  
  fovdPrintLog (LOG_DEBUG, "genEnclosureAccountSummary: Balance is: %lf\n", loflBalance); 
  sprintf(payment, "%lf", loflBalance);    

  fovdFormatMoney(payment);  

  fovdGen("AccountActualSaldo", period_end, payment, currency, EOL);  

  fovdPopFunctionName ();
  return TRUE;
}

/**************************************************************************/
/*  FUNCTION : genEnclosureAccountSimList                                 */
/*                                                                        */
/*  ARGUMENTS : IN sum_ti - SUMMARY SHEET TIMM structure                  */
/*              OUT out - output stream                                   */
/*                                                                        */
/*  RETURN : TRUE iff all functions evaluted to TRUE                      */
/*           FALSE iff first of the called functions evaluated as FALSE   */
/*                                                                        */
/*  DESCRIPTION : Count the number of contracts in SUM msg. For each of   */
/*                them find payments summary amount. Produce list of      */
/*                tuples with signature <SIM, RATE PLAN NAME, CURRENCY,   */
/*                VALUE>.                                                 */ 
/**************************************************************************/

int genEnclosureAccountSimList(struct s_TimmInter *sum_ti) 
{
  int loiSubscribersNo, loiSubscriber;
  int loiContractsNo, loiContract;
  static char lachzDirectoryNumber[MAX_BUFFER], lachzCurrency[MAX_BUFFER]; 
  static char lachzAmount[MAX_BUFFER], lachzSubscriberCode[MAX_BUFFER];
  static char lachzMarket[MAX_BUFFER], lachzMarketCode[MAX_BUFFER], lachzSM[MAX_BUFFER];
  static char lachzAccount[MAX_BUFFER], lachzNC[MAX_BUFFER];
  int iRet, loiSize;
  char lasnzSize[16];
  int foiCountSubscribers(struct s_TimmInter *);
  int foiCountSubscriberContracts(struct s_TimmInter *, int);

  fovdPushFunctionName ("genEnclosureAccountSimList");

  /*
   * Count list size
   */
 
  loiSize = 0;
  loiSubscribersNo = foiCountSubscribers(sum_ti);
  for (loiSubscriber = 0; loiSubscriber < loiSubscribersNo; loiSubscriber++) 
    {
      loiContractsNo = foiCountSubscriberContracts(sum_ti, loiSubscriber);
      for (loiContract = 0; loiContract < loiContractsNo; loiContract++) 
        {
          loiSize++;
        }

      loiSize++;      
    }

  sprintf(lasnzSize, "%d", loiSize);
  fovdGen("AccountPhonePaymentItemListStart", lasnzSize, EOL);

  loiSubscribersNo = foiCountSubscribers(sum_ti);
  for (loiSubscriber = 0; loiSubscriber < loiSubscribersNo; loiSubscriber++) 
    {
      loiContractsNo = foiCountSubscriberContracts(sum_ti, loiSubscriber);
      for (loiContract = 0; loiContract < loiContractsNo; loiContract++) 
        {
          fovdPrintLog (LOG_DEBUG, "Loading Contract Market\n");     
          iRet = fetchContractMarket(sum_ti, loiSubscriber, loiContract, lachzMarket, MAX_BUFFER, lachzMarketCode, MAX_BUFFER);
          if (iRet == FALSE)
            {
              fovdPopFunctionName ();
              return FALSE;
            }

          fovdPrintLog (LOG_DEBUG, "Loading Contract DN\n");     
          iRet = fetchContractDirectoryNumber(sum_ti, loiSubscriber, loiContract, lachzDirectoryNumber, MAX_BUFFER);
          if (iRet == FALSE)
            {
              fovdPopFunctionName ();
              return FALSE;
            }

          fovdPrintLog (LOG_DEBUG, "Loading Contract SIM\n");     
          iRet = fetchContractStorageMedium(sum_ti, loiSubscriber, loiContract, lachzSM, MAX_BUFFER);
          if (iRet == FALSE)
            {
              fovdPopFunctionName ();
              return FALSE;
            }

          fovdPrintLog (LOG_DEBUG, "Loading Contract Amount\n");
          iRet = fetchContractAmount(sum_ti, loiSubscriber, loiContract, lachzAmount, MAX_BUFFER, lachzCurrency, MAX_BUFFER);
          if (iRet == FALSE)
            {
              fovdPopFunctionName ();
              return FALSE;
            }
          
          fovdFormatMoney(lachzAmount);  
          fovdGen("AccountPhonePaymentItem", 
                  lachzMarket, lachzMarketCode, lachzDirectoryNumber, lachzSM, lachzAmount, lachzCurrency, 
                  EOL);      
        }

      /*
       * Sum up each subscriber
       */

      iRet = fetchSubscriberAccountPayment(sum_ti, loiSubscriber,
                                           lachzAccount, MAX_BUFFER,
                                           lachzNC, MAX_BUFFER,
                                           lachzAmount, MAX_BUFFER, 
                                           lachzCurrency, MAX_BUFFER);
      if (iRet == FALSE)
        {
          fovdPopFunctionName ();
          return FALSE;
        }
      
      fovdFormatMoney(lachzAmount);
      fovdGen("AccountSubscriberPayment", 
              lachzAccount, lachzNC, lachzAmount, lachzCurrency, 
              EOL);
    }
  
  fovdGen("AccountPhonePaymentItemListEnd", EOL);

  fovdPopFunctionName ();
  return TRUE;
}

/**************************************************************************/
/*  FUNCTION : genEnclosureAccountTrailer                                 */
/*                                                                        */
/*  ARGUMENTS : IN sum_ti - SUMMARY SHEET TIMM structure                  */
/*              OUT out - output stream                                   */
/*                                                                        */
/*  RETURN : TRUE iff all functions evaluted to TRUE                      */
/*           FALSE iff first of the called functions evaluated as FALSE   */
/*                                                                        */
/*  DESCRIPTION : End of enclosure marker. Nothing special.               */
/**************************************************************************/

int genEnclosureAccountTrailer(struct s_TimmInter *ti) 
{
  return TRUE;
}

toenBool foenGenEnclosureAccountInvoiceHeader() 
{
  char date[MAX_BUFFER];
  char address[4][MAX_BUFFER];
  char name[MAX_BUFFER];
  char customer_account[MAX_BUFFER];
  char customer_nip[MAX_BUFFER];
  char seller_nip[MAX_BUFFER];
  char period_begin[MAX_BUFFER];
  char invoice_number[MAX_BUFFER];
  toenBool loenStatus;
  int rc, i, n;

  fovdPushFunctionName ("foenGenInvoiceHeader");

  fovdPrintLog (LOG_DEBUG, "HEADER START\n");

  /*
   * CUSTOMER NAME, ADDRESS
   */
  
  rc = fetchInvoiceCustomerName(spstInvoice, name, MAX_BUFFER);
  if (rc == FALSE)
    {
      sprintf (szTemp, "ERROR in function foenFetchInvoiceCustomerName\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName ();
      return FALSE;
    }
  
  fovdGen("AccountCustomerName", name, EOL); 
  fovdPrintLog (LOG_DEBUG, "CUSTOMER NAME : %s\n", name); 

  for (i = 0; i < 4; i++) 
    {
      address[i][0] = EOS;
    }
  rc = fetchInvoiceCustomerAddress(spstInvoice, 
                                   address[0], MAX_BUFFER, address[1], MAX_BUFFER, 
                                   address[2], MAX_BUFFER, address[3], MAX_BUFFER);
  if (rc == FALSE)
    {
      sprintf (szTemp, "ERROR in function foenFetchInvoiceCustomerAddress\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName ();
      return FALSE;
    }

  fovdGen("AccountCustomerAddress", address[0], address[1], address[2], address[3], EOL); 
  /*
  fovdPrintLog (LOG_DEBUG, "CUSTOMER ADDRESS : %s %s %s %s\n",  address[0], address[1], address[2], address[3]); 
  */

  /*
   * CUSTOMER ACCOUNT NO
   */

  rc = fetchInvoiceCustomerAccountNo(spstInvoice, customer_account, MAX_BUFFER);
  if (rc == FALSE)
    {
      sprintf (szTemp, "ERROR in function foenFetchInvoiceCustomerAccountNo\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);      
      fovdPopFunctionName ();
      return FALSE;
    }

  fovdGen("AccountCustomerAccount", customer_account, EOL);
  fovdPrintLog (LOG_DEBUG, "CUSTOMER ACCOUNT NO : %s\n", customer_account); 


  /*
   * CUSTOMER NIP
   */
  
  rc = fetchInvoiceCustomerNIP(spstInvoice, customer_nip, MAX_BUFFER);
  if (rc == FALSE)
    {
      sprintf (szTemp, "ERROR in function foenFetchInvoiceCustomerNIP\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);     
      fovdPopFunctionName ();
      return FALSE;
    }

  fovdPrintLog (LOG_DEBUG, "CUSTOMER NIP : %s\n", customer_nip); 

  loenStatus = foenCheckNIP(customer_nip);      
  if (loenStatus == TRUE)
    {
      /*
       * NIP not zero
       */

      fovdGen("AccountCustomerNIP", customer_nip, EOL);
      fovdPrintLog (LOG_DEBUG, "CUSTOMER NIP VALIDATED: %s\n", customer_nip); 
    }
  else
    {
      /*
       * NIP zero
       */

      fovdPrintLog (LOG_DEBUG, "CUSTOMER NIP SUPPRESSED : %s\n", customer_nip); 
    }

  /*
   * NAME
   */

  rc = fetchInvoiceSellerName(spstInvoice, name, MAX_BUFFER);
  if (rc == FALSE)
    {
      sprintf (szTemp, "ERROR in function foenFetchInvoiceSellerName\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName ();
      return FALSE;
    }

  fovdGen("AccountSellerName", name, EOL);  
  fovdPrintLog (LOG_DEBUG, "SELLER NAME : %s\n", name); 


  for (i = 0; i < 4; i++) 
    {
      address[i][0] = '\0';
    }
  rc = fetchInvoiceSellerAddress(spstInvoice, 
                                 address[0], MAX_BUFFER, address[1], MAX_BUFFER, 
                                 address[2], MAX_BUFFER, address[3], MAX_BUFFER);
  if (rc == FALSE)
    {
      sprintf (szTemp, "ERROR in function foenFetchInvoiceSellerAddress\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName ();      
      return FALSE;
    }

  fovdGen("AccountSellerAddress", name, "", address[1], address[2], EOL); 
  fovdPrintLog (LOG_DEBUG, "SELLER ADDRESS : %s %s %s %s\n",  address[0], address[1], address[2], address[3]); 

  /*
   * SELLER NIP
   */

  rc = fetchInvoiceSellerNIP(spstInvoice, seller_nip, MAX_BUFFER);
  if (rc == FALSE)
    {
      sprintf (szTemp, "ERROR in function foenFetchInvoiceSellerNIP\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);    
      fovdPopFunctionName ();
      return FALSE;
    }

  fovdGen("AccountSellerNIP", seller_nip, EOL); 
  fovdPrintLog (LOG_DEBUG, "SELLER NIP : %s\n", seller_nip); 


  /*
   * INVOICE DATE
   */

  rc = fetchInvoiceDate(spstInvoice, date, MAX_BUFFER);
  if (rc == FALSE) 
    {
      sprintf (szTemp, "ERROR in function foenFetchInvoiceDate\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName ();
      return FALSE;
    }
  
  fovdFormatDate(date, YY_MM_DD);
  fovdGen("AccountHeader", date, EOL);
  fovdPrintLog (LOG_DEBUG, "DATE : %s\n", date); 


  /*
   * INVOICE NUMBER, PERIOD BEGIN, PERIOD END
   */

  rc = fetchInvoiceNumber(spstInvoice, invoice_number, MAX_BUFFER);
  if (rc == FALSE)
    {
      sprintf (szTemp, "ERROR in function foenFetchInvoiceNumber\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName ();
      return FALSE;
    }

  rc = fetchInvoicePeriodBegin(spstInvoice, period_begin, MAX_BUFFER);
  if (rc == FALSE)
    {
      sprintf (szTemp, "ERROR in function foenFetchInvoicePeriodBegin\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);      
      fovdPopFunctionName ();
      return FALSE;
    }

  rc = fetchInvoicePeriodEnd(spstInvoice, period_end, MAX_BUFFER);
  if (rc == FALSE)
    {
      sprintf (szTemp, "ERROR in function foenFetchInvoicePeriodEnd\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);      
      fovdPopFunctionName ();      
      return FALSE;
    }

  fovdFormatDate(period_begin, YY_MM_DD);
  fovdFormatDate(period_end, YY_MM_DD);
  fovdGen("AccountTitle", invoice_number, period_begin, period_end, EOL); 
  fovdPrintLog (LOG_DEBUG, "NUMBER : %s, BEGIN : %s, END : %s\n", invoice_number, period_begin, period_end);   

  fovdPrintLog (LOG_DEBUG, "HEADER END\n");
  
  fovdPopFunctionName ();
  
  return TRUE;
}

/************************************************************************************************************
 *
 * foiGenEnclosureAccountOCCList
 *
 * DESCRIPTION:
 * Converts list from SUM TIM message G22 item with LIN.v_7140 = 'O' - OCC  to PS format
 *
 ************************************************************************************************************
 */

static int foiAccountOCCList_Gen(struct s_TimmInter *ppstInvTimm, struct s_TimmInter *ppstSumTimm)
{
  struct s_group_22 *g_22;
  struct s_lin_seg *lin;
  double loflSumAmount = 0.0;
  int loiItemsNo = 0, rc = 0;
  static char lasnzItemsNo[MAX_BUFFER];
  static char lasnzSumAmount[MAX_BUFFER];
  static char laszDiscountAmount[MAX_BUFFER];

  /*
   * Count matching items
   */

  g_22 = ppstSumTimm->timm->g_22;
  while (g_22) 
    {
      lin = g_22->lin;
      if (lin && lin->v_1222 && EQ(lin->v_1222, "02") && lin->v_7140 && lin->v_7140[0] == 'O')
        {                 
          /*
           * This is OCC for a contract
           */
          
          loiItemsNo++;
        }

      g_22 = g_22->g_22_next;
    }

  if (loiItemsNo == 0 && goflDiscountAmount < 0.01)
    {
      /*
       * No OCC in general so suppress printing list
       */
      
      return TRUE;
    }
  
  /*
   * Gen whole list
   */
  
  sprintf(lasnzItemsNo, "%d", loiItemsNo);
  fovdGen("AccountOCCItemListStart", lasnzItemsNo, EOL);

  g_22 = ppstSumTimm->timm->g_22;
  while (g_22) 
    {
      lin = g_22->lin;
      if (lin && lin->v_1222 && EQ(lin->v_1222, "02") && lin->v_7140 && lin->v_7140[0] == 'O')
        {                 
          /*
           * This is OCC for a contract
           */

          if ((rc = foiAccountOCCItem_Gen(g_22, &loflSumAmount)) == FALSE)
            {
              return FALSE;
            }
        }

      g_22 = g_22->g_22_next;
    }
  
  if (goflDiscountAmount >= 0.01)
    {
      sprintf(laszDiscountAmount, "%lf", (-1.0) * goflDiscountAmount);
      fovdFormatMoney(laszDiscountAmount);
      
      fovdGen("AccountOCCItem", 
              "",                      /* Tariff Modell */
              "",                      /* Service Name */
              "Rabat",                 /* Fee description if OCC */
              laszDiscountAmount,      /* Item net amount */ 
              "",                      /* Item tax name */
              "",                      /* Item tax amount */
              "",                      /* Item brut amount */
              EOL);      
    }

  fovdGen("AccountOCCItemListEnd", EOL);

  /*
   * Gen summary amount for all items
   */

  if (goflDiscountAmount >= 0.01)
    {
      loflSumAmount -= goflDiscountAmount;
    }

  sprintf(lasnzSumAmount, "%lf", loflSumAmount);
  fovdFormatMoney(lasnzSumAmount);
  fovdGen("AccountOCCSum", lasnzSumAmount, "", "", EOL);

  return TRUE;
}

/************************************************************************************************************
 *
 * foiAccountOCCItem_Gen
 *
 * DESCRIPTION:
 * Converts item from SUM TIM message G22 item with LIN.v_7140 = 'O' - OCC  to PS format
 *
 ************************************************************************************************************
 */

static int foiAccountOCCItem_Gen(struct s_group_22 *g_22, double *ppflSumAmount)
{
  struct s_group_23 *g_23;
  struct s_lin_seg *lin;
  struct s_imd_seg *imd;
  struct s_moa_seg *moa;
  double loflAmount;
  int n;
  static char lasnzAmount[MAX_BUFFER];
  static char lasnzFeeDesc[MAX_BUFFER];

  lin = g_22->lin;
  
  /*
   * Find Fee Description
   */

  imd = g_22->imd;
  lasnzFeeDesc[0] = '\0';
  while (imd)
    {
      /*
       * Get full description of OCC item
       */

      strncat(lasnzFeeDesc, imd->v_7008a, MAX_BUFFER);
      if (imd->imd_next != NULL)
        {
          strncat(lasnzFeeDesc, " ", MAX_BUFFER);
        }

      imd = imd->imd_next;
    }
  
  /*
   * Find Net Amount in MOA+125
   */

  moa = NULL;
  g_23 = g_22->g_23;
  while (g_23)
    {
      /*
       * Find MOA+125 with rounded value
       */

      if (g_23->moa && g_23->moa->v_5025 && EQ(g_23->moa->v_5025, "125") && EQ(g_23->moa->v_4405, "19"))
        {
          moa = g_23->moa;
          break;
        }
      
      g_23 = g_23->g_23_next;
    }
  
  if (moa == NULL)
    {
      return FALSE;
    }

  if ((n = sscanf(moa->v_5004, "%lf", &loflAmount)) != 1)
    {
      return FALSE;
    }

  strcpy(lasnzAmount, moa->v_5004);
  *ppflSumAmount += loflAmount;
             
  /*
   * Add to the output queue
   */

  fovdFormatMoney(lasnzAmount);
  fovdGen("AccountOCCItem", 
          "",                      /* Tariff Modell */
          "",                      /* Service Name */
          lasnzFeeDesc,            /* Fee description if OCC */
          lasnzAmount,             /* Item net amount */ 
          "",                      /* Item tax name */
          "",                      /* Item tax amount */
          lasnzAmount,             /* Item brut amount */
          EOL);
  
  return TRUE;
}

  
              
              
