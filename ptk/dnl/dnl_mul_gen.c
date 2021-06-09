/**************************************************************************/
/*  MODULE : DUNNING data generator                                       */
/*                                                                        */
/*  AUTHOR : N.Bondarczuk                                                 */
/*                                                                        */
/*  CREATION DATE : 16.12.97                                              */
/*                                                                        */
/*  DESCRIPTION : Generates output with information about dunning         */
/*                from several dunnning messages given in a table         */ 
/*                                                                        */
/**************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bgh.h"
#include "protos.h"
#include "parser.h"

#include "types.h"
#include "gen.h"
#include "inv_item.h"
#include "strutl.h"
#include "queue.h"
#include "inv_fetch.h"
#include "dnl_fetch.h"

#if 0	/* just for version.sh */
static char *SCCS_VERSION = "4.1";
#endif

static tostAddress dostAddress;
static struct s_TimmInter *dpstDunningLetter;
static char szTemp[128];


toenBool foenGenDunningLetterEnvelope();
toenBool foenGenDunningLetterHeader();
toenBool foenGenDunningLetterText();

/**************************************************************************************************
 *                                                                                          
 * FUNCTION : foenGenDunningMultiLetter             
 *                                                                                          
 * LOCAL FUNCTIONS USED:                                                                    
 *                                                                                          
 * EXTERNAL FUNCTIONS USED:                                                                 
 *                                                                                          
 **************************************************************************************************
 */

int foenGenDunningMultiLetter(struct s_TimmInter **papstTimm, int poiTimmDocs, int poiLevel, int poiEnvelope)
{
  int loiTimmDoc, loiQueue, loiQueueLen, loiDunningLevel, iRc;
  toenBool loenStatus;
  int loiEnvelope;
  static char lachzDunningLevel[MAX_BUFFER];
  Queue lpstDunningQueue[3];
  struct s_TimmInter **lapstTimmTab;
  toenBool foenGenDunningLevelLetter(struct s_TimmInter **, int, int, int);

  fovdPushFunctionName("foenGenDunningMultiLetter");

  /*
   * INIT
   */

  loenStatus = TRUE;

  for (loiQueue = 0; loiQueue < 3; loiQueue++)
    {
      InitQueue(&lpstDunningQueue[loiQueue]);
    }

  /*
   * Partition table of TIMM messages
   */

  if (loenStatus == TRUE)
    {
      fovdPrintLog (LOG_DEBUG, "Partitioning messages list\n");
      for (loiTimmDoc = 0; loiTimmDoc < poiTimmDocs; loiTimmDoc++)
        {
          loenStatus = foenFetchDunningLevel(papstTimm[loiTimmDoc], lachzDunningLevel, MAX_BUFFER);
          if (loenStatus == FALSE)
            {
              fovdPopFunctionName();
              fovdPrintLog (LOG_CUSTOMER, "ERROR : foenFetchDunningLevel\n");
              return FALSE;
            }

          fovdPrintLog (LOG_DEBUG, "Message : %d has level : %s\n", loiTimmDoc, lachzDunningLevel);

          iRc = sscanf(lachzDunningLevel, "%d", &loiDunningLevel);
          if (iRc == 1 && loiDunningLevel >= 1 && loiDunningLevel <= 3)
            {
              fovdPrintLog (LOG_DEBUG, "Message : %d on level : %d append to queue : %d\n", loiTimmDoc, loiDunningLevel, loiDunningLevel - 1);
              AppendToQueue(&lpstDunningQueue[loiDunningLevel - 1], papstTimm[loiTimmDoc]);
              fovdPrintLog (LOG_DEBUG, "TIMM appended to queue list\n");
            }
          else
            {
              /*
               * We can't immediately return becouse of allocated Queue internal data structures
               */
              loenStatus = FALSE;
              fovdPrintLog (LOG_CUSTOMER, "ERROR : bad level number\n");
              break;
            }
        }
    }
  
  /*
   * lpstDunningQueue[3] contains 3 sequences of dunning messages for level: 1, 2, 3 
   */

  if (loenStatus == TRUE)
    {
      fovdPrintLog (LOG_DEBUG, "Processing queues\n");
      loiEnvelope = poiEnvelope;
      for (loiQueue = 2; loiQueue >= 0; loiQueue--)
        {
          fovdPrintLog (LOG_DEBUG, "Processing Queue %d\n", loiQueue);
          
          /*
           * Create interface table in dynamic memory for function processing letters
           */

          loiQueueLen = SizeOfQueue(&lpstDunningQueue[loiQueue]);
          if ((lapstTimmTab = (struct s_TimmInter **)calloc((loiQueueLen + 1), sizeof(struct s_TimmInter *))) == NULL)
            {
              loenStatus = FALSE;
              fovdPrintLog (LOG_CUSTOMER, "ERROR : memory allocation for lapstTimmTab\n");
              break;
            }

          fovdPrintLog (LOG_DEBUG, "Dynamic table allocated, size = %d\n", loiQueueLen + 1);

          /*
           * Load dynamic interface table
           */

          loiTimmDoc = 0;
          while (NOT(IsEmptyQueue(&lpstDunningQueue[loiQueue])))
            {
              fovdPrintLog (LOG_DEBUG, "Inserting TIMM message to dynamic table, index = %d\n", loiTimmDoc);

              lapstTimmTab[loiTimmDoc++] = (struct s_TimmInter *)FrontOfQueue(&lpstDunningQueue[loiQueue]);
              RemoveFront(&lpstDunningQueue[loiQueue]);
            }
          lapstTimmTab[loiTimmDoc] = NULL;
          fovdPrintLog (LOG_DEBUG, "Dynamic table updated, items inserted = %d\n", loiTimmDoc);
          
          /*
           * Process given duning level
           */ 
          if (loiTimmDoc > 0)
            {
              fovdPrintLog (LOG_DEBUG, "Level %d not empty, found %d messages\n", loiQueue + 1, loiTimmDoc);  
              loenStatus = foenGenDunningLevelLetter(lapstTimmTab, loiQueue + 1, loiTimmDoc, loiEnvelope);
              if (loenStatus == FALSE)
                {
                  free(lapstTimmTab);
                  fovdPrintLog (LOG_CUSTOMER, "ERROR : foenGenDunningLevelLetter\n");
                  break;
                }

              /*
               * Dynamic table is now useless and may be at last 1 entry long
               */

              free(lapstTimmTab);
              loiEnvelope = FALSE;
            }
          else
            {
              fovdPrintLog (LOG_DEBUG, "Queue %d is empty\n", loiQueue);
            }
        }
    }

  /*
   * Clean up dynamic data structures allocated : we can destroy uninitialized data structures
   */
  fovdPrintLog (LOG_DEBUG, "Clean up all queues\n");  

  for (loiQueue = 0; loiQueue < 3; loiQueue++)
    {
      fovdPrintLog (LOG_DEBUG, "Clean up queue %d\n", loiQueue);  
      DeleteQueue(&lpstDunningQueue[loiQueue]);
    }

  fovdPopFunctionName();  
  return loenStatus;
}


/**************************************************************************************************
 *                                                                                          
 * FUNCTION : foenGenDunningLevelLetter             
 *                                                                                          
 * LOCAL FUNCTIONS USED:                                                                    
 *                                                                                          
 * EXTERNAL FUNCTIONS USED:                                                                 
 *                                                                                          
 **************************************************************************************************
 */

extern tostCustSim *gpstCustSimTab;
extern int goiCustSimTabSize;

extern int *gpiSubCustTab;
extern int goiSubCustTabSize;

extern int foiGetCustSim(int);
extern void fovdFreeCustSim();
extern int foiGetSubCustTree(int, int);

toenBool foenGenDunningLevelLetter(struct s_TimmInter **papstTimmTab, int poiLevel, int poiTimmDocs, int poiEnvelope)
{
  int loiTimmDoc, i, rc, loiLevel;
  toenBool loenStatus;
  static char lachzLevel[MAX_BUFFER];
  static char lachzInvoiceNo[MAX_BUFFER];
  static char lachzDate[MAX_BUFFER];
  static char lachzCurrency[MAX_BUFFER];
  static char lachzAmount[MAX_BUFFER];
  static char lasnzCoId[32];
  static char lasnzDate[16];
  char lasnzItemsNo[32];

  fovdPushFunctionName("foenGenDunningLevelLetter");

  /*
   * First message contains data for envelope and header
   */

  dpstDunningLetter = papstTimmTab[0];
  ASSERT(dpstDunningLetter != NULL);

  if (poiEnvelope == TRUE)
    {
      if (NOT(loenStatus = foenGenDunningLetterEnvelope()))
        {
          fovdPrintLog (LOG_CUSTOMER, "Can't generate envelope\n");
          fovdPopFunctionName();
          return FALSE;
        }
      
      fovdPrintLog (LOG_DEBUG, "Envelope generated\n");
    }
  
  fovdGen("DunningLetterStart", EOL);
  
  sprintf(lachzLevel, "%d", poiLevel);
  fovdGen("DunningLetterLevel", lachzLevel, EOL);

  if (NOT(loenStatus = foenGenDunningLetterHeader()))
    {
      fovdPrintLog (LOG_CUSTOMER, "Can't generate header\n");
      fovdPopFunctionName();
      return FALSE;
    }

  fovdPrintLog (LOG_DEBUG, "Header generated\n");

  /*
   * For each invoice beeing dunned reference must be inserted
   * INVREF = <INVOICE NO, INVOICE AMOUNT, CURRENCY, DUE DATE>
   */

  fovdGen("DunningLetterInvoiceListStart", EOL);  
  for (loiTimmDoc = 0; loiTimmDoc < poiTimmDocs; loiTimmDoc++)
    {
      fovdPrintLog (LOG_DEBUG, "Processing TIMM document no %d\n", loiTimmDoc);
      
      dpstDunningLetter = papstTimmTab[loiTimmDoc];      
      ASSERT(dpstDunningLetter != NULL);

      /*
       * INVOICE NO
       */
      
      if (NOT(loenStatus = foenFetchDunningLetterInvoiceNo(papstTimmTab[loiTimmDoc], lachzInvoiceNo, MAX_BUFFER)))
        {
          /*
           * May be missing
           */
          
          strcpy(lachzInvoiceNo, "");
        }

      fovdPrintLog (LOG_DEBUG, "Invoice no: %s\n", lachzInvoiceNo);
      
      /*
       * INVOICE AMOUNT
       */
      
      if (NOT(loenStatus = foenFetchDunningLetterInvoiceAmount(papstTimmTab[loiTimmDoc], 
                                                              lachzAmount, MAX_BUFFER, lachzCurrency, MAX_BUFFER )))
        {
          fovdPrintLog (LOG_CUSTOMER, "ERROR : foenFetchDunningLetterInvoiceAmount\n");
          fovdPopFunctionName();
          return FALSE;
        }
      
      fovdPrintLog (LOG_DEBUG, "Invoice amount: %s\n", lachzAmount);

      /*
       * INVOICE DUE DATE
       */
      
      if (NOT(loenStatus = foenFetchDunningLetterDueDate(dpstDunningLetter, lachzDate, MAX_BUFFER)))
        {
          fovdPrintLog (LOG_CUSTOMER, "ERROR : foenFetchDunningLetterDueDate\n");
          fovdPopFunctionName();
          return FALSE;
        }

      fovdPrintLog (LOG_DEBUG, "Invoice due date: %s\n", lachzDate);
      
      fovdFormatDate(lachzDate, YY_MM_DD);
      fovdFormatInvoiceNumber(lachzInvoiceNo);
      fovdFormatMoney(lachzAmount);
      fovdGen("DunningLetterInvoice", lachzInvoiceNo, lachzAmount, lachzCurrency, lachzDate, EOL);

    }

  fovdGen("DunningLetterInvoiceListEnd", EOL);

  
  /*
   * DUNING TEXT : one for all messages on given level
   */
  
  fovdGen("DunningLetterTextListStart", EOL);
  if (NOT(loenStatus = foenGenDunningLetterText()))
    {
      fovdPrintLog (LOG_CUSTOMER, "Can't generate text\n");
      fovdPopFunctionName();
      return FALSE;
    }

  fovdGen("DunningLetterTextListEnd", EOL);


  /*
   * SIM LIST
   */

#ifdef _DUNSIM_ON_LEVEL_3_
  if (poiLevel == 3)
    {
#endif
      /*
       * Only for final dunning
       */
      
      fovdPrintLog (LOG_DEBUG, "SIM list search started for customer: %s\n", papstTimmTab[0]->unb->v_0010);

      /*
       * fill table SubCustTab with all subcustomers
       */

      rc = foiGetSubCustTree(atoi (papstTimmTab[0]->unb->v_0010), FALSE);
      if (rc != 0)
        {
          fovdPrintLog (LOG_CUSTOMER, "Can't get the tree of subcustomers\n");
          fovdPopFunctionName();
          return FALSE;
        }
      
      /*
       * for each subcustomer add his SIM cards
       */

      for (i = 0; i < goiSubCustTabSize; i++)
        {
          fovdPrintLog (LOG_DEBUG, "Loading SIM no for customer: %d\n", gpiSubCustTab[i]);
          rc = foiGetCustSim(gpiSubCustTab[i]);
          if (rc != 0)
            {
              fovdPrintLog (LOG_CUSTOMER, "Can't get next SIM\n");
              fovdPopFunctionName();
              return FALSE;
            }
        }

      fovdPrintLog (LOG_DEBUG, "SIM table size: %d\n", goiCustSimTabSize);

      sprintf(lasnzItemsNo, "%d", goiCustSimTabSize);
      
      fovdGen("DunningLetterSimItemListStart", lasnzItemsNo, EOL);
      
      for (i = 0; i < goiCustSimTabSize; i++)
        {
          sprintf(lasnzCoId, "%d", gpstCustSimTab[i].soiCoId);      
          strcpy(lasnzDate, gpstCustSimTab[i].sasnzCoActivated);
          fovdFormatDate(lasnzDate, YY_MM_DD);          

          fovdGen("DunningLetterSimItem", 
                  gpstCustSimTab[i].sasnzCustcode,
                  gpstCustSimTab[i].sasnzCCFName,
                  gpstCustSimTab[i].sasnzCCLName,
                  lasnzCoId,
                  lasnzDate,
                  gpstCustSimTab[i].sasnzSubmShdes,
                  gpstCustSimTab[i].sasnzSubmDes,
                  gpstCustSimTab[i].sasnzDnNum,
                  gpstCustSimTab[i].sasnzCdSmNum,
                  EOL);
        }
 
      fovdFreeCustSim();
     
      fovdGen("DunningLetterSimItemListEnd", EOL);
#ifdef _DUNSIM_ON_LEVEL_3_
    }
#endif

  /*
   * The End
   */

  fovdGen("DunningLetterEnd", EOL);

  fovdPopFunctionName();
  return TRUE;
}

/**************************************************************************************************
 *                                                                                          
 * FUNCTION : foenGenDunningLetterEnvelope
 *                                                                                          
 * LOCAL FUNCTIONS USED:                                                                    
 *                                                                                          
 * EXTERNAL FUNCTIONS USED:                                                                 
 * MODULE: INV_FETCH                                                                        
 *  rc = fetchInvoiceTemporaryAddress(struct s_TimmInter *dpstItemizedBill, char *, int, char *, int, char *, int, char *, int);
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenGenDunningLetterEnvelope()
{
  static char address[4][MAX_BUFFER];
  static char name[MAX_BUFFER];
  int i, rc;

  fovdPushFunctionName ("foenGenDunningLetterEnvelope");
  
  fovdGen("EnvelopeStart", EOL);

  memset(&dostAddress, 0, sizeof(tostAddress));
  rc = GetTempAddressFromDatabase(atoi(dpstDunningLetter->unb->v_0010), &dostAddress);
  if (rc == 0)
    {
      fovdPrintLog (LOG_DEBUG, "Using temporary name and address from DB for envelope\n");  
      fovdGen( "EnvelopeName", dostAddress.sachzLine1, EOL);
      fovdGen( "EnvelopeAddress", 
               dostAddress.sachzLine2, 
               dostAddress.sachzLine3,
               dostAddress.sachzLine4,
               dostAddress.sachzLine5, 
               EOL); 
      
      fovdGen("EnvelopeEnd", EOL);
      
      fovdPopFunctionName ();                                                                    
      return TRUE;
    }

  /*
   * No TMP address found
   */
  
  memset(name, 0, MAX_BUFFER);
  rc = fetchInvoiceCustomerName(dpstDunningLetter, name, MAX_BUFFER);
  if (rc == FALSE)
    {
      sprintf (szTemp, "ERROR in function foenFetchInvoiceCustomerName\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName ();                                                                    
      return FALSE;
    }

  fovdGen("EnvelopeName", name, EOL); 
  
  for (i = 0; i < 4; i++) 
    {
      memset(address[i], 0, MAX_BUFFER);
    }

  rc = fetchInvoiceCustomerAddress(dpstDunningLetter, 
                                   address[0], MAX_BUFFER, address[1], MAX_BUFFER, 
                                   address[2], MAX_BUFFER, address[3], MAX_BUFFER);
  if (rc == FALSE)
    {
      sprintf (szTemp, "ERROR in function foenFetchInvoiceCustomerAddress\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName ();                                                                    
      return FALSE;
    }
  
  fovdGen("EnvelopeAddress", address[0], address[1], address[2], address[3], EOL); 

  fovdGen("EnvelopeEnd", EOL);

  fovdPopFunctionName ();
  return TRUE;
}

/**************************************************************************************************
 *                                                                                          
 * FUNCTION : foenGenDunningLetterHeader             
 *                                                                                          
 * LOCAL FUNCTIONS USED:                                                                    
 *                                                                                          
 * EXTERNAL FUNCTIONS USED:                                                                 
 * toenBool foenFetchInvoiceDate(struct s_TimmInter *ppstSumSheet, char *poszDate, int poiDateBufferLength);
 * toenBool foenFetchInvoiceCustomerAccountNo(struct s_TimmInter *ppstSumSheet, char *poszAccountNo, int poiAcountNoBufferLength);
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenGenDunningLetterHeader() 
{
  toenBool loenState;
  static char loszDate[MAX_BUFFER];
  static char loszAccountNo[MAX_BUFFER];
  static char address[4][MAX_BUFFER];
  static char name[MAX_BUFFER];
  int i, rc;
  
  fovdPushFunctionName("foenGenDunningLetterHeader");

  if (NOT(loenState = fetchInvoiceLetterDate(dpstDunningLetter, loszDate, MAX_BUFFER)))
    {      
      sprintf (szTemp, "ERROR in function foenFetchInvoiceLetterDate.\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName ();                                                                    
      return FALSE;
    }
  
  fovdFormatDate(loszDate, YY_MM_DD);
  fovdGen("DunningLetterHeader", loszDate, EOL);
  
  if (NOT(loenState = foenFetchDunningLetterCustomerAccountNo(dpstDunningLetter, loszAccountNo, MAX_BUFFER)))
    {
      sprintf (szTemp, "ERROR in function foenFetchInvoiceCustomerAccountNo\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName ();                                                                    
      return FALSE;
    }

  fovdGen("DunningLetterAccount", loszAccountNo, EOL);
  
  rc = fetchInvoiceCustomerName(dpstDunningLetter, name, MAX_BUFFER);
  if (rc == FALSE)
    {
      sprintf (szTemp, "ERROR in function foenFetchInvoiceCustomerName\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName ();                                                                    
      return FALSE;
    }

  fovdGen("DunningLetterName", name, EOL); 


  for (i = 0; i < 4; i++) 
    {
      memset(address[i], 0, MAX_BUFFER);
    }
  
  rc = fetchInvoiceCustomerAddress(dpstDunningLetter, 
                                   address[0], MAX_BUFFER, address[1], MAX_BUFFER, 
                                   address[2], MAX_BUFFER, address[3], MAX_BUFFER);
  if (rc == FALSE)
    {
      sprintf (szTemp, "ERROR in function foenFetchInvoiceCustomerAddress\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName ();                                                                    
      return FALSE;
    }
  
  fovdGen("DunningLetterAddress", address[0], address[1], address[2], address[3], EOL); 
  
  fovdPopFunctionName();
  return TRUE;
}

/**************************************************************************************************
 *                                                                                          
 * FUNCTION : foenGenDunningLetterText             
 *                                                                                          
 * LOCAL FUNCTIONS USED:                                                                    
 *                                                                                          
 * EXTERNAL FUNCTIONS USED:                                                                 
 * toenBool foenFetchInvoiceDate(struct s_TimmInter *ppstSumSheet, char *poszDate, int poiDateBufferLength);
 * toenBool foenFetchInvoiceCustomerAccountNo(struct s_TimmInter *ppstSumSheet, char *poszAccountNo, int poiAcountNoBufferLength);
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenGenDunningLetterText()
{
  toenBool loenStatus;
  static char lachzText[5][MAX_BUFFER];
  int loiTextsNo, loiText, loiLine, i;
  
  loiTextsNo = foiCountDunningLetterTexts(dpstDunningLetter);
  for (loiText = 0; loiText < loiTextsNo; loiText++)
    {
      for (i = 0; i < 5; i++)
        {
          memset(lachzText[i], 0, MAX_BUFFER);
        }

      loenStatus = foenFetchDunningLetterText(dpstDunningLetter, loiText, lachzText);
      if (loenStatus == FALSE)
        {
          return TRUE;
        }
      
      for (loiLine = 0; loiLine < 5; loiLine++)
        {
          if (strlen(lachzText[loiLine]) > 0)
            {
              fovdGen("DunningLetterText", lachzText[loiLine], EOL);
            }
        }
    }
  
  return TRUE;
}






