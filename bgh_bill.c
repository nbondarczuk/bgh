/*******************************************************************************
 * LH-Specification GmbH 1996.
 *
 * All rights reserved.
 * Copying of this software or parts of this software is a violation of German
 * and International laws and will be prosecuted.
 *
 * Project  :   BGH
 *
 * File     :   bgh_bill.c
 * Created  :   Mar. 1996
 * Author(s):   B. Michler
 *
 * Modified :
 * 05.11.96	B. Michler	write itemized bills per contract
 * 26.06.96	B. Michler	additional parameter of type stDBCOL * for
 * 				database access routines
 *
 *
 *******************************************************************************/

#if 0	/* just for version.sh */
static char *SCCS_VERSION = "1.52";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>		/* for unlink */


/* User includes                */
#include "bgh.h"
#include "protos.h"
#include "layout.h"             /* needed for pointer to layout-arrays */

#include <types.h>
#include <line_list.h>
#include <stream.h>
#include <generator.h>

#include <acc_enc_gen.h>
#include <sim_enc_gen.h>
#include <itb_gen.h>
#include <wll_gen.h>
#include <dnl_mul_gen.h>
#include <ing_gen.h>
#include <timm.h>
#include <workparval.h>
#include "env_gen.h"
#include "pay_slip_gen.h"

/* User defines                 */


/* Function macros              */


/* Local typedefs               */


/* External globals             */
extern stBGHGLOB stBgh;		/* globals for bgh */
long golLastCustId = -1;

#ifdef BSCS4
extern stIMGTYPES	*pstImgTypes;		/* pointer to array of structures */
extern stDOCTYPES	*pstDocTypes;		/* pointer to array of structures */
extern stIMGTYPESLV	*pstImgTypLevel;       	/* pointer to array of structures */
extern stCUSTIMAGES	*pstCustImg;		/* pointer to array of structures */
#endif

/* Globals                      */
stTIMM        *pstTLegend;		/* legend TIMM */
long goilCustomerId;
int goiBillMedium;

/* Static globals               */
static char szTemp[PATH_MAX];		/* temporay storage for error messages */

#ifdef BSCS4
static char *pszDocName[] = {
  "Balance",
  "Invoice",
  "Itemized",
  "Sumsheet",
  "Roaming",
  "Legend",
  "DCH-Invoice",
  "DCH-Itemized",
  "Dunning",
  "Welcome",
  "IR-Invoice",
  "EC-Invoice",
  "IHH-Note Periodic",
  "IHH-Note Final",
  "ENC"
};
#endif

/* Static function prototypes   */


/* External function prototypes   */

extern void PrintVerInfoLayout (void);

extern void PrintVerInfoBghForm (void);

extern void PrintVerInfoPostPars (void);

extern int foiWriteFileToDb (char *, TYPEID, stDBCOL *);

extern int foiGenDunnings (toenInvType poenInvType, 
                           int poiCustomerId, 
                           char *poszCustomerCode, 
                           int poiBillMedium, 
                           int poiEnvelope, 
                           stDBCOL *ppstAddCol, 
                           toenBool poenInitializeLineList);

extern toenBool foenEqualBillAddress(stTIMM *, stTIMM *);

extern toenBool foenOneEnvelope(stTIMM *, stTIMM *);

extern toenInvType foenInvoiceClass(stTIMM *pstTimm, 
                                    stTIMM *pstBalTimm, 
                                    stTIMM *pstSumTimm);

/*!@#
extern toenBool foenGen_NextDoc(toenDocType poenDoc, toenInvType poenInv, int poiBillMedium, 
                                int poiCustomerId, char *ppsnzCustCode);
*/

extern toenBool foenGen_NextDoc(toenDocType poenDoc, 
                                toenInvType poenInv, 
                                int poiBillMedium, 
                                int poiCustomerId, 
                                char *ppsnzCustCode, 
                                toenBool poenInitializeLineList);

extern int fetchInvoiceCustomerAccountNo(stTIMM *pstTimm, 
                                         char *lasnzCustCode, 
                                         int poiBufLen);

extern int foiAddress_GetRuleNo(stTIMM *ppstInvTimm, 
                                stTIMM *ppstSumTimm);

/* --- bgh_prep.c --- */

extern int    BghPrepareData (stTIMM *, stLAYINF *);

void          fovdFreePrepMemory(void);


/* ---- bgh_form.c ---- */

extern stLAYINF *pstGetFormat (char *);

extern void vdPrintLists (stLAYINF *);


/* ---- layout.c ---- */

extern void vdPostFreeMem (field *);

extern void vdInitLayout (stLAYINF *);

extern int  iOpenDocument (char *, FILE **, int);

extern void vdCloseDocument (FILE *, int);

extern int  iPrintLayout (FILE  *, stLAYINF *, char *, int);

/* ---- bgh_proc.c ---- */

extern int loiGetTimmMsg (TYPEID , stTIMM **, stDBCOL *);

extern int foiGetDunnings (stTIMM ***pstTimm, int *iNrDunnings, stDBCOL *pstColumn);

extern int foiLoyal_ProcServInfo(stDBCOL *pstColumn,
                                 struct s_TimmInter *ppstInv, 
                                 struct s_TimmInter *ppstSum);

/******************************************************************************
 * fovdGenHeader
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *  void
 *
 * RETURNS:
 *  - none -
 ******************************************************************************
 */

static void fovdGenHeader(int poiCustomerId, stDBCOL *ppstAddCol)
{
  char lasnzCustCode[16];

  sprintf(lasnzCustCode, "%ld", poiCustomerId);
  
  fovdGen("Version_" GEN_VER "_" BGH_VER, GEN_VER, BGH_VER, EOL);
  fovdGen("CustomerCode", lasnzCustCode, EOL);
  fovdGen("BillInsert", ppstAddCol->szBillIns, EOL);
}

/******************************************************************************
 * PrintVersInfoGenBill
 *
 * DESCRIPTION:
 * print the version info of BGH_BILL and all customer specific files
 *
 * PARAMETERS:
 *  void
 *
 * RETURNS:
 *  - none -
 ******************************************************************************
 */
void PrintVerInfoGenBill (void)
{
  static char *SCCS_ID = "1.52.1.2";

  printf ("%s\t\t\t%s\n", __FILE__, SCCS_ID);

  /*
   * ADD THE CALLS TO THE PrintVerInfo... PROCEDURES
   * OF ALL CUSTOMER SPECIFIC FILES
   */
  PrintVerInfoBghForm ();
  PrintVerInfoLayout ();
  PrintVerInfoPostPars ();
}


/******************************************************************************
 * folGetNrCopies
 *
 * DESCRIPTION:
 * get the number of copies to print for the document
 *
 * PARAMETERS:
 *  stDBCOL  *pstColumn	   - pointer to additional DB columns
 *  long     lCopies	   - default copies
 *
 * RETURNS:
 *  number of copies to print
 ******************************************************************************
 */
long folGetNrCopies (stDBCOL *pstCol, long lCopies)
{
  int 	i;			/* loop counter */

  /* if at least one copy should be printed, check CUST_IMAGES */
  if ((lCopies > 0) && (pstCustImg != NULL))
  {
    /* check table CUST_IMAGES if it overrules default value */
    for (i = 0; pstCustImg[i].lTypeId != -1; i++)
    {
      if ((pstCustImg[i].lCustId == pstCol->lCustId) &&
	  (pstCustImg[i].lTypeId == pstCol->lTypeId))
      {
	lCopies = pstCustImg[i].lCopies;
	break;
      }
    }
  }

  return (lCopies);
}


/******************************************************************************
 * foiGenDunning
 *
 * DESCRIPTION:
 * generate one document part with dunnings for each dunning level
 * This is a special case of 'foiGenDocument' which processes documents con-
 * taining dunning messages. We can expect multiple dunnings for several
 * levels existing in DOCUMENT_ALL becouse of irregular use of DWH.
 * Load all TIMM messages from DOCUMENT_ALL and stard for all of them
 * external PS generator. Free allocated memory. Processor rceives a table of
 * TimmInterchange, sorts them, and creates as many DNL as necessary. 
 * the number of dunning steps may me found in table DUNNING_PROC 
 *
 * PARAMETERS:
 *  stDBCOL  *pstColumn	   - pointer to additional DB columns
 *
 * RETURNS:
 *  0 - processed all messages for this customer
 ******************************************************************************
 */
/*!@#
int foiGenDunnings (int poiCustomerId, int poiEnvelope, stDBCOL *ppstAddCol)
 */
int foiGenDunnings (toenInvType poenInvType, 
                    int poiCustomerId, 
                    char *poszCustomerCode, 
                    int poiBillMedium, 
                    int poiEnvelope, 
                    stDBCOL *ppstAddCol, 
                    toenBool poenInitializeLineList)
{
  stTIMM **pstCurDNL;
  int	iRc = TRUE;
  int loiLevel = 0;
  int loiTimmDocs = 0, loiTimmDoc = 0;
  toenBool loenStatus;
  
  stDBCOL pstColumn;
  
  fovdPushFunctionName ("foiGenDunnings");
  
  golLastCustId = poiCustomerId;
  pstColumn.lCustId = poiCustomerId;
  iRc = foiGetDunnings (&pstCurDNL, &loiTimmDocs, &pstColumn);
  if (iRc == 1 && loiTimmDocs == 0) 
    {
      fovdPrintLog (LOG_CUSTOMER, "No Dunning Letters for customer %06ld!\n", poiCustomerId);
      fovdPopFunctionName ();
      return loiTimmDocs;
    }
  else if (iRc < 0)
    {
      sprintf (szTemp, "Error loading TIMM messages for customer %06ld!\n", poiCustomerId);
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName ();
      return -1;
    }
  
  fovdPrintLog (LOG_DEBUG, "Found %d Dunning Letters for customer %06ld!\n", loiTimmDocs, poiCustomerId);
  
  /*  if ((loenStatus = foenGen_NextDoc(DOC_DNL_TYPE, 
                                    INV_UNDEFINED_TYPE, 
                                    UNDEFINED_BILL_MEDIUM, 
                                    poiCustomerId, 
                                    NULL)) == FALSE)
*/
  if ((loenStatus = foenGen_NextDoc(DOC_DNL_TYPE, 
				    poenInvType, 
				    poiBillMedium, 
				    poiCustomerId, 
				    poszCustomerCode,
				    poenInitializeLineList)) == FALSE)
    {
      sprintf (szTemp, "Can't init  next document for customer %ld!\n", poiCustomerId);
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName ();
      return -1;
    }  

  /*
   * Version control header
   */
  
  if (TRUE == poiEnvelope)
    {
      fovdGenHeader(poiCustomerId, ppstAddCol);
    }
  
  /*
   * Try to create one document for each duning level 
   * if any TIMM on this level exists
   */
  
  loenStatus = foenGenDunningMultiLetter(pstCurDNL, loiTimmDocs, loiLevel, poiEnvelope);
  if (loenStatus != TRUE) 
    {
      fovdPrintLog (LOG_CUSTOMER, "ERROR : Multi Dunning Letter for customer %06ld !\n", poiCustomerId);
      sprintf (szTemp, "DUNNING LETTER NOT GENERATED FOR CUSTOMER %ld on level %d!\n", poiCustomerId, loiLevel);
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      return -1;
    } 
  else
    {
      fovdPrintLog (LOG_DEBUG, "Dunning Letter OK for customer %06ld\n",  poiCustomerId);
    } 
  
  /*
   * Free space allocated for TimmInter structures 
   * for each TIMM message loades from DB
   */
  
  if (pstCurDNL != NULL) 
    {
      for (loiTimmDoc = 0; loiTimmDoc < loiTimmDocs; loiTimmDoc++)
	{
          free_timm (pstCurDNL[loiTimmDoc]);
	}
      
      free (pstCurDNL);
    }
  
  fovdPopFunctionName ();
  return loiTimmDocs;
}

/******************************************************************************
 * foiGenerateBill
 *
 * DESCRIPTION:
 * process all TIMM messages for one customer and generate the bill
 *
 * PARAMETERS:
 *  stTIMM   *pstTimm      - pointer to C-structure of invoice
 *  stDBCOL  *pstColumn	   - pointer to additional DB columns
 *  TYPEID   enType	   - typeid of message
 *
 * RETURNS:
 *  STATUS_OK - processed all messages for this customer
 *  STATUS_REJECT - this document is rejected and all TIMMs must be marked with 'R'
 *  STATUS_ERROR - stop processing
 ******************************************************************************
 */

extern void fovdSumSheet_CheckCallList(stTIMM *pstSumTimm);
extern void fovdBalSheet_CheckReverse(stTIMM *ppstTimmInter);

int foiGenerateBill (stTIMM *pstTimm, stDBCOL *pstColumn, TYPEID enType) 
{
  stTIMM        *pstCurTimm = NULL;
  stTIMM        *pstSumTimm = NULL;
  stTIMM        *pstBalTimm = NULL;
  stTIMM        *pstRoaTimm = NULL;
  stTIMM        *pstWelTimm = NULL;
  stTIMM        *pstINPTimm = NULL;
  stTIMM        *pstINLTimm = NULL;
  stTIMM        *pstENCTimm = NULL;
  TYPEID        loenProcessedType; /* document type fetched from TIMM text */
  int		iRet = 0;          /* dunning geneartor status */
  int           stRet = 0;         /* document generator status */
  toenBool      loenStatus;        /* general status flag */
  stDBCOL	stAddCol;          /* DOCUMENT_ALL attribute structure */
  unsigned int  i;
  char          szCustCode[16];
  long          lId;
  toenDocType   loenDoc;
  toenInvType   loenInv;
  int           loiBillMedium;
  char          lasnzCustCode[MAX_CUSTCODE_LEN];
  char laszPaymentType [16];
  int loiRuleNo;
  toenMailingType loenMailingType;

  fovdPushFunctionName ("foiGenerateBill");
  ASSERT (pstTimm != NULL);
  
  lId = atol (pstTimm->unb->v_0010);
  sprintf(szCustCode, "%ld", lId);
  fovdPrintLog (LOG_CUSTOMER, "Processing customer: %ld\n", lId);
  goilCustomerId = lId;
  golLastCustId = lId;
  
  goiBillMedium = UNDEFINED_BILL_MEDIUM;
    
  /*-----------------------------------------------------------------------------------*/
  /* --- what kind of TIMM message we get ----- */
  /*-----------------------------------------------------------------------------------*/
  if (!strcmp(pstTimm->timm->bgm->v_1000, "BCH-PAYMENT"))
    {
      loenProcessedType = INV_TYPE;
      
      /*-----------------------------------------------------------------------------------*/
      /* --- get supplementary TIMM messages --- */
      /*-----------------------------------------------------------------------------------*/
      
      if (foiGetSumsheet (&pstSumTimm, &stAddCol) != 0) 
        {
          sprintf (szTemp, "No sumsheet message for customer %ld\n", pstTimm->unb->v_0010);
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          fovdPopFunctionName ();
          return STATUS_ERROR;
        }
      
      if (foiGetBalance (&pstBalTimm, &stAddCol) != 0) 
        {
          sprintf (szTemp, "No balance message for customer %ld\n", pstTimm->unb->v_0010);
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          fovdPopFunctionName ();
          return STATUS_ERROR;
        }
      
      if (foiGetRoaming (&pstRoaTimm, &stAddCol) != 0) 
        {
          sprintf (szTemp, "No roaming message for customer %ld\n", pstTimm->unb->v_0010);
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          fovdPopFunctionName ();
          return STATUS_ERROR;
        }
      
      if (stBgh.bWelcomeGen == TRUE) 
        {
          if (foiGetWelcome (&pstWelTimm, &stAddCol) != 0)
            {
              fovdPrintLog (LOG_DEBUG, "foiGenerateBill: No welcome message found\n");
              pstWelTimm = NULL;
            }
        }
      
      if (stBgh.bINPGen == TRUE) 
        {
          if (loiGetTimmMsg (INH_INP, &pstINPTimm, &stAddCol) != 0)
            {
              fovdPrintLog (LOG_DEBUG, "foiGenerateBill: No message found\n");
              pstINPTimm = NULL;
            }
          else
            {
              fovdPrintLog (LOG_DEBUG, "foiGenerateBill: INP message found\n");
            }	    
        }
      
      if (stBgh.bINLGen == TRUE) 
        {
          if (loiGetTimmMsg (INH_INL, &pstINLTimm, &stAddCol) != 0)
            {
              fovdPrintLog (LOG_DEBUG, "foiGenerateBill: No message found\n");
              pstINLTimm = NULL;
            }
          else
            {
              fovdPrintLog (LOG_DEBUG, "foiGenerateBill: INL message found\n");
            }
        }
      
      /*-----------------------------------------------------------------------------------*/
      /* --- work around defect 85 ------ */
      /*-----------------------------------------------------------------------------------*/
      
      fovdSumSheet_CheckCallList(pstSumTimm);
      
      /*-----------------------------------------------------------------------------------*/
      /* --- work around corrective invoice ------ */
      /*-----------------------------------------------------------------------------------*/
      
      fovdBalSheet_CheckReverse(pstBalTimm);

      /*-----------------------------------------------------------------------------------*/
      /* --- prepare WORKPARVAL info retrieval ------ */
      /*-----------------------------------------------------------------------------------*/
      
      if ((stRet = foiParVal_NextDoc(pstSumTimm)) < 0)
        {
          sprintf (szTemp, "Can't init WORKPARVAL table for customer %ld!\n", lId);
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          fovdPopFunctionName ();
          return STATUS_ERROR;
        }

      /*-----------------------------------------------------------------------------------*/
      /* --- prepare LOYAL info retrieval ------ */
      /*-----------------------------------------------------------------------------------*/
      
      if ((stRet = foiLoyal_ProcServInfo(pstColumn, pstTimm, pstSumTimm)) < 0)
        {
          sprintf (szTemp, "Can't process LOYAL info for customer %ld!\n", lId);
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          fovdPopFunctionName ();
          return STATUS_ERROR;
        }
            
      /*-----------------------------------------------------------------------------------*/
      /* --- invoice type dependent output stream handling  ------ */
      /*  ustalanie wlasciwosci wyjscia, specyficznych dla dokumentu */        
      /*-----------------------------------------------------------------------------------*/
      
      loenDoc = DOC_INV_TYPE;
      if ((loenInv = foenInvoiceClass(pstTimm, pstBalTimm, pstSumTimm)) == INV_UNDEFINED_TYPE)
        {
          sprintf (szTemp, "Can't classify document for customer %ld!\n", lId);
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          fovdPopFunctionName ();
          return STATUS_ERROR;
        }
      
      if ((loiBillMedium = foiGetBillMedium(pstTimm)) == UNDEFINED_BILL_MEDIUM)
        {
          sprintf (szTemp, "Can't get bill medium !\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          fovdPopFunctionName ();
          return STATUS_ERROR;
        }
	
      goiBillMedium = loiBillMedium;

      if ((stRet = fetchInvoiceCustomerAccountNo(pstTimm, lasnzCustCode, MAX_CUSTCODE_LEN)) == FALSE)
        {
          sprintf (szTemp, "Can't get customer code !\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          fovdPopFunctionName ();
          return STATUS_ERROR;
        }

      memset(laszPaymentType, 0x00, 16);
	    
      if (FALSE == foenInvFetch_PaymentType(pstTimm, 
                                            laszPaymentType, 
                                            16))
        {
          sprintf (szTemp, "Can't get payment type in function foenInvFetch_PaymentType\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          fovdPopFunctionName ();
          return STATUS_ERROR;      
        }	    

      if ((loiRuleNo = foiAddress_GetRuleNo(pstTimm, pstSumTimm)) == -1)
        {
          sprintf (szTemp, "Can't guess mailing rule for customer %ld\n", lId);
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          fovdPopFunctionName ();
          return STATUS_ERROR;                                        
        }
            
      fovdPrintLog (LOG_CUSTOMER, "Creating INVOICE with Doc: %d, Inv: %d, BM: %d, Code: %s\n", 
                    loenDoc, 
                    loenInv, 
                    loiBillMedium, 
                    lasnzCustCode);
      
      if ((loenStatus = foenGen_NextDoc(loenDoc, 
                                        loenInv, 
                                        loiBillMedium, 
                                        lId, 
                                        lasnzCustCode, 
                                        TRUE)) == FALSE)
        {
          sprintf (szTemp, "Can't init  next document for customer %ld!\n", lId);
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          fovdPopFunctionName ();
          return STATUS_ERROR;
        }
	
      /*-----------------------------------------------------------------------------------*/
      /* --- print invoice main section for INV and SIM messages ------ */
      /*-----------------------------------------------------------------------------------*/
      
      fovdGenHeader(lId, pstColumn);
      
      stRet = foiGenInvoice(pstTimm, pstSumTimm, loiRuleNo);  
      if (stRet != STATUS_OK)
        {
          sprintf (szTemp, "Invoice document not created for customer %ld!\n", lId);
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          fovdPopFunctionName ();
          return stRet;
        }
      else
        {
          fovdPrintLog (LOG_DEBUG, "foiGenerateBill: Invoice document created\n");
        }
      
      /*-----------------------------------------------------------------------------------*/
      /* --- print account enlosure section --- */
      /*   zalacznik z informacja o kliencie, saldo, oplaty za poszczegolne telefony */
      /*-----------------------------------------------------------------------------------*/
      
      fovdPrintLog (LOG_CUSTOMER, "Creating ACCOUNT ENCLOSURE document\n");
      stRet = genEnclosureAccount(INV_TYPE, 
                                  pstTimm,  
                                  pstSumTimm, 
                                  pstBalTimm, 
                                  loiRuleNo);
      if (stRet != TRUE) 
        {
          sprintf (szTemp, "Account enclosure document not created for customer %ld!\n", lId);
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          fovdPopFunctionName ();
          return STATUS_ERROR;
        }
      else
        {
          fovdPrintLog (LOG_DEBUG, "foiGenerateBill: Account enclosure document created\n");
        }
      
      /*-----------------------------------------------------------------------------------*/
      /* --- print directory number enlosure section --- */
      /* abonamenty, access, itemized bill, brany z sumsheet TIMM message*/
      /*-----------------------------------------------------------------------------------*/
      
      fovdPrintLog (LOG_CUSTOMER, "Creating SIM ENCLOSURE document\n");
      if (TRUE == foenOneEnvelope(pstTimm, pstSumTimm) 
          || ((EQ(laszPaymentType,"D")) && loiRuleNo != 2))
        {
          stRet = foenGenSimEnclosure(SUM_TYPE, pstSumTimm, pstRoaTimm);
        }
      else
        {
          stRet = foenGenSimEnclosure(INV_TYPE, pstSumTimm, pstRoaTimm);
        }
      
      if (stRet != TRUE) 
        {
          sprintf (szTemp, "SIM enclosure document not created for customer %ld!\n", lId);
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          fovdPopFunctionName ();
          return STATUS_ERROR;
        }
      else
        {
          fovdPrintLog (LOG_DEBUG, "foiGenerateBill: SIM enclosure document created\n");
        }
	
      /*-----------------------------------------------------------------------------------*/
      /* --- print WELCOME --- */
      /*-----------------------------------------------------------------------------------*/
      
      if (stRet == TRUE && stBgh.bWelcomeGen == TRUE && pstWelTimm != NULL) 
        {
          fovdPrintLog (LOG_CUSTOMER, "Creating WLL document\n");
          stRet = foenGenWelcomeLetter(pstWelTimm, pstTimm, FALSE);
          if (stRet != TRUE)
            {
              sprintf (szTemp, "Welcome letter document not created for customer %ld!\n", lId);
              macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
              fovdPopFunctionName ();
              return STATUS_ERROR;
            } 
          else
            {
              fovdPrintLog (LOG_DEBUG, "foiGenerateBill: Welcome letter document created\n");
            }
        }
      else
        {
          fovdPrintLog (LOG_DEBUG,  "foiGenerateBill: Welcome letter document not created - ");
          if (stRet != TRUE)
            {
              fovdPrintLog (LOG_DEBUG,  "previous error occured.\n");
            }
          else if (stBgh.bWelcomeGen != TRUE)
            {
              fovdPrintLog (LOG_DEBUG,  "it is not needed.\n");
            }
          else
            {
              fovdPrintLog (LOG_DEBUG,  "foiGenerateBill: TIMM message not loaded.\n");
            }
        }

      /*-----------------------------------------------------------------------------------*/
      /* --- print DUNNING  --- */
      /*-----------------------------------------------------------------------------------*/

      if (stRet == TRUE && stBgh.bDunningGen == TRUE)
        {
          fovdPrintLog (LOG_CUSTOMER, "Creating DNL document\n");
          iRet = foiGenDunnings (loenInv, lId, lasnzCustCode, loiBillMedium, FALSE, pstColumn,FALSE);
          if (iRet < 0) 
            {
              stRet = FALSE;
              sprintf (szTemp, "Dunning letter document not created for customer  %ld!\n", lId);
              macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
            } 
          else
            {
              stRet = TRUE;
              fovdPrintLog (LOG_DEBUG, "foiGenerateBill: Dunning letter document created\n");              
            }
        }
      else
        {
          fovdPrintLog (LOG_DEBUG,  "foiGenerateBill: DUN document not created\n");
        }	

      /*-----------------------------------------------------------------------------------*/
      /* --- print INTEREST NOTE PARIODIC  --- */
      /*-----------------------------------------------------------------------------------*/

      if (stRet == TRUE && stBgh.bINPGen == TRUE && pstINPTimm != NULL)
        {
          fovdPrintLog (LOG_CUSTOMER, "Creating INP document\n");
          stRet = foenINPGen(pstINPTimm, FALSE); 
          if (stRet != TRUE) 
            {
              sprintf (szTemp, "Interest note periodic document not created for customer  %ld!\n", lId);
              macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
              fovdPopFunctionName ();
              return STATUS_ERROR;
            } 
          else
            {
              fovdPrintLog (LOG_DEBUG, "foiGenerateBill: Interest note document created\n");              
            }
        }
      else
        {
          fovdPrintLog (LOG_DEBUG,  "foiGenerateBill: INP document not created\n");
        }
	

      /*-----------------------------------------------------------------------------------*/
      /* --- print INTEREST NOTE LAST  --- */
      /*-----------------------------------------------------------------------------------*/

      if (stRet == TRUE && stBgh.bINLGen == TRUE && pstINLTimm != NULL)
        {
          fovdPrintLog (LOG_CUSTOMER, "Creating INL document\n");
          stRet = foenINLGen(pstINLTimm, FALSE); 
          if (stRet != TRUE) 
            {
              sprintf (szTemp, "Interest note last document not created for customer  %ld!\n", lId);
              macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
              fovdPopFunctionName ();
              return STATUS_ERROR;
            } 
          else
            {
              fovdPrintLog (LOG_DEBUG, "foiGenerateBill: Interest note last document created\n");              
            }
        }
      else
        {
          fovdPrintLog (LOG_DEBUG,  "foiGenerateBill: INL document not created\n");
        }

      /*---------------------------------------------------------------------------------------*/
      /* --- Print additional ENCLOSURE doc when SUM address is not equal with INV address --- */
      /* --- that means that CUSTOMER_ALL.CSSUMADDR = 'C' for this customer that means     --- */
      /* --- the contract address will be used for sum sheet                               --- */ 
      /*---------------------------------------------------------------------------------------*/
      
      if (stRet == TRUE 
          && FALSE == foenOneEnvelope(pstTimm, pstSumTimm) 
          && (NOT(EQ(laszPaymentType, "D")) 
              || (EQ(laszPaymentType, "D") && loiRuleNo == 2)))
        {
          fovdPrintLog (LOG_DEBUG, "foiGenerateBill: Flushing document output\n");
          if ((loenStatus = foenGen_FlushAll()) == FALSE)
            {
              sprintf (szTemp, "Can't flush document for customer %ld\n", lId);
              macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
              fovdPopFunctionName ();
              return STATUS_ERROR;                                        
            }
	    
          /*
           * Create ENC document
           */
	    
          fovdPrintLog (LOG_CUSTOMER, "Creating ENCLOSURE\n");          
	    
          /*
           * Turn on enclosure filling mode
           */
	    
          loenDoc = DOC_INV_TYPE;
          loenInv = INV_ENCLOSURE_TYPE;
          fovdPrintLog (LOG_CUSTOMER, "Creating ENCLOSURE with Doc: %d, Inv: %d, BM: %d\n", 
                        loenDoc, 
                        loenInv, 
                        loiBillMedium);
	    
          if ((loenStatus = foenGen_NextDoc(loenDoc, 
                                            loenInv, 
                                            loiBillMedium, 
                                            lId, 
                                            lasnzCustCode, 
                                            TRUE)) == FALSE)
            {
              sprintf (szTemp, "Can't init  next document for customer %ld!\n", lId);
              macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
              fovdPopFunctionName ();
              return STATUS_ERROR;
            }
	    
          fovdGenHeader(lId, pstColumn);
	    
          /*
           * ACCOUNT enclosure
           */
          
          fovdPrintLog (LOG_CUSTOMER, "Creating ACCOUNT ENCLOSURE document\n");          
          stRet = genEnclosureAccount(ENC_TYPE, 
                                      pstTimm,  
                                      pstSumTimm,
                                      pstBalTimm, 
                                      loiRuleNo);
          if (stRet != TRUE) 
            {
              sprintf (szTemp, "Account enclosure document not created for customer %ld!\n", lId);
              macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
              fovdPopFunctionName ();
              return STATUS_ERROR;
            }
          else
            {
              fovdPrintLog (LOG_DEBUG, "foiGenerateBill: Account enclosure document created\n");
            }
	    
          /*
           * SIM enclosure
           */
	    
          fovdPrintLog (LOG_CUSTOMER, "Creating SIM ENCLOSURE document\n");          
          stRet = foenGenSimEnclosure(ENC_TYPE, pstSumTimm, pstRoaTimm);
          if (stRet != TRUE) 
            {
              sprintf (szTemp, "Enclosure document not created for customer %ld!\n", lId);
              macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
              fovdPopFunctionName ();
              return STATUS_ERROR;
            }
          else
            {
              fovdPrintLog (LOG_DEBUG, "foiGenerateBill: Enclosure document created\n");
            }	  
        }
      else
        {
          fovdPrintLog (LOG_DEBUG,  "foiGenerateBill: ENC document not created\n");
        }
	
      /*
       * PAYMENT SLIP section - Bank Mailing
       */
	
      if (stRet == TRUE) 
        {
          if (EQ(laszPaymentType, "D") && (loiRuleNo == 3 || loiRuleNo == 4))
            {
              fovdPrintLog (LOG_DEBUG, "foiGenerateBill: Flushing document output\n");
              if (FALSE == (loenStatus = foenGen_FlushAll()))
                {
                  sprintf (szTemp, "Can't flush document for customer %ld\n", lId);
                  macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
                  fovdPopFunctionName ();
                  return STATUS_ERROR;                                        
                }

              loenDoc = DOC_INV_TYPE;
              loenInv = INV_ENCLOSURE_TYPE;
              /*
              loenDoc = DOC_INV_TYPE;
              loenInv = INV_PAYMENT_TYPE;
              */
              if (FALSE == (loenStatus = foenGen_NextDoc(loenDoc, 
                                                         loenInv, 
                                                         loiBillMedium, 
                                                         lId, 
                                                         lasnzCustCode, 
                                                         TRUE)))
                {
                  sprintf (szTemp, "Can't init  next document for customer %ld!\n", lId);
                  macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
                  fovdPopFunctionName ();
                  return STATUS_ERROR;
                }
	      
	      fovdGenHeader(lId, pstColumn);
	      
              loenMailingType = MAILING_PAY;

              if (FALSE == (loenStatus = foenGenPaymentSlip(pstTimm, 
                                                            pstSumTimm,
                                                            loenMailingType,
                                                            loiRuleNo)))
                {
                  sprintf (szTemp, "Can't create payment slip for customer %ld!\n", lId);
                  macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
                  fovdPopFunctionName ();
                  return STATUS_ERROR;
                }
            }
        }
	
      /*-----------------------------------------------------------------------------------*/
      /* --- clean up all this stuff --- */
      /*-----------------------------------------------------------------------------------*/
	
      if (pstSumTimm != NULL)
        {
          free_timm (pstSumTimm);
        }
	
      if (pstBalTimm != NULL)
        {
          free_timm (pstBalTimm);
        }
	
      if (pstRoaTimm != NULL)
        {
          free_timm (pstRoaTimm);    
        }
	
      if (pstWelTimm != NULL)
        {
          free_timm (pstWelTimm);
        }

      if (pstINPTimm != NULL)
        {
          free_timm (pstINPTimm);
        }
	
      if (pstINLTimm != NULL)
        {
          free_timm (pstINLTimm);
        }      
    }
  else if (!strcmp(pstTimm->timm->bgm->v_1000, "DWH-WELCOME"))
    {
      /*-----------------------------------------------------------------------------------*/
      /* --- print WELCOME LETTER --- */
      /*-----------------------------------------------------------------------------------*/
	
      loenProcessedType = WLL_DWH;
      loenDoc = DOC_WLL_TYPE;
      loenInv = INV_UNDEFINED_TYPE;
      loiBillMedium = UNDEFINED_BILL_MEDIUM;
      fovdPrintLog (LOG_CUSTOMER, "Creating WELCOME LETTER with Doc: %d, Inv: %d, BM: %d\n", 
                    loenDoc, 
                    loenInv, 
                    loiBillMedium);
	
      if ((loenStatus = foenGen_NextDoc(loenDoc, 
                                        loenInv, 
                                        UNDEFINED_BILL_MEDIUM, 
                                        lId, 
                                        NULL, 
                                        TRUE)) == FALSE)
        {
          sprintf (szTemp, "Can't init  next document for customer %ld!\n", lId);
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          fovdPopFunctionName ();
          return STATUS_ERROR;
        }
      
      fovdGenHeader(lId, pstColumn);
	
      if ((stRet = foenGenWelcomeLetter(pstTimm, NULL, TRUE)) == FALSE)
        {
          sprintf (szTemp, "No welcome letter created for customer %ld!\n", lId);
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          fovdPopFunctionName ();
          return STATUS_ERROR;
        } 
      else
        {
          fovdPrintLog (LOG_DEBUG, "foiGenerateBill: Welcome letter processed\n");
        }
    }
  else if (!strcmp(pstTimm->timm->bgm->v_1000, "DWH-DUNNING"))
    {
      /*-----------------------------------------------------------------------------------*/
      /* --- print DUNNING LETTER --- */
      /*-----------------------------------------------------------------------------------*/
	
      loenProcessedType = DNL_DWH;      
      fovdPrintLog (LOG_CUSTOMER, "Creating DUNING LETTER\n");      
      if ((iRet = foiGenDunnings (loenProcessedType, 
                                  lId, 
                                  lasnzCustCode, 
                                  UNDEFINED_BILL_MEDIUM, 
                                  TRUE, 
                                  pstColumn, 
                                  TRUE)) < 0)
        {
          stRet = FALSE;
          sprintf (szTemp, "No duning letter created for customer %ld!\n", lId);
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
        } 
      else
        {
          stRet = TRUE;
          fovdPrintLog (LOG_DEBUG, "foiGenerateBill: Dunning letter processed: %d, typeid: %d\n", 
                        iRet, 
                        loenProcessedType);
        }
    }
  else if (!strcmp(pstTimm->timm->bgm->v_1000, "INH-INP"))
    {
      /*-----------------------------------------------------------------------------------*/
      /* --- print INTEREST NOTE PERIODIC --- */
      /*-----------------------------------------------------------------------------------*/
	
      loenProcessedType = INH_INP;      	
      loenDoc = DOC_INP_TYPE;
      loenInv = INV_UNDEFINED_TYPE;
      loiBillMedium = UNDEFINED_BILL_MEDIUM;
      fovdPrintLog (LOG_CUSTOMER, "Creating INTEREST NOTE PERIODIC with Doc: %d, Inv: %d, BM: %d\n", 
                    loenDoc, 
                    loenInv, 
                    loiBillMedium);
		
      if ((loenStatus = foenGen_NextDoc(loenDoc, 
                                        loenInv, 
                                        UNDEFINED_BILL_MEDIUM, 
                                        lId, 
                                        NULL, 
                                        TRUE)) == FALSE)
        {
          sprintf (szTemp, "Can't init  next document for customer %ld!\n", lId);
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          fovdPopFunctionName ();
          return STATUS_ERROR;
        }
	
      fovdGenHeader(lId, pstColumn);
	
      if ((stRet = foenINPGen(pstTimm, TRUE)) == FALSE)
        {
          sprintf (szTemp, "No interest note periodic created for customer %ld!\n", lId);
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          fovdPopFunctionName ();
          return STATUS_ERROR;
        } 
	
      fovdPrintLog (LOG_DEBUG, "foiGenerateBill: Periodic interest note processed\n");
    }
  else if (!strcmp(pstTimm->timm->bgm->v_1000, "INH-INL"))
    {
      /*-----------------------------------------------------------------------------------*/
      /* --- print INTEREST NOTE LAST --- */
      /*-----------------------------------------------------------------------------------*/
	
      loenProcessedType = INH_INL;      
      loenDoc = DOC_INL_TYPE;
      loenInv = INV_UNDEFINED_TYPE;
      loiBillMedium = UNDEFINED_BILL_MEDIUM;
      fovdPrintLog (LOG_CUSTOMER, "Creating INTEREST NOTE PERIODIC with Doc: %d, Inv: %d, BM: %d\n", 
                    loenDoc, 
                    loenInv, 
                    loiBillMedium);

      if ((loenStatus = foenGen_NextDoc(loenDoc, 
                                        loenInv, 
                                        UNDEFINED_BILL_MEDIUM, 
                                        lId, 
                                        NULL, 
                                        TRUE)) == FALSE)
        {
          sprintf (szTemp, "Can't init  next document for customer %ld!\n", lId);
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          fovdPopFunctionName ();
          return STATUS_ERROR;
        }
	
      fovdGenHeader(lId, pstColumn);
	
      if ((stRet = foenINLGen(pstTimm, TRUE)) == FALSE)
        {
          sprintf (szTemp, "No interest note periodic created for customer %ld!\n", lId);
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          fovdPopFunctionName ();
          return STATUS_ERROR;
        } 
	
      fovdPrintLog (LOG_DEBUG, "foiGenerateBill: Last interest note processed\n");
    }
  else
    {
      sprintf (szTemp, "Bad type of TIMM message: %s\n", pstTimm->timm->bgm->v_1000);
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName ();
      return STATUS_ERROR;
    }
    
  /*-----------------------------------------------------------------------------------*/
  /* --- update document stream output state ---- */
  /*-----------------------------------------------------------------------------------*/

  if (loenProcessedType == DNL_DWH && iRet == 0)
    {
      fovdPrintLog (LOG_DEBUG, "foiGenerateBill: Not flushing output queue, no DNL found\n");
    }
  else if (stRet == TRUE)
    {
      fovdPrintLog (LOG_DEBUG, "foiGenerateBill: Flushing document output\n");
      if ((loenStatus = foenGen_FlushAll()) == FALSE)
        {
          sprintf (szTemp, "Can't flush document for customer %ld\n", lId);
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          fovdPopFunctionName ();
          return STATUS_ERROR;                                        
        }
    }
    
  fovdPopFunctionName ();
    
  if (stRet == FALSE)
    {
      return STATUS_ERROR;
    }
    
  return STATUS_OK;
}


  

