/*******************************************************************************
 * LH-Specification GmbH 1995.
 *
 * All rights reserved.
 * Copying of this software or parts of this software is a violation of German
 * and International laws and will be prosecuted.
 *
 * Project  :   BGH
 *
 * File     :   bgh_file.c
 * Created  :   Feb. 1996
 * Author(s):   B. Michler
 *
 *
 * Modified :
 * 26.06.96	B. Michler	additional parameter of type stDBCOL * for
 * 				database access routines
 *
 *******************************************************************************/

#if 0	/* just for version.sh */
static char *SCCS_VERSION = "1.25";
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>              /* "errno" for logging                */
#include <time.h>               /* time information for log-file      */
#include <dirent.h>             /* reading files with wildcards       */
#include <unistd.h>		/* for 'access' */
#include <sys/stat.h>		/* for stat (writing of bill images)  */


/* User includes                */
#include "bgh.h"
#include "protos.h"


/* User defines                 */

#define DIR_BASE	"./"		/* default base directory */
#define DIR_SUBBGH	"BGH/"		/* default bgh subdirectory */
#define DIR_SUBLOG	"LOG/"		/* default log subdirectory */
#define DIR_SUBLAY	"LAYOUT/"	/* default layout subdirectory */
#define DIR_SUBMETA	"BILLS.OUT/"	/* default output subdir for metalanguage */
#define DIR_SUBPRINT	"BILLS.OUT/"	/* default output subdir for printfiles */
#define DIR_IMAGE    "IMAGE/"


#define FILESUFFIX ".etis"
#define FILEASTERISK ".*"

#define DEFIMAGELEN     10240           /* default image length */


/* Function macros              */


/* Enumerations                 */


/* Local typedefs               */


/* External globals             */
extern char     szStartUp[];
extern stBGHGLOB stBgh;		/* bgh globals */
extern char     *SCCS_LONGVERSION;


/* Globals                      */


/* Static globals               */
static char szTempName[PATH_MAX];
static char szPathName[PATH_MAX];
static FILE *fiBghLog = NULL;           /* file handle of log-file */
static DIR  *pDir = NULL;               /* for GetFirst, GetNext   */
static char szTemp[PATH_MAX+128];
static char szLogName[PATH_MAX];	/* basename of logfile */

struct stTYPNAM {
  TYPEID    enMsg;      /* message type */
  BOOL      fMulti;     /* build a filename like 'ITBxxxxx.*.etis', if TRUE */
  char      *szName;    /* beginning of filename */
} stTypeName[] = {
  {INV_TYPE, FALSE,     "INV"},
  {ITB_TYPE, TRUE,      "ITB"},
  {SUM_TYPE, FALSE,     "SUM"},
  {ROA_TYPE, FALSE,     "ROA"},
  {LGN_TYPE, FALSE,     "LGN"},
  {BAL_TYPE, FALSE,     "BAL"},
  {INH_INP,  FALSE,     "INP"},
  {INH_INL, FALSE,     "INL"}
};

/* month names */
char *pszMonth[] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun", 
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

/* Static function prototypes   */
int foiReopenLog (void);
static int bgh_access(const char *, int);

/******************************************************************************
 * PrintVersInfoBghFile
 *
 * DESCRIPTION:
 * print the version info of all BGH_FILE
 *
 * PARAMETERS:
 *  void
 *
 * RETURNS:
 *  - none -
 ******************************************************************************
 */
void PrintVerInfoBghFile (void)
{
  static char *SCCS_ID = "1.25.2.2";

  printf ("%s\t\t\t%s\n", __FILE__, SCCS_ID);
}


/******************************************************************************
 * foiGetDirectories
 *
 * DESCRIPTION:
 * sets the different directories
 * reads the base directory from MPSCFTAB, entry 17; if there is none
 * set it takes the current directory ("./") as default.
 * The tree looks like:
 *	{MPSCFTAB.17}-+-DIR_SUBBGH-+-DIR_SUBMETA
 *                    |            |
 *                    |            +-DIR_SUBPRINT
 *                    |            |
 *                    |            +-DIR_SUBLAY
 *                    |
 *                    +-DIR_SUBLOG
 *
 * The directories DIR_SUBBGH, DIR_SUBLOG, and DIR_SUBPRINT are created if they
 * do not exist (with access rights for user and group), directory DIR_SUBLAY
 * is not created, because it must contain the layout descriptions for BGH.
 *
 * PARAMETERS:
 * - none -
 *
 * RETURNS:
 *  0 if succesful
 ******************************************************************************
 */
int foiGetDirectories (void)
{
  int 		iRetCode;	    	/* return code */
  unsigned int	uLen;			/* length of path */
  
  iRetCode = 0;

  /*
   * get the basedirectory, if no customer selected should be taken
   */
  if (stBgh.szBaseDirectory[0] == '\0') 
    {
      iRetCode = AccessDatabaseAndReadBaseDirectory ();

      if (iRetCode != (int) NO_ERROR) 
        {
          fovdPrintLog (LOG_MAX, "Couldn't read base directory, taking current as default!\n");
          strcpy (stBgh.szBaseDirectory, DIR_BASE);
        }
    }
  /*
   * now set the various subdirectories
   */
  uLen = strlen (stBgh.szBaseDirectory);
  if (stBgh.szBaseDirectory[uLen - 1] != '/') 
    {
      /* add a '/' at the end if its missing */
      strcat (stBgh.szBaseDirectory, "/");
    }

  /* check access for base directory */
  if (bgh_access (stBgh.szBaseDirectory, R_OK) < 0) 
    {
      sprintf (szTemp, "No access to base directory [%s]", stBgh.szBaseDirectory);
      macErrorMessage (FILE_NOACCESS, WARNING, szTemp);
      iRetCode = (int) FILE_NOACCESS;
      return (iRetCode);
    }

  strcpy(stBgh.szTmpDir, stBgh.szBaseDirectory);
  strcat(stBgh.szTmpDir, "TMP/");

  strcpy (szTempName, stBgh.szBaseDirectory);
  strcat (szTempName, DIR_SUBLOG); 		/* Log-file directory */
  uLen = strlen (szTempName);
  if (szTempName[uLen - 1] != '/') 
    {
      /* add a '/' at the end if its missing */
      strcat (szTempName, "/");
    }
  /* check access for log directory */
  if (bgh_access (szTempName, W_OK) < 0) 
    {
      sprintf (szTemp, "No access to log directory [%s]"
#ifdef MAKEDIRS
               ", making it"
#endif
               , szTempName);
      macErrorMessage (FILE_NOACCESS, WARNING, szTemp);

#ifdef MAKEDIRS
      /* try to make directory */
      if (mkdir (szTempName, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP) != 0)
        {
          sprintf (szTemp, "Failed to create log directory [%s]", szTempName);
          macErrorMessage (FILE_NOACCESS, WARNING, szTemp);
          iRetCode = (int) FILE_NOACCESS;
        } 
      else 
        {
          /* open final log file and copy temporary */
          strcpy (stBgh.szLogDir, szTempName);
          (void) foiReopenLog ();
        }
#else
      iRetCode = (int) FILE_NOACCESS;
#endif
    } 
  else 
    {
      /* open final log file and copy temporary */
      strcpy (stBgh.szLogDir, szTempName);
      (void) foiReopenLog ();
    }

  /* BGH Base directory is base directory + 'BGH' */
  strcat (stBgh.szBaseDirectory, DIR_SUBBGH);
  uLen = strlen (stBgh.szBaseDirectory);
  if (stBgh.szBaseDirectory[uLen - 1] != '/') 
    {
      /* add a '/' at the end if its missing */
      strcat (stBgh.szBaseDirectory, "/");
    }
  if (bgh_access (stBgh.szBaseDirectory, R_OK) < 0) 
    {
      sprintf (szTemp, "No access to BGH base directory [%s]"
#ifdef MAKEDIRS
               ", making it"
#endif
               , stBgh.szBaseDirectory);
      macErrorMessage (FILE_NOACCESS, WARNING, szTemp);

#ifdef MAKEDIRS
      /* try to make directory */
      if (mkdir (stBgh.szBaseDirectory, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP) != 0)
        {
          sprintf (szTemp, "Failed to create BGH base directory [%s]", stBgh.szBaseDirectory);
          macErrorMessage (FILE_NOACCESS, WARNING, szTemp);
          iRetCode = (int) FILE_NOACCESS;
          return (iRetCode);
        }
#else
      iRetCode = (int) FILE_NOACCESS;
      return (iRetCode);
#endif
    }

  /* set image directory */
  strcpy (stBgh.szImageDir, stBgh.szBaseDirectory);
  strcat (stBgh.szImageDir, DIR_IMAGE); 	/* image directory */
  uLen = strlen (stBgh.szImageDir);
  if (stBgh.szImageDir[uLen - 1] != '/') 
    {
      /* add a '/' at the end if its missing */
      strcat (stBgh.szImageDir, "/");
    }
  /* check access for image directory */
  if (bgh_access (stBgh.szImageDir, R_OK) < 0) 
    {
      sprintf (szTemp, "No access to image directory [%s]", stBgh.szImageDir);
      macErrorMessage (FILE_NOACCESS, WARNING, szTemp);
      iRetCode = (int) FILE_NOACCESS;
      sprintf (szTemp, "No access to image directory [%s]"
#ifdef MAKEDIRS
               ", making it"
#endif
               , stBgh.szImageDir);
      macErrorMessage (FILE_NOACCESS, WARNING, szTemp);
      
#ifdef MAKEDIRS
      /* try to make directory */
      if (mkdir (stBgh.szImageDir, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP) != 0)
        {
          sprintf (szTemp, "Failed to create image directory [%s]", stBgh.szImageDir);
          macErrorMessage (FILE_NOACCESS, WARNING, szTemp);
          iRetCode = (int) FILE_NOACCESS;
        }
#else
      iRetCode = (int) FILE_NOACCESS;
#endif
    }
   
  /* set layout directory */
  strcpy (stBgh.szLayoutDir, stBgh.szBaseDirectory);
  strcat (stBgh.szLayoutDir, DIR_SUBLAY); 	/* layout directory */
  uLen = strlen (stBgh.szLayoutDir);
  if (stBgh.szLayoutDir[uLen - 1] != '/') 
    {
      /* add a '/' at the end if its missing */
      strcat (stBgh.szLayoutDir, "/");
    }
  /* check access for layout directory */
  if (bgh_access (stBgh.szLayoutDir, R_OK) < 0) 
    {
      sprintf (szTemp, "No access to layout directory [%s]", stBgh.szLayoutDir);
      macErrorMessage (FILE_NOACCESS, WARNING, szTemp);
      iRetCode = (int) FILE_NOACCESS;
    }
  
  /* set meta (PS) directory */
  strcpy (stBgh.szMetaDir, stBgh.szBaseDirectory);
  strcat (stBgh.szMetaDir, DIR_SUBMETA); 	/* metalanguage directory */
  uLen = strlen (stBgh.szMetaDir);
  if (stBgh.szMetaDir[uLen - 1] != '/') 
    {
      /* add a '/' at the end if its missing */
      strcat (stBgh.szMetaDir, "/");
    }
  /* check access for meta directory */
  if (bgh_access (stBgh.szMetaDir, W_OK) < 0) 
    {
      sprintf (szTemp, "No access to meta directory [%s]"
#ifdef MAKEDIRS
               ", making it"
#endif
               , stBgh.szMetaDir);
      macErrorMessage (FILE_NOACCESS, WARNING, szTemp);
      
#ifdef MAKEDIRS
      /* try to make directory */
      if (mkdir (stBgh.szMetaDir, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP) != 0)
        {
          sprintf (szTemp, "Failed to create meta directory [%s]", stBgh.szMetaDir);
          macErrorMessage (FILE_NOACCESS, WARNING, szTemp);
          iRetCode = (int) FILE_NOACCESS;
        }
#else
      iRetCode = (int) FILE_NOACCESS;
#endif
    }
    
  strcpy (stBgh.szPrintDir, stBgh.szBaseDirectory);
  strcat (stBgh.szPrintDir, DIR_SUBPRINT); 	/* print-file directory */
  uLen = strlen (stBgh.szPrintDir);
  if (stBgh.szPrintDir[uLen - 1] != '/') 
    {
      /* add a '/' at the end if its missing */
      strcat (stBgh.szPrintDir, "/");
    }
  /* check access for print directory */
  if (bgh_access (stBgh.szPrintDir, W_OK) < 0) 
    {
      sprintf (szTemp, "No access to print directory [%s]"
#ifdef MAKEDIRS
               ", making it"
#endif
               , stBgh.szPrintDir);
      macErrorMessage (FILE_NOACCESS, WARNING, szTemp);
      
#ifdef MAKEDIRS
      /* try to make directory */
      if (mkdir (stBgh.szPrintDir, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP) != 0)
        {
          sprintf (szTemp, "Failed to create print directory [%s]", stBgh.szPrintDir);
          macErrorMessage (FILE_NOACCESS, WARNING, szTemp);
          iRetCode = (int) FILE_NOACCESS;
        }
#else
      iRetCode = (int) FILE_NOACCESS;
#endif
    }
  
  return (iRetCode);
}

/******************************************************************************
 * foiWriteFileToDb
 *
 * DESCRIPTION:
 * loads the file into a string and writes it to bill_images
 *
 * PARAMETERS:
 *  char    *szFileName         - filename
 *  TYPEID  type		- type of document
 *  stDBCOL *stColumn		- pointer to additional columns structure
 *
 * RETURNS:
 *  0 if succesful
 ******************************************************************************
 */
int foiWriteFileToDb (char *szFileName, TYPEID type, stDBCOL *stColumn)
{
  FILE          *fiImage;
  unsigned int  uActLen;
  char          *pImage;
  int		iRet;
  struct stat   statBuf;

  fovdPushFunctionName ("foiWriteFileToDb");

  iRet = 0;

  /* do nothing if we read from file */
  if (stBgh.bProcFile == TRUE) {
    fovdPopFunctionName ();
    return (iRet);
  }

  /* get information about file - I need the length */
  iRet = stat (szFileName, &statBuf);

  if (iRet != 0)
  {
    /* stat didn't work */
    if (sizeof (szTemp) < (strlen (szFileName) + 40)) {
      sprintf (szTemp, "WRITE IMAGE: cannot open image file \"%s\"!", szFileName);
    } else {
      sprintf (szTemp, "WRITE IMAGE: cannot open image file!");
    }
    macErrorMessage (FILE_OPEN, WARNING,  szTemp);
    fovdPopFunctionName ();
    return ((int) FILE_OPEN);
  }

  /*
   * some prerequisites: get a buffer for the file
   * and open the file
   */
  uActLen = statBuf.st_size;
  pImage = (char *) malloc (uActLen + 1);

  if (pImage == NULL) {
    macErrorMessage (FILE_MALLOC, CRITICAL,
                     "WRITE IMAGE: no memory for Bill Image string!");
    fovdPopFunctionName ();
    return ((int) FILE_MALLOC);
  }

  fiImage = fopen (szFileName, "r");
  if (fiImage == NULL) {
    free (pImage);
    if (sizeof (szTemp) < (strlen (szFileName) + 40)) {
      sprintf (szTemp, "WRITE IMAGE: cannot open image file \"%s\"!", szFileName);
    } else {
      sprintf (szTemp, "WRITE IMAGE: cannot open image file!");
    }
    macErrorMessage (FILE_OPEN, WARNING,  szTemp);
    fovdPopFunctionName ();
    return ((int) FILE_OPEN);
  }

  fovdPrintLog (LOG_TIMM, "WRITE IMAGE: reading (%d) \"%s\"\n", uActLen, szFileName);

  /*
   * now read the complete file
   */
  uActLen = fread ((void *) pImage, sizeof (char), uActLen, fiImage);

  /*
   * now the complete image is in pImage, the length is uActLen;
   * write it to the database bill_images
   */
  pImage[uActLen] = '\0';

  iRet = PutBillIntoDatabase (pImage, type, stColumn);

  if (pImage != NULL) {
    free (pImage);
  }

  if (fclose (fiImage) != 0) {
    macErrorMessage (FILE_CLOSE, WARNING, 
                     "WRITE IMAGE: couldn't close file!");
    fovdPopFunctionName ();
    return ((int) FILE_CLOSE);
  }
  fovdPopFunctionName ();
  return (iRet);
}


/******************************************************************************
 * foiReadTimm
 *
 * DESCRIPTION:
 * reads a TIMM-message from a file
 *
 * PARAMETERS:
 *  char    *szFileName     - name of the file
 *  char    **pszTimm       - return pointer for TIMM-message
 *
 * RETURNS:
 *  0 if successful
 ******************************************************************************
 */
int foiReadTimm (char *szFileName, char **pszTimmString)
{
    unsigned int     uStringLen;
    FILE    *fiTimm;
    long    i;
    int     c;

    fovdPushFunctionName ("foiReadTimm");

    /*
     * some prerequisites: get a buffer for the TIMM-string
     * and open the file
     */
    uStringLen = DEFTIMMLEN;
    *pszTimmString = (char *) malloc (uStringLen);
    if (*pszTimmString == NULL) {
      macErrorMessage (FILE_MALLOC, CRITICAL, 
                       "no memory for TIMM-string!");
      fovdPopFunctionName ();
      return ((int) FILE_MALLOC);
    }

    fiTimm = fopen (szFileName, "r");
    if (fiTimm == NULL) {
      free (*pszTimmString);
      *pszTimmString = NULL;
      sprintf (szTemp, "cannot open TIMM-file \"%s\"!", szFileName);
      macErrorMessage (FILE_OPEN, WARNING,  szTemp);
      fovdPopFunctionName ();
      return ((int) FILE_OPEN);
    }

    fovdPrintLog (LOG_TIMM, "reading \"%s\"\n", szFileName);

    i = 0;
    /*
     * now read the complete file, if the buffer
     * becomes too small, reallocate doubling size
     */
    while ((c = fgetc (fiTimm)) != EOF) {
      (*pszTimmString)[i++] = (char) c;
      
      if (i >= uStringLen) {
        /*
         * line gets too long - get more space for it
         */
        uStringLen *= 2;
        *pszTimmString = (char *) realloc (*pszTimmString, uStringLen);

        if (*pszTimmString == NULL) {
          macErrorMessage (FILE_MALLOC, CRITICAL, 
                           "no memory for TIMM-string!");
          fclose (fiTimm);
          fovdPopFunctionName ();
          return ((int) FILE_MALLOC);
        }
      }
    } /* while */

    (*pszTimmString)[i++] = '\0';

    if (fclose (fiTimm) != 0) {
      macErrorMessage (FILE_CLOSE, WARNING, 
                       "couldn't close file!");
      fovdPopFunctionName ();
      return ((int) FILE_CLOSE);
    }
    fovdPopFunctionName ();
    return (0);
}


/******************************************************************************
 * lopszGetFileNameFromPath
 *
 * DESCRIPTION:
 * returns pointer to the filename in the given path
 *
 * PARAMETERS:
 *  char            *szFileName     - name of the file (including path)
 *
 * RETURNS:
 *  char *      - pointer to filename
 ******************************************************************************
 */
static char *lopszGetFileNameFromPath (char *szFileName)
{
  char *pRet;

  /*
   * search the beginning of the filename in the path
   */
  pRet = szFileName + strlen (szFileName);
  while (pRet != szFileName) {
    if (*pRet == '/') {
      pRet++;
      break;
    }
    pRet--;
  }
  return (pRet);
}


/******************************************************************************
 * foiGetNameBase
 *
 * DESCRIPTION:
 * takes the given filename and retrieves the base number of the name
 * e. g.: from INV0000012.etis it retrieves 0000012.
 *
 * PARAMETERS:
 *  char            *szFileName     - name of the file
 *  char            *pszBaseName    - return buffer
 *  unsigned int    uBufLen         - length of return buffer
 *
 * RETURNS:
 *  0 if successful
 ******************************************************************************
 */
int foiGetBaseName (char *szFileName, char *pszBaseName, unsigned int uBufLen)
{
  char *pTemp;
  char *pFileNm;
  int  i;


  /*
   * search the beginning of the filename in the path
   */
  pFileNm = lopszGetFileNameFromPath (szFileName);

  /* copy path */
  strncpy (szPathName, szFileName, (unsigned int) (pFileNm - szFileName));
  szPathName[pFileNm - szFileName] = '\0';

  if (szPathName[0] == '\0') {
    strcpy (szPathName, "./");
  }

  /*
   * exit if the current file name is not from an invoice
   * or if the path name is too long (check PATH_MAX in bgh.h)
   */
  if ((pFileNm = strstr (szFileName, "INV")) == NULL) {
    /* invalid name */
    return ((int) FILE_INVALIDNAME);
  }
  if ((unsigned int) (pFileNm - szFileName) > sizeof (szPathName)) {
    /* path too long */
    return ((int) FILE_PATHLENGTH);
  }

  /*
   * search dot and copy base
   */
  pTemp = pFileNm;
  while (*pTemp != '\0')  pTemp++;
  while (*pTemp != '.') {
    if (pTemp == pFileNm) {
      /* no dot -> invalid filename */
      return (-1);
    }
    pTemp--;
  }
  i = pTemp - pFileNm - 3;
  if ((i < 0) || (i > (int) (uBufLen - 1))) {
    /* 
     * either there was nothing between "INV" and the dot 
     * or the buffer is too small
     */
    return (-1);
  }
  pTemp = &pFileNm[3];
  strncpy (pszBaseName, pTemp, (unsigned int) i);
  pszBaseName[i] = '\0';

  return (0);
}


/******************************************************************************
 * loiGetNext
 *
 * DESCRIPTION:
 * primitive wildcard search for the next name
 * only one '*' as wildcard is allowed
 * a prior call to loiSetFirst is mandatory!!
 *
 * PARAMETERS:
 *  char    *szFileName     - name of the file (with wildcard)
 *  char    *szFullName     - buffer to return the full name
 *  int     iFullLen        - length of return buffer
 *
 * RETURNS:
 *  0 if successful
 ******************************************************************************
 */
int foiGetNext (char *szFileName, char *szFullName, int iFullLen)
{
  char              *pcTemp;
  char              *pcName;            /* pointer to filename without path */
  struct dirent     *dirInfo;
  char              szNameFirst [32];
  char              szNameLast [32];
  unsigned int      uLen;

  /* 
   * search the first wildcard '*' in the filename
   * and copy that string to szNameFirst
   */
  pcName = lopszGetFileNameFromPath (szFileName);
  pcTemp = pcName;
  while ((*pcTemp != '\0') && (*pcTemp != '*')) pcTemp++;

  if (*pcTemp == '\0') {
    uLen = sizeof (szNameFirst);
  } else {
    uLen = (unsigned int) (pcTemp - pcName);
    if (uLen > sizeof (szNameFirst)) {
      uLen = sizeof (szNameFirst);
    }
  }
 
  (void) strncpy (szNameFirst, pcName, uLen);
  szNameFirst[uLen] = '\0';         /* just to be sure... */

  /*
   * copy the rest of the name (after '*') to szNameLast
   */
  pcTemp++;
  uLen = (unsigned int) (pcName + strlen (pcName) - pcTemp);
  if (uLen > sizeof (szNameLast)) {
    uLen = sizeof (szNameLast);
  }
  pcTemp = strncpy (szNameLast, pcTemp, uLen);
  szNameLast[uLen] = '\0';          /* just to be sure... */


  /* search filename until the next matching name is found */
  uLen = strlen (szNameFirst);      /* for performance reasons */

  while ((dirInfo = readdir (pDir)) != NULL) {

    if (strncmp (szNameFirst, dirInfo->d_name, uLen) == 0) {
      /* 
       * found a matching entry, now check for szNameLast
       *
       * first, the found name must be longer than szNameLast
       * (otherwise it couldn't be part of it),
       * then pcTemp is set to that position in d_name, where
       * szNameLast has to start if it is the last part 
       * of d_name
       */
      if (strlen (dirInfo->d_name) > strlen (szNameLast)) {
        pcTemp = dirInfo->d_name + strlen (dirInfo->d_name)
                                 - strlen (szNameLast);

        if (strcmp (pcTemp, szNameLast) == 0) {

          /* found it ! */
          if ((strlen (dirInfo->d_name) +
               strlen (szPathName)) < (unsigned int) iFullLen) {
            strcpy (szFullName, szPathName);
            strcat (szFullName, dirInfo->d_name);
            return (0);
          }

        } /* if (strcmp (pcTemp, szNameLast) == 0) */

      } /* if (strlen (dirInfo->d_name) > strlen (szNameLast)) */

    } /* if (strncmp (szNameFirst, dirInfo->d_name, uLen) == 0) */

  } /* of while... */
  return ((int) FILE_NOMATCH);
}


/******************************************************************************
 * loiSetFirst
 *
 * DESCRIPTION:
 * set DIR to the first file with the name in the directory
 *
 * PARAMETERS:
 *  char    *pszFileName    - name of the file
 *
 * RETURNS:
 *  0 if successful
 ******************************************************************************
 */
int foiSetFirst (char *szFileName)
{
  char          *pName;             /* pointer to the filename without path */
  char          szOut [255];        /* buffer for error message */

  /* get path and filename */
  pName = lopszGetFileNameFromPath (szFileName);
  strncpy (szPathName, szFileName, (size_t) (pName - szFileName));
  szPathName[(int) (pName - szFileName)] = '\0';

  if (szPathName[0] == '\0') {
    strcpy (szPathName, "./");
  }

  /*
   * open the given directory and, if possible,
   * return the first filename
   */
  pDir = opendir (szPathName);

  if (pDir == NULL) {
    if (strlen (szPathName) > (sizeof (szOut) - 30)) {
      sprintf (szOut, "Can not open directory!");
    } else {
      sprintf (szOut, "Can not open directory %s !", szPathName);
    }
    macErrorMessage (FILE_OPENDIR, WARNING, szOut);
    
    return ((int) FILE_OPENDIR);
  }

  return (0);
}


/******************************************************************************
 * foiBuildFileName
 *
 * DESCRIPTION:
 * takes the base name and the message type and builds a new filename
 *
 * PARAMETERS:
 *  TYPEID  enMsgType       - message type
 *  char    *szBaseName     - base name
 *  char    *pszFileName    - name of the file
 *  int     iBufLen         - length of return buffer
 *
 * RETURNS:
 *  0 if successful
 ******************************************************************************
 */
int foiBuildFileName (TYPEID enMsgType, char *szBaseName,
                      char *pszFileName, unsigned int uBufLen)
{
  unsigned int   i;

  /* 
   * search table for type
   */
  for (i = 0; i < (sizeof (stTypeName) / sizeof (struct stTYPNAM)); i++) {
    if (enMsgType == stTypeName[i].enMsg) {
      break;
    }
  }

  /*
   * if we fell off the end of the table -> index was invalid
   */
  ASSERT (i < (sizeof (stTypeName) / sizeof (struct stTYPNAM)));

  ASSERT (uBufLen >= (strlen (stTypeName[i].szName) + 
		      strlen (szPathName) +
		      strlen (FILESUFFIX) +
		      strlen (FILEASTERISK) +
		      strlen (szBaseName) + 1));


  /* build the filename */
  strcpy (pszFileName, szPathName);
  strcat (pszFileName, stTypeName[i].szName);
  strcat (pszFileName, szBaseName);
  if (stTypeName[i].fMulti) {
    strcat (pszFileName, FILEASTERISK);
  }
  strcat (pszFileName, FILESUFFIX);
  return (0);
}


/******************************************************************************
 * foiReopenLog
 *
 * DESCRIPTION:
 * open the final log file and copy the temporary log file to it, then delete 
 * the temporary log file
 *
 * PARAMETERS:
 *  void
 *
 * RETURNS:
 *  0 if successful
 ******************************************************************************
 */

int foiReopenLog (void)
{
  FILE	*fpTemp;
  FILE	*fpTemp2;
  int	iRet;
  int	c;

  fovdPushFunctionName ("foiReopenLog");

  iRet = 0;

  /* open final logfile */
  strcat (stBgh.szLogDir, szLogName);

  fpTemp = fopen (stBgh.szLogDir, "w+");

  if (fpTemp == NULL) {
    sprintf (szTemp, "Could not open LOG-file, errno=%d !", errno);
    macErrorMessage (FILE_OPEN, CRITICAL, szTemp);
    iRet = (int) FILE_OPEN;
    fovdPopFunctionName ();
    return ((int) FILE_OPEN);
  }

  /* reset filepointer */
  rewind (fiBghLog);
  /* copy file */
  while ((c = getc (fiBghLog)) != EOF)
    {
      putc (c, fpTemp);
    }
  /*
   * the final log file is now created and filled
   * set the filepointer to it
   */
  fpTemp2 = fiBghLog;
  fiBghLog = fpTemp;

  /* close and delete temporary log file */
  if (fclose (fpTemp2) != 0) {
    sprintf (szTemp, "Error while closing temporary LOG-file, errno=%d !", errno);
    macErrorMessage (FILE_CLOSE, WARNING, szTemp);
    iRet = (int) FILE_CLOSE;
  }
  unlink (szLogName);

  fovdPopFunctionName ();
  return (iRet);
}


/******************************************************************************
 * foiOpenLog - open log-file
 *
 * DESCRIPTION:
 * open the temporary log file and remember the file handle
 *
 * PARAMETERS:
 *  void
 *
 * RETURNS:
 *  0 if successful
 ******************************************************************************
 */
int foiOpenLog (char* ppszProcessNr)
{
  time_t      tiCurrent;
  struct tm   *tmCET;
  int         iYear;

  fovdPushFunctionName ("foiOpenLog");

  /*
   * get the current date and time
   */
  tiCurrent = 0;
  tiCurrent = time (&tiCurrent);
  tmCET = localtime (&tiCurrent);
  iYear = tmCET->tm_year + 1900;
  if (tmCET->tm_year < 90) {
    iYear += 100;
  }
  /* generate logfilename */
  sprintf (szLogName, "BGH%04d%02d%02d%02d%02d%02d.%slog",       
           iYear, tmCET->tm_mon + 1, tmCET->tm_mday,
           tmCET->tm_hour, tmCET->tm_min, tmCET->tm_sec,
           ppszProcessNr);
  
  /*
   * open temporary log file, when the base directory
   * is loaded this log file is copied to the new
   * location {base}/LOG/
   */
  fiBghLog = fopen (szLogName, "w+");
  
  if (fiBghLog == NULL) 
    {
      sprintf (szTemp, "Could not open LOG-file, errno=%d !", errno);
      macErrorMessage (FILE_OPEN, CRITICAL, szTemp);
      fovdPopFunctionName ();
      return ((int) FILE_OPEN);
    }

  fovdPrintLog (LOG_MAX, "Starting Log at: %02d:%02d.%02d   %02d. %s %d\n",
                tmCET->tm_hour, tmCET->tm_min, tmCET->tm_sec,
                tmCET->tm_mday, pszMonth[tmCET->tm_mon], iYear);
  
  fovdPrintLog (LOG_MAX, "BGH Version: %s\n", BGH_VER);

  fovdPopFunctionName ();
  return (0);
}


/******************************************************************************
 * fovdCloseLog - close log-file
 *
 * DESCRIPTION:
 * close the log file
 *
 * PARAMETERS:
 *  void
 *
 * RETURNS:
 *  - void -
 ******************************************************************************
 */
void fovdCloseLog (void)
{
  time_t      tiCurrent;
  struct tm   *tmCET;
  int         iYear;

  fovdPushFunctionName ("foiCloseLog");

  /*
   * get the current date and time and write it 
   * as last information to the log file
   */

  tiCurrent = 0;
  tiCurrent = time (&tiCurrent);
  tmCET = localtime (&tiCurrent);
  iYear = tmCET->tm_year + 1900;
  if (tmCET->tm_year < 90) {
      iYear += 100;
  }

  fovdPrintLog (LOG_MAX, "Ending Log at: %02d:%02d.%02d   %02d. %s %d\n",
                tmCET->tm_hour, tmCET->tm_min, tmCET->tm_sec,
                tmCET->tm_mday, pszMonth[tmCET->tm_mon], iYear);

  if (fclose (fiBghLog) != 0) {
    /*
     * write out an error message that closing the log-file
     * failed.
     * DONT DECLARE IT AS CRITICAL ERROR, THIS MIGHT CAUSE A
     * RECURSION (this procedure is called while processing a 
     * critical error) !!
     */
      
    sprintf (szTemp, "Error while closing LOG-file, errno=%d !", errno);
    macErrorMessage (FILE_CLOSE, WARNING, szTemp);
    fovdPopFunctionName ();
    return;
  }
  fiBghLog = NULL;

  /*
   * tell the user where the log-file is
   */
  if (stBgh.szLogDir[0] != '\0')
    {
      printf ("\nLog-file written to \"%s\"\n", stBgh.szLogDir);
    }
  else
    {
      printf ("\nLog-file written to \"%s\"\n", szLogName);
    }

  fovdPopFunctionName ();
}

/******************************************************************************
 * fovdRemoveLog - remove log-file
 *
 * DESCRIPTION:
 * remove the log file
 *
 * PARAMETERS:
 *  void
 *
 * RETURNS:
 *  - void -
 ******************************************************************************
 */

void fovdRemoveLog (void)
{
  time_t      tiCurrent;
  struct tm   *tmCET;
  int         iYear;
  
  fovdPushFunctionName ("foiRemoveLog");
  
  /*
   * get the current date and time and write it 
   * as last information to the log file
   */

  tiCurrent = 0;
  tiCurrent = time (&tiCurrent);
  tmCET = localtime (&tiCurrent);
  iYear = tmCET->tm_year + 1900;
  if (tmCET->tm_year < 90) 
    {
      iYear += 100;
    }

  fovdPrintLog (LOG_MAX, "Ending Log at: %02d:%02d.%02d   %02d. %s %d\n",
                tmCET->tm_hour, tmCET->tm_min, tmCET->tm_sec,
                tmCET->tm_mday, pszMonth[tmCET->tm_mon], iYear);
  
  if (fclose (fiBghLog) != 0) 
    {
      /*
       * write out an error message that closing the log-file
       * failed.
       * DONT DECLARE IT AS CRITICAL ERROR, THIS MIGHT CAUSE A
       * RECURSION (this procedure is called while processing a 
       * critical error) !!
       */
      
      sprintf (szTemp, "Error while removing LOG-file, errno=%d !", errno);
      macErrorMessage (FILE_CLOSE, WARNING, szTemp);
      fovdPopFunctionName ();
      return;
    }
  
  fiBghLog = NULL;
  
  /*
   * tell the user where the log-file is
   */
  
  if (stBgh.szLogDir[0] != '\0')
    {
      unlink(stBgh.szLogDir);
    }
  else
    {
      unlink(szLogName);
    }

  fovdPopFunctionName ();
}

/******************************************************************************
 * fovdPrintLog - output to log-file
 *
 * DESCRIPTION:
 * write the output to the log file, but only if the logging level
 * is smaller than the specified logging level of this message 
 *
 * PARAMETERS:
 *  LOGLEVEL enLogLevel     - logging level of this message
 *  char    *fmt            - format string
 *  ...                     - variable arguments (uses sprintf internally)
 *
 * RETURNS:
 *  - void -
 ******************************************************************************
 */
void fovdPrintLog (LOGLEVEL enLogLevel, const char *fmt, ...) 
{
  va_list    ap;            /* pointer to data field */

  /*
   * return immediately if the message specific logging 
   * level is smaller than the global logging level
   */

  if (enLogLevel < stBgh.enLogLevel) 
    {
      return;
    }
  
  va_start (ap, fmt);
  
  /*
   * try to write information to log file,
   * if this fails, write it to stdout
   */
  
  if (fiBghLog != NULL) 
    {
      if (vfprintf (fiBghLog, fmt, ap) < 0) 
        {
          printf ("Output to logfile failed !: ");
          (void) vprintf (fmt, ap);
        }
      
      if (stBgh.bDebugMode == TRUE)
        {
          fflush(fiBghLog);
          /*
            if (vfprintf (stderr, fmt, ap) < 0) 
            {
            printf ("Output to stderr failed !: ");
            (void) vprintf (fmt, ap);
            }
           */
        } 
    }
  else 
    {
      if (TRUE == stBgh.soenLogToScreen)
        {
          (void) vprintf (fmt, ap);
        }
    }

  if (TRUE == stBgh.soenLogToScreen)
    {
      (void) vprintf (fmt, ap);
    }
  
  va_end (ap);
}

static int bgh_access(const char *path, int rights)
{
  struct stat st;
  int mask, rc;

  if (stat(path, &st) < 0)
    {
      return -1;
    }

  rc = 1;
  switch (rights)
    {
    case R_OK:
      mask = S_IRUSR;
      break;
    case W_OK:
      mask = S_IWUSR;
      break;
    case X_OK:
      mask = S_IXUSR;
      break;
    case F_OK:
      rc = 0;
      break;
    default:
      rc = -1;
    }

  if (rc >= 0 && st.st_mode & mask)
    {
      rc = 0;
    }
  else
    {
      return -1;
    }
  
  return rc;
}



