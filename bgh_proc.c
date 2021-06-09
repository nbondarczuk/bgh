/*******************************************************************************
 * LH-Specification GmbH 1996.
 *
 * All rights reserved.
 * Copying of this software or parts of this software is a violation of German
 * and International laws and will be prosecuted.
 *
 * Project  :   BGH
 *
 * File     :   bgh_proc.c
 * Created  :   Mar. 1996
 * Author(s):   B. Michler
 *
 * Modified :
 * 01.11.96	B. Michler	removed MarkWithX - it is done with the 
 *				rowid stack now
 * 26.06.96	B. Michler	additional parameter of type stDBCOL * for
 * 				database access routines
 *
 *
 * The processing of the customers is done in this module.
 * The main routine calls 'foiProcessAll' with the number of customers to
 * Further processing is done like:
 * - 'foiProcessAll' open and connects to the database, then it calls for
 *                 every customer
 * - 'loiProcessCustomer' reads the next invoice from the database, 
 *                 checks the version and parses the TIMM-message. Afterwards
 *                 it calls
 * - 'foiGenerateBill' with a pointer to the C-structure of the invoice. This
 *                 procedure is from the data preprocessor; it may subsequently
 *                 call the procedures 'foiGetSumsheet', 'foiGetItemizedBills',
 *                 'foiGetRoaming', 'foiGetBalance' and 'foiGetLegend' to 
 *                 retrieve the various documents belonging to the invoice. 
 *                 After returning to
 * - 'loiProcessCustomer' the invoice C-stucture is discarded and the database 
 *                 changes are committed.
 *
 *
 *******************************************************************************/

#if 0	/* just for version.sh */
static char *SCCS_VERSION = "1.33";
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>		/* for 'access' */

/* User includes                */
#include "bgh.h"
#include "protos.h"
#include <types.h>
#include <gen.h>
#include <inv_gen.h>
#include <itb_gen.h>
#include <acc_enc_gen.h>
#include <sim_enc_gen.h>
#include <wll_gen.h>
#include <dnl_mul_gen.h>


/* User defines                 */
#define CLEANUP			0		/* free memory after run */

#define NEWDBHANDLING		1
#define MEASURE_TIME         	0
#define DONT_PARSE_WRONG_VERSION 1

/* Function macros              */


/* Local typedefs               */


/* External globals             */
extern long golLastCustId;
extern stBGHGLOB stBgh;			/* bgh globals... */
extern tostBGHStat gostBGHStat;  
extern int  aiParsVersion[];		/* parser.y, the versions array */
#ifdef BSCS4
extern stIMGTYPES	*pstImgTypes;		/* pointer to array of structures */
extern stDOCTYPES	*pstDocTypes;		/* pointer to array of structures */
extern stIMGTYPESLV	*pstImgTypLevel;       	/* pointer to array of structures */
extern stCUSTIMAGES	*pstCustImg;		/* pointer to array of structures */
#endif

/* Globals                      */
long    lLenTIMM = 0L;
long    lNrTIMM = 0L;
long    golCust = 0L;


/* Static globals               */
static char szBaseName[PATH_MAX];   /* basename of TIMM-message filename */
static char szFileName[PATH_MAX];   /* buffer for temp. filename */

/* Static function prototypes   */

/* External function prototypes   */
#if CLEANUP
extern void fovdFreeFormMem (void);	/* bgh_form.c */
extern void fovdFreeFixstringMem (void); /* bgh_prep.c */
#endif

extern int foiSetFirst (char *szFileName);
extern int foiGetNext (char *szFileName, char *szFullName, int iFullLen);

extern int foiGen_RollBackWork();
extern int foiGen_SavePoint();

extern toenBool foenGen_Init();

/******************************************************************************
 * PrintVersInfoBghProc
 *
 * DESCRIPTION:
 * print the version info of all BGH_PROC
 *
 * PARAMETERS:
 *  void
 *
 * RETURNS:
 *  - none -
 ******************************************************************************
 */
void PrintVerInfoBghProc (void)
{
  static char *SCCS_ID = "1.33.1.4";

  printf ("%s\t\t\t%s\n", __FILE__, SCCS_ID);
}


/******************************************************************************
 * loiGetTimmVersion
 *
 * DESCRIPTION:
 * retrieves the version of the TIMM-message so that a correct parser
 * can be chosen
 *
 * PARAMETERS:
 *  char *szTimm         - TIMM string
 *  char *szVersion      - pointer to buffer which takes the version 
 *                         string as return
 *  int  iVerLen         - length of version buffer
 *
 * RETURNS:
 *  0 - found a version info
 * -1 - otherwise
 ******************************************************************************
 */
static int loiGetTimmVersion (char *szTimm, char *szVersion, unsigned int uVerLen)
{
    char *pTemp;
    int  rc;
    unsigned int  i;

    fovdPushFunctionName ("loiGetTimmVersion");

    rc = 0;
    pTemp = szTimm;

    /*
     * search UNH (the segment with the version info)
     */
    pTemp = (char *) strstr (szTimm, "UNH+");
    if (pTemp == NULL) {
      fovdPopFunctionName ();
      return (-1);
    }

    /*
     * now we are at the beginning of "UNH+",
     * set the pointer past this string,
     * overread one '+' and one ':' and we should
     * be at the version string which has to be 
     * 3 characters long
     */
    pTemp += 4;
    pTemp = (char *) strchr (pTemp, '+');
    if (pTemp == NULL) {
      fovdPopFunctionName ();
      return (-1);
    }
    pTemp = (char *) strchr (pTemp, ':');
    if ((pTemp == NULL) || (strlen (pTemp) < 5)) {
      fovdPopFunctionName ();
      return (-1);
    }
    
    pTemp++;
    if (pTemp[3] != ':') {
      fovdPopFunctionName ();
      return (-1);
    }
    i = 4;
    if (uVerLen < i) {
      i = uVerLen;
      rc = 1;
    }
    strncpy (szVersion, pTemp, i);
    szVersion[i-1] = '\0';

    fovdPopFunctionName ();
    return (rc);
}



/******************************************************************************
 * loiGetCustId
 *
 * DESCRIPTION:
 * retrieves the the customer ID from the TIMM-message
 *
 * PARAMETERS:
 *  char *szTimm         - TIMM string
 *  char *szVersion      - pointer to buffer which takes the version 
 *                         string as return
 *  int  iVerLen         - length of version buffer
 *
 * RETURNS:
 *  0 - found a version info
 * -1 - otherwise
 ******************************************************************************
 */
static int loiGetCustId (char *szTimm, char *szId, unsigned int uLen)
{
    char *pTemp;
    char *pEnd;
    int  rc;
    unsigned int  i;

    fovdPushFunctionName ("loiGetCustId");

    rc = 0;
    pTemp = szTimm;

    /*
     * search UNB (the segment with the customer id)
     */
    pTemp = (char *) strstr (szTimm, "UNB+");
    if (pTemp == NULL) {
      fovdPopFunctionName ();
      return (-1);
    }

    /*
     * now we are at the beginning of "UNB+",
     * set the pointer past this string,
     * overread two '+' and we should
     * be at the customer id string which could be
     * up to 35 characters long
     */
    pTemp += 4;
    pTemp = (char *) strchr (pTemp, '+');
    if (pTemp == NULL) {
      fovdPopFunctionName ();
      return (-1);
    }
    pTemp++;
    pTemp = (char *) strchr (pTemp, '+');
    if (pTemp == NULL) {
      fovdPopFunctionName ();
      return (-1);
    }
    pTemp++;

    /* search next '+' which marks the end of the customer id */
    pEnd = (char *) strchr (pTemp, '+');
    if (pEnd == NULL) {
      fovdPopFunctionName ();
      return (-1);
    }
    /* check length of buffer */
    i = pEnd - pTemp;
    if (i >= uLen) {
      i = uLen - 1;
      rc = 1;
    }

    /*
     * For a nice outfit, ensure the ID is at least 6 characters
     * long, if it is less it should have leading '0'
     */
    if ((i < 6) && (uLen > 6)) {
      memset (szId, '0', uLen - 1);
      strncpy (&(szId[6 - i]), pTemp, i);
      szId[6] = '\0';
    } else {
      strncpy (szId, pTemp, i);
      szId[i] = '\0';
    }

    fovdPopFunctionName ();
    return (rc);
}



/******************************************************************************
 * lovdUnlinkTestPrintTimmMsg
 *
 * DESCRIPTION:
 * unlink the TIMM-string file
 *
 * PARAMETERS:
 *  char     *pszTimm	   - pointer to TIMM-string
 *
 * RETURNS:
 * - void -
 ******************************************************************************
 */
void lovdUnlinkTestPrintTimmMsg (char *pszTimm)
{
  char	szName[PATH_MAX];
  char	szCustId[64];

  ASSERT (pszTimm != NULL);

  (void) loiGetCustId (pszTimm, szCustId, sizeof (szCustId));
  sprintf (szName, "%s/TIMM%s.etis", stBgh.szMetaDir, szCustId);
  unlink (szName);
}


/******************************************************************************
 * lovdTestPrintTimmMsg
 *
 * DESCRIPTION:
 * write the TIMM-string to file
 *
 * PARAMETERS:
 *  TYPEID   enType        - type of message
 *  char     *pszTimm	   - pointer to TIMM-string
 *
 * RETURNS:
 * - void -
 ******************************************************************************
 */
void lovdTestPrintTimmMsg (TYPEID enType, stTIMM *pstTimm, char *pszTimm)
{
  FILE	*fp;
  char	szName[PATH_MAX];
  char *str;
  char	szCustId[64];
  int	nLineLen, rc;
  char	*pLast;
  register char	*pTmp;
  char *mode;

  ASSERT (pszTimm != NULL);

  (void) loiGetCustId (pszTimm, szCustId, sizeof (szCustId));

  switch(enType) {
  case INV_TYPE: str = "BCH_INV"; mode = "w"; break;
  case SUM_TYPE: str = "BCH_SUM"; mode = "w"; break;
  case BAL_TYPE: str = "BCH_BAL"; mode = "w"; break;
  case ITB_TYPE: str = "BCH_ITB"; mode = "a"; break;
  case ROA_TYPE: str = "BCH_ROA"; mode = "w"; break;
  case LGN_TYPE: str = "BCH_LGN"; mode = "w"; break;
  case INV_DCH:  str = "DCH_INV"; mode = "w"; break;
  case ITM_DCH:  str = "DCH_ITM"; mode = "w"; break;
  case DNL_DWH:  str = "DWH_DNL"; mode = "w"; break;
  case WLL_DWH:  str = "DWH_WLL"; mode = "w"; break;
  case INV_IR:   str = "IR_INV"; mode = "w"; break;
  case INV_EC:   str = "EC_INV"; mode = "w"; break;
  case INH_INP:   str = "INH_INP"; mode = "w"; break;
  case INH_INL:   str = "INH_INL"; mode = "w"; break;
  case ENC_TYPE: str = "ENC"; mode = "w"; break;
  default: str = "XXX"; break;    
  }
  
  sprintf (szName, "%s/TIMM%s.%s.etis", stBgh.szMetaDir, szCustId, str);

  /*
   * Do some formatting, like <CR> - if missing
   */
  pTmp     = pszTimm;
  pLast    = pTmp;
  nLineLen = 0;

  while (*pTmp != '\0') {
    switch (*pTmp) {
    case ':':
      /* FALLTHROUGH */
    case '+':
      pLast = pTmp;
      break;
    case '\n':
      pLast    = pTmp;
      nLineLen = 0;
      break;
    case '\'':
      if (pTmp[1] != '\n') {
	*pTmp = '\n';
      }
      nLineLen = 0;
    }
    nLineLen++;

    /* insert an intermediate line break, if necessary */
    if (nLineLen > 78) {
      /*
       * only if there is not already a break - if so, there is no
       * place where we may break the line
       */
      if (*pLast != '\n') {
	pTmp     = pLast;
	*pTmp    = '\n';
	nLineLen = 0;
      }
    }
    pTmp++;
  }

  fp = fopen (szName, mode);

  if (fp != NULL) {
    fprintf (fp, "\n******************** Customer: %s  Type: %d (TIMM) *********************\n\n%s\n", szCustId, enType, pszTimm);
  }

  fclose (fp);
}

/******************************************************************************
 * loiGetTimmMsg
 *
 * DESCRIPTION:
 * try to get a TIMM-message for the current customer
 *
 * PARAMETERS:
 *  TYPEID   enType        - type of message
 *  stTIMM   **pstTimm     - pointer to pointer C-structure of sumsheet
 *  stDBCOL  *pstColumn	   - additional DB columns
 *
 * RETURNS:
 *  0 - read and parsed TIMM-message
 *  1 - reached the end of the database
 * -1 - otherwise
 ******************************************************************************
 */


int loiGetTimmMsg (TYPEID enType, stTIMM **pstTimm, stDBCOL *pstColumn)
{
  int     iRetVal;
  char    *pszTimmString;
  int     iTimmStrLen;
  void    *pvdFree;
  BOOL    fEnd;
  
  fovdPushFunctionName ("loiGetTimmMsg");

  fEnd          = FALSE;
  iRetVal       = 0;
  pszTimmString = NULL;
  pvdFree       = NULL;

  /*
   * read from file or use database ??
   */
  if (stBgh.bProcFile == TRUE) 
    {
      /*
       * if we have a filename and a basename, then build
       * a filename for a sumsheet and try to read it in
       */
      if (szBaseName[0] == '\0') 
        {
          macErrorMessage (PROC_NOBASENAME, WARNING,
                           "TIMM from file requested, but no basename specified;\n"
                           "BGH was probably not called with an INVOICE as file or\n"
                           "the filename was not like \"INVnnnnnnnn.etis\"!");
          fEnd = TRUE;
        }
      
      if (foiBuildFileName (enType, szBaseName, szFileName, sizeof (szFileName)) == 0) 
        {
          iRetVal = foiReadTimm (szFileName, &pszTimmString);
          pvdFree = pszTimmString;     /* because pvdFree gets freed lateron */
        } 
      else 
        {
          fEnd = TRUE;
        }
      
    } 
  else 
    { /* if (stBgh.bProcFile != TRUE)  */
      iRetVal = fnOpenSecondaryCursor(enType);
      if (iRetVal == 0) 
        {  
          iRetVal = GetTimmFromSecondaryCursor (&pszTimmString, &iTimmStrLen, &pvdFree, enType, &fEnd, pstColumn);
        }    
      
      if (iRetVal == 0) 
        {
          /* 
           * close the second cursor
           */
          fnCloseSecondaryCursor(enType);
        }
    }
  if (fEnd == TRUE) 
    {
      fovdPopFunctionName ();
      return (1);
    }
  
  if (iRetVal != 0) 
    {
#if DEBUGOUTPUT
      macErrorMessage (PROC_INVERR, WARNING, "Error reading TIMM");
#endif
    } 
  else 
    {

      /* parse the message */
      *pstTimm = parse_timm (pszTimmString);
      
      /*
       * Print the TIMM for debugging purposes
       */
      if (stBgh.bShowTimm) 
        {
          lovdTestPrintTimmMsg (enType, *pstTimm, pszTimmString);
        }
    }
  
  if (pvdFree != NULL) 
    {
      /*
       * pszTimmString is a member of sql-structure pvdFree.
       * To discard pszTimmString we must call free with pvdFree.
       */
      free (pvdFree);
    }
  
  fovdPopFunctionName ();
  return (iRetVal);
}


/******************************************************************************
 * foiGetSumsheet
 *
 * DESCRIPTION:
 * try to get a sumsheet for the current customer
 *
 * PARAMETERS:
 *  stTIMM   **pstTimm     - pointer to pointer C-structure of sumsheet
 *  stDBCOL  *pstColumn	   - additional DB columns
 *
 * RETURNS:
 *  error code from loiGetTimmMsg
 ******************************************************************************
 */
int foiGetSumsheet (stTIMM **pstTimm, stDBCOL *pstColumn)
{
  int     iRetVal;

  fovdPushFunctionName ("foiGetSumsheet");

  fovdPrintLog (LOG_TIMM, "Try to read SUMSHEET\n");

  iRetVal = loiGetTimmMsg (SUM_TYPE, pstTimm, pstColumn);

  fovdPopFunctionName ();
  return (iRetVal);
}




/******************************************************************************
 * foiGetDunnings
 *
 * DESCRIPTION:
 * try to get (all) dunnings for the current customer
 *
 * PARAMETERS:
 *  stTIMM   ***pstTimm     - pointer to pointer-array of C-structures of ITB
 *  stDBCOL  *pstColumn	    - additional DB columns
 *
 * RETURNS:
 *  0 - read and parsed TIMM-message
 *  1 - reached the end of the database
 * -1 - otherwise
 ******************************************************************************
 */
int foiGetDunnings (stTIMM ***pstTimm, int *iNrDunnings, stDBCOL *pstColumn)
{
  int     iRetVal;
  int     iRetDB;
  char    *pszTimmString;
  int     iTimmStrLen;
  void    *pvdFree;
  BOOL    fEnd;
  unsigned int uNr;
  stTIMM  *pstCurrent;
  stTIMM  *pstNew;
  stTIMM  *pstFirst = NULL;
  stTIMM  **pastInsert;
  char    szFullName[PATH_MAX];

  fovdPushFunctionName ("foiGetDunnings");

  pstCurrent = NULL;
  pstNew = NULL;
  pstFirst = NULL;

  iRetVal = fnOpenSecondaryCursor(DNL_DWH);
  if(iRetVal == 0) 
  {
      do {
          fovdPrintLog (LOG_DEBUG, "Try to read DNL message\n");
          iRetDB = GetTimmFromSecondaryCursor (&pszTimmString, &iTimmStrLen, &pvdFree, DNL_DWH, &fEnd, pstColumn);     
          if ((iRetDB != 0) || (fEnd == TRUE))
          {
              fovdPrintLog (LOG_DEBUG, "No DNL message in document_all\n");
              /*
                macErrorMessage (PROC_INVERR, WARNING, "Error reading DUNNING LETTER from document_all");
              */
              if (pstCurrent != NULL)
              {
                  pstCurrent->next = NULL;
              }
          } 
          else 
          {
              pstNew = parse_timm (pszTimmString);
              
              fovdPrintLog (LOG_DEBUG, "TIMM message parsed\n");
              
              /*
               * Print the TIMM for debugging purposes
               */
              /*
                if (stBgh.bShowTimm)
                {
                lovdTestPrintTimmMsg (DNL_DWH, **pstTimm, pszTimmString);
                }
              */
          }

          /*
           * build a chain of messages
           */

        if (pstNew != NULL) 
        {
            if (pstCurrent == NULL) 
            {
                pstFirst = pstNew;
            } 
            else 
            {
                pstCurrent->next = pstNew;
            }
            pstCurrent = pstNew;
            pstCurrent->next = NULL;
        }
        
        /*
         * release memory for DB-access
         */
        if (pvdFree != NULL) 
        {
            free (pvdFree);
        }

      } while ((iRetDB == 0) && (fEnd == FALSE));            
      
      fnCloseSecondaryCursor(DNL_DWH);
      
      fovdPrintLog (LOG_DEBUG, "All DUNNINGS loaded\n");      
  }
  else
  {
      macErrorMessage (PROC_INVERR, WARNING, "Error opening secondary cursor");
      fovdPopFunctionName ();
      return -1;
  }

  /*
   * count messages and build an array 
   */
  
  if (pstFirst == NULL) 
    {
        fovdPrintLog (LOG_DEBUG, "TIMM[DNL] messages list is empty\n");
        iRetVal = 1;
        *iNrDunnings = 0;
        *pstTimm = NULL;
    } 
  else 
  {
      fovdPrintLog (LOG_DEBUG, "TIMM[DNL] messages list is not empty\n");

      /*
       * Count items on list pstFirst with variable uNr
       */
      uNr = 0;
      pstCurrent = pstFirst;
      while (pstCurrent != NULL) 
      {
          pstCurrent = pstCurrent->next;
          uNr++;
      }

      fovdPrintLog (LOG_DEBUG, "TIMM[DNL] messages list size : %d \n", uNr);

      *iNrDunnings = uNr;

      *pstTimm = (stTIMM **) malloc (uNr * sizeof (stTIMM *));
      if (*pstTimm == NULL) 
      {
          fovdPrintLog (LOG_DEBUG, "Memory allocation error for TIMM[DNL] table : free TIMM list\n");
          
          iRetVal = -1;
          /* free the complete structures now */
          pstCurrent = pstFirst;
          while (pstCurrent != NULL) 
          {
              pstNew = pstCurrent;
              pstCurrent = pstCurrent->next;
              free_timm (pstNew);
          }
      } 
      else 
      {
          fovdPrintLog (LOG_DEBUG, "Memory allocated for TIMM table : copy TIMM list\n");
          
          /*
           * Add document from single cursor
           */
          /* 
           * walk the 'next' pointers and fill the array
           */
          pstCurrent = pstFirst;
          pastInsert = *pstTimm;          
          while (pstCurrent != NULL) 
          {
              *pastInsert++ = pstCurrent;
              pstCurrent = pstCurrent->next;
              fovdPrintLog (LOG_DEBUG, "Next TIMM[DNL] loaded\n");
          }
      }
  }
  
  fovdPopFunctionName ();
  return (iRetVal);
}


/******************************************************************************
 * foiGetItemizedBills
 *
 * DESCRIPTION:
 * try to get (all) itemized bills for the current customer
 *
 * PARAMETERS:
 *  stTIMM   ***pstTimm     - pointer to pointer-array of C-structures of ITB
 *  stDBCOL  *pstColumn	    - additional DB columns
 *
 * RETURNS:
 *  0 - read and parsed TIMM-message
 *  1 - reached the end of the database
 * -1 - otherwise
 ******************************************************************************
 */
int foiGetItemizedBills (stTIMM ***pstTimm, unsigned int *iNrBills, stDBCOL *pstColumn)
{
  int     iRetVal;
  int     iRetDB;
  char    *pszTimmString;
  int     iTimmStrLen;
  void    *pvdFree;
  BOOL    fEnd;
  unsigned int uNr;
  stTIMM  *pstCurrent;
  stTIMM  *pstNew;
  stTIMM  *pstFirst;
  stTIMM  **pastInsert;
  char    szFullName[PATH_MAX];

  fovdPushFunctionName ("foiGetItemizedBill");

  fovdPrintLog (LOG_TIMM, "Try to read ITEMIZED BILLS\n");

  iRetVal       = 0;
  fEnd          = FALSE;
  pstCurrent    = NULL;
  pstNew        = NULL;
  pstFirst      = NULL;

  if (stBgh.bProcFile == TRUE) 
    {
      /*
       * if we have a filename and a basename, then build
       * a filename for an itemized bill
       */
      if (szBaseName[0] == '\0') 
        {
        macErrorMessage (PROC_NOBASENAME, WARNING,
                         "ITEMIZED BILL from file requested, but no basename specified;\n"
                         "BGH was probably not called with an INVOICE as file or\n"
                         "the filename was not like \"INVnnnnnnnn.etis\"!");
        fEnd = TRUE;
        }

      if (foiBuildFileName (ITB_TYPE, szBaseName, szFileName, sizeof (szFileName)) != 0) 
        {
          fovdPopFunctionName ();
          return (iRetVal);
        }
      if ((iRetVal = foiSetFirst (szFileName)) != 0) 
        {
          fovdPopFunctionName ();
          return (iRetVal);
        }
    }

  iRetVal = fnOpenSecondaryCursor(ITB_TYPE);
  if(iRetVal == 0) 
    {
      do 
        {
          pstNew = NULL;
          if (stBgh.bProcFile == TRUE) 
            {
              iRetDB = foiGetNext (szFileName, szFullName, sizeof (szFullName));
              if (iRetDB == 0) 
                {
                  iRetVal = foiReadTimm (szFullName, &pszTimmString);
                  pvdFree = pszTimmString;     /* because pvdFree gets freed lateron */
                  if (iRetVal != 0) 
                    {
#if DEBUGOUTPUT
                      macErrorMessage (PROC_INVERR, WARNING, "Error reading ITEMIZED");
#endif
                      pvdFree = NULL;
                    } 
                  else 
                    { /* if (iRetVal != 0) */
                      pstNew = parse_timm (pszTimmString);
                    } /* if (iRetVal != 0) */
                } 
              else 
                { /* if (iRetDB == 0) */
                  fEnd    = TRUE;
                  pvdFree = NULL;
                } /* if (iRetDB == 0) */
            } 
          else 
            { /*if (stBgh.bProcFile == TRUE)  */
              iRetDB = GetTimmFromSecondaryCursor (&pszTimmString, &iTimmStrLen, &pvdFree, ITB_TYPE, &fEnd, pstColumn);
              if ((iRetDB != 0) || (fEnd == TRUE)) 
                {
#if DEBUGOUTPUT
                  macErrorMessage (PROC_INVERR, WARNING, "Error reading ITEMIZED BILL from document_all");
#endif
                } 
              else 
                {
                  pstNew = parse_timm (pszTimmString);
                  
                  /*
                   * Print the TIMM for debugging purposes
                   */
                  if (stBgh.bShowTimm)
                    {
                      lovdTestPrintTimmMsg (ITB_TYPE, **pstTimm, pszTimmString);
                    }
                }
            } /*if (stBgh.bProcFile == TRUE) */
          
          /*
           * build a chain of messages
           */
          if (pstNew != NULL) 
            {
              if (pstCurrent == NULL) 
                {
                  pstFirst = pstNew;
                } 
              else 
                {
                  pstCurrent->next = pstNew;
                }
              pstCurrent = pstNew;
              pstCurrent->next = NULL;
            }

          /*
           * release memory for DB-access
           */
          if (pvdFree != NULL) 
            {
              free (pvdFree);
            }
        } while ((iRetDB == 0) && (fEnd == FALSE));
    }

  fnCloseSecondaryCursor( ITB_TYPE);
 
  /*
   * count itemized bills and build an array 
   */
  if (pstFirst == NULL) 
    {
      iRetVal = 1;
      *iNrBills = 0;
      *pstTimm = NULL;
    } 
  else 
    {
      uNr = 0;
      pstCurrent = pstFirst;
      while (pstCurrent != NULL) 
        {
          pstCurrent = pstCurrent->next;
          uNr++;
        }
      *iNrBills = uNr;
      *pstTimm = (stTIMM **) malloc (uNr * sizeof (stTIMM *));
      if (*pstTimm == NULL) 
        {
          iRetVal = -1;
          /* free the complete structures now */
          pstCurrent = pstFirst;
          while (pstCurrent != NULL) 
            {
              pstNew = pstCurrent;
              pstCurrent = pstCurrent->next;
              free_timm (pstNew);
            }
        } 
      else 
        {
          /* 
           * walk the 'next' pointers and fill the array
           */
          pstCurrent = pstFirst;
          pastInsert = *pstTimm;
          while (pstCurrent != NULL) 
            {
              *pastInsert++ = pstCurrent;
              pstCurrent = pstCurrent->next;
            }
        }
    }
  fovdPopFunctionName ();
  return (iRetVal);
}

/******************************************************************************
 * foiGetRoaming
 *
 * DESCRIPTION:
 * try to get a roaming sheet for the current customer
 *
 * PARAMETERS:
 *  stTIMM   **pstTimm     - pointer to pointer to C-structure of roaming
 *  stDBCOL  *pstColumn	   - additional DB columns
 *
 * RETURNS:
 *  error code from loiGetTimmMsg
 ******************************************************************************
 */
int foiGetRoaming (stTIMM **pstTimm, stDBCOL *pstColumn)
{
  int     iRetVal;
  
  fovdPushFunctionName ("foiGetRoaming");

  fovdPrintLog (LOG_TIMM, "Try to read ROAMING\n");

  iRetVal = loiGetTimmMsg (ROA_TYPE, pstTimm, pstColumn);

  fovdPopFunctionName ();
  return (iRetVal);
}


/******************************************************************************
 * foiGetBalance
 *
 * DESCRIPTION:
 * try to get a balance sheet for the current customer
 *
 * PARAMETERS:
 *  stTIMM   **pstTimm     - pointer to pointer C-structure of balance
 *  stDBCOL  *pstColumn	   - additional DB columns
 *
 * RETURNS:
 *  error code from loiGetTimmMsg
 ******************************************************************************
 */
int foiGetBalance (stTIMM **pstTimm, stDBCOL *pstColumn)
{
  int     iRetVal;
  
  fovdPushFunctionName ("foiGetBalance");

  fovdPrintLog (LOG_TIMM, "Try to read BALANCE\n");

  iRetVal = loiGetTimmMsg (BAL_TYPE, pstTimm, pstColumn);

  fovdPopFunctionName ();
  return (iRetVal);
}

/******************************************************************************
 * foiGetLegend
 *
 * DESCRIPTION:
 * try to get a legend for the current customer
 *
 * PARAMETERS:
 *  stTIMM   **pstTimm     - pointer to pointer C-structure of legend
 *  stDBCOL  *pstColumn	   - additional DB columns
 *
 * RETURNS:
 *  error code from loiGetTimmMsg
 ******************************************************************************
 */
int foiGetLegend (stTIMM **pstTimm, stDBCOL *pstColumn)
{
  int     iRetVal;
  
  fovdPushFunctionName ("foiGetLegend");

  fovdPrintLog (LOG_TIMM, "Try to read LEGEND\n");

  iRetVal = loiGetTimmMsg (LGN_TYPE, pstTimm, pstColumn);

  fovdPopFunctionName ();
  return (iRetVal);
}


/******************************************************************************
 * foiGetDuning
 *
 * DESCRIPTION:
 * try to get a duning message for the current customer
 *
 * PARAMETERS:
 *  stTIMM   **pstTimm     - pointer to pointer C-structure of message
 *  stDBCOL  *pstColumn	   - additional DB columns
 *
 * RETURNS:
 *  error code from loiGetTimmMsg
 ******************************************************************************
 */
int foiGetDuning (stTIMM **pstTimm, stDBCOL *pstColumn)
{
  int     iRetVal;
  
  fovdPushFunctionName ("foiGetDuning");

  fovdPrintLog (LOG_TIMM, "Try to read DUNING\n");

  iRetVal = loiGetTimmMsg (DNL_DWH, pstTimm, pstColumn);

  fovdPopFunctionName ();
  return (iRetVal);
}

/******************************************************************************
 * foiGetWelcome
 *
 * DESCRIPTION:
 * try to get a welcome message for the current customer
 *
 * PARAMETERS:
 *  stTIMM   **pstTimm     - pointer to pointer C-structure of welcome
 *  stDBCOL  *pstColumn	   - additional DB columns
 *
 * RETURNS:
 *  error code from loiGetTimmMsg
 ******************************************************************************
 */
int foiGetWelcome (stTIMM **pstTimm, stDBCOL *pstColumn)
{
  int     iRetVal;
  
  fovdPushFunctionName ("foiGetWelcome");

  fovdPrintLog (LOG_TIMM, "Try to read WELCOME\n");

  iRetVal = loiGetTimmMsg (WLL_DWH, pstTimm, pstColumn);

  fovdPopFunctionName ();
  return (iRetVal);
}



/******************************************************************************
 * loiProcessCustomer
 *
 * DESCRIPTION:
 * process all TIMM messages for one customer
 *
 * PARAMETERS:
 * TYPEID 	enType		- the type of message
 *
 * RETURNS:
 *  - error code -
 ******************************************************************************
 */
int loiProcessCustomer (TYPEID enType) 
{
  int     iRetVal, iRetRollVal, iRetMarkVal;
  char    *pszTimmString;
  int     iTimmStrLen;
  void    *pvdFree;
  stTIMM  *pstTimm;
  BOOL    fEnd;
  stDBCOL stColumns;
  char    szVersion[4];
  char	  szCustId[11];
  int	  iVersion;
  char    szTemp[80];
  int	  i;

  
  fovdPushFunctionName ("loiProcessCustomer");

  /* write information about customer */
  fovdPrintLog (LOG_CUSTOMER, "\n---------------- Process Customer ----------------\n");

  pstTimm = NULL;

  /*
   * try to get INVOICE of next customer from the database
   */
  iRetVal = GetTimmFromPrimaryCursor (&pszTimmString, &iTimmStrLen, &pvdFree, enType, &fEnd, stBgh.lCustId, &stColumns);  
  if ((fEnd == TRUE) || (iRetVal != 0)) 
    {
      fovdPopFunctionName ();
      return (iRetVal);
    }
  
  fovdPrintLog (LOG_DEBUG, "Processing TIMM message\n");
  
  if (iRetVal != 0) 
    {
      macErrorMessage (PROC_INVERR, WARNING, "Error reading INVOICE");
    } 
  else 
    {
      /*
       * try to read the TIMM version from segment UNH,
       * compare this version against the versions the
       * parser understands and parse if possible
       */
      iRetVal = loiGetTimmVersion (pszTimmString, szVersion, sizeof (szVersion));
      if (iRetVal != 0) 
        {
          macErrorMessage (PROC_INVERR, WARNING, "Cannot find TIMM-version number in TIMM-message");
        } 
      else 
        {
          /* look if you find the version in the versions array */
          iVersion = atoi (szVersion);
          for (i = 0; aiParsVersion[i] != -1; i++) 
            {
              if (aiParsVersion[i] == iVersion) 
                {
                  /* parser is build for this version */
                  break;
                }
            }
          /*
           * Get the customer id from the TIMM message
           */
          szCustId[0] = '\0';
          if (loiGetCustId (pszTimmString, szCustId, sizeof (szCustId)) == 0) 
            {
              /*
               * Skip next dunning letter
               */

              if ((golLastCustId == atol(szCustId)) && (enType == DNL_DWH))
                {
                  fovdPrintLog (LOG_DEBUG, "Skipping DNL message that was already processed\n");
                  fovdPopFunctionName ();
                  return 0;
                }
	      else
		{
		  golLastCustId = atol(szCustId);
		}
            }
          
          if (aiParsVersion[i] == -1) 
            {
              sprintf (szTemp, "Customer %s: TIMM-version mismatch: %s", szCustId, szVersion);
              macErrorMessage (PROC_INVERR, WARNING, szTemp);
            }
#if DONT_PARSE_WRONG_VERSION 
          else 
            {
#endif
              /*
               * the message version is correct
               */
              pstTimm = parse_timm (pszTimmString);

#if DONT_PARSE_WRONG_VERSION 
            }
#endif
        }
      
      /*
       * Print the TIMM for debugging purposes
       */
      
      if (stBgh.bShowTimm) 
        {
          if (enType == INV_TYPE) 
            {
              /* delete any previous file of that name */
              lovdUnlinkTestPrintTimmMsg (pszTimmString);
            }

          lovdTestPrintTimmMsg (enType, pstTimm, pszTimmString);
        }
      
    }
  
  if (pvdFree != NULL) 
    {
      free (pvdFree);
    }
  
  if (pstTimm != NULL) 
    {
      /*
       * pass the structure to bill generator; 
       * bill generator will make subsequent calls
       * to get other TIMM-messages
       */
      
      iRetVal = foiGenerateBill (pstTimm, &stColumns, enType);
      
      free_timm (pstTimm);
      
      /*
       * Commit the database changes
       */

      if (iRetVal == STATUS_REJECT || iRetVal == STATUS_OK) 
        {
          if (iRetVal == STATUS_REJECT)
            {
              fovdPrintLog (LOG_DEBUG, "Marking customer TIMM messages with R\n");      
              iRetMarkVal = foiMarkCustomer ("R");
              gostBGHStat.soiInvRejected++;
            }
          else
            {          
              fovdPrintLog (LOG_DEBUG, "Marking customer TIMM messages with X\n");      
              iRetMarkVal = foiMarkCustomer ("X");
              gostBGHStat.soiInvProcessed++;
            } 

	  fovdPrintLog (LOG_DEBUG, "Commiting DB work ...\n");      
	  iRetVal = foiCommitWork ();
	  if (iRetVal != 0)
	    {	 
	      fovdPrintLog (LOG_MAX, "Can not commit work\n");
	      
	      /* 
	       * must roll back all file operations done since last save point
	       * and save point for next possible rollback event
	       */
	      
	      iRetVal = foiGen_RollBackWork();
	      if (iRetVal != 0)
		{
		  fovdPrintLog (LOG_MAX, "Can not roll back file operations\n");
		  macErrorMessage (PROC_FILE_ROLL_BACK_ERROR, CRITICAL, "Cannot roll back file operation!");          
		  iRetVal = PROC_FILE_ROLL_BACK_ERROR;
		}
	      else
		{		  
		  iRetVal = PROC_COMMIT_ERROR;
		}
	    }
	  else
	    {
	      iRetVal = foiGen_SavePoint();
	      if (iRetVal != 0)
		{
		  fovdPrintLog (LOG_MAX, "Can not save point for file operations\n");
		  macErrorMessage (PROC_FILE_SAVE_POINT_ERROR, CRITICAL, "Cannot save point for file operation!");          
		  iRetVal = PROC_FILE_SAVE_POINT_ERROR;
		}	  	  
	    }	  
        }
      else if (iRetVal == STATUS_ERROR)
        {	      
	  /* 
	   * must roll back all file operations done since last save point
	   * and save point for next possible rollback event
	   */
	  
	  iRetVal = foiGen_RollBackWork();
	  if (iRetVal != 0)
	    {
	      fovdPrintLog (LOG_MAX, "Can not roll back file operations\n");
	      macErrorMessage (PROC_FILE_ROLL_BACK_ERROR, CRITICAL, "Cannot roll back file operation!");          
	      iRetVal = PROC_FILE_ROLL_BACK_ERROR;
	    }
	  else
	    {
	      fovdPrintLog (LOG_MAX, "Trying to roll back\n");
	      macErrorMessage (PROC_STATUS_ERROR, CRITICAL, "Cannot continue processing!");
	      iRetRollVal = foiRollbackWork();
	      iRetVal = STATUS_ERROR;
	    }
        }      
      else
        {
	  macErrorMessage (PROC_STATUS_ERROR, CRITICAL, "Cannot continue processing!");          
          ASSERT(FALSE);          
        }      
    }
  
  /*
   * ECHO STATUS
   */
  
  if (iRetVal == STATUS_OK)
    {
      golCust++;
      fovdPrintLog (LOG_MAX, ">>>>> Customer with ID: %s <<<<<\n", szCustId);
      printf ("Customer ID: %s\n", szCustId);
    }
  else
    {
      fovdPrintLog (LOG_MAX, "#####\n##### Process stopped on customer with ID: %s\n#####\n", szCustId);
    }

  fovdPopFunctionName ();
  return (iRetVal);
}

/******************************************************************************
 * foiProcessAll
 *
 * DESCRIPTION:
 * process iNrInvoices TIMM messages
 * First open a cursor for INVOICE and process all customers, this supports
 * all payment responsible customers and the complete flat structure.
 * Then open the cursor for LEGEND, this supports all non-payment responsible
 * with at least one contract with itemized bill.
 *
 * PARAMETERS:
 * int poiBghId		ID of the actual BGH (parallel running BGHs)
 *
 * RETURNS:
 *  - error code -
 ******************************************************************************
 */
int foiProcessAll (TYPEID enDocType, long poiBghId)
{
  int i, j;
  enum ERROR_NUMBERS enRet;
  char	szTemp[64];
  toenBool loenStatus;
  
  fovdPushFunctionName ("foiProcessAll");

  fovdTimer_Init();

  enRet = NO_ERROR;

  if (stBgh.lCustId == DEFNOCUSTID) 
    {
      /* open cursor for multiple entries */
      enRet = (enum ERROR_NUMBERS) fnOpenPrimaryCursor (enDocType, MULTIPLE, poiBghId);
    } 
  else 
    {
      /* open cursor for single entry */
      enRet = (enum ERROR_NUMBERS) fnOpenPrimaryCursor (enDocType, SINGLE, poiBghId);
    }
  
  if (enRet != NO_ERROR) 
    {
      macErrorMessage (PROC_DBOPENERR, CRITICAL, "Cannot open database cursor!");
    }
  
  /* 
   * initialize counters 
   */

  lNrTIMM = 0;
  i = 0;		            
  golCust = 0;
  
  /*
   * Initialize document generator
   */
  
  if ((loenStatus = foenGen_Init()) == FALSE)
    {
      macErrorMessage (PROC_DBOPENERR, CRITICAL, "Cannot init document generator");
    }
  
  fovdPrintLog (LOG_MAX, " -- Read customers with invoices\n");

  /*
   * process first customer seperately to check whether there is anything
   * at all in document_all
   */

  enRet = (enum ERROR_NUMBERS) loiProcessCustomer (enDocType);
  switch (enRet) 
    {
    case ESQL_NOMOREDATA:                 /* nothing in database */
      fovdPrintLog (LOG_CUSTOMER, "No customer with invoice to process!");
      enRet = NO_ERROR;
      break;

    case NO_ERROR:                        /* ok, go on processing */
      i++;
      while ((golCust < stBgh.iNrCust) || (stBgh.bAllCust == TRUE)) 
        {
          if ((enRet = (enum ERROR_NUMBERS) loiProcessCustomer (enDocType)) != NO_ERROR) 
            {
              break;
            }

          i++;
        }
      break;

    default:                              /* some other error, already printed */
      break;
    }

  if (stBgh.bProcFile == FALSE) 
    {
      if (stBgh.lCustId == DEFNOCUSTID) 
        {
          /* close cursor for multiple entries */
          fnClosePrimaryCursor(enDocType, MULTIPLE);
        } 
      else 
        {
          /* close cursor for single entry */
          fnClosePrimaryCursor(enDocType, SINGLE);
        }
    }

  if (enRet == NO_ERROR || enRet == ESQL_NOMOREDATA)
    {      
      fovdTimer_Show(golCust);      
      if (enRet == ESQL_NOMOREDATA)
	{
	  enRet = NO_ERROR;
	}
    }
  else
    { 
      fovdPrintLog (LOG_CUSTOMER, "Can not process all documents");       
    }

#if CLEANUP
  /* free the memory from the layout lists */
  fovdFreeFormMem ();
  
  /* free the memory from the fixstrings */
  fovdFreeFixstringMem ();
  
#ifdef BSCS4
  /* free the memory from the additional BSCS4 databases */
  if (stBgh.fDbBscs4 == TRUE)
    {
      if (pstImgTypes != NULL) 
        {
          free (pstImgTypes);
          pstImgTypes = NULL;
        }

      if (pstDocTypes != NULL) 
        {
          free (pstDocTypes);
          pstDocTypes = NULL;
        }

      if (pstImgTypLevel != NULL) 
        {
          free (pstImgTypLevel);
          pstImgTypLevel = NULL;
        }

      if (pstCustImg != NULL) 
        {
          free (pstCustImg);
          pstCustImg = NULL;
        }
    }
#endif
#endif

  fovdPopFunctionName ();

  return ((int) enRet);
}
