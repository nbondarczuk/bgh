/**************************************************************************/
/*  MODULE : Enclosure Account Generator                                  */
/*                                                                        */
/*  AUTHOR : N.Bondarczuk                                                 */
/*                                                                        */
/*  CREATION DATE : 24.09.97                                              */
/*                                                                        */
/*  DESCRIPTION : Contains function  creating tagged information          */
/*                                                                        */
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
#include "inv_gen.h"
#include "inv_item.h"
#include "inv_types.h"
#include "inv_fetch.h"
#include "num_pl.h"
#include "shdes2des.h"
#include "env_gen.h"
#include "pay_slip_gen.h"

#define CR110298

/*
 * extern functions
 */

extern toenBool foenGenInvoiceItemList(struct s_TimmInter *spstInvoice);

extern toenBool foenInvFetch_PaymentType(struct s_TimmInter *ppstTimm, 
                                         char *pasnzType, 
                                         int poiTypeBufLen);

/*
 * extern variables
 */

extern stBGHGLOB stBgh;	           /* structure with globals for BGH */

/*
 * static functions
 */

toenBool foenGenInvoiceHeader();

toenBool foenGenInvoiceMarketingText();

toenBool foenGenInvoiceCustomerGroup();

/*
 * static variables
 */

static char szTemp[128];

static char saszCurrency[MAX_BUFFER];

/*
 * global variables
 */

struct s_TimmInter *spstInvoice = NULL;

struct s_TimmInter *spstSumSheet = NULL;

float gofInvoiceType;

char gachzInvoiceType[MAX_BUFFER];

toenBool goenCustomerIsExempt = FALSE;

toenBool goenIsDocumentValid = TRUE;

/****************************************************************************************************************
 *
 * FUNCTION: foenGenInvoice
 *
 ****************************************************************************************************************
 */

int foiGenInvoice(struct s_TimmInter *ppstInvoice, 
                  struct s_TimmInter *ppstSumSheet,
                  int poiAddrRuleNo)
{
  toenBool loenStatus;
  int rc = 0;
  double val;
  char *str;
  toenMailingType loenMailing;
  static char total_net_value [MAX_BUFFER]; 
  static char currency        [MAX_BUFFER];
  static char total_brut_value[MAX_BUFFER];
  static char lachzTerm       [MAX_BUFFER];
  
  fovdPushFunctionName ("foenGenInvoice");

  fovdPrintLog (LOG_DEBUG, "INVOICE START\n");

  /*
   * SETUP for global values
   */

  spstInvoice = ppstInvoice;
  spstSumSheet = ppstSumSheet;
  
  /*
   * Only one allowed currency
   */

  strncpy(saszCurrency, (char *)LOCAL_CURRENCY, MAX_BUFFER);
  
  /* 
   * tax exemption for this customer
   */

  memset(lachzTerm, 0, MAX_BUFFER);
  if(NOT(loenStatus = fetchInvoiceTaxExempt(ppstInvoice, lachzTerm, MAX_BUFFER)))
    {
      sprintf (szTemp, "ERROR in function foenGenEnvelope.\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName ();
      return STATUS_ERROR;
    }
      
  if (strlen(lachzTerm) > 0)
    {
      fovdPrintLog (LOG_CUSTOMER, "Tax exempt until %s\n", lachzTerm);
      goenCustomerIsExempt = TRUE;
    }
  else
    {
      goenCustomerIsExempt = FALSE;
    }


  /*
   * ENVELOPE
   */

  loenMailing = MAILING_INV;
  
  if (FALSE == foenGenEnvelope(ppstInvoice,
                               ppstSumSheet,
                               loenMailing,
                               poiAddrRuleNo))
    {
      sprintf (szTemp, "ERROR in function foenGenEnvelope.\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName ();
      return STATUS_ERROR;
    }

  fovdGen( "InvoiceStart", EOL);

  /*
   * TYPE : 0 - amount > 0, 1 - amount = 0, 2 - amount < 0
   */

  rc = fetchInvoiceTotalNetValue(spstInvoice, total_net_value, MAX_BUFFER, currency, MAX_BUFFER);
  if (rc == TRUE)
    {
      sscanf(total_net_value, "%lf", &val);
      gofInvoiceType = val;

      fovdPrintLog (LOG_DEBUG, "INVOICE VALUE : %s -> %lf\n", total_net_value, val);
      if (val > 0.0)
        {
          str = "0";
          fovdGen( "InvoiceType", "0", EOL); 
        }
      else if (val == 0.0)
        {
          str = "1";
          fovdGen( "InvoiceType", "1", EOL); 
        }
      else
        {
          str = "2";
          fovdGen( "InvoiceType", "2", EOL); 
        }

      strcpy(gachzInvoiceType, str);
    }
  else
    {
      sprintf (szTemp, "ERROR in function foenFetchInvoicetotalNetValue.\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);      
      fovdPopFunctionName ();      
      return STATUS_ERROR;
    }
  
  /*
   * HEADER
   */

  if ((loenStatus = foenGenInvoiceHeader()) != TRUE)
    {
      sprintf (szTemp, "ERROR in function foenGenInvoiceHeader.\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName ();
      return STATUS_ERROR;
    }

  /*
   * ITEMS
   */

  if ((loenStatus = foenGenInvoiceItemList(spstInvoice)) == FALSE)
    {
      if (goenIsDocumentValid == TRUE)
        {
          /*
           * ERROR
           */
	  
          sprintf (szTemp, "ERROR in function foenGenInvoiceItemList\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          fovdPopFunctionName ();
          return STATUS_ERROR;
        }  
      else
        {
          /*
           * REJECT
           */

          sprintf (szTemp, "REJECT in function foenGenInvoiceItemList\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          fovdPopFunctionName ();
          return STATUS_REJECT;
        }      
    }

  /*
   * PAYMENT SLIP
   */

  {
    static char laszPaymentType [16];
    
    memset(laszPaymentType, 0x00, 16);
    
    if (FALSE == foenInvFetch_PaymentType(ppstInvoice, 
                                          laszPaymentType, 
                                          16))
      {
        sprintf (szTemp, "Can't get payment type in function foenInvFetch_PaymentType\n");
        macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
        fovdPopFunctionName ();
        return STATUS_ERROR;      
      }
    
    if (EQ(laszPaymentType, "D") && (poiAddrRuleNo == 3 || poiAddrRuleNo == 4))
      {
        fovdPrintLog (LOG_DEBUG, "No payment slip printed, Direct Debit payment used\n");
      }
    else
      {
        fovdGen("PaymentSlipType", gachzInvoiceType, EOL);

        loenMailing = MAILING_INV;

        if (FALSE == foenGenPaymentSlip(spstInvoice, 
                                        ppstSumSheet,
                                        loenMailing,
                                        poiAddrRuleNo))
          {
            sprintf (szTemp, "ERROR in function foenGenInvoicePaymentSlip.\n");
            macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
            fovdPopFunctionName ();
            return STATUS_ERROR;
          }
        
        fovdPrintLog (LOG_DEBUG, "Payment Slip created\n");
      }
  }

  fovdPopFunctionName ();
  
  return STATUS_OK;
}

extern toenBool foenCheckNIP(char *);      

/****************************************************************************************************************
 *
 * FUNCTION: foenGenInvoiceHeader
 *
 ****************************************************************************************************************
 */

toenBool foenGenInvoiceHeader() 
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
  toenBool loenStatus;
  int rc, i, n;

  fovdPushFunctionName ("foenGenInvoiceHeader");

  fovdPrintLog (LOG_DEBUG, "HEADER START\n");

  /*
   * CUSTOMER NAME, ADDRESS
   */
  
  rc = fetchInvoiceCustomerName(spstInvoice, name, MAX_BUFFER);
  if (rc == TRUE)
    {
      fovdGen( "InvoiceCustomerName", name, EOL); 
      /*
        fovdPrintLog (LOG_DEBUG, "CUSTOMER NAME : %s\n", name); 
      */
    }
  else
    {
      sprintf (szTemp, "ERROR in function foenFetchInvoiceCustomerName.\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);      
      fovdPopFunctionName ();      
      return FALSE;
    }

  for (i = 0; i < 4; i++) 
    {
      address[i][0] = EOS;
    }

  rc = fetchInvoiceCustomerAddress(spstInvoice, 
                                   address[0], MAX_BUFFER, 
                                   address[1], MAX_BUFFER, 
                                   address[2], MAX_BUFFER, 
                                   address[3], MAX_BUFFER);
  if (rc == TRUE)
    {
      fovdGen( "InvoiceCustomerAddress", address[0], address[1], address[2], address[3], EOL); 
    }
  else
    {
      sprintf (szTemp, "ERROR in function foenFetchInvoiceCustomerAddress.\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);      
      fovdPopFunctionName ();      
      return FALSE;
    }
  
  /*
   * CUSTOMER ACCOUNT NO
   */

  rc = fetchInvoiceCustomerAccountNo(spstInvoice, customer_account, MAX_BUFFER);
  if (rc == TRUE)
    {
      fovdGen( "InvoiceCustomerAccount", customer_account, EOL);
      fovdPrintLog (LOG_DEBUG, "CUSTOMER ACCOUNT NO : %s\n", customer_account); 
    }
  else
    {
      sprintf (szTemp, "ERROR in function foenFetchInvoiceCustomerAccountNo.\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);      
      fovdPopFunctionName ();      
      return FALSE;
    }


  /*
   * CUSTOMER NIP
   */
  
  memset(customer_nip, 0, MAX_BUFFER);
  rc = fetchInvoiceCustomerNIP(spstInvoice, customer_nip, MAX_BUFFER);
  if (rc == TRUE)
    {
      /*
       * NIP check
       */
      
      fovdPrintLog (LOG_DEBUG, "CUSTOMER NIP : %s\n", customer_nip); 
      loenStatus = foenCheckNIP(customer_nip);      
      if (loenStatus == TRUE)
        {
          /*
           * NIP not zero
           */

          fovdGen("InvoiceCustomerNIP", customer_nip, EOL);
          fovdPrintLog (LOG_DEBUG, "CUSTOMER NIP VALIDATED : %s\n", customer_nip); 
        }
      else
        {
          /*
           * NIP zero
           */

          fovdPrintLog (LOG_DEBUG, "CUSTOMER NIP SUPPRESSED: %s\n", customer_nip); 
        }
    }
  else
    {
      sprintf (szTemp, "ERROR in function foenFetchInvoiceCustomerNIP.\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);      
      fovdPopFunctionName ();      
      return FALSE;
    }

  /*
   * SELLER NAME
   */

  rc = fetchInvoiceSellerName(spstInvoice, name, MAX_BUFFER);
  if (rc == TRUE)
    {
      fovdGen( "InvoiceSellerName", name, EOL);  
      fovdPrintLog (LOG_DEBUG, "SELLER NAME : %s\n", name); 
    }
  else
    {
      sprintf (szTemp, "ERROR in function foenFetchInvoiceSellerName.\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName ();
      return FALSE;
    }

  for (i = 0; i < 4; i++) 
    {
      address[i][0] = '\0';
    }

  rc = fetchInvoiceSellerAddress(spstInvoice, 
                                 address[0], MAX_BUFFER, 
                                 address[1], MAX_BUFFER, 
                                 address[2], MAX_BUFFER, 
                                 address[3], MAX_BUFFER);
  if (rc == TRUE)
    {
      fovdGen( "InvoiceSellerAddress", name, "", address[1], address[2], EOL); 
      fovdPrintLog (LOG_DEBUG, "SELLER ADDRESS : %s %s %s %s\n",  address[0], address[1], address[2], address[3]); 
    }
  else
    {
      sprintf (szTemp, "ERROR in function foenFetchInvoiceSellerAddress.\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName ();      
      return FALSE;
    }

  /*
   * SELLER NIP
   */

  rc = fetchInvoiceSellerNIP(spstInvoice, seller_nip, MAX_BUFFER);
  if (rc == TRUE)
    {
      fovdGen( "InvoiceSellerNIP", seller_nip, EOL); 
      fovdPrintLog (LOG_DEBUG, "SELLER NIP : %s\n", seller_nip); 
    }
  else
    {
      sprintf (szTemp, "ERROR in function foenFetchInvoiceSellerNIP.\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);      
      fovdPopFunctionName ();
      return FALSE;
    }


  /*
   * INVOICE DATE
   */

  rc = fetchInvoiceDate(spstInvoice, date, MAX_BUFFER);
  if (rc == TRUE) 
    {
      fovdFormatDate(date, YY_MM_DD);
      fovdGen( "InvoiceHeader", date, EOL);
      fovdPrintLog (LOG_DEBUG, "DATE : %s\n", date); 
    }
  else
    {
      sprintf (szTemp, "ERROR in function foenFetchInvoiceDate.\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName ();
      return FALSE;
    }


  /*
   * INVOICE NUMBER, PERIOD BEGIN, PERIOD END
   */

  rc = fetchInvoiceNumber(spstInvoice, invoice_number, MAX_BUFFER);
  if (rc == FALSE)
    {
      sprintf (szTemp, "ERROR in function foenFetchInvoiceNumber.\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);      
      fovdPopFunctionName ();      
      return FALSE;
    }

  rc = fetchInvoicePeriodBegin(spstInvoice, period_begin, MAX_BUFFER);
  if (rc == FALSE)
    {
      sprintf (szTemp, "ERROR in function foenFetchInvoicePeriodBegin.\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);      
      fovdPopFunctionName ();      
      return FALSE;
    }

  rc = fetchInvoicePeriodEnd(spstInvoice, period_end, MAX_BUFFER);
  if (rc == FALSE)
    {
      sprintf (szTemp, "ERROR in function foenFetchInvoicePeriodEnd.\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);      
      fovdPopFunctionName ();      
      return FALSE;
    }

  fovdFormatDate(period_begin, YY_MM_DD);
  fovdFormatDate(period_end, YY_MM_DD);
  fovdGen( "InvoiceTitle", invoice_number, period_begin, period_end, EOL); 
  fovdPrintLog (LOG_DEBUG, "NUMBER : %s, BEGIN : %s, END : %s\n", invoice_number, period_begin, period_end);   
  
  fovdPrintLog (LOG_DEBUG, "HEADER END\n");
  
  fovdPopFunctionName ();  
  return TRUE;
}



