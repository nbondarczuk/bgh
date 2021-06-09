/**************************************************************************/
/*  MODULE : Envelope section generator                                   */
/*                                                                        */
/*  AUTHOR : N.Bondarczuk                                                 */
/*                                                                        */
/*  CREATION DATE : 29.02.00                                              */
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

/* 
 * Static globals               
 */

static char szTemp[PATH_MAX];		/* temporay storage for error messages */

/*
 * Extern variables
 */

extern struct s_TimmInter *spstInvoice;

/****************************************************************************************************************
 *
 * FUNCTION: foenGenInvoiceMarketingText
 *
 ****************************************************************************************************************
 */

toenBool foenGenInvoiceMarketingText(struct s_TimmInter *ppstTimm)
{
  toenBool loenStatus;
  static char lachzText[6][MAX_BUFFER];
  int loiTextsNo, loiText, loiLine;
  int lachzMarketingText[2000];

  lachzMarketingText[0] = '\0';
  loiTextsNo = countInvoiceMarketingTexts(ppstTimm);
  
  for (loiText = 0; loiText < loiTextsNo; loiText++)
    {
      if (FALSE == fetchInvoiceMarketingText(ppstTimm, 
                                             loiText, 
                                             lachzText))
        {
          break;
        }
      else
        {
          for (loiLine = 0; loiLine < 6; loiLine++)
            {
              if (strlen(lachzText[loiLine]) > 0)
                {
                  fovdGen( "InvoiceMarketingText", 
                           lachzText[loiLine], 
                           EOL);                  
                }
            }
        }
    }

  return TRUE;
}

/*****************************************************************************************************************
 *
 * FUNCTION: foenGenInvoiceCustomerGroup
 *
 * DESCRIPTION: CR42 - printing info about customer's group
 *
 *****************************************************************************************************************
 */

toenBool foenGenInvoiceCustomerGroup(struct s_TimmInter *ppstTimm)
{
  toenBool loenStatus;
  char lasnzGroup[MAX_BUFFER];
  char *lpsnzName;
  
  memset(lasnzGroup, 0, MAX_BUFFER);
  if (FALSE == foenInvFetch_CustomerGroup(ppstTimm, 
                                          lasnzGroup, 
                                          MAX_BUFFER))
    {
      return FALSE;
    }

  if (NULL == (lpsnzName = fpsnzMapPriceGroupCode2Name(lasnzGroup)))
    {
      lpsnzName = "";
    }
  
  fovdGen("CustomerGroup", lasnzGroup, lpsnzName, EOL);

  return TRUE;
}

/*****************************************************************************************************************
 *
 * FUNCTION: foenGenInvoicePaymentType
 *
 * DESCRIPTION: CR42 - printing info about payment type
 *
 *****************************************************************************************************************
 */

#define CODE_LEN 8
#define CREDIT_CARD_PAYMENT 'R'

toenBool foenGenInvoicePaymentType(struct s_TimmInter *ppstTimm)
{
  toenBool loenStatus;
  char lasnzCode[CODE_LEN];
  tostPaymentTypeBuf lostPaymentType;

  memset(&lostPaymentType, 0, sizeof(tostPaymentType));
  if (FALSE == foenInvFetch_PaymentType(ppstTimm, 
                                        lasnzCode, 
                                        CODE_LEN))
    {
      return FALSE;
    }
  
  if (FALSE == foenMapPaymentCode2Type(lasnzCode[0], 
                                       &lostPaymentType))
    {
      return FALSE;
    }
  
  fovdGen("PaymentType", 
          lostPaymentType.sasnzPaymentId, 
          lostPaymentType.sasnzPaymentCode, 
          lostPaymentType.sasnzPaymentName, 
          EOL);
  
  return TRUE;
}

/****************************************************************************************************************
 *
 * FUNCTION: foenGenEnvelope
 *
 ****************************************************************************************************************
 */

toenBool foenGenEnvelope(struct s_TimmInter *ppstInvTimm,
                         struct s_TimmInter *ppstSumTimm,
                         enum toenMailingType poenMailing,
                         int poiAddrRuleNo)
  
{
  static char address[4][MAX_BUFFER];
  int i, rc = 0;
  int loiAddress = 0;
  char lasnzCode[CODE_LEN];
  char lochPaymentType;

  fovdPushFunctionName ("foenGenEnvelope");

  fovdPrintLog(LOG_DEBUG, "Mailing: %d, Rule: %d\n", poenMailing, poiAddrRuleNo);
  
  /*
   * Init
   */

  for (i = 0; i < 4; i++) 
    {
      memset(address[i], 0, MAX_BUFFER);
    }

  if (FALSE == foenInvFetch_PaymentType(ppstInvTimm, 
                                        lasnzCode, 
                                        CODE_LEN))
    {
      return FALSE;
    }
  else
    {
      lochPaymentType = lasnzCode[0];
    }

  if (poenMailing  == MAILING_INV)
    {
      if (poiAddrRuleNo == 1)
        {
          /* Bill Address */
          loiAddress = 1;
       }
      else if (poiAddrRuleNo == 2)
        {
          /* Bill Address */
          loiAddress = 1;
        }
      else if (poiAddrRuleNo == 3)
        {
          if (lochPaymentType == 'D')
            {
              /* Bill Address */
              loiAddress = 1;
            }
          else
            {
              /* Temporary Address */
              loiAddress = 2;
            }
        }
      else if (poiAddrRuleNo == 4)
        {
          if (lochPaymentType == 'D')
            {
              /* Contract Address */
              loiAddress = 3;
            }
          else
            {
              /* Temporary Address */
              loiAddress = 2;
            }
        }
      else if (poiAddrRuleNo == 5)
        {      
          /* Temporary Address */
          loiAddress = 2;
        }
      else
        {
          ASSERT(FALSE);
        }      
    }
  else if (poenMailing  == MAILING_ENC)
    {
      if (poiAddrRuleNo == 1)
        {
          ASSERT(FALSE);
        }
      else if (poiAddrRuleNo == 2)
        {
          /* Contract Address */
          loiAddress = 3;
        }
      else if (poiAddrRuleNo == 3)
        {
          /* Contract Address */
          loiAddress = 3;
        }
      else if (poiAddrRuleNo == 4)
        {
          /* Contract Address */
          loiAddress = 3;
        }
      else if (poiAddrRuleNo == 5)
        {      
          ASSERT(FALSE);
        }
      else
        {
          ASSERT(FALSE);
        }      
    }
  else if (poenMailing  == MAILING_PAY)
    {
      if (poiAddrRuleNo == 1)
        {
          ASSERT(FALSE);
        }
      else if (poiAddrRuleNo == 2)
        {
          ASSERT(FALSE);
        }
      else if (poiAddrRuleNo == 3)
        {
          /* Temporary Address */
          loiAddress = 2;
        }
      else if (poiAddrRuleNo == 4)
        {
          /* Temporary Address */
          loiAddress = 2;
        }
      else if (poiAddrRuleNo == 5)
        {      
          ASSERT(FALSE);
        }
      else
        {
          ASSERT(FALSE);
        }
    }
  else
    {
      ASSERT(FALSE);
    }
  
  if (loiAddress == 1) /* Bill Address */
    {
      if (FALSE == fetchInvoiceCustomerAddress(ppstInvTimm, 
                                               address[0], MAX_BUFFER, 
                                               address[1], MAX_BUFFER, 
                                               address[2], MAX_BUFFER, 
                                               address[3], MAX_BUFFER))
        {
          sprintf (szTemp, "ERROR in function fetchInvoiceCustomerAddress.\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);          
          fovdPopFunctionName ();          
          return FALSE;
        }      
    }
  else if (loiAddress == 2) /* Temporary Address */
    {
      if (FALSE == fetchInvoiceTemporaryAddress(ppstInvTimm, 
                                                address[0], MAX_BUFFER, 
                                                address[1], MAX_BUFFER, 
                                                address[2], MAX_BUFFER, 
                                                address[3], MAX_BUFFER))
        {
          sprintf (szTemp, "ERROR in function fetchInvoiceTemporaryAddress.\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);      
          fovdPopFunctionName ();      
          return FALSE;
        }      
    }
  else if (loiAddress == 3) /* Contract Address */
    {
      if (FALSE == fetchInvoiceCustomerAddress(ppstSumTimm, 
                                               address[0], MAX_BUFFER, 
                                               address[1], MAX_BUFFER, 
                                               address[2], MAX_BUFFER, 
                                               address[3], MAX_BUFFER))
        {
          sprintf (szTemp, "ERROR in function fetchInvoiceCustomerAddress.\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);          
          fovdPopFunctionName ();          
          return FALSE;
        }      
    }
  else
    {
      ASSERT(FALSE);
    }  

  fovdGen( "EnvelopeStart", EOL);

  fovdGen("EnvelopeAddress", 
          address[0], 
          address[1], 
          address[2], 
          address[3], 
          EOL); 

  /*
   * Customer Price Group from CR 42 - table PRICEGROUP_ALL and attr INV.G2.RFF:PG
   */ 
  
  if (FALSE == foenGenInvoiceCustomerGroup(ppstInvTimm))
    {
      sprintf (szTemp, "ERROR in function foenGenInvoiceCustomerGroup\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName ();
      return FALSE;
    }      
  
  /*
   * Customer Payment Type from CR 44 - table PAYMENTTYPE_ALL and attr INV.G2.RFF.PM
   */
  
  if (FALSE == foenGenInvoicePaymentType(ppstInvTimm))
    {
      sprintf (szTemp, "ERROR in function foenGenInvoicePaymentType\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName ();
      return FALSE;          
    }
  
  if (poenMailing  != MAILING_PAY)  
    {
      /*
       * Marketing Text
       */
      
      fovdGen("MarketingTextListStart", EOL);
      
      if (FALSE == foenGenInvoiceMarketingText(ppstInvTimm))
        {
          sprintf (szTemp, "ERROR in function foenGenMarketingText.\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          fovdPopFunctionName ();
          return FALSE;
        }
      
      fovdGen("MarketingTextListEnd", EOL);
    }

  fovdGen("EnvelopeEnd", EOL);  
  
  fovdPopFunctionName ();
  
  return TRUE;
}



