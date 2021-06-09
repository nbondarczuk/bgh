/*****************************************************************************/
/*                                                                           */
/* ABSTRACT : BGH data preprocessor                                          */
/*                                                                           */
/* AUTHOR   : Bernhard Michler, LHS Projects                                 */
/*                                                                           */
/* CREATED  : 24-Apr-1996                                                    */
/*                                                                           */
/* MODIFIED :                                                                */
/*		B. Michler	15-May-1996				     */
/*			no more sorting and printing in invoice, print is    */
/*			done immediately				     */
/*		B. Michler	29-May-1996				     */
/*                      roaming page                                         */
/*		B. Michler	30-May-1996				     */
/*                      balance page                                         */
/*		B. Michler	31-May-1996				     */
/*                      legend added to ITB                                  */
/*		B. Michler	03-June-1996				     */
/*			from version 1.14: changed 'while (ptr && strcmp..)' */
/*			to 'while (ptr) { if (strcmp..)}' because of SUN     */
/*		B. Michler	20-June-1996				     */
/*			in 'foiPrintSumBlock' and 'fostPrintSim': improved   */
/*                      the grouping of layout elements; now it is grouped   */
/*                      for subscribers and within subscribers for contracts */
/*		B. Michler	20-June-1996				     */
/*			added printing of LIN 04 elements in Sumsheet        */
/*		B. Michler	21-June-1996				     */
/*			invoice: print LINs with no article number as other  */
/*			elements                                             */
/*		B. Michler	28-June-1996				     */
/*			roaming: don't print header a.s.o. when there are no */
/*			elements                                             */
/*		B. Michler	28-June-1996				     */
/*			don't print header, address ... when they were al-   */
/*			ready printed (with another TIMM)                    */
/*           	B. Michler  	14-Oct-1996				     */
/*		    	String SCCS_VERSION for version.sh		     */
/*           	B. Michler  	15-Oct-1996				     */
/*                      better checks in get_next_line                       */
/*                                                                           */
/*                                                                           */
/*                                                                           */
/*                                                                           */
/* DESCRIPTION :                                                             */
/*                                                                           */
/* The purpose of this module is, to collect and assemble all data from one  */
/* TIMM message (version 2.10) , which are necessary for printing of this    */
/* message according to the layout specification.                            */
/* This module is called after parsing of the Timm messages for one customer.*/
/* It gets a pointer to one TIMM message and a pointer to a layout structure */
/* which is then filled with the necessary data.                             */
/* The code is highly dependent on the layout of the page, which is stored   */
/* within a file with the extension '.lay'.                                  */
/* Import notice:                                                            */
/* After printing the calling function must call the function ...            */
/* fovdFreePrepMemory(void) to free all allocated memory.                    */
/*                                                                           */
/* The layout of each output page is defined by three parts:                 */
/*                                                                           */
/*  1. The page description file (e.g. invoice.lay).                         */
/*  2. The fixed text strings which are store within the directory TMP/FIX   */
/*     and pushed to the page by vdAddElement-calls.                         */
/*  3. The variable text strings which are pushed to the page too by         */
/*     vdAddElement-calls.                                                   */
/*                                                                           */
/* Supported TIMM messages:                                                  */
/*                                           status:                         */
/*  Invoice              (BCH-PAYMENT)       ready                           */
/*  Roaming              (BCH-ROAMING)       ready                           */
/*  Itemized bill        (BCH-SUBSCRIBER)    ready                           */
/*  Sumsheet             (BCH-SUMSHEET)      ready                           */
/*  Balance              (BCH-BALANCE)       ready                           */
/*  Legend               (BCH-LEGEND)        ready                           */
/*  Invoice for a dealer (DCH-DEALER)         ---                            */
/*  Invoice for model    (DCH-MODEL)          ---                            */
/*  Dunning letter       (DWH-LETTER)         ---                            */
/*                                                                           */
/*									     */
/* GENERAL DESCRIPTION OF LAYOUT GENERATION:				     */
/*									     */
/* The layout information is used to build up two arrays:		     */
/*								      	     */
/* - one array with all element information; each element of this array	     */
/* contains the data of an element description of the layout file (in        */
/* general the additional positioning offsets and information about the      */
/* character font plus a default text).					     */
/*									     */
/* - one array with all line information; each element of this array	     */
/* contains  the data of a line description of the layout file plus	     */
/* additional line inforamtion for the text to be printed		     */
/* 									     */
/*									     */
/* The data preprocessor knows how many and which elements a line has,	     */
/* the relation is the line index (the number in '@L2').		     */
/* For any line that should be printed, the data preprocessor collects all   */
/* the necessary text-pointers (one for each element of the line) and then   */
/* calls a routine ('fovdAddElement') which attaches the text pointers       */
/* to the line. For this connection each line has a pointer to an 'Array     */
/* of pointer to lines' (in fact 'char ***'); these array has one pointer    */
/* to an array of string-pointers which has a string pointer for each        */
/* element of the line. Every element of the 'Array of pointer to lines'     */
/* represents a single printed line of the actal line type (i. e. if a       */
/* line should appear more than once with different text but with the        */
/* same attributes, it its represented by one single line in the layout      */
/* description, but mutliple elements get attached to that line via	     */
/* multiple calls to 'fovdAddElement').					     */
/* If the data preprocessor does not attach text pointers to a certain       */
/* line (it does not call 'fovdAddElement' for that line), this line will    */
/* not be printed (to use the default text from the layout information       */
/* file 'fovdAddElement' has to be called with NULL pointers).		     */
/*									     */
/* The layouter goes through the array of the lines and prints all lines     */
/* with the corresponding attributes (for which it also uses the elements    */
/* array).								     */
/*									     */
/* Representation of 'lines' in memory:					     */
/* ------------------------------------					     */
/*									     */
/*                    Array      Array of pointers     Line		     */
/*                     of          to text junks     elements		     */
/*                   pointer        of the line				     */
/*                     to                          +----------+		     */
/*                    lines        +-------------->|   Text   |		     */
/*                                 |               +----------+		     */
/*                                 |               +----------+		     */
/*                                 |    +--------->|   Text   |		     */
/*                                 |    |          +----------+		     */
/*                   +-----+    +----+----+----+   +----------+		     */
/*         Line      |     |--->|    |    |    |-->|   Text   |		     */
/*         Array     +-----+    +----+----+----+   +----------+		     */
/*                   |     |    :    :    :    :		     	     */
/*         +----+    +-----+    +----+----+----+		    	     */
/*         |    |--->|     |--->|    |    |    |		     	     */
/*         |    |    +-----+    +----+----+----+		     	     */
/*         | L  |		     					     */
/*         |    |                                  +----------+		     */
/*         | I  |                  +-------------->|   Text   |		     */
/*         |    |                  |               +----------+		     */
/*         | N  |                  |               +----------+		     */
/*         |    |                  |    +--------->|   Text   |		     */
/*         | E  |                  |    |          +----------+		     */
/*         |    |    +-----+    +----+----+----+   +----------+		     */
/*         |    |    |     |--->|    |    |    |-->|   Text   |		     */
/*         |    |    +-----+    +----+----+----+   +----------+		     */
/*         |    |    |     |    :    :    :    :		     	     */
/*         +----+    +-----+    +----+----+----+		     	     */
/*         |    |--->|     |--->|    |    |    |		     	     */
/*         |    |    +-----+    +----+----+----+		     	     */
/*         | L  |		     					     */
/*         |    |		     					     */
/*         | I  |		     					     */
/*         |    |		     					     */
/*         | N  |		     					     */
/*         |    |		     					     */
/*         | E  |		     					     */
/*         |    |		     					     */
/*         |    |		     					     */
/*         |    |		     					     */
/*         |    |		     					     */
/*         +----+		     					     */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/

#include <stdio.h>           /* Declarations and defini-     */
                             /* tions for standard I/O.      */
#include <string.h>          /* string functions             */
#include <stdlib.h>          /* atol, atof                   */
#include <time.h>
#include <memory.h>
#include <ctype.h>
#include <stdarg.h>             /* for variable arguments */
#include "bgh.h"             /* Declarations and defines     */
                             /* common for BGH.              */
#include "bgh_prep.h"
#include "protos.h"
#include "fixtext.h"
#include "layout.h"


#define DURATIONINSECS	1	/* print duration in secs. else in hh:mm:ss */
#define ANONYMIZENUMBER	0
#define CALLRATINGTYPE	0
#define EUROPEAN_DATE	1
#define XCDINSUMSHEET	1	/* print XCD in sumsheet */

#define CONDENSESPLITXCDS	1 /* condense XCDs with splitted calls */
#define BELOWXCDTHRESHOLD	0 /* skip XCDs below Threshold */

#define DATE_SEPERATOR	'.'	/* seperator for date */
#define AMNTSTRING	32	/* a string length for moa strings */

/* duration qualifier for itemisation */
/*#define DURATION_QUALIFIER " [hms]"*/
#define DURATION_QUALIFIER ""

/* cast to (char *) is necessary for insight */
#define COND_FETCH(a,b) (((a != NULL) && (a->b != NULL)) ? a->b : (char *)BLANK_CHAR)
#define VAL_ONE(a)      ( (a != NULL)              	 ? a    : (char *)BLANK_CHAR)
#define FIXTEXT(a)	((fixstring[a] != NULL) ? fixstring[a] : (char *)BLANK_CHAR)

#define DOC_TYPE enum e_doc_type

/* defines for StrAdd */
#define MAXFRAC	3			/* number of max. frac. digits	    */
#define FRACPOT 1000			/* 10 ^ MAXFRAC (1 with MAXFRAC 0)  */
#define FRACNULLS	"00000000"	/* sting with at least MAXFRAC '0'  */

/* chars that will be used to add to the TIMM-message        */
#define RELEVANT_TIMM_CHAR_OF_SORT_CRITERIA   27

DOC_TYPE { invoice, roaming_page };

/* french and italian are not supported */
#define LANG_FRENCH	0
#define LANG_ITALIAN	0

/* names of the language specific fixstring files            */
/* place where the fix strings are stored:                   */
#define FIX_DIR           "BGH/LAYOUT/"
#define MAX_LANGUAGES      4
 
/* blocksize for allocation of large memory blocks */
#define MEMORY_BLOCK_SIZE           16384


/* Function macros              */



/*************************************************************/
/* external variables                                        */
/*************************************************************/
extern char gszPageString[];	/* current page number */
extern stBGHGLOB stBgh;		/* structure with globals for BGH */

/*************************************************************/
/* global variables                                          */
/*************************************************************/

/* variables for header printing for itemisation */
BOOL	fHeader1, fHeader2, fHeader3;
BOOL	fHeader4, fHeader5, fHeader6;

static char	*pszCustId;	/* pointer to curent cust. id in unb */

struct LARGE_MEM_BLOCK
{
  struct LARGE_MEM_BLOCK *next;           /* ptr to next large memory block */
  /* buffer for small memory blocks */
  unsigned char buf [MEMORY_BLOCK_SIZE- sizeof(struct LARGE_MEM_BLOCK *)];
};


struct s_group_22 *g_22_sorted_1;  /* subscription                           */


m_char           *buf_ptr;               /* pointer to allocated memory block*/
void             *allocated_memory = NULL; /* ptr to 1. allocated memory seg.  */


/* array for all languages       */
char *fixstring_table_all[MAX_LANGUAGES][MAXFIXSTRNUM + 1];

char **fixstring;                         /* pointer to a language           */
char laszErrMsg [PATH_MAX + 128];         /* buffer for error messages       */

static char BLANK_CHAR [] = "";

/* structure for reference tables */
typedef struct {
  char	*pszSea;		/* pointer to search text */
  int	iNew;			/* index to new text in fixtext table */
} stSTRS;

/*
 * structuretable with the replacemets for the service code identifiers
 * in the IMD-Segment for the balance
 */
stSTRS stBalItems[] = {
  {"PAYMN", 	FIX_BAL_PAYMN},
  {"WROFF", 	FIX_BAL_WROFF},
  {"OINVC", 	FIX_BAL_OINVC},
  {"CRMEM", 	FIX_BAL_CRMEM},
  {"COACC", 	FIX_BAL_COACC},
  {"CSHGL", 	FIX_BAL_CSHGL},
  {"PMTRI", 	FIX_BAL_PMTRI},
  {"INTRI", 	FIX_BAL_INTRI},
  {"TROUT", 	FIX_BAL_TROUT},
  {"ADJST", 	FIX_BAL_ADJST},
  {"BOUNC", 	FIX_BAL_BOUNC},
  {NULL,	0}
};

/*
 * structuretable with the replacemets for the service code identifiers
 * in the IMD-Segment for the legend
 */
stSTRS stLegItems[] = {
  {"SN", 	LEG_SVCE},
  {"TT", 	LEG_TTIME},
  {"TZ", 	LEG_TZONE},
  {NULL,	0}
};

/*************************************************************/
/* prototypes of external functions                          */
/*************************************************************/
extern void vdAddElement (stLAYINF *, int, int, ...);
extern void vdBreakList (stLAYINF *, int, ...);
extern BOOL fofIsPrinted (stLAYINF *, int);
extern BOOL fofLineExists (stLAYINF *, int);


/*************************************************************/
/* prototypes of module global functions                     */
/*************************************************************/

int BghPrepareData (stTIMM *interchange, stLAYINF *pLay);

static int foiPrintInvLinSegs( stTMSG *, stLAYINF *);

static void fovdPrintXcdElHead (int, stLAYINF *);
static short loiPrintXcdElem (struct s_xcd_seg *, int, stLAYINF *);
static int foiPrintXcdItb (stTMSG *, stLAYINF *);
static int foiPrintXcd (struct s_group_22 *, stTMSG *, stLAYINF *);
static int foiPrintXcd3xx (struct s_group_22 *, stTMSG *, stLAYINF *);

#if 0
void       fovdadd_str( char *, char *);
#endif

void          fovdFreePrepMemory(void);
void          *fpvdGetPrepMemory(long size);
static int    read_fix_array(void);
static void   get_next_line(FILE *, int);
static int    check_version( FILE *, int );
void          fsAnonymizeDigitsX (char *, char);
void          Anonymize (char *, int);
static int    set_fix_array (stTMSG *);
static short  format_date_fields (stTMSG *);
static char   *form_date (char *);
static void   lovdPrintInvoice (stTMSG *, stLAYINF *);
 
static void    print_header_block (stTMSG *, stLAYINF *);
static void    print_foot_block (stTMSG *, stLAYINF *);
static void    fovdPrintSender (stTMSG *, stLAYINF *);

static void             fovdPrintRec (stTMSG *, stLAYINF *);
static short            foiPrintSumTotal (stTMSG *, stLAYINF *);
static struct s_group_22 *fostPrintOther (struct s_group_22 *, short *, stLAYINF *);
static struct s_group_22 *fostPrintSubad (struct s_group_22 *, stLAYINF *);
static struct s_group_22 *fostPrintSim (stTMSG *, struct s_group_22 *, stLAYINF *);
static int              foiPrintSumIds (stTMSG *, short *, stLAYINF *);
static short            fovdPrintSum (stTMSG *, stLAYINF *);
static short            foiPrintSumBlock (stTMSG *, short, stLAYINF *);
static void             fovdPrintTitle (int, stTMSG *, stLAYINF *);
static void             fovdPrintItb (stTMSG *, stLAYINF *);
static void             fovdPrintItbEmpty (stTMSG *, stLAYINF *);
static void             fovdPrintItbDate (stTMSG *, stLAYINF *);
static char             *foszFormTime (char *elem);
static char             *foszFormDur (char *elem);
static int              foiPrintRoam (stTMSG *, stLAYINF *);
static void             fovdPrintBal (stTMSG *, stLAYINF *);
static void 		fovdPrintLeg (stTMSG *, stLAYINF *);
static void 		fovdPrintAdv (stTMSG *timm, stLAYINF *pLay);




/******************************************************************************
 * PrintVersInfoPostPars
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
void PrintVerInfoPostPars (void)
{
  static char *SCCS_VERSION = "1.36";

  printf ("%s\t\t\t%s\n", __FILE__, SCCS_VERSION);
}


/******************************************************************************
 * BghPrepareData
 *
 * DESCRIPTION:
 * 
 *
 * PARAMETERS:
 *  stTIMM	*interchange		- TIMM-structure
 *  stLAYINF	*pLay			- layout information
 *
 * RETURNS:
 *  0 	- no error
 ******************************************************************************
 */
int BghPrepareData (stTIMM *interchange,
		    stLAYINF *pLay)
{
   int		iRc;
   char         *lpchChar;
   static BOOL  FirstPass = TRUE;
   stTMSG 	*timm;

   fovdPushFunctionName("BghPrepareData");

   iRc = 0;
   timm = interchange->timm;
   pszCustId = interchange->unb->v_0010;

   if (FirstPass == TRUE)
   {
     /* clear the pointerarray */
     memset (fixstring_table_all, 0, sizeof (fixstring_table_all));
     read_fix_array ();
     set_fix_array (timm);
     FirstPass = FALSE;
   }
   format_date_fields (timm); /* convert from YYMMDD to DD.MM.YY */


   lpchChar = timm->bgm->v_1000;

   /* check BGM.1000 to find out, which type of the bill we    */
   /* have here                                                */ 

   if (0 == strcmp (lpchChar, INVOICE))
   {
     /* ---------------------------------------------------------*/
     /*       invoice                                            */
     /* ---------------------------------------------------------*/

     /* print lin segments */
     iRc = foiPrintInvLinSegs (timm, pLay);

     if (iRc == 0)
     {
       /* if it worked, print header */
       lovdPrintInvoice (timm, pLay);
     }
   }
   else if (0 == strcmp( lpchChar, ITEMIZED_BILL))
   {
     /* ---------------------------------------------------------*/
     /*       itemized bill                                      */
     /* ---------------------------------------------------------*/

     /*print XCD segments                                        */

     iRc = foiPrintXcdItb (timm, pLay);

     fovdPrintItb (timm, pLay);

   }
   else if (0 == strcmp (lpchChar, ROAMING_DOC))
   {
     /* ---------------------------------------------------------*/
     /*       roaming page                                       */
     /* ---------------------------------------------------------*/

     /*     iRc = condense_lin_segs_roa( timm );*/

     iRc = foiPrintRoam (timm, pLay);

   }
   else if (0 == strcmp (lpchChar, BALANCE_DOC))
   {
     /* ---------------------------------------------------------*/
     /*       balance page                                       */
     /* ---------------------------------------------------------*/

     fovdPrintBal (timm, pLay);

   }
   else if (0 == strcmp (lpchChar, SUM_SHEET))
   {
     /* ---------------------------------------------------------*/
     /*       sumsheet                                           */
     /* ---------------------------------------------------------*/

     fovdPrintSum (timm, pLay);
   }
   else if (0 == strcmp (lpchChar, LEGEND))
   {
     /* ---------------------------------------------------------*/
     /*       legend for an itemized bill                        */
     /* ---------------------------------------------------------*/

     fovdPrintLeg (timm, pLay);
   }
   else
   {
     /* ---------------------------------------------------------*/
     /*       undefined document type                            */
     /* ---------------------------------------------------------*/
     sprintf (laszErrMsg,
	      "DATA PREPARATION: document type %s undefined",
	      timm ->bgm->v_1000);
     macErrorMessage (PREP_UNDEF_DOCTYPE, WARNING, laszErrMsg );
     iRc = (int) PREP_UNDEF_DOCTYPE;
   }

   fovdPopFunctionName();

   return (iRc);
} /* end of BghPrepareData       */ 


/******************************************************************************
 * fofGetStr
 *
 * DESCRIPTION:
 * search string in a structuretable and returns an index to the corresponding
 * entry in fixtext table
 * The last element in the structure table must have NULL pointers
 *
 * PARAMETERS:
 *  int		*piText			- return pointer
 *  char	*szSearch		- text to search
 *  stSTRS	**pstTable		- pointer to structure table
 *
 * RETURNS:
 *  TRUE if succesful
 ******************************************************************************
 */
BOOL fofGetStr (int *piText, char *szSearch, stSTRS pstTab[])
{
  int 	i;			/* loop counter */

  ASSERT (szSearch != NULL);
  ASSERT (pstTab != NULL);
  ASSERT (piText != NULL);

  /*
   * search through the table for the corresponding entry
   */
  *piText = 0;
  i = 0;

  while (pstTab[i].pszSea != NULL)
  {
    if (strcmp (szSearch, pstTab[i].pszSea) == 0)
    {
      /* found the element, return value */
      *piText = pstTab[i].iNew;
      return (TRUE);
    }
    i++;
  }
  return (FALSE);
}


/******************************************************************************
 * folStrAdd
 *
 * DESCRIPTION:
 * add contents of passed strings, return sum in first pointer and as return
 * value in long value
 *
 * PARAMETERS:
 * char		*paszSum	- buffer for the sum (return value)
 * long		lAddVal		- long value to add
 * int		nElem		- number of elements to add
 * ...				- list of pointers
 *
 * RETURNS:
 *  long value of sum
 ******************************************************************************
 */
long folStrAdd (char *paszSum, long lAddVal, int nElem, ...)
{
  va_list	ap;
  long		lSum, lInt;		/* integer representation of number */
  char		*pValue;
  char 		*pTmp;			/* temporary pointer 		    */
  char 		szNum[AMNTSTRING];	/* temporary number string 	    */
  short		iLen;			/* string len 			    */
  char		*pOut;
  int		i, j;
  short		iFracSize = 0;		/* number of chars. after dot       */

  lSum = lAddVal;

  ASSERT (paszSum != NULL);
  ASSERT (nElem != 0);

  pOut = paszSum;

  va_start (ap, nElem);

  for (i = 0; i < nElem; i++)
  {
    pValue = va_arg (ap, char *); 	/* fetch next parameter */

    if (pValue != NULL)
    {
      /*
       * Search dot, copy integer part to temp. string, then append
       * fractional part without dot; the fractional part is always
       * expanded to 4 digits:
       * "145.60" is converted to "1456000"
       */
      pTmp = strchr (pValue, '.');

      if (pTmp != NULL)
      {
	j = pTmp - pValue;
	strncpy (szNum, pValue, j);

	szNum[j] = '\0';

	pTmp++;				/* set pointer past '.' 	*/

	iFracSize = strlen (pTmp);	/* remember digits after dot    */

	strcat (szNum, pTmp);		/* concatenate strings without dot */

	/* append '0' to length of 4 fractional digits */
	strncat (szNum, FRACNULLS, MAXFRAC - iFracSize);

	lInt = atol (szNum);
      }
      else
      {
	/* string is pure integer -> convert it */
	lInt = FRACPOT * atol (pValue);
      }
      /*
       * Now add the values...
       */
      lSum += lInt;
    }
  }
  va_end (ap);

  /*
   * convert integer to ASCII
   */
  if (iFracSize == 0)
  {
    /* just integers, easy */
    sprintf (pOut, "%ld", lSum / FRACPOT);
  }
  else
  {
    /* take care of negative numbers */
    if (lSum < 0) {
      lSum = -lSum;
      lInt = lSum % FRACPOT;
      sprintf (pOut, "-%ld.%0*ld", lSum / FRACPOT, MAXFRAC, lInt);
    } else {
      lInt = lSum % FRACPOT;
      sprintf (pOut, "%ld.%0*ld", lSum / FRACPOT, MAXFRAC, lInt);
    }

    /* cut it off at the desired position */
    iLen = strlen (pOut);
    pOut[iLen + iFracSize - MAXFRAC] = '\0';
  }
  return (lSum);
}
/*---------------------------------------------------------------------------*/


/******************************************************************************
 * fovdPrintG22Element
 *
 * DESCRIPTION:
 * write one of the following parts to the output:
 *  subscription
 *  access
 *  usage, national inland
 *  usage, international inland
 *  usage, international abroad
 *  other elements
 *
 *
 * PARAMETERS:
 *  stTMSG	*timm_message		- TIMM-structure
 *  char	chr			- usage type: 'S','A','U','O'
 *  int         lay_el_group		- layout number of group line
 *  stLAYINF	*pLay			- layout information
 *
 * RETURNS:
 *  - void -
 ******************************************************************************
 */
static void fovdPrintG22Element (stTMSG			*timm,
				 struct s_group_22 	*g_22_s,
				 char              	chr,
				 int                    lay_el_group,
				 stLAYINF          	*pLay)
{
  char *szItemDescr;               /* temporary pointer to item description */

  struct s_group_23 *g_23;
  struct s_group_25 *g_25;
  struct s_group_30 *g_30;
  struct s_moa_seg  *moa_125;
  struct s_moa_seg  *moa_203;
  struct s_moa_seg  *moa_124;
  struct s_moa_seg  *moa_919;
  struct s_tax_seg  *tax;
  struct s_imd_seg  *imd;
  struct s_imd_seg  *imd_fee;
  struct s_pri_seg  *pri;
  struct s_qty_seg  *qty;
  struct s_pia_seg  *pia;
  struct s_cux_seg  *cux;
  struct s_lin_seg  *lin;
  char		*pszMoa124;		/* pointer to moa 124 amount	*/
  char		*pszTaxAmnt;		/* pointer to tax amount	*/

  fovdPushFunctionName ("fovdPrintG22Element");

   /* initialization */

  g_23        = NULL;
  g_25        = NULL;
  moa_125     = NULL;
  moa_203     = NULL;
  moa_124     = NULL;
  moa_919     = NULL;
  tax         = NULL;
  imd         = NULL;
  pri         = NULL;
  qty         = NULL;
  pia         = NULL;
  cux	      = NULL;
  pszMoa124   = NULL;
  pszTaxAmnt  = NULL;

  szItemDescr = NULL;            /* initialize temporary string pointer */

  /* LIN is always present, otherwise this would not be a group 22 */
  lin = g_22_s->lin;

  /* get pointer imd */
  imd = g_22_s->imd;
  while (imd)
  {
    if (strcmp (imd->v_7009, IMD_SERVICE) == 0)		break;
    imd = imd->imd_next;
  }
  /* get pointer for imd fees */
  imd_fee = g_22_s->imd;
  while (imd_fee)
  {
    if (strcmp (imd_fee->v_7009, IMD_SERVICE_FEE) == 0)	break;
    imd_fee = imd_fee->imd_next;
  }  
  if (imd == NULL && imd_fee == NULL)
  {
    sprintf (laszErrMsg,
	     "INVOICE: LIN %s No service description available.",
	     lin->v_1082);
    macErrorMessage (PREP_IMD_NO_SERVICE_DESCR, WARNING, laszErrMsg);
  }

  /* get pointer pri if charge type is subscription or access */
  if ((chr == 'S') || (chr == 'A'))
  {
    g_25 = g_22_s->g_25;
    if (g_25)
    {
      pri = g_25->pri;

      /* get the currency information */
      if (timm->g_7) {
	cux = timm->g_7->cux;
      }
    }
    else
    {
      if (chr == 'S') {
	sprintf (laszErrMsg,
		 "INVOICE: LIN %s No subscription price available.",
		 lin->v_1082);
	macErrorMessage (PREP_PRI_NO_PRICE, WARNING, laszErrMsg);
      } else {
	sprintf (laszErrMsg,
		 "INVOICE: LIN %s No access price available.",
		 lin->v_1082);
	macErrorMessage (PREP_PRI_NO_PRICE, WARNING, laszErrMsg);
      }
    }
  }

  /* get pointer qty */
  qty = g_22_s->qty;
  while (qty)
  {
    if (strcmp (qty->v_6063, QUANTITY) == 0)	break;
    qty = qty->qty_next;
  }
  if (!qty)
  {
    sprintf (laszErrMsg,
	     "INVOICE: LIN %s No quantity available.",
	     lin->v_1082);
    macErrorMessage (PREP_NO_QTY, WARNING, laszErrMsg);
  }

  /* get pointer moa_125 */
  g_23 = g_22_s->g_23;
  while (g_23) {
    /* taxable amount subject to final payment (not rounded) */
    if ((strcmp (g_23->moa->v_5025,"125") == 0)	&&
	(strcmp (g_23->moa->v_4405,"5") == 0)) {
	break;
    }
    g_23 = g_23->g_23_next;
  }
  if (g_23) {
    moa_125 = g_23->moa;
  } else {
    sprintf (laszErrMsg,
	     "INVOICE: LIN %s No amount of item without VAT available.",
	     lin->v_1082);
    macErrorMessage (PREP_MOA_NO_NETAMOUNT, WARNING, laszErrMsg);
  }

  /* get pointer moa_203 */
  g_23 = g_22_s->g_23;
  while (g_23 && g_23->moa) {
    if (strcmp (g_23->moa->v_5025, "203") == 0)	break;
    g_23 = g_23->g_23_next;
  }
  if (g_23) {
    moa_203 = g_23->moa;
  } else {
    sprintf (laszErrMsg,
	     "INVOICE: LIN %s No amount of item with VAT available.",
	     lin->v_1082);
    macErrorMessage (PREP_MOA_NO_GROSAMOUNT, WARNING, laszErrMsg);
  }

  /* get pointer moa_919 - Discount */
  g_23 = g_22_s->g_23;
  while (g_23 && g_23->moa) {
    if (strcmp (g_23->moa->v_5025, "919") == 0)	break;
    g_23 = g_23->g_23_next;
  }
  if (g_23) {
    moa_919 = g_23->moa;
  }

  /* get pointer tax; tax segment is mandatory in g_30 */
  g_30 = g_22_s->g_30;
  if (g_30) {
    tax = g_30->tax;
    pszTaxAmnt = g_30->tax->v_5278;
  } else {
    sprintf (laszErrMsg,
	     "INVOICE: LIN %s No tax rate available.",
	     lin->v_1082);
    macErrorMessage (PREP_NO_TAX_RATE, WARNING, laszErrMsg);
  }

  /* get pointer moa_124 */
  g_30 = g_22_s->g_30;
  while (g_30 && g_30->moa) {

    if (strcmp (g_30->moa->v_5025, "124") == 0)	{

      if (strcmp (g_30->moa->v_4405, "5") == 0) {
	/*
	** tax amount "subject to final payment"
	*/
	pszMoa124 = g_30->moa->v_5004;
	break;
      } else if (strcmp (g_30->moa->v_4405, "9") == 0) {
	/*
	** Customers which do not have to pay taxes
	** do not have a tax amount "subject to final payment"
	*/
	pszMoa124 = (char *) fpvdGetPrepMemory (8);
	strcpy (pszMoa124, "0.00");

	/*
	** We have to reset the tax rate when we have no MOA for final payment
	*/
	if (tax != NULL) {
	  pszTaxAmnt = (char *) fpvdGetPrepMemory (8);
	  strcpy (pszTaxAmnt, "0.000");
	}
	break;
      }
    }
    g_30 = g_30->g_30_next;
  }
  if (g_30) {
    moa_124 = g_30->moa;
  } else {
    sprintf (laszErrMsg,
	     "INVOICE: LIN %s No amount of items tax available.",
	     lin->v_1082);
    macErrorMessage (PREP_MOA_NO_VAT_FOR_ITEM, WARNING, laszErrMsg);
  }

  if (chr == 'U') {
    pia = g_22_s->pia;
    while (pia) {
      if (strcmp(pia->v_4347, "1") == 0)		break;
      pia = pia->pia_next;
    }
    if (!pia) {
      sprintf (laszErrMsg,
	       "INVOICE: LIN %s No additional identification available.",
	       lin->v_1082);
      macErrorMessage (PREP_PIA_NO_ADD_INFO, WARNING, laszErrMsg);
    }
  }

  if (imd != NULL) {
    szItemDescr = imd->v_7008a;      /* save pointer to item description */
  } else if (imd_fee != NULL) {
    szItemDescr = imd_fee->v_7008a;  /* save pointer to fee description  */
  } else {
    /* no imd and no imd_fee available... */
    szItemDescr = "---";
  }

  /* add group line to layout */
  vdAddElement (pLay, lay_el_group, 15,
	        szItemDescr,			/* item description */
		COND_FETCH (pri,v_5118),	/* price            */
		COND_FETCH (cux,v_6345),	/* currency code    */
		COND_FETCH (qty,v_6060),	/* quantity         */
		COND_FETCH (moa_125,v_5004),	/* item without tax */
		COND_FETCH (moa_125,v_6345),	/* currency code    */
		VAL_ONE    (pszTaxAmnt),	/* tax rate         */
		VAL_ONE    (pszMoa124),		/* VAT for item     */
		COND_FETCH (moa_124,v_6345),	/* currency code    */
		COND_FETCH (moa_203,v_5004),	/* item with VAT    */
		COND_FETCH (moa_203,v_6345),	/* currency code    */
		COND_FETCH (lin, v_7140), 	/* Article code     */
		COND_FETCH (pia, v_7140), 	/* Subarticle code  */
		COND_FETCH (moa_919,v_5004),	/* discount 	    */
		COND_FETCH (moa_919,v_6345)	/* currency code    */
		);

   fovdPopFunctionName();
}
/*---------------------------------------------------------------------------*/


/****************************************************************************
 * foiPrintInvLinSegs - print lin segments of invoice message       
 *
 * DESCRIPTION:
 * After parsing there exists only one linked list timm->g_22, which contains
 * all g_22 groups.
 * The function foiPrintInvLinSegs prints the g_22 groups within this
 * linked list according to the contents of the lin-, imd- and pia-segments.
 *
 * RETURNS:
 *   0     -  Success
 *   != 0  -  Error
 ******************************************************************************
 */
static int foiPrintInvLinSegs (stTMSG *timm, stLAYINF *pLay) {
  int  loiRc;                /* return code                */
  UINT i;                    /* count variable             */
  UINT j;                    /* count variable             */
  UINT len;                  /* string length              */
  char *lpchChar;            /* pointer for strrchr        */ 
  char *lpchChar1;           /* 2. pointer                 */
  struct s_group_22          *spstTmp;       /* temporary pointer to list  */
  struct s_group_22          *spstTmpElement;/* temporary pointer to one   */
                                             /* specific list element      */
 
   BOOL	fSub1,			/* first element->header 	*/
        fSub2,			/*  ...				*/
        fSub3,			/*  ...				*/
        fSub4,			/*  ...				*/
        fSub5,			/*  ...				*/
        fSub6;			/*  ...				*/

   struct s_imd_seg           *spstImd;       /* IMD segment with SN code   */
   enum   loenPIAIdentifier { CTI = 0,
                              PpU,      /* price per unit                   */
                              PLDES,    /* short description of VPLMN       */
                              TTDES=2,  /* short description of tariff time */
                              ZNDES };  /*short description of tariff zone  */

   char   laszPIA7140val[4][6];

   /* The array laszPIA7140val contains the values of                        */
   /* pia field 7140 separated in 3 or 4 different strings:                  */
   /* 1.) type of call {A,I,C,R}                                             */
   /* 2.) price per unit indicator {R,r,m,F,B,S,T}                           */
   /* 3.) either the VPLMN short description or the tariff time short descr. */
   /* 4.) tariff zone short description                                      */

   char	cType;			/* type indicator: Charge type from LIN.7140 */
   UINT loisizeofPIA1;		/* size of whole array        */
   UINT loisizeofPIA2;		/* size of one string         */
   UINT loisizeofPIA3;		/* number of strings          */

   fovdPushFunctionName("foiPrintInvLinSegs");

   /* initialize temporary pointers */

   fSub1 = TRUE;
   fSub2 = TRUE;
   fSub3 = TRUE;
   fSub4 = TRUE;
   fSub5 = TRUE;
   fSub6 = TRUE;

   loiRc     = 0;
   spstTmp   = timm->g_22;

   loisizeofPIA1 = sizeof (laszPIA7140val);
   loisizeofPIA2 = sizeof (laszPIA7140val[0]);
   loisizeofPIA3 = loisizeofPIA1 / loisizeofPIA2;

   while (spstTmp != NULL && loiRc == 0) { /* for all groups 22   */
     ASSERT (spstTmp != NULL);
     if (spstTmp->lin->v_7140[0] == '\0') {
       /*
        * LIN.7140 does not exist, so it should be a fee, which should
        * be printed in the 'others' section
        */
       cType = CHARGE_TYPE_OTHER;
       len = 0;
     } 
     else {
       len   = strlen (spstTmp->lin->v_7140);
       cType = spstTmp->lin->v_7140[len - 1];
     }
     spstTmpElement = spstTmp;
     spstTmp        = spstTmp->g_22_next;

     switch (cType) {
     case CHARGE_TYPE_SUBSCRIPTION :
       /* print elements */

       if (fSub1) {
         /* first subscription line->print header as well */
         vdAddElement (pLay, LAY_INV_FIX_SUBSCRIPTION, 1, FIXTEXT (FIX_LIN_S));
         fSub1 = FALSE;
       }
       fovdPrintG22Element (timm, spstTmpElement, 'S', LAY_INV_ITEMS_SUBSCRIPTION, pLay);
       break;

     case CHARGE_TYPE_ACCESS :
       /* print elements */

       if (fSub2) {
         /* first subscription line->print header as well */
         vdAddElement (pLay, LAY_INV_FIX_ACCESS, 1, FIXTEXT (FIX_LIN_A));
         fSub2 = FALSE;
       }
       fovdPrintG22Element (timm, spstTmpElement, 'A', LAY_INV_ITEMS_ACCESS, pLay);
       break;

     case CHARGE_TYPE_USAGE :
       /* look for IMD segment with SN-code                       */
       spstImd = spstTmpElement->imd;

       /* find any imd segment with 7009 == IMD_SERVICE_CODE     */
       while (spstImd != NULL) {
         if (strcmp (spstImd->v_7009, IMD_SERVICE_CODE_ID) == 0)
           break;
         spstImd = spstImd->imd_next;
       } 
       lpchChar = NULL;
       if (spstTmpElement->pia) {
         lpchChar = spstTmpElement->pia->v_7140;
       }
       if ((lpchChar == NULL) || (lpchChar[0] == '\0')) {
         /* first pia segment not empty */
         /* field C212.7140 of first pia segment not empty */
         macErrorMessage (PREP_UNDEF_PIA_7140, WARNING, "INVOICE: PIA.7140: no value avaible");
         loiRc = (int) PREP_UNDEF_PIA_7140;
       } 
       else {
         /* split field 7140 of the first pia segment */
         lpchChar1 = lpchChar;

         for (j = 0; j < loisizeofPIA3; ++j) {
           laszPIA7140val[j][0] = '\0';
         }
         i = 0; /* start with first part */
         while ((lpchChar != NULL) && (i < loisizeofPIA3)) {
           j = strcspn (lpchChar1, ".");
           if (j >= loisizeofPIA2) { /* check for max. string size */
             macErrorMessage (PREP_ILL_PIA_7140, WARNING, "INVOICE: PIA.7140 subvalue too large");
             j = loisizeofPIA2 - 1;
           }
           /* copy first part */
           strncpy (laszPIA7140val[i], lpchChar1, j);
           laszPIA7140val[i][j] = '\0';
           lpchChar  = strchr (lpchChar1, '.');/* get index of next '.'*/
           lpchChar1 = lpchChar + 1;
           ++i;                            /* next part            */
         } /* while */


#if 1
         /* different grouping method */
         if (fSub3) {
           /* first subscription line->print header as well */
           vdAddElement (pLay, LAY_INV_FIX_NAT_USAGE, 1, fixstring[FIX_LIN_UI]);
           fSub3 = FALSE;
         }

         fovdPrintG22Element (timm, spstTmpElement, 'U', LAY_INV_ITEMS_NAT_USAGE, pLay);
#else
         /* assign the group 22 to the corresponding queue           */

         /* normal usage */
         if ((laszPIA7140val[PpU][0] == PpU_FLAT) 	|| 
             (laszPIA7140val[PpU][0] == PpU_BUNDLED) 	||
             (laszPIA7140val[PpU][0] == PpU_SCALED) 	||
             (laszPIA7140val[PpU][0] == PpU_TIERED)) {
           
#if 0 /* that's the way it is in Mobia... */
           if (laszPIA7140val[ZNDES][0] == '\0') {
             /* for this type always a ZNDES should be available */
             /* insert them as international calls/fees in HPLMN */
             if (fSub6) {
               /* first subscription line->print header as well */
               vdAddElement (pLay, LAY_INV_FIX_INT_USAGE_INL, 1, fixstring[FIX_LIN_UR]);
               fSub6 = FALSE;
             }
             fovdPrintG22Element (timm, spstTmpElement, 'U', LAY_INV_ITEMS_INT_USAGE_INL, pLay);
             
#if 0
             macErrorMessage (PREP_ILL_PIA_7140, WARNING, "INVOICE: PIA.7140: no value for ZNDES");
#endif
           }
           else
#endif
             {
               /* national call                  */
               if (fSub3) {
                 /* first subscription line->print header as well */
                 vdAddElement (pLay, LAY_INV_FIX_NAT_USAGE, 1, fixstring[FIX_LIN_UI]);
                 fSub3 = FALSE;
               }
               fovdPrintG22Element (timm, spstTmpElement, 'U', LAY_INV_ITEMS_NAT_USAGE, pLay);
             }
         }
         else if ((laszPIA7140val[PpU][0] == PpU_VPLMN_CHARGES_TA_MOC) || (laszPIA7140val[PpU][0] == PpU_VPLMN_CHARGES_TA_MTC)) {
           /* charges from foreign network provider */
           if (fSub4) {
             /* first subscription line->print header as well */
             vdAddElement (pLay, LAY_INV_FIX_INT_USAGE_ABROAD, 1, fixstring[FIX_LIN_UA]);
             fSub4 = FALSE;
           }
           fovdPrintG22Element (timm, spstTmpElement, 'U', LAY_INV_ITEMS_INT_USAGE_ABROAD, pLay);
           
         } else if (laszPIA7140val[PpU][0] == PpU_HPLMN_SURCHARGE_TA_MOC) {
           /* roaming surcharge */
           if (fSub6) {
             /* first subscription line->print header as well */
             vdAddElement (pLay, LAY_INV_FIX_INT_USAGE_INL, 1, fixstring[FIX_LIN_UR]);
             fSub6 = FALSE;
           }
           fovdPrintG22Element (timm, spstTmpElement, 'U', LAY_INV_ITEMS_INT_USAGE_INL, pLay);
         } 
         else {
           sprintf (laszErrMsg, "INVOICE: PIA.7140  <%c> not a correct PpU value", laszPIA7140val[PpU][0]);
           macErrorMessage (PREP_UNDEF_PIA_7140_PpU, WARNING, laszErrMsg);
           loiRc = (int) PREP_UNDEF_PIA_7140_PpU;
         }
#endif /* else */
       }
       break;
     case CHARGE_TYPE_OTHER :
       if (fSub5) {
         /* first subscription line->print header as well */
         vdAddElement (pLay, LAY_INV_FIX_OTHER_CHARGES, 1, fixstring[FIX_LIN_O]);
         fSub5 = FALSE;
       }
       fovdPrintG22Element (timm, spstTmpElement, 'O', LAY_INV_ITEMS_OTHER_CHARGES, pLay);
       break;
     default:
       sprintf (laszErrMsg, "INVOICE: LIN contains undefined charge type '%c'", cType);
       macErrorMessage (PREP_UNDEF_PIA_7140_CHARGE_TYPE, WARNING, laszErrMsg );
       loiRc = (int) PREP_UNDEF_PIA_7140_CHARGE_TYPE;
       break;
     } /* switch */
   } /* while */

   fovdPopFunctionName();
 
   return (loiRc);
}
/*---------------------------------------------------------------------------*/


#if 0
/******************************************************************************
 * fovdadd_str
 *
 * DESCRIPTION:
 * add contents of paszStr2 to paszStr1
 *
 * PARAMETERS:
 * char		*paszStr1
 * char         *paszStr2
 *
 * RETURNS:
 *  - void -
 ******************************************************************************
 */
void fovdadd_str( char *paszStr1,      /* floating point number as a string */
                  char *paszStr2)      /* floating point number as a string */
{
   double lofoFloat1;
   double lofoFloat2;
   char   *lpchChar;

   fovdPushFunctionName("fovdadd_str");

   lofoFloat1 = atof( paszStr1 );
   lofoFloat2 = atof( paszStr2 );

   lofoFloat1 = lofoFloat1 + lofoFloat2;

   sprintf( paszStr1, "%-18.2f", lofoFloat1 );

   /* skip trailing blanks                                     */

   lpchChar = strchr( paszStr1, ' '); 
   if( lpchChar != NULL )
   {
      *lpchChar = '\0';
   }

   fovdPopFunctionName();

   return;
}
/*---------------------------------------------------------------------------*/
#endif

/*****************************************************************************
 * fpvdGetPrepMemory -  return pointer to small dynamically allocated memory
 * 			block for segment buffer
 *
 * DESCRIPTION:
 * For performance reasons this functions internally dynamically allocates
 * large (size MEMORY_BLOCK_SIZE) linked memory blocks and on request returns
 * small blocks of it to the calling functions.
 *
 * RETURNS:
 * Pointer to allocated small memory block.
 ******************************************************************************
 */
void *fpvdGetPrepMemory(long size)
{
  static long                   current_size_left;/* no of free bytes within */
                                                  /* current large block     */
  static struct LARGE_MEM_BLOCK *large_block;     /* pointer to begin of     */
                                                  /* actual large block      */
  static unsigned char          *current_next;    /* pointer to next free    */
                                                  /* small block             */
  struct LARGE_MEM_BLOCK        *p;               /* temporary pointer       */
  void                          *sb;        /* pointer to small memory block */
  long   even_size;


  fovdPushFunctionName("fpvdGetPrepMemory");

  even_size = (size + 0x0F) & 0xFFFFFFF0;
  /* check requested size */
  if (even_size > MEMORY_BLOCK_SIZE-sizeof(p))
  {
    macErrorMessage (PREP_ILL_MALLOC_REQUEST, CRITICAL,
                     "size too big, increase MEMORY_BLOCK_SIZE and recompile");
  }

  /* --- allocate large internal memory block --- */
  if (allocated_memory == NULL)
  {
     p = (struct LARGE_MEM_BLOCK*)malloc(MEMORY_BLOCK_SIZE);
     if (p != NULL)
     {
        allocated_memory   = p;
        large_block        = p;
        p->next            = NULL;
        current_next       = p->buf;
        current_size_left  = MEMORY_BLOCK_SIZE - sizeof(p);
     }
     else
     {
       fovdFreePrepMemory(); /* free all allocated dynamic memory   */

       macErrorMessage (PREP_MALLOC_FAILED, CRITICAL,
                        "can't allocate additional memory");
     }
  }
  else if (even_size > current_size_left)
  {
     p = (struct LARGE_MEM_BLOCK*)malloc(MEMORY_BLOCK_SIZE);
     if (p != NULL)
     {
        large_block->next  = p;    /* set pointer to next large block */
        large_block        = p;    /* define new current large block  */
        p->next            = NULL;
        current_next       = p->buf;
        current_size_left  = MEMORY_BLOCK_SIZE - sizeof(p);
     }
     else
     {
       fovdFreePrepMemory(); /* free all allocated dynamic memory  */

       macErrorMessage (PREP_MALLOC_FAILED, CRITICAL,
                        "can't allocate additional memory");
     }
  }

  /* --- allocate a small memory block --- */
  sb                = current_next;
  current_next      = current_next + even_size;
  current_size_left = current_size_left - even_size;

  fovdPopFunctionName();

  return (sb);
}

/*****************************************************************************
 * fovdFreePrepMemory - release all dynamically allocated
 *                               memory blocks
 *
 * DESCRIPTION:
 * All memory blocks which were dynamically allocated by function
 * fpvdGetPrepMemory are released.
 *
 * RETURNS:
 * none
 ******************************************************************************
 */
void fovdFreePrepMemory(void)
{
  /* free all allocated dynamic memory */
  struct LARGE_MEM_BLOCK *p1, *p2;

  fovdPushFunctionName("fovdFreePrepMemory");

  p1 = allocated_memory;    /* get pointer to 1. allocated memory block */

  /* --- free large internal memory block --- */
  while (p1 != NULL)
  {
    p2 = p1->next;          /* save pointer to next allocated memory block */
    free(p1);               /* free memory block                           */
    p1 = p2;                /* copy pointer to next memory block           */
  }

  allocated_memory = NULL;	/* reset for repeated use.. */
  fovdPopFunctionName();
}
/*---------------------------------------------------------------------------*/


/******************************************************************************
 * foiReadLanguage
 *
 * DESCRIPTION:
 * read a language file into the fixed arrays
 *
 * PARAMETERS:
 *  int		iLang		- language (array index)
 *  char	*szFile		- filename
 *
 * RETURNS:
 *  int 	- error code
 ******************************************************************************
 */
static int foiReadLanguage (int iLang, char *szFile)
{
   FILE *filep;
   int  ex;
   int  linenumber;
   int  version;

   fovdPushFunctionName("fovdReadLanguage");

   /* initialisation                                            */
   ex    = 0;
   filep = NULL;

   if ((ex == 0) && ((filep = fopen(szFile, "r")) == NULL))
   {
     sprintf (laszErrMsg, "cannot open fixed string file: %s", szFile);
     macErrorMessage (PREP_FIXED_UNDEF, CRITICAL,
		      laszErrMsg);
     ex  = (int) PREP_FIXED_UNDEF;
   }

   if (ex == 0)
   {
     version = check_version(filep, iLang);
     if (version != 0)
     {
       sprintf (laszErrMsg, "wrong version of fixed string file: %s", szFile);
       macErrorMessage (PREP_FIXED_WRONG_VERSION,
			CRITICAL,laszErrMsg );
       ex  = (int) PREP_FIXED_WRONG_VERSION;
     }
   }

   if (ex == 0)
   {
     ASSERT ((iLang < MAX_LANGUAGES) && (iLang >= 0));
     fixstring = (fixstring_table_all[iLang]);
     for (linenumber = 1; linenumber <= MAXFIXSTRNUM; linenumber++)
     {
       get_next_line (filep, linenumber);
     }
   }

   if (filep != NULL)
   {
     ex = fclose (filep);
   }

   fovdPopFunctionName();

   return (ex);
}
/*---------------------------------------------------------------------------*/


/******************************************************************************
 * read_fix_array
 *
 * DESCRIPTION:
 * read all language files into the fixed arrays
 *
 * PARAMETERS:
 *  void
 *
 * RETURNS:
 *  int 	- error code
 ******************************************************************************
 */
static int read_fix_array(void)
{
   int  ex;
   char filename[PATH_MAX];

   fovdPushFunctionName("read_fix_array");

   if (stBgh.szLayoutDir[0] == '\0') {
     strcpy (filename, FIX_DIR);
   } else {
     strcpy (filename, stBgh.szLayoutDir);
   }
   strcat (filename, ENGLTXT);
   ex = foiReadLanguage (ENGLTAG, filename);

   if (ex == 0)
   {
     if (stBgh.szLayoutDir[0] == '\0') {
       strcpy (filename, FIX_DIR);
     } else {
       strcpy (filename, stBgh.szLayoutDir);
     }
     strcat (filename, GERMTXT);
     ex = foiReadLanguage (GERMTAG, filename);
   }
#if LANG_FRENCH
   if (ex == 0)
   {
     if (stBgh.szLayoutDir[0] == '\0') {
       strcpy (filename, FIX_DIR);
     } else {
       strcpy (filename, stBgh.szLayoutDir);
     }
     strcat (filename, FRENTXT);
     ex = foiReadLanguage (FRENTAG, filename);
   }
#endif
#if LANG_ITALIAN
   if (ex == 0)
   {
     if (stBgh.szLayoutDir[0] == '\0') {
       strcpy (filename, FIX_DIR);
     } else {
       strcpy (filename, stBgh.szLayoutDir);
     }
     strcat (filename, ITALTXT);
     ex = foiReadLanguage (ITALTAG, filename);
   }
#endif

   fovdPopFunctionName();

   return (ex);
}
/*---------------------------------------------------------------------------*/


/******************************************************************************
 * set_fix_array
 *
 * DESCRIPTION:
 * set the fixstring-basepointer corresponding to the language of the TIMM
 * document
 *
 * PARAMETERS:
 *  stTIMM	*interchange		- TIMM-structure
 *
 * RETURNS:
 *  int		- error code
 ******************************************************************************
 */
static int set_fix_array (stTMSG *timm_message)
{
   struct s_group_2 *g_2;
   struct s_group_4 *g_4;
   BOOL   lofDefaultLanguage = FALSE;
   int    ex;
   char   *doc;
   char   filename[PATH_MAX];

   fovdPushFunctionName("set_fix_array");

   /* initialisation                                            */
   ex    = 0;
   g_2   = NULL;
   g_4   = NULL;
   doc   = NULL;

   /* getting pointers to data segments                         */

   g_2 = timm_message->g_2;

   if( g_2 != NULL) {

      while (g_2 != NULL) {
	ASSERT (g_2->nad != NULL);

	if (strcmp(g_2->nad->v_3035, NAD_RECEIVER) == 0)
	  break;
	g_2 = g_2->g_2_next;
      }
   }

   if (g_2 != NULL) {
      g_4 = g_2->g_4;
   } else {
     macErrorMessage (PREP_NAD_NO_5035, CRITICAL,
                      "No receiver specification at all");
     ex = (int) PREP_NAD_NO_5035;
   }

   /* get pointer to document specification */

   if( g_4 != NULL) {

      while (g_4) {
	ASSERT (g_4->doc != NULL);

	if (strcmp (g_4->doc->v_1001, "380") == 0)
	  break;
	g_4 = g_4->g_4_next;
      }
   }

   if (g_4 != NULL) {
      doc = g_4->doc->v_3453;
   } else {
     macErrorMessage (PREP_UNDEF_DOC_LANG, WARNING,
                      "No document language specified!");
     ex = (int) PREP_UNDEF_DOC_LANG;
   }
 
   strcpy (filename, FIX_DIR);

   if (doc != NULL) {

      if (0 == strcmp (doc, DOC_LAN_GER_C) ||
          0 == strcmp (doc, DOC_LAN_GER) ) {
         fixstring = fixstring_table_all[GERMTAG];
      } else if (0 == strcmp (doc, DOC_LAN_GB_C) ||
		 0 == strcmp (doc, DOC_LAN_GB)) {
         fixstring = fixstring_table_all[ENGLTAG];
      }
#if LANG_FRENCH
      else if (0 == strcmp (doc, DOC_LAN_FR_C) ||
	       0 == strcmp (doc, DOC_LAN_FR)) {
         fixstring = fixstring_table_all[FRENTAG];
      }
#endif
#if LANG_ITALIAN
      else if (0 == strcmp (doc, DOC_LAN_IT_C) ||
               0 == strcmp (doc, DOC_LAN_IT)) {
         fixstring = fixstring_table_all[ITALTAG];
      }
#endif
      else {
         lofDefaultLanguage = TRUE;
      }
   } else {
      lofDefaultLanguage = TRUE;
   }


   if (lofDefaultLanguage) {
     fixstring = fixstring_table_all[ENGLTAG];

     macErrorMessage (PREP_UNDEF_DOC_LANG, WARNING,
		      "wrong or no language specified for doc (default: english)");
     ex = (int) PREP_UNDEF_DOC_LANG;
   }

   fovdPopFunctionName();

   return (ex);
}


/*---------------------------------------------------------------------------*/


static void get_next_line(FILE *filep, int linenumber)
{
   int  i,z=0;
   int  ordNum;                 /* number which assigns texstring  */
   int  character;
   char numStr[4];
   char tmp[MAXFIXSTRLEN];

   /* initialization */
   memset(tmp,    (int) 0, sizeof(tmp));
   memset(numStr, (int) 0, sizeof(numStr));

   character = fgetc(filep); /* new */

   /* search for the next TEXTMARK character in this line       */
   while ( (character != TEXTMARK) && (character != '\n')
                                   && (character != EOF) )
   {
     if (isdigit(character))
     {
	if (z < sizeof (numStr)) {
	  numStr[z++] = character;
	}
     }
     character = fgetc(filep);
   }  /* while */
   ordNum = atoi(numStr);

   /* if the character is a newline than this line is empty     */
   /* read the next character to skip the newline and return    */
   if (character == '\n')
   {
      /* character = fgetc(filep); */
      return;
   }
   if (character != EOF)
   {
      i = 0;
      character = fgetc(filep);

      /* copy the characters till the next occurance of TEXTMARK   */
      while ((character != TEXTMARK) && (character != EOF))
      {
	if (i < sizeof (tmp) - 1) {
	  tmp[i] = character;
	  i++;
	}
	character = fgetc(filep);
      } /* while */
   } /* if */

   if (*tmp != 0)
   {
      fixstring[ordNum]   = malloc (strlen (tmp) + 1);

      if (fixstring[ordNum] == NULL) {
	macErrorMessage (PREP_MALLOC_FAILED, CRITICAL,
			 "can't allocate additional memory for fix-strings");
      }

      strcpy (fixstring[ordNum], tmp);
   }
   
   /* read the next character to skip the final TEXTMARK        */
   if (character != EOF)
   {
      character = fgetc(filep);
   }

/* if this character is a newline skip it                    */
   if ((character != EOF) && (character == '\n'))
   {
      /* character = fgetc(filep); */
   }

}
/*---------------------------------------------------------------------------*/


/******************************************************************************
 * fovdFreeFixstringMem
 *
 * DESCRIPTION:
 * free all memory allocated for the fixstrings
 *
 * PARAMETERS:
 *  void
 *
 * RETURNS:
 *   void
 ******************************************************************************
 */
void fovdFreeFixstringMem (void)
{
  int 	i, j;

  /*
   * loop over all languages and all fixstring
   * freeing them when necessary
   */
  for (i = 0; i < MAX_LANGUAGES; i++)
  {
    for (j = 0; j < MAXFIXSTRNUM; j++)
    {
      if (fixstring_table_all[i][j] != NULL)
      {
	free (fixstring_table_all[i][j]);
      }
    }
  }
  /* clear the array */
  memset (fixstring_table_all, 0, sizeof (fixstring_table_all));
}

/*************************************************************/
/* Functions for reading language depending fix strings.     */
/*************************************************************/

static int check_version( FILE *filep, int language )
{
   int  ex;
   int  i;
   char *ptr;
   char first_line[128];

   fovdPushFunctionName("check_version");

   /* initialisation                                            */
   ex = 0;
   ptr = NULL;

   /* read first line of language string with the version       */
   i = -1;
   do
   {
     i++;
     first_line[i] = fgetc (filep);
   } while (first_line[i] != '\n');

   first_line[i] = '\0';
   switch (language)
   {
     case GERMTAG:
     {
        ptr = strstr(first_line, GERMVER);
        break;
     }
     case ENGLTAG:
     {
        ptr = strstr(first_line, ENGLVER);
        break;
     }
#if LANG_FRENCH
     case FRENTAG:
     {
        ptr = strstr(first_line, FRENVER);
        break;
     }
#endif
#if LANG_ITALIAN
     case ITALTAG:
     {
        ptr = strstr(first_line, ITALVER);
        break;
     }
#endif
     default:
     {
        ex = WRONG_LANGUAGE;
     }
   }

   if ((ex != 0) || (ptr == NULL))
   {
      ex = WRONG_VERSION;
   }
   fovdPopFunctionName();

   return (ex);
}
/*---------------------------------------------------------------------------*/


/******************************************************************************
 * foiPrintXcdLine
 *
 * DESCRIPTION:
 * print a single XCD line
 * The following groups are distinguished:
 * 1. national GSM outbound home:
 *                   XCD.X017 = 'O'         - outbound
 *                   XCD.X018 = 'H'         - home
 *                   XCD.X021 = 'NV' || 'NP - national
 * 2. international GSM outbound home:
 *                   XCD.X017 = 'O'         - outbound
 *                   XCD.X018 = 'H'         - home
 *                   XCD.X021 = 'IV' || 'IP - international
 * 3. GSM outbound visitor:
 *                   XCD.X017 = 'O'         - outbound
 *                   XCD.X018 = 'V'         - visitor
 * 4. GSM inbound home:
 *                   XCD.X017 = 'I'         - inbound
 *                   XCD.X018 = 'H'         - home
 * 5. GSM inbound visitor:
 *                   XCD.X017 = 'I'         - inbound
 *                   XCD.X018 = 'V'         - visitor
 * 6. additional services:
 *                   XCD.X026 exists
 *
 * PARAMETERS:
 *  float	flThreshold		- threshold
 *  struct s_xcd_seg *spstXcd		- pointer to XCD
 *  stTMSG	*timm			- TIMM-structure
 *  stLAYINF	*pLay			- layout information
 *
 * RETURNS:
 *   0     -  Success
 *   != 0  -  Error
 ******************************************************************************
 */
static int foiPrintXcdLine (double flThreshold, struct s_xcd_seg *spstXcd,
			    stTMSG *timm, stLAYINF *pLay)
{
   int          loiRc;             	/* return code                  */
   short        lofOutbound;		/* TRUE : outbound call         */
					/* FALSE: inbound call          */
   short        lofVisitor;		/* TRUE : VPLMN network         */
					/* FALSE: HPLMN network         */  
   double   	flSurcha;

   loiRc = 0;

   flSurcha = atof (spstXcd->v_5004);

   /*
    * compare the sum with the threshold
    */
   if ((flSurcha < flThreshold) ||
       (flSurcha == 0.0))
   {
     /*  XCD total amount < threshold amount->drop XCD         */
     /*  XCD total amount == 0.0            ->drop XCD         */
     fovdPrintLog (LOG_CUSTOMER,
		   "XCD amount: %f; threshold: %f -> drop XCD\n",
		   flSurcha, flThreshold);
#if BELOWXCDTHRESHOLD
     return (loiRc);
#endif
   }

   /*
    * The BCH inserts '_' in test mode.
    * We have to delete this sign in an VAS remark, because we
    * recognize a VAS if a remark exists!
    */
   if ((spstXcd->v_X026[0] == '_')	&&
       (spstXcd->v_X026[1] == '\0')) {
     spstXcd->v_X026[0] = '\0';
   }


   if (0 == strcmp(spstXcd->v_X017, X017_OUTBOUND)) {
     lofOutbound = TRUE;
   } else {
     lofOutbound = FALSE;
   }

   if (0 == strcmp(spstXcd->v_X018, X018_VISITOR)) {
     lofVisitor = TRUE;
   } else {
     lofVisitor = FALSE;
   }

   if (spstXcd->v_X021[0] != '\0' && lofOutbound == TRUE)
   {
     /*    outbound call                                         */
     if (lofVisitor == TRUE)
     {
       /*    outbound calls from other GSM networks              */

       /* --- print GSM-outbound visitor --- */
       loiPrintXcdElem (spstXcd, LAY_ITB_ITEMS_TABLE_3, pLay);
     }
     else
     {
       /*    outbound calls from HPLMN                             */
       if (0 == strcmp (spstXcd->v_X021, X021_NAT_PLMN) ||
	   0 == strcmp (spstXcd->v_X021, X021_NAT_PSTN))
       {
	 /*    outbound national calls from HPLMN                  */

	 /* --- print national GSM-outbound home --- */
	 loiPrintXcdElem (spstXcd, LAY_ITB_ITEMS_TABLE_1, pLay);
       }
       else if (0 == strcmp (spstXcd->v_X021, X021_INT_PLMN) ||
		0 == strcmp (spstXcd->v_X021, X021_INT_PSTN))
       {
	 /*    outbound international calls from HPLMN              */

	 /* --- print international GSM-outbound home --- */
	 loiPrintXcdElem (spstXcd, LAY_ITB_ITEMS_TABLE_2, pLay);
       }
       else
       {
	 /*   undefined call destination                            */
	 sprintf (laszErrMsg, "ITB: xcd %s: undefined call destination",
		  spstXcd->v_X001);
	 macErrorMessage (PREP_XCD_UNDEF_CALL_DEST ,
			  WARNING, laszErrMsg );
	 loiRc = (int) PREP_XCD_UNDEF_CALL_DEST;
       }
     }
   }
   else if(spstXcd->v_X026[0] == '\0' && lofOutbound == FALSE )
   {
     if (lofVisitor == TRUE)
     {
       /*    inbound calls in other GSM networks                   */

       /* --- print GSM-inbound visitor --- */
       loiPrintXcdElem (spstXcd, LAY_ITB_ITEMS_TABLE_5, pLay);

     }
     else
     {
       /*    inbound calls in HPLMN                                */

       /* --- print GSM-inbound home --- */
       loiPrintXcdElem (spstXcd, LAY_ITB_ITEMS_TABLE_4, pLay);
     }
   }
   else if (spstXcd->v_X026[0] != '\0')
   {
     /*    other services                                        */

     /* --- print additional services --- */
     loiPrintXcdElem (spstXcd, LAY_ITB_ITEMS_TABLE_6, pLay);
   }
   else
   {
     macErrorMessage (PREP_XCD_UNDEF_CALL_TYPE, CRITICAL,
		      "ITB: xcd: call type is undefined");
     loiRc = (int) PREP_XCD_UNDEF_CALL_TYPE;
   }

   return (loiRc);
}


/******************************************************************************
 * foiPrintXcd
 *
 * DESCRIPTION:
 * print the XCD segments (itemized bill or sumsheet)
 *
 * PARAMETERS:
 *  struct s_group_22 *spstG22		- pointer to group 22
 *  stTMSG	*timm			- TIMM-structure
 *  stLAYINF	*pLay			- layout information
 *
 * RETURNS:
 *   0     -  Success
 *   != 0  -  Error
 ******************************************************************************
 */
static int foiPrintXcd (struct s_group_22 *spstG22,
			stTMSG *timm, stLAYINF *pLay)
{
   int               loiRc;             /* return code                  */
   struct s_xcd_seg  *spstTmp;		/* temporary pointer to XCD     */

   struct s_group_22 *spstG22Tmp; 	/* pointer to group 22 list 	*/
   struct s_group_99 *spstG99;		/* pointer to actual group 99	*/
#if CONDENSESPLITXCDS
   struct s_group_99 *spstSG99;		/* pointer to current group 99 with sub XCD */
   struct s_xcd_seg  *spstSub;		/* temporary pointer to sub XCD */
#endif
   struct s_group_23 *spstG23;
   char		     szSum[AMNTSTRING];	/* sum of values 		*/
   long		     lSum;
#if CONDENSESPLITXCDS
   long		     lClicks1, lClicks2;
#endif

   double	flThreshold;		/* values for calculating...	*/

   fovdPushFunctionName ("fovdPrintXcd");

   loiRc       = 0;

   /* something for FLINT ... */
   flThreshold = 0.0;
   spstG99 = NULL;

   /*
    * Check if the related layout elements exist,
    * stop right now if they do not exist.
    */
   if (! (fofLineExists (pLay, LAY_ITB_ITEMS_TABLE_1)	||
	  fofLineExists (pLay, LAY_ITB_ITEMS_TABLE_2)	||
	  fofLineExists (pLay, LAY_ITB_ITEMS_TABLE_3)	||
	  fofLineExists (pLay, LAY_ITB_ITEMS_TABLE_4)	||
	  fofLineExists (pLay, LAY_ITB_ITEMS_TABLE_5)	||
	  fofLineExists (pLay, LAY_ITB_ITEMS_TABLE_6)))
   {
     /*
      * No layout element for printing XCDs
      */
     fovdPopFunctionName ();
     return (0);
   }

   if (spstG22->g_99 == NULL)
   {
     /*
      * No XCD's in this Group 22
      */
     return (PREP_NO_XCDS);
   }

   /* Reset flags for header printing */
   fHeader1 = FALSE;
   fHeader2 = FALSE;
   fHeader3 = FALSE;
   fHeader4 = FALSE;
   fHeader5 = FALSE;
   fHeader6 = FALSE;


   /* Reset all totals */

   spstG22Tmp 	= spstG22;
   spstG99 	= spstG22Tmp->g_99;
   spstTmp 	= spstG99->xcd;

   /* now search threshold for XCDs */
   if (spstTmp != NULL) {
     spstG23 = spstG22Tmp->g_23;

     while (spstG23 != NULL) {

       if (strcmp (spstG23->moa->v_5025, MOA_THRESHOLD_VALUE) == 0) {
	 /* remember the Threshold */
	 flThreshold = atof (spstG23->moa->v_5004);
	 break;
       }
       spstG23 = spstG23->g_23_next;
     }

     /* if spstG23 is NULL then we havent found a threshold */
     if (spstG23 == NULL) {
       /*  group 23 moa segment with threshold does not exist      */
       loiRc = (int) PREP_G23_MOA_NO_THRESHOLD;
       macErrorMessage (PREP_G23_MOA_NO_THRESHOLD, WARNING,
			"ITB: group 23 MOA segment with threshold does not exist");
       flThreshold = 0.0;
     }

     while (spstTmp != NULL && loiRc == 0)
     {
       /*
	* for each XCD: 
	* calculate the total sum and write it back to the XCD
	* (add the VPLMN amount)
	*/
       lSum = folStrAdd (szSum, 0, 3, spstTmp->v_5004,
			 spstTmp->v_5004a, spstTmp->v_5004b);
#if 0
       /*
	* reset the actual values (remember: they are strings)
	*/
       spstTmp->v_5004[0]  = '\0';
       spstTmp->v_5004a[0] = '\0';
       spstTmp->v_5004b[0] = '\0';
#endif
#if CONDENSESPLITXCDS
       /*
	* look for splitted calls,
	* add their amounts (and clicks) as well
	* (look if there is another XCD with the same running main number
	* and a running sub number that is not 0, collect all subsequent
	* elements that fulfil this criteria; afterwards the pointer spstG99
	* is set to the last of these elements, so that the outer loop
	* skips the sub XCDs)
	*/
       lClicks1 = atol (spstTmp->v_X007);

       spstSG99 = spstG99;
       while (spstSG99->g_99_next != NULL) {
	 /* there is another XCD ... */
	 spstSub = spstSG99->g_99_next->xcd;

	 /* look if it is a sub number of the current XCD */
	 if ((strcmp (spstTmp->v_X001, spstSub->v_X001) != 0) ||
	     (atoi (spstSub->v_X002) == 0))
	 {
	   /* skip the sub-XCDs */
	   /* spstG99 = spstSG99; */
	   break;
	 }
	 lSum = folStrAdd (szSum, lSum, 3, spstSub->v_5004,
			   spstSub->v_5004a, spstSub->v_5004b);
	 /*
	  * reset the actual values (remember: they are strings)
	  */
	 spstSub->v_5004[0]  = '\0';
	 spstSub->v_5004a[0] = '\0';
	 spstSub->v_5004b[0] = '\0';

	 /* Add clicks */
	 lClicks2 = atol (spstSub->v_X007);

	 if (lClicks2 >= 0) {
	   lClicks1 += lClicks2;
	 }

	 spstSG99 = spstSG99->g_99_next;
       }

       sprintf (spstTmp->v_X007, "%ld", lClicks1);

       /* adjust the pointer */
       spstG99 = spstSG99;
#endif
       /* write the sum back to XCD */
       strcpy (spstTmp->v_5004, szSum);

       loiRc = foiPrintXcdLine (flThreshold, spstTmp, timm, pLay);

       /* proceed to next XCD */
       spstTmp = NULL;
       spstG99 = spstG99->g_99_next;
       if (spstG99 != NULL) {
	 spstTmp = spstG99->xcd;
       }
     }                       /* end of inner while loop    */

   } /* if (fFoundXcd).. */

   fovdPopFunctionName();

   return (loiRc);
}
/*---------------------------------------------------------------------------*/


/******************************************************************************
 * foiPrintXcdItb
 *
 * DESCRIPTION:
 * print the XCD segments for an itemized bill
 *
 * PARAMETERS:
 *  stTMSG	*timm			- TIMM-structure
 *  stLAYINF	*pLay			- layout information
 *
 * RETURNS:
 *   0     -  Success
 *   != 0  -  Error
 ******************************************************************************
 */
static int foiPrintXcdItb (stTMSG *timm, stLAYINF *pLay)
{

   int               loiRc;             /* return code                  */
   struct s_xcd_seg  *spstTmp;		/* temporary pointer to XCD     */

   struct s_group_22 *spstG22Tmp; 	/* pointer to sorted group 22 list */
   struct s_group_99 *spstG99;		/* pointer to actual group 99       */

   BOOL		fFoundXcd;


   fovdPushFunctionName ("fovdPrintXcdItb");

   loiRc       = 0;

   spstG99 = NULL;

   spstG22Tmp = timm->g_22;

   spstTmp = NULL;
   fFoundXcd = FALSE;

   /* search LINs with XCDs */
   while (spstG22Tmp != NULL && loiRc == 0)
   {
     if (spstG22Tmp->g_99 != NULL)
     {
       spstG99 = spstG22Tmp->g_99;
       spstTmp = spstG22Tmp->g_99->xcd;
       fFoundXcd = TRUE;
       break;
     }
     spstG22Tmp = spstG22Tmp->g_22_next;
   }

   if (fFoundXcd == TRUE)
   {
     /* distinguish between TIMM versions */
     if (timm->unh->version >= 302) {
       loiRc = foiPrintXcd3xx (spstG22Tmp, timm, pLay);
     } else {
       loiRc = foiPrintXcd (spstG22Tmp, timm, pLay);
     }

     if (loiRc == PREP_NO_XCDS)
     {
       /* this is no error */
       loiRc = 0;
     }
   }
   fovdPopFunctionName();

   return (loiRc);
}
/*---------------------------------------------------------------------------*/


/******************************************************************************
 * fsAnonymizeDigitsX
 *
 * DESCRIPTION:
 * Function : This function anonymizes a number. The number is shown so 
 *            that the beginning of the number is shown as digits and 
 *            the rest is shown as dots ('.').
 *
 * Input    : Called Number is the number to anonymize
 *            NumberType is the type of the number
 *
 *                        Values of NumberType are
 *                  'U'   Undefined number
 *                  '0'   Unknown number
 *                  '1'   International number
 *                  '2'   National number
 *                  '3'   Network specific number
 *                  '5','6','7' Reserved for future use
 *
 * Output   : CalledNumber contains the anonymized number
 *
 * RETURNS:
 *  - void -
 ******************************************************************************
 */

void fsAnonymizeDigitsX ( char *CalledNumber, char NumberType )
{

  fovdPushFunctionName("fsAnonymizeDigitsX");

#if ANONYMIZENUMBER
  /* Check if it is an international number */
  if ( NumberType == '1' )
  {
    /* Show only the first 7 characters */
    Anonymize( CalledNumber, 7 );
  }
  else
  {
    /* Check the first number */
    switch ( CalledNumber[0] )
    {
    case '0': /* Check if the second number is a zero */
      if ( CalledNumber[1] == '0' )
      {
	/* Show only the first 9 characters */
	Anonymize( CalledNumber, 9 );
      }      
      else
      {
	/* Show only the first 6 characters */
	Anonymize( CalledNumber, 6 );
      }      
      break;

    case '1': /* Show only the first 5 characters */
      Anonymize( CalledNumber, 5 );
      break;

    default : /* Show only the first 3 characters */
      Anonymize( CalledNumber, 3 );
      break;
    }
  }
#endif
  fovdPopFunctionName();
}
/*---------------------------------------------------------------------------*/


/******************************************************************************
 * Anonymize
 *
 * DESCRIPTION:
 * Function : This function anonymizes the number in psNumber
 * Examples : 1.  Call Anonymize( '1234567890', 5 )
 *                                Result is '12345.....'
 *
 *            2.  Call Anonymize( '12345', 2 )
 *                                Result is '12...'
 *
 *
 * PARAMETERS:
 *
 * Input    : psNumber is the number to anonymize.
 *
 *            pnDigits contains the number of digits NOT to change to a dot.
 *            By using this number as an index in the string psNumber, you
 *            get the first character to change to a dot.
 * Output   : psNumber contains the anonymized number after leaving this function
 *
 * RETURNS:
 *  - void -
 ******************************************************************************
 */
void Anonymize( char *psNumber, int pnDigits )
{
  UINT nCnt;       /* Counter for index in the string */
  UINT uLen;	  /* length of string */

  /* Set the Counter to the first character to change to a dot */
  nCnt = pnDigits;
  uLen = strlen (psNumber);

  /* As long as there are characters in the numbers to anonymize */
  while (nCnt < uLen)
  {
    /* Change the number to a dot and go to the next number */
    psNumber[nCnt++] = '.';
  }
}

/*---------------------------------------------------------------------------*/

/******************************************************************************
 * format_date_fields
 *
 * DESCRIPTION:
 * convert all date fields in TIMM-structure to output format
 * THIS FUNCTION ASSUMES THAT THERE IS ENOUGH ROOM ALLOCATED (IN TIMM STRUCTURE)
 * FOR THE CONVERTED DATE FIELDS. THIS IS NORMALLY TRUE IF THE PARSER ALLOCATES
 * THE MAXIMUM SPACE OF 35 CHARACTERS.
 *
 * PARAMETERS:
 *  stTIMM	*interchange		- TIMM-structure
 *
 * RETURNS:
 *  short
 ******************************************************************************
 */
static short format_date_fields( stTMSG *timm)
{
   short  losReturn = 0;
   struct s_dtm_seg *dtm_tmp;  /* tmp pointer to DTM segment        */
   struct s_group_8 *g_8;      /* tmp pointer to group 8            */
    
   /************************************************************/
   /* get invoice date in order to change the date format      */
   /************************************************************/

   fovdPushFunctionName("format_date_fields");

   dtm_tmp = timm->dtm;
   while (dtm_tmp && losReturn == 0)
   {
      if( 0 == strcmp (dtm_tmp->v_2005, DTM_INV) ||
          0 == strcmp (dtm_tmp->v_2005, DTM_ST)  ||
          0 == strcmp (dtm_tmp->v_2005, DTM_EN)  ||
          0 == strcmp (dtm_tmp->v_2005, DTM_TIL) )

       /* DTM_INV - invoicing date                             */
       /* DTM_ST  - start of invoicing period                  */
       /* DTM_EN  - end of invoicing period                    */
       /* DTM_TIL - received payments taken into account until */

       /* Due date is not contents of timm->dtm                     */
       /*        0 == strcmp( dtm_tmp->v_2005, DTM_DUE) ||        */
      {
         if (dtm_tmp->v_2380[0] == '\0')
         {
            losReturn = -1;
            continue; 
         } else {
	   strcpy (dtm_tmp->v_2380, form_date (dtm_tmp->v_2380));
	 }
      }
      dtm_tmp = dtm_tmp->dtm_next;
   }

   /* Get due date (DTM of group 8)                             */
   dtm_tmp = NULL;

   g_8     = timm->g_8;
   while (g_8 != NULL)
   {
     ASSERT (g_8->dtm != NULL);

     if (strcmp (g_8->dtm->v_2005, DTM_DUE) == 0)
       break;
     g_8 = g_8->g_8_next;
   }

   if (g_8 != NULL && g_8->dtm)
   {
      dtm_tmp = g_8->dtm;
   }

   if (dtm_tmp != NULL)
   {
      if (dtm_tmp->v_2380[0] == '\0')
      {
         losReturn = -1;
      } else {
	strcpy (dtm_tmp->v_2380, form_date (dtm_tmp->v_2380));
      }
   }

   fovdPopFunctionName();
   return( losReturn);
}
/*---------------------------------------------------------------------------*/


/******************************************************************************
 * form_date
 *
 * DESCRIPTION:
 * converts the date from YYMMDD format to DD.MM.YYYY
 *
 * PARAMETERS:
 *  char	*elem		- date in YYMMDD format
 *
 * RETURNS:
 *  char * 	- pointer to converted date
 ******************************************************************************
 */
static char *form_date(char *elem)    /* pointer to date element  */
{
  static char tmp[16];
  int	iCent;

  /* don't do it twice */
  if ((elem[2] >= '0') &&
      (elem[2] <= '9')) {

    /*
     * Build the century, assume that no date in BSCS
     * is before 1990, so if the year is less than 90
     * we say it is 21. century
     */
    if (*elem < '9')	iCent = 20;
    else		iCent = 19;

#ifdef EUROPEAN_DATE
    sprintf (tmp, "%c%c%c%c%c%c%d%c%c",
	     *(elem+4), *(elem+5),
	     DATE_SEPERATOR,
	     *(elem+2), *(elem+3),
	     DATE_SEPERATOR,
	     iCent,
	     *(elem+0), *(elem+1));
#else
    sprintf (tmp, "%c%c%c%c%c%c%d%c%c",
	     *(elem+2), *(elem+3),
	     DATE_SEPERATOR,
	     *(elem+4), *(elem+5),
	     DATE_SEPERATOR,
	     iCent,
	     *(elem+0), *(elem+1));
#endif
  }

  return (tmp);
}
/*---------------------------------------------------------------------------*/


/******************************************************************************
 * lovdPrintInvoice
 *
 * DESCRIPTION:
 * print detail table
 *
 * PARAMETERS:
 *  stTMSG	*timm			- TIMM-structure
 *  stLAYINF	*pLay			- layout information
 *
 * RETURNS:
 *  - void -
 ******************************************************************************
 */
static void lovdPrintInvoice (stTMSG *timm, stLAYINF *pLay)
{
  fovdPushFunctionName ("lovdPrintInvoice");

  /* print sender address */
  fovdPrintSender (timm, pLay);

  /* print header block */
  print_header_block (timm, pLay);

  /* print advertisments */
  fovdPrintAdv (timm, pLay);

  /* print totals */
  print_foot_block (timm, pLay);

  fovdPopFunctionName();
}


/*---------------------------------------------------------------------------*/



/******************************************************************************
 * fovdPrintCInfo
 *
 * DESCRIPTION:
 * print information about customer, like account number, billing period a.s.o.
 * 
 *
 * PARAMETERS:
 *  stTMSG	*timm			- TIMM-structure
 *  TYPEID	type			- type of message
 *  stLAYINF	*pLay			- layout information
 *
 * RETURNS:
 *  - void -
 ******************************************************************************
 */
static void fovdPrintCInfo (stTMSG *timm, TYPEID type, stLAYINF *pLay)
{
  struct s_group_1 *g_1;
  struct s_group_2 *g_2;
  struct s_group_3 *g_3;
  struct s_dtm_seg *dtm, *dtm_start, *dtm_end;
  struct s_rff_seg *rffIT, *rffIV;

  fovdPushFunctionName ("fovdPrintCInfo");


  if (fofIsPrinted (pLay, LAY_ACCOUNT_NO))
  {
    /* already (or not at all) printed in this document */
    fovdPopFunctionName();
    return;
  }


  rffIT = NULL;
  rffIV = NULL;

  /* get customer code and invoice number */
  if (timm->unh->version >= 302) {
    /* for TIMM version > 302 */

    g_2 = timm->g_2;
    while (g_2 != NULL)
    {
      ASSERT (g_2->nad != NULL);

      if (strcmp (g_2->nad->v_3035, "IV") == 0) {
	/* found the right group, now search the RFFs */
	g_3 = g_2->g_3;

	while (g_3 != NULL)
	{
	  ASSERT (g_3->rff != NULL);

	  if (strcmp (g_3->rff->v_1153, RFF_IT) == 0) {
	    rffIT = g_3->rff;
	  } else if (strcmp (g_3->rff->v_1153, RFF_IV) == 0) {
	    rffIV = g_3->rff;
	  }
	  if ((rffIT != NULL) && (rffIV != NULL))	break;
	  g_3 = g_3->g_3_next;
	}
	break;
      }
      g_2 = g_2->g_2_next;
    }

    if ((rffIT == NULL) && (rffIV == NULL)) {
      /*
      ** No information under the invoicee, it might
      ** now be that, due to an BCH defect, the information
      ** is packed to the temporary address,
      ** -> search it there !!
      */
      g_2 = timm->g_2;
      while (g_2 != NULL) {
	ASSERT (g_2->nad != NULL);

	if (strcmp (g_2->nad->v_3035, "IT") == 0) {
	  /* found the right group, now search the RFFs */
	  g_3 = g_2->g_3;

	  while (g_3 != NULL) {
	    ASSERT (g_3->rff != NULL);

	    if (strcmp (g_3->rff->v_1153, RFF_IT) == 0) {
	      rffIT = g_3->rff;
	    } else if (strcmp (g_3->rff->v_1153, RFF_IV) == 0) {
	      rffIV = g_3->rff;
	    }
	    if ((rffIT != NULL) && (rffIV != NULL))	break;
	    g_3 = g_3->g_3_next;
	  }
	  break;
	}
	g_2 = g_2->g_2_next;
      }
    }

  } else {
    /* for TIMM version 210 */

    g_1 = timm->g_1;
    while (g_1 != NULL)
    {
      ASSERT (g_1->rff != NULL);

      if (strcmp (g_1->rff->v_1153, RFF_IT) == 0) {
	rffIT = g_1->rff;
      } else if (strcmp (g_1->rff->v_1153, RFF_IV) == 0) {
	rffIV = g_1->rff;
      }
      if ((rffIT != NULL) && (rffIV != NULL))	break;
      g_1 = g_1->g_1_next;
    }
  }

  /* --- print customer code --- */

  if (rffIT != NULL) {
    vdAddElement (pLay, LAY_ACCOUNT_NO, 2,
		  FIXTEXT (FIX_RFF_IT),
		  COND_FETCH (rffIT, v_1154));
  } else {
    macErrorMessage (PREP_NO_CUSTOMER_NUMBER, CRITICAL,
		     "No customer code available.");
  }

  /* --- print invoice number --- */

  if (rffIV != NULL) {
    vdAddElement (pLay, LAY_INVOICE_NO, 2,
		  FIXTEXT (FIX_RFF_IV),
		  COND_FETCH (rffIV, v_1154));
  } else {
    if (type == INV_TYPE) {
      /* not mandatory in all documents */
      macErrorMessage (PREP_NO_INVOICE_NUMBER, CRITICAL,
		       "No invoice number available.");
    }
  }

  /* --- print invoicing date --- */

  dtm = timm->dtm;
  while (dtm != NULL) {
    if (strcmp (dtm->v_2005, DTM_INV) == 0)
      break;
    dtm = dtm->dtm_next;
  }

  if (dtm != NULL) {
    vdAddElement (pLay, LAY_INVOICE_DATE, 2,
		  FIXTEXT (FIX_DTM_3),
		  dtm->v_2380);
  } else {
    macErrorMessage (PREP_NO_INVOICE_DATE, CRITICAL,
		     "No invoice date available.");
  }

  /* --- print billing period --- */

  dtm_start = timm->dtm;
  while (dtm_start != NULL)
  {
    if (strcmp (dtm_start->v_2005, DTM_ST) == 0)
      break;
    dtm_start = dtm_start->dtm_next;
  }
  dtm_end = timm->dtm;
  while (dtm_end != NULL)
  {
    if (strcmp (dtm_end->v_2005, DTM_EN) == 0)
      break;
    dtm_end = dtm_end->dtm_next;
  }

  if ((dtm_start != NULL) && (dtm_end != NULL)) {
    vdAddElement (pLay, LAY_BILLING_PERIOD, 4,
		  FIXTEXT (FIX_DTM_167),
		  dtm_start->v_2380,
		  " - ", /*FIXTEXT (FIX_DATE_TO),*/
		  dtm_end->v_2380);
  } else {
    macErrorMessage (PREP_NO_BILLING_PERIOD, CRITICAL,
		     "No billing period available.");
  }

  fovdPopFunctionName();
}


/******************************************************************************
 * print_header_block
 *
 * DESCRIPTION:
 * print invoice header
 * 
 *
 * PARAMETERS:
 *  stTMSG	*timm_message		- TIMM-structure
 *  stLAYINF	*pLay			- layout information
 *
 * RETURNS:
 *  - void -
 ******************************************************************************
 */
static void print_header_block (stTMSG *timm, stLAYINF *pLay)
{
   fovdPushFunctionName("print_header_block");

   /* initialisation                                             */


  if (fofIsPrinted (pLay, LAY_INV_FIX_AUTO_PAY))
  {
    /* already (or not at all) printed in this document */
    fovdPopFunctionName();
    return;
  }


   /* --- print 'invoice' and 'Telecom' --- */

   vdAddElement(pLay, LAY_FIX_HEADER, 1, FIXTEXT (FIX_INVOICE));
   vdAddElement(pLay, LAY_FIX_TELECOM, 1, FIXTEXT (FIX_TELECOM));


   /* --- print name and address of customer --- */
   fovdPrintRec (timm, pLay);

   fovdPrintCInfo (timm, INV_TYPE, pLay);

   /* print 'The amount will be debited to your account.' */
   vdAddElement (pLay, LAY_INV_FIX_AUTO_PAY, 1, FIXTEXT (FIX_DEBIT));

   /* print 'Taken into account are all charges within the billing cycle.' */
   vdAddElement (pLay, LAY_INV_FIX_BALANCING, 1, FIXTEXT (FIX_TAK_I_ACC));

   /* print header of lin blocks */
   vdAddElement (pLay, LAY_INV_FIX_HEADER_1, 12,
                 FIXTEXT (FIX_LIN_SVE),
                 FIXTEXT (FIX_PRI_INV),
                 FIXTEXT (FIX_PRI_INVa),
                 FIXTEXT (FIX_QTY_107),
		 FIXTEXT (FIX_MOA_125),
                 FIXTEXT (FIX_MOA_125a),
                 FIXTEXT (FIX_TAX_1),
                 FIXTEXT (FIX_TAX_1a),
                 FIXTEXT (FIX_MOA_124),
                 FIXTEXT (FIX_MOA_124a),
                 FIXTEXT (FIX_MOA_203),
                 FIXTEXT (FIX_MOA_203a));

   fovdPopFunctionName();
}
/*---------------------------------------------------------------------------*/

/******************************************************************************
 * fovdPrintSender
 *
 * DESCRIPTION:
 * print invoice sender address information
 * 
 *
 * PARAMETERS:
 *  stTMSG	*timm_message		- TIMM-structure
 *  stLAYINF	*pLay			- layout information
 *
 * RETURNS:
 *  - void -
 ******************************************************************************
 */
static void fovdPrintSender (stTMSG *timm_message, stLAYINF *pLay)
{
   struct s_group_2 *g_2;
   struct s_group_3 *g_3;
   struct s_com_seg *com;
   struct s_com_seg *com_fx;
   struct s_com_seg *com_te;
   struct s_com_seg *com_tx;
   struct s_nad_seg *nad;
   struct s_fii_seg *fii;
   struct s_rff_seg *rff;

   char 	    *szstr1;

   char		    *szStrAll;

   fovdPushFunctionName("fovdPrintSender");

   /* initialisation                                             */

   g_2     = NULL;
   g_3     = NULL;
   com     = NULL;
   com_fx  = NULL;
   com_te  = NULL;
   com_tx  = NULL;
   nad     = NULL;
   fii     = NULL;
   rff     = NULL;


  if (fofIsPrinted (pLay, LAY_FIX_STARTFIRST))
  {
    /* already (or not at all) printed in this document */
    fovdPopFunctionName();
    return;
  }

   /* getting pointers to data segments                         */

   g_2 = timm_message->g_2;
   while (g_2 != NULL)
   {
    ASSERT (g_2->nad != NULL);

     if (strcmp (g_2->nad->v_3035, NAD_SENDER) == 0)
       break;
     g_2 = g_2->g_2_next;
   }

   if (g_2 != NULL)
   {
      nad = g_2->nad;
      fii = g_2->fii;
      g_3 = g_2->g_3;
      if (g_2->g_5 != NULL)
      {
         com = g_2->g_5->com;
      }
      else
      {
        macErrorMessage (PREP_NO_SENDER_PHONE_FAX_TELEX, WARNING,
			 "No sender address phone, fax and telex number available.");
      }
   }
   else
   {
      macErrorMessage (PREP_NO_SENDER_ADDRESS, WARNING,
                       "No sender address available at all.");
   }

   /* check for phone numbers .. (not printed yet) */
   while (com != NULL)
   {
      if (!strcmp(com->v_3155, COM_FX))
      {
         com_fx = com;
      }
      else if (!strcmp(com->v_3155, COM_TE))
      {
         com_te = com;
      }
      else if (!strcmp(com->v_3155, COM_TX))
      {
         com_tx = com;
      }
      com = com->com_next;
   }
      
      
   /* printing data                                             */
   szstr1 = fpvdGetPrepMemory (128);
   szstr1 [0] = 0;       /* initialize string */
   if (nad != NULL)
   {
     /* fifth line                                              */
     strcpy (szstr1, COND_FETCH (nad, v_3251));	/* zip code */
     strcat (szstr1, " ");
     strcat (szstr1, COND_FETCH (nad, v_3164));	/* company city */

     vdAddElement (pLay, LAY_SENDER_LINE_1, 1, COND_FETCH (nad, v_3036));
     vdAddElement (pLay, LAY_SENDER_LINE_2, 1, COND_FETCH (nad, v_3036a));
     vdAddElement (pLay, LAY_SENDER_LINE_3, 1, COND_FETCH (nad, v_3036b));

     /* forth line                                              */
     if (fii != NULL)
     {
       vdAddElement (pLay, LAY_SENDER_LINE_4, 3,
		     COND_FETCH (nad, v_3036c),
		     FIXTEXT (FIX_PC),
		     COND_FETCH (fii, v_3194));
     }
     else
     {
       macErrorMessage (PREP_FII_NO_SENDER_POST_NO, WARNING,
			"No sender address post number available.");
       vdAddElement (pLay, LAY_SENDER_LINE_4, 1, nad->v_3036c);
     }

#if 1
     /*
      * Get memory for one large string containing the full address
      * and fill the string with the address
      */
     szStrAll = fpvdGetPrepMemory (sizeof (nad->v_3036) + 
				   sizeof (nad->v_3036a) +
				   sizeof (nad->v_3036b) +
				   sizeof (nad->v_3036c) +
				   sizeof (nad->v_3251) +
				   sizeof (nad->v_3164) +
				   64);

     strcpy (szStrAll, COND_FETCH (nad, v_3036));
     strcat (szStrAll, "   ");
     strcat (szStrAll, COND_FETCH (nad, v_3036a));
     strcat (szStrAll, "   ");
     strcat (szStrAll, COND_FETCH (nad, v_3036b));
     strcat (szStrAll, "   ");
     strcat (szStrAll, COND_FETCH (nad, v_3251));
     strcat (szStrAll, " ");
     strcat (szStrAll, COND_FETCH (nad, v_3164));

     /* add it for fulline (follow up pages) */
     vdAddElement (pLay, LAY_SENDER_FULLINE, 5, szStrAll, "", "", "", "");
#else
     /* add it for fulline (follow up pages) */
     vdAddElement (pLay, LAY_SENDER_FULLINE, 5,
		   COND_FETCH (nad, v_3036),
		   COND_FETCH (nad, v_3036a),
		   COND_FETCH (nad, v_3036b),
		   COND_FETCH (nad, v_3036c),
		   szstr1);
#endif
   }

   while (g_3 != NULL)
   {
     ASSERT (g_3->rff != NULL);

     if (strcmp (g_3->rff->v_1153, RFF_VA) == 0)
       break;
     g_3 = g_3->g_3_next;
   }

   if ((g_3 != NULL) && (g_3->rff != NULL))
   {
     vdAddElement(pLay, LAY_SENDER_LINE_5, 3,
                  szstr1,
                  FIXTEXT (FIX_RFF_VA),
                  COND_FETCH (g_3->rff, v_1154));
   }
   else
   {
     macErrorMessage (PREP_G3_NO_SENDER_VAT_NO, WARNING,
		      "No sender address VAT number available.");
     vdAddElement (pLay, LAY_SENDER_LINE_5, 1, szstr1);
   }

   /*
    * write two dummy lines for fixed starting point on
    * first and follow-up pages
    */
   vdAddElement (pLay, LAY_FIX_STARTFIRST, 1, NULL);
   vdAddElement (pLay, LAY_FIX_STARTFOLLOW, 1, NULL);

   fovdPopFunctionName();
}
/*---------------------------------------------------------------------------*/


/******************************************************************************
 * print_foot_block
 *
 * DESCRIPTION:
 * print lin footer
 * 
 *
 * PARAMETERS:
 *  stTMSG	*timm			- TIMM-structure
 *  stLAYINF	*pLay			- layout information
 *
 * RETURNS:
 *  - void -
 ******************************************************************************
 */
static void print_foot_block(stTMSG *timm,stLAYINF *pLay)
{
   struct s_group_45 *g_45;
   struct s_group_47 *g_47;
   struct s_moa_seg  *moa_77;
   struct s_moa_seg  *moa_77_rounded;
   struct s_moa_seg  *moa_79;
   struct s_moa_seg  *moa_124;
   struct s_moa_seg  *moa_178;
   struct s_moa_seg  *moa_980;


   fovdPushFunctionName("print_foot_block");

   /* initialization */

   g_45           = NULL;
   g_47           = NULL;
   moa_77         = NULL;
   moa_77_rounded = NULL;
   moa_79         = NULL;
   moa_124        = NULL;
   moa_178        = NULL;
   moa_980        = NULL;

   /* get pointer moa_77 */

   g_45 = timm->g_45;
   while (g_45)
   {
     ASSERT (g_45->moa != NULL);

     if( 0 == strcmp(g_45->moa->v_5025, "77") &&
	 0 == strcmp(g_45->moa->v_4405, "5"))
     {
       break;         
     }
     else
     {
       g_45 = g_45->g_45_next;
     }
   }


   if (g_45)
   {
     moa_77 = g_45->moa;
   }
   else
   {
     macErrorMessage (PREP_MOA_NO_INV_SUM_AMOUNT, WARNING,
		      "No invoice sum amount available.");
   }


   /* get pointer moa_77_rounded */
   
   g_45 = timm->g_45;
   while (g_45)
   {
     ASSERT (g_45->moa != NULL);

     if( 0 == strcmp(g_45->moa->v_5025, "77") &&
	 0 == strcmp(g_45->moa->v_4405, "19"))
     {
       break;         
     }
     else
     {
       g_45 = g_45->g_45_next;
     }
   }


   if (g_45)
   {
     moa_77_rounded = g_45->moa;
   }
   else
   { 
     macErrorMessage (PREP_MOA_NO_INV_SUM_AMOUNT, WARNING,
		      "No invoice sum amount (rounded) available.");
   }

   /* get pointer moa_79 */

   g_45 = timm->g_45;
   while (g_45)
   {
     ASSERT (g_45->moa != NULL);

     if (strcmp(g_45->moa->v_5025,"79") == 0)	break;
     g_45 = g_45->g_45_next;
   }

   if (g_45)
   {
     moa_79 = g_45->moa;
   }
   else
   {
     macErrorMessage (PREP_MOA_NO_NETTO_INV_SUM, WARNING,
		      "No netto invoice sum amount available.");
   }

   /* get pointer moa_124 */

   g_47 = timm->g_47;
   while (g_47 && g_47->moa)
   {
     if (strcmp(g_47->moa->v_5025,"124") == 0)	break;
     g_47 = g_47->g_47_next;
   }

   if (g_47)
   {
     moa_124 = g_47->moa;
   }
   else
   {
     macErrorMessage (PREP_MOA_NO_INV_TAX_SUM, WARNING,
		      "No invoice tax sum amount available.");
   }

   /* get pointer moa_178 */

   g_45 = timm->g_45;
   while (g_45)
   {
     ASSERT (g_45->moa != NULL);

     if (strcmp(g_45->moa->v_5025,"178") == 0)	break;
     g_45 = g_45->g_45_next;
   }

   if (g_45)
   {
     moa_178 = g_45->moa;
   }
   else
   {
     macErrorMessage (PREP_MOA_NO_SUM_TO_PAY, WARNING,
		      "No sum to pay available.");
   }

   /* get pointer moa_980 */

   g_45 = timm->g_45;
   while (g_45)
   {
     ASSERT (g_45->moa != NULL);

     if (strcmp(g_45->moa->v_5025,"980") == 0)	break;
     g_45 = g_45->g_45_next;
   }

   if (g_45)
   {
     moa_980 = g_45->moa;
   }
   else
   {
     macErrorMessage (PREP_G45_NO_ROUNDING_DIFF, WARNING,
		      "No rounding difference available.");
   }


   /* --- print subtotal --- */
   vdAddElement(pLay, LAY_INV_SUBTOTAL, 7,
                FIXTEXT (FIX_MOA_79),
                COND_FETCH(moa_79, v_5004),
		COND_FETCH(moa_79, v_6345),
                COND_FETCH(moa_124, v_5004),
		COND_FETCH(moa_124, v_6345),
                COND_FETCH(moa_77, v_5004),
		COND_FETCH(moa_77, v_6345));

   
   /* --- print adjustment --- */
   vdAddElement(pLay, LAY_INV_ADJUSTMENT, 3,
                FIXTEXT (FIX_MOA_80),
                COND_FETCH(moa_980, v_5004),
		COND_FETCH(moa_980, v_6345));

   /* --- print total invoice amount --- */
   vdAddElement(pLay, LAY_INV_TOTALS, 3,
                FIXTEXT (FIX_MOA_178),
                COND_FETCH(moa_77_rounded, v_5004),
		COND_FETCH(moa_77_rounded, v_6345));

   fovdPopFunctionName();
}
/*---------------------------------------------------------------------------*/


/******************************************************************************
 * fovdPrintRec
 *
 * DESCRIPTION:
 * print receiver address
 * 
 *
 * PARAMETERS:
 *  stTMSG	*timm_message		- TIMM-structure
 *  TYPEID 	type			- type of message
 *  stLAYINF	*pLay			- layout information
 *
 * RETURNS:
 *  - void -
 ******************************************************************************
 */
static void fovdPrintRec (stTMSG *timm_message,
			  stLAYINF *pLay)
{
  struct s_group_2 *g_2;
  struct s_nad_seg *nad;


  fovdPushFunctionName("fovdPrintRec");

  if (fofIsPrinted (pLay, LAY_INVOICEE_LINE_1))
  {
    /* already (or not at all) printed in this document */
    fovdPopFunctionName();
    return;
  }

  /* initialisation                                            */
  g_2     = NULL;
  nad     = NULL;

  /* getting pointers to data segments                         */

  g_2 = timm_message->g_2;
  while (g_2 != NULL)
  {
    ASSERT (g_2->nad != NULL);

    if (strcmp (g_2->nad->v_3035, NAD_RECEIVER) == 0)
      break;
    g_2 = g_2->g_2_next;
  }

  if (g_2 != NULL)
  {
    nad = g_2->nad;
  }
  else
  {
    macErrorMessage (PREP_NO_RECEIVER_ADDRESS, CRITICAL,
		     "No receiver address available.");
  }

  vdAddElement(pLay, LAY_INVOICEE_LINE_1, 1, COND_FETCH(nad,v_3036));
  vdAddElement(pLay, LAY_INVOICEE_LINE_2, 1, COND_FETCH(nad,v_3036a));
  vdAddElement(pLay, LAY_INVOICEE_LINE_3, 1, COND_FETCH(nad,v_3036b));
  vdAddElement(pLay, LAY_INVOICEE_LINE_4, 1, COND_FETCH(nad,v_3036c));
  vdAddElement(pLay, LAY_INVOICEE_LINE_5, 1, COND_FETCH(nad,v_3036d));
  vdAddElement(pLay, LAY_INVOICEE_LINE_6, 1, COND_FETCH(nad,v_3164));

  /* add it for fulline (follow up pages) */
  vdAddElement (pLay, LAY_INVOICEE_FULLINE, 6,
		COND_FETCH (nad, v_3036),
		COND_FETCH (nad, v_3036a),
		COND_FETCH (nad, v_3036b),
		COND_FETCH (nad, v_3036c),
		COND_FETCH (nad, v_3036d),
		COND_FETCH (nad, v_3164));

  /* add Pagenumber */
  vdAddElement (pLay, LAY_PAGE, 4,
		FIXTEXT (FIX_PAGE_1), gszPageString,
		FIXTEXT (FIX_PAGE_2), ".");

  fovdPopFunctionName();
}

/*---------------------------------------------------------------------------*/


/******************************************************************************
 * fovdPrintSumTotal
 *
 * DESCRIPTION:
 * print totals
 * 
 *
 * PARAMETERS:
 *  stTMSG	*timm_message		- TIMM-structure
 *  stLAYINF	*pLay			- layout information
 *
 * RETURNS:
 *  int
 ******************************************************************************
 */
static short foiPrintSumTotal (stTMSG *timm_message,
			       stLAYINF *pLay)
{
   short              losReturn;
   struct s_moa_seg   *moa;
   struct s_group_45  *g_45;


   fovdPushFunctionName ("foiPrintSumTotal");

   /* initializations */
   losReturn = 0;
   moa       = NULL;


   /* get pointers to MOA segment in group 45 */

   g_45 = timm_message->g_45;
   if (g_45)
   {
      if (g_45->moa)
      {
         while (g_45)
         {
	   if (strcmp (g_45->moa->v_5025, "77") == 0)
	     break;
	   g_45 = g_45->g_45_next;
         }

	 if (g_45 != NULL)
	 {
	   moa = g_45->moa;

	   /* build header for TOTAL block             */

	   vdAddElement (pLay, LAY_SUM_EMPTY_LINE_1, 1,  " ");

	   vdAddElement (pLay, LAY_SUM_TOTAL, 3,
			 FIXTEXT (FIX_SUM_TOTAL),
			 COND_FETCH (moa, v_5004),
			 COND_FETCH (moa, v_6345));
	 }
	 else
	 {
	   macErrorMessage (PREP_G45_NO_MOA, WARNING,
			    "No total sum available (moa 77 in group 45).");
	   losReturn = (int) PREP_G45_NO_MOA;
	 }
      }
      else
      {
        macErrorMessage (PREP_G45_NO_MOA, WARNING,
                         "No moa segment in group 45 available at all.");
	losReturn = (int) PREP_G45_NO_MOA;
      }

   }
   else
   {
      losReturn = (int) PREP_NO_G45;
      macErrorMessage (PREP_NO_G45, WARNING,
                       "No group 45 in timm message.");
   }

   fovdPopFunctionName ();
   return (losReturn);
}
/*---------------------------------------------------------------------------*/


/******************************************************************************
 * fostPrintOther
 *
 * DESCRIPTION:
 * print other charges
 *
 * PARAMETERS:
 *  struct s_group_22	*pstG22cur	- Group 22
 *  short		*posReturn	- return value
 *  stLAYINF		*pLay		- layout information
 *
 * RETURNS:
 *  - void -
 ******************************************************************************
 */
static struct s_group_22 *fostPrintOther (struct s_group_22 *pstG22cur,
					  short *posReturn, stLAYINF *pLay)
{
   struct s_imd_seg *imd;
   struct s_moa_seg *moa;

   short            lofLoop;

   fovdPushFunctionName("fostPrintOther");

   /* initializations */
   *posReturn = 0;
   imd        = NULL;
   moa        = NULL;

   /* build OTHER block */
   if (pstG22cur)
   {
      /* build header for OTHER block             */

      vdAddElement (pLay, LAY_SUM_FIX_OTHER_CHARGES, 1,
                    FIXTEXT (FIX_IMD_OTHER));
      vdAddElement(pLay, LAY_SUM_FIX_AMOUNT, 2,
                   FIXTEXT (FIX_SUM_TITLE1),
                   FIXTEXT (FIX_SUM_TITLE2));

      /* Now the right LIN segment for the other block    */
      /* is found. Process the following LIN blocks       */
      /* with nesting level "02" and build printer output */

      lofLoop = TRUE;
      while (pstG22cur && (*posReturn == 0) && 
	     (strcmp (pstG22cur->lin->v_1222, LIN_NESTING_02) == 0))
      {
         /* get pointers for imd and moa segments   */

         imd = pstG22cur->imd;

         if (pstG22cur->g_23)
         {
            moa = pstG22cur->g_23->moa;
         }
         else
         {
            macErrorMessage (PREP_G22_G23_NO_MOA, WARNING,
			     "No moa segment in group 22/23 available at all.");
            *posReturn = (int) PREP_G22_G23_NO_MOA;
	    pstG22cur = pstG22cur->g_22_next;
            continue;
         }

         /* build up printer output */

         vdAddElement(pLay, LAY_SUM_ITEMS_OTHER_CHARGES, 3,
                      COND_FETCH(imd,v_7008a),
                      COND_FETCH(moa,v_5004),
                      COND_FETCH(moa,v_6345));

         /* abort loop, if there is an LIN segment  */
         /* with a nesting level != LIN_NESTING_02  */

	 pstG22cur = pstG22cur->g_22_next;
      }  /* end of while loop */

   }
   else
   {
      macErrorMessage (PREP_UNDEF_DOCTYPE, WARNING,
                       "invalid group 22 sorted was passed as a parameter");
      *posReturn = (int) PREP_UNDEF_DOCTYPE;
   }

   if (*posReturn == 0)
   {
      fovdPopFunctionName();
      return (pstG22cur);
   }
   else
   {
      fovdPopFunctionName();
      return ((struct s_group_22 *) NULL);
   }
}
/*--------------------------------------------------------------------*/


/******************************************************************************
 * fovdRepSumList
 *
 * DESCRIPTION:
 * repeat (break) a list of sumsheet information
 *
 * PARAMETERS:
 *  stLAYINF	*pLay			- layout information
 *
 * RETURNS:
 *  - void -
 ******************************************************************************
 */
static void fovdRepSumList (stLAYINF *pLay)
{
  fHeader1 = FALSE;
  fHeader2 = FALSE;
  fHeader3 = FALSE;
  fHeader4 = FALSE;
  fHeader5 = FALSE;
  fHeader6 = FALSE;

  vdBreakList (pLay, 34,
	       LAY_ITB_NO_CALL_REC_12,	LAY_ITB_NO_CALL_REC_3,
	       LAY_ITB_FIX_HEADER_1,   	LAY_ITB_ITEMS_TABLE_1,
	       LAY_ITB_FIX_HEADER_2,	LAY_ITB_ITEMS_TABLE_2,
	       LAY_ITB_FIX_HEADER_3,	LAY_ITB_ITEMS_TABLE_3,
	       LAY_ITB_FIX_HEADER_4,	LAY_ITB_ITEMS_TABLE_4,
	       LAY_ITB_FIX_HEADER_5,	LAY_ITB_ITEMS_TABLE_5,
	       LAY_ITB_FIX_HEADER_6,	LAY_ITB_ITEMS_TABLE_6,
	       LAY_SUM_LIN04_HU,       	LAY_SUM_LIN04_U,
	       LAY_SUM_LIN04_HA,	LAY_SUM_LIN04_A,
	       LAY_SUM_LIN04_HS,	LAY_SUM_LIN04_S,
	       LAY_LEG_TITLE,		LAY_LEG_HINT,
	       LAY_LEG_INFO,	        LAY_SUM_SUBADDRESS,
	       LAY_SUM_SIM_NUMBER,
	       LAY_SUM_HEADER,
	       LAY_SUM_SUBSCRIPTION,
	       LAY_SUM_ACCESS,
	       LAY_SUM_USAGE,
	       LAY_SUM_SUBTOTAL,
	       LAY_SUM_FIX_OTHER_CHARGES,
	       LAY_SUM_FIX_AMOUNT,
	       LAY_SUM_ITEMS_OTHER_CHARGES,
	       LAY_SUM_TOTAL);
}


/******************************************************************************
 * foiPrintSumBlock
 *
 * DESCRIPTION:
 * This function prints all recurring blocks on the
 * sumsheet. That means all subaddress blocks, SIM blocks
 * and "other" blocks. The blocks which can occur only
 * once on the sumsheet are not printed. 
 *
 * PARAMETERS:
 *  stTMSG	*timm_message		- TIMM-structure
 *  short	posIsFlatSubscr		- info about flat subscriber
 *  stLAYINF	*pLay			- layout information
 *
 * RETURNS:
 *  - void -
 ******************************************************************************
 */
static short foiPrintSumBlock (stTMSG *timm_message,
			       short posIsFlatSubscr, stLAYINF *pLay)
{
  short             	losReturn;
  struct s_group_22 	*stG22cur;
  struct s_lin_seg  	*lin;
  BOOL              	fFirstPrint = TRUE;
  BOOL	    		fBreakSubs = FALSE;
#ifdef DEBUG
  struct s_group_22 	*stG22old;
#endif

  fovdPushFunctionName("foiPrintSumBlock");

  /* intitializations */
  losReturn   = 0;
  lin         = NULL;
  stG22cur    = NULL; 

  fHeader1 = FALSE;
  fHeader2 = FALSE;
  fHeader3 = FALSE;
  fHeader4 = FALSE;
  fHeader5 = FALSE;
  fHeader6 = FALSE;

  if (timm_message->g_22)
  {
    stG22cur = timm_message->g_22;

    if (stG22cur)
    {
      /* go through the list of g_22 elements, look at */
      /* each LIN segment's nesting indicator and call */
      /* the suitable function for this nesting level  */

      while (stG22cur && losReturn == 0)
      {
#ifdef DEBUG
	stG22old = stG22cur;
#endif
	lin = stG22cur->lin;
 
	if (0 == strcmp (lin->v_1222, LIN_NESTING_01))
	{
	  /* This lin segment contains the receiver's*/
	  /* address (subaddress for large accounts) */

	  if (posIsFlatSubscr == FALSE)
	  { 
	    /*
	     * this is a new subscriber, break the elements
	     * for one subscriber to build a new group
	     */
	    if (!fFirstPrint)
	    {
	      /* signalize repetition of last group */
	      fovdRepSumList (pLay);
	      fBreakSubs = TRUE;
	    }

	    stG22cur = fostPrintSubad (stG22cur, pLay);
	    if (stG22cur == NULL)
	    {
	      losReturn = -1;
	    }
	  } else {
	    stG22cur = stG22cur->g_22_next;
	  }
	  ASSERT (stG22old != stG22cur);
	}
	else if (0 == strcmp(lin->v_1222, LIN_NESTING_02))
	{
	  /*
	   * if there is something other to print, do it
	   */
	  if (stG22cur->imd)
	  {
	    if ((0==strcmp(stG22cur->imd->v_7009, IMD_SERVICE_PACK)) ||
		(0==strcmp(stG22cur->imd->v_7009, IMD_SERVICE))      ||
		(0==strcmp(stG22cur->imd->v_7009, IMD_SERVICE_FEE)))
	    {
	      /* print other charges */
	      stG22cur = fostPrintOther (stG22cur, &losReturn, pLay);
	    }
	    else
	    {
	      /*
	       * this is a new contract, break the elements
	       * for one contract to build a new group
	       */
	      if (!fFirstPrint && !fBreakSubs)
	      {
		/* signalize repetition of last group */
		fovdRepSumList (pLay);
	      }
	      fBreakSubs = FALSE;

	      /* this lin segment contains information   */
	      /* about a subscriber contract             */

	      stG22cur = fostPrintSim (timm_message, stG22cur, pLay);
	    }
	    fFirstPrint = FALSE;

	    if (stG22cur == NULL)
	    {
	      losReturn = -1;
	    }
	  }
	  ASSERT (stG22old != stG22cur);
	}
	else if (0 == strcmp(lin->v_1222, LIN_NESTING_03))
	{
	  /* this lin segment contains information  */
	  /* about summarized charge types, but the */
	  /* processing of this information is      */
	  /* already done in fostPrintSim, so do    */
	  /* nothing here.                          */
	  stG22cur = stG22cur->g_22_next;

	  ASSERT (stG22old != stG22cur);
	}
	else if (0 == strcmp(lin->v_1222, LIN_NESTING_04))
	{
	  /* this lin segment contains information  */
	  /* about the different services for the   */
	  /* subscriber but is not needed in        */
	  /* sumsheets.                             */  
	  stG22cur = stG22cur->g_22_next;

	  ASSERT (stG22old != stG22cur);
	}
	else if (0 == strcmp(lin->v_1222, LIN_NESTING_05))
	{
	  /*
	  ** Activation / Deactivation, not printed
	  */
	  stG22cur = stG22cur->g_22_next;

	  ASSERT (stG22old != stG22cur);
	}
	else
	{
	  sprintf (laszErrMsg, "SUMSHEET: Undefined nesting level %s",
		   lin->v_1222 );                          
	  macErrorMessage (PREP_UNDEF_NESTING_LEVEL,
			   WARNING, laszErrMsg);
	  losReturn = (int) PREP_UNDEF_NESTING_LEVEL;
	}

	ASSERT (stG22old != stG22cur);
      }
    }
    else
    {
      macErrorMessage (PREP_NO_G22, WARNING,
		       "SUMSHEET: No group 22 (sorted 1) available in TIMM msg");
      losReturn = (int) PREP_NO_G22;
    }

  }
  else
  {
    macErrorMessage (PREP_NO_G22, WARNING,
		     "SUMSHEET: No group 22 available in TIMM msg");
    losReturn = (int) PREP_NO_G22;
  }

  fovdPopFunctionName();
  return (losReturn);
}
/*---------------------------------------------------------------------------*/


/******************************************************************************
 * fostPrintSubad
 *
 * DESCRIPTION:
 * Print a Customer subaddress
 *
 * PARAMETERS:
 *  struct s_group_22	*pstG22cur	- Group 22 sorted
 *  stLAYINF		*pLay		- layout information
 *
 * RETURNS:
 *  struct s_group_22 *
 ******************************************************************************
 */
static struct s_group_22 *fostPrintSubad (struct s_group_22 *pstG22cur,
					  stLAYINF *pLay)
{
   struct s_rff_seg  *rff;
   struct s_nad_seg  *nad;
   struct s_lin_seg  *lin;
   struct s_group_26 *g_26;
   struct s_group_31 *g_31;

   short             losReturn, lofFound;


   fovdPushFunctionName("fostPrintSubad");


   /* initializations */
   losReturn = 0;
   rff       = NULL;
   nad       = NULL;
   lin       = NULL;
   g_26      = NULL;
   g_31      = NULL;

   if( pstG22cur )
   {
      /* check, if passed pointer is a pointer to a LIN */
      /* segment with nesting level 'LIN_NESTING_01'    */
      /* if not return error                            */

      lin = pstG22cur->lin;

      if (lin)
      {
         if (0 == strcmp (lin->v_1222, LIN_NESTING_01))
         {
            if (pstG22cur->g_26)
            {
               /* search for rff segment with custcode  */

               g_26 = pstG22cur->g_26;
               lofFound = FALSE;
               while (g_26 && lofFound == FALSE)
               {
		 ASSERT (g_26->rff != NULL);

                  if (0 == strcmp( g_26->rff->v_1153, RFF_IT))
                  {
                     lofFound = TRUE;
                  }
                  else
                  {  
                     g_26 = g_26->g_26_next;
                  }

               }

               rff = g_26-> rff;

            }
            else
            {
               macErrorMessage (PREP_NO_G22_G26_RFF, WARNING,
                                "Group 22 has no group 26 with RFF segment.");
               losReturn = (int) PREP_NO_G22_G26_RFF;
            }

            /* search for nad segment with customer address */

            if( pstG22cur->g_31 )
            {
               g_31 = pstG22cur->g_31;
               lofFound = FALSE;
               while( g_31 && lofFound == FALSE )
               {
 		  ASSERT (g_31->nad != NULL);

                  if (0 == strcmp (g_31->nad->v_3035, NAD_RECEIVER))
                  {
                     lofFound = TRUE;
                  }
                  else
                  {  
                     g_31 = g_31->g_31_next;
                  } 
               }

               nad = g_31->nad;

            }
            else
            {
               macErrorMessage (PREP_NO_G22_G31_NAD, WARNING,
                                "Group 22 has no group 31 with NAD segment.");
               losReturn = (int) PREP_NO_G22_G31_NAD;
            }

            if( losReturn == 0 )
            {
               /* sumadr block with subreceiver address */
               if (nad != NULL)
               {
                 vdAddElement(pLay, LAY_SUM_SUBADDRESS, 6,
                              COND_FETCH (nad, v_3036),
                              COND_FETCH (nad, v_3036a),
                              COND_FETCH (nad, v_3036b),
                              COND_FETCH (nad, v_3036c),
                              COND_FETCH (nad, v_3036d),
                              COND_FETCH (nad, v_3164));
               }
            }
         }
         else
         {
            macErrorMessage (PREP_LIN_ILL_NEST_IND, WARNING,
                             "LIN segment with incorrect nesting indicator.");
            losReturn = (int) PREP_LIN_ILL_NEST_IND;
         }

      }
      else
      {
         macErrorMessage (PREP_NO_G22_LIN, WARNING,
			  "Group 22 pointer does not contain a LIN segment at all.");
         losReturn = (int) PREP_NO_G22_LIN;
      }
   }
   else
   {
      macErrorMessage (PREP_INVALID_G22_SORTED, WARNING,
		       "Invalid group 22 pointer was passed as parameter.");
      losReturn = (int) PREP_INVALID_G22_SORTED;
   }

   if (losReturn == 0)
   {
      fovdPopFunctionName ();
      return (pstG22cur->g_22_next);
   }
   else
   {
      fovdPopFunctionName();
      return ((struct s_group_22 *) NULL);
   }
}
/*---------------------------------------------------------------------------*/


/******************************************************************************
 * fostPrintSimServices
 *
 * DESCRIPTION:
 * prints all LIN04 information for a sumsheet for the given LIN03 group
 *
 * PARAMETERS:
 *  stTMSG		*timm		- pointer to TIMM
 *  struct s_imd	*pImd		- pointer to IMD (charge type)
 *  struct s_group_22	*pstG22       	- Group 22
 *  stLAYINF		*pLay		- layout information
 *
 * RETURNS:
 *  struct s_group_22 *
 ******************************************************************************
 */
static struct s_group_22 *fostPrintSimServices (struct s_group_22 *pstG22,
						struct s_imd_seg *pImd,
						stLAYINF *pLay)
{
   struct s_imd_seg 	*imd;
   struct s_moa_seg 	*moa;
   struct s_pia_seg 	*pia;
   struct s_qty_seg 	*qty;
   struct s_lin_seg 	*lin;
   struct s_group_23 	*pstG23;
   char 		*szType;
   char 		*szPpU;
   char 		*szTtdes;
   char 		*szZndes;
   int			iBody;		/* index for body */

   fovdPushFunctionName("fostPrintSimServices");

   /* get pointers for TIMM information */

   if (pstG22 && pImd)
   {
     /*
      * add the header for the detailed list of LIN04 elements
      */
     if (pImd->v_7008a[1] == '\0') {
       switch (pImd->v_7008a[0]) {
       case 'U':
	 vdAddElement (pLay, LAY_SUM_LIN04_HU, 3,
		       FIXTEXT (FIX_SUM_SERV),
		       FIXTEXT (FIX_SUM_QTY),
		       FIXTEXT (FIX_AMOUNT));
	 iBody   = LAY_SUM_LIN04_U;
	 break;
       case 'A':
	 vdAddElement (pLay, LAY_SUM_LIN04_HA, 3,
		       FIXTEXT (FIX_SUM_SERV),
		       FIXTEXT (FIX_SUM_QTY),
		       FIXTEXT (FIX_AMOUNT));
	 iBody   = LAY_SUM_LIN04_A;
	 break;
       case 'S':
	 vdAddElement (pLay, LAY_SUM_LIN04_HS, 3,
		       FIXTEXT (FIX_SUM_SERV),
		       FIXTEXT (FIX_SUM_QTY),
		       FIXTEXT (FIX_AMOUNT));
	 iBody   = LAY_SUM_LIN04_S;
	 break;
       default:
	 /* should never be reached */
	 ASSERT (FALSE);
	 break;
       }
     }

     while (pstG22 && pstG22->lin &&
	    (strcmp (pstG22->lin->v_1222, LIN_NESTING_04) == 0))
     {
       lin = pstG22->lin;

       /* search IMD segment with service information */
       imd = pstG22->imd;

       while (imd) {
	 if (strcmp (imd->v_7009, "SN") == 0)
	   break;
	 imd = imd->imd_next;
       }

       /* search quantity information */
       qty = pstG22->qty;

       while (qty) {
	 if (strcmp (qty->v_6063, "107") == 0)
	   break;
	 qty = qty->qty_next;
       }

       /* search moa information */
       pstG23 = pstG22->g_23;

       moa = NULL;
       while (pstG23) {
	 ASSERT (pstG23->moa != NULL);
	 /* if ((strcmp (pstG23->moa->v_5025, "125") == 0) && ) */
	 if (strcmp (pstG23->moa->v_4405, "9") == 0)
	 {
	   moa = pstG23->moa;
	   break;
	 }
	 pstG23 = pstG23->g_23_next;
       }

       /* clear the pointers in case pia doesnt exist */
       szType  = BLANK_CHAR;
       szPpU   = BLANK_CHAR;
       szTtdes = BLANK_CHAR;
       szZndes = BLANK_CHAR;

	/* get PIA segment */
       pia = pstG22->pia;

       if (pia != NULL) {
	 /*
	  * PIA exists, now retrieve the information hidden between dots
	  * For this, we go through the subarticle number replacing all
	  * dots by '\0' and memorizing the pointers to the sub-strings
	  * for appending them to the output.
	  */

	  /* remember pointer to type indicator */
	 szType = pia->v_7140;

	 /* search price per unit indicator */
	 if (szType != NULL) {
	   szPpU = strchr (szType, '.');
	   if (szPpU != NULL) {
#if 0
	     *szPpU = '\0';		/* replace dot by string terminator */
#endif
	     szPpU++;
	   }
	 }

	 /* search tariff time */
	 if (szPpU != NULL) {
	   szTtdes = strchr (szPpU, '.');
	   if (szTtdes != NULL) {
#if 0
	     *szTtdes = '\0';	/* replace dot by string terminator */
#endif
	     szTtdes++;
	   }
	 }

	 /* search tariff zone */
	 if (szTtdes != NULL) {
	   szZndes = strchr (szTtdes, '.');
	   if (szZndes != NULL) {
	     *szZndes = '\0';	/* replace dot by string terminator */
	     szZndes++;
	   }
	 }
       }


       vdAddElement (pLay, iBody, 6,
		     COND_FETCH (imd, v_7008a),
		     COND_FETCH (qty, v_6060),
		     COND_FETCH (moa, v_5004),
		     COND_FETCH (moa, v_6345),
		     COND_FETCH (lin, v_7140), 	/* Article code     */
		     COND_FETCH (pia, v_7140));

       pstG22 = pstG22->g_22_next;
     }
   }

   fovdPopFunctionName();
   return (pstG22);
}
/*---------------------------------------------------------------------------*/


/******************************************************************************
 * fostPrintSim
 *
 * DESCRIPTION:
 *
 *
 * PARAMETERS:
 *  stTMSG		*timm		- pointer to TIMM
 *  struct s_group_22	*pstG22		- Group 22 sorted
 *  stLAYINF		*pLay		- layout information
 *
 * RETURNS:
 *  struct s_group_22 *
 ******************************************************************************
 */
static struct s_group_22 *fostPrintSim (stTMSG *timm,
					struct s_group_22 *pstG22, 
					stLAYINF *pLay)
{
   struct s_imd_seg *imd;
   struct s_imd_seg *imd_sim;
   struct s_imd_seg *imd_msisdn;
   struct s_moa_seg *moa;
   struct s_moa_seg *moa_subtotal;
#if XCDINSUMSHEET
   struct s_moa_seg *moaThr;
#endif

   short losReturn;
   short lofSubCharge;
   short lofAccCharge;
   short lofUsgCharge;
   short lofLoop;

   char *szNumber;		/* pointer to MSISDN or DNNUM */
   char	*szCard;		/* pointer to SIM or SMNUM */

   fovdPushFunctionName("fostPrintSim");

   /* initializations */
   losReturn  = 0;
   imd        = NULL;
   imd_sim    = NULL;
   imd_msisdn = NULL;

   /* set pointer according to TIMM version */
   if (timm->unh->version >= 304) {
     szNumber = "DNNUM";
     szCard   = "SMNUM";
   } else {
     szNumber = "MSISDN";
     szCard   = "SIM";
   }

   /* get pointers for TIMM information */

   if (pstG22)
   {
      while (pstG22 && pstG22->lin)
      {
	if (strcmp (pstG22->lin->v_1222, LIN_NESTING_02) == 0)
	  break;
	pstG22 = pstG22->g_22_next;
      } 

      /* get imd segments with SIM and MSISDN number */

      imd_sim    = NULL;
      imd_msisdn = NULL;
      imd = pstG22->imd;         
      while (imd)
      {
	if (strcmp (imd->v_7009, szCard) == 0) {
	  imd_sim = imd;
	} else if (strcmp (imd->v_7009, szNumber) == 0) {
	  imd_msisdn = imd;
	}
	if (imd_sim && imd_msisdn)
	  break;
	imd = imd->imd_next;
      }

      /* get correct moa segment */
      if (pstG22->g_23 && losReturn == 0)
      {
         moa = pstG22->g_23->moa;

         while (moa)
         {
	   if (strcmp (moa->v_5025, MOA_SUBTOTAL_CODE) == 0)
	     break;
	   moa = moa->moa_next;
         }

         moa_subtotal = moa;

         /* now print text */

         /*****************************************************************/
         /* Get further LIN segments with nesting level "03" for detailed */
         /* information about subscription,access and usage charges.      */
         /*****************************************************************/

	 /* search first LIN_NESTING_03 after current with nesting 02 */
         while (pstG22 && pstG22->lin)
         {
	   if (strcmp (pstG22->lin->v_1222, LIN_NESTING_03) == 0)
	     break;
	   pstG22 = pstG22->g_22_next;
         }

         lofLoop      = TRUE;
         lofSubCharge = FALSE;
         lofAccCharge = FALSE;
         lofUsgCharge = FALSE;

         do
         {
            while (pstG22 && pstG22->lin)
            {
	      if ((strcmp (pstG22->lin->v_1222, LIN_NESTING_04) != 0) &&
	          (strcmp (pstG22->lin->v_1222, LIN_NESTING_05) != 0))
		break;
	      pstG22 = pstG22->g_22_next;
            }

            imd = pstG22->imd;

            if (imd == NULL)
            {
               macErrorMessage (PREP_NO_G22_IMD, WARNING,
				"No imd segment in group 22 available at all.");
               losReturn = (int) PREP_NO_G22_IMD;
	       pstG22 = pstG22->g_22_next;
               continue;
            }

            moa = NULL;
            if (pstG22->g_23)
            {
               moa = pstG22->g_23->moa;
            }

            if (moa == NULL)
            {
               macErrorMessage (PREP_NO_G22_MOA, WARNING,
				"No moa segment in group 22 available at all.");
               losReturn = (int) PREP_NO_G22_MOA;
	       pstG22 = pstG22->g_22_next;
               continue;
            }

            if (imd->v_7008a[1] == '\0') {
	      switch (imd->v_7008a[0]) {
	      case 'S':		/* subscription */
		lofSubCharge   = TRUE;

		vdAddElement (pLay, LAY_SUM_SUBSCRIPTION, 3,
			      FIXTEXT (FIX_SUM_S),
			      COND_FETCH (moa, v_5004),
			      COND_FETCH (moa, v_6345));
		break;

	      case 'A':		/* Access */
		lofAccCharge = TRUE;

		vdAddElement (pLay, LAY_SUM_ACCESS, 3,
			      FIXTEXT (FIX_SUM_A),
			      COND_FETCH (moa, v_5004),
			      COND_FETCH (moa, v_6345));
		break;

	      case 'U':		/* Usage */
		lofUsgCharge = TRUE;

		vdAddElement (pLay, LAY_SUM_USAGE, 3,
			      FIXTEXT (FIX_SUM_U),
			      COND_FETCH (moa, v_5004),
			      COND_FETCH (moa, v_6345));
		break;

	      case 'O':		/* Other */
		/*
		** To be done !!!
		*/
		break;

	      default:
		sprintf (laszErrMsg, "Charge type %s is undefined",
			 imd->v_7008a);
		macErrorMessage (PREP_UNDEF_IMD_CHARGE_TYPE, WARNING,
				 laszErrMsg);
		losReturn = (int) PREP_UNDEF_IMD_CHARGE_TYPE;
		pstG22 = pstG22->g_22_next;
		continue;
	      }

	      /* add LIN04 elements if there are some */
	      pstG22 = pstG22->g_22_next;

	      if (strcmp (pstG22->lin->v_1222, LIN_NESTING_04) == 0)
	      {
		pstG22 = fostPrintSimServices (pstG22, imd, pLay);
	      }
	    } else {
		sprintf (laszErrMsg, "Charge type %s is undefined",
			 imd->v_7008a);
		macErrorMessage (PREP_UNDEF_IMD_CHARGE_TYPE, WARNING,
				 laszErrMsg);
		losReturn = (int) PREP_UNDEF_IMD_CHARGE_TYPE;
		pstG22 = pstG22->g_22_next;
		continue;
	    }

            if (lofSubCharge && lofUsgCharge && lofAccCharge)
            {
               lofLoop = FALSE;
            }

         } while (pstG22      	 &&
		  pstG22->lin    &&
		  losReturn == 0 &&
		  lofLoop        &&
		   (0 == strcmp (pstG22->lin->v_1222, LIN_NESTING_03) ||
		    0 == strcmp (pstG22->lin->v_1222, LIN_NESTING_04) ||
		    0 == strcmp (pstG22->lin->v_1222, LIN_NESTING_05)));

         /***************************************/
         /* Now the detailed lines are printed  */
         /***************************************/
#if XCDINSUMSHEET
	 if ((pstG22      != NULL)	&&
	     (pstG22->lin != NULL)	&&
	     (strcmp (pstG22->lin->v_1222, LIN_NESTING_03) == 0))
	 {
	   /*
	    * Print threshold information, if present
	    */
	   if (pstG22->g_23) {
	     moaThr = pstG22->g_23->moa;

	     if (moaThr 				&&
		 (strcmp (moaThr->v_5025, "901") == 0)	&&
		 (moaThr->v_5004[0] != '\0')		&&
		 (atof (moaThr->v_5004) != 0.0))
	     {
	       vdAddElement (pLay, LAY_ITB_THRESHOLD, 3,
			     FIXTEXT (FIX_THRESHOLD),
			     COND_FETCH (moaThr, v_5004),
			     COND_FETCH (moaThr, v_6345));
	     }
	   }
	   /*
	    * Check for XCD, print it if present
	    */
	   if ((pstG22->g_99      != NULL) &&
	       (pstG22->g_99->xcd != NULL))
	   {
	      /* distinguish between TIMM versions */
	      if (timm->unh->version >= 302) {
		(void) foiPrintXcd3xx (pstG22, timm, pLay);
	      } else {
		(void) foiPrintXcd (pstG22, timm, pLay);
	      }
	   }
	   else
	   {
	     vdAddElement (pLay, LAY_ITB_NO_CALL_REC_12, 1,
			   FIXTEXT (FIX_NO_CALLS_1));
	     vdAddElement (pLay, LAY_ITB_NO_CALL_REC_3, 1,
			   FIXTEXT (FIX_NO_CALLS_2));
	   }
	 }
#endif
         /* print block */

         vdAddElement(pLay, LAY_SUM_SIM_NUMBER, 4,
                      FIXTEXT (FIX_IMD_SIM),
                      COND_FETCH (imd_sim, v_7008a),
                      FIXTEXT (FIX_IMD_MSISDN),
                      COND_FETCH (imd_msisdn, v_7008a));

         vdAddElement(pLay, LAY_SUM_HEADER, 2,
                      FIXTEXT (FIX_SUM_TITLE1),      /* Betrag */
                      FIXTEXT (FIX_SUM_TITLE2));     /* brutto */

         vdAddElement(pLay, LAY_SUM_SUBTOTAL, 3,
                      FIXTEXT (FIX_SUM_SUB),      	/* Subtotal */
                      COND_FETCH (moa_subtotal, v_5004),
                      COND_FETCH (moa_subtotal, v_6345));

      }
      else
      {
         macErrorMessage (PREP_NO_G23_MOA, WARNING,
			  "No group 23 (MOA) available at all.");
         losReturn = (int) PREP_NO_G23_MOA;
      }

   }
   else
   {
      macErrorMessage (PREP_INVALID_G22_SORTED, WARNING,
		       "Invalid group 22 pointer was passed as parameter.");
      losReturn = (int) PREP_INVALID_G22_SORTED;
   }

   if (losReturn == 0)
   {
      fovdPopFunctionName();
      return (pstG22);
   }
   else
   {
      fovdPopFunctionName();
      return ((struct s_group_22 *) NULL);
   }
}
/*---------------------------------------------------------------------------*/


/******************************************************************************
 * foiPrintSumIds
 *
 * DESCRIPTION:
 * print invoice number and customer code ...
 * 
 *
 * PARAMETERS:
 *  stTMSG	*timm_message		- TIMM-structure
 *  short	*posIsFlatSubscr	- info about flat subscriber
 *  stLAYINF	*pLay			- layout information
 *
 * RETURNS:
 *  - void -
 ******************************************************************************
 */
static int foiPrintSumIds (stTMSG *timm_message,
			   short *posIsFlatSubscr, stLAYINF *pLay)
{
   int            	losReturn;
   struct s_group_1 	*g_1;
   struct s_group_2 	*g_2;
   struct s_group_3 	*g_3;
   struct s_rff_seg 	*rff_it;

   fovdPushFunctionName("foiPrintSumIds");

   /* initialisation */

   losReturn    = 0;
   g_1          = NULL;
   g_2          = NULL;
   g_3          = NULL;
   rff_it       = NULL;

  if (timm_message->unh->version >= 302) {
    /* for TIMM version > 302 */
    g_2 = timm_message->g_2;
    while (g_2 != NULL)
    {
      ASSERT (g_2->nad != NULL);

      if (strcmp (g_2->nad->v_3035, "IV") == 0) {
	/* found the right group, now search the RFFs */
	g_3 = g_2->g_3;

	while (g_3 != NULL)
	{
	  ASSERT (g_3->rff != NULL);

	  if (strcmp (g_3->rff->v_1153, RFF_IT) == 0) {
	    rff_it = g_3->rff;
	    break;
	  }
	  g_3 = g_3->g_3_next;
	}
	break;
      }
      g_2 = g_2->g_2_next;
    }

    if (rff_it == NULL) {
      /*
      ** No information under the invoicee, it might
      ** now be that, due to an BCH defect, the information
      ** is packed to the temporary address,
      ** -> search it there !!
      */
      g_2 = timm_message->g_2;
      while (g_2 != NULL) {
	ASSERT (g_2->nad != NULL);

	if (strcmp (g_2->nad->v_3035, "IT") == 0) {
	  /* found the right group, now search the RFFs */
	  g_3 = g_2->g_3;

	  while (g_3 != NULL) {
	    ASSERT (g_3->rff != NULL);

	    if (strcmp (g_3->rff->v_1153, RFF_IT) == 0) {
	      rff_it = g_3->rff;
	    }
	    if (rff_it != NULL)		break;
	    g_3 = g_3->g_3_next;
	  }
	  break;
	}
	g_2 = g_2->g_2_next;
      }
    }
  } else {
    /* get internal customer code */
    g_1 = timm_message->g_1;
    while (g_1 != NULL)
    {
      ASSERT (g_1->rff != NULL);

      if (strcmp (g_1->rff->v_1153, RFF_IT) == 0) {
	rff_it = g_1->rff;
	break;
      }
      g_1 = g_1->g_1_next;
    }
  }

  if (rff_it != NULL)
  {
    if (rff_it->v_1154[0] == CUSTCODE_FLAT)
    {
      *posIsFlatSubscr = TRUE;
    }
    else
    {
      *posIsFlatSubscr = FALSE;
    }
  }

  /* print additional information */
  fovdPrintCInfo (timm_message, SUM_TYPE, pLay);

  fovdPopFunctionName();
  return( losReturn);
}
/*--------------------------------------------------------------------------*/

/******************************************************************************
 * fovdPrintSum
 *
 * DESCRIPTION:
 * Function for printing TIMM sumsheet
 * 
 *
 * PARAMETERS:
 *  stTMSG	*timm_message		- TIMM-structure
 *  stLAYINF	*pLay			- layout information
 *
 * RETURNS:
 *  - void -
 ******************************************************************************
 */
static short fovdPrintSum (stTMSG *timm_message, stLAYINF *pLay)
{

   short losReturn = 0;
   short losIsFlatSubscr;

   fovdPushFunctionName("fovdPrintSum");

   /* put the sender address (page header)                      */
   fovdPrintSender (timm_message, pLay);

   /* put the title                                             */
   fovdPrintTitle (FIX_SUM, timm_message, pLay);

   /* put the invoice number and customer_code                  */
   foiPrintSumIds (timm_message, &losIsFlatSubscr, pLay);

   /* put the receiver address                                  */
   fovdPrintRec (timm_message, pLay);

   /* put subaddr, SIM and other blocks                         */
   /* (as many times as they are available                      */

   losReturn = foiPrintSumBlock (timm_message, losIsFlatSubscr, pLay);

   /* put "total block"                                         */
   if( losReturn == 0)
   {
      losReturn = foiPrintSumTotal (timm_message, pLay);  
   }

   fovdPopFunctionName();
   return( losReturn);

}
/*----------------------------------------------------------------*/


/******************************************************************************
 * fovdPrintTitle
 *
 * DESCRIPTION:
 * print title
 * 
 *
 * PARAMETERS:
 *  int		type		- type of document
 *  stTMSG	*timm		- TIMM-structure
 *  stLAYINF	*pLay		- layout information
 *
 * RETURNS:
 *  - void -
 ******************************************************************************
 */
static void fovdPrintTitle (int type,
			    stTMSG *timm,
			    stLAYINF *pLay)
{
  struct s_group_22 *g_22;
  struct s_imd_seg  *imd;
  struct s_imd_seg  *sim;
  struct s_imd_seg  *msisdn;

  char  *szNumber;		/* pointer to MSISDN or DNNUM */
  char	*szCard;		/* pointer to SIM or SMNUM */

  fovdPushFunctionName("fovdPrintTitle");

  /* initialization */

  g_22   = NULL;
  imd    = NULL;
  msisdn = NULL;
  sim    = NULL;

  /* set pointer according to TIMM version */
  if (timm->unh->version >= 304) {
    szNumber = "DNNUM";
    szCard   = "SMNUM";
  } else {
    szNumber = "MSISDN";
    szCard   = "SIM";
  }

  if (type == FIX_ITB)
  {
    /* SIM and MSISDN are in the first lin segment (first G 22) */
    /* get imd segment with MSISDN number */
    if (timm->g_22 != NULL)
    {
      imd = timm->g_22->imd;
      while (imd)
      {
	if (strcmp (imd->v_7009, szNumber) == 0)
	  break;
	imd = imd->imd_next;
      }
      msisdn = imd;
    }

    if (msisdn == NULL)
    {
      macErrorMessage (PREP_IMD_NO_SIM_NUMBER, WARNING,
		       "ITB: No DNNUM/MSISDN available.");
    }

    /* get imd segment with SIM number */
    if (timm->g_22 != NULL)
    {
      imd = timm->g_22->imd;
      while (imd)
      {
	if (strcmp (imd->v_7009, szCard) == 0)
	  break;
	imd = imd->imd_next;
      }
      sim = imd;
    }

    if (sim == NULL)
    {
      macErrorMessage (PREP_IMD_NO_SIM_NUMBER, WARNING,
		       "ITB: No SMNUM/SIM available.");
    }

    if (imd)
    {
      if (!fofIsPrinted (pLay, LAY_ITB_FIX_TO_SIM)) {
	vdAddElement(pLay, LAY_ITB_FIX_TO_SIM, 2,
		     FIXTEXT (FIX_TO_SIM),
		     COND_FETCH (imd, v_7008a));
      }
    }
    if (msisdn)
    {
      if (!fofIsPrinted (pLay, LAY_ITB_FIX_MSISDN)) {
	vdAddElement(pLay, LAY_ITB_FIX_MSISDN, 2,
		     FIXTEXT (FIX_MSISDN),
		     COND_FETCH (msisdn, v_7008a));
      }
    }
  }

  if (!fofIsPrinted (pLay, LAY_FIX_TELECOM))
  {
    if ((type == FIX_SUM) ||
	(type == FIX_ITB))
    {
      vdAddElement(pLay, LAY_FIX_HEADER, 1, FIXTEXT (type));
      vdAddElement(pLay, LAY_FIX_TELECOM, 1, FIXTEXT (FIX_TELECOM));
    }
  }

  fovdPopFunctionName();
}
/*---------------------------------------------------------------------*/


/******************************************************************************
 * fovdPrintItb
 *
 * DESCRIPTION:
 * Function for printing TIMM itemised bill
 * 
 *
 * PARAMETERS:
 *  stTMSG	*timm_message		- TIMM-structure
 *  stLAYINF	*pLay			- layout information
 *
 * RETURNS:
 *  - void -
 ******************************************************************************
 */
static void fovdPrintItb (stTMSG *timm_message, stLAYINF *pLay)
{
  fovdPushFunctionName ("fovdPrintItb");

  fovdPrintSender    (timm_message, pLay);
  fovdPrintTitle     (FIX_ITB, timm_message, pLay);

  fovdPrintRec       (timm_message, pLay);
  fovdPrintItbDate   (timm_message, pLay);
  fovdPrintItbEmpty  (timm_message, pLay);

  fovdPopFunctionName();
}
/*---------------------------------------------------------------------*/



/******************************************************************************
 * loiPrintXcdElem
 *
 * DESCRIPTION:
 * print a XCD
 * 
 *
 * PARAMETERS:
 *  struct s_xcd_seg *pXcd		- XCD element
 *  int iTable				- printing table
 *  stLAYINF	*pLay			- layout information
 *
 * RETURNS:
 *  int
 ******************************************************************************
 */
static short loiPrintXcdElem (struct s_xcd_seg *pXcd,
			      int iTable,
			      stLAYINF *pLay)
{
  short		losReturn;
  char          *lpchCRTimeStamp;
  char   	*lpchDurationBuf2 = NULL;
  char		*pszZone;
#if CALLRATINGTYPE
  char 	     	*pszTmp;
#endif

  losReturn = 0;

  ASSERT (pXcd != NULL);

  /*
   * The tariff zone is in different positions for TIMM 2.09 and
   * TIMM 2.10 ... in versions prior to 2.10 field X009a does not
   * exist and is therefore always empty (in 2.10 field X022 does not
   * exist and is always empty)
   */
  if (pXcd->v_X009a[0] == '\0') {
    pszZone = pXcd->v_X022;
  } else {
    pszZone = pXcd->v_X009a;
  }

  /* format the timestamp of an XCD segment         */

  lpchCRTimeStamp = foszFormTime (pXcd->v_X005);
  if (lpchCRTimeStamp != NULL)
  {

    if (iTable != LAY_ITB_ITEMS_TABLE_6)	/* != additional services */
    {
      /* format the volume and measure unit qualifier      */

      if (0 == strcmp (pXcd->v_6411, X6411_UNIT_SEC))
      {
	/* build buffer with volume (duration) */
	lpchDurationBuf2 = foszFormDur (pXcd->v_X006);
	if (lpchDurationBuf2 == NULL)
	{
	  return (-1);
	}
      }
      else
      {
	/* build buffer with volume (duration) */
	lpchDurationBuf2 = fpvdGetPrepMemory (32);
	sprintf (lpchDurationBuf2, "%s [%s]",
		 pXcd ->v_X006, pXcd->v_6411);
      }
    }

#if CALLRATINGTYPE
    /* format late call indicator and rate indicator            */

    if( 0 == strcmp( pXcd->v_X012, X012_NORMAL_CALL))
    {
      pXcd->v_X012[0] = '\0';
    }

    if( 0 == strcmp( pXcd->v_X012, X012_LATE_CALL) &&
	0 == strcmp( pXcd->v_X013, X013_NORMAL_CALL))
    {
      pXcd->v_X013[0] = '\0';
    }

    pszTmp = fpvdGetPrepMemory (sizeof (pXcd->v_X012) + 
				sizeof (pXcd->v_X013) + 1);
    pszTmp[0] = '\0';
    if (pXcd->v_X012[0] != '\0')
      strcpy (pszTmp, pXcd->v_X012);
    if (pXcd->v_X013[0] != '\0')
      strcat (pszTmp, pXcd->v_X013);
    if (pszTmp[0] == '\0')
      strcpy (pszTmp, " ");

    /*
     * now pszTmp may be:  F  free rating
     *			   P  partially free
     *			   N  normal
     * 			   L  late call
     *   		   LF late call, free
     * 			   LP late call, partially free
     */
#endif

    switch (iTable) {
    case LAY_ITB_ITEMS_TABLE_3:		/* --- print GSM-outbound visitor --- */
      vdAddElement (pLay, LAY_ITB_ITEMS_TABLE_3,
#if CALLRATINGTYPE
		    10,
#else
		    9,
#endif
		    lpchCRTimeStamp,
		    lpchDurationBuf2,
		    COND_FETCH(pXcd, v_X020),
		    COND_FETCH(pXcd, v_X019),
		    COND_FETCH(pXcd, v_X008),
#if CALLRATINGTYPE
		    pszTmp,
#endif
		    COND_FETCH(pXcd, v_5004),
		    COND_FETCH(pXcd, v_6345),
		    COND_FETCH(pXcd, v_X007), 	/* clicks */
		    COND_FETCH(pXcd, v_X023));	/* zonepoint description */
      break;

    case LAY_ITB_ITEMS_TABLE_4:		/* --- print GSM-inbound home --- */
      /* FALLTHROUGH */
    case LAY_ITB_ITEMS_TABLE_5:		/* --- print GSM-inbound visitor --- */
      vdAddElement (pLay, iTable,
#if CALLRATINGTYPE
		    9,
#else
		    8,
#endif
		    lpchCRTimeStamp,
		    lpchDurationBuf2,
		    COND_FETCH(pXcd, v_X025),
		    COND_FETCH(pXcd, v_X019),
		    COND_FETCH(pXcd, v_X008),
#if CALLRATINGTYPE
		    pszTmp,
#endif
		    COND_FETCH(pXcd, v_5004),
		    COND_FETCH(pXcd, v_6345),
		    COND_FETCH(pXcd, v_X007)); 	/* clicks */
      break;

    case LAY_ITB_ITEMS_TABLE_6:		/* --- print additional services --- */
      vdAddElement (pLay, LAY_ITB_ITEMS_TABLE_6,
#if CALLRATINGTYPE
		    7,
#else
		    6,
#endif
		    lpchCRTimeStamp,
		    COND_FETCH(pXcd, v_X026),
		    COND_FETCH(pXcd, v_X008),
#if CALLRATINGTYPE
		    pszTmp,
#endif
		    COND_FETCH(pXcd, v_5004),
		    COND_FETCH(pXcd, v_6345),
		    COND_FETCH(pXcd, v_X007)); 	/* clicks */
      break;

    default:
      /* --- print national GSM-outbound home --- */
      /* --- print international GSM-outbound home --- */
      vdAddElement (pLay, iTable,
#if CALLRATINGTYPE
		    11,
#else
		    10,
#endif
		    lpchCRTimeStamp,
		    lpchDurationBuf2,
		    COND_FETCH(pXcd, v_X020),
		    pszZone,
		    COND_FETCH(pXcd, v_X009),
		    COND_FETCH(pXcd, v_X008),
#if CALLRATINGTYPE
		    pszTmp,
#endif
		    COND_FETCH(pXcd, v_5004),
		    COND_FETCH(pXcd, v_6345),
		    COND_FETCH(pXcd, v_X007), 	/* clicks */
		    COND_FETCH(pXcd, v_X023));	/* zonepoint description */
      break;
    }
  } else {
    losReturn = -1;
  }

  if (losReturn == 0) {
     fovdPrintXcdElHead (iTable, pLay);
  }
  return (losReturn);
}


/******************************************************************************
 * fovdPrintXcdElHead
 *
 * DESCRIPTION:
 * print group header for xcd table
 * 
 *
 * PARAMETERS:
 *  stTMSG	*timm_message		- TIMM-structure
 *  int		iType			- type of XCD group
 *  stLAYINF	*pLay			- layout information
 *
 * RETURNS:
 *  - void -
 ******************************************************************************
 */
static void fovdPrintXcdElHead (int	iType,
				stLAYINF *pLay)
{
   fovdPushFunctionName("fovdPrintXcdElHead");

   switch (iType) {

   case LAY_ITB_ITEMS_TABLE_1:     /* --- print national GSM-outbound home --- */
     /* print table body    */
     if (! fHeader1) {
       fHeader1 = TRUE;
       vdAddElement (pLay, LAY_ITB_FIX_HEADER_1,
#if CALLRATINGTYPE
		     9,
#else
		     8,
#endif
		     FIXTEXT (FIX_NL_OUT_HOME),
		     FIXTEXT (FIX_XCD_TME),
		     FIXTEXT (FIX_XCD_AMT),
		     FIXTEXT (FIX_XCD_DIAL),
		     FIXTEXT (FIX_XCD_ZONE),
		     FIXTEXT (FIX_XCD_TTME),
		     FIXTEXT (FIX_XCD_SVCE),
#if CALLRATINGTYPE
		     FIXTEXT (FIX_XCD_RTYP),
#endif
		     FIXTEXT (FIX_XCD_MAMT));
     }
     break;

   case LAY_ITB_ITEMS_TABLE_2:   /* --- print international GSM-outbound home --- */

     if (! fHeader2) {
       fHeader2 = TRUE;
       vdAddElement (pLay, LAY_ITB_FIX_HEADER_2,
#if CALLRATINGTYPE
		     9,
#else
		     8,
#endif
		     FIXTEXT (FIX_IL_OUT_HOME),
		     FIXTEXT (FIX_XCD_TME),
		     FIXTEXT (FIX_XCD_AMT),
		     FIXTEXT (FIX_XCD_DIAL),
		     FIXTEXT (FIX_XCD_ZONE),
		     FIXTEXT (FIX_XCD_TTME),
		     FIXTEXT (FIX_XCD_SVCE),
#if CALLRATINGTYPE
		     FIXTEXT (FIX_XCD_RTYP),
#endif
		     FIXTEXT (FIX_XCD_MAMT));
     }
     break;


   case LAY_ITB_ITEMS_TABLE_3:   /* --- print GSM-outbound visitor --- */

     if (! fHeader3) {
       fHeader3 = TRUE;
       vdAddElement (pLay, LAY_ITB_FIX_HEADER_3,
#if CALLRATINGTYPE
		     8,
#else
		     7,
#endif
		     FIXTEXT (FIX_OUT_VISITOR),
		     FIXTEXT (FIX_XCD_TME),
		     FIXTEXT (FIX_XCD_AMT),
		     FIXTEXT (FIX_XCD_DIAL),
		     FIXTEXT (FIX_XCD_NET),
		     FIXTEXT (FIX_XCD_SVCE),
#if CALLRATINGTYPE
		     FIXTEXT (FIX_XCD_RTYP),
#endif
		     FIXTEXT (FIX_XCD_MAMT));
     }
     break;

   case LAY_ITB_ITEMS_TABLE_4:   /* --- print GSM-inbound home --- */

     if (! fHeader4) {
       fHeader4 = TRUE;
       vdAddElement (pLay, LAY_ITB_FIX_HEADER_4,
#if CALLRATINGTYPE
		     8,
#else
		     7,
#endif
		     FIXTEXT (FIX_IN_HOME),
		     FIXTEXT (FIX_XCD_TME),
		     FIXTEXT (FIX_XCD_AMT),
		     FIXTEXT (FIX_XCD_CALL),
		     FIXTEXT (FIX_XCD_NET),
		     FIXTEXT (FIX_XCD_SVCE),
#if CALLRATINGTYPE
		     FIXTEXT (FIX_XCD_RTYP),
#endif
		     FIXTEXT (FIX_XCD_MAMT));
     }
     break;

   case LAY_ITB_ITEMS_TABLE_5:   /* --- print GSM-inbound visitor --- */

     if (! fHeader5) {
       fHeader5 = TRUE;
       vdAddElement (pLay, LAY_ITB_FIX_HEADER_5,
#if CALLRATINGTYPE
		     7,
#else
		     6,
#endif
		     FIXTEXT (FIX_IN_VISITOR),
		     FIXTEXT (FIX_XCD_AMT),
		     FIXTEXT (FIX_XCD_CALL),
		     FIXTEXT (FIX_XCD_NET),
		     FIXTEXT (FIX_XCD_SVCE),
#if CALLRATINGTYPE
		     FIXTEXT (FIX_XCD_RTYP),
#endif
		     FIXTEXT (FIX_XCD_MAMT));
     }
     break;

   case LAY_ITB_ITEMS_TABLE_6:   /* --- print additional services --- */

     if (! fHeader6) {
       fHeader6 = TRUE;
       vdAddElement (pLay, LAY_ITB_FIX_HEADER_6,
#if CALLRATINGTYPE
		     6,
#else
		     5,
#endif
		     FIXTEXT (FIX_SERVICES),
		     FIXTEXT (FIX_XCD_TME),
		     FIXTEXT (FIX_XCD_REM),
		     FIXTEXT (FIX_XCD_SVCE),
#if CALLRATINGTYPE
		     FIXTEXT (FIX_XCD_RTYP),
#endif
		     FIXTEXT (FIX_XCD_MAMT));
     }
     break;
   }

   fovdPopFunctionName();
}
/*--------------------------------------------------------------------*/


/******************************************************************************
 * fovdPrintItbDate
 *
 * DESCRIPTION:
 * print itemised bill date and number and ...
 * 
 *
 * PARAMETERS:
 *  stTMSG	*timm_message		- TIMM-structure
 *  stLAYINF	*pLay			- layout information
 *
 * RETURNS:
 *  - void -
 ******************************************************************************
 */
static void fovdPrintItbDate (stTMSG *timm_message,
			      stLAYINF *pLay)
{
   struct s_moa_seg  *moa_threshold;
   struct s_group_22 *g_22_threshold;


   fovdPushFunctionName("fovdPrintItbDate");

   /* initialisation                                            */

   moa_threshold  = NULL;
   g_22_threshold = NULL; 


   /* get lin segment with threshold for XCDs                   */

   if( timm_message->g_22 )
   {
      g_22_threshold =  timm_message->g_22;


      /* get the g_22 group which contains all xcd segments */
      while(( g_22_threshold != NULL) && (g_22_threshold->g_99 == NULL ))
      {
         g_22_threshold = g_22_threshold->g_22_next;
      }

      if( g_22_threshold == NULL )
      {
#if 0
        macErrorMessage (PREP_NO_XCDS, WARNING,
                         "No xcd segment within itemized bill message" );
#endif
      }
      else
      {
        if( g_22_threshold->g_23 )
        {
           moa_threshold = g_22_threshold->g_23->moa;
        }
        else
        {
          macErrorMessage (PREP_G23_MOA_NO_THRESHOLD, WARNING,
                           "No moa segment for threshold available.");
        }
      }
   }

   /* print threshold information                               */

   if (moa_threshold && (moa_threshold->v_5004[0] != '\0'))
   {
      vdAddElement(pLay, LAY_ITB_THRESHOLD, 3,
                   FIXTEXT (FIX_THRESHOLD),
                   COND_FETCH(moa_threshold,v_5004),
                   COND_FETCH(moa_threshold,v_6345));
   }

   fovdPrintCInfo (timm_message, ITB_TYPE, pLay);

   fovdPopFunctionName();
}
/*---------------------------------------------------------------------------*/



/******************************************************************************
 * fovdPrintItbEmpty
 *
 * DESCRIPTION:
 * print information for ITBs without call records
 * 
 *
 * PARAMETERS:
 *  stTMSG	*timm_message		- TIMM-structure
 *  stLAYINF	*pLay			- layout information
 *
 * RETURNS:
 *  - void -
 ******************************************************************************
 */
static void fovdPrintItbEmpty	(stTMSG *timm_message,
				 stLAYINF *pLay)
{
   short             lofCallRecords;
   struct s_group_22 *pstG22cur;

   fovdPushFunctionName("fovdPrintItbEmpty");

   /* check, if timm Message contains call records */

   pstG22cur = timm_message->g_22;
   if( pstG22cur )
   {
      lofCallRecords = FALSE;
      while( pstG22cur && lofCallRecords == FALSE)
      {
        if( pstG22cur->g_99 )
        {
          lofCallRecords = TRUE;
        }
        else
        {
           pstG22cur = pstG22cur->g_22_next;
        }
      }
   }
   else
   {
      lofCallRecords = FALSE;
   }

   if( lofCallRecords == FALSE )
   {
#if 0
      macErrorMessage (PREP_ITB_NO_CALL_REC_INFO, WARNING,
                       "No information on call records in ITB");
#endif
      vdAddElement(pLay, LAY_ITB_NO_CALL_REC_12, 1,
                   FIXTEXT (FIX_NO_CALLS_1));
      vdAddElement(pLay, LAY_ITB_NO_CALL_REC_3, 1,
                   FIXTEXT (FIX_NO_CALLS_2));
   }

   fovdPopFunctionName();
}
/*---------------------------------------------------------------------------*/




/******************************************************************************
 * foszFormTime
 *
 * DESCRIPTION:
 * format and add points to timestamp segments 
 *
 * PARAMETERS:
 *  char	*elem			- pointer to date element
 *
 * RETURNS:
 *  char *	- pointer to formatted time stamp
 ******************************************************************************
 */
static char *foszFormTime (char *elem)
{
  char *laszTmp;
  char *pszCentennial;

  fovdPushFunctionName("foszFormTime");

  laszTmp = fpvdGetPrepMemory (32);

  if (laszTmp != NULL) {
    /*
     * Take care of year 2000 and greater
     */
    if (elem[0] < '9') {
      /* if we are under the year 90 */
      pszCentennial = "20";
    } else {
      pszCentennial = "19";
    }

    sprintf (laszTmp, "%c%c%c%c%c%c%s%c%c %c%c:%c%c:%c%c",
#ifdef EUROPEAN_DATE
	     elem[4], elem[5], DATE_SEPERATOR,
	     elem[2], elem[3], DATE_SEPERATOR,
#else
	     elem[2], elem[3], DATE_SEPERATOR,
	     elem[4], elem[5], DATE_SEPERATOR,
#endif
	     pszCentennial,
	     elem[0], elem[1],
	     elem[6], elem[7],
	     elem[8], elem[9],
	     elem[10], elem[11]);
  }

  fovdPopFunctionName ();
  return (laszTmp);
}
/*---------------------------------------------------------------------------*/


/******************************************************************************
 * foszFormDur
 *
 * DESCRIPTION:
 * format and add columns to duration segments
 *
 * PARAMETERS:
 *  char	*elem			- pointer to time element
 *
 * RETURNS:
 *  char *	- pointer to formatted time stamp
 ******************************************************************************
 */
static char *foszFormDur(char *elem)
{
   char *laszTmp;
   int 	hour,min,sec;
   int	sectotal;

   fovdPushFunctionName("foszFormDur");

   sectotal  	= atoi(elem);
   hour 	= sectotal/3600;
   min  	= (sectotal-3600*hour)/60;
   sec  	= sectotal-3600*hour-60*min;

   laszTmp = fpvdGetPrepMemory(DUR_STRLEN+7);
   /*
    * in fact, laszTmp will never be NULL, because if we ran out
    * of memory, fpvdGetPrepMemory would exit BGH
    */
   if (laszTmp != NULL) {
     if (sectotal < 0) {
       sprintf (laszTmp, "-:--:--%s", DURATION_QUALIFIER);
     } else {
       sprintf (laszTmp, "%01d:%02d:%02d%s", hour, min, sec, DURATION_QUALIFIER);
     }
   }

   fovdPopFunctionName();
   return (laszTmp);

}
/*---------------------------------------------------------------------------*/


/******************************************************************************
 * fovdPrintRoaTail
 *
 * DESCRIPTION:
 * print footer for roaming page
 * 
 *
 * PARAMETERS:
 *  stTMSG	*timm_message		- TIMM-structure
 *  stLAYINF	*pLay			- layout information
 *
 * RETURNS:
 *  - void -
 ******************************************************************************
 */
static void fovdPrintRoaTail (stTMSG *timm_message,
			      stLAYINF *pLay)
{
   struct s_group_45 *g_45;
   struct s_moa_seg  *moa_950;
   struct s_moa_seg  *moa_951;
   struct s_moa_seg  *moa_952;
   struct s_moa_seg  *moa_953;

   /* initialization */

   g_45    = NULL;
   moa_950 = NULL;
   moa_951 = NULL;
   moa_952 = NULL;
   moa_953 = NULL;


   /* get pointer moa_950                                        */
   /* (total net amount charged by all VPLMN's for all contracts */
   g_45 = timm_message->g_45;
   while (g_45)
   {
     ASSERT (g_45->moa != NULL);

     if (strcmp (g_45->moa->v_5025, "950") == 0)
       break;
     g_45 = g_45->g_45_next;
   }
   
   if (g_45)
   {
      moa_950 = g_45->moa;
   }
   else
   {
      macErrorMessage (PREP_NO_TOTAL_USAGE_AMOUNT, WARNING,
                       "ROAMING: No total net amount available." );
   }

   /* get pointer moa_951                                        */
   /* (total foreign tax raised by all VPLMN's for all contracts */
   g_45 = timm_message->g_45;
   while (g_45)
   {
     ASSERT (g_45->moa != NULL);

     if (strcmp (g_45->moa->v_5025, "951") == 0)
       break;
     g_45 = g_45->g_45_next;
   }
   
   if (g_45)
   {
      moa_951 = g_45->moa;
   }
   else
   {
      macErrorMessage (PREP_NO_TOTAL_USAGE_AMOUNT, WARNING,
                       "ROAMING: No total foreign tax available." );
   }

   /* get pointer moa_952                                        */
   /* (total usage amount raised by all VPLMN's for all contracts */
   g_45 = timm_message->g_45;
   while (g_45)
   {
     ASSERT (g_45->moa != NULL);

     if (strcmp (g_45->moa->v_5025,"952") == 0)
       break;
     g_45 = g_45->g_45_next;
   }
   
   if (g_45)
   {
      moa_952 = g_45->moa;
   }
   else
   {
      macErrorMessage (PREP_NO_TOTAL_USAGE_AMOUNT, WARNING,
                       "ROAMING: No total usage amount available." );
   }

   /* get pointer moa_953 */
   /* (total surcharge  amount raised by all HPLMN for all contracts */

   g_45 = timm_message->g_45;
   while (g_45)
   {
     ASSERT (g_45->moa != NULL);

     if (strcmp (g_45->moa->v_5025, "953") == 0)
       break;
     g_45 = g_45->g_45_next;
   }
   
   if (g_45)
   {
      moa_953 = g_45->moa;
   }
   else
   {
      macErrorMessage (PREP_NO_TOTAL_SURCHARGE_AMOUNT, WARNING,
                       "ROAMING: No total surcharge amount  available." );
   }


   vdAddElement (pLay, LAY_ROA_TOTAL, 9,
		 FIXTEXT (FIX_ROA_TOT),
		 COND_FETCH (moa_950, v_5004),
		 COND_FETCH (moa_950, v_6345),
		 COND_FETCH (moa_951, v_5004),
		 COND_FETCH (moa_951, v_6345),
		 COND_FETCH (moa_953, v_5004),
		 COND_FETCH (moa_953, v_6345),
		 COND_FETCH (moa_952, v_5004),
		 COND_FETCH (moa_952, v_6345));
}


/******************************************************************************
 * foiPrintRoam
 *
 * DESCRIPTION:
 * print roaming part
 * 
 *
 * PARAMETERS:
 *  stTMSG	*timm_message		- TIMM-structure
 *  stLAYINF	*pLay			- layout information
 *
 * RETURNS:
 *  0 if succesful
 ******************************************************************************
 */
static int foiPrintRoam (stTMSG *timm_message, stLAYINF *pLay)
{
  struct s_group_22 *g_22;
  struct s_group_23 *g_23;
  struct s_group_30 *g_30;
  struct s_tax_seg  *tax;
  struct s_imd_seg  *imd;
  struct s_imd_seg  *imdSIM;		/* SIM imd */
  struct s_imd_seg  *imdMSISDN;		/* MSISDN imd */
  struct s_moa_seg  *moa_940;
  struct s_moa_seg  *moa_941;
  struct s_moa_seg  *moa_942;
  struct s_moa_seg  *moa_943;
  BOOL		    fRoamL2;		/* flag for Roaming information */
  BOOL		    fFirstGroup;	/* used for grouping */
  UINT		    i;			/* temporary int variable */
  int		    iRet;		/* return value */

  char  *szNumber;			/* pointer to MSISDN or DNNUM */
  char	*szCard;			/* pointer to SIM or SMNUM */

  fovdPushFunctionName ("fovdPrintRoam");

  /* initialization */
  fFirstGroup 	= TRUE;
  fRoamL2  	= FALSE;
  g_22     	= NULL;
  g_23     	= NULL;
  g_30     	= NULL;
  moa_940  	= NULL;
  moa_941  	= NULL;
  moa_942  	= NULL;
  moa_943  	= NULL;
  tax      	= NULL;
  imd  		= NULL;
  imdSIM      	= NULL;
  imdMSISDN 	= NULL;
  iRet		= 0;

  /* set pointer according to TIMM version */
  if (timm_message->unh->version >= 304) {
    szNumber = "DNNUM";
    szCard   = "SMNUM";
  } else {
    szNumber = "MSISDN";
    szCard   = "SIM";
  }

  /* print lin body data elements */

  g_22 = timm_message->g_22;

  while (g_22)
  {
    /*
     * differentiate between nesting level 01 and 02
     * (roaming has only these two)
     */
    ASSERT (g_22->lin != NULL);

    if (strcmp (g_22->lin->v_1222, "02")) {

      /*
       * if it is not the first group, it has to be broken
       */
      if (fFirstGroup)
      {
	fFirstGroup = FALSE;
      } else {
	vdBreakList (pLay, 3,
		     LAY_ROA_SIM_NR,
		     LAY_ROA_FIX_DESCRIPTION,
		     LAY_ROA_ITEMS,
		     LAY_ROA_DUMMY);
      }

      /* print SIM number */

      imd = g_22->imd;

      while (imd)
      {
	if (strcmp (imd->v_7009, szCard) == 0)
	{
	  imdSIM = imd;
	} else
	if (strcmp (imd->v_7009, szNumber) == 0)
	{
	  imdMSISDN = imd;
	}

	imd = imd->imd_next;
      }
      vdAddElement (pLay, LAY_ROA_SIM_NR, 4,
		    FIXTEXT (FIX_IMD_SIM),
		    COND_FETCH (imdSIM, v_7008a),
		    FIXTEXT (FIX_IMD_MSISDN),
		    COND_FETCH (imdMSISDN, v_7008a));

      vdAddElement (pLay, LAY_ROA_DUMMY, 1, BLANK_CHAR);

      vdAddElement (pLay, LAY_ROA_FIX_DESCRIPTION, 6,
		    FIXTEXT (FIX_XCD_REM),
		    FIXTEXT (FIX_VAT_RATE),
		    FIXTEXT (FIX_MOA_942),
		    FIXTEXT (FIX_TAX_1),
		    FIXTEXT (FIX_MOA_943),
		    FIXTEXT (FIX_GROSS_VALUE));
    }
    else
    {
      fRoamL2 = TRUE;

      /* get pointer imd */
      imd = g_22->imd;
      while (imd)
      {
	if (strcmp (imd->v_7009, "VPLMN") == 0)
	  break;
	imd = imd->imd_next;
      }
      if (!imd)
      {
        macErrorMessage (PREP_NO_VPLMN_NAME, WARNING,
                         "ROAMING: No VPLMN name available" );
      }

      /* get pointer tax */

      g_30 = g_22->g_30;
      if (g_30)
      {
	tax = g_30->tax;
      }
      else
      {
        macErrorMessage (PREP_NO_TAX_NUMBER, WARNING,
                         "ROAMING: No tax number available.");
      }

      /* get pointer moa_940 */

      g_23 = g_22->g_23;
      while (g_23)
      {
	ASSERT (g_23->moa != NULL);

	if (strcmp (g_23->moa->v_5025, "940") == 0)
	  break;
	g_23 = g_23->g_23_next;
      }

      if (g_23)
      {
	moa_940 = g_23->moa;
      }
      else
      {
	macErrorMessage (PREP_NO_ROAMING_INFO, WARNING,
			 "ROAMING: No VPLMN net amount available.");
      }

      /* get pointer moa_941 */

      g_23 = g_22->g_23;
      while (g_23)
      {
	ASSERT (g_23->moa != NULL);

	if (strcmp (g_23->moa->v_5025, "941") == 0)
	  break;
	g_23 = g_23->g_23_next;
      }

      if (g_23)
      {
	moa_941 = g_23->moa;
      }
      else
      {
	macErrorMessage (PREP_NO_FOREIGN_TAX, WARNING,
			 "ROAMING: No foreign tax available." );
      }

      /* get pointer moa_942 */

      g_23 = g_22->g_23;
      while (g_23)
      {
	ASSERT (g_23->moa != NULL);

	if (strcmp (g_23->moa->v_5025, "942") == 0)
	  break;
	g_23 = g_23->g_23_next;
      }

      if (g_23)
      {
	moa_942 = g_23->moa;
      }
      else
      {
	macErrorMessage (PREP_NO_SUM_INCL_TAX, WARNING,
			 "ROAMING: No sum of net amount and foreign tax available." );
      }

      /* get pointer moa_943 */

      g_23 = g_22->g_23;
      while (g_23)
      {
	ASSERT (g_23->moa != NULL);

	if (strcmp (g_23->moa->v_5025, "943") == 0)
	  break;
	g_23 = g_23->g_23_next;
      }

      if (g_23)
      {
	moa_943 = g_23->moa;
      }
      else
      {
	macErrorMessage (PREP_NO_ROAMING_INFO, WARNING,
			 "ROAMING: No surcharge amount for outgoing roaming calls available.");
      } 

      /*
       * add a percent ('%') sign to VAT percentage, this should always be
       * possible as moa->v_5004 is 18 characters and 100.00% only 9 (inc. '\0')
       */
      if (tax)
      {
	ASSERT (tax->v_5278 != NULL);
	i = strlen (tax->v_5278);
	if ((i > 0) &&			/* is there anything to add '%' to ?? */
	    ((i + 2) < sizeof (tax->v_5278)))
	{
	  tax->v_5278[i++] = '%';
	  tax->v_5278[i]   = '\0';
	}
      }

      vdAddElement (pLay, LAY_ROA_ITEMS, 10,
		    COND_FETCH (imd, v_7008a),
		    COND_FETCH (tax, v_5278),
		    COND_FETCH (moa_940, v_5004),
		    COND_FETCH (moa_940, v_6345),
		    COND_FETCH (moa_941, v_5004),
		    COND_FETCH (moa_941, v_6345),
		    COND_FETCH (moa_943, v_5004),
		    COND_FETCH (moa_943, v_6345),
		    COND_FETCH (moa_942, v_5004),
		    COND_FETCH (moa_942, v_6345));
    }
    g_22 = g_22->g_22_next;
  }

  if (fRoamL2 == FALSE)
  {
    /* don't complain, because this is a common case */
    /*
     * macErrorMessage (PREP_NO_ROAMING_INFO, WARNING,
     *	 	        "ROAMING: No roaming information at all");
     */
    /*
    ** Defect 23751: B. Michler, 24.02.1997
    ** Return 0 (NO_ERROR) to prevent error post processing
    ** for a common case.
    ** old:    iRet = (int) PREP_NO_ROAMING_INFO;
    */
    iRet = 0;
  }
  else
  {
    /* print sender address       */
    fovdPrintSender (timm_message, pLay);

    /* print the receiver address */
    fovdPrintRec (timm_message, pLay);

    /* print additional information */
    fovdPrintCInfo (timm_message, ROA_TYPE, pLay);

    vdAddElement (pLay, LAY_FIX_HEADER, 1,
		  FIXTEXT (FIX_ROA_TITLE));   /* print title    */
    vdAddElement (pLay, LAY_FIX_TELECOM, 1,
		  FIXTEXT (FIX_TELECOM));

    fovdPrintRoaTail (timm_message, pLay);
  }

  fovdPopFunctionName();

  return (iRet);
}
/*---------------------------------------------------------------------------*/


/******************************************************************************
 * fovdPrintBalTail
 *
 * DESCRIPTION:
 * print footer for balance page
 * 
 *
 * PARAMETERS:
 *  stTMSG	*timm_message		- TIMM-structure
 *  stLAYINF	*pLay			- layout information
 *
 * RETURNS:
 *  - void -
 ******************************************************************************
 */
static void fovdPrintBalTail (stTMSG *timm_message,
			      stLAYINF *pLay)
{
   struct s_group_45 *g_45;
   struct s_moa_seg  *moa;


   vdAddElement (pLay, LAY_BAL_TR_HEADER, 2,
		 FIXTEXT (FIX_ACTION),
		 FIXTEXT (FIX_AMOUNT));

   /* get pointer moa_961                                        */
   /* (previous balance amount from end of last BCH run)          */
   g_45 = timm_message->g_45;
   while (g_45)
   {
     ASSERT (g_45->moa != NULL);

     if (strcmp (g_45->moa->v_5025, "961") == 0)
       break;
     g_45 = g_45->g_45_next;
   }
   
   if (g_45)
   {
      moa = g_45->moa;

      vdAddElement (pLay, LAY_BAL_TR_ELEMENTS, 3,
		    FIXTEXT (FIX_MOA_961),
		    COND_FETCH (moa, v_5004),
		    COND_FETCH (moa, v_6345));

   }
   else
   {
      macErrorMessage (PREP_BALANCE_ERROR, WARNING,
                       "BALANCE: No previous balance amount available." );
   }

   /* get pointer moa_962                                        */
   /* (sum of received payments)                                 */
   g_45 = timm_message->g_45;
   while (g_45)
   {
     ASSERT (g_45->moa != NULL);

     if (strcmp (g_45->moa->v_5025, "962") == 0)
       break;
     g_45 = g_45->g_45_next;
   }

   if (g_45)
   {
      moa = g_45->moa;

      vdAddElement (pLay, LAY_BAL_TR_ELEMENTS, 3,
		    FIXTEXT (FIX_MOA_962),
		    COND_FETCH (moa, v_5004),
		    COND_FETCH (moa, v_6345));
   }
   else
   {
      macErrorMessage (PREP_BALANCE_ERROR, WARNING,
                       "BALANCE: No sum of received payments available." );
   }

   /* get pointer moa_963                                        */
   /* (sum of write offs)                                        */
   g_45 = timm_message->g_45;
   while (g_45)
   {
     ASSERT (g_45->moa != NULL);

     if (strcmp (g_45->moa->v_5025, "963") == 0)
       break;
     g_45 = g_45->g_45_next;
   }

   if (g_45)
   {
      moa = g_45->moa;

      if (atof (moa->v_5004) != 0.0)
      {
	vdAddElement (pLay, LAY_BAL_TR_ELEMENTS, 3,
		      FIXTEXT (FIX_MOA_963),
		      COND_FETCH (moa, v_5004),
		      COND_FETCH (moa, v_6345));
      }
   }
   else
   {
      macErrorMessage (PREP_BALANCE_ERROR, WARNING,
                       "BALANCE: No sum of write offs available." );
   }

   /* get pointer moa_964                                        */
   /* (sum of other invoices)                                    */
   g_45 = timm_message->g_45;
   while (g_45)
   {
     ASSERT (g_45->moa != NULL);

     if (strcmp (g_45->moa->v_5025, "964") == 0)
       break;
     g_45 = g_45->g_45_next;
   }

   if (g_45)
   {
      moa = g_45->moa;

      if (atof (moa->v_5004) != 0.0)
      {
	vdAddElement (pLay, LAY_BAL_TR_ELEMENTS, 3,
		      FIXTEXT (FIX_MOA_964),
		      COND_FETCH (moa, v_5004),
		      COND_FETCH (moa, v_6345));
      }
   }
   else
   {
      macErrorMessage (PREP_BALANCE_ERROR, WARNING,
                       "BALANCE: No sum of other invoices available." );
   }

   /* get pointer moa_965                                        */
   /* (sum of adjustments)                                       */
   g_45 = timm_message->g_45;
   while (g_45)
   {
     ASSERT (g_45->moa != NULL);

     if (strcmp (g_45->moa->v_5025, "965") == 0)
       break;
     g_45 = g_45->g_45_next;
   }

   if (g_45)
   {
      moa = g_45->moa;

      if (atof (moa->v_5004) != 0.0)
      {
	vdAddElement (pLay, LAY_BAL_TR_ELEMENTS, 3,
		      FIXTEXT (FIX_MOA_965),
		      COND_FETCH (moa, v_5004),
		      COND_FETCH (moa, v_6345));
      }
   }
   else
   {
      macErrorMessage (PREP_BALANCE_ERROR, WARNING,
                       "BALANCE: No sum of adjustments available.");
   }

   /* get pointer moa_967                                        */
   /* (current total invoice amount */
   g_45 = timm_message->g_45;
   while (g_45)
   {
     ASSERT (g_45->moa != NULL);

     if (strcmp (g_45->moa->v_5025, "967") == 0)
       break;
     g_45 = g_45->g_45_next;
   }

   if (g_45)
   {
      moa = g_45->moa;

      vdAddElement (pLay, LAY_BAL_TR_ELEMENTS, 3,
		    FIXTEXT (FIX_MOA_967),
		    COND_FETCH (moa, v_5004),
		    COND_FETCH (moa, v_6345));
   }
   else
   {
      macErrorMessage (PREP_BALANCE_ERROR, WARNING,
                       "BALANCE: No current total invoice amount from BCH available." );
   }

   /* get pointer moa_968                                        */
   /* (current balance amount)			                 */
   g_45 = timm_message->g_45;
   while (g_45)
   {
     ASSERT (g_45->moa != NULL);

     if (strcmp (g_45->moa->v_5025, "968") == 0)
       break;
     g_45 = g_45->g_45_next;
   }

   if (g_45)
   {
      moa = g_45->moa;

      vdAddElement (pLay, LAY_BAL_TR_ELEMENTS, 3,
		    FIXTEXT (FIX_MOA_968),
		    COND_FETCH (moa, v_5004),
		    COND_FETCH (moa, v_6345));
   }
   else
   {
      macErrorMessage (PREP_BALANCE_ERROR, WARNING,
                       "BALANCE: No current balance of customers account available." );
   }
}


/******************************************************************************
 * fovdPrintBal
 *
 * DESCRIPTION:
 * print balance part
 * 
 *
 * PARAMETERS:
 *  stTMSG	*timm_message		- TIMM-structure
 *  stLAYINF	*pLay			- layout information
 *
 * RETURNS:
 *  - void -
 ******************************************************************************
 */
static void fovdPrintBal (stTMSG *timm_message, stLAYINF *pLay)
{
  struct s_group_22 *g_22;
  struct s_group_23 *g_23;
  struct s_group_26 *g_26;
  struct s_dtm_seg  *dtm;
  struct s_imd_seg  *imd;
  struct s_moa_seg  *moa_960;
  struct s_rff_seg  *rff;
  int		    iText;	/* text index for IMD */
  char		    *pszText;	/* pointer to text for IMD */
  char 		    *pszOInv;	/* pointer to header text for other inv. */
  BOOL		    fPrintHead;	/* print list header */

  fovdPushFunctionName ("fovdPrintBal");

  /* initialisations */
  fPrintHead = FALSE;
  pszOInv = BLANK_CHAR;		/* no other invoices */
  moa_960 = NULL;

  /* print sender address       */
  fovdPrintSender (timm_message, pLay);

  /* print the receiver address */
  fovdPrintRec (timm_message, pLay);

  /* print additional information */
  fovdPrintCInfo (timm_message, BAL_TYPE, pLay);

  if (! fofIsPrinted (pLay, LAY_FIX_HEADER))
  {
    vdAddElement (pLay, LAY_FIX_HEADER, 1,
		  FIXTEXT (FIX_BAL_TITLE));   /* print title    */
    vdAddElement (pLay, LAY_FIX_TELECOM, 1,
		  FIXTEXT (FIX_TELECOM));
  }

  /* print tail */
  fovdPrintBalTail (timm_message, pLay);

  /* print lin body data elements */
  g_22 = timm_message->g_22;

  while (g_22)
  {
    ASSERT (g_22->lin != NULL);

    /* get type of transaction from IMD */
    imd = g_22->imd;

    pszText = COND_FETCH (imd, v_7009);

    /*
     * try to get long-text from table, if it is not possible
     * then show the original text
     */
    if (imd && fofGetStr (&iText, imd->v_7009, stBalItems))
    {
      pszText = FIXTEXT (iText);
    }

    /* get date of transaction */
    dtm = g_22->dtm;

    while (dtm)
    {
      if (strcmp (dtm->v_2005, "900") == 0)
	break;
      dtm = dtm->dtm_next;
    }
    if (dtm == NULL)
    {
        macErrorMessage (PREP_BALANCE_ERROR, WARNING,
                         "BALANCE: No date for transaction available.");
    }
    strcpy (dtm->v_2380, form_date (dtm->v_2380));

    /* get pointer moa_960 */

    g_23 = g_22->g_23;
    while (g_23)
    {
      ASSERT (g_23->moa != NULL);

      if (strcmp (g_23->moa->v_5025, "960") == 0)
	break;
      g_23 = g_23->g_23_next;
    }

    if (g_23)
    {
      moa_960 = g_23->moa;
    }
    else
    {
      macErrorMessage (PREP_BALANCE_ERROR, WARNING,
		       "BALANCE: No balance amount available.");
    }

    /* get rff for other invoice */
    g_26 = g_22->g_26;
    while (g_26)
    {
      ASSERT (g_26->rff != NULL);

      if (strcmp (g_26->rff->v_1153, "RF") == 0)
	break;
      g_26 = g_26->g_26_next;
    }
    if (g_26)
    {
      rff = g_26->rff;

      /* set the header text for other invoices */
      pszOInv = FIXTEXT (FIX_REFNUM);
    }
    else
    {
      /* set rff to NULL, macro COND_FETCH will the print a blank text */
      rff = NULL;
    }

    vdAddElement (pLay, LAY_BAL_IT_ELEMENTS, 5,
		  pszText,
		  COND_FETCH (rff, v_1154),
		  COND_FETCH (dtm, v_2380),
		  COND_FETCH (moa_960, v_5004),
		  COND_FETCH (moa_960, v_6345));
    fPrintHead = TRUE;

    g_22 = g_22->g_22_next;
  } /* while (g22)...*/

  if (fPrintHead) {
    /* print header of list */
    vdAddElement (pLay, LAY_BAL_IT_HEADER, 4,
		  FIXTEXT (FIX_TRANSACTION),
		  pszOInv,
		  FIXTEXT (FIX_DATE),
		  FIXTEXT (FIX_AMOUNT));
  }
  fovdPopFunctionName();
}

/*---------------------------------------------------------------------------*/



/******************************************************************************
 * fovdPrintLeg
 *
 * DESCRIPTION:
 * print legend part (normally to an itemized bill)
 *
 *
 * PARAMETERS:
 *  stTMSG	*timm_message		- TIMM-structure
 *  stLAYINF	*pLay			- layout information
 *
 * RETURNS:
 *  - void -
 ******************************************************************************
 */
static void fovdPrintLeg (stTMSG *timm_message, stLAYINF *pLay)
{
  struct s_group_22 *g_22;
  struct s_imd_seg  *imd;
  int		    iText;	/* text index for IMD */
  char		    *pszText;	/* pointer to text for IMD */
  char 		    szId[sizeof (imd->v_7009)];	/* pointer to current service code ID */
  BOOL		    fSkip;	/* marker for skipped IMDs */
  BOOL		    fFirst;	/* first element */

  fovdPushFunctionName ("fovdPrintLeg");

  /* make sure that imd->v_7009 is an array and not just a pointer */
#ifndef OSF1
  /* the DEC ALPHA has 64 bit pointer, szId is 8 byte long ... */
  ASSERT (sizeof (szId) > sizeof (char *));
#endif

  /* initialize */
  fFirst = TRUE;

#if 0
  if (fofIsPrinted (pLay, LAY_LEG_TITLE))
  {
    /* already (or not at all) printed in this document */
    fovdPopFunctionName();
    return;
  }
#endif

  /*
   * check if the product identifier is always '8' as specified,
   * because it will be reset to that value at the end of the procedure
   */
  g_22 = timm_message->g_22;

  while (g_22)
  {
    imd = g_22->imd;

    if (imd != NULL) {
      if (imd->v_7081[0] != '8') {
	macErrorMessage (PREP_IMD_NO_SERVICE_DESCR, WARNING,
			 "PrintLegend: Wrong product identifier in IMD!");
      }
    }
    g_22 = g_22->g_22_next;		/* proceed to next element */
  }

  /*
   * 'do' loop until g22 is equal to timm_message->g_22
   * at the end of the loop. This is in effect when
   * every IMDs service code was deleted; they
   * are deleted when the IMD gets printed
   */
  do {
    fSkip = FALSE;

    g_22 = timm_message->g_22;

    szId[0] = '\0';

    while (g_22)
    {
      ASSERT (g_22->lin != NULL);

      /* get type of transaction from IMD */
      imd = g_22->imd;

      if (imd)
      {
	/* was it already processed ?? */
	if (imd->v_7081[0] != '\0')
	{
	  /*
	   * set the service code id to the id of the current
	   * LIN if it is not set, otherwise take only LINs
	   * with the same id
	   */
	  if (szId[0] == '\0')
	  {
	    /* break the list if it is not he first part */
	    if (fFirst)
	    {
	      /* print the header only if there is something in Legend */
	      vdAddElement (pLay, LAY_LEG_TITLE, 1, FIXTEXT (LEG_TITLE));
	    }
	    else
	    {
	      /*	      vdBreakList (pLay, 2, LAY_LEG_HINT, LAY_LEG_INFO);*/
	    }
	    fFirst = FALSE;

	    strcpy (szId, imd->v_7009);	/* remember string */

	    /*
	     * this must have been the first id of that type,
	     * so print the header now
	     */
	    pszText = COND_FETCH (imd, v_7009);

	    /*
	     * try to get long-text from table, if it is not possible
	     * then show the original text
	     */
	    if (imd && fofGetStr (&iText, imd->v_7009, stLegItems))
            {
	      pszText = FIXTEXT (iText);
	    }
	    /*	    vdAddElement (pLay, LAY_LEG_HINT, 1, pszText);*/
	  }

	  if (strcmp (szId, imd->v_7009) == 0)
	  {
	    /*
	     * modify identifier of IMD so that it wont be
	     * printed again
	     */
	    imd->v_7081[0] = '\0';
	    /* now imd points to a valid one that should be printed */
	    vdAddElement (pLay, LAY_LEG_INFO, 3,
			  pszText,
			  imd->v_7008,
			  imd->v_7008a);
	    pszText = BLANK_CHAR; /* print only once */
	  }
	  else
	  {
	    /*
	     * this element was not printed up to now and it is
	     * not the type we are actually printing,
	     * ->remember that we skipped it
	     */
	    fSkip = TRUE;
	  }
	} /* if (imd->v_7009[0]..)..*/
      } /* if (imd) ...*/

      g_22 = g_22->g_22_next;		/* proceed to next element */
    } /* while (g_22)...*/
  } while (fSkip);

  /*
   * at the end reset imd->7081 to its inital value of '8'
   */
  g_22 = timm_message->g_22;

  while (g_22)
  {
    imd = g_22->imd;

    if (imd) {
      imd->v_7081[0] = '8';
    }

    g_22 = g_22->g_22_next;		/* proceed to next element */
  }

  fovdPopFunctionName();
}

/*---------------------------------------------------------------------------*/

/******************************************************************************
 * fovdPrintAdv
 *
 * DESCRIPTION:
 * print advertisements
 *
 *
 * PARAMETERS:
 *  stTMSG	*timm			- TIMM-structure
 *  stLAYINF	*pLay			- layout information
 *
 * RETURNS:
 *  - void -
 ******************************************************************************
 */
#define FTXMAXLEN	80

static void fovdPrintAdv (stTMSG *timm, stLAYINF *pLay)
{
  struct s_ftx_seg  *ftx;
  int		    iAdvCnt;	/* counter for adv ftx-segments */
  long		    lAdvLen;	/* length counter for adv */
  char		    *pachAdv;	/* pointer to memory for adv */
  char		    *pTemp;	/* temporary pointer into adv */
  char		    *pBlank;	/* last blank found */
  char		    *pLast;	/* behind last printing position */

  fovdPushFunctionName ("fovdPrintAdv");

  if (! fofLineExists (pLay, LAY_ADV_INFO))
  {
    /* no line of that type */
    fovdPopFunctionName();
    return;
  }

  /* first count all ftx segments and calculate needed space */
  ftx = timm->ftx;
  iAdvCnt = 0;
  lAdvLen = 0;
  pLast   = NULL;
  pTemp   = NULL;

  while (ftx)
  {
    /* just in case someone changes the parser... */
    ASSERT (ftx->v_4451  != NULL);
    ASSERT (ftx->v_4440  != NULL);
    ASSERT (ftx->v_4440a != NULL);
    ASSERT (ftx->v_4440b != NULL);
    ASSERT (ftx->v_4440c != NULL);
    ASSERT (ftx->v_4440d != NULL);

    if (strcmp (ftx->v_4451, "ADV") == 0)
    {
      lAdvLen += strlen (ftx->v_4440);
      lAdvLen += strlen (ftx->v_4440a);
      lAdvLen += strlen (ftx->v_4440b);
      lAdvLen += strlen (ftx->v_4440c);
      lAdvLen += strlen (ftx->v_4440d);
    }
    ftx = ftx->ftx_next;
  }

  if (lAdvLen > 0)
  {
    /*
     * there IS some advertisement... get space for it
     * (if memory allocation fails, fpvdGetMemory will take care
     * of it and terminate BGH)
     */
    pachAdv = (char *) fpvdGetPrepMemory (lAdvLen);
    pTemp = pachAdv;

    /*
     * now copy all advertisement to pachAdv
     */
    ftx = timm->ftx;

    while (ftx)
    {
      if (strcmp (ftx->v_4451, "ADV") == 0)
      {
	strcpy (pTemp, ftx->v_4440);
	pTemp += strlen (pTemp);
	strcpy (pTemp, ftx->v_4440a);
	pTemp += strlen (pTemp);
	strcpy (pTemp, ftx->v_4440b);
	pTemp += strlen (pTemp);
	strcpy (pTemp, ftx->v_4440c);
	pTemp += strlen (pTemp);
	strcpy (pTemp, ftx->v_4440d);
	pTemp += strlen (pTemp);
      }
      ftx = ftx->ftx_next;
    }

    /*
     * now all text is copied to pachAdv,
     * break it up in FTXMAXLEN segments and add them to layout,
     * replacing all '\n' with ' '
     */
    pTemp = pachAdv;
    pLast = pTemp;
    lAdvLen = 0;
    pBlank = NULL;

    while (*pTemp)
    {
      /*
       * force a new line with '\n'
       */
      if (*pTemp == '\n')
      {
	pBlank = pTemp;
	lAdvLen = FTXMAXLEN;
      }

      /* remember blank positions for a break */
      if (*pTemp == ' ')	pBlank = pTemp;

      if (lAdvLen >= FTXMAXLEN)
      {
	/* we reached the maximum length */
	if (pBlank != NULL)
	{
	  pTemp = pBlank;
	  *pTemp = '\0';
	  lAdvLen = -1;		/* it gets increased soon afterwards */

	  pBlank = NULL;

	  /* really add the line */
	  vdAddElement (pLay, LAY_ADV_INFO, 1, pLast);

	  pLast = pTemp + 1;
	}
      }
      /* next character */
      lAdvLen++;
      pTemp++;
    }
  }
  /*
   * add the rest of the string if it wasn't already done
   */
  if (pLast != pTemp)
  {
    vdAddElement (pLay, LAY_ADV_INFO, 1, pLast);
  }

  fovdPopFunctionName();
}



/******************************************************************************
 * loiPrintXcdElem3xx
 *
 * DESCRIPTION:
 * print a XCD of TIMM version 3xx
 * 
 *
 * PARAMETERS:
 *  struct s_xcd_seg *pXcd		- XCD element
 *  int iTable				- printing table
 *  stLAYINF	*pLay			- layout information
 *
 * RETURNS:
 *  int
 ******************************************************************************
 */
static short loiPrintXcdElem3xx (struct s_xcd_seg *pXcd,
				 int iTable,
				 stLAYINF *pLay)
{
  short		losReturn;
  char          *lpchCRTimeStamp;
  char   	*lpchDurationBuf2 = NULL;
#if CALLRATINGTYPE
  char 	     	*pszTmp;
#endif

  losReturn = 0;

  /* format the timestamp of an XCD segment         */

  lpchCRTimeStamp = foszFormTime (pXcd->v_X005);
  if (lpchCRTimeStamp != NULL) {

    if (iTable != LAY_ITB_ITEMS_TABLE_6) {	/* != additional services */

      /* format the volume and measure unit qualifier      */

      if (0 == strcmp (pXcd->v_6411, X6411_UNIT_SEC)) {
	/* build buffer with volume (duration) */
	lpchDurationBuf2 = foszFormDur (pXcd->v_X008);

	if (lpchDurationBuf2 == NULL) {
	  return (-1);
	}
      } else {
	/* build buffer with volume (duration) */
	lpchDurationBuf2 = fpvdGetPrepMemory (32);
	sprintf (lpchDurationBuf2, "%s [%s]",
		 pXcd ->v_X008, pXcd->v_6411);
      }
    }

#if CALLRATINGTYPE
    /* format late call indicator and rate indicator            */

    if (0 == strcmp (pXcd->v_X020, X012_NORMAL_CALL)) {
      pXcd->v_X020[0] = '\0';
    }

    if (0 == strcmp (pXcd->v_X020, X012_LATE_CALL) &&
	0 == strcmp (pXcd->v_X021, X013_NORMAL_CALL)) {
      pXcd->v_X021[0] = '\0';
    }

    pszTmp = fpvdGetPrepMemory (sizeof (pXcd->v_X020) +
				sizeof (pXcd->v_X021) + 1);
    pszTmp[0] = '\0';

    if (pXcd->v_X020[0] != '\0')
      strcpy (pszTmp, pXcd->v_X020);
    if (pXcd->v_X021[0] != '\0')
      strcat (pszTmp, pXcd->v_X021);
    if (pszTmp[0] == '\0')
      strcpy (pszTmp, " ");

    /*
     * now pszTmp may be:  F  free rating
     *			   P  partially free
     *			   N  normal
     * 			   L  late call
     *   		   LF late call, free
     * 			   LP late call, partially free
     */
#endif

    switch (iTable) {
    case LAY_ITB_ITEMS_TABLE_3:		/* --- print GSM-outbound visitor --- */
      vdAddElement (pLay, LAY_ITB_ITEMS_TABLE_3,
#if CALLRATINGTYPE
		    8,
#else
		    7,
#endif
		    lpchCRTimeStamp,
		    lpchDurationBuf2,
		    COND_FETCH (pXcd, v_X043),
		    COND_FETCH (pXcd, v_X029),
		    COND_FETCH (pXcd, v_X013),
#if CALLRATINGTYPE
		    pszTmp,
#endif
		    COND_FETCH (pXcd, v_5004),
		    COND_FETCH (pXcd, v_6345));
      break;

    case LAY_ITB_ITEMS_TABLE_4:		/* --- print GSM-inbound home --- */
      /* FALLTHROUGH */
    case LAY_ITB_ITEMS_TABLE_5:		/* --- print GSM-inbound visitor --- */
      vdAddElement (pLay, iTable,
#if CALLRATINGTYPE
		    8,
#else
		    7,
#endif
		    lpchCRTimeStamp,
		    lpchDurationBuf2,
		    COND_FETCH (pXcd, v_X047),
		    COND_FETCH (pXcd, v_X029),
		    COND_FETCH (pXcd, v_X013),
#if CALLRATINGTYPE
		    pszTmp,
#endif
		    COND_FETCH (pXcd, v_5004),
		    COND_FETCH (pXcd, v_6345));
      break;

    case LAY_ITB_ITEMS_TABLE_6:		/* --- print additional services --- */
      vdAddElement (pLay, LAY_ITB_ITEMS_TABLE_6,
#if CALLRATINGTYPE
		    6,
#else
		    5,
#endif
		    lpchCRTimeStamp,
		    COND_FETCH (pXcd, v_X048),
		    COND_FETCH (pXcd, v_X013),
#if CALLRATINGTYPE
		    pszTmp,
#endif
		    COND_FETCH (pXcd, v_5004),
		    COND_FETCH (pXcd, v_6345));
      break;

    default:
      /* --- print national GSM-outbound home --- */
      /* --- print international GSM-outbound home --- */
      vdAddElement (pLay, iTable,
#if CALLRATINGTYPE
		    9,
#else
		    8,
#endif
		    lpchCRTimeStamp,
		    lpchDurationBuf2,
		    COND_FETCH (pXcd, v_X043),
		    COND_FETCH (pXcd, v_X015),
		    COND_FETCH (pXcd, v_X014),
		    COND_FETCH (pXcd, v_X013),
#if CALLRATINGTYPE
		    pszTmp,
#endif
		    COND_FETCH (pXcd, v_5004),
		    COND_FETCH (pXcd, v_6345));
      break;
    }
  } else {
    losReturn = -1;
  }

  if (losReturn == 0) {
     fovdPrintXcdElHead (iTable, pLay);
  }
  return (losReturn);
}


/******************************************************************************
 * foiPrintXcdLine3xx
 *
 * DESCRIPTION:
 * print a single XCD line
 * The following groups are distinguished:
 * 1. national GSM outbound home:
 *                   XCD.X029 = 'O'         - outbound
 *                   XCD.X022 = 'H'         - home
 *                   XCD.X038 = 'NV' || 'NP - national
 * 2. international GSM outbound home:
 *                   XCD.X029 = 'O'         - outbound
 *                   XCD.X022 = 'H'         - home
 *                   XCD.X038 = 'IV' || 'IP - international
 * 3. GSM outbound visitor:
 *                   XCD.X029 = 'O'         - outbound
 *                   XCD.X022 = 'V'         - visitor
 * 4. GSM inbound home:
 *                   XCD.X029 = 'I'         - inbound
 *                   XCD.X022 = 'H'         - home
 * 5. GSM inbound visitor:
 *                   XCD.X029 = 'I'         - inbound
 *                   XCD.X022 = 'V'         - visitor
 * 6. additional services:
 *                   XCD.X048 exists
 *
 * PARAMETERS:
 *  float	flThreshold		- threshold
 *  struct s_xcd_seg *spstXcd		- pointer to XCD
 *  stTMSG	*timm			- TIMM-structure
 *  stLAYINF	*pLay			- layout information
 *
 * RETURNS:
 *   0     -  Success
 *   != 0  -  Error
 ******************************************************************************
 */
static int foiPrintXcdLine3xx (double flThreshold, struct s_xcd_seg *spstXcd,
			       stTMSG *timm, stLAYINF *pLay)
{
   int               loiRc;             /* return code                  */
   short             lofOutbound;	/* TRUE : outbound call         */
					/* FALSE: inbound call          */
   short             lofVisitor;	/* TRUE : VPLMN network         */
					/* FALSE: HPLMN network         */  
   double	flSurcha;

   loiRc = 0;


   flSurcha = atof (spstXcd->v_5004);

   /*
    * compare the sum with the threshold
    */
   if ((flSurcha < flThreshold) ||
       (flSurcha == 0.0))
   {
     /*  XCD total amount < threshold amount->drop XCD         */
     /*  XCD total amount == 0.0            ->drop XCD         */
#if 0
     fovdPrintLog (LOG_CUSTOMER,
		   "XCD amount: %f; threshold: %f -> drop XCD\n",
		   flSurcha, flThreshold);
#endif
#if BELOWXCDTHRESHOLD
     return (loiRc);
#endif
   }

   /*
    * The BCH inserts '_' in test mode.
    * We have to delete this sign in an VAS remark, because we
    * recognize a VAS if a remark exists!
    */
   if ((spstXcd->v_X048[0] == '_')	&&
       (spstXcd->v_X048[1] == '\0')) {
     spstXcd->v_X048[0] = '\0';
   }

   if (0 == strcmp(spstXcd->v_X026, X017_OUTBOUND)) {
     lofOutbound = TRUE;
   } else {
     lofOutbound = FALSE;
   }

   if (0 == strcmp(spstXcd->v_X028, X018_VISITOR)) {
     lofVisitor = TRUE;
   } else {
     lofVisitor = FALSE;
   }

   if (spstXcd->v_X044[0] != '\0' && lofOutbound == TRUE) {
     /*    outbound call                                         */
     if (lofVisitor == TRUE) {
       /*    outbound calls from other GSM networks              */

       /* --- print GSM-outbound visitor --- */
       loiPrintXcdElem3xx (spstXcd, LAY_ITB_ITEMS_TABLE_3, pLay);
     } else {
       /*    outbound calls from HPLMN                             */
       if (0 == strcmp (spstXcd->v_X044, X021_NAT_PLMN) ||
	   0 == strcmp (spstXcd->v_X044, X021_NAT_PSTN)) {

	 /*    outbound national calls from HPLMN                  */

	 /* --- print national GSM-outbound home --- */
	 loiPrintXcdElem3xx (spstXcd, LAY_ITB_ITEMS_TABLE_1, pLay);

       } else if (0 == strcmp (spstXcd->v_X044, X021_INT_PLMN) ||
		  0 == strcmp (spstXcd->v_X044, X021_INT_PSTN)) {

	 /*    outbound international calls from HPLMN              */

	 /* --- print international GSM-outbound home --- */
	 loiPrintXcdElem3xx (spstXcd, LAY_ITB_ITEMS_TABLE_2, pLay);
       } else {

	 /*   undefined call destination                            */
	 sprintf (laszErrMsg, "ITB: xcd %s: undefined call destination",
		  spstXcd->v_X001);
	 macErrorMessage (PREP_XCD_UNDEF_CALL_DEST ,
			  WARNING, laszErrMsg );
	 loiRc = (int) PREP_XCD_UNDEF_CALL_DEST;
       }
     }
   } else if(spstXcd->v_X048[0] == '\0' && lofOutbound == FALSE) {
     if (lofVisitor == TRUE) {

       /*    inbound calls in other GSM networks                   */

       /* --- print GSM-inbound visitor --- */
       loiPrintXcdElem3xx (spstXcd, LAY_ITB_ITEMS_TABLE_5, pLay);

     } else {
       /*    inbound calls in HPLMN                                */

       /* --- print GSM-inbound home --- */
       loiPrintXcdElem3xx (spstXcd, LAY_ITB_ITEMS_TABLE_4, pLay);
     }
   } else if (spstXcd->v_X048[0] != '\0') {

     /* --- print additional services --- */
     loiPrintXcdElem3xx (spstXcd, LAY_ITB_ITEMS_TABLE_6, pLay);
   } else {
     macErrorMessage (PREP_XCD_UNDEF_CALL_TYPE, CRITICAL,
		      "ITB: xcd: call type is undefined");
     loiRc = (int) PREP_XCD_UNDEF_CALL_TYPE;
   }

   return (loiRc);
}


/******************************************************************************
 * foiPrintXcd3xx
 *
 * DESCRIPTION:
 * print the XCD segments for TIMM version 3xx
 *
 * PARAMETERS:
 *  struct s_group_22 *spstG22		- pointer to group 22
 *  stTMSG	*timm			- TIMM-structure
 *  stLAYINF	*pLay			- layout information
 *
 * RETURNS:
 *   0     -  Success
 *   != 0  -  Error
 ******************************************************************************
 */
static int foiPrintXcd3xx (struct s_group_22 *spstG22, stTMSG *timm, stLAYINF *pLay)
{

   int               loiRc;             /* return code                  */
   struct s_xcd_seg  *spstTmp;		/* temporary pointer to list    */

   struct s_group_22 *spstG22Tmp; 	/* pointer to sorted group 22 list */
   struct s_group_99 *spstG99;		/* pointer to actual group 99       */
#if CONDENSESPLITXCDS
   struct s_group_99 *spstSG99;		/* pointer to actual group 99 with sub XCD */
   struct s_xcd_seg  *spstSub;		/* temporary pointer to sub XCD */
#endif
   struct s_group_23 *spstG23;

   char		szSum[32];		/* sum of values 		*/
   long		lSum;
#if CONDENSESPLITXCDS
   long		     lClicks1, lClicks2;
#endif

   double	flThreshold;		/* values for calculating... */

   fovdPushFunctionName ("fovdPrintXcd3xx");

   /* initialize variables                                     */
   loiRc       = 0;

   /* something for FLINT ... */
   flThreshold = 0.0;
   spstG99 = NULL;

   /*
    * Check if the related layout elements exist,
    * stop right now if they do not exist.
    */
   if (! (fofLineExists (pLay, LAY_ITB_ITEMS_TABLE_1)	||
	  fofLineExists (pLay, LAY_ITB_ITEMS_TABLE_2)	||
	  fofLineExists (pLay, LAY_ITB_ITEMS_TABLE_3)	||
	  fofLineExists (pLay, LAY_ITB_ITEMS_TABLE_4)	||
	  fofLineExists (pLay, LAY_ITB_ITEMS_TABLE_5)	||
	  fofLineExists (pLay, LAY_ITB_ITEMS_TABLE_6)))
   {
     /*
      * No layout element for printing XCDs
      */
     fovdPopFunctionName ();
     return (0);
   }

   if (spstG22->g_99 == NULL) {
     /*
      * No XCD's in this Group 22
      */
     return (PREP_NO_XCDS);
   }

   /* Reset flags for header printing */
   fHeader1 = FALSE;
   fHeader2 = FALSE;
   fHeader3 = FALSE;
   fHeader4 = FALSE;
   fHeader5 = FALSE;
   fHeader6 = FALSE;


   /* Reset all totals */

   spstG22Tmp 	= spstG22;
   spstG99 	= spstG22Tmp->g_99;
   spstTmp 	= spstG99->xcd;

   /* now search threshold for XCDs */
   if (spstTmp != NULL) {
     spstG23 = spstG22Tmp->g_23;

     while (spstG23 != NULL) {

       if (strcmp (spstG23->moa->v_5025, MOA_THRESHOLD_VALUE) == 0) {
	 /* remember the Threshold */
	 flThreshold = atof (spstG23->moa->v_5004);
	 break;
       }
       spstG23 = spstG23->g_23_next;
     }

     /* if spstG23 is NULL then we havent found a threshold */
     if (spstG23 == NULL) {
       /*  group 23 moa segment with threshold does not exist      */
       loiRc = (int) PREP_G23_MOA_NO_THRESHOLD;
       macErrorMessage (PREP_G23_MOA_NO_THRESHOLD , WARNING,
			"ITB: group 23 MOA seg with threshold does't exist");
       flThreshold = 0.0;
     }

     while (spstTmp != NULL && loiRc == 0) {
       /*
	* for each XCD: 
	* calculate the total sum and write it back to the XCD
	*/
       lSum = folStrAdd (szSum, 0, 3, spstTmp->v_5004,
			 spstTmp->v_5004a, spstTmp->v_5004b);
#if 0
       /*
	* reset the actual values (remember: they are strings)
	*/
       spstTmp->v_5004[0]  = '\0';
       spstTmp->v_5004a[0] = '\0';
       spstTmp->v_5004b[0] = '\0';
#endif
#if CONDENSESPLITXCDS
       /*
	* look for splitted calls,
	* add their amounts as well
	* (look if there is another XCD with the same running main number
	* and a running sub number that is not 0, collect all subsequent
	* elements that fulfil this criteria; afterwards the pointer spstG99
	* is set to the last of these elements, so that the outer loop
	* skips the sub XCDs)
	*/
       lClicks1 = atol (spstTmp->v_X008);

       spstSG99 = spstG99;
       while (spstSG99->g_99_next != NULL) {
	 /* there is another XCD ... */
	 spstSub = spstSG99->g_99_next->xcd;

	 /* look if it is a sub number of the current XCD */
	 if ((strcmp (spstTmp->v_X001, spstSub->v_X001) != 0) ||
	     (atoi (spstSub->v_X002) == 0)) {
	   /* skip the sub-XCDs */
	   /* spstG99 = spstSG99; */
	   break;
	 }
	 lSum = folStrAdd (szSum, lSum, 3, spstSub->v_5004,
			   spstSub->v_5004a, spstSub->v_5004b);

	 /*
	  * reset the actual values (remember: they are strings)
	  */
	 spstSub->v_5004[0]  = '\0';
	 spstSub->v_5004a[0] = '\0';
	 spstSub->v_5004b[0] = '\0';

	 /* Add clicks */
	 lClicks2 = atol (spstSub->v_X007);

	 if (lClicks2 >= 0) {
	   lClicks1 += lClicks2;
	 }

	 spstSG99 = spstSG99->g_99_next;
       }

       sprintf (spstTmp->v_X007, "%ld", lClicks1);

       /* adjust the pointer */
       spstG99 = spstSG99;
#endif
       strcpy (spstTmp->v_5004, szSum);

       loiRc = foiPrintXcdLine3xx (flThreshold, spstTmp, timm, pLay);

       /* proceed to next XCD */
       spstTmp = NULL;
       spstG99 = spstG99->g_99_next;
       if (spstG99 != NULL) {
	 spstTmp = spstG99->xcd;
       }
     }                       /* end of inner while loop    */
   } /* if (fFoundXcd).. */

   fovdPopFunctionName();

   return( loiRc );
}
/*---------------------------------------------------------------------------*/

