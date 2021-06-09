/**************************************************************************/
/*  MODULE : WELCOME data generator                                       */
/*                                                                        */
/*  AUTHOR : N.Bondarczuk                                                 */
/*                                                                        */
/*  CREATION DATE : 25.11.97                                              */
/*                                                                        */
/*  DESCRIPTION : Generates information about welcome letter from TIMM    */
/*                WELCOME   message.                                      */ 
/**************************************************************************/

#include <stdio.h>
#include <string.h>
#include "bgh.h"
#include "protos.h"
#include "parser.h"

#include "types.h"
#include "gen.h"
#include "strutl.h"
#include "inv_item.h"
#include "inv_fetch.h"
#include "wll_fetch.h"

#if 0	/* just for version.sh */
static char *SCCS_VERSION = "4.1.2";
#endif

static struct s_TimmInter *dpstWelcomeLetter;
static struct s_TimmInter *dpstInvoice;
static char szTemp[128];
static tostAddress dostAddress;
toenBool doenAddressLoaded;

toenBool foenGenWelcomeLetterEnvelope();
toenBool foenGenWelcomeLetterAddress2();
toenBool foenGenWelcomeLetterHeader();
toenBool foenGenWelcomeLetterText();

int foenGenWelcomeLetter(struct s_TimmInter *ppstTimm, struct s_TimmInter *ppstInvoice, int poiPrintEnvelope)
{
  toenBool loenState;

  fovdPushFunctionName("foenGenWelcomeLetter");

  ASSERT(ppstTimm != NULL);

  dpstWelcomeLetter = ppstTimm;
  dpstInvoice = ppstInvoice;
  doenAddressLoaded = FALSE;

  fovdPrintLog (LOG_TIMM, "WELCOME LETTER\n");  

  if (poiPrintEnvelope == TRUE)
    {
      if (NOT(loenState = foenGenWelcomeLetterEnvelope()))
        {
          fovdPopFunctionName();          
          return FALSE;
        }
    }

  fovdGen( "WelcomeLetterStart", EOL);

  if (NOT(loenState = foenGenWelcomeLetterHeader()))
    {
      fovdPopFunctionName();
      return FALSE;
    }

  fovdGen( "WelcomeLetterTextListStart", EOL);
  if (NOT(loenState = foenGenWelcomeLetterText()))
    {
      fovdPopFunctionName();
      return FALSE;
    }
  fovdGen( "WelcomeLetterTextListEnd", EOL);  

  if (NOT(loenState = foenGenWelcomeLetterAddress2()))
    {
      fovdPopFunctionName();          
      return FALSE;
    }
  
  fovdGen( "WelcomeLetterEnd", EOL);

  fovdPopFunctionName();

  return TRUE;
}



/**************************************************************************************************
 *                                                                                          
 * FUNCTION : foenGenwelcomeLetterEnvelope
 *                                                                                          
 * LOCAL FUNCTIONS USED:                                                                    
 *                                                                                          
 * EXTERNAL FUNCTIONS USED:                                                                 
 * MODULE: INV_FETCH                                                                        
 *  rc = fetchInvoiceTemporaryAddress(struct s_TimmInter *dpstItemizedBill, char *, int, char *, int, char *, int, char *, int);
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenGenWelcomeLetterEnvelope()
{
  static char address[4][MAX_BUFFER];
  static char name[MAX_BUFFER];
  int i, rc;
  toenBool loenState;

  fovdPushFunctionName ("foenGenWelcomeLetterEnvelope");
  
  fovdGen( "EnvelopeStart", EOL);

  /*
   * NAME, ADDRESS
   */
  
  /*
   * Use customer real address and name from WL doc.
   */
  
  if (dpstInvoice == NULL)
    {
      /*
       * No invoice available
       */
      
      rc = GetTempAddressFromDatabase(atoi(dpstWelcomeLetter->unb->v_0010), &dostAddress);
      if (rc == 0)
        {
          doenAddressLoaded = TRUE;
          fovdPrintLog (LOG_DEBUG, "Using temporary name and address from DB for envelope\n");  
          fovdGen( "EnvelopeName", dostAddress.sachzLine1, EOL);
          fovdGen( "EnvelopeAddress", 
                   dostAddress.sachzLine2, 
                   dostAddress.sachzLine3,
                   dostAddress.sachzLine4,
                   dostAddress.sachzLine5, 
                   EOL); 
        }
      else
        {
          /*
           * Can't load address from DB
           */

          fovdPrintLog (LOG_DEBUG, "Using customer official name and address from DL for envelope\n");     
          
          if (NOT(loenState = fetchInvoiceCustomerName(dpstWelcomeLetter, name, MAX_BUFFER)))
            {
              sprintf (szTemp, "ERROR in function fetchInvoiceCustomerName\n");
              macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
              fovdPopFunctionName ();
              return FALSE;
            }
          
          for (i = 0; i < 4; i++) 
            {
              address[i][0] = EOS;
            }
          if (NOT(loenState = fetchInvoiceCustomerAddress(dpstWelcomeLetter, 
                                                          address[0], MAX_BUFFER, address[1], MAX_BUFFER, 
                                                          address[2], MAX_BUFFER, address[3], MAX_BUFFER)))
            {
              sprintf (szTemp, "ERROR in function fetchInvoiceCustomerAddress\n");
              macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
              fovdPopFunctionName ();
              return FALSE;
            }          
          
          fovdGen( "EnvelopeName", name, EOL);
          fovdGen("EnvelopeAddress", address[0], address[1], address[2], address[3], EOL);           
        }
    }
  else
    {
      /*
       * We have invoice
       */
      
      if (NOT(loenState = fetchInvoiceTemporaryName(dpstInvoice, name, MAX_BUFFER)))
        {
          sprintf (szTemp, "ERROR in function fetchInvoiceTemporaryName\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          fovdPopFunctionName ();
          return FALSE;
        }
      
      for (i = 0; i < 4; i++) 
        {
          address[i][0] = EOS;
        }
      if (NOT(loenState = fetchInvoiceTemporaryAddress(dpstInvoice, 
                                                       address[0], MAX_BUFFER, address[1], MAX_BUFFER, 
                                                       address[2], MAX_BUFFER, address[3], MAX_BUFFER)))
        {
          sprintf (szTemp, "ERROR in function fetchInvoiceTemporaryAddress\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          fovdPopFunctionName ();
          return FALSE;
        }
      
      /*
       * Is name and address loaded from temporary address ?
       */

      if (name[0] == EOS && 
          address[0][0] == EOS && 
          address[1][0] == EOS && 
          address[2][0] == EOS &&
          address[3][0] == EOS)
        {          
          fovdPrintLog (LOG_DEBUG, "Using customer official name and address for envelope\n");  
          
          if (NOT(loenState = fetchInvoiceCustomerName(dpstWelcomeLetter, name, MAX_BUFFER)))
            {
              sprintf (szTemp, "ERROR in function fetchInvoiceCustomerName\n");
              macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);          
              fovdPopFunctionName ();          
              return FALSE;
            }
          
          for (i = 0; i < 4; i++) 
            {
              address[i][0] = EOS;
            }
          if (NOT(loenState = fetchInvoiceCustomerAddress(dpstWelcomeLetter, 
                                                          address[0], MAX_BUFFER, 
                                                          address[1], MAX_BUFFER, 
                                                          address[2], MAX_BUFFER, 
                                                          address[3], MAX_BUFFER)))
            {
              sprintf (szTemp, "ERROR in function fetchInvoiceCustomerAddress\n");
              macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);          
              fovdPopFunctionName ();          
              return FALSE;
            }
        }
      else
        {
          fovdPrintLog (LOG_DEBUG, "Using customer temporary name and address for envelope\n");  
        }
      
      fovdGen( "EnvelopeName", name, EOL);
      fovdGen( "EnvelopeAddress", address[0], address[1], address[2], address[3], EOL); 
    }

  fovdGen( "EnvelopeEnd", EOL);
  
  fovdPopFunctionName ();
  return TRUE;
}


/**************************************************************************************************
 *                                                                                          
 * FUNCTION : foenGenHeader             
 *                                                                                          
 * LOCAL FUNCTIONS USED:                                                                    
 *                                                                                          
 * EXTERNAL FUNCTIONS USED:                                                                 
 * toenBool foenFetchInvoiceDate(struct s_TimmInter *ppstSumSheet, char *poszDate, int poiDateBufferLength);
 * toenBool foenFetchInvoiceCustomerAccountNo(struct s_TimmInter *ppstSumSheet, char *poszAccountNo, int poiAcountNoBufferLength);
 * toenBool foenFetchInvoiceNo(struct s_TimmInter *ppstSumSheet, char *poszInvoiceNo, int poiInvoiceNoBufferLength);
 * toenBool foenFetchPeriodBegin(struct s_TimmInter *ppstSumSheet, char *poszPeriodBegin, int poiPeriodBeginBufferLength);
 * toenBool foenFetchPeriodEnd(struct s_TimmInter *ppstSumSheet, char *poszPeriodEnd, int poiPeriodEndBufferLength);
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenGenWelcomeLetterHeader() {
  toenBool loenState;
  static char loszDate[MAX_BUFFER];
  static char loszAccountNo[MAX_BUFFER];
  static char loszName[MAX_BUFFER];
  static char address[4][MAX_BUFFER];
  static char seller_nip[MAX_BUFFER];
  static char name[MAX_BUFFER];
  int i, rc;

  
  fovdPushFunctionName("foenGenWelcomeLetterHeader");

  /*
   * DATE
   */

  fovdPrintLog (LOG_TIMM, "HEADER\n");  
  if (NOT(loenState = fetchInvoiceLetterDate(dpstWelcomeLetter, loszDate, MAX_BUFFER)))
    {      
      sprintf (szTemp, "ERROR in function foenFetchInvoiceLetterDate\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName ();
      return FALSE;
    }
  fovdFormatDate(loszDate, YY_MM_DD);
  fovdGen( "WelcomeLetterDate", loszDate, EOL);
  fovdPrintLog (LOG_TIMM, "DATE : %s\n", loszDate);  

  /*
   * ACCOUNT
   */

  if (NOT(loenState = foenFetchWelcomeLetterCustomerAccountNo(dpstWelcomeLetter, loszAccountNo, MAX_BUFFER)))
    {
      sprintf (szTemp, "ERROR in function foenFetchInvoiceCustomerAccountNo.\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName ();
      return FALSE;
    }
  fovdGen( "WelcomeLetterAccount", loszAccountNo, EOL);

  /*
   * NAME< ADDRESS
   */

  if (NOT(loenState = fetchInvoiceCustomerName(dpstWelcomeLetter, loszName, MAX_BUFFER)))
    {
      sprintf (szTemp, "ERROR in function foenFetchInvoiceCustomerName\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);

      fovdPopFunctionName ();

      return FALSE;
    }
  fovdGen("WelcomeLetterName", loszName, EOL);

  for (i = 0; i < 4; i++) 
    {
      address[i][0] = EOS;
    }
  if (NOT(loenState = fetchInvoiceCustomerAddress(dpstWelcomeLetter, address[0], MAX_BUFFER, address[1], MAX_BUFFER, address[2], MAX_BUFFER, address[3], MAX_BUFFER)))
    {
      sprintf (szTemp, "ERROR in function foenFetchInvoiceCustomerName\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName ();
      return FALSE;
    }
  fovdGen("WelcomeLetterAddress", address[0], address[1], address[2], address[3], EOL); 

  /*
   * SELLER DATA
   */

  rc = fetchInvoiceSellerName(dpstWelcomeLetter, name, MAX_BUFFER);
  if (rc == TRUE)
    {
      fovdGen( "WelcomeLetterSellerName", name, EOL);  
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
  rc = fetchInvoiceSellerAddress(dpstWelcomeLetter, 
                                 address[0], MAX_BUFFER, address[1], MAX_BUFFER, 
                                 address[2], MAX_BUFFER, address[3], MAX_BUFFER);
  if (rc == TRUE)
    {
      fovdGen( "WelcomeLetterSellerAddress", name, "", address[1], address[2], EOL); 
      fovdPrintLog (LOG_DEBUG, "SELLER ADDRESS : %s:%s:%s:%s\n", address[0], address[1], address[2], address[3]); 
    }
  else
    {
      sprintf (szTemp, "ERROR in function foenFetchInvoiceSellerAddress.\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName ();      
      return FALSE;
    }

  /*
   * SELLER NIP from WLL
   */
  
  rc = foenFetchWelcomeLetterSellerNIP(dpstWelcomeLetter, seller_nip, MAX_BUFFER);
  if (rc == TRUE)
    {
      fovdGen( "WelcomeLetterSellerNIP", seller_nip, EOL); 
      fovdPrintLog (LOG_DEBUG, "SELLER NIP : %s\n", seller_nip); 
    }
  else
    {
      sprintf (szTemp, "ERROR in function foenFetchInvoiceSellerNIP.\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);      
      fovdPopFunctionName ();      
      return FALSE;
    }
  
  
  fovdPopFunctionName();
  return TRUE;
}


toenBool foenGenWelcomeLetterText()
{
  toenBool loenStatus;
  static char lachzText[5][MAX_BUFFER];
  int loiTextsNo, loiText, loiLine;
  int lachzMarketingText[2000];

  lachzMarketingText[0] = '\0';
  loiTextsNo = foiCountWelcomeLetterTexts(dpstWelcomeLetter);
  for (loiText = 0; loiText < loiTextsNo; loiText++)
    {
      loenStatus = foenFetchWelcomeLetterText(dpstWelcomeLetter, loiText, lachzText);
      if (loenStatus == FALSE)
        {
          return TRUE;
        }
      
      for (loiLine = 0; loiLine < 5; loiLine++)
        {
          if (strlen(lachzText[loiLine]) > 0)
            {
              fovdGen("WelcomeLetterText", lachzText[loiLine], EOL);
            }
        }
    }

  return TRUE;
}



toenBool foenGenWelcomeLetterAddress2()
{
  static char address[4][MAX_BUFFER];
  static char name[MAX_BUFFER];
  static char loszAccountNo[MAX_BUFFER];
  static char lachzSocSecNo[21];
  int i, rc;
  toenBool loenState;

  fovdPushFunctionName ("foenGenWelcomeLetterAddress2");

  /*
   * NIP number
   */
  rc = GetSocSecNoFromDatabase(atoi(dpstWelcomeLetter->unb->v_0010), lachzSocSecNo);
  if (rc == 0)
    {
      fovdGen( "WelcomeLetterCustomerNIP", lachzSocSecNo, EOL);      
    }
  
  /*
   * NAME, ADDRESS
   */
  
  if (NOT(loenState = fetchInvoiceCustomerName(dpstWelcomeLetter, name, MAX_BUFFER)))
    {
      sprintf (szTemp, "ERROR in function fetchInvoiceTemporaryName\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName ();
      return FALSE;
    }
  
  for (i = 0; i < 4; i++) 
    {
      address[i][0] = EOS;
    }
  if (NOT(loenState = fetchInvoiceCustomerAddress(dpstWelcomeLetter, address[0], MAX_BUFFER, address[1], MAX_BUFFER, address[2], MAX_BUFFER, address[3], MAX_BUFFER)))
    {
      sprintf (szTemp, "ERROR in function fetchInvoiceTemporaryAddress\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName ();
      return FALSE;
    }

  fovdGen("WelcomeLetterAddress2", 
          address[0], "", address[1], 
          fpchzFormatCity(address[2]), /* City */
          fpchzFormatZip(address[2]), /* ZIP */ 
          EOL);       
  
  /*
   * ACCOUNT
   */

  if (NOT(loenState = foenFetchWelcomeLetterCustomerAccountNo(dpstWelcomeLetter, loszAccountNo, MAX_BUFFER)))
    {
      sprintf (szTemp, "ERROR in function foenFetchInvoiceCustomerAccountNo.\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName ();
      return FALSE;
    }
  fovdGen( "WelcomeLetterAccount2", loszAccountNo, EOL);

  
  /*
   * NAME, ADDRESS
   */

  if (dpstInvoice != NULL) 
    {
      /*
       * We have invoice
       */

      if (NOT(loenState = fetchInvoiceTemporaryName(dpstInvoice, name, MAX_BUFFER)))
        {
          sprintf (szTemp, "ERROR in function fetchInvoiceTemporaryName\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          fovdPopFunctionName ();
          return FALSE;
        }
      
      for (i = 0; i < 4; i++) 
        {
          address[i][0] = EOS;
        }
      if (NOT(loenState = fetchInvoiceTemporaryAddress(dpstInvoice, 
                                                       address[0], MAX_BUFFER, address[1], MAX_BUFFER, 
                                                       address[2], MAX_BUFFER, address[3], MAX_BUFFER)))
        {
          sprintf (szTemp, "ERROR in function fetchInvoiceTemporaryAddress\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          fovdPopFunctionName ();
          return FALSE;
        }

      /*
       * Is name and address loaded from temporary address ?
       */
      
      if (name[0] == EOS && 
          address[0][0] == EOS && 
          address[1][0] == EOS && 
          address[2][0] == EOS &&
          address[3][0] == EOS)
        {
          /*
           * Use official address
           */
        
          fovdPrintLog (LOG_DEBUG, "Using customer official name and address for envelope\n");     
          
          if (NOT(loenState = fetchInvoiceCustomerName(dpstInvoice, name, MAX_BUFFER)))
            {
              sprintf (szTemp, "ERROR in function fetchInvoiceCustomerName\n");
              macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
              fovdPopFunctionName ();
              return FALSE;
            }
          
          for (i = 0; i < 4; i++) 
            {
              address[i][0] = EOS;
            }
          if (NOT(loenState = fetchInvoiceCustomerAddress(dpstInvoice, 
                                                          address[0], MAX_BUFFER, address[1], MAX_BUFFER, 
                                                          address[2], MAX_BUFFER, address[3], MAX_BUFFER)))
            {
              sprintf (szTemp, "ERROR in function fetchInvoiceCustomerAddress\n");
              macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
              fovdPopFunctionName ();
              return FALSE;
            }          
        }
      else
        {
          fovdPrintLog (LOG_DEBUG, "Using customer temporary name and address for envelope\n");    
        }
      
      /*
       * Find ZIP code being a first string in address[3]
       */
      
      fovdGen("EnvelopeAddress2", 
              address[0], "", address[1], 
              fpchzFormatCity(address[2]), /* City */
              fpchzFormatZip(address[2]), /* ZIP */ 
              EOL);       
    }
  else /* NULL -> no invoice given */
    {
      if (doenAddressLoaded == FALSE)
        {
          /*
           * No cached value from DB so load it
           */
          rc = GetTempAddressFromDatabase(atoi(dpstWelcomeLetter->unb->v_0010), &dostAddress);          
          if (rc == 0)
            {
              /* NOP */
            }
          else
            {
              fovdPrintLog (LOG_DEBUG, "Using customer official name and address for envelope\n");     
              
              if (NOT(loenState = fetchInvoiceCustomerName(dpstWelcomeLetter, name, MAX_BUFFER)))
                {
                  sprintf (szTemp, "ERROR in function fetchInvoiceCustomerName\n");
                  macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
                  fovdPopFunctionName ();
                  return FALSE;
                }
              
              for (i = 0; i < 4; i++) 
                {
                  address[i][0] = EOS;
                }
              if (NOT(loenState = fetchInvoiceCustomerAddress(dpstWelcomeLetter, 
                                                              address[0], MAX_BUFFER, address[1], MAX_BUFFER, 
                                                              address[2], MAX_BUFFER, address[3], MAX_BUFFER)))
                {
                  sprintf (szTemp, "ERROR in function fetchInvoiceCustomerAddress\n");
                  macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
                  fovdPopFunctionName ();
                  return FALSE;
                }          
              
              /*
               * Find ZIP code being a first string in address[3]
               */
              
              fovdGen("EnvelopeAddress2", 
                      address[0], "", address[1], 
                      fpchzFormatCity(address[2]), /* City */
                      fpchzFormatZip(address[2]), /* ZIP */ 
                      EOL); 

              fovdPopFunctionName ();
              return TRUE;              
            }
        }

      fovdGen("EnvelopeAddress2", 
              dostAddress.sachzLine2,
              "",
              dostAddress.sachzLine3,
              fpchzFormatCity(dostAddress.sachzLine4), /* City */
              fpchzFormatZip(dostAddress.sachzLine4), /* ZIP */ 
              EOL);       
      
      /*
       * Must be loaded
       */      
    }

  fovdPopFunctionName ();
  return TRUE;
}






