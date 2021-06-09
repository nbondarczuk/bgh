/*************************************************************/
/*                                                           */
/* LH-Specification GmbH 1996                                */
/* All rights reserved.                                      */
/* Copying of this software or parts of this software is a   */
/* violation of German and International laws and will be    */
/* prosecuted.                                               */
/*                                                           */
/* PROJECT  : BGH                                            */
/*                                                           */
/* FILE     : bgh_access.c                                   */
/*                                                           */
/* AUTHOR(S): Trajan Rotaru                                  */
/*                                                           */
/* CREATED  : Mar. 1996                                      */
/*                                                           */
/* MODIFIED :                                                */
/* 15-07-96 B. Michler:	Type id for main cursor, 	     */
/*			CloseDatabase and CloseCursor now    */
/*			two different procedures	     */
/*                                                           */
/* ABSTRACT : C-Interface for the embedded-SQL-routines      */
/*                                                           */
/* DESCRIPTION :                                             */
/*                                                           */
/* This module is the central module for database access     */
/* of the BGH.                                               */
/*                                                           */
/* ROUTINES DEFINED HERE:                                    */
/*                                                           */
/*              PrintVersInfoBghAccess()                     */
/*                       CloseDatabase()                     */
/*                    GetBaseDirectory()                     */
/*              InitAndConnectDatabase()                     */
/*               AccessDatabaseAndTest()                     */
/*          AccessDatabaseAndResetTest()                     */
/*    AccessDatabaseAndReadVersionTest()                     */
/*  AccessDatabaseAndReadBaseDirectory()                     */
/* AccessDatabaseAndReadProcessProgram()                     */
/*                                                           */
/* CALLED SUBROUTINES :                                      */
/*                                                           */
/*                   GetPassword() - libcommon               */
/*                    GetConnect() - libcommon               */
/*        InitAndConnectDatabase()                           */
/*             foiHandleDatabase() - embedded-SQL            */
/*           GetTestFromDatabase() - embedded-SQL            */
/*        GetVersionFromDatabase() - embedded-SQL            */
/*  GetBaseDirectoryFromDatabase() - embedded-SQL            */
/* GetProcessProgramFromDatabase() - embedded-SQL            */
/*     GetCheckBatchFromDatabase() - embedded-SQL            */ 
/*                 foiCommitWork() - embedded-SQL            */
/*                   foiRollback() - embedded-SQL            */
/*                     foiEraseX() - embedded-SQL            */
/*                                                           */
/*                                                           */
/* RETURN CODES :                                            */
/*                                                           */
/*   0 : The function worked properly                        */
/*   X : Error codes delivered by subroutines                */
/*                                                           */
/*************************************************************/

#if 0	/* just for version.sh */
static char *SCCS_VERSION = "1.19";
#endif


#include <stdio.h>           /* Declarations and defini-     */
                             /* tions for standard I/O.      */
#include <string.h>          /* used for strncmp, memset     */
#include <fcntl.h>           /* files                        */
#include <unistd.h>          /* close, write                 */
#include <errno.h>           /* strerror                     */
#include <stdlib.h>

#include "bgh.h"             /* Declarations and defines     */
                             /* common for BGH.              */
#include "bgh_esql.h"
#include "protos.h"          /* definition and description   */
                             /* of used programs             */
#include "com_login.h"       /* definitions for DB-login     */
                             /* libcommon for BGH.           */
#include "types.h"

#ifdef _GRANT_
#include "grant.h"
#endif                  
           
/*************************************************************/
/* Extern variables                                          */
/*************************************************************/

extern   stBGHGLOB stBgh;

/*************************************************************/
/* Global variables                                          */
/*************************************************************/
#ifdef BSCS4
stDOCTYPES	*pstDocTypes;		/* pointer to array of structures */
stIMGTYPES	*pstImgTypes;		/* pointer to array of structures */
stIMGTYPESLV	*pstImgTypLevel;       	/* pointer to array of structures */
stIMGLNKDOC	*pstImgLnk;		/* pointer to array of structures */
stCUSTIMAGES	*pstCustImg;		/* pointer to array of structures */
#endif

static    char message[ MAX_BUF ];

/*************************************************************/
/* Prototypes                                                */
/*************************************************************/
int foiHandleDatabase (HANDLE, char *, char *);
int foiEraseX (void);
int GetVersionFromDatabase(char **);
int GetBaseDirectoryFromDatabase(void);
int GetProcessProgramFromDatabase(void);
int GetCheckBatchFromDatabase(BOOL *);
toenBool foenGen_Init();
int foiGrant_LoadConfig();

/******************************************************************************
 * PrintVersInfoBghAccess
 *
 * DESCRIPTION:
 * print the version info of all BGH_ACCESS
 *
 * PARAMETERS:
 *  void
 *
 * RETURNS:
 *  - none -
 ******************************************************************************
 */
void PrintVerInfoBghAccess (void)
{
  static char *SCCS_ID = "1.19.2.2";

  printf ("%s\t\t\t%s\n", __FILE__, SCCS_ID);
}

/*************************************************************/
/*                                                           */
/* InitAndConnectDatabase()                                  */
/*                                                           */
/* ABSTRACT: Reads database-configuration and opens it.      */
/*                                                           */
/* DESCRIPTION: Reads the context database settings and      */
/*              opens it.                                    */
/*                                                           */
/* CALLED SUBROUTINES:                                       */
/*                                                           */
/*                GetPassword() - libcommon                  */
/*                 GetConnect() - libcommon                  */
/*          foiHandleDatabase() - embedded-SQL               */
/*                                                           */
/* RETURN CODES:                                             */
/*                                                           */
/*   0 : The fuction worked properly                         */
/*   ERRORS                                                  */
/*   ACC_NOPASSWORD     : no corresponding password          */
/*   ACC_NOCONNECTSTRING: no connect-string for database     */
/*   X : Error codes delivered by subroutines                */
/*                                                           */
/*************************************************************/

int InitAndConnectDatabase(void)
{
  int   rc;
  char *char_ptr;

  fovdPushFunctionName ("InitAndConnectDatabase");

  rc = 0;
  char_ptr = NULL;

  /* Settings for connecting to database */
  strcpy(bgh_username, PRG_NAME);   /* program-name BGH is username too */ 

  /* Get the password of the module */
  char_ptr = GetPassword(bgh_username);
  if (char_ptr != NULL)
  {
    strcpy(bgh_password, char_ptr);
    char_ptr = GetConnect(bgh_username, bgh_password);
    if (char_ptr != NULL)
      {
        strcpy(connect_string, char_ptr);
        if (strlen(connect_string) < 1)
          {
            rc = (int) ACC_NOCONNECTSTRING;
            sprintf(message, "InitAndConnectDatabase: No connect-string for database exists");
            macErrorMessage(rc, CRITICAL, message);
          }
      }
    else
      {
        rc = (int) ACC_NOCONNECTSTRING;
        sprintf(message, "InitAndConnectDatabase: No connect-string for database exists");
        macErrorMessage(rc, CRITICAL, message);
      }
  }
  else
  {
    rc = (int) ACC_NOPASSWORD;
    sprintf(message, "InitAndConnectDatabase: No password for BGH-module exists");
    macErrorMessage(rc, CRITICAL, message);
  }

  if (rc == 0)
  {
    rc = foiHandleDatabase(connect, bgh_username, bgh_password);
  }

  fovdPopFunctionName ();
  return (rc);
}


/*************************************************************/
/*                                                           */
/* CloseDatabase()                                           */
/*                                                           */
/* ABSTRACT: Closes the database.                            */
/*                                                           */
/* DESCRIPTION: Closes the context database.                 */
/*                                                           */
/* CALLED SUBROUTINES:                                       */
/*                                                           */
/*          foiHandleDatabase() - embedded-SQL               */
/*                                                           */
/* RETURN CODES:                                             */
/*                                                           */
/*   0 : The fuction worked properly                         */
/*   ERRORS                                                  */
/*   X : Error codes delivered by subroutines                */
/*                                                           */
/*************************************************************/

int CloseDatabase (void)
{
  /* database will be closed  *******************/
  return (foiHandleDatabase(disconnect, "", ""));
}


/*************************************************************/
/*                                                           */
/* AccessDatabaseAndTest()                                   */
/*                                                           */
/* ABSTRACT: Reading database information.                   */
/*                                                           */
/* DESCRIPTION: Reading test-data from the database-tables   */
/*              'document_all' and 'doc_types'.              */
/*                                                           */
/* CALLED SUBROUTINES:                                       */
/*                                                           */
/*     InitAndConnectDatabase()                              */
/*        GetTestFromDatabase() - embedded-SQL               */
/*          foiHandleDatabase() - embedded-SQL               */
/*                                                           */
/* RETURN CODES:                                             */
/*                                                           */
/*   0 : The fuction worked properly                         */
/*   ERRORS                                                  */
/*   X : Error codes delivered by subroutines                */
/*                                                           */
/*************************************************************/

int AccessDatabaseAndTest(void)
{
  int rc;

  rc = GetTestFromDatabase();
  
  return (rc);
}

/*************************************************************/
/*                                                           */
/* AccessDatabaseAndReset()                                  */
/*                                                           */
/* ABSTRACT: Resets X-Marks for processed rows.              */
/*                                                           */
/* DESCRIPTION: Erases X-Marks in column 'processed_by_bgh'  */
/*              from the database-table 'document_all'.      */
/*                                                           */
/* CALLED SUBROUTINES:                                       */
/*                                                           */
/*     InitAndConnectDatabase()                              */
/*          foiHandleDatabase() - embedded-SQL               */
/*                  foiEraseX() - embedded-SQL               */
/*              foiCommitWork() - embedded-SQL               */
/*            foiRollbackWork() - embedded-SQL               */
/*                                                           */
/* RETURN CODES:                                             */
/*                                                           */
/*   0 : The fuction worked properly                         */
/*   ERRORS                                                  */
/*   X : Error codes delivered by subroutines                */
/*                                                           */
/*************************************************************/

int AccessDatabaseAndReset(void)
{
  int rc;

  rc  = 0;

  if (rc == 0)
  {
    rc = foiEraseX();
    if (rc == 0)
    {
      rc = foiCommitWork();
    }
    else if (rc == ESQL_NOX)
    {
      rc = 0; /* ESQL_NOX is a warning not a error */ 
    }
    else
    {
      (void) foiRollbackWork();
    }
  }

  return (rc);
}

/*************************************************************/
/*                                                           */
/* AccessDatabaseAndReadVersion()                            */
/*                                                           */
/* ABSTRACT: Gets version number for BGH                     */
/*                                                           */
/* DESCRIPTION: Reading the version number of the modul BGH  */
/*              from the database-table 'app_modules'.       */
/*                                                           */
/* CALLED SUBROUTINES:                                       */
/*                                                           */
/*     InitAndConnectDatabase()                              */
/*     GetVersionFromDatabase() - embedded-SQL               */
/*          foiHandleDatabase() - embedded-SQL               */
/*                                                           */
/* RETURN CODES:                                             */
/*                                                           */
/*   0 : The fuction worked properly                         */
/*   ERRORS                                                  */
/*   X : Error codes delivered by subroutines                */
/*                                                           */
/*************************************************************/

int AccessDatabaseAndReadVersion (char **pszVersionString)
{
  int 	rc;
  BOOL	bVCheckB;			/* check version of batch module */

  rc  = 0;
  *pszVersionString = NULL;

  if (rc == 0)
  {
    /*
     * first look in BSCSPROJECT_ALL if a check is to be done
     */
    rc = GetCheckBatchFromDatabase (&bVCheckB);

    if (rc == 0) {

      if (bVCheckB == TRUE) {

	/*
	 * checking is enabled -> read the version
	 */

	rc = GetVersionFromDatabase (pszVersionString);

      } else {
	/*
	 * no checking...
	 */
	rc = (int) ACC_NOVERSIONCHECK;
      }
    }
  }

  return (rc);
}

/*************************************************************/
/*                                                           */
/* AccessDatabaseAndReadBaseDirectory()                      */
/*                                                           */
/* ABSTRACT: Gets base directory for BGH                     */
/*                                                           */
/* DESCRIPTION: Reading the base directory of the modul BGH  */
/*              from the database-table 'mpscftab'.          */
/*                                                           */
/* CALLED SUBROUTINES:                                       */
/*                                                           */
/*     InitAndConnectDatabase()                              */
/*     GetBaseDirectoryFromDatabase() - embedded-SQL         */
/*     foiHandleDatabase()            - embedded-SQL         */
/*                                                           */
/* RETURN CODES:                                             */
/*                                                           */
/*   0 : The fuction worked properly                         */
/*   ERRORS                                                  */
/*   ACC_NOBASEDIR: no base-directory info in database       */
/*   X : Error codes delivered by subroutines                */
/*                                                           */
/*************************************************************/

int AccessDatabaseAndReadBaseDirectory (void)
{
  int 	rc;
  
  rc  = 0;
  
  memset(stBgh.szBaseDirectory, '\0', MAX_BUF); /* char-array initialisation */
  

  rc = GetBaseDirectoryFromDatabase ();
  
  if ((rc == ESQL_NOMOREDATA) || (rc == ESQL_NULLBASEDIR))
  {
	rc = ACC_NOBASEDIR;
        sprintf(message, "AccessDatabaseAndReadBaseDirectory: No base-directory info in database exists");
        macErrorMessage(rc, WARNING, message);
  }

  return (rc);
}

/*************************************************************/
/*                                                           */
/* AccessDatabaseAndReadProcessProgram()                     */
/*                                                           */
/* ABSTRACT: Gets bill-images-process-program                */
/*                                                           */
/* DESCRIPTION: Reading the bill-images-process-program      */
/*              from the database-table 'mpscftab'.          */
/*                                                           */
/* CALLED SUBROUTINES:                                       */
/*                                                           */
/*     InitAndConnectDatabase()                              */
/*    GetProcessProgramFromDatabase() - embedded-SQL         */
/*     foiHandleDatabase()            - embedded-SQL         */
/*                                                           */
/* RETURN CODES:                                             */
/*                                                           */
/*   0 : The fuction worked properly                         */
/*   ERRORS                                                  */
/*   ACC_NOPROCPROG: no base-directory info in database      */
/*   X : Error codes delivered by subroutines                */
/*                                                           */
/*************************************************************/

int AccessDatabaseAndReadProcessProgram (void)
{
  int 	rc;
  int 	ret;
  char path[PATH_MAX];
  toenBool loenStatus;

  rc  = 0;
  ret = 0;

  memset(stBgh.szBIProcessProgram, '\0', PATH_MAX); /* char-array initialisation */
  
  rc = GetProcessProgramFromDatabase ();
  
  if ((rc == ESQL_NOMOREDATA) || (rc == ESQL_NULLPROCPROG))
    {
      rc = ACC_NOPROCPROG;
      sprintf (message,
               "AccessDatabaseAndReadProcessProgram: No bill-images-process-program info in\n"
               "database exists, loading default!\n");
      macErrorMessage(rc, WARNING, message);
      fovdPrintLog (LOG_MAX, message);
      strcpy (stBgh.szBIProcessProgram, BI_IMAGE_PROCESS); 
    }

  strcpy(path, stBgh.szLayoutDir);
  strcat(path, "header_" GEN_VER "_" BGH_VER ".ps");  
  strcpy (stBgh.szHeaderPath, path); 
  
  if ((loenStatus = foenGen_Init()) == FALSE)
    {
      rc = ACC_NOPROCPROG;
      sprintf (message, "AccessDatabaseAndReadProcessProgram: Can't init document generator !\n");
      macErrorMessage(rc, WARNING, message);
      fovdPrintLog (LOG_MAX, message);
    }
  
#ifdef _NIC_
  if ((rc == ESQL_NOMOREDATA) || (rc == ESQL_NULLPROCPROG))
    {
      rc = ACC_NOPROCPROG;
      strcpy(path, stBgh.szLayoutDir);
      /*
      strcat(path, COMMON_HEADER_FILE);
      */
      stract(path, "header_" GEN_VAR "_" BGH_VER);
      sprintf (message,
               "AccessDatabaseAndReadProcessProgram: No header-path info in\n"
               "database exists, loading default common file %s!\n", path);
      macErrorMessage(rc, WARNING, message);
      fovdPrintLog (LOG_MAX, message);
      strcpy (stBgh.szHeaderPath, path); 
    }
#endif  

  return (rc);
}

#ifdef BSCS4
/******************************************************************************
 * fovdGetAddDB
 *
 * DESCRIPTION:
 * read the additional DB tables for nr of copies a.s.o.
 * read it only if it is called first and the database is processed.
 * It assumes that the database is open already.
 *
 * PARAMETERS:
 *  void
 *
 * RETURNS:
 *  - none -
 ******************************************************************************
 */

void fovdGetAddDB (void) 
{
  int rc;

  if (stBgh.bProcFile == FALSE) 
    {
      pstDocTypes = NULL;
      pstImgTypes = NULL;
      pstImgLnk   = NULL;
      pstCustImg  = NULL;
      
      if ((rc = GetDocTables ()) != 0) 
        {
          if (pstDocTypes != NULL) 
            {	
              free (pstDocTypes);
              pstDocTypes = NULL;
            }
      
          if (pstImgTypes != NULL) 
            {	
              free (pstImgTypes);
              pstImgTypes = NULL;
            }

          if (pstImgLnk != NULL) 
            {
              free (pstImgLnk);
              pstImgLnk   = NULL;
            }

          if (pstCustImg != NULL) 
            {
              free (pstCustImg);
              pstCustImg  = NULL;
            }
        }
    }

#ifdef _GRANT_
  if ((rc = foiGrant_LoadConfig()) != 0)
    {
      sprintf(message, "fovdGetAddDB: Can't init granting module");
      macErrorMessage(rc, CRITICAL, message);
    }
#endif
}
#endif


/* THE END ***************************************************/


