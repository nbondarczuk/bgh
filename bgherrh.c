/******************************************************************************
 * LH-Specification GmbH 1996.
 *
 * All rights reserved.
 * Copying of this software or parts of this software is a violation of German
 * and International laws and will be prosecuted.
 *
 * Project: BSCS V 3.0 for multi markets
 *
 * File   : BGHERRH.C
 * Created: Feb. 1996
 * Author : M. Mustroph
 *****************************************************************************/

#if 0	/* just for version.sh */
static char *SCCS_VERSION = "1.19";
#endif


#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>

/* user includes */
#include "bgh.h"
#include "protos.h"


/* user defines */
#define  SIZE_CALL_STACK       100
#define  NO_CALLBACK_FUNCTIONS  32


/* --- ANSI C signals --- */
/* please make comment lines out of the following lines, if ... */
/* the signals are not defined or should not be intercepted     */
#define  CATCH_SlIGABRT_AND_EXIT  1
/* #define  CATCH_SIGFPE_AND_EXIT    1 */
/* #define  CATCH_SIGILL_AND_EXIT    1 */
#define  CATCH_SIGINT_AND_EXIT    1 
/* #define  CATCH_SIGSEGV_AND_EXIT   1 */
/* #define  CATCH_SIGTERM_AND_EXIT   1 */


/* --- global declarations --- */
static int  doiProcessId   = -1;             /* process id of the program   */
static BOOL dofFirstCall   = TRUE;           /* used for initializing local variables   */
static char *gaszCallStack[SIZE_CALL_STACK]; /* call tree stack             */
static int  goiSpCallStack = 0;              /* stack pointer for call tree */
static void (*dpavdCallbackFunctions [NO_CALLBACK_FUNCTIONS]) (void);

/* the following structure (-table) is a table with all return codes.   */
/* this table is used on exit of BGH to print a message for nonzero     */
/* return codes                                                         */
struct ERRTABLE {
  enum ERROR_NUMBERS    enErrNr;        /* error number */
  char                  *szErrText;     /* error verbose */
  char                  *szLongErr;     /* detailed error message */
};
static struct ERRTABLE stErrorTable [] = {
  {NO_ERROR,                  "NO_ERROR",                  NULL},

  {ERRH_SIGINT,               "ERRH_SIGINT",               NULL},
  {ERRH_SIGILL,               "ERRH_SIGILL",               NULL},
  {ERRH_SIGABORT,             "ERRH_SIGABORT",             NULL},
  {ERRH_SIGFPE,               "ERRH_SIGFPE",               NULL},
  {ERRH_SIGSEGV,              "ERRH_SIGSEGV",              NULL},
  {ERRH_SIGTERM,              "ERRH_SIGTERM",              NULL},

  {MAIN_NOTIMM,               "MAIN_NOTIMM",               "no TIMM message"},
  {MAIN_INVTYPE,              "MAIN_INVTYPE",              "invalid TIMM type"},
  {MAIN_PARAMERR,             "MAIN_PARAMERR",             "error in passed parameter"},
  {MAIN_LOGLEVERR,            "MAIN_LOGLEVERR",            "invalid loglevel"},
  {MAIN_NOFNAME,              "MAIN_NOFNAME",              "filename missing although option needs one"},
  {MAIN_UNKNOWNOPT,           "MAIN_UNKNOWNOPT",           "unknown option"},
  {MAIN_VERERROR,             "MAIN_VERERROR",             "wrong version of BGH"},

  {PROC_DBOPENERR,            "PROC_DBOPENERR",            "cannot open database"},
  {PROC_INVERR,               "PROC_INVERR",               "error reading invoice"},
  {PROC_TIMMVERERR,           "PROC_TIMMVERERR",           "wrong version of TIMM message"},
  {PROC_NOTIMMVER,            "PROC_NOTIMMVER",            "TIMM version not found in TIMM message"},
  {PROC_NOBASENAME,           "PROC_NOBASENAME",           "no basename for file input of TIMM"},
  {PROC_MULTITIMM,            "PROC_MULTITIMM",            "more TIMMs than expected in database"},
  {PROC_FILE_ROLL_BACK_ERROR, "PROC_FILE_ROLL_BACK_ERROR", "prolem with removal of the last document in case of commit error"},
  {PROC_FILE_SAVE_POINT_ERROR,"PROC_FILE_SAVE_POINT_ERROR","error when saving the offsets for the last document"},
  {PROC_COMMIT_ERROR,         "PROC_COMMIT_ERROR",         "error when DB commit but file roll back OK - the last document parts removed from files"},
  {PROC_STATUS_ERROR,         "PROC_STATUS_ERROR",         "status returned from proceor is not OK"},

  {PARSE_MALLOC,              "PARSE_MALLOC",              "no memory for malloc"},
  {PARSE_NOPOINTER,           "PARSE_NOPOINTER",           "pointer not set"},
  {PARSE_NOATTACH,            "PARSE_NOATTACH",            "Attach-point for segment not set"},
  {PARSE_MANDMISSING,         "PARSE_MANDMISSING",         "mandatory element missing"},
  {PARSE_ELEMTOOLONG,         "PARSE_ELEMTOOLONG",         "element is too long"},
  {PARSE_ILLEGALELEM,         "PARSE_ILLEGALELEM",         "element not allowed a this position"},
  {PARSE_TOOMUCHELEM,         "PARSE_TOOMUCHELEM",         "too much elements in a repetition"},

  {ACC_NOPASSWORD,            "ACC_NOPASSWORD",            "no database-password for BGH"},
  {ACC_NOCONNECTSTRING,       "ACC_NOCONNECTSTRING",       "no database-connect-string for BGH"},
  {ACC_SECCURSOR_ILLUSAGE,    "ACC_SECCURSOR_ILLUSAGE",    "try to close 2nd cursor with illegal usage"},
  {ACC_NOVERSIONCHECK,        "ACC_NOVERSIONCHECK",        "no version check to be done"},
  {ACC_NOPROCPROG,	      "ACC_NOPROCPROG",            "no bill image processing program"},

  {ESQL_ERROR,         "ESQL_ERROR",           "error in esql execution"},
  {ESQL_DBCONNECT,     "ESQL_DBCONNECT",       "cannnot connect database"},
  {ESQL_DBDISCONNECT,  "ESQL_DBDISCONNECT",    "cannnot disconnect database"},
  {ESQL_OPENMAINCURSOR, "ESQL_OPENMAINCURSOR", "error by open from main cursor"},
  {ESQL_OPENSECONDCURSOR, "ESQL_OPENSECONDCURSOR", "error by open from second cursor"},
  {ESQL_OPENSECMAINCLOSE, "ESQL_OPENSECMAINCLOSE", "try to open second cursor with main closed"},
  {ESQL_OPENTESTCURSOR, "ESQL_OPENTESTCURSOR", "error by open from test cursor"},
  {ESQL_CLOSEMAINCURSOR, "ESQL_CLOSEMAINCURSOR", "error by close from main cursor"},
  {ESQL_CLOSESECONDCURSOR, "ESQL_CLOSESECONDCURSOR", "error by close from second cursor"},
  {ESQL_CLOSETESTCURSOR, "ESQL_CLOSETESTCURSOR", "error by close from test cursor"},
  {ESQL_FETCHCURSOR,   "ESQL_FETCHCURSOR",     "error by fetching for main(second) cursor"},
  {ESQL_FETCHTESTCURSOR, "ESQL_FETCHTESTCURSOR", "error by fetching for test cursor"},
  {ESQL_SECCURSOR_ILLUSAGE, "ESQL_SECCURSOR_ILLUSAGE","try to manipulate second cursor with illegal usage"},
  {ESQL_ILLCUSTPROC,   "ESQL_ILLCUSTPROC",     "illegal customer-process-usage for main cursor"},
  {ESQL_SELECT,        "ESQL_SELECT",          "error in SQL-SELECT statement"},
  {ESQL_SELVERS,       "ESQL_SELVERS",         "error in SQL-SELECT statement version-number"},
  {ESQL_CHECKBATCH,    "ESQL_CHECKBATCH",      "error in SQL-SELECT statement version-check"},
  {ESQL_SELPROCPROG,   "ESQL_SELPROCPROG",     "error in SQL-SELECT statement bill-images-process-program"},
  {ESQL_NULLPROCPROG,  "ESQL_NULLPROCPROG",   	"NULL detected in database for bill-images-process-program"},
  {ESQL_SELCOUNT,      "ESQL_SELCOUNT",        "error in SQL-SELECT statement count(*) Xs"},
  {ESQL_UPDATE,        "ESQL_UPDATE",          "error in SQL-UPDATE statement"},
  {ESQL_UPDATEMOREASONE, "ESQL_UPDATEMOREASONE", "update more as a record"},
  {ESQL_RESETX,        "ESQL_RESETX",          "error in SQL-UPDATE statement reset Xs"},
  {ESQL_NOX,           "ESQL_NOX",             "nothing to reset"},
  {ESQL_COMMIT,        "ESQL_COMMIT",          "error in SQL-COMMIT statement"},
  {ESQL_COMMIT_DBCLOSED, "ESQL_COMMIT_DBCLOSED", "commit with closed database"},
  {ESQL_ROLLBACK,      "ESQL_ROLLBACK",        "error in SQL-ROLLBACK statement"},
  {ESQL_ROLLBACK_DBCLOSED, "ESQL_ROLLBACK_DBCLOSED", "rollback with closed database"},
  {ESQL_ALLOC,         "ESQL_ALLOC",           "no memory for malloc"},
  {ESQL_REALLOC,       "ESQL_REALLOC",         "no memory for realloc"},
  {ESQL_NOMOREDATA,    "ESQL_NOMOREDATA",      "no more data to read from database"},
  {ESQL_DOCSIZENULL,   "ESQL_DOCSIZENULL",     "readed document has size zero"},
  {ESQL_BILLINSERT,    "ESQL_BILLINSERT",      "insert into table bill_images"},
  {ESQL_BILL_ILLTYPE,  "ESQL_BILL_ILLTYPE",    "illegal timm-type for bill_image"},
  {ESQL_BILL_DBCLOSED, "ESQL_BILL_DBCLOSED",   "insert bill_image with closed database"},
  {ESQL_BILL_NULL,     "ESQL_BILL_NULL",       "nothing to insert, no image-information"},
  {ESQL_INV_COLUMN,	   "ESQL_INV_COLUMN",	"invalid column name in database"},
  {ESQL_SET_NLS_LANG,  "ESQL_SET_NLS_LANG",    "error setting up NLS_LANG for session"},

  {FILE_MALLOC,        "FILE_MALLOC",          "no memory for malloc"},
  {FILE_OPEN,          "FILE_OPEN",            "file open failed"},
  {FILE_CLOSE,         "FILE_CLOSE",           "file close failed"},
  {FILE_WRITE,         "FILE_WRITE",           "writing to file failed"},
  {FILE_INVALIDNAME,   "FILE_INVALIDNAME",     "name was not from invoice"},
  {FILE_PATHLENGTH,    "FILE_PATHLENGTH",      "path was too long for given buffer"},
  {FILE_NOMATCH,       "FILE_NOMATCH",         "no match for wildcard search"},
  {FILE_OPENDIR,       "FILE_OPENDIR",         "open on directory failed"},
  {FILE_NOACCESS,	     "FILE_NOACCESS",	"no access to directory"},

  {FORM_LINETOOSHORT,  "FORM_LINETOOSHORT",    "truncated line in layout-file"},
  {FORM_LISTERROR,     "FORM_LISTERROR",       "error in list"},
  {FORM_CANNOTOPEN,    "FORM_CANNOTOPEN",      "error opening layout-file"},
  {FORM_NOCLOSE,       "FORM_NOCLOSE",         "error closing layout-file"},
  {FORM_OUTMEM,        "FORM_OUTMEM",          "no memory from malloc"},
  {FORM_LINEERR,       "FORM_LINEERR",         "error in line"},

  {LAY_NULLPOINTER,    "LAY_NULLPOINTER",      "wrong NULL-pointer in paField"},
  {LAY_NOMETAHEAD,     "LAY_NOMETAHEAD",       "file 'metahead.ps' not found"},
  {LAY_INEXPCOUNT,     "LAY_INEXPCOUNT",       "count was not expected in vdInitLayout"},
  {LAY_MALLOC,         "LAY_MALLOC",           "no memory from malloc"},
  {LAY_INITLINE,       "LAY_INITLINE",         "line was not correct initialized"},
  {LAY_YPOSOFFPAGE,    "LAY_YPOSOFFPAGE",      "element should be printed off pageend"},
  {LAY_XPOSOFFPAGE,    "LAY_XPOSOFFPAGE",      "element should be printed off pageend"},
  {LAY_FROMTOOBIG,     "LAY_FROMTOOBIG",       "'from' element too big"},
  {LAY_TOFROM,         "LAY_TOFROM",           "'to' smaller than 'from'"},
  {LAY_TOTOOBIG,       "LAY_TOTOOBIG",         "'to' element too big"},
  {LAY_TOUNDEFINED,    "LAY_TOUNDEFINED",      "'to' element undefined (may not be 0)"},
  {LAY_NOBLOCKSIZE,    "LAY_NOBLOCKSIZE",      "no blocksize in put_next_line"},
  
    /* Errors from post parser (data collection unit) */
  {PREP_UNDEF_DOCTYPE,		         "PREP_UNDEF_DOCTYPE",		        NULL},
  {PREP_UNDEF_LIN_7140,		         "PREP_UNDEF_LIN_7140",		        NULL},
  {PREP_UNDEF_PIA_7140,		         "PREP_UNDEF_PIA_7140",		        NULL},
  {PREP_UNDEF_PIA_7140_PpU,		   "PREP_UNDEF_PIA_7140_PpU",	        NULL},
  {PREP_UNDEF_PIA_7140_CHARGE_TYPE,	"PREP_UNDEF_PIA_7140_CHARGE_TYPE", NULL},
  {PREP_ILL_PIA_7140,			      "PREP_ILL_PIA_7140",		           NULL},
  {PREP_NO_IMD_WITH_VPLMN_CODE,	   "PREP_NO_IMD_WITH_VPLMN_CODE",	  NULL},
  {PREP_NO_IMD_SERVICE_CODE_ID,	   "PREP_NO_IMD_SERVICE_CODE_ID",	  NULL},
  {PREP_MALLOC_FAILED,		         "PREP_MALLOC_FAILED",		        NULL},
  {PREP_ILL_MALLOC_REQUEST,	      "PREP_ILL_MALLOC_REQUEST",	        NULL},
  {PREP_IMD_NO_SERVICE_DESCR,		   "PREP_IMD_NO_SERVICE_DESCR",	     NULL},
  {PREP_PRI_NO_PRICE,			      "PREP_PRI_NO_PRICE",		           NULL},
  {PREP_MOA_NO_NETAMOUNT,		      "PREP_MOA_NO_NETAMOUNT",	        NULL},
  {PREP_MOA_NO_GROSAMOUNT,		      "PREP_MOA_NO_GROSAMOUNT",	        NULL},
  {PREP_MOA_NO_VAT_FOR_ITEM,		   "PREP_MOA_NO_VAT_FOR_ITEM",	     NULL},
  {PREP_NO_TAX_RATE,			         "PREP_NO_TAX_RATE",		           NULL},
  {PREP_PIA_NO_ADD_INFO,		      "PREP_PIA_NO_ADD_INFO",		        NULL},
  {PREP_NO_QTY,			            "PREP_NO_QTY",			              NULL},
  {PREP_NAD_NO_5035,			         "PREP_NAD_NO_5035",		           NULL},
  {PREP_UNDEF_DOC_LANG,		         "PREP_UNDEF_DOC_LANG",		        NULL},
  {PREP_FIXED_UNDEF,			         "PREP_FIXED_UNDEF",		           NULL},
  {PREP_FIXED_WRONG_VERSION,		   "PREP_FIXED_WRONG_VERSION",	     NULL},
  {PREP_FIXED_FILE_MISSING,		   "PREP_FIXED_FILE_MISSING",	        NULL},
  {PREP_G23_MOA_NO_THRESHOLD,		   "PREP_G23_MOA_NO_THRESHOLD",	     NULL},
  {PREP_AMNT_LESS_THAN_THRESHOLD,	"PREP_AMNT_LESS_THAN_THRESHOLD",   NULL},
  {PREP_XCD_UNDEF_CALL_DEST,		   "PREP_XCD_UNDEF_CALL_DEST",	     NULL},
  {PREP_XCD_UNDEF_CALL_TYPE,		   "PREP_XCD_UNDEF_CALL_TYPE",	     NULL},
  {PREP_G45_NO_ROUNDING_DIFF,		   "PREP_G45_NO_ROUNDING_DIFF",	     NULL},
  {PREP_MOA_NO_SUM_TO_PAY,		      "PREP_MOA_NO_SUM_TO_PAY",	        NULL},
  {PREP_MOA_NO_INV_TAX_SUM,		   "PREP_MOA_NO_INV_TAX_SUM",	        NULL},
  {PREP_MOA_NO_NETTO_INV_SUM,		   "PREP_MOA_NO_NETTO_INV_SUM",	     NULL},
  {PREP_MOA_NO_INV_SUM_AMOUNT,	   "PREP_MOA_NO_INV_SUM_AMOUNT",	     NULL},
  {PREP_G3_NO_SENDER_VAT_NO,		   "PREP_G3_NO_SENDER_VAT_NO",	     NULL},
  {PREP_FII_NO_SENDER_POST_NO,	   "PREP_FII_NO_SENDER_POST_NO",	     NULL},
  {PREP_NO_SENDER_FAX_NO,		      "PREP_NO_SENDER_FAX_NO",	        NULL},
  {PREP_NO_SENDER_TELEX_NO,		   "PREP_NO_SENDER_TELEX_NO",	        NULL},
  {PREP_NO_SENDER_PHONE_NO,		   "PREP_NO_SENDER_PHONE_NO",	        NULL},
  {PREP_NO_SENDER_ADDRESS,		      "PREP_NO_SENDER_ADDRESS",	        NULL},
  {PREP_NO_SENDER_PHONE_FAX_TELEX,	"PREP_NO_SENDER_PHONE_FAX_TELEX",  NULL},
  {PREP_NO_CUSTOMER_ADDRESS,		   "PREP_NO_CUSTOMER_ADDRESS",	     NULL},
  {PREP_NO_CUSTOMER_NUMBER,		   "PREP_NO_CUSTOMER_NUMBER",	        NULL},
  {PREP_NO_INVOICE_NUMBER,		      "PREP_NO_INVOICE_NUMBER",	        NULL},
  {PREP_NO_INVOICE_DATE,		      "PREP_NO_INVOICE_DATE",		        NULL},
  {PREP_NO_BILLING_PERIOD,		      "PREP_NO_BILLING_PERIOD",	        NULL},
  {PREP_NO_RECEIVER_ADDRESS,		   "PREP_NO_RECEIVER_ADDRESS",	     NULL},
  {PREP_G45_NO_MOA,			         "PREP_G45_NO_MOA",		           NULL},
  {PREP_NO_G45,			            "PREP_NO_G45",			              NULL},
  {PREP_G22_G23_NO_MOA,		         "PREP_G22_G23_NO_MOA",		        NULL},
  {PREP_INVALID_G22_SORTED,		   "PREP_INVALID_G22_SORTED",	        NULL},
  {PREP_UNDEF_NESTING_LEVEL,		   "PREP_UNDEF_NESTING_LEVEL",	     NULL},
  {PREP_NO_G22,			            "PREP_NO_G22",			              NULL},
  {PREP_NO_G22_G26_RFF,		         "PREP_NO_G22_G26_RFF",		        NULL},
  {PREP_NO_G22_G31_NAD,		         "PREP_NO_G22_G31_NAD",		        NULL},
  {PREP_LIN_ILL_NEST_IND,		      "PREP_LIN_ILL_NEST_IND",	        NULL},
  {PREP_NO_G22_LIN,			         "PREP_NO_G22_LIN",		           NULL},
  {PREP_NO_G22_IMD,			         "PREP_NO_G22_IMD",		           NULL},
  {PREP_NO_G22_MOA,			         "PREP_NO_G22_MOA",		           NULL},
  {PREP_UNDEF_IMD_CHARGE_TYPE,	   "PREP_UNDEF_IMD_CHARGE_TYPE",	     NULL},
  {PREP_NO_G23_MOA,			         "PREP_NO_G23_MOA",		           NULL},
  {PREP_RFF_NO_CUSTOMER_CODE,		   "PREP_RFF_NO_CUSTOMER_CODE",	     NULL},
  {PREP_IMD_NO_SIM_NUMBER,		      "PREP_IMD_NO_SIM_NUMBER",	        NULL},
  {PREP_IMD_NO_MSISDN_NUMBER,		   "PREP_IMD_NO_MSISDN_NUMBER",	     NULL},
  {PREP_NO_XCDS,			            "PREP_NO_XCDS",			           NULL},
  {PREP_ITB_NO_CALL_REC_INFO,		   "PREP_ITB_NO_CALL_REC_INFO",	     NULL},
  {PREP_NO_ROAMING_INFO,		      "PREP_NO_ROAMING_INFO",		        NULL},
  {PREP_NO_VPLMN_NAME,		         "PREP_NO_VPLMN_NAME",		        NULL},
  {PREP_NO_FOREIGN_TAX,		         "PREP_NO_FOREIGN_TAX",		        NULL},
  {PREP_NO_SUM_INCL_TAX,		      "PREP_NO_SUM_INCL_TAX",		        NULL},
  {PREP_NO_TOTAL_USAGE_AMOUNT,	   "PREP_NO_TOTAL_USAGE_AMOUNT",	     NULL},
  {PREP_NO_TOTAL_SURCHARGE_AMOUNT,	"PREP_NO_TOTAL_SURCHARGE_AMOUNT",  NULL},
  {PREP_NO_TAX_NUMBER,		         "PREP_NO_TAX_NUMBER",		        NULL},
  {PREP_BALANCE_ERROR,		         "PREP_BALANCE_ERROR",		        "Error generating Balance sheet!"},

  {GEN_MALLOC,                      "GEN_MALLOC",                      NULL},
  {GEN_INCORRECT_ENV,               "GEN_INCORRECT_ENV",               NULL},
  {GEN_INCORRECT_DOC_TYPE,          "GEN_INCORRECT_DOC_TYPE",          NULL},
  {GEN_NEW_STREAM,                  "GEN_NEW_STREAM",                  NULL},
  {GEN_NEXT_DOC,                    "GEN_NEXT_DOC",                    NULL},
  {GEN_NEXT_DOC_FLUSH,              "GEN_NEXT_DOC_FLUSH",              NULL},
  {GEN_ACCESS_DIR,                  "GEN_ACCESS_DIR",                  NULL},
  {GEN_CREATE_DIR,                  "GEN_CREATE_DIR",                  NULL},
  {GEN_INCORRECT_BILL_MEDIUM,       "GEN_INCORRECT_BILL_MEDIUM",       NULL},
  {GEN_NEXT_IMAGE,                  "GEN_NEXT_IMAGE",                  NULL},
  {GEN_CREATE_GENERATOR,            "GEN_CREATE_GENERATOR",            NULL},
  {GEN_PRINT,                       "GEN_PRINT",                       NULL},
  {GEN_FORMAT_ITB_LINE,             "GEN_FORMAT_ITB_LINE",             NULL},
  {GEN_FLUSH_DOCUMENT,              "GEN_FLUSH_DOCUMENT",              NULL},  
  {GEN_FLUSH_ITB,                   "GEN_FLSUH_ITB",                   NULL},
  {GEN_FLUSH_IMAGE,                 "GEN_FLSUH_IMAGE",                 NULL},
  {GEN_INSERT_IMAGE,                "GEN_INSERT_IMAGE",                NULL},
  
  {STREAM_MALLOC,                   "STREAM_MALLOC",                   NULL},
  {STREAM_MAP_FILE,                 "STREAM_MAP_FILE",                 NULL},
  {STREAM_CREATE_LINE_LIST,         "STREAM_CREATE_LINE_LIST",         NULL},
  {STREAM_CREATE_LINE_LIST,         "STREAM_CREATE_LINE_LIST",         NULL},
  {STREAM_OPEN_OUTPUT_FILE,         "STREAM_OPEN_OUTPUT_FILE",         NULL},
  {STREAM_WRITE_OUTPUT_FILE,        "STREAM_WRITE_OUTPUT_FILE",        NULL},
  {STREAM_CLOSE_OUTPUT_FILE,        "STREAM_CLOSE_OUTPUT_FILE",        NULL},
  {STREAM_DELETE_LINE_LIST,         "STREAM_DELETE_LINE_LIST",         NULL},
  {STREAM_APPEND_LINE_LIST,         "STREAM_APPEND_LINE_LIST",         NULL},
  {STREAM_CONVERT_LINE_LIST,        "STREAM_CONVERT_LINE_LIST",        NULL},
  {STREAM_INSERT_BILL_IMAGE,        "STREAM_INSERT_BILL_IMAGE",        NULL},
  {STREAM_Z_MEM_ERROR,              "STREAM_Z_MEM_ERROR",              NULL},
  {STREAM_Z_BUF_ERROR,              "STREAM_Z_BUF_ERROR",              NULL},
  {STREAM_COMPRESSION_ERROR,        "STREAM_COMPRESSION_ERROR",        NULL},
  {STREAM_SAVE_BILL_IMAGE,          "STREAM_SAVE_BILL_IMAGE",          NULL},
  {STREAM_BAD_BILL_MEDIUM,          "STREAM_BAD_BILL_MEDIUM",          NULL},
  {STREAM_MAP_TYPE,                 "STREAM_MAP_TYPE",                 NULL},
  {STREAM_SAVEPOINT_LSEEK_FILE,     "STREAM_SAVEPOINT_LSEEK_FILE",     NULL},
  {STREAM_ROLLBACK_UNLINK_FILE,     "STREAM_ROLLBACK_UNLINK_FILE",     NULL},
  {STREAM_ROLLBACK_TRUNCATE_FILE,   "STREAM_ROLLBACK_TRUNCATE_FILE",   NULL},

  {BILL_NO_OUTPUT_GENERATED,	    "BILL_NO_OUTPUT_GENERATED",	       "No bill image generated!"},

  {ERR_END,                         "ERR_END",                         "the last element..."}
};

/* --- external declarations --- */
extern char *pszProgName;                    /* from bgh_main.c */
extern char *SCCS_VERSION;                   /* from bgh_main.c */

void user_f1(void);
void user_f2(void);

/* Static function prototypes   */
static void fovdInitCallTrace(void);
static void fovdListCallTree(void);
void signalABRT(int signo);
void signalFPE(int signo);
void signaILL(int signo);
void signaINT(int signo);
void signalSEGV(int signo);
void signaTERM(int signo);
static void ShutDownErrh (enum ERROR_NUMBERS polMsgNo);


/******************************************************************************
 * PrintVersInfoBghErrh
 *
 * DESCRIPTION:
 * print the version info of all BGHERRH
 *
 * PARAMETERS:
 *  void
 *
 * RETURNS:
 *  - none -
 ******************************************************************************
 */
void PrintVerInfoBghErrh (void)
{
  static char *SCCS_ID = "1.19";

  printf ("%s\t\t\t%s\n", __FILE__, SCCS_ID);
}


/******************************************************************************
 * PrintErrorInfo
 *
 * DESCRIPTION:
 * print an info about the current error / return value
 *
 * PARAMETERS:
 * enum ERROR_NUMBERS   enErrNr         - error number
 *
 * RETURNS:
 *  - none -
 ******************************************************************************
 */
void PrintErrorInfo (enum ERROR_NUMBERS enErrNr)
{
  int   i;                              /* counter */

  /*
   * search the error in the table, print a message if it is not found
   */
  printf ("Code: %d", enErrNr);
  for (i = 0; i < (sizeof (stErrorTable) / sizeof (struct ERRTABLE)); i++) {

    if (enErrNr == stErrorTable[i].enErrNr) {
      printf (", \"%s\"\n", stErrorTable[i].szErrText);
      if (stErrorTable[i].szLongErr != NULL) {
        printf ("Info: \"%s\"\n", stErrorTable[i].szLongErr);
      }
      break;                            /* terminate for-loop */
    }
  }
  if (i == (sizeof (stErrorTable) / sizeof (struct ERRTABLE))) {
    printf (" UNKNOWN ERROR!\n");
  }
}


/******************************************************************************
 * fovdErrorMessage - process one error message
 *
 * DESCRIPTION:
 *
 * input parameters:
 *
 *   poszSourceFile   - The file name of the source module in which the
 *                      error occured.
 *   polLineNo        - The exact line number in the source module in which
 *                      the error occured
 *   polMsgNo         - A system wide unique message number identifying the
 *                      message
 *   polSeverity      - One of the following three levels:
 *                      normal, warning, critical.
 *   poszMessageText  - Up to 2048 characters message text. Linefeeds within
 *                      the message text are allowed. Leading and trailing
 *                      linefeeds are added by this function automatically.
 *
 * This functions first checks the parameters of the message, then, when
 * the actual logging level is sufficient, writes it to the log file and
 * eventually passes the message on to a remote operations control center.
 * If severity is critical, the actual context is saved within the log file,
 * then the function bgh_exit is started to close all open files, return
 * all ressources to the operating system and shut down the BGH process.
 *
 * RETURNS: none
 ******************************************************************************
 */
void fovdErrorMessage (char *poszSourceFile, long polLineNo, 
                       enum ERROR_NUMBERS polMsgNo,
                       enum ERROR_TYPES polSeverity, 
                       char *poszMessageText)
{
  char        loszSeverityText [10];/* temporary buffer                      */
  struct tm   *lotmsTimeStruct;      
  time_t      timer;
  int         iYear;
  static char szTemp[256];

  fovdPushFunctionName ("fovdErrorMessage");
  /*
   * check input parameters
   */

  /*
   * write error message to log file
   */
  if      (polSeverity == NORMAL)   strcpy(loszSeverityText,"normal");
  else if (polSeverity == WARNING)  strcpy(loszSeverityText,"warning");
  else if (polSeverity == CRITICAL) strcpy(loszSeverityText,"critical");
  else                              strcpy(loszSeverityText,"????????");

  fflush (stdout);

  if (polSeverity > WARNING) {
    time(&timer);                           /* get current time in seconds */
    lotmsTimeStruct = localtime(&timer);    /* fill time structure         */

    iYear = lotmsTimeStruct->tm_year + 1900;
    if (lotmsTimeStruct->tm_year < 90) {
      iYear += 100;
    }
    sprintf (szTemp,
             "\nprogram: %s, version: %s, source: %s, line: %ld, pid: %X"
             "\nmessage: %d, level: %s, "
             "time: %4.4d:%2.2d:%2.2d:%2.2d:%2.2d:%2.2d\n",
             pszProgName, SCCS_VERSION, poszSourceFile,
             polLineNo, doiProcessId,
             polMsgNo, loszSeverityText,iYear,
             1+lotmsTimeStruct->tm_mon, lotmsTimeStruct->tm_mday,
             lotmsTimeStruct->tm_hour, lotmsTimeStruct->tm_min,
             lotmsTimeStruct->tm_sec);
    fputs (szTemp, stderr);
    fovdPrintLog (LOG_MAX, "%s", szTemp);
  }

  fprintf (stderr, "%s\n", poszMessageText);
  fovdPrintLog (LOG_MAX, "%s\n", poszMessageText);


  /*
   * shut down the BGH after a critical error
   */
  if (polSeverity == CRITICAL) {

    ShutDownErrh (polMsgNo);

  } else if (polSeverity > WARNING) {

    /* donnt list call tree twice */
    fovdListCallTree();

  }

  fovdPopFunctionName ();
}

/******************************************************************************
 * fovdInitCallTrace - initialize data structure for trace of call tree
 *
 * DESCRIPTION:
 *
 * input parameters: none
 *
 * This function initializes the stack for the call tree, so that in the
 * case of hard errors, it's possible to catch the error signal and write
 * the actual call tree to the log file.
 *
 * RETURNS: none
 ******************************************************************************
 */
static void fovdInitCallTrace(void)
{
  memset (&gaszCallStack[0], 0, sizeof (gaszCallStack));
  goiSpCallStack = 0;
}

/******************************************************************************
 * fovdPushFunctionName - push address of function name to call stack
 *
 * DESCRIPTION:
 *
 * input parameters:
 *
 *   NamePtr - pointer to name of function
 *
 * RETURNS: none
 ******************************************************************************
 */
void fovdPushFunctionName(char *NamePtr)
{
   static char *doszStr = "Stack Overflow";

   if (goiSpCallStack < SIZE_CALL_STACK)
   {
     gaszCallStack [goiSpCallStack++] = NamePtr; /* save string pointer */
   }
   else
   {
     goiSpCallStack--;
     gaszCallStack [goiSpCallStack++] = doszStr; /* indicate overflow  */
   }
}

/******************************************************************************
 * fovdPopFunctionName - push address of function name to call stack
 *
 * DESCRIPTION:
 *
 * input parameters: none
 *
 * RETURNS: none
 ******************************************************************************
 */
void fovdPopFunctionName(void)
{
   static char *doszStr = "Stack Underflow";

   if (goiSpCallStack > 0)
   {
     goiSpCallStack--;                        /* remove last string pointer */
   }
   else
   {
     gaszCallStack [0] = doszStr;             /* indicate underflow  */
   }
}

/******************************************************************************
 * fovdErrorPopFunctionName - push address of function name to call stack
 *
 * DESCRIPTION:
 *
 * input parameters: none
 *
 * RETURNS: none
 ******************************************************************************
 */
void fovdErrorPopFunctionName(void)
{
   static char *doszStr = "Stack Underflow";

   if (goiSpCallStack > 0)
   {
     fprintf (stderr, "RETURNING ERROR FROM : %s\n", gaszCallStack[goiSpCallStack]);
     goiSpCallStack--;                        /* remove last string pointer */
   }
   else
   {
     gaszCallStack [0] = doszStr;             /* indicate underflow  */
   }
}


/******************************************************************************
 * fovdListCallTree - Write contents of call stack to device stderr
 *
 * DESCRIPTION:
 *
 * input parameters: none
 *
 * RETURNS: none
 ******************************************************************************
 */
static void fovdListCallTree(void)
{
  int i;

  fprintf(stderr,"\nContents of call tree:\n");
  for (i=0; i < goiSpCallStack; i++)
    {
      fovdPrintLog (LOG_MAX, "  %s\n",gaszCallStack [i]);
      fprintf(stderr,"  %s\n",gaszCallStack [i]);
    }
}

/******************************************************************************
 * SignalABRT - process abort signal
 *
 * DESCRIPTION:
 *
 * input parameters: none
 *
 * RETURNS: none
 ******************************************************************************
 */
void signalABRT(int signo)
{
  fovdPrintLog (LOG_MAX, "\nSIGABRT signal received\n");
  fprintf(stderr,"\nSIGABRT signal received\n");
  /*
  ShutDownErrh(ERRH_SIGABORT);
  */
  
  abort();
}

/******************************************************************************
 * SignalFPE - process arithmetic fault signal
 *
 * DESCRIPTION:
 *
 * input parameters: none
 *
 * RETURNS: none
 ******************************************************************************
 */
void signalFPE(int signo)
{
  fovdPrintLog (LOG_MAX, "\nSIGFPE signal received\n");
  fprintf(stderr,"\nSIGFPE signal received\n");
  ShutDownErrh(ERRH_SIGFPE);
}

/******************************************************************************
 * SignalILL - illegal hardware command signal
 *
 * DESCRIPTION:
 *
 * input parameters: none
 *
 * RETURNS: none
 ******************************************************************************
 */
void signalILL(int signo)
{
  fovdPrintLog (LOG_MAX, "\nSIGILL signal received\n");
  fprintf(stderr,"\nSIGILL signal received\n");
  ShutDownErrh(ERRH_SIGILL);
}

/******************************************************************************
 * SignalINT - terminate signal from terminal
 *
 * DESCRIPTION:
 *
 * input parameters: none
 *
 * RETURNS: none
 ******************************************************************************
 */
void signalINT(int signo)
{
  fovdPrintLog (LOG_MAX, "\nSIGINT signal received\n");
  fprintf(stderr,"\nSIGINT signal received\n");
  ShutDownErrh(ERRH_SIGINT);
}

/******************************************************************************
 * SignalSEGV - process illegal memory address fault signal
 *
 * DESCRIPTION:
 *
 * input parameters: none
 *
 * RETURNS: none
 ******************************************************************************
 */
void signalSEGV(int signo)
{
  fovdPrintLog (LOG_MAX, "\nSIGSEGV signal received\n");
  fprintf(stderr,"\nSIGSEGV signal received\n");
  ShutDownErrh(ERRH_SIGSEGV);
}

/******************************************************************************
 * SignalTERM - process terminate signal from kill command
 *
 * DESCRIPTION:
 *
 * input parameters: none
 *
 * RETURNS: none
 ******************************************************************************
 */
void signalTERM(int signo)
{
  fovdPrintLog (LOG_MAX, "\nSIGTERM signal received\n");
  fprintf(stderr,"\nSIGTERM signal received\n");
  ShutDownErrh(ERRH_SIGTERM);
}

/******************************************************************************
 * fovdInitBGHErrh - initialize the BGH error handler
 *
 * DESCRIPTION:
 *
 * input parameters: none
 *
 * RETURNS: none
 ******************************************************************************
 */
void fovdInitBGHErrh(void)
{
   dofFirstCall = FALSE;                   /* set flag 'data initialized'  */
   doiProcessId = getpid();                /* get process id of BGH        */

   fovdInitCallTrace ();

   /* --- define function addresses for intercept of special signals --- */
#ifdef CATCH_SIGABRT_AND_EXIT
     signal(SIGABRT, signalABRT);          /* define user routine for ...  */
                                           /* abort signal                 */
#endif
#ifdef CATCH_SIGFPE_AND_EXIT
     signal(SIGFPE, signalFPE);            /* define user routine for ...  */
                                           /* arithmetic fault signals     */
#endif
#ifdef CATCH_SIGILL_AND_EXIT
     signal(SIGILL, signalILL);            /* define user routine for ...  */
                                           /* illegal hardware cmd signal  */
#endif
#ifdef CATCH_SIGINT_AND_EXIT
     signal(SIGINT, signalINT);            /* define user routine for ...  */
                                           /* terminal interrupt signal    */
#endif
#ifdef CATCH_SIGSEGV_AND_EXIT
     signal(SIGSEGV, signalSEGV);          /* define user routine for ...  */
                                           /* memory address fault signals */
#endif
#ifdef CATCH_SIGTERM_AND_EXIT
     signal(SIGTERM, signalTERM);          /* define user routine for ...  */
                                           /* terminate signal from kill   */
#endif

   memset(&dpavdCallbackFunctions[0], 0, sizeof(dpavdCallbackFunctions));
}


/******************************************************************************
 * foiRegisterCallbackFunction - save address of user function
 *
 * DESCRIPTION:
 *
 * Before anormal termination of the process the user function is called back
 * by the error handler to clean up, e.g. close all open files etc.
 * Depending on the error condition this will not always succeed.
 *
 *
 * input parameters:
 *  address of user function of type void name(void)
 *
 * RETURNS:
 *
 *   0 - callback function was successful registered
 *  -1 - error (table overflow)
 *
 ******************************************************************************
 */
int foiRegisterCallbackFunction (void (*UserFunction) (void))
{
   int i;
   for (i=0;i<NO_CALLBACK_FUNCTIONS;i++)
   {
     if (dpavdCallbackFunctions [i] == UserFunction) /* already registered ?*/
     {
       return 0;                                   /* yes, return 'success'  */
     }
     else if (dpavdCallbackFunctions [i] == 0)     /* free entry found ?     */
     {
       dpavdCallbackFunctions [i] = UserFunction;  /* yes, insert address    */
       return 0;                                   /* return 'success'       */
     }
   }
   return -1;                                      /* return 'no success'    */
}

/******************************************************************************
 * ShutDownErrh - terminate process
 *
 * DESCRIPTION:
 *
 * Write call tree to log file, call back all registered user functions
 * and terminate process.
 *
 * input parameters:
 *  nothing
 *
 * RETURNS:
 *
 *  nothing
 *
 ******************************************************************************
 */

void ShutDownErrh (enum ERROR_NUMBERS polMsgNo)
{
   int i;
   void (*func)(void);             /* declare function pointer    */

   return;

   fovdListCallTree();               /* write actual call tree to log file */
   for (i=0;i<NO_CALLBACK_FUNCTIONS;i++)
   {
     if (dpavdCallbackFunctions[i] != 0)
     {
       func = dpavdCallbackFunctions[i];   /* get address of user function */
       func();                             /* call user function           */
     }
   }
   /*
    * close the log file just before terminating
    */
   fovdCloseLog ();

   /*
    * exit and return error-number
    */
   exit ((int) polMsgNo);
}

void ShutDown ()
{
   int i;
   void (*func)(void);             /* declare function pointer    */

   fovdListCallTree();               /* write actual call tree to log file */
   for (i=0;i<NO_CALLBACK_FUNCTIONS;i++)
   {
     if (dpavdCallbackFunctions[i] != 0)
     {
       func = dpavdCallbackFunctions[i];   /* get address of user function */
       func();                             /* call user function           */
     }
   }
   /*
    * close the log file just before terminating
    */
   fovdCloseLog ();

   /*
    * exit and return error-number
    */
   exit ((int) 1);
}




