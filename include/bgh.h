/*******************************************************************************
 * LH-Specification GmbH 1995.
 *
 * All rights reserved.
 * Copying of this software or parts of this software is a violation of German
 * and International laws and will be prosecuted.
 *
 * Project  :   BGH
 *
 * File     :   bgh.h
 * Created  :   Feb. 1996
 * Author(s):   B. Michler
 *
 * Modified :
 * 26.06.96     B. Michler      additional parameter of type stDBCOL * for
 *                              database access routines
 * 14.08.96     B. Michler      added parameter for passing the document type
 *                              to process
 *
 * 20.07.98 N. Bondarczuk
 *
 * 23.07.99 L. Dabrowski        additional global structure member of stBGHGLOB 
 *                              double    sofoThresholdValue
 *                              BOOL      soContractCallDetailListExist
 *                              BOOL      soSubscriberCallDetailListPrinted
 *                              to provide information whether to print detail call list
 *
 *******************************************************************************/

#ifndef BGH_H
#define BGH_H

#if 0 /* just for version.sh */
static char *SCCS_VERSION = "1.45";
#endif


#include "parser.h"            /* needed for typedef */

/* 
 *  Defines                      
*/

#define COMMON_HEADER_FILE "header.ps"
#define LOCAL_CURRENCY "PLN"
#define SPECIAL_NUMBER_PREFIX "501000000*"
#define SPECIAL_NUMBER_PREFIX_INDEX 9
#define INVOICE_IMAGE_TYPE 1

/* 
 * take care of lowercase define for BSCS4 
 */

#ifdef bscs4
#  define BSCS4         1
#endif


#define FALSE   0
#define TRUE       1

#define UINT    unsigned int
#define ULONG   unsigned long
#define BYTE    unsigned char

#ifndef PATH_MAX                /* ensure that PATH_MAX is set */
#define PATH_MAX    1024
#endif

#define BGHSTRLEN   32          /* standard string in BGH is 32 bytes long  */
#define DEFTIMMLEN  2048        /* starting length for TIMM-message         */

#define DEFNRINVOICES   1
#define DEFNOCUSTID     -1              /* no customer id was set                   */

/*
 * filenames
 */

#define BGH_LOGFILE "./bgh.log"


/* 
 * Enumerations                 
*/

typedef enum                   /* document type */
{
  BAL_TYPE = 0,
  INV_TYPE = 1,
  ITB_TYPE = 2,
  SUM_TYPE = 3,
  ROA_TYPE = 4,
  LGN_TYPE = 5,
  INV_DCH  = 6,
  ITM_DCH  = 7,
  DNL_DWH  = 8,
  WLL_DWH  = 9,
  INV_IR   = 10,
  INV_EC   = 11,
  INH_INP  = 12,
  INH_INL  = 13,
  ENC_TYPE = 14,
  TYPLAST                                    /* the last one */
} TYPEID;

typedef enum                   /* customer-process type    */
{
  SINGLE   = 0,
  MULTIPLE = 1,
  CUR2ND   = 2                 /* 2nd cursor */ 

} TYPECUST;

/* 
 * logging levels, the last has to be LOG_MAX 
 */

typedef enum 
{
  LOG_DEBUG = 1,               /* logs everything, for debugging purposes */
  LOG_TIMM = 2,
  LOG_CUSTOMER = 3,
  LOG_NORMAL = 4,
  LOG_SELDOM = 5,
  LOG_MAX = 6
} LOGLEVEL;

/* 
 * enumerations for error handling 
*/

enum ERROR_TYPES 
{
  UNDEFINED = 0, 
  NORMAL, 
  WARNING, 
  CRITICAL
};

enum {
  NO_PARALLEL_PROC = 0,
  NR_OF_PROC_MIN   = 2,
  NR_OF_PROC_MAX   = 50
};


/* CAUTION CAUTION CAUTION CAUTION CAUTION CAUTION CAUTION CAUTION CAUTION      */
/* if an error is added to the following table, be sure to add a corres-        */
/* ponding line to 'bgherrh.c' !!                                               */

enum ERROR_NUMBERS 
{
  NO_ERROR = 0,
  
  ERRH_SIGINT           = 2,      /* signals */
  ERRH_SIGILL           = 4,
  ERRH_SIGABORT         = 6,
  ERRH_SIGFPE           = 8,
  ERRH_SIGSEGV          = 11,
  ERRH_SIGTERM          = 15,

  /*
   * MAIN
   */

  MAIN_NOTIMM           = 100,     /* no TIMM message */
  MAIN_INVTYPE,                    /* invalid TIMM type */
  MAIN_PARAMERR,                   /* error in passed parameter */
  MAIN_LOGLEVERR,                  /* invalid loglevel */
  MAIN_NOFNAME,                    /* filename missing although option needs one */
  MAIN_UNKNOWNOPT,                 /* unknown option */
  MAIN_VERERROR,                   /* wrong version of BGH */
  MAIN_FORKERR,                  /* cannot fork process */
  MAIN_NR_OF_PROC_ERR,          /* wrong number of processes */

  /*
   * PROC
   */

  PROC_DBOPENERR        = 200,  /* cannot open database */
  PROC_INVERR,                  /* error reading invoice */
  PROC_TIMMVERERR,              /* wrong version of TIMM message */
  PROC_NOTIMMVER,               /* TIMM version not found in TIMM message */
  PROC_NOBASENAME,              /* no basename for file input of TIMM */
  PROC_MULTITIMM,               /* more TIMMs than expected in database */
  PROC_FILE_ROLL_BACK_ERROR,
  PROC_FILE_SAVE_POINT_ERROR,
  PROC_COMMIT_ERROR,
  PROC_STATUS_ERROR,
  
  /*
   * PARSER
   */
  
  PARSE_MALLOC  = 300,     /* no memory for malloc */
  PARSE_NOPOINTER,              /* pointer not set */
  PARSE_NOATTACH,               /* Attach-point for segment not set */
  PARSE_MANDMISSING,            /* mandatory element missing */
  PARSE_ELEMTOOLONG,            /* element is too long */
  PARSE_ILLEGALELEM,            /* element not allowed a this position */
  PARSE_TOOMUCHELEM,            /* too much elements in a repetition */

  /*
   * ACCESS
   */

  ACC_NOPASSWORD        = 400,  /* no database-password for BGH */
  ACC_NOCONNECTSTRING,          /* no database-connect-string for BGH */
  ACC_NOBASEDIR,        /* no base-directory info  */
  ACC_SECCURSOR_ILLUSAGE,       /* try to close 2nd cursor with illegal usage */
  ACC_NOVERSIONCHECK,           /* no version check to be done */
  ACC_NOPROCPROG,               /* no bill image processing program */

  /*
   * ESQL
   */

  ESQL_ERROR            = 500,  /* error in esql execution */
  ESQL_DBCONNECT,               /* cannnot connect database */
  ESQL_DBDISCONNECT,            /* cannnot disconnect database */
  ESQL_OPENMAINCURSOR,          /* error by open from main cursor */
  ESQL_OPENSECONDCURSOR,        /* error by open from second cursor */
  ESQL_OPENSECMAINCLOSE,        /* try to open second cursor with main closed */
  ESQL_OPENTESTCURSOR,          /* error by open from test cursor */
  ESQL_OPENBALANCECURSOR,     /* error by open from balance cursor */
  ESQL_CLOSEMAINCURSOR,         /* error by close from main cursor */
  ESQL_CLOSESECONDCURSOR,       /* error by close from second cursor */
  ESQL_CLOSETESTCURSOR,         /* error by close from test cursor */
  ESQL_CLOSEBALANCECURSOR,    /* error by close from balance cursor */
  ESQL_FETCHCURSOR,             /* error by fetching for main(second) cursor */
  ESQL_FETCHTESTCURSOR,         /* error by fetching for test cursor */
  ESQL_SECCURSOR_ILLUSAGE,      /* try to manipulate second cursor with illegal usage */
  ESQL_ILLCUSTPROC,             /* illegal customer-process-usage for main cursor */
  ESQL_SELECT,                  /* error in SQL-SELECT statement */
  ESQL_SELVERS,                 /* error in SQL-SELECT statement version-number */
  ESQL_CHECKBATCH,              /* error in SQL-SELECT statement version-check  */
  ESQL_SELBASEDIR,              /* error in SQL-SELECT statement base directory */
  ESQL_NULLBASEDIR,             /* NULL detected in database for base directory */
  ESQL_SELPROCPROG,             /* error in SQL-SELECT statement bill-images-process-program */
  ESQL_NULLPROCPROG,            /* NULL detected in database for bill-images-process-program */
  ESQL_SELCOUNT,                /* error in SQL-SELECT statement count(*) Xs */
  ESQL_UPDATE,                  /* error in SQL-UPDATE statement */
  ESQL_UPDATEMOREASONE,         /* update more as a record */
  ESQL_RESETX,                  /* error in SQL-UPDATE statement reset Xs */
  ESQL_NOX,                     /* nothing to reset */
  ESQL_COMMIT,                  /* error in SQL-COMMIT statement */
  ESQL_COMMIT_DBCLOSED,         /* commit with closed database */
  ESQL_ROLLBACK,                /* error in SQL-ROLLBACK statement */
  ESQL_ROLLBACK_DBCLOSED,       /* rollback with closed database */
  ESQL_ALLOC,                   /* no memory for malloc */
  ESQL_REALLOC,                 /* no memory for realloc */
  ESQL_NOMOREDATA,              /* no more data to read from database */
  ESQL_DOCSIZENULL,             /* readed document has size zero */
  ESQL_BILLINSERT,              /* insert into table bill_images */
  ESQL_BILL_ILLTYPE,            /* illegal timm-type for bill_image */
  ESQL_BILL_DBCLOSED,           /* insert bill_image with closed database */
  ESQL_BILL_NULL,               /* nothing to insert, no image-information */
  ESQL_INV_COLUMN,                 /* invalid column name */
  ESQL_SET_NLS_LANG,        /* error during set up of NLS_LANG for sessiog */
  ESQL_OPENPARVALCURSOR,    /* error while openning the workparval cursor */
  ESQL_CLOSEPARVALCURSOR,   /* error while closing the workparval cursor */
  ESQL_FETCHPARVALCURSOR,   /* error while fetching with parval cursor */
  /*
   * FILE
   */

  FILE_MALLOC           = 600,  /* no memory for malloc */
  FILE_OPEN,                    /* file open failed */
  FILE_CLOSE,                   /* file close failed */
  FILE_WRITE,                   /* writing to file failed */
  FILE_INVALIDNAME,             /* name was not from invoice */
  FILE_PATHLENGTH,              /* path was too long for given buffer */
  FILE_NOMATCH,                 /* no match for wildcard search */
  FILE_OPENDIR,                 /* open on directory failed */
  FILE_NOACCESS,                /* no access to directory */

  /*
   * FORM
   */

  FORM_LINETOOSHORT     = 700,  /* truncated line in layout-file */
  FORM_LISTERROR,               /* error in list */
  FORM_CANNOTOPEN,              /* error opening layout-file */
  FORM_NOCLOSE,                 /* error closing layout-file */
  FORM_OUTMEM,                  /* no memory from malloc */
  FORM_LINEERR,                 /* error in line */

  /*
   * LAYOUT
   */
  
  LAY_NULLPOINTER       = 800,  /* wrong NULL-pointer in paField */
  LAY_NOMETAHEAD,               /* file 'metahead.ps' not found */
  LAY_INEXPCOUNT,               /* count was not expected in vdInitLayout */
  LAY_MALLOC,                   /* no memory from malloc */
  LAY_INITLINE,         /* line was not correct initialized */
  LAY_YPOSOFFPAGE,              /* element should be printed off pageend */
  LAY_XPOSOFFPAGE,              /* element should be printed off pageend */
  LAY_FROMTOOBIG,               /* 'from' element too big */
  LAY_TOFROM,                   /* 'to' smaller than 'from' */
  LAY_TOTOOBIG,         /* 'to' element too big */
  LAY_TOUNDEFINED,              /* 'to' element undefined (may not be 0) */
  LAY_NOBLOCKSIZE,              /* no blocksize in put_next_line */

  /*
   * PREP
   */

  PREP_UNDEF_DOCTYPE    = 900,
  PREP_UNDEF_LIN_7140,
  PREP_UNDEF_PIA_7140,
  PREP_UNDEF_PIA_7140_PpU,
  PREP_UNDEF_PIA_7140_CHARGE_TYPE,
  PREP_ILL_PIA_7140,
  PREP_NO_IMD_WITH_VPLMN_CODE,
  PREP_NO_IMD_SERVICE_CODE_ID,
  PREP_MALLOC_FAILED,
  PREP_ILL_MALLOC_REQUEST,
  PREP_IMD_NO_SERVICE_DESCR,
  PREP_PRI_NO_PRICE,
  PREP_MOA_NO_NETAMOUNT,
  PREP_MOA_NO_GROSAMOUNT,
  PREP_MOA_NO_VAT_FOR_ITEM,
  PREP_NO_TAX_RATE,
  PREP_PIA_NO_ADD_INFO,
  PREP_NO_QTY,
  PREP_NAD_NO_5035,
  PREP_UNDEF_DOC_LANG,
  PREP_FIXED_UNDEF,
  PREP_FIXED_WRONG_VERSION,
  PREP_FIXED_FILE_MISSING,
  PREP_G23_MOA_NO_THRESHOLD,
  PREP_AMNT_LESS_THAN_THRESHOLD,
  PREP_XCD_UNDEF_CALL_DEST,
  PREP_XCD_UNDEF_CALL_TYPE,
  PREP_G45_NO_ROUNDING_DIFF,
  PREP_MOA_NO_SUM_TO_PAY,
  PREP_MOA_NO_INV_TAX_SUM,
  PREP_MOA_NO_NETTO_INV_SUM,
  PREP_MOA_NO_INV_SUM_AMOUNT,
  PREP_G3_NO_SENDER_VAT_NO,
  PREP_FII_NO_SENDER_POST_NO,
  PREP_NO_SENDER_FAX_NO,
  PREP_NO_SENDER_TELEX_NO,
  PREP_NO_SENDER_PHONE_NO,
  PREP_NO_SENDER_ADDRESS,
  PREP_NO_SENDER_PHONE_FAX_TELEX,
  PREP_NO_CUSTOMER_ADDRESS,
  PREP_NO_CUSTOMER_NUMBER,
  PREP_NO_INVOICE_NUMBER,
  PREP_NO_INVOICE_DATE,
  PREP_NO_BILLING_PERIOD,
  PREP_NO_RECEIVER_ADDRESS,
  PREP_G45_NO_MOA,
  PREP_NO_G45,
  PREP_G22_G23_NO_MOA,
  PREP_INVALID_G22_SORTED,
  PREP_UNDEF_NESTING_LEVEL,
  PREP_NO_G22,
  PREP_NO_G22_G26_RFF,
  PREP_NO_G22_G31_NAD,
  PREP_LIN_ILL_NEST_IND,
  PREP_NO_G22_LIN,
  PREP_NO_G22_IMD,
  PREP_NO_G22_MOA,
  PREP_UNDEF_IMD_CHARGE_TYPE,
  PREP_NO_G23_MOA,
  PREP_RFF_NO_CUSTOMER_CODE,
  PREP_IMD_NO_SIM_NUMBER,
  PREP_IMD_NO_MSISDN_NUMBER,
  PREP_NO_XCDS,
  PREP_ITB_NO_CALL_REC_INFO,
  PREP_NO_ROAMING_INFO,
  PREP_NO_VPLMN_NAME,
  PREP_NO_FOREIGN_TAX,
  PREP_NO_SUM_INCL_TAX,
  PREP_NO_TOTAL_USAGE_AMOUNT,
  PREP_NO_TOTAL_SURCHARGE_AMOUNT,
  PREP_NO_TAX_NUMBER,
  PREP_LIN_LEVEL_ERROR,
  PREP_BALANCE_ERROR,
  
  /*
   * GEN
   */

  GEN_MALLOC = 1000,
  GEN_INCORRECT_ENV,
  GEN_INCORRECT_DOC_TYPE,
  GEN_NEW_STREAM,
  GEN_NEXT_DOC,
  GEN_NEXT_DOC_FLUSH,
  GEN_ACCESS_DIR,
  GEN_CREATE_DIR,
  GEN_INCORRECT_BILL_MEDIUM,
  GEN_NEXT_IMAGE,
  GEN_CREATE_GENERATOR,
  GEN_PRINT,
  GEN_FORMAT_ITB_LINE,
  GEN_FLUSH_DOCUMENT,
  GEN_FLUSH_ITB,
  GEN_FLUSH_IMAGE,
  GEN_INSERT_IMAGE,


  /*
   * STREAM
   */

  STREAM_MALLOC = 1100,
  STREAM_MAP_FILE,
  STREAM_CREATE_LINE_LIST,
  STREAM_INIT_LINE_LIST,
  STREAM_OPEN_OUTPUT_FILE,
  STREAM_WRITE_OUTPUT_FILE,
  STREAM_CLOSE_OUTPUT_FILE,
  STREAM_DELETE_LINE_LIST,
  STREAM_APPEND_LINE_LIST,
  STREAM_CONVERT_LINE_LIST,
  STREAM_INSERT_BILL_IMAGE,
  STREAM_Z_MEM_ERROR,
  STREAM_Z_BUF_ERROR,
  STREAM_COMPRESSION_ERROR,
  STREAM_SAVE_BILL_IMAGE,
  STREAM_BAD_BILL_MEDIUM,
  STREAM_MAP_TYPE,
  STREAM_SAVEPOINT_LSEEK_FILE,
  STREAM_ROLLBACK_UNLINK_FILE,
  STREAM_ROLLBACK_TRUNCATE_FILE,
  
  /*
   * Other error
   */

  BILL_NO_OUTPUT_GENERATED = 10000,
  
  ERR_END                       /* the last element... */
};

/* 
 * Local typedefs               
 */

typedef long BOOL;

typedef TIMM_INTERCHANGE stTIMM;

typedef TIMM_MESSAGE stTMSG;


/* 
 * interface structure for database access to document_all and bill_images 
 */

typedef struct 
{
  long          lCustId;                         /* Customer ID */
  long          lContId;                         /* Contract ID */
  long          lTypeId;                         /* Type ID */
  long          lCopies;                         /* no of copies for BILL_IMAGES */
  char          szCsLevel[3];              /* CSLEVEL */
  char          szBillIns[101];         /* Bill inserts */

} stDBCOL;

#ifdef BSCS4

/* 
 * structures for new DB tables 
 */

/* 
 *  structure for DOC_TYPES 
*/

typedef struct 
{
  long          lTypeId;                          /* TYPE_ID */
  char          cGenThisMsg;          /* GEN_THIS_MSG */
  stTIMM        *stTimm;                             /* Timm interchange */
  stDBCOL       stCol;                         /* info structure */

} stDOCTYPES;

/* 
 * structure for IMG_TYPES 
 */

typedef struct 
{
  long          lTypeId;                          /* TYPE_ID */
  char          szShdes[6];                    /* TYPE_SHDES */
  long          lDefCopies;                    /* DEFAULT_COPIES */

} stIMGTYPES;

/* 
 * structure for IMG_TYPES_LEVEL 
*/

typedef struct 
{
  long          lTypeId;                          /* TYPE_ID */
  char          szCslevel[3];               /* CSLEVEL */
  long          lCopies;                          /* COPIES */
  int              aiDocTyp[TYPLAST+1];  /* array of doc_types needed */
  int              BchItb: 1;                  /* 1 => needs Itemized TIMM */
} stIMGTYPESLV;

/* 
 *  structure for IMG_LNK_DOC 
 */

typedef struct 
{  
  long          lDocType;       /* DOC_TYPE_ID */
  long          lImgType;       /* IMG_TYPE_ID */

} stIMGLNKDOC;

/* 
 * structure for CUST_IMAGES 
*/

typedef struct 
{
  long          lCustId;                /* CUSTOMER_ID */
  long          lTypeId;                /* TYPE_ID */
  long          lCopies;                /* COPIES */

} stCUSTIMAGES;

#endif


typedef struct 
{
  char szPrefix[32];
  char szDest[32];
  char szZone[16];
  int  lIndex;
  char *szRegexp;

} stDest;

/*
 * VPLMN Info for roaming usage labels
 */

typedef struct 
{
  long lIndex;                 /* index in table */
  char szShdes[8];             /* Short Description of Roaming Partner */
  char szPlmnName[16];         /* Description of Roaming Partner */
  char szCountry[32];          /* countery of Roaming Partner */

} stVPLMN;

/*
 * Tax Exemption Info from table TAX_EXEMPT
 */

typedef struct 
{
  int  lIndex;                 /* index in table */
  long lCustId;                /* CUSTOMER_ALL.CUSTOMER_ID attribute of exempted customer */
  long lTaxTypeId;             /* Type of tax */
  char szExpirationDate[16];   /* Date of exemption expiry */

} stTaxExempt;

/*
 * Tariff Zone Info from table MPUZNTAB
 */

typedef struct 
{
  BOOL enIsKey;
  int  lIndex;                 /* index in table */
  long lTariffZoneCode;        /* code of TT */
  char szDes[32];              /* Description of TT */
  char szShdes[8];             /* Short Description of TT */

} stTariffZone;

/*
 * Tariff Time Info from table MPUTTAB
 */

typedef struct 
{
  int  lIndex;                 /* index in table */
  long lTariffTimeCode;        /* code of TT */
  char szDes[32];              /* Description of TT */
  char szShdes[16];            /* Short Description of TT */

} stTariffTime;

/*
 * Service Info from table MPUSNTAB
 */

typedef struct 
{
  int lIndex;                 /* index in table */
  long lServiceNameCode;      /* code of SN */
  char szDes[32];             /* Description of SN */
  char szShdes[8];            /* Short Description of SN */
  long lMult;                 /* Multiplicator for number of usages printed in enclosure */
  long lCFMult;               /* Multiplicator for number of CF usages fro TELEF service printed in enclosure */
    /* the following members might be of toenBool type, but it requires the change of the order of the include
                                 files (bgh.h and types.h). Checking, whether is this possible is cumbersome...*/
  short soenWasVPNCall;      
  short soenWasNormalCall;
  short soenWasVPNCFCall;
  short soenWasNormalCFCall;

} stServiceName;

/*
 * Tariff Modell Info from table MPUTMTAB
 */

typedef struct 
{
  int lIndex;                 /* index in table */
  long lTariffModelCode;      /* code of TM */
  long lTariffModelVersion;   /* version of TM */
  char szDes[32];             /* Description of TM */
  char szShdes[8];            /* Short Description of TM */

} stTariffModel;

/*
 * Service Package Info from table MPUSPTAB
 */

typedef struct 
{
  int lIndex;                 /* index in table */
  long lServicePackageCode;   /* code of SP */
  char szDes[32];             /* Description of TM */
  char szShdes[8];            /* Short Description of TM */

} stServicePackage;


typedef struct tostMarket
{
  int soiIndex;
  int soiPLMNId;
  int soiMarketId;
  char sasnzMarketDes[60];
  char soszNetworkName[13];

} tostMarket; 

/*
 * Hash Item for SHDES 2 DES mappings
 */

#define HASH_TAB_SIZE 3
#define SHDES2HASH(s) (s[0] + (s[1] << 8) + (s[2] << 16) + (s[3] << 24) + (s[4] << 32) + (s[5] << 40))

typedef struct
{
  long lHashVal;              /* hash value */
  int  lIndex;                /* index of element in a table */

} stHashItem;

/*
 * Hash table 
 */

typedef struct
{
  int soiNextItem;
  stHashItem sastHashItem[HASH_TAB_SIZE];
  
} stHashTable;

/*
 * Special Number Info from table MPUPNTAB
 */

typedef struct 
{
  int lIndex;
  char szDigits[32];
  
} stPN;

/*
 * Container for Address Info
 */

typedef struct 
{
  char sachzLine1[64];
  char sachzLine2[64];
  char sachzLine3[64];
  char sachzLine4[64];
  char sachzLine5[64];
  char sachzZip[16];
  
} tostAddress;

/*
 * Contract Info
 */

typedef struct 
{
  int  soiIndex;
  char sasnzCustcode[32];
  char sasnzCCFName[64];
  char sasnzCCLName[64];
  int  soiCoId;
  char sasnzCoActivated[8];
  char sasnzSubmShdes[8];
  char sasnzSubmDes[32];
  char sasnzDnNum[32];
  char sasnzCdSmNum[64];

} tostCustSim;

/*
 * Price Group of Customer from table PRICEGROUP_ALL
 */

#define PRICEGROUP_CODE_SIZE 16
#define PRICEGROUP_NAME_SIZE 32

typedef struct
{
  long soilIndex;
  char sasnzCode[PRICEGROUP_CODE_SIZE];
  long soilSeqNo;
  char sasnzName[PRICEGROUP_NAME_SIZE];
  
} tostPriceGroup;

/*
 * Payment Type from table PAYMENTTYPE_ALL
 */

#define PAYMENT_NAME_LEN 32

typedef struct tostPaymentType
{
  long soilIndex;
  int soiPaymentId;
  char sochPaymentCode;
  char sasnzPaymentName[PAYMENT_NAME_LEN];

} tostPaymentType;

typedef enum {XGF, TEX} GEN_TYPE;

/*
 * BILL_MEDIUM
 */

#define BILL_MEDIUM_DES_LEN      64
#define BILL_MEDIUM_DEFAULT_LEN  8
#define DEFAULT_BILL_MEDIUM_CODE 'X'
#define DEFAULT_BILL_MEDIUM      1
#define EMPTY_BILL_MEDIUM        -1
#define NEGATIVE_BILL_MEDIUM     -2

typedef struct tostBillMedium
{
  long soilIndex;
  int  soiId;
  char sasnzDes     [BILL_MEDIUM_DES_LEN];
  char sasnzDefault [BILL_MEDIUM_DEFAULT_LEN];

} tostBillMedium;

/* structure with global information */
typedef struct 
{
  BOOL      bCompressImage;
  long      soiSettlingPeriod;
  long      lSetNo;
  long      lCustSetSize;        /* how many customers in one file */
  long      lCustNo;             /* which customer is actually processed */
  TYPEID    enTimmProcessing;    /* Which TIMM is the main type */
  BOOL          bProcFile;              /* TRUE - process file */
  BOOL      bSlipGen;
  BOOL      bAccEncGen; 
  BOOL      bSimEncGen;
  BOOL      bWelcomeGen;
  BOOL      bDunningGen;
  BOOL      bINPGen;
  BOOL      bINLGen;
  BOOL      bDebugMode;
  BOOL      bInsertHeader;
  BOOL      bListTimmDocuments;   
  BOOL      bWriteBillImage;     /* copy to BILL_IMAGES */
  BOOL      bSuppressEmptyList;
  BOOL      bShowZeroSum;
  BOOL      bUseThreshold;
  BOOL      bInsertComission;
  char          *pszFileName;           /* file to process */
  BOOL          bAllCust;               /* process complete document_all */
  BOOL      bEnclosureGen;  /* do we generate enclosure ? */ 
  GEN_TYPE  enOutputType; /* XGF or TeX */
  int              iNrCust;             /* process number of customers */
  BOOL          bShowTimm;              /* show contents of TIMM message */
  long          lCustId;                /* customer Id for single cutomer */
  char          *pszOutFile;            /* output file name */
  LOGLEVEL      enLogLevel;             /* logging level */
  BOOL          bForceEnglish;          /* force english document language */
  char      szBaseDirectory[PATH_MAX]; /* origin point for used directories */
  char      szBIProcessProgram[PATH_MAX]; /* complete path & name for bill img proc. program */
  char      szBIZProcessProgram[PATH_MAX]; /* complete path & name for bill zipped img proc. program */
  char          szImageDir[PATH_MAX];   /* image directory */
  char          szLayoutDir[PATH_MAX];  /* layout directory */
  char          szMetaDir[PATH_MAX];    /* metalanguage directory */
  char          szPrintDir[PATH_MAX];   /* print file directory */
  char          szNoPrintDir[PATH_MAX]; /* view file directory */
  char          szLogDir[PATH_MAX];     /* log file directory */
  char      szHeaderPath[PATH_MAX]; /* path of header file for actual TIMM message */
  char      szBRECode[16];        /* internal BRE code for a company */  
  char      szTmpDir[PATH_MAX];
  char      szEndDate[16];
  double    soflItbTaxRate;
  double    sofoThresholdValue;                   /* if > 0.0 thedetailed list is not printed */
  BOOL      soContractCallDetailListExist;        /* whether there was a detaled list for the contract */
  BOOL      soSubscriberCallDetailListPrinted;      /* whether there was a detaled list printed for the subscriber */
#ifdef BSCS4
  BOOL          fDbBscs4;               /* use new tables and columns of BSCS4 databases */
#endif
  long      pad;
  BOOL      soenLogToScreen;
  char      sochLoyalReportLevel;
  
} stBGHGLOB;

typedef struct tostBGHStat
{
  int soiInvProcessed;
  int soiInvRejected; 

} tostBGHStat;  

/* 
 * Function macros              
*/

#define macErrorMessage(a,b,c) fovdErrorMessage(__FILE__,__LINE__,(enum ERROR_NUMBERS)(a),(b),(c))

/* 
 * definition of macro ASSERT 
*/

#ifdef DEBUG

# define _myassert(ex)  {if(!(ex)){(void)fprintf(stderr,"ASSERT (%s) failed in file \"%s\", line %d\n", #ex, __FILE__, __LINE__);abort();}}
#  define ASSERT(par)   _myassert (par)

#else

#  define ASSERT(par)

#endif

#endif /* ifndef BGH_H */

#define STATUS_OK     0  /* alles ist ganz OK */
#define STATUS_ERROR  -1 /* stop processing   */
#define STATUS_REJECT -2 /* reject document   */



