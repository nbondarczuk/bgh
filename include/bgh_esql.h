#ifndef BGH_ESQL                /* allow multiple inclusions    */
#define BGH_ESQL
/*************************************************************/
/*                                                           */
/* LH-Specification GmbH 1996                                */
/* All rights reserved.                                      */
/* Copying of this software or parts of this software is a   */
/* violation of German and International laws and will be    */
/* prosecuted.                                               */
/*                                                           */
/* PROJECT     : BGH                                         */
/*                                                           */
/* FILE        : bgh_esql.h                                  */
/*                                                           */
/* AUTHOR(S)   : Trajan Rotaru                               */
/*                                                           */
/* CREATED     : Mar. 1996                                   */
/*                                                           */
/* MODIFIED    :                                             */
/*                                                           */
/* ABSTRACT    : Defines  for embedded-SQL                   */
/*                                                           */
/* DESCRIPTION :                                             */
/*                                                           */
/* Contains global defines for Context Database.             */
/* Defines which are used only local, are normally defined   */
/* at the place they are used.                               */
/*                                                           */
/*************************************************************/

#if 0 /* just for version.sh */
static char *SCCS_VERSION = "1.12";
#endif

#include <stdio.h>


/*************************************************************/
/* Global Defines Miscellanious                              */
/*************************************************************/
#define CONST_DEFAULT_BLOB_SIZE 16384

#define LINE_SIZE           79

#ifndef MAX_BUF                 /* frequently used define    */
#define MAX_BUF           1024  /* potential conflict        */
#endif

#define DEF_DIM_STACK        6  /* default dimension         */ 
                                /* of rowid's-stack          */
#define ROWID_ELEM_SIZE     21  /* length of rowid-string    */

#define MAX_ATTEMPTS         5 /* maximum number of          */
                               /* rollback-attempts          */

#define LANG Polish_Poland.EE8ISO8859P2
/*
#define LANG Polish_Poland.EE8PC852
#define LANG Polish_Poland.EE8MSWIN1250
*/

/*************************************************************/
/* Global Defines used in context with the database          */
/*************************************************************/

#define PRG_NAME        "BGH" 
#define USERNAME        "BGH"
#define MODULE_BGH      "BGH"  /* used for version-number of BGH */

#define STATUS_PROCESSED   "X" /* used to mark whith X the field */
                               /* procesed_by_bgh for processed  */
                               /* records in table document_all  */

#define STATUS_PROBLEMS    "1" /* used to mark whith '1' the field */
                               /* procesed_by_bgh for processed    */
                               /* records in table document_all    */
                               /* when a customer-process return   */
                               /* a non-zero code.                 */
 
#define STATUS_ENABLED     'X' /* version check is enabled for   */
                               /* batch-modules in table         */
                               /* bscsproject_all                */

/* defines used to enter data in table bill_images */
#define BI_IMAGE_PROCESS_CFVAL   6001     /* cfvalue for bill-images-process-program in the table mpscftab */
#define BIZ_IMAGE_PROCESS_CFVAL  6003     /* cfvalue for zipped-bill-images-process-program in the table mpscftab */
#define ITB_TAX_RATE_CFCODE      6201  
#define BI_IMAGE_PROCESS  "ghostview"    /* "c:\\gs\\gsview.exe" */
#define BI_EXTENSION      "ps"
#define BIZ_EXTENSION     "psz"


/* defines header paths */
#define TEX_HEADER_INV_CFCODE 6100
#define XGF_HEADER_INV_CFCODE 6101
#define TEX_HEADER_ITB_CFCODE 6110
#define XGF_HEADER_ITB_CFCODE 6111
#define TEX_HEADER_WLL_CFCODE 6120
#define XGF_HEADER_WLL_CFCODE 6121
#define TEX_HEADER_DNL_CFCODE 6130
#define XGF_HEADER_DNL_CFCODE 6131
#define TEX_HEADER_INH_CFCODE 6140
#define XGF_HEADER_INH_CFCODE 6141

#define BRE_ID_CFCODE 6200
#define ARGS_CFCODE 6002


/* Enumeration for the handling of database and cursors      */
/* used in handle_cursor(handle, ...)                        */
/* and     handle_database(handle, ...)                      */
typedef enum {
    connect,
    disconnect,
    sql_open,
    sql_close
} HANDLE;

char connect_string[MAX_BUF];/* This data is used            */ 
char bgh_username[MAX_BUF];  /* for logging into database in */
char bgh_password[MAX_BUF];  /* module connect_database()    */

#endif
