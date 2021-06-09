/*******************************************************************************
 * LH-Specification GmbH 1996.
 *
 * All rights reserved.
 * Copying of this software or parts of this software is a violation of German
 * and International laws and will be prosecuted.
 *
 * Project  :   BGH
 *
 * File     :   bgh_form.c
 * Created  :   Mar. 1996
 * Author(s):   B. Michler
 *
 * Changed  :
 * 24.05.96 	B. Michler  	added flag FOPA
 * 28.05.96	B. Michler	added version check, changed @F to @L
 * 18.06.96	B. Michler	do not allow elements with index 0 (@E0)
 *				as they are supressed in the layouter
 * 28.06.96	B. Michler	do not allow lines with index 0 (@L0)
 *				as they are index to nonexisting lines
 * 04.07.96	B. Michler	removed other_layinds (unused so far),
 *				added header information
 *
 * Description:
 * The routines in this file read the layout description and build up
 * the arrays of structures needed by the layouter.
 *
 *******************************************************************************/

#if 0	/* just for version.sh */
static char *SCCS_VERSION = "1.34";
#endif


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>			/* for isdigit */

/* User includes                */
#include "bgh.h"
#include "protos.h"
#include "layout.h"


/* User defines                 */
#define LAYOUT_VERSION	"0.2"	/* version of layout language */

#define NLINEDEFLEN 128
#define SYMBOLFIRST '@'

#define BEG_ARR    '('
#define END_ARR    ')'
#define COMMENT    '#'

#define NALLOCSTATE 10          /* number of elements to allocate as default */
#define NALLOCFIELD 10          /* number of elements to allocate as default */
#define NALLOCGRAPH 5           /* number of elements to allocate as default */


/* Function macros              */


/* Enumerations                 */


/* Local typedefs               */


/* External globals             */
extern s_line_array pstLineElem[MAXLINEELEM];


/* Globals                      */
struct STAMEM {
  UINT      size;
  UINT      next;
  state     *array;
};


struct FLDMEM {
  UINT      size;
  UINT      next;
  field     *array;
};


struct GRAMEM {
  UINT      size;
  UINT      next;
  st_graphic *array;
};


struct LAYCACHE {
  char		szFile[PATH_MAX];	/* filename of the layout information */
  stLAYINF	stLay;			/* layout information structure */
};


/* Static globals               */
static struct LAYCACHE astLayCache[10];	/* representation of layout cache */

static UINT nLineLen;
static char *pInpLine = NULL;
static char szTemp[256 + PATH_MAX];	/* temporary buffer */
static char szErrOut[256 + PATH_MAX];	/* for error output */
static char szToken[64];		/* tokenbuffer needed by various procedures */

FONTARRAY stfoCache;

/*
 * structure array for translating the flags
 * from verbose to numeric
 */
struct TYPESTRUCT {
  int   iValue;
  char  *szText;
};

/*
 * the following structurearrays are the connection between
 * the text for flags / graphic elements in the layout description
 * file and the values.
 */
struct TYPESTRUCT stAllFlags[] = {
  {ABSO,        "ABSO"},	/* absolute positioning */
  {EVPA,        "EVPA"},	/* on every page */
  {FOPA,	"FOLPA"},	/* on every page but the first */
  {FEED,        "FEED"},	/* generate form feed */
  {INDE,        "INDE"},	/* independant positioning */
  {LAPA,        "LAPA"},	/* on last page only */
  {PAGE,        "PAGE"},	/* next page */
  {GEND,	"GROUP"},	/* end of a group */
  {0,           NULL}           /* last element */
};

struct TYPESTRUCT stGraphTypes[] = {
  {O_LINE,      "TLINE"},
  {U_LINE,      "ULINE"},
  {G_RECT,      "RECT"},
  {G_FILL,      "FBOX"},
  {0,           NULL}           /* last element */
};

struct TYPESTRUCT stTextTypes[] = {
  {TEXTNORMAL,  "NORMAL"},
  {TEXTRIGHT,   "RIGHT"},
  {TEXTLEFT,    "LEFT"},
  {TEXTCENTER,  "CENTER"},
  {TEXTUL,      "UL"},
  {0,           NULL}           /* last element */
};


/* additional information about a layout */
#define ADDPGLEN      1
#define ADDPGHEIGHT   2

struct TYPESTRUCT stAddTypes[] = {
  {ADDPGLEN,    "PAGELEN"},
  {ADDPGHEIGHT, "PAGEHEIGHT"},
  {0,           NULL}           /* last element */
};

/* position flags for additional information */
#define ADDFLFIRST      1
#define ADDFLMIDD       2
#define ADDFLLAST       4

struct TYPESTRUCT stAddFlags[] = {
  {ADDFLFIRST, "FIRST"},
  {ADDFLMIDD,  "MIDDLE"},
  {ADDFLLAST,  "LAST"},
  {0,           NULL}           /* last element */
};

/* Static function prototypes   */
static char *pchGetToken (char *src, char *tok, int size, BOOL);
static int iGetLine (FILE *file);
static int iGetFieldLine (field *pfldCurrent, char *pLine);
static int iGetElemLine (state *pstaCurrent, char *pLine);
int iCacheFont (char *szFont, char *szSize);


void vdPrintLists (stLAYINF *);




/******************************************************************************
 * PrintVersInfoBghForm
 *
 * DESCRIPTION:
 * print the version info of BGH_FORM
 *
 * PARAMETERS:
 *  void
 *
 * RETURNS:
 *  - none -
 ******************************************************************************
 */
void PrintVerInfoBghForm (void)
{
  static char *SCCS_ID = "1.34.5.1";

  printf ("%s\t\t\t%s\n", __FILE__, SCCS_ID);
}


/******************************************************************************
 * pchGetToken - get next token from string
 *
 * DESCRIPTION:
 * reads the next token from the string 'src' into token 'tok', returns
 * pointer into 'src' past the read token
 *
 * PARAMETERS:
 *  char     *src   - pointer to source line
 *  char     *tok   - pointer to token
 *  int       size  - size of token pointer
 *  char      sep   - seperator
 *
 * RETURNS:
 *   pointer to remaining string
 ******************************************************************************
 */
static char *pchGetToken (char *src, char *tok, int size, BOOL bBlankTerm)
{
    BOOL   quotef;     /* is the current string quoted? */
    char   c;          /* temporary character */
    char   *tmp;

    tmp = src;

    /*
     * first scan past any whitespace in the source string
     */
    while (*tmp == ' ' || *tmp == '\t') {
        tmp++;
    }
    /*
     * check for comment
     */
    if (*tmp == COMMENT) {
      while (*tmp != '\0')  tmp++;
    }

    /*
     * scan through the source string
     */
    quotef = FALSE;
    while (*tmp != '\0') {

        if (quotef) {
            /*
             * copy everything inside quotes - including terminators
             */
            if (*tmp == '"') {
                tmp++;
                break;
            } else {
                c = *tmp++;
                if (--size > 0) {
                    *tok++ = c;
                }
                continue;
            }
        } else {
            /*
             * check for end of token
             */
            if ((*tmp == ',')	||
		(*tmp == COMMENT)) {
                break;
	    }
	    if ((bBlankTerm == TRUE) &&
		((*tmp == '\t') ||
		 (*tmp == ' '))) {
	      /* ignore all blanks */
	      while ((*tmp == ' ') ||
		     (*tmp == '\t')) {
		tmp++;
	      }
	      break;
	    }
        }

        /*
         * start of quote, enter quote mode
         */
        if (*tmp == '"') {
            quotef = TRUE;
            c = *tmp++;             /* dont copy quoting character  */
            continue;
        }

        /* record the character */
        c = *tmp++;
        if (--size > 0)
            *tok++ = c;
    }

    /*
     * terminate the token and exit
     */
    if (*tmp == ',')
        ++tmp;

    *tok = '\0';

    return  (tmp);
}


/******************************************************************************
 * iGetLine - get next line
 *
 * DESCRIPTION:
 * reads next line from file into 'pInpLine', extends 'pInpLine' if necessary
 *
 * PARAMETERS:
 *  FILE     *file      -file pointer
 *
 * RETURNS:
 *   0:     succesful read
 *   EOF:   End Of File reached
 ******************************************************************************
 */
static int iGetLine (FILE *file)
{
  int  c;
  UINT i;


  /*
   * if line is greater than default -> kill it
   */
  if ((nLineLen > NLINEDEFLEN) && (pInpLine != NULL)) {
    free (pInpLine);
    pInpLine = NULL;
    nLineLen = NLINEDEFLEN;
  }

  /*
   * allocate space for line if it is not existing
   */
  if (pInpLine == NULL) {
    pInpLine = (char *) malloc (nLineLen);
    if (pInpLine == NULL) {
      return (EOF);
    }
  }

  /* read first character */
  c = fgetc (file);

  i = 0;
  while ((c != EOF) && (c != '\n')) {
    pInpLine[i++] = (char) c;

    if (i >= nLineLen) {
      /*
       * line gets too long - get more space for it
       */
      nLineLen *= 2;
      pInpLine = (char *) realloc (pInpLine, nLineLen);
      if (pInpLine == NULL) {
        return (EOF);
      }
    }
    c = fgetc (file);       /* get next character */
  }
  /*
   * delete all <CR><LF> at the end of the line
   */
  while ((i > 0) && (pInpLine[i - 1] == '\r' || pInpLine[i - 1] == '\n')) {
    i--;
  }
  pInpLine[i] = '\0';            /* make it a C-String */

  if (c == EOF) {
    return (EOF);
  } else {
    return (0);
  }
}


/******************************************************************************
 * fIsNumber
 *
 * DESCRIPTION:
 * checks if a given string is a number
 *
 * PARAMETERS:
 *  char	*szString	- string to check
 *
 * RETURNS:
 *  TRUE	- if numeric
 *  FALSE	- otherwise
 ******************************************************************************
 */
BOOL fIsNumber (char *szString)
{
  do {
    if (*szString == '-')		szString++;
    if (!isdigit ((char) *szString))	return (FALSE);
  } while (*(++szString));

  return (TRUE);
}


/******************************************************************************
 * vdFreeFormList - discard the complete form lists
 *
 * DESCRIPTION:
 * discard the complete form list beginning with the first element
 *
 * PARAMETERS:
 *  stLAYINF    *pLay           - pointer to layout info structure
 *
 * RETURNS:
 *  - none -
 ******************************************************************************
 */
void vdFreeFormList (stLAYINF *pLay)
{
  int   i;
  state *paStates;


  paStates = pLay->paSta;

  if (paStates != NULL) {
    /* 
     * the initial strings bound to the states
     * must be discarded too
     */
    for (i = 0; paStates[i].iIndex != -1; i++) {
      if (paStates[i].info != NULL) {
        free (paStates[i].info);
      }
    }
    free (paStates);
    paStates = NULL;
  }

  if (pLay->paFld != NULL) {
    free (pLay->paFld);
    pLay->paFld = NULL;
  }

  if (pLay->paGra != NULL) {
    free (pLay->paGra);
    pLay->paGra = NULL;
  }

  if (pLay->paFInd != NULL) {
    free (pLay->paFInd);
    pLay->paFInd = NULL;
  }
}



/******************************************************************************
 * iGetType - read the types
 *
 * DESCRIPTION:
 * szFlags is a string that contains all the text attributes flags that should
 * be combined into the return value.
 * psType is a pointer to a structure that contains correlation information 
 * between the flags and the textual description of the flags.
 * As the only combination for flags is the binary 'or', the terminator between
 * the flags must be '|'.
 *
 * PARAMETERS:
 *  char              *szFlags            - character string with flags
 *  struct TYPESTRUCT *psTypes            - pointer to correlation struct
 *  BOOL              fOr                 - TRUE if flags may be combined
 *
 * RETURNS:
 *  int         - word with the flags set
 ******************************************************************************
 */
static int iGetType (char *szFlags, struct TYPESTRUCT *psType, BOOL fOr)
{
  int       iFlag;                  /* return value                         */
  int       iFlagIndex;             /* index into flags structure           */
  char      *pTemp;                 /* temporary pointer into szFlags       */

  iFlag      = 0;
  iFlagIndex = 0;
  pTemp      = szFlags;

  /*
   * search the string flag in psType
   */
  while (*pTemp != '\0') {
    if (strncmp (pTemp,
                 psType[iFlagIndex].szText, 
                 strlen (psType[iFlagIndex].szText)) != 0) {

      iFlagIndex++;
      if (psType[iFlagIndex].szText == NULL) {
        /* flag not found */
        break;
      }
    } else {

      /* found a match */
      iFlag |= (int) psType[iFlagIndex].iValue;

      /* terminate the loop if combination is not allowed */
      if (fOr == FALSE) {
        break;
      }

      /* increment pTemp to proceed to the next flag */
      pTemp += strlen (psType[iFlagIndex].szText);

      while ((*pTemp == ' ') || (*pTemp == '\t'))	pTemp++;
      if (*pTemp == '|') {
        /* ignore '|' and trailing blanks */
        pTemp++;
        while ((*pTemp == ' ') || (*pTemp == '\t'))  	pTemp++;
      }

      iFlagIndex = 0;
    }
  }
  return (iFlag);
}


/******************************************************************************
 * iGetFieldLine
 *
 * DESCRIPTION:
 * fills a field structure with the information of a line
 *
 * PARAMETERS:
 *  field       *pfldCurrent            - pointer to field structure
 *  char        *pLine                  - pointer into line
 *
 * RETURNS:
 *   0:     succesfully read
 ******************************************************************************
 */

/*
 *  Formats:
 *  @L, x, y, distance, (item1, item2, ...), (otherlay1, otherlay2, ...), flags
 *     ^ our current position
 */

static int iGetFieldLine (field *pfldCurrent, char *pLine)
{
  char          *pRest;                 /* remaining part of line       */
  char          *pTemp;                 /* temporary pointer into line  */
  UINT          j;
  UINT		iLinElem;		/* index for Line array */

  /* 
   * fill the structure with the information in the line
   * return an error if information is missing (line 
   * is too short)
   */
  pRest = pchGetToken (pLine, szToken, sizeof (szToken), TRUE);
  if (fIsNumber (szToken) == FALSE)	return ((int) FORM_LINEERR);
  pfldCurrent->x = atoi (szToken);
  if (*pRest == '\0')         		return ((int) FORM_LINETOOSHORT);

  pRest = pchGetToken (pRest, szToken, sizeof (szToken), TRUE);
  if (fIsNumber (szToken) == FALSE)	return ((int) FORM_LINEERR);
  pfldCurrent->y = atoi (szToken);
  if (*pRest == '\0')         return ((int) FORM_LINETOOSHORT);

  pRest = pchGetToken (pRest, szToken, sizeof (szToken), TRUE);
  if (fIsNumber (szToken) == FALSE)	return ((int) FORM_LINEERR);
  pfldCurrent->max_dist = atoi (szToken);
  if (*pRest == '\0')         return ((int) FORM_LINETOOSHORT);

  /*
   * now we are at the beginning of the item list which has
   * at maximum 10 elements
   */
  while (*pRest == ' ' || *pRest == '\t') pRest++;

  if (*pRest != BEG_ARR)      return ((int) FORM_LISTERROR);
  pRest++;

  /*
   * set pTemp to the terminating brace. 
   * Not a beautiful way to find the end of the list...
   */
  pTemp = pRest;
  while ((*pTemp != END_ARR) && (*pTemp != '\0'))   pTemp++;

  j = 0;
  do {
    pRest = pchGetToken (pRest, szToken, sizeof (szToken), TRUE);

    if (szToken[0] == END_ARR) {
      break;
    }
#if 0
    if (szToken[strlen (szToken) - 1] == END_ARR) {
      szToken[strlen (szToken) - 1] = '\0';
    }
    if (fIsNumber (szToken) == FALSE)	return ((int) FORM_LINEERR);
#endif

    pfldCurrent->item_list[j] = atoi (szToken);

    j++;

    /* did we reach the end ? */
    if (pRest > pTemp) {
      break;
    }
  } while (j < (sizeof (pfldCurrent->item_list) / sizeof (int)));

  pfldCurrent->num = j;

  iLinElem = pfldCurrent->iIndex;

  /* preset line information - it is not stored in the file */
#if NEWLINEHDL
  if (iLinElem >= MAXLINEELEM)	iLinElem = 0;
  pfldCurrent->pstLine = &(pstLineElem[iLinElem]);
#else
  pfldCurrent->st_line.count = 0;
  pfldCurrent->st_line.block_size = 1;
  pfldCurrent->st_line.my_argvs = NULL;
#endif

  /* read the information about header, if present */
  pRest = pchGetToken (pRest, szToken, sizeof (szToken), FALSE);
  pfldCurrent->iIndHead = atoi (szToken);

  pRest = pchGetToken (pRest, szToken, sizeof (szToken), FALSE);
  /*
   * now we have to interprete the flags,
   * szToken contains a combination of the 
   * flags in flag_devs
   */
  pfldCurrent->flags = iGetType (szToken, stAllFlags, TRUE);

  return (0);
}


/******************************************************************************
 * iGetElemLine
 *
 * DESCRIPTION:
 * fills a state structure with the information of a line
 *
 * PARAMETERS:
 *  state       *pstaCurrent        - pointer to state structure
 *  char        *pLine              - pointer into line
 *
 * RETURNS:
 *   0:     succesfully read
 ******************************************************************************
 */

/*
 *  Formats:
 *  @E1, x, y, distance, fnt, fontsize, definfo, align
 *      ^ our current position
 */

static int iGetElemLine (state *pstaCurrent, char *pLine)
{
  char          *pRest;                 /* remaining part of line           */
  char          *pTemp;
  UINT          uLen;                   /* length of text                   */

  pstaCurrent->info = NULL;

  /* 
   * fill the structure with the information in the line
   * return an error if information is missing (line 
   * is too short)
   */
  pRest = pchGetToken (pLine, szToken, sizeof (szToken), TRUE);
  if (fIsNumber (szToken) == FALSE)	return ((int) FORM_LINEERR);
  pstaCurrent->x = atoi (szToken);
  if (*pRest == '\0')         return ((int) FORM_LINETOOSHORT);

  pRest = pchGetToken (pRest, szToken, sizeof (szToken), TRUE);
  if (fIsNumber (szToken) == FALSE)	return ((int) FORM_LINEERR);
  pstaCurrent->y = atoi (szToken);
  if (*pRest == '\0')         return ((int) FORM_LINETOOSHORT);

  pRest = pchGetToken (pRest, szToken, sizeof (szToken), TRUE);
  if (fIsNumber (szToken) == FALSE)	return ((int) FORM_LINEERR);
  pstaCurrent->dist_y = atoi (szToken);
  if (*pRest == '\0')         return ((int) FORM_LINETOOSHORT);

  pRest = pchGetToken (pRest, szToken, sizeof (szToken), TRUE);

#if USEFONTCACHE
  strcpy (szTemp, szToken);
#else
  strncpy (pstaCurrent->font, szToken, sizeof (pstaCurrent->font));
  pstaCurrent->font [sizeof (pstaCurrent->font) - 1] = '\0';
#endif
  if (*pRest == '\0')         return ((int) FORM_LINETOOSHORT);

  pRest = pchGetToken (pRest, szToken, sizeof (szToken), TRUE);
  if (fIsNumber (szToken) == FALSE)	return ((int) FORM_LINEERR);
#if USEFONTCACHE
  pstaCurrent->iFontIndex = iCacheFont (szTemp, szToken);
#endif
  pstaCurrent->size = atoi (szToken);
  if (*pRest == '\0')         return ((int) FORM_LINETOOSHORT);

  /* compute the length of the string and copy it */
  while ((*pRest != '\"') && (*pRest != '\0'))    pRest++;
  pRest++;
  pTemp = pRest;
  while ((*pTemp != '\"') && (*pTemp != '\0'))    pTemp++;
  uLen = pTemp - pRest;
  if (uLen > 0) {
    pstaCurrent->info = (char *) malloc (uLen + 1);
  } else {
    pstaCurrent->info = NULL;
  }
  if (pstaCurrent->info != NULL) {
    strncpy (pstaCurrent->info, pRest, uLen);
    pstaCurrent->info[uLen] = '\0';
  }

  /* proceed to next token */
  while ((*pTemp != ',') && (*pTemp != '\0'))    pTemp++;
  if (*pTemp == ',')                             pTemp++;

  pRest = pchGetToken (pTemp, szToken, sizeof (szToken), FALSE);
  pstaCurrent->pos_flag = iGetType (szToken, stTextTypes, TRUE);

  return (0);
}


/******************************************************************************
 * iGetGraphLine
 *
 * DESCRIPTION:
 * fills a st_graphic structure with the information of a line
 *
 * PARAMETERS:
 *  state       *pgraCurrent        - pointer to graphic structure
 *  char        *pLine              - pointer into line
 *
 * RETURNS:
 *   0:     succesfully read
 ******************************************************************************
 */

/*
 *  Formats:
 *  @G, x, y, height, width, from_lay, to_lay, type, color, dist, pos
 *     ^ our current position
 */

static int iGetGraphLine (st_graphic *pgraCurrent, char *pLine)
{
  char          *pRest;                 /* remaining part of line           */

  /* 
   * fill the structure with the information in the line
   * return an error if information is missing (line 
   * is too short)
   */
  pRest = pchGetToken (pLine, szToken, sizeof (szToken), TRUE);
  if (fIsNumber (szToken) == FALSE)	return ((int) FORM_LINEERR);
  pgraCurrent->x = atoi (szToken);
  if (*pRest == '\0')         return ((int) FORM_LINETOOSHORT);

  pRest = pchGetToken (pRest, szToken, sizeof (szToken), TRUE);
  if (fIsNumber (szToken) == FALSE)	return ((int) FORM_LINEERR);
  pgraCurrent->y = atoi (szToken);
  if (*pRest == '\0')         return ((int) FORM_LINETOOSHORT);

  pRest = pchGetToken (pRest, szToken, sizeof (szToken), TRUE);
  if (fIsNumber (szToken) == FALSE)	return ((int) FORM_LINEERR);
  pgraCurrent->height = atoi (szToken);
  if (*pRest == '\0')         return ((int) FORM_LINETOOSHORT);

  pRest = pchGetToken (pRest, szToken, sizeof (szToken), TRUE);
  if (fIsNumber (szToken) == FALSE)	return ((int) FORM_LINEERR);
  pgraCurrent->width = atoi (szToken);
  if (*pRest == '\0')         return ((int) FORM_LINETOOSHORT);

  pRest = pchGetToken (pRest, szToken, sizeof (szToken), TRUE);
  if (fIsNumber (szToken) == FALSE)	return ((int) FORM_LINEERR);
  pgraCurrent->from_lay = atoi (szToken);
  if (*pRest == '\0')         return ((int) FORM_LINETOOSHORT);

  pRest = pchGetToken (pRest, szToken, sizeof (szToken), TRUE);
  if (fIsNumber (szToken) == FALSE)	return ((int) FORM_LINEERR);
  pgraCurrent->to_lay = atoi (szToken);
  if (*pRest == '\0')         return ((int) FORM_LINETOOSHORT);

  pRest = pchGetToken (pRest, szToken, sizeof (szToken), FALSE);
  pgraCurrent->type = iGetType (szToken, stGraphTypes, FALSE);
  if (*pRest == '\0')         return ((int) FORM_LINETOOSHORT);

  pRest = pchGetToken (pRest, szToken, sizeof (szToken), TRUE);
  if (fIsNumber (szToken) == FALSE)	return ((int) FORM_LINEERR);
  pgraCurrent->black = atoi (szToken);
  if (*pRest == '\0')         return ((int) FORM_LINETOOSHORT);

  pRest = pchGetToken (pRest, szToken, sizeof (szToken), TRUE);
  if (fIsNumber (szToken) == FALSE)	return ((int) FORM_LINEERR);
  pgraCurrent->dist = atoi (szToken);

  if (pgraCurrent->height != 0) {
    /* absolute graphic */
    pgraCurrent->fAbs = TRUE;
  } else {
    pgraCurrent->fAbs = FALSE;
  }

  return (0);
}

/******************************************************************************
 * iGetInfoLine
 *
 * DESCRIPTION:
 * reads additional info element
 *
 * PARAMETERS:
 *  stLAYINF    *pLay           - pointer to layout info structure
 *  char        *pLine          - pointer into line
 *
 * RETURNS:
 *   0          succesfully read
 ******************************************************************************
 */

/*
 *  Formats:
 *  @I, InfoType, y, Flags
 *     ^ our current position
 */

static int iGetInfoLine (stLAYINF *pLay, char *pLine)
{
  char          *pRest;                 /* remaining part of line           */
  int           iType;                  /* type of information              */
  int           iVal;                   /* temp. storage for read value     */
  int           iFlags;                 /* temp. storage for read flags     */

  /*
   * get the InfoType text and search it in table
   */
  pRest = pchGetToken (pLine, szToken, sizeof (szToken), FALSE);
  if (*pRest == '\0')           return ((int) FORM_LINETOOSHORT);
  iType = iGetType (szToken, stAddTypes, FALSE);

  switch (iType) {

  case ADDPGLEN:                        /* set pagelength for var. list */
    /* read the position information */
    pRest = pchGetToken (pRest, szToken, sizeof (szToken), TRUE);
    if (fIsNumber (szToken) == FALSE)	return ((int) FORM_LINEERR);
    iVal = atoi (szToken);

    /* read the page information */
    pRest = pchGetToken (pRest, szToken, sizeof (szToken), FALSE);
    iFlags = iGetType (szToken, stAddFlags, TRUE);

    /* now set the value accordingly */
    if (iFlags & ADDFLFIRST) {
      /* first page */
      pLay->iVEndFirst = iVal;
    }
    if (iFlags & ADDFLMIDD) {
      /* middle pages */
      pLay->iVEndMiddle = iVal;
    }
    if (iFlags & ADDFLLAST) {
      /* last page */
      pLay->iVEndLast = iVal;
    }

    break;

  case ADDPGHEIGHT:                     /* set pagestart (papersize) */
    /* read the position information */
    pRest = pchGetToken (pRest, szToken, sizeof (szToken), TRUE);
    if (fIsNumber (szToken) == FALSE)	return ((int) FORM_LINEERR);
    iVal = atoi (szToken);

    /* now set the value accordingly */
    pLay->iPageHeight = iVal;

    break;

  default:
    break;
  }

  return (0);
}


/******************************************************************************
 * staMalloc
 *
 * DESCRIPTION:
 * adds a state structure to an array, allocates more memory if necessary
 *
 * PARAMETERS:
 *  struct STAMEM   *pstaMem       - pointer to header structure for allocation
 *
 * RETURNS:
 *  state *     - pointer to next element in array
 ******************************************************************************
 */
state *staMalloc (struct STAMEM *pstaMem)
{
  if (pstaMem != NULL) {
    if (pstaMem->next == pstaMem->size) {
      /*
       * nothing left, allocation necessary
       */
      if (pstaMem->array == NULL) {
        /* first allocation */
        pstaMem->array = (state *) calloc (sizeof (state), NALLOCSTATE);
        if (pstaMem->array == NULL) {
          macErrorMessage (FORM_OUTMEM, CRITICAL, "Out of Memory in \"iReadFormat\"");
          return ((state*) NULL);
        }
        pstaMem->size  = NALLOCSTATE;
        pstaMem->next  = 0;

      } else {
        /*
         * reallocation necessary
         */
        pstaMem->size += NALLOCSTATE;
        pstaMem->array = (state *) realloc (pstaMem->array, 
                                            sizeof (state) * pstaMem->size);
        
        if (pstaMem->array == NULL) {
          macErrorMessage (FORM_OUTMEM, CRITICAL, "Out of Memory in \"iReadFormat\"");
          return ((state*) NULL);
        }
        /* clear the memory */
        memset (&(pstaMem->array[pstaMem->next]), 
                '\0', 
                sizeof (state) * NALLOCSTATE);
      }
    }
    /* return pointer */
    return (&(pstaMem->array[pstaMem->next++]));
  }
  return ((state*) NULL);
}


/******************************************************************************
 * fldMalloc
 *
 * DESCRIPTION:
 * adds a field structure to an array, allocates more memory if necessary
 *
 * PARAMETERS:
 *  struct FLDMEM   *pfldMem       - pointer to header structure for allocation
 *
 * RETURNS:
 *  field *     - pointer to next element in array
 ******************************************************************************
 */
field *fldMalloc (struct FLDMEM *pfldMem)
{
  UINT 	k;
  field *pfld;

  if (pfldMem != NULL) {
    if (pfldMem->next == pfldMem->size) {
      /*
       * nothing left, allocation necessary
       */
      if (pfldMem->array == NULL) {
        /* first allocation */
        pfldMem->array = (field *) calloc (sizeof (field), NALLOCFIELD);
        if (pfldMem->array == NULL) {
          macErrorMessage (FORM_OUTMEM, CRITICAL, "Out of Memory in \"iReadFormat\"");
          return ((field *) NULL);
        }
        pfldMem->size  = NALLOCFIELD;
        pfldMem->next  = 0;

      } else {
        /*
         * reallocation necessary
         */
        pfldMem->size += NALLOCFIELD;
        pfldMem->array = (field *) realloc (pfldMem->array, 
                                            sizeof (field) * pfldMem->size);

        if (pfldMem->array == NULL) {
          macErrorMessage (FORM_OUTMEM, CRITICAL, "Out of Memory in \"iReadFormat\"");
          return ((field *) NULL);
        }
        /* clear the memory */
        memset (&(pfldMem->array[pfldMem->next]), 
                '\0', 
                sizeof (field) * NALLOCFIELD);
      }
    }
    /* preset graph array */
    pfld = &(pfldMem->array[pfldMem->next]);
    for (k = 0;
	 k < (sizeof (pfld->iaGrFrom) / sizeof (int));
	 k++) {
      pfld->iaGrFrom[k] = -1;
    }
    for (k = 0;
	 k < (sizeof (pfld->iaGrTo) / sizeof (int));
	 k++) {
      pfld->iaGrTo[k] = -1;
    }

    /* return pointer */
    return (&(pfldMem->array[pfldMem->next++]));
  }
  return ((field *) NULL);
}


/******************************************************************************
 * graMalloc
 *
 * DESCRIPTION:
 * adds a graphics structure to an array, allocates more memory if necessary
 *
 * PARAMETERS:
 *  struct GRAMEM   *pgraMem       - pointer to header structure for allocation
 *
 * RETURNS:
 *  st_graphic *     - pointer to next element in array
 ******************************************************************************
 */
st_graphic *graMalloc (struct GRAMEM *pgraMem)
{
  if (pgraMem != NULL) {
    if (pgraMem->next == pgraMem->size) {
      /*
       * nothing left, allocation necessary
       */
      if (pgraMem->array == NULL) {
        /* first allocation */
        pgraMem->array = (st_graphic *) calloc (sizeof (st_graphic), NALLOCGRAPH);
        if (pgraMem->array == NULL) {
          macErrorMessage (FORM_OUTMEM, CRITICAL, "Out of Memory in \"iReadFormat\"");
          return ((st_graphic *) NULL);
        }
        pgraMem->size  = NALLOCGRAPH;
        pgraMem->next  = 0;

      } else {
        /*
         * reallocation necessary
         */
        pgraMem->size += NALLOCGRAPH;
        pgraMem->array = (st_graphic *) realloc (pgraMem->array, 
                                                 sizeof (st_graphic) * pgraMem->size);

        if (pgraMem->array == NULL) {
          macErrorMessage (FORM_OUTMEM, CRITICAL, "Out of Memory in \"iReadFormat\"");
          return ((st_graphic *) NULL);
        }
        /* clear the memory */
        memset (&(pgraMem->array[pgraMem->next]),
                '\0',
                sizeof (st_graphic) * NALLOCGRAPH);
      }
    }
    /* return pointer */
    return (&(pgraMem->array[pgraMem->next++]));
  }
  return ((st_graphic *) NULL);
}




/******************************************************************************
 * iReadFormat - read the format information
 *
 * DESCRIPTION:
 * reads the complete format information
 *
 * PARAMETERS:
 *  char        *szFilename        - file name
 *  stLAYINF    *pLay              - pointer to layout info structure
 *                                   (contents returned)
 *
 * RETURNS:
 *   0:     succesfully read
 ******************************************************************************
 */
int iReadFormat (char *szFilename, stLAYINF *pLay)
{
  FILE          *file;          /* file handle / pointer                */
  int           nLine;          /* linecount (for error output)         */
  int           nEnd;
  char          *pRestLine;
  state         *pstaNew;       /* ptr to new state                     */
  field         *pfldNew;       /* ptr to new field                     */
  st_graphic    *pgraNew;       /* ptr to new st_graphic                */
  int           iMaxIndex;      /* the biggest index of a state         */
  int           iMaxIndField;   /* the biggest index of a field         */
  int           iRetCode;
  int           i;
  struct STAMEM staMem;         /* structures for allocating the arrays */
  struct FLDMEM fldMem;
  struct GRAMEM graMem;

  BOOL	      	fVersionOK;	/* file version ok */

  int           *iaActIndex;
  int           iTemp;
  int           iTemp2;
  UINT          j;
  UINT      	k;


  /* this should never happen ... */
  if (pLay == NULL) {
    macErrorMessage (FORM_LINEERR, CRITICAL, "Illegal Parameter !!");
  }

  memset (&staMem, 0, sizeof (struct STAMEM));
  memset (&fldMem, 0, sizeof (struct FLDMEM));
  memset (&graMem, 0, sizeof (struct GRAMEM));

  fVersionOK = FALSE;

    /* reset line and linelength        */
  nLineLen    = NLINEDEFLEN;
  nLine       = 0;
  iMaxIndex   = 0;
  iMaxIndField= 0;
  iRetCode    = 0;

  /* set some layout defaults */
  pLay->iPageHeight = 842;            /* height DIN A 4 page */
  pLay->iVEndFirst  = 0;              /* page end */
  pLay->iVEndMiddle = 0;              /* page end */
  pLay->iVEndLast   = 0;              /* page end */

  pLay->paSta    = NULL;
  pLay->paFld    = NULL;
  pLay->paGra    = NULL;
  pLay->paFInd   = NULL;
  pLay->iFIndMax = 0;

  /* open the file for reading */
  file = fopen (szFilename, "r");
  if (file == NULL) {
    sprintf (szErrOut, "Can not open format file %s !", szFilename);
    macErrorMessage (FORM_CANNOTOPEN, WARNING, szErrOut);

    return ((int) FORM_CANNOTOPEN);
  }

  fovdPrintLog (LOG_CUSTOMER, "Reading layout file %s\n", szFilename);

  /*
  ** Insert a first line which stays empty; this is the default or
  ** dummy element which is referenced (later) by line elements known
  ** to the layouter and layout prepraration (bgh_prep.c) but which
  ** are not in the layout
  */
  pfldNew = fldMalloc (&fldMem);
  pfldNew->iIndex = 0;

  /*
   * read all lines from the file and build three chains,
   * one with states, one with fields and one with graphics
   */
  do {
    /* increment line count for error messages */
    nLine++;

    nEnd = iGetLine (file);

    /*
     * now a new line is read, parse it and build the structure
     */
    if ((pInpLine[0] == '\0') || (pInpLine[0] == COMMENT)) {
      /*
       * empty line or comment -> try next one
       */
      continue;
    }
    /* get the first token which is information about the line */
    pRestLine = pchGetToken (pInpLine, szToken, sizeof (szToken), TRUE);

    if (szToken[0] != SYMBOLFIRST) {
      /* not the right beginning of a line */
      sprintf (szErrOut, "File \"%s\":Wrong beginning of line in line %d!", 
               szFilename, nLine);
      macErrorMessage (FORM_LINEERR, WARNING, szErrOut);
      iRetCode = (int) FORM_LINEERR;
    }

    /* the version has to be the first entry after comments */
    if (fVersionOK == FALSE) {

      if (szToken[1] == 'V') {		/* Version information */

	pRestLine = pchGetToken (pRestLine, szToken, sizeof (szToken), TRUE);

	if (strcmp (szToken, LAYOUT_VERSION) == 0) {
	  fVersionOK = TRUE;
	  continue;
	} else {
	  sprintf (szErrOut, "File \"%s\": Wrong version %s instead of %s !", 
		   szFilename, szToken, LAYOUT_VERSION);
	  macErrorMessage (FORM_LINEERR, WARNING, szErrOut);
	  iRetCode = (int) FORM_LINEERR;
	  break;
	}

      } else {
	sprintf (szErrOut, "File \"%s\": No version in file !", 
		 szFilename);
	macErrorMessage (FORM_LINEERR, WARNING, szErrOut);
	iRetCode = (int) FORM_LINEERR;
	break;
      }
    }

    /* differentiate the different information */
    switch (szToken[1]) {

    case 'E':                         /* Element line */
      pstaNew = staMalloc (&staMem);

      pstaNew->iIndex = atoi (&szToken[2]);

      if ((fIsNumber (&szToken[2]) == FALSE) 	||
	  (pstaNew->iIndex <= 0)		||
          (iGetElemLine (pstaNew, pRestLine) != 0)) {
        /* discard the erronous line */
        staMem.next--;

        sprintf (szErrOut, "File \"%s\": Error in line %d!", 
                 szFilename, nLine);
        macErrorMessage (FORM_LINEERR, WARNING, szErrOut);
        iRetCode = (int) FORM_LINEERR;

      } else {
        /* remember the biggest index */
        if (pstaNew->iIndex > iMaxIndex) {
          iMaxIndex = pstaNew->iIndex;
        }
      }
      break;

    case 'L':                         /* Field line */
      pfldNew = fldMalloc (&fldMem);

      pfldNew->iIndex = atoi (&szToken[2]);

      if ((fIsNumber (&szToken[2]) == FALSE) 	||
	  (pfldNew->iIndex <= 0) 		||
          (iGetFieldLine (pfldNew, pRestLine) != 0)) {
        /* discard the erronous line */
        fldMem.next--;

        sprintf (szErrOut, "File \"%s\": Error in line %d!", 
                 szFilename, nLine);
        macErrorMessage (FORM_LINEERR, WARNING, szErrOut);
        iRetCode = (int) FORM_LINEERR;

      } else {
        /* remember the biggest index */
        if (pfldNew->iIndex > iMaxIndField) {
          iMaxIndField = pfldNew->iIndex;
        }
      }
      break;

    case 'G':                         /* Graphics line */
      pgraNew = graMalloc (&graMem);

      if (iGetGraphLine (pgraNew, pRestLine) != 0) {
        /* discard the erronous line */
        graMem.next--;

        sprintf (szErrOut, "File \"%s\": Error in line %d!", 
                 szFilename, nLine);
        macErrorMessage (FORM_LINEERR, WARNING, szErrOut);
        iRetCode = (int) FORM_LINEERR;
      }
      break;

    case 'I':                         /* additional information */
      (void) iGetInfoLine (pLay, pRestLine);
      break;

    default:                          /* illegal type */
      sprintf (szErrOut, "File \"%s\": Error in line %d (unknown type) !", 
               szFilename, nLine);
      macErrorMessage (FORM_LINEERR, WARNING, szErrOut);
      iRetCode = (int) FORM_LINEERR;
      continue;
    }

  } while (nEnd != EOF);

  if (fclose (file) != 0) {
    if (strlen (szFilename) > (sizeof (szErrOut) - 32)) {
      sprintf (szErrOut, "Can not close format file!");
    } else {
      sprintf (szErrOut, "Can not close format file %s !", szFilename);
    }
    macErrorMessage (FORM_NOCLOSE, WARNING, szErrOut);

    iRetCode = (int) FORM_NOCLOSE;
  }
  /*
   * free memory for input line
   */
  free (pInpLine);
  pInpLine = NULL;

  /* stop here if something went wrong */
  if (iRetCode != 0) {
    return (iRetCode);
  }

  /*
   * some replacements for the indices should be done.
   * The index to a state in a field should be the direct index
   * into the state-array. So for each state the actual index is 
   * written to the position of the symbolic index in field
   */

  /* first build an array ... */
  iaActIndex = (int *) calloc ((UINT) (iMaxIndex + 1), sizeof (int));

  if (iaActIndex == NULL) {
    macErrorMessage (FORM_OUTMEM, CRITICAL, 
                     "Out of Memory in \"iReadFormat\"");
    return ((int) FORM_OUTMEM);
  }

  for (i = 0; i < staMem.next; i++) {
    pstaNew = &(staMem.array[i]);
    iaActIndex[pstaNew->iIndex] = i;

    /* check font size */
    if (pstaNew->size < 0) {
      sprintf (szErrOut,
	       "Layout information file \"%s\":\n"
	       "\tElement @E%d: negative fontsize!!",
	       szFilename, pstaNew->iIndex);
      macErrorMessage (FORM_LINEERR, WARNING, szErrOut);
    }
  }

  /* check if lines exist */
  if (fldMem.size == 0) {
    sprintf (szErrOut,
	     "Layout information file \"%s\":\n"
	     "\tNo line information found!",
	     szFilename);
    macErrorMessage (FORM_LINEERR, WARNING, szErrOut);
    return ((int) FORM_LINEERR);
  }

  /* .. now replace indices in fields ... */
  for (i = 0; i < fldMem.next; i++) {

    pfldNew = &(fldMem.array[i]);
    for (j = 0; j < pfldNew->num; j++) {
      iTemp = pfldNew->item_list[j];

      if ((iTemp < 0) || (iTemp > iMaxIndex)) {
        /* wrong index */  
        sprintf (szErrOut,
                 "Layout information file \"%s\":\n"
                 "\tElement @L%d: index of 'list of elements' nr. %d\n"
                 "\t              points to nonexisting element @E%d",
                 szFilename, pfldNew->iIndex, j + 1, iTemp);
        macErrorMessage (FORM_LINEERR, WARNING, szErrOut);

        pfldNew->item_list[j] = -1;
      } else if (iTemp == 0) {
	/* index 0 is the NULL element for suppressing a single element */
        pfldNew->item_list[j] = -1;
      } else {
        pfldNew->item_list[j] = iaActIndex[iTemp];
      }
    }
  }
  free (iaActIndex);

  /* first build an array ... */
  iaActIndex = (int *) calloc ((UINT) (iMaxIndField + 1), sizeof (int));
  if (iaActIndex == NULL) {
    macErrorMessage (FORM_OUTMEM, CRITICAL, 
                     "Out of Memory in \"iReadFormat\"");
    return ((int) FORM_OUTMEM);
  }

  for (i = 0; i < fldMem.next; i++) {
    iaActIndex[fldMem.array[i].iIndex] = i;
  }

  /* .. now replace indices in graphics ... */
  for (i = 0; i < graMem.next; i++) {
    pgraNew = &(graMem.array[i]);

    /* check index condition */
    iTemp = pgraNew->from_lay;

    if ((iTemp < 0) || (iTemp > iMaxIndField)) {
      /* wrong index */
      sprintf (szErrOut,
               "Layout information file \"%s\":\n"
               "\tElement @G nr. %d: index of 'from format element'\n"
               "\t                   points to nonexisting element @L%d",
               szFilename, i, iTemp);
      macErrorMessage (FORM_LINEERR, WARNING, szErrOut);

      pgraNew->from_lay = 0;
    } else {
      pgraNew->from_lay = iaActIndex[iTemp];
      iTemp2 		= iaActIndex[iTemp];
      /*
       * insert the index to this graphic element in
       * the table of the related field element
       */
      for (k = 0;
	   k < (sizeof (pfldNew->iaGrFrom) / sizeof (int));
	   k++) {
	if (fldMem.array[iTemp2].iaGrFrom[k] == -1) {
	  fldMem.array[iTemp2].iaGrFrom[k] = i;
	  break;
	}
      }
    }

    iTemp = pgraNew->to_lay;

    if ((iTemp < 0) || (iTemp > iMaxIndField)) {
      /* wrong index */
      sprintf (szErrOut,
               "Layout information file \"%s\":\n"
               "\tElement @G nr. %d: index of 'to format element'\n"
               "\t                   points to nonexisting element @L%d",
               szFilename, i, iTemp);
      macErrorMessage (FORM_LINEERR, WARNING, szErrOut);

      pgraNew->to_lay = 0;
    } else {
      pgraNew->to_lay = iaActIndex[iTemp];
      iTemp2 		= iaActIndex[iTemp];
      /*
       * insert the index to this graphic element in
       * the table of the related field element
       */
      for (k = 0;
	   k < (sizeof (pfldNew->iaGrTo) / sizeof (int));
	   k++) {
	if (fldMem.array[iTemp2].iaGrTo[k] == -1) {
	  fldMem.array[iTemp2].iaGrTo[k] = i;
	  break;
	}
      }
    }
  }

  /*
   * the 'header' elements in the field array have to
   * be 'renumbered' too!
   */
  for (i = 0; i < fldMem.next; i++) {

    pfldNew = &(fldMem.array[i]);

    iTemp = pfldNew->iIndHead;
    if ((iTemp < 0) ||
	(iTemp > iMaxIndField)) {
      /* wrong index */
      sprintf (szErrOut,
	       "Layout information file \"%s\":\n"
	       "\tElement @L%d: index of 'header' points to nonexisting element @L%d",
	       szFilename, pfldNew->iIndex, iTemp);
      macErrorMessage (FORM_LINEERR, WARNING, szErrOut);

      pfldNew->iIndHead = 0;
    } else {
      pfldNew->iIndHead = iaActIndex[iTemp];
    }
  }

  /*
   * add a last element as an end indicator to each list 
   */
  pstaNew = staMalloc (&staMem);
  pstaNew->iIndex = -1;
  pfldNew = fldMalloc (&fldMem);
  pfldNew->iIndex = -1;
  pgraNew = graMalloc (&graMem);
  pgraNew->black = -1;                  /* sorry, no index in graphics */


  /*
   * Now we have four arrays, one with all elements (state), one with all 
   * fields, one with all graphics and one array for indexed access
   * to the fields. Set the pointers to these arrays
   */
  pLay->paSta    = staMem.array;
  pLay->paFld    = fldMem.array;
  pLay->paGra    = graMem.array;
  pLay->paFInd   = iaActIndex;
  pLay->iFIndMax = iMaxIndField;

  return (iRetCode);
}


/******************************************************************************
 * pstGetFormat - get layout format
 *
 * DESCRIPTION:
 * search the layout format in table, read it in if not found
 *
 * PARAMETERS:
 *  char        *szFilename        - file name
 *
 * RETURNS:
 *  stLAYINF * - pointer to layout information
 *
 ******************************************************************************
 */
stLAYINF *pstGetFormat (char *szFileName)
{
  UINT i;				/* counter... */

  static BOOL	bFirstCall = TRUE;	/* for initialization */
  static stLAYINF stLayStatic;		/* static representation for uncached layouts */

  /* clear the array if called first time */
  if (bFirstCall == TRUE) {
    memset (astLayCache, 0, sizeof (astLayCache));
    memset (&stLayStatic, 0, sizeof (stLayStatic));
    bFirstCall = FALSE;
  }


  /*
   * search the cache table for an entry with the given name,
   */
  for (i = 0; i < (sizeof (astLayCache) / sizeof (struct LAYCACHE)); i++) {
    if (strcmp (astLayCache[i].szFile, szFileName) == 0) {

      /* found corresponding entry, return pointer to struct */
      return (&(astLayCache[i].stLay));
    }

    if (astLayCache[i].szFile[0] == '\0') {
      /*
       * found an empty entry, as the table is filled from
       * the beginning and entries are never deleted, the
       * searched element is not found and may be inserted here
       */
      if (iReadFormat (szFileName, &(astLayCache[i].stLay)) == 0) {
	strcpy (astLayCache[i].szFile, szFileName);
	return (&(astLayCache[i].stLay));
      } else {
	return ((stLAYINF *) NULL);
      }
    }
  }

  /*
   * entry was not found and table is full if it was searched
   * until the end. In this case the layout information is read
   * into a single global structure and returned. It is not
   * cached, but it is processed correctly.
   * If this code is reached in production, the table
   * 'astLayCache' should be increased.
   */
  if (i >= (sizeof (astLayCache) / sizeof (struct LAYCACHE))) {

    fovdPrintLog (LOG_NORMAL,
		  "Layout cache too small, increase 'astLayCache' in %s for better performance!\n",
		  __FILE__);

    /*
     * discard the array if it was previously used,
     * if it wasn't, all pointers are NULL and nothing is
     * done in vdFreeFormList
     */
    vdFreeFormList (&stLayStatic);
    memset (&stLayStatic, 0, sizeof (stLayStatic));

    if (iReadFormat (szFileName, &stLayStatic) == 0) {
      return (&stLayStatic);
    }
  }

  return ((stLAYINF *) NULL);
}


/******************************************************************************
 * fovdFreeFormMem
 *
 * DESCRIPTION:
 * free all memory allocated for layouts
 *
 * PARAMETERS:
 *  void
 *
 * RETURNS:
 *  void
 *
 ******************************************************************************
 */
void fovdFreeFormMem (void)
{
  int	i;				/* counter... */


  for (i = 0; i < (sizeof (astLayCache) / sizeof (struct LAYCACHE)); i++) {
    /* free the corresponding list */
    if (astLayCache[i].szFile[0] != '\0') {
      vdFreeFormList (&(astLayCache[i].stLay));
    }
  }
  /* clear the whole array now */
  memset (astLayCache, 0, sizeof (astLayCache));

  /* free memory of font cache */
  if (stfoCache.font != NULL) {
    free (stfoCache.font);
  }
  memset (&stfoCache, 0, sizeof (stfoCache));
}


/******************************************************************************
 * vdPrintList
 *
 * DESCRIPTION:
 * print the lists for debugging purposes
 *
 * PARAMETERS:
 *  void
 *
 * RETURNS:
 *  void
 ******************************************************************************
 */
void vdPrintLists (stLAYINF *pLay)
{
  UINT  i, j;
  state *paStates;
  field *paFields;
  st_graphic *paGraphics;

  paStates = pLay->paSta;
  paFields = pLay->paFld;
  paGraphics = pLay->paGra;

  if (paStates != NULL) {
    printf ("state =\n");
    for (i = 0; paStates[i].iIndex != -1; i++) {
      printf ("@E%d %d, %d, %d, \"%d\", %d, \"%s\", %x,\n", paStates[i].iIndex,
              paStates[i].x, paStates[i].y, paStates[i].dist_y, 
              paStates[i].iFontIndex, paStates[i].size, 
              paStates[i].info, paStates[i].pos_flag); 
    }
  }

  if (paFields != NULL) {
    printf ("\nfield =\n");
    for (i = 0; paFields[i].iIndex != -1; i++) {
      printf ("@L%d, %d, %d, %d, %d, {", paFields[i].iIndex,
              paFields[i].x, paFields[i].y, paFields[i].max_dist, paFields[i].num); 
      for (j = 0; j < paFields[i].num; j++) {
        printf ("%d, ", paFields[i].item_list[j]);
      }
      printf ("}, {%d, %d, NULL}, {",
#if NEWLINEHDL
              paFields[i].pstLine->count, paFields[i].pstLine->block_size);
#else
              paFields[i].st_line.count, paFields[i].st_line.block_size);
#endif
      for (j = 0; j < 5; j++) {
        printf ("%d | ", paFields[i].iaGrFrom[j]);
        printf ("%d, ", paFields[i].iaGrTo[j]);
      }
      printf ("}, %d, %d\n", paFields[i].iIndHead, paFields[i].flags);
    }
  }

  if (paGraphics != NULL) {
    printf ("\ngraphics =\n");
    for (i = 0; paGraphics[i].black != -1; i++) {
      printf ("@G, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
              paGraphics[i].x, paGraphics[i].y, paGraphics[i].height, paGraphics[i].width,
              paGraphics[i].from_lay, paGraphics[i].to_lay, paGraphics[i].type, 
              paGraphics[i].black, paGraphics[i].dist);
    }
  }
}


/******************************************************************************
 * iCacheFont
 *
 * DESCRIPTION:
 * search the font in the font cache table, if it is not found there - add it,
 * return the index to the font cache
 *
 * PARAMETERS:
 *  char	*szFont		- name of the font
 *  char       	*szSize		- size of the font
 *
 * RETURNS:
 *  int		- index to the font cache table
 ******************************************************************************
 */
int iCacheFont (char *szFont, char *szSize)
{
  int	iIndex;
  char	szTFont[NFONTSIZE];
  static BOOL	fFirst = TRUE;

  /* if assertion fails -> increase NFONTSIZE */
  ASSERT ((strlen (szFont) + strlen (szSize)) < NFONTSIZE - 12);

  if (fFirst) {
    memset (&stfoCache, 0, sizeof (stfoCache));
    fFirst = FALSE;
  }

  /* write the given parameter as a temporary string for search and insert */
  sprintf (szTFont, "%s findfont %s", szFont, szSize);

  /* search the font in the table */
  for (iIndex = 0; iIndex < stfoCache.next; iIndex += NFONTSIZE) {

    if (strcmp (&(stfoCache.font[iIndex]), szTFont) == 0) {
      /* found the font in the table -> return index */
      return (iIndex / NFONTSIZE);
    }
  }
  /* did not find font -> add it */

  if (stfoCache.next == stfoCache.size) {
    /*
     * nothing left, allocation necessary
     */
    if (stfoCache.font == NULL) {
      /* first allocation */
      stfoCache.font = (char *) calloc (NFONTSIZE, NALLOCFONT);

      if (stfoCache.font == NULL) {
	macErrorMessage (FORM_OUTMEM, CRITICAL, "Out of Memory in \"iCacheFont\"");
	return (0);
      }
      stfoCache.size = NALLOCFONT * NFONTSIZE;
      stfoCache.next = 0;

    } else {

      /*
       * reallocate memory
       */
      stfoCache.size = stfoCache.size + (NALLOCFONT * NFONTSIZE);
      stfoCache.font = (char *) realloc (stfoCache.font,
					 stfoCache.size);
      if (stfoCache.font == NULL) {
	macErrorMessage (FORM_OUTMEM, CRITICAL, "Out of Memory in \"iCacheFont\"");
	return (0);
      }
      /* clear the new allocated memory */
      memset (&(stfoCache.font[stfoCache.next]),
	      0,
	      NALLOCFONT * NFONTSIZE);
    }
  }
  /* now add the font */
  strcpy (&(stfoCache.font[stfoCache.next]), szTFont);
  iIndex = stfoCache.next;
  stfoCache.next += NFONTSIZE;

  return (iIndex / NFONTSIZE);
}


/******************************************************************************
 * vdPrintFontDef
 *
 * DESCRIPTION:
 * print the font definition
 *
 * PARAMETERS:
 *  FILE	*fPtr		- file pointer
 *
 * RETURNS:
 *  void
 ******************************************************************************
 */
void vdPrintFontDef (FILE *fPtr)
{
  int iIndex;

  iIndex = 0;

  do {
    fprintf (fPtr, "/FN%d /%s scalefont def\n",
	     iIndex / NFONTSIZE, 
	     &(stfoCache.font[iIndex]));

    iIndex += NFONTSIZE;

  } while (iIndex < stfoCache.next);
}

