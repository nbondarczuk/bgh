/******************************************************************************
 * LH-Specification GmbH 1995.
 *
 * All rights reserved.
 * Copying of this software or parts of this software is a violation of German
 * and International laws and will be prosecuted.
 *
 * Project  :   BGH
 *
 * This file contains the prototypes of all non-static functions of BGH
 *
 * File     :   protos.h
 * Created  :   Feb. 1996
 * Author(s):   B. Michler
 ******************************************************************************
 */

#ifndef PROTOS_H
#define PROTOS_H

#if 0 /* just for version.sh */
static char *SCCS_VERSION = "1.22";
#endif

/* ---- bgh_file.c ---- */
int  foiReadTimm (char *szFileName, char **pszTimmString);
void fovdPrintLog (LOGLEVEL enLogLevel, const char *parmN, ...);
int  foiOpenLog (char* ppszProcessNr);
void fovdCloseLog (void);
void fovdRemoveLog (void);
int  foiGetBaseName (char *szFileName, char *pszBaseName, unsigned int uBufLen);
int  foiBuildFileName (TYPEID, char *, char *, unsigned int);
int  foiGetDirectories (void);

/* ---- bgherrh.c ---- */
void fovdErrorMessage(char *poszSourceFile, long polLineNo, enum ERROR_NUMBERS polMsgNo,
                      enum ERROR_TYPES polSeverity, char *poszMessageText);
void fovdInitBGHErrh(void);
void fovdPushFunctionName(char *NamePtr);
void fovdPopFunctionName(void);
void fovdErrorPopFunctionName(void);
int  foiRegisterCallbackFunction (void (*) (void));
void PrintErrorInfo (enum ERROR_NUMBERS enErrNr);

/* ---- bgh_access.c ---- */
int AccessDatabaseAndProcess(void);
int AccessDatabaseAndTest(void);
int AccessDatabaseAndReset(void);
int AccessDatabaseAndReadVersion(char **);
int AccessDatabaseAndReadBaseDirectory(void);
int AccessDatabaseAndReadProcessProgram (void);
int InitAndConnectDatabase(void);
int CloseDatabase (void);
int OpenCursor (TYPECUST, TYPEID);
int CloseCursor (TYPECUST, TYPEID);
int CloseSecondCursor (TYPEID);
#ifdef BSCS4
void fovdGetAddDB (void);
#endif

/* ---- bgh_esql.pc ---- */
int fnOpenPrimaryCursor( TYPEID   usage_type, TYPECUST cust_process,   long poiBghId);
int GetTimmFromPrimaryCursor (char  **string_to_parse, /* address of pointer to BLOB-string */
                         int    *string_to_parse_len,
                         void  **string_to_free,  /* address of pointer to memory-block */
                         TYPEID  timm_type,       /* i.e. INV_TYPE */
                         BOOL   *end_indicator,   /* no more data */
                         long     customer_id,    /* for single_customer */
			 stDBCOL *stCol); 	  /* for database columns */
int fnOpenSecondaryCursor( TYPEID   usage_type);
int fnCloseSecondaryCursor( TYPEID   usage_type);
int GetTimmFromSecondaryCursor (char  **string_to_parse, /* address of pointer to BLOB-string */
                         int    *string_to_parse_len,
                         void  **string_to_free,  /* address of pointer to memory-block */
                         TYPEID  timm_type,       /* i.e. INV_TYPE */
                         BOOL   *end_indicator,   /* no more data */

			 stDBCOL *stCol); 	  /* for database columns */
int fnClosePrimaryCursor( TYPEID   usage_type,TYPECUST cust_process);

int GetTestFromDatabase (void);
int PutBillIntoDatabase(char *, TYPEID, stDBCOL *);
int MarkWithX (void);
int foiResetX (void);
int foiCommitWork (void);
int foiRollbackWork (void);
void FreeStack (void);
int foiMarkCustomer (char *);
#ifdef BSCS4
int GetDocTables (void);
#endif
int GetHeaderPathFromDatabase(void);
int GetTempAddressFromDatabase(int, tostAddress *);
int GetSocSecNoFromDatabase(int, char *);


/* ---- bgh_proc.c ---- */
int foiProcessAll (TYPEID,long poiBghId);
int loiProcessCustomer (TYPEID);
int foiGetSumsheet (stTIMM **, stDBCOL *);
int foiGetDunnings (stTIMM ***, int *, stDBCOL *);
int foiGetItemizedBills (stTIMM ***, unsigned int *, stDBCOL *);
int foiGetRoaming (stTIMM **, stDBCOL *);
int foiGetBalance (stTIMM **, stDBCOL *);
int foiGetLegend (stTIMM **, stDBCOL *);
int foiGetDuning (stTIMM **, stDBCOL *);
int foiGetWelcome (stTIMM **, stDBCOL *);
int foiGetTimmVersion (char *szTimm, char *szVersion, int iVerLen);

/* ---- bgh_bill.c ---- */
int foiGenerateBill (stTIMM *, stDBCOL *, TYPEID);

#endif /* ifndef PROTOS_H */

