
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
#include "inv_gen.h"
#include "inv_item.h"
#include "inv_types.h"
#include "inv_fetch.h"
#include "num_pl.h"
#include "shdes2des.h"
#include "env_gen.h"
#include "pay_slip_gen.h"

#if 0	/* just for version.sh */
static char *SCCS_VERSION = "4.1.1";
#endif

#define CR110298

/*
 * extern variables
 */

extern stBGHGLOB stBgh;			/* structure with globals for BGH */
extern double goflInvoicePayment;

/*
 * static variables
 */

static char szTemp[128];

/*
 * static functions
 */  

char *spchzFormatInvoiceNumber(char *);
char *spchzFormatCustomerAccount(char *, int);
char *spchzCountCKSM(char *, char *, char *);

/****************************************************************************************************************
 *
 * FUNCTION: foenGenPaymentSlip
 *
 ****************************************************************************************************************
 */

toenBool foenGenPaymentSlip(struct s_TimmInter *ppstInvoice, 
                            struct s_TimmInter *ppstSumTimm, 
                            toenMailingType poenMailing, /* INV ENC or PAY */                                                           
                            int poiAddrRuleNo) /* rule number */
{  
    toenBool loenStatus;
    int rc = 0, i;
    static char name                [MAX_BUFFER];
    static char invoice_number      [MAX_BUFFER];
    static char adr              [4][MAX_BUFFER];
    static char customer_account    [MAX_BUFFER];
    static char bank_name           [MAX_BUFFER];
    static char account             [MAX_BUFFER];
    static char total_value         [MAX_BUFFER];
    static char amount_in_words     [MAX_BUFFER];
    static char currency            [MAX_BUFFER];
    static char lachzCustomerAccount[MAX_BUFFER];
    static char lachzInvoiceNumber  [MAX_BUFFER];
    char laszDueDate                [MAX_BUFFER];
    char *lpchzSellerAccountBankId;
    char *lpchzCustomerAccount;
    char *lpchzInvoiceNumber;
    char *lpchzCKSM;
    toenBool loenPrintMarketingText; 
    
    fovdPushFunctionName ("foenGenPaymentSlip");

    /*
     * Invocation with NULL means that Temporary address of the default
     * EDI message is to be used; not hte bill address of the TIMM from 
     * argument.
     */

  
    if (poenMailing == MAILING_PAY) 
    {
	fovdGen("EnvelopeBank", EOL); 
	if (FALSE == foenGenEnvelope(ppstInvoice, 
				     ppstSumTimm,
				     poenMailing,
				     poiAddrRuleNo))
        {
	    sprintf (szTemp, "ERROR in function foenGenEnvelope\n");
	    macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
	    fovdPopFunctionName ();
	    return FALSE;
        }
    }

    fovdGen("PaymentSlipStart", EOL);
  
    /*
     * SELLER NAME
     */

    rc = fetchInvoiceSellerName(ppstInvoice, name, MAX_BUFFER);
    if (rc != TRUE)
    {
	sprintf (szTemp, "ERROR in function foenFetchInvoiceSellerName.\n");
	macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
	fovdPopFunctionName ();
	return FALSE;
    }

    fovdGen("PaymentSlipSellerName", name, EOL);  

    for (i = 0; i < 4; i++) 
    {
	memset(adr[i], 0, MAX_BUFFER);
    }
  
    rc = fetchInvoiceSellerAddress(ppstInvoice, 
				   adr[0], MAX_BUFFER, 
				   adr[1], MAX_BUFFER, 
				   adr[2], MAX_BUFFER, 
				   adr[3], MAX_BUFFER);
    if (rc != TRUE)
    {
	sprintf (szTemp, "ERROR in function foenFetchInvoiceSellerAddress.\n");
	macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
	fovdPopFunctionName ();      
	return FALSE;
    }
  
    fovdGen("PaymentSlipSellerAddress", name, "", adr[1], adr[2], EOL); 

    /*
     * CUSTOMER ACCOUNT
     */

    memset(customer_account, 0, MAX_BUFFER);
    rc = fetchInvoiceCustomerAccountNo(ppstInvoice, customer_account, MAX_BUFFER);
    if (rc != TRUE)
    {
	sprintf (szTemp, "ERROR in function fetchInvoiceCustomerAccountNo\n");
	macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);      
	fovdPopFunctionName ();
	return FALSE;
    }
  
    fovdPrintLog (LOG_DEBUG, "Customer Account No : %s\n", customer_account); 
    fovdGen("PaymentSlipAccount", customer_account, EOL);

    /*
     * SELLER BANK
     */

    memset(bank_name, 0, MAX_BUFFER);
    memset(account, 0, MAX_BUFFER);  
    loenStatus = fetchInvoiceBank(ppstInvoice, bank_name, MAX_BUFFER, account, MAX_BUFFER);
    if (loenStatus != TRUE)
    {
	sprintf (szTemp, "ERROR in function fetchInvoiceBank.\n");
	macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
	fovdPopFunctionName ();
	return FALSE;
    }

    fovdGen("PaymentSlipSellerBank", bank_name, account, EOL);

    /*
     * INVOICE VALUE
     */

    memset(total_value, 0, MAX_BUFFER);
    memset(amount_in_words, 0, MAX_BUFFER);

    sprintf(total_value, "%lf", goflInvoicePayment);
    strcpy(currency, "PLN");

    /*
      rc = fetchInvoiceTotalValue(ppstInvoice, total_value, MAX_BUFFER, currency, MAX_BUFFER);
      if (rc != TRUE)
      {
      sprintf (szTemp, "ERROR in function fetchInvoiceTotalValue.\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName ();
      return FALSE;
      }
    */
  
    fovdFormatMoney(total_value);
    rc = moa2str_pl(total_value, MAX_BUFFER, amount_in_words, MAX_BUFFER);
    if (rc != TRUE)
    {
	sprintf (szTemp, "ERROR in function fetchInvoiceBank.\n");
	macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
	fovdPopFunctionName ();
	return FALSE;
    }
  
    fovdPrintLog (LOG_TIMM, "Payment: %s, %s\n", total_value, amount_in_words);       
    fovdGen("PaymentSlipAmount", 
	    amount_in_words, 
	    total_value, 
	    currency, 
	    EOL);


    /*
     * INVOICE CUSTOMER BANK
     */

    memset(bank_name, 0, MAX_BUFFER);
    memset(account, 0, MAX_BUFFER);
  
    loenStatus = fetchInvoiceCustomerBank(ppstInvoice, bank_name, MAX_BUFFER, account, MAX_BUFFER);
    if (loenStatus != TRUE)
    {
	sprintf (szTemp, "ERROR in function fetchInvoiceCustomerAccountNo.\n");
	macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
	fovdPopFunctionName ();
	return FALSE;
    }

    fovdPrintLog (LOG_DEBUG, "Customer Bank: %s, Account No: %s\n", bank_name, account);     
    fovdGen("PaymentSlipCustomerBank", 
	    bank_name, 
	    account, 
	    EOL);
  
    /*
     * INVOICE CUSTOMER NAME, ADDRESS
     */
  
    memset(name, 0, MAX_BUFFER);
    loenStatus = fetchInvoiceCustomerName(ppstInvoice, name, MAX_BUFFER);
    if (loenStatus != TRUE)
    {
	sprintf (szTemp, "ERROR in function foenFetchInvoiceCustomerName.\n");
	macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
	fovdPopFunctionName ();
	return FALSE;
    }
  
    fovdGen( "PaymentSlipCustomerName", name, EOL); 
    fovdPrintLog (LOG_DEBUG, "Customer Name : %s\n", name); 
  
    for (i = 0; i < 4; i++) 
    {
	memset(adr[i], 0, MAX_BUFFER);
    }
  
    loenStatus = fetchInvoiceCustomerAddress(ppstInvoice, 
					     adr[0], MAX_BUFFER, 
					     adr[1], MAX_BUFFER, 
					     adr[2], MAX_BUFFER, 
					     adr[3], MAX_BUFFER);
    if (loenStatus != TRUE)
    {
	sprintf (szTemp, "ERROR in function fetchInvoiceCustomerAccountNo.\n");
	macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
	fovdPopFunctionName ();
	return FALSE;
    }

    fovdGen("PaymentSlipCustomerAdddress", adr[0], adr[1], adr[2], adr[3], EOL);    
    fovdPrintLog (LOG_DEBUG, "Customer Address : %s, %s, %s, %s\n", adr[0], adr[1], adr[2], adr[3]);    
  
    /*
     * INVOICE NO
     */
  
    loenStatus = fetchInvoiceNo(ppstInvoice, lachzInvoiceNumber, MAX_BUFFER);
    if (loenStatus != TRUE)
    {
	sprintf (szTemp, "ERROR in function fetchInvoiceNo.\n");
	macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
	fovdPopFunctionName ();
	return FALSE;
    }

    lpchzInvoiceNumber = spchzFormatInvoiceNumber(lachzInvoiceNumber);

    /*
     * INVOICE CUSTOMER ACCOUNT NO
     */
  
    loenStatus = fetchInvoiceCustomerAccountNo(ppstInvoice, lachzCustomerAccount, MAX_BUFFER);
    if (loenStatus != TRUE)
    {
	sprintf (szTemp, "ERROR in function fetchInvoiceCustomerAccountNo.\n");
	macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
	fovdPopFunctionName ();
	return FALSE;
    }

    lpchzCustomerAccount = spchzFormatCustomerAccount(lachzCustomerAccount, MAX_BUFFER);
    lpchzSellerAccountBankId = stBgh.szBRECode;
    lpchzCKSM = spchzCountCKSM(lpchzSellerAccountBankId, lpchzCustomerAccount, lpchzInvoiceNumber);

    fovdGen("PaymentSlipCheckSum", 
	    lpchzCKSM, 
	    lpchzSellerAccountBankId, 
	    lpchzCustomerAccount, 
	    lpchzInvoiceNumber, 
	    EOL);      

    fovdPrintLog (LOG_DEBUG, "Invoice No : %s\n", lpchzInvoiceNumber);

    fovdGen("PaymentSlipInvoice", 
	    lpchzInvoiceNumber, 
	    EOL);       

    /* CR 100bis - PaymentSlipDueDate */

    memset(laszDueDate, 0x00, MAX_BUFFER);
  
    if (TRUE != fetchInvoiceDueDate(ppstInvoice, 
				    laszDueDate, 
				    MAX_BUFFER))
    {
	sprintf (szTemp, "Can't load due date from TIMM INV message\n");
	macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
	fovdPopFunctionName ();
	return FALSE;
    }
    else
    {
	fovdFormatDate(laszDueDate, YYYY_MM_DD);
	fovdGen("PaymentSlipDueDate", laszDueDate, EOL);
    }
  
    fovdGen("PaymentSlipEnd", EOL);

    fovdPopFunctionName ();
    return TRUE;
}

/****************************************************************************************************************
 *
 * FUNCTION: spchzFormatInvoiceNumber
 *
 ****************************************************************************************************************
 */

char *spchzFormatInvoiceNumber(char *number) 
{
  static char buffer[MAX_BUFFER];
  int i;
  
 
  strcpy(buffer, number); 
  for (i = 0; i < strlen(buffer); i++)
    {
      if (buffer[i] == '/')
        {
          buffer[i] = '*';
        }
    }

  return buffer;
}

/****************************************************************************************************************
 *
 * FUNCTION: spchzFormatCustomerAccount
 *
 ****************************************************************************************************************
 */

char *spchzFormatCustomerAccount(char *number, int max) 
{
  static char buffer[MAX_BUFFER], account[MAX_BUFFER];
  int i;

  strcpy(buffer, number); 
  for (i = 0; i < strlen(buffer); i++)
    {
      if (buffer[i] == '.')
        {
          buffer[i] = '*';
        }
    }
  sprintf(account, "%020s", buffer);
  for (i = 0; i < strlen(account); i++)
    {
      if (account[i] == ' ')
        {
          account[i] = '0';
        }
    }

  return account;
}

/****************************************************************************************************************
 *
 * FUNCTION: spchzCountCKSM
 *
 ****************************************************************************************************************
 */

char *spchzCountCKSM(char *a, char *b, char *c)
{
  static char cksm[MAX_BUFFER];
  int i, sum;

  sum = 0;
  for (i = 0; i < strlen(a); i++)
    {
      if (isdigit(a[i]))
        {
          sum = sum + (a[i] - '0');
        }
    }
  for (i = 0; i < strlen(b); i++)
    {
      if (isdigit(b[i]))
        {
          sum = sum + (b[i] - '0');
        }
    }
  for (i = 0; i < strlen(c); i++)
    {
      if (isdigit(c[i]))
        {
          sum = sum + (c[i] - '0');
        }
    }

  sum = sum % 11;
  if (sum == 10)
    {
      sum = 1;
    }
  sprintf(cksm, "%d", sum);
  
  return cksm;
}
