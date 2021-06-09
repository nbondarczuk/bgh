/*******************************************************************************
 * LH-Specification GmbH 1996.
 *
 * All rights reserved.
 * Copying of this software or parts of this software is a violation of German
 * and International laws and will be prosecuted.
 *
 * Project  :   BGH
 *
 * File     :   bgh_main.c
 * Created  :   Feb. 1996
 * Author(s):   B. Michler LHS
 *              N. Bondarczuk EDS PTK Version
 *
 *
@(#) ABSTRACT : C-program
@(#) BSCS V4.0, BGH module
 *
 *******************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

/* User includes                */
#include "bgh.h"
#include "protos.h"
#include "types.h"

/* User defines                 */
#define CHECKVERS       1


/* Function macros              */


/* Enumerations                 */


/* Local typedefs               */


/* External globals             */


/* Globals                      */
stBGHGLOB	stBgh;			/* structure with globals for BGH */
tostBGHStat gostBGHStat;  

stVPLMN *pstVPLMN;
stDest *pstDest;
stTariffZone *pstTariffZone;
stTariffTime *pstTariffTime;
stServiceName *pstServiceName;
stTariffModel *pstTariffModel;
stServicePackage *pstServicePackage;
stPN *pstPN;

tostPriceGroup *gpstPriceGroup;
long goilPriceGroupSize;

int goilPaymentTypeSize;
tostPaymentType *gpstPaymentType;

long glPNCount;
long glSPCount;
long glTMCount;
long glSNCount;
long glTZCount, glTTCount;
long lVPLMNCount;

/* SCCS information */
char *SCCS_VERSION = "4.0.1.21.0";

char    *SCCS_WHAT          = LEVEL_ID;

char    szStartUp[80];

char    *pszProgName;			/* pointer to program name (argv[0])    */

/* Static globals               */
struct doctypes {
  char	*szName;		/* name of document type*/
  TYPEID enType;		/* type of document 	*/
} stDocType[] = {
  {"INV",	INV_TYPE},
  {"ITB",	ITB_TYPE},
  {"BAL",	BAL_TYPE},
  {"SUM",	SUM_TYPE},
  {"ROA",	ROA_TYPE},
  {"LGN",	LGN_TYPE},
  {"INV_DCH",	INV_DCH},
  {"ITM_DCH",	ITM_DCH},
  {"DNL",	DNL_DWH},
  {"WLL",	WLL_DWH},
  {"INV_IR", INV_IR},
  {"INV_EC", INV_EC},
  {"INH_INP", INH_INP},
  {"INH_INL", INH_INL}
};



/* Static function prototypes   */


/* External function prototypes */
extern void PrintVerInfoBghAccess (void);
extern void PrintVerInfoBghFile (void);
extern void PrintVerInfoBghErrh (void);
extern void PrintVerInfoBghProc (void);
extern void PrintVerInfoBghEsql (void);
extern void PrintVerInfoBghPYacc(void);
extern void PrintVerInfoBghPLex (void);
extern void PrintVerInfoGenBill (void);

/******************************************************************************
 * PrintHelp
 *
 * DESCRIPTION:
 * print a help screen
 *
 * PARAMETERS:
 *  void
 *
 * RETURNS:
 *  - none -
 ******************************************************************************
 */

static void PrintHelp (void)
{
    printf ("Usage:   %s [options]\n\n", pszProgName);
    printf ("-i            print version numbers\n");
    printf ("-h            print this screen\n");
    printf ("-l [level]    set loglevel level; %d <= level <= %d\n", (int) LOG_DEBUG, (int) LOG_MAX - 1);
    printf ("-a            process all customers from \"document_all\"\n");
    printf ("-b [dir]      base directory\n");
    printf ("-s            show (print) parsed TIMM structure\n");
    printf ("-c [number]   process number of customers\n");
    printf ("-p [cust. id] process specific customer\n");
    printf ("-v            view contents of database\n");
    printf ("-x            reset \"processed_by_bgh\" in \"document_all\"\n");
    printf ("-m [number]   number of parallel BGHs to run \n");
    printf ("-n [number]   number of documents in one output file\n");
    printf ("-w            write image file to BILL_IMAGES\n");
    printf ("-r [level]    report Loyal info, level = 0, 1\n");
    printf ("-d [doc]      generate documents of type INV, WLL, DNL, INP, INL\n");  
    printf ("-C            insert comission for dealers into table EDS_BGH_REP\n");
    printf ("-D            create Dunning Enclosure for INV document\n");
    printf ("-W            create Welcome Enclosure for INV document\n");
    printf ("-I            create Interest Note Periodic for INV document\n");
    printf ("-L            create Interest Note Last for INV document\n");
    printf ("-O            show log in the stderr\n");
    printf ("-V            do not buffer log output\n");
#ifdef _COMPRESS_IMAGE_
    printf ("-z            compress record inserted to BILL_IMAGES\n");
#endif

    exit(0);
}

/******************************************************************************
 * iCheckVersion
 *
 * DESCRIPTION:
 * check the version number against the version number in APP_MODULES
 *
 * PARAMETERS:
 *  char **pszVersion      - pointer for version string
 *
 * RETURNS:
 *  0   - version o.k.
 ******************************************************************************
 */
static int iCheckVersion (void)
{
  int  iRet;
  char *pszVersion;
  char szTemp[80];

  fovdPushFunctionName ("iCheckVersion");

  /* read the BGH-Version stored in APP_MODULES */
  pszVersion = NULL;
  iRet = AccessDatabaseAndReadVersion (&pszVersion);

  if (iRet == 0) {

    if (strcmp (pszVersion, SCCS_VERSION) != 0) {
      sprintf (szTemp, 
               "BGH version differs from the version in APP_MODULES\n"
               "Should be: %s\n"
               "Is       : %s",
               pszVersion, SCCS_VERSION);
      macErrorMessage (MAIN_VERERROR, WARNING, szTemp);
      iRet = (int) MAIN_VERERROR;
    }
  } else if (iRet == (int) ACC_NOVERSIONCHECK) {
    iRet = 0;
  }

  if (pszVersion != NULL) {
    free (pszVersion);
  }

  fovdPopFunctionName ();
  return (iRet); 
}

/******************************************************************************
 * PrintVersInfoBghMain
 *
 * DESCRIPTION:
 * print the version info of all BGH_MAIN
 *
 * PARAMETERS:
 *  void
 *
 * RETURNS:
 *  - none -
 ******************************************************************************
 */
void PrintVerInfoBghMain (void)
{
  static char *SCCS_ID = "1.34.1.3";

  printf ("%s\t\t\t%s\n", __FILE__, SCCS_ID);
}

/******************************************************************************
 * PrintVersionInfo
 *
 * DESCRIPTION:
 * print the version info of all BGH parts
 *
 * PARAMETERS:
 *  void
 *
 * RETURNS:
 *  - none -
 ******************************************************************************
 */

static void PrintVersionInfo (void)
{
  fovdPushFunctionName ("PrintVersionInfo");

  printf("\n%s\n\n", LEVEL_ID);

  printf ("BGH Version    : %s\n", BGH_VER);
  printf ("Layout Version : %s\n", GEN_VER);
  printf ("Build date     : %s, %s\n", __DATE__, __TIME__);
  
  printf ("\nUSED OPTIONS:\n");

#ifdef _INVOICE_V3_
  printf ("\t_INVOICE_V3_\n");
#endif

#ifdef _INVOICE_V4_
  printf ("\t_INVOICE_V4_\n");
#endif

#ifdef _INVOICE_V5_
  printf ("\t_INVOICE_V5_\n");
#endif

#ifdef _DUNSIM_ON_LEVEL_3_
  printf ("\t_DUNSIM_ON_LEVEL_3_\n");
#else
  printf ("\t_DUNSIM_ON_ALL_LEVELS_\n");
#endif

#ifdef _MAILING_RULES_V2_
  printf ("\t_MAILING_RULES_V2_\n");
#endif

#ifdef _MAILING_RULES_V1_
  printf ("\t_MAILING_RULES_V1_\n");
#endif

#ifdef _MAILING_RULES_V0_
  printf ("\t_MAILING_RULES_V0_\n");
#endif

#ifdef _ACCESS_INTERVAL_
  printf ("\t_ACCESS_INTERVAL_\n");
#endif

#ifdef _UNIT_PRICES_
  printf("\t_UNIT_PRICES_\n");
#endif

#ifdef _NO_UNIT_PRICES_
  printf("\t_NO_UNIT_PRICES_\n");
#endif

#ifdef _COMPRESS_IMAGE_
  printf("\t_COMPRESS_IMAGE_\n");
#endif

#ifdef _CLEAN_IMAGE_
  printf("\t_CLEAN_IMAGE_\n");
#else
  printf("\t_NO_CLEAN_IMAGE_\n");
#endif

#ifdef _LONG_DES_
  printf("\t_LONG_DES_\n");
#else
  printf("\t_SHORT_DES_\n");
#endif

#ifdef _DEBUG_
  printf ("\t_DEBUG_\n");
#endif  

#ifdef TEST
  printf ("\t_TEST_\n");
#endif  

#ifdef _USE_BCH_PAYMENT_
  printf ("\t_USE_BCH_PAYMENT_\n");  
#endif

#ifdef _ENC_USE_BCH_CAT_
  printf ("\t_ENC_USE_BCH_CAT_\n");  
#endif

#ifdef _MACHINE_ROUNDING_
  printf ("\t_MACHINE_ROUNDING_\n");  
#endif

#ifdef _STAT_
  printf ("\t_STAT_\n");  
#endif

#ifdef _COMPRESS_IMAGE_
  printf ("\t_COMPRESS_IMAGE_\n");  
#endif

#ifdef US_TIMM_LAYOUT
  printf ("\tUS_TIMM_LAYOUT\n");  
#endif
  
  fovdPopFunctionName ();
}


/******************************************************************************
 * fnStartParallelProcesses
 *
 * DESCRIPTION:
 * start several BGHs to speed up the procession of bills
 *
 * PARAMETERS:
 * TYPEID enDocType 		- Type Id
 * long polNumberOfProcesses	- number of Processes to run
 *
 * RETURNS:
 *  error code
 ******************************************************************************
 */
int fnStartParallelProcesses(TYPEID enDocType, long polNumberOfProcesses)
{
  long i,k;
  int loiRet;
  int status;
  pid_t pid;
  int rc;
  char    szTemp[81];
  char    szProcessNr[81];
  int foiReopenLog (void);

  rc = 0;
  
  
  if(rc == 0) 
    {
      /* 
       * flush the buffers, the created processes will inherit the buffers and if flushed later
       * they are printed several times in the files 
       */
      fovdPrintLog (LOG_MAX,"Starting %d processes\n", polNumberOfProcesses);
      fflush(NULL);

      for(i=0; i < polNumberOfProcesses; ++i)
        {
          pid = fork();
          /* If it's the new created process */
          if (pid == 0) 
            {
              if(rc == 0) 
                {
                  /* open a new logfile */
                  sprintf(szProcessNr,"%ld",i+1);
                  foiOpenLog (szProcessNr);
                }
              /* connect to the database */
              if(rc == 0) 
                {        
                  rc = InitAndConnectDatabase();        
                }
              
              if(rc == 0)
                {
                  /* process all customers assigned to this process */
                  rc = foiProcessAll (enDocType,i);
                }

              if(rc == 0)
                {
                  rc = CloseDatabase ();
                }
              fflush(NULL);
              fovdCloseLog ();

              /* end the child - BGH */
              exit(0);
              
            } 
          else if (pid < 0) 
            {
              macErrorMessage (MAIN_FORKERR,CRITICAL , "Can not create process\n");
            }
          else
            { /* else if (pid < 0) */
              /* do nothing */ 
            } /* else else if (pid < 0) */
        }
    
      /* wait until the processes finished */
      fovdPrintLog (LOG_MAX,"Wait until the processes finished\n");  
      
      for(i=0;i<polNumberOfProcesses;++i) 
        {
          wait(NULL);
        }
      fovdPrintLog (LOG_MAX, "All processes ended\n");
    }
  return(rc);
}



/******************************************************************************
 * main - the main program
 *
 * DESCRIPTION:
 * the main program of the BGH
 *
 * PARAMETERS:
 *  int     argc        - standard parameter
 *  char    *argv[]     - standard parameter
 *
 * RETURNS:
 *  error code
 ******************************************************************************
 */
int main (int _argc, char *_argv[]) {
  int     iRetCode;
  int	    iRC;		    		/* return code for get directories 	*/
  int     i, j;
  BOOL    fProcess;               		/* actually do the processing 		*/
  BOOL    fResetXDb;              		/* reset 'processed_by_bgh' in database */
  BOOL    fViewDb;                		/* view database contents 		*/
  char    szTemp[81];
  TYPEID  enDocType;		    		/* document type to process 		*/
  long    lolNumberOfParallelProcesses; 	/* Number of parallel processes */
  char    *lpszXGFViewer;
  char    *GetArgsFromDatabase();
  char    *db_argv[64];
  int     argc;
  char    **argv;
  char    *lpchzArgs, *lpchzToken;
  int     loiTokenNo;
  char    lasnzStr[32];

    /*
     * initialise modules (THIS MUST BE FIRST)
     */

  fovdInitBGHErrh ();
  (void) foiOpenLog ("");
  
  lolNumberOfParallelProcesses = NO_PARALLEL_PROC;
  
  fovdPushFunctionName ("main");
  
  /*
   * startup message
   */

  sprintf (szStartUp, "BGH (build: %s, %s)\n", __DATE__, __TIME__);

  /*
   * initialise variables / defaults
   */
  fProcess            = TRUE;
  fResetXDb           = FALSE;
  fViewDb             = FALSE;
  iRetCode	      = (int) NO_ERROR;
  enDocType	      = INV_TYPE;

  /* init stat */
  gostBGHStat.soiInvProcessed = 0;
  gostBGHStat.soiInvRejected = 0;

  /* preset globals with defaults */
  memset (&stBgh, 0, sizeof (stBgh));
  stBgh.lCustSetSize    = 0;
  stBgh.lCustNo         = 1;
  stBgh.lSetNo          = 1;
  stBgh.bInsertHeader   = TRUE; 
  stBgh.enTimmProcessing= INV_TYPE;
  stBgh.enLogLevel    	= LOG_NORMAL;
  stBgh.bShowTimm     	= FALSE;
  stBgh.bProcFile		   = FALSE;
  stBgh.bAllCust		   = FALSE;
  stBgh.iNrCust       	= DEFNRINVOICES;
  stBgh.lCustId       	= DEFNOCUSTID;
  stBgh.pszFileName   	= NULL;
  stBgh.pszOutFile    	= NULL;
  stBgh.szBaseDirectory[0] 	= '\0';
  stBgh.szLogDir[0] 		= '\0';
  stBgh.szLayoutDir[0]  = '\0';
  stBgh.bEnclosureGen   = FALSE;
  stBgh.enOutputType    = XGF;
  stBgh.bForceEnglish   = TRUE;
  stBgh.bWriteBillImage = FALSE;
  stBgh.bDebugMode      = FALSE;
  stBgh.bAccEncGen      = FALSE; 
  stBgh.bSimEncGen      = FALSE;
  stBgh.bWelcomeGen     = FALSE;
  stBgh.bDunningGen     = FALSE;
  stBgh.bSlipGen        = FALSE;
  stBgh.bSuppressEmptyList = FALSE;
  stBgh.bUseThreshold = FALSE;
  stBgh.bShowZeroSum = TRUE;
  stBgh.bINPGen = FALSE;
  stBgh.bINLGen = FALSE;
  stBgh.bInsertComission = FALSE;
  stBgh.szBaseDirectory[0] = '\0';
  stBgh.soiSettlingPeriod = -1;
  stBgh.bCompressImage = FALSE;
  stBgh.soenLogToScreen = FALSE;
  stBgh.sochLoyalReportLevel = '\0';

#ifdef BSCS4
  /*
   * as default BSCS4 Database flag is false. It is set to TRUE
   * in bgh_esql.pc when 'GetDocTables' is able to read the
   * new tables of BSCS4
   */
  stBgh.fDbBscs4		= FALSE;
#endif

  pszProgName         = _argv[0];
  
  if (_argc == 1)
    {
      iRetCode = InitAndConnectDatabase();
      
      /*
       *
       */
      
      if (iRetCode == (int) NO_ERROR)
	{
	  /*
	   * Load argument string from MPSCFTAB
	   */
	  
	  if (_argc == 1)
	    {
	      fovdPrintLog (LOG_MAX, "Loading parameteres from DB\n");
	      lpchzArgs = GetArgsFromDatabase(); 
	      fovdPrintLog (LOG_MAX, "Parameteres from DB: %s\n", lpchzArgs);
	      if (strlen(lpchzArgs) > 0)
		{
		  db_argv[0] = _argv[0];
		  fovdPrintLog (LOG_MAX, "Parameter[%d]: %s\n", 0, _argv[0]);
		  
		  loiTokenNo = 1;
		  if ((lpchzToken = strtok(lpchzArgs, " ")) != NULL)
		    {
		      fovdPrintLog (LOG_MAX, "Parameter[%d]: %s\n", loiTokenNo, lpchzToken);
		      db_argv[loiTokenNo++] = lpchzToken;
		      while ((lpchzToken = strtok((char *)NULL, " ")) != NULL && loiTokenNo < 62)
			{
			  fovdPrintLog (LOG_MAX, "Parameter[%d]: %s\n", loiTokenNo, lpchzToken);
			  db_argv[loiTokenNo++] = lpchzToken;
			}
		    }
		  
		  argc = loiTokenNo;
		  argv = db_argv;
		}
	      else
		{
		  argc = _argc;
		  argv = _argv;
		}      
	    }
	  else
	    {
	      argc = _argc;
	      argv = _argv;
	    }
	}

      iRetCode = CloseDatabase ();
    }
  else
    {
      argc = _argc;
      argv = _argv;
    }

  /*
   * write the command line to log-file
   */
  
  fovdPrintLog (LOG_MAX, "Command line: ");
  for (i = 0; i < argc; i++) 
    {
      fovdPrintLog (LOG_MAX, "%s ", argv[i]);
    }

  fovdPrintLog (LOG_MAX, "\n\n");


  /*
   * scan through parameters
   */
  for (i = 1; (i < argc) && (iRetCode == (int)NO_ERROR); i++) 
    {
      if (argv[i][0] != '-') 
        {
          sprintf (szTemp, "Parameter must start with '-' [\"%s\"]\n", argv[i]);
          macErrorMessage (MAIN_PARAMERR, WARNING, szTemp);
          iRetCode = (int) MAIN_PARAMERR;
          continue;
        }

      if (argv[i][2] != '\0') 
        {
          sprintf (szTemp, "Parameter \"%s\" too long\n", argv[i]);
          macErrorMessage (MAIN_PARAMERR, WARNING, szTemp);
          iRetCode = (int) MAIN_PARAMERR;
          continue;
        }

      switch (argv[i][1]) 
        {
        case 'a':			/* process all customers from documen_all */
          stBgh.bAllCust = TRUE;
          break;
          
        case 'b':                 /* set base directory */
          if (i < (argc - 1)) 
            {
              if (argv[i+1][0] == '-') 
                {
                  /*
                   * next parameter is a new argument
                   */
                  sprintf (szTemp, "a directory is needed for -%c\n", argv[i][1]);
                  macErrorMessage (MAIN_NOFNAME, WARNING, szTemp);
                  iRetCode = (int) MAIN_NOFNAME;
                } 
              else 
                {
                  i++;        /* proceed to next argument */
                  strncpy (stBgh.szBaseDirectory, argv[i], sizeof (stBgh.szBaseDirectory));
                  stBgh.szBaseDirectory[sizeof (stBgh.szBaseDirectory) - 1] = '\0';
                }
            } 
          else 
            {
              sprintf (szTemp, "a directory is needed for -%c\n", argv[i][1]);
              macErrorMessage (MAIN_NOFNAME, WARNING, szTemp);
              iRetCode = (int) MAIN_NOFNAME;
            }
          break;

        case 'c':                 /* set number of customers to process */
          if (i < (argc - 1)) 
            {
              if (argv[i+1][0] == '-') 
                {
                  /*
                   * next parameter is a new argument
                   */
                } 
              else 
                {
                  i++;        /* proceed to next argument */
                  stBgh.iNrCust = atoi (argv[i]);
                }
            }
          break;

        case 'd':           /* create this message */ 
          if (i < (argc - 1)) 
            {
              if (argv[i+1][0] == '-') 
                {
                  /*
                   * next parameter is a new argument
                   */
                  sprintf (szTemp, "a document type (INV, ITB, DNL, WLL) is needed for -%c\n", argv[i][1]);
                  macErrorMessage (MAIN_NOFNAME, WARNING, szTemp);
                  iRetCode = (int) MAIN_NOFNAME;
                } 
              else 
                {
                  i++;        /* proceed to next argument */
                  if (!strcmp(argv[i], "INV"))
                    {
                      enDocType = INV_TYPE;
                    }
                  else if (!strcmp(argv[i], "DNL"))
                    {
                      enDocType = DNL_DWH;
                    }
                  else if (!strcmp(argv[i], "WLL"))
                    {
                      enDocType = WLL_DWH;
                    }
                  else if (!strcmp(argv[i], "INP"))
                    {
                      enDocType = INH_INP;
                    }
                  else if (!strcmp(argv[i], "INL"))
                    {
                      enDocType = INH_INL;
                    }
                  else
                    {
                      sprintf (szTemp, "Bad document type given in option -%c\n", argv[i][1]);
                      macErrorMessage (MAIN_NOFNAME, WARNING, szTemp);
                      iRetCode = (int) MAIN_NOFNAME;
                    }
                }
            } 
          else 
            {
              sprintf (szTemp, "a document type (INV, ITB, DNL, WLL, INP, INL) is needed for -%c\n", argv[i][1]);
              macErrorMessage (MAIN_NOFNAME, WARNING, szTemp);
              iRetCode = (int) MAIN_NOFNAME;
            }

          stBgh.enTimmProcessing = enDocType;
          break;

        case 'f':                 /* read from file */
          if (i < (argc - 1)) 
            {
              if (argv[i+1][0] == '-') 
                {
                  /*
                   * next parameter is a new argument
                   */
                  sprintf (szTemp, "a filename is needed for -%c\n", argv[i][1]);
                  macErrorMessage (MAIN_NOFNAME, WARNING, szTemp);
                  iRetCode = (int) MAIN_NOFNAME;
                } 
              else 
                {
                  i++;        /* proceed to next argument */
                  stBgh.pszFileName = argv[i];
                  stBgh.bProcFile = TRUE;
                }
            } 
          else 
            {
              sprintf (szTemp, "a filename is needed for -%c\n", argv[i][1]);
              macErrorMessage (MAIN_NOFNAME, WARNING, szTemp);
              iRetCode = (int) MAIN_NOFNAME;
            }
          break;

        case 'g':
          stBgh.enOutputType = XGF;
          break;

        case 'i':                 /* string info feature */
          PrintVersionInfo ();          
	  fovdRemoveLog ();
	  
	  fovdPopFunctionName ();
	  return (iRetCode);

        case 'l':                 /* set log level */
          if (i < (argc - 1)) 
            {
              if (argv[i+1][0] == '-') 
                {
                  /*
                   * next parameter is a new argument
                   */
                } 
              else 
                {
                  i++;        /* proceed to next argument */
                  stBgh.enLogLevel = (LOGLEVEL) atoi (argv[i]);
                }
              if ((stBgh.enLogLevel >= LOG_MAX) ||  (stBgh.enLogLevel < LOG_DEBUG)) 
                {
                  sprintf (szTemp, "Loglevel must be between %d and %d, is set to %d!\n", LOG_MAX, LOG_DEBUG, LOG_NORMAL);
                  macErrorMessage (MAIN_LOGLEVERR, WARNING, szTemp);
                  stBgh.enLogLevel = LOG_NORMAL;
                }
            }
          break;
	  
        case 'm':                 /* number of parallel processes */
          if (i < (argc - 1)) 
            {
              if (argv[i+1][0] == '-') 
                {
                  /*
                   * next parameter is a new argument
                   */
                } 
              else 
                {
                  i++;        /* proceed to next argument */
                  lolNumberOfParallelProcesses = atol (argv[i]);
                }
              if ((lolNumberOfParallelProcesses > NR_OF_PROC_MAX) ||  (lolNumberOfParallelProcesses < NR_OF_PROC_MIN )) 
                {
                  sprintf (szTemp, "Number of parallel processes must be between %d and %d, is set to %d!\n", NR_OF_PROC_MAX, NR_OF_PROC_MIN,NO_PARALLEL_PROC);
                  macErrorMessage (MAIN_NR_OF_PROC_ERR, WARNING, szTemp);
                  lolNumberOfParallelProcesses = NO_PARALLEL_PROC;
                }
            }
          break;

        case 'n':
          if (i < (argc - 1)) 
            {
              if (argv[i+1][0] == '-') 
                {
                  /*
                   * next parameter is a new argument
                   */
                } 
              else 
                {
                  i++;        /* proceed to next argument */
                  stBgh.lCustSetSize = atol (argv[i]);
                }
            }
          break;

        case 'h':                 /* several options for help... */
          /* FALLTHROUGH */
        case 'H':
          /* FALLTHROUGH */
        case '?':
          PrintHelp ();
          fProcess = FALSE;
          break;

        case 'o':                 /* write to file */
          if (i < (argc - 1)) 
            {
              if (argv[i+1][0] == '-') {
                /*
                 * next parameter is a new argument
                 */
                sprintf (szTemp, "a filename is needed for -%c\n", argv[i][1]);
                macErrorMessage (MAIN_NOFNAME, WARNING, szTemp);
                iRetCode = (int) MAIN_NOFNAME;
              } 
              else 
                {
                  i++;        /* proceed to next argument */
                  stBgh.pszOutFile = argv[i];
                }
            } 
          else 
            {
              sprintf (szTemp, "a filename is needed for -%c\n", argv[i][1]);
              macErrorMessage (MAIN_NOFNAME, WARNING, szTemp);
              iRetCode = (int) MAIN_NOFNAME;
            }
          break;
          
        case 'p':                 /* process given customer */
          if (i < (argc - 1)) 
            {
              if (argv[i+1][0] == '-') 
                {
                  /*
                   * next parameter is a new argument
                   */
                } 
              else 
                {
                  i++;        /* proceed to next argument */
                  stBgh.lCustId = atol (argv[i]);
                }
            }
          break;

        case 'r':
          if (i < (argc - 1)) 
            {
              if (argv[i+1][0] == '-') 
                {
                  /*
                   * next parameter is a new argument
                   */
                } 
              else 
                {
                  i++;        /* proceed to next argument */
                  stBgh.sochLoyalReportLevel = argv[i][0];
                }
            }
          break;

        case 's':                 /* print the TIMM-structure */
          stBgh.bShowTimm = TRUE;
          break;
          
        case 't':
          stBgh.enOutputType = TEX;
          break;

        case 'v':                 /* view database contents */
          fViewDb = TRUE;
          fProcess = FALSE;
          break;
          
        case 'w':
          stBgh.bWriteBillImage = TRUE;
          break;
          
        case 'x':                 /* reset 'processed_by_bgh' */
          fResetXDb = TRUE;
          fProcess = FALSE;
          break;

        case 'V':
          stBgh.bDebugMode      = TRUE;
          break;
	  
        case 'A':
          stBgh.bAccEncGen      = TRUE;
          break;
          
        case 'C':
          stBgh.bInsertComission = TRUE;
          break;

        case 'S':
          stBgh.bSimEncGen      = TRUE;
          break;

        case 'W':
          stBgh.bWelcomeGen     = TRUE;
          break;

        case 'D':
          stBgh.bDunningGen     = TRUE;
          break;

        case 'P':
          stBgh.bSlipGen        = TRUE;
          break;

        case 'I':
          stBgh.bINPGen = TRUE;
          break;

        case 'L':
          stBgh.bINLGen = TRUE;
          break;

        case 'T':
          stBgh.bUseThreshold = TRUE;
          break;

        case 'Z':
          stBgh.bShowZeroSum = FALSE;
          stBgh.bSuppressEmptyList = TRUE;
          break;

        case 'z':
          stBgh.bCompressImage = TRUE;
          break;

        case 'O':
          stBgh.soenLogToScreen = TRUE;
          break;

        default:
          sprintf (szTemp, "unknown option \"%s\" (try \"-h\" for help) !\n", argv[i]);
          macErrorMessage (MAIN_UNKNOWNOPT, WARNING, szTemp);
          iRetCode = (int) MAIN_UNKNOWNOPT;
          break;
        }
    }

  
  iRetCode = InitAndConnectDatabase();
  
#ifdef BSCS4
  fovdGetAddDB ();		/* get additional info from db */
#endif
  
  /*
   * get the basedirectory
   */
  if (iRetCode == (int) NO_ERROR)
    {
      iRC = foiGetDirectories ();
      if (iRC != (int) NO_ERROR) 
        {
          fovdPopFunctionName ();
          fovdCloseLog ();
          return (iRC);
        }
    }

  if (iRetCode == (int) NO_ERROR) 
    {
      /*
       * check the version number against database APP_MODULES
       * if something important is to be done
       */
      if (fViewDb || fResetXDb || fProcess) {
#if CHECKVERS
        iRetCode = iCheckVersion ();
        
        if (iRetCode != 0) 
          {
#if 0
            fovdPopFunctionName ();
            foiCloseLog ();
            return (iRetCode);
#endif
          }
#endif
      }
    }

  
  if (iRetCode == (int) NO_ERROR) 
    {
      /*
       * ignore the count for multiple customers
       * if a certain customer is to be processed
       */
      if (stBgh.lCustId != DEFNOCUSTID) 
        {
          stBgh.iNrCust = 1;
        }
    
      /*
       * now do the processing
       */
      if (fViewDb == TRUE) 
        {
          /*
           * view contents of document_all
           */
          iRetCode = AccessDatabaseAndTest ();
        }

      if (fResetXDb == TRUE) 
        {
          /*
           * clear all 'processed_by_bgh' in document_all
           */
      
          printf ("Resetting 'processed-by-bgh' of 'document_all' for all customer...");
          fflush (stdout);
          iRetCode = AccessDatabaseAndReset ();
        } 
      else if (fProcess == TRUE) 
        {
          /* Print the versions before processing */

          PrintVersionInfo ();

          /*
           * get bill image viewer and additional tables for BSCS4
           */
          if (stBgh.bProcFile == FALSE) 
            {
              (void) AccessDatabaseAndReadProcessProgram ();
            }


          /* If parallel running is selected and
             multiple Customer is selected and
             number of customers to process > 1 
          */
          if(
             (lolNumberOfParallelProcesses != NO_PARALLEL_PROC) && (stBgh.lCustId ==  DEFNOCUSTID) && 
             (stBgh.iNrCust ==  DEFNRINVOICES)
             ) 
            {
              if (iRetCode == (int) NO_ERROR) 
                {
                  
                  /*
                   * process data
                   */
                  if(iRetCode == 0) 
                    {
                      iRetCode = fnBalanceData(enDocType,lolNumberOfParallelProcesses);
                    }
          
                  if(iRetCode == 0) 
                    {	
                      fnStartParallelProcesses(enDocType,lolNumberOfParallelProcesses);
                    }   
                } /* if (iRetCode... */
            } 
          else 
            {
              iRetCode = foiProcessAll (enDocType, NO_PARALLEL_PROC);
              if(iRetCode == (int) NO_ERROR)
                {
                  iRetCode = CloseDatabase ();
                }
              else 
                {
                  printf("BGH (foiProcessAll) : Nonzero return value !\n");
                }
            }
        }
    } /* if (iRetCode... */

  
  if (iRetCode != (int) NO_ERROR) 
    {
      /* print additional info about nonzero return codes */
      printf ("BGH (pid: %d): Nonzero return value !\n", getpid ());
      PrintErrorInfo ((enum ERROR_NUMBERS) iRetCode);
    }
  
  fovdCloseLog ();
  fovdPopFunctionName ();
  return (iRetCode);
}










