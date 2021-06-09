/*******************************************************************************
 * LH-Specification GmbH 1996.
 *
 * All rights reserved.
 * Copying of this software or parts of this software is a violation of German
 * and International laws and will be prosecuted.
 *
 * Project  :   BGH
 *
 * File     :   layout.c
 * Created  :   Mar. 1996
 * Author(s):   Alexander Dreier - Solit GmbH (Sub)
 *
 * Changed  :
 * 14.01.97	B.Michler	an element with ABSO (absolute positioned)
 *				is NO list!!
 * 09.09.96	B.Michler	regard graphics even after broken groups;
 *				do FEED even for not-printed elements
 * 27.08.96	B.Michler	Parameter 'black' for graphics sets line width
 *				for Rects and Lines
 * 22.08.96	B.Michler	allow offsets at graphic starting point
 * 11.07.96 	B.Michler  	insert "%%Copies: ", clean up, PSVERSION to 0.3
 * 04.07.96 	B.Michler  	take headline into account: do not print a single
 *				headline at the end of a page
 * 21.06.96 	B.Michler  	in 'PrintItem': remember the last element, if
 *				a pagend is reached within a list
 * 20.06.96 	B.Michler  	in 'PrintItem': do not apply an offset to an
 *				element that is not printed because of a broken
 *				list (actually remove an already applied offset)
 * 24.05.96 	B.Michler  	added flag FOPA
 *
 * DESCRIPTION:
 *
 * This layouter is supposed to offer a template and frame for future
 * layout needs for the BSCS System.
 * Output is realized in a human readable subset of postscript, that 
 * can easily be interpreted by some additional printerdriver if the 
 * target printer doesn't understand Postscript itself.
 * -  All coordinates in the generated output are absolute. 
 * -  You'll find a "showpage" befor the next page starts.
 * -  All commands fit on one line of the output (no stackmachine necessary)
 * -  Some features (so far : centertext,leftalign,rightalign,underline) are
 *    realized by Postscript subroutines which can and probably must 
 *    be ignored by an additional printerdriver. In the output the printer 
 *    driver will just find an (for example: ctext- for center text) with an 
 *    absolute position. The printerdriver must then find some probably
 *    printerdependant solution how to realise that, or even chose to ignore
 *    the commands (which will of course change the appearance of the layout).
 *    all this is only relevant if printer or viewer can't handle postscript.
 * -  Output in Postscript - directly view and printable with many programs
 *    and printers (for example the public domain ghostview).
 * -  You are offered abstract functionality to collect all the information 
 *    you want to appear on a document, before the printout (postscript) starts.
 * -  The whole layouter is controlled by two arrays : s[] and lay[]
 *
 * The real output in postscript is done by the routines 'PrintLine',
 * 'PrintGraphic' and 'fovdPrintPSText', so if the language should be
 * changed (e. g. to HP-PCL) it only affects these routines.
 *
 *******************************************************************************/

#if 0	/* just for version.sh */
static char *SCCS_VERSION = "1.63";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>             /* for variable arguments */

/* User includes                */
#include "bgh.h"
#include "protos.h"
#include "layout.h"



/* User defines                 */
#define PSVERSION	"0.3"	/* version of the postscript metacode */

#ifdef LAYOUT_OLDLIST
#define ISLIST(a)	(!((a)->flags & (PAGE | LAPA | ABSO)))
#else
#define ISLIST(a)	(!((a)->flags & (PAGE | LAPA | EVPA | ABSO)))
#endif

#define PILISTBROKEN	-1
#define PIPAGEEND	-2
#define PIGROUP		-3

/* Function macros              */


/* Enumerations                 */


/* Local typedefs               */


/* External globals             */
extern stBGHGLOB stBgh;		/* globals for bgh */


/* Globals                      */
char gszPageString[32] = "";            /* string to hold current page number */
static int giLastListElem;		/* the last processed list element (field index) */
static int	iNewPageEl = -1; 	/* element at which page break occured */


/* These are macrodefinitions needed by postscript to handle :       
 * centering,left or right alignment,and underlining of textitems  
 * (subroutines written in postscript themself)                      
 * This can (and probably must) be ignored by a printerdriver for   
 * a no postscript printer - it must find its own way to interpret   
 * ltext,rtext,ctext (this is nevertheless copied into the outputfile)
 * maybe clear the parts behind % since this is copied into every output
 * if the file metahead.ps isn't found. (contents is the same so far).
 */


static char * meta[] = { 
  "% Macros",
  "",
  "%  !!! By Default insert  !!!",
  "%underlining text",
  "/UnderLineString {",
  "    currentfont dup             % get current font dictionary",
  "    /FontMatrix get             % get current font matrix",
  "    exch                        % font dictionary on top",
  "    /FontInfo get dup",
  "    /UnderlinePosition get",
  "    exch                        % font info dictionary on top",
  "    /UnderlineThickness get     % get stroke width",
  "    3 -1 roll dtransform        % scale position and thickness",
  "    /UnderThick exch def",
  "    /UnderPos exch def",
  "    currentpoint                % get current point",
  "    pop",
  "    /StartX exch def",
  "    show                        % show string",
  "    currentpoint",
  "    /EndY exch def",
  "    /EndX exch def",
  "    0 UnderPos rmoveto          % set start of line",
  "    StartX EndX sub 0 rlineto",
  "    currentlinewidth            % remember current linewidth",
  "    UnderThick setlinewidth",
  "    stroke                      % stroke the line",
  "    setlinewidth                % restore linewidth",
  "    EndX EndY moveto",
  "} def",
  "",
  "% ltext",
  "/ltext  {",
  "    (ul) eq {",
  "        UnderLineString         % underline string",
  "    } {",
  "        show                    % normal output",
  "    } ifelse",
  "} def",
  "",
  "% ctext",
  "/ctext  {",
  "    /Attrib exch def",
  "    dup                         % copy string",
  "    stringwidth pop 2 div       % compute half width of string",
  "    0 exch sub                  % new x-position",
  "    0 rmoveto                   % move point",
  "    Attrib (ul) eq {",
  "        UnderLineString         % underline string",
  "    } {",
  "        show                    % normal output",
  "    } ifelse",
  "} def",
  "",
  "% rtext",
  "/rtext  {",
  "    /Attrib exch def",
  "    dup                         % copy string",
  "    stringwidth pop             % compute width of string",
  "    0 exch sub                  % new x-position",
  "    0 rmoveto                   % move point",
  "    Attrib (ul) eq {",
  "        UnderLineString         % underline string",
  "    } {",
  "        show                    % normal output",
  "    } ifelse",
  "} def",
  "",
  "%picture empty",
  "/picture {pop pop pop pop pop} def",
  ""
};


static char szFile[PATH_MAX];	/* for file access */
static char szError[128];       /* string for error messages */
static int yact;        /* Actual y position of output "Cursor" (in dots)   */
static int ymin;        /* Minimum y position for variable output (in dots) */
static int ymin_flag=0; /* The layouter has reached the given end (ymin) of a page  */
                        /* (The variable calculation is over for this page ->       */
                        /* Absolute lay_units or actions that depend on precalculated*/
                        /* absolute x,y coordinates can be taken care off now befor */
                        /* we finally continue with the next page.                  */

static int iLineWidth;		/* actual line width */
static int activ_size;  /* two global variables to remember the font and it's size */
#if USEFONTCACHE
static int iActiveFont;
#else
static char activ_font[30]; /* that's "now" activ"  */
#endif

/*
 * pointer to an array of lines.
 * There is an element for every possible element type.
 * The data preprocessor attaches the corresponding string pointers
 * to this array. The layout information has indices into
 * this table.
 */
#if NEWLINEHDL
s_line_array pstLineElem[MAXLINEELEM];
#endif

/* external Function prototypes          */
extern void vdPrintFontDef (FILE *fPtr);	/* bgh_form.c */


/* Static function prototypes   */





/******************************************************************************
 * PrintVersInfoLayout
 *
 * DESCRIPTION:
 * print the version info of LAYOUT
 *
 * PARAMETERS:
 *  void
 *
 * RETURNS:
 *  - none -
 ******************************************************************************
 */
void PrintVerInfoLayout (void)
{
  static char *SCCS_ID = "1.63";

  printf ("%s\t\t\t%s\n", __FILE__, SCCS_ID);
}


#if NEWLINEHDL
/******************************************************************************
 * vdInitLines
 *
 * DESCRIPTION:
 * Initialise the lines array 'pstLineElem'. This procedure has to be
 * called once after startup
 *
 * PARAMETERS:
 *  - none -
 *
 * RETURNS:
 *  - void -
 ******************************************************************************
 */
void vdInitLines (void)
{
  static BOOL	fFirst = TRUE;	/* make sure it gets called only once */
  int 	iIndLay;

  if (fFirst) {
    for (iIndLay = 0; iIndLay < MAXLINEELEM; iIndLay++) {
      pstLineElem[iIndLay].flags	= 0;
      pstLineElem[iIndLay].count	= 0;
      pstLineElem[iIndLay].block_size	= 1;
      pstLineElem[iIndLay].my_argvs	= NULL;
    }
    fFirst = FALSE;
  }
}
#endif

/******************************************************************************
 * vdPostFreeMem
 *
 * DESCRIPTION:
 * release memory allocated by put_next_line
 *
 * PARAMETERS:
 *  state       *paField        - field with layout information
 *
 * RETURNS:
 *  0 - processed all messages for this customer
 ******************************************************************************
 */
void vdPostFreeMem (field *paField)
{
  int           lay_ind;
  UINT		i;
#if !NEWLINEHDL
  field         *p_lay;                
#endif
  s_line_array  *p_asline;

  /* release all by put_next_line allocated memory (!This does not
   * include the memory maybe allocated for strings themselves)
   * put_next_line is building up arrays with dynamic allocated parts
   * attached at lay[x].stline.my_argvs "[0-count]"
   */

#if NEWLINEHDL
  for (lay_ind = 0; lay_ind < MAXLINEELEM; lay_ind++) {
    p_asline = &pstLineElem[lay_ind];
#else
  for (lay_ind = 0; paField[lay_ind].iIndex != -1; lay_ind++) {
    p_lay    = &paField[lay_ind];
    p_asline = &p_lay->st_line;
#endif

    /* should always branch now (no simple_string anymore) */
    if (p_asline->block_size) {
      /*scan array builded up by put_next_line */
      for (i = 0;i < p_asline->count; i++) {
	/* the pointers in the array point to dynamic allocated objects */

	/* return memory allocated in lay_alloc */
        if (p_asline->my_argvs[i]) {
	  free (p_asline->my_argvs[i]);
	  p_asline->my_argvs[i] = NULL;
        } else {
	  /*Every pointer should really be there.Unlikely that else part is reached  */
          macErrorMessage (LAY_NULLPOINTER, WARNING,
                           "Something strange in pointer array?\n"
                           "Unexpected NULL-pointer\n");
        }
      }
      if (p_asline->my_argvs) {
	/* throw away the array itself (was allocated by put_next_line (or ..lay))*/
	free (p_asline->my_argvs);
	p_asline->my_argvs = NULL;
      }
      /* reset the counter to since we've thrown away the corresponding array */
      p_asline->count = 0;
#if NEWLINEHDL
      p_asline->flags = 0;
#endif
    }  /* of if(block_size>0) */
  } /* of for(;iIndex!=-1;) */
}


/******************************************************************************
 * fovdPrintPSText
 *
 * DESCRIPTION:
 *
 * Print postscript text, i. e. the text between the brackets of a 'show'
 * command, every 'real' text (text to be printed, not commands for the printer)
 * must be printed by this routine.
 * 
 * PARAMETERS:
 *  FILE        *fpOut          - File pointer into which the output is produced
 *  char	*szOut		- text to print to fpOut
 *
 * RETURNS:
 * - void -
 ******************************************************************************
 */
static void fovdPrintPSText (FILE *fpOut, char *szOut)
{
  register char *tmp;

  ASSERT (szOut != NULL);
  ASSERT (fpOut != NULL);

  tmp = szOut;

  while (*tmp)
  {
    switch (*tmp)
    {
    case '(':
      /* FALLTHROUGH */
    case ')':
      /* FALLTHROUGH */
    case '\\':
      /* FALLTHROUGH */
      putc ((unsigned int) '\\', fpOut);
      putc ((unsigned int) *tmp, fpOut);
      break;
    default:
      putc ((unsigned int) *tmp, fpOut);
    }
    tmp++;
  }
}


/******************************************************************************
 * PrintLine
 *
 * DESCRIPTION:
 *
 * An ISO A4 Page is 842 Dots high (y) and 595 Dots wide (x) 
 * (US Standard letter = 792 x 612 
 * Only a very small subset from postsript is used. A statement is
 * always completly issued in one line and holds only absolute coordinates
 * - so it shouldn't be a problem to transform the tiny postscript into 
 * some other printerlanguage. Of course if the printer or the viewer
 * (for example: The Public Domain Viewer Ghostview) is able to handle
 * Postscript directly, no further transformation is nessesary.
 * 
 * PARAMETERS:
 *  stLAYINF    *pLay           - pointer to layout info structure
 *  int		iYAct		- current y position to print at
 *  field	*pField		- pointer to current field
 *  int         iActPos       	- index into the may_argv[] array 
 *  FILE        *fpOut          - File pointer into which the output is produced
 *
 * RETURNS:
 * 0 = OK , everything else error (unused so far- just doesn't produce
 *          anything for this layoutunit )
 ******************************************************************************
 */

static int PrintLine (stLAYINF *pLay, 
		      int iYact, 
		      field *pField, 
		      UINT uActPos, 
		      FILE *fpOut)
{
  state *s;

  UINT 	j;				/* for counting      */
  char 	*pc;
  char	*pszOut;

  int item, x, y;			/* simplify referencing */        
  s_line_array  *p_asline;

  s        = pLay->paSta;

#if NEWLINEHDL
  p_asline = pField->pstLine;
#else
  p_asline = &(pField->st_line);
#endif
  x        = pField->x;
  y 	   = iYact - pField->max_size;

  /*
   * Already processed by this function before, if you want to process a table 
   * more than once (maybe after a new sort),its possible to clear the bit.   
   */
  if (pField->fDyn & PRIN)
    return (0);

  /* Since y_min is not reached a PAGE related item is not executed */
#ifdef OLDPAGEUSAGE
  if (!ymin_flag && pField->flags & PAGE) {
    return(1);				/* No Error */
  }
#endif

  if (y < 0) {				/* No longer on page */
    return ((int) LAY_YPOSOFFPAGE);
  }
  if (x < 0) {				/* No longer on page */
    return ((int) LAY_XPOSOFFPAGE);
  }

  for (j = 0;j < pField->num; j++) {	/* for number of rows */
    item = pField->item_list[j];	/* which items belong to line ? */ 

    /*
     * skip items with index < 0, these items are not allowed
     * in the layout; they are used for suppressing single
     * items of a line
     */
    if (item < 0)
    {
      continue;
    }

    /* Set pointer to text */
    pszOut = p_asline->my_argvs[uActPos][j];

    /*
     * skip empty items
     */
    if ((pszOut == NULL) || (pszOut[0] == '\0'))
    {
      continue;
    }

#if USEFONTCACHE
    if (iActiveFont != s[item].iFontIndex) {
      iActiveFont = s[item].iFontIndex;
      fprintf (fpOut, "FN%d setfont\n", iActiveFont);
    }
#else
    if (strcmp (activ_font, s[item].font) ||
	activ_size != s[item].size) {
      /* The size or the font changed */

      strcpy (activ_font, s[item].font);
      activ_size = s[item].size;
      fprintf (fpOut, "/%s findfont %d scalefont setfont\n",
	       activ_font, activ_size);
    }
#endif
    /* print positioning part */
    fprintf (fpOut, "%d %d moveto (",
	     x + s[item].x, y + s[item].y);

    /* print text */
    fovdPrintPSText (fpOut, pszOut);

    /* show - the standard (normal) output */
    if (s[item].pos_flag == 0) {

      fprintf (fpOut, ") show\n");

    } else {
      /* ! ltext rtext ctext can be printed underlined  - show not*/
      if (s[item].pos_flag & (int) TEXTUL) {
	pc = "ul";			/* underline it */
      } else {
	pc = "";
      }

      /* choose between the "advanced" printout possibilitys */

      switch (s[item].pos_flag & 3) {
      case TEXTRIGHT:
	fprintf (fpOut, ") (%s) rtext\n", pc);
	break;
      case TEXTCENTER:
	fprintf (fpOut, ") (%s) ctext\n", pc);
	break;
      case TEXTNORMAL:
	/* FALLTHROUGH */
      case TEXTLEFT:
	/* FALLTHROUGH */
      default:
	fprintf (fpOut, ") (%s) ltext\n", pc);
	break;
      }
    }
  }

  return(0);
}


/******************************************************************************
 * PrintGraphic
 *
 * DESCRIPTION:
 *
 * print a graphics element
 * 
 * PARAMETERS:
 *  st_graphic  *pGraph		- the element to print
 *  int		iYPos		- current y-position
 *  int		iDist		- distance for underlining
 *  FILE	*fpOut		- file pointer for output 
 *
 * RETURNS:
 * - void -
 ******************************************************************************
 */
static void PrintGraphic (st_graphic 	*pGraph, 
			  int 		iYPos,
			  int		iDist,
			  FILE 		*fpOut)
{
  int 		x;			/* values for better referencing */
  int		y;
  int		width;
  int		height;
  int		dist;


  x = pGraph->xPrint;
  y = pGraph->yPrint;
  dist = pGraph->dist;
  width  = pGraph->width + 2 * dist;

  if (!pGraph->fAbs) {			/* relative positioning */
    height = pGraph->yPrint - iYPos + 2 * dist;
  } else {				/* ..absolute positioning */
    height = pGraph->height + 2 * dist;
  }

  /*
   * actually do the graphics output
   */

  /*
   * B. Michler, 27.08.1996:
   * greyscales only for filled boxes, for lines it its
   * the linewidth
   */
  if (pGraph->black != 0) {
    if (pGraph->type == G_FILL) {
      fprintf (fpOut, "%3.2f setgray\n", (float)(pGraph->black)/100);
    } else {
      if (pGraph->black != iLineWidth) {
	fprintf (fpOut, "%3.2f setlinewidth\n", (float)(pGraph->black)/100);
	iLineWidth = pGraph->black;
      }
    }
  }

  switch (pGraph->type) {

  case O_LINE:
    y = y + dist;
    fprintf (fpOut, "%d %d moveto %d %d lineto stroke\n",
	     x, y, x + pGraph->width, y);
    break;

  case U_LINE:
    y = y - iDist - dist;
    fprintf (fpOut,"%d %d moveto %d %d lineto stroke\n",
	     x, y, x + pGraph->width, y);
    break;

  case G_FILL:
    fprintf (fpOut, "%d %d %d %d rectfill\n",
	     x - dist, y - height + dist, width, height);
    break;

  case G_RECT:
    fprintf (fpOut, "%d %d %d %d rectstroke\n",
	     x - dist, y - height + dist, width, height);
    break;

  default:
    break;
  }; /* of switch */

  if (pGraph->black != 0) {
    if (pGraph->type == G_FILL) {
      fprintf (fpOut, "0 setgray\n");
    }
  }
}


/******************************************************************************
 * CheckGraStart
 *
 * DESCRIPTION:
 *
 * check if the starting point for a relative graphics element, or a complete
 * absolute graphics element should be printed
 * 
 * PARAMETERS:
 *  stLAYINF    *pLay           - pointer to layout info structure
 *  field       *pField         - pointer to the field
 *  FILE	*fpOut		- file pointer for output 
 *
 * RETURNS:
 * - void -
 ******************************************************************************
 */
static void CheckGraStart (stLAYINF *pLay, field *pField, FILE *fpOut)
{
  st_graphic 	*pGraph;		/* simplify referencing */
  int		iIndFld;		/* index for field */
  unsigned int	i;			/* simple counter */

  /*
   * search for related graphics now ...
   */
  for (i = 0; i < (sizeof (pField->iaGrFrom) / sizeof (int)); i++) {

    iIndFld = pField->iaGrFrom[i];

    if (iIndFld == -1) {
      /* no more elements */
      break;
    }

    pGraph = &(pLay->paGra[iIndFld]);

    /*
     * the current element is the starting point for graphics ...
     */
    if (pGraph->fAbs) {		/* absolute positioning */

      pGraph->yPrint = pGraph->y;
      pGraph->xPrint = pGraph->x;

      /*
       * Print absolute graphics before the first related element
       * is printed. This takes care of the fact, that a filled
       * box covers everything beyond that was printed before the
       * box itself is printed.
       *
       * Do not print the graphic if the related element was already
       * printed (on another page perhaps)
       */
      if ((pField->fDyn & PRIN) == 0) {

	/* 
	 * the parameter iYPos is ignored for absolute graphics,
	 * iDist is only relevant for relative positioning
	 */
	PrintGraphic (pGraph, 0, 0, fpOut);
      }
    } else {			/* relative positioning */
      /*
       * Changed from:
       * 	pGraph->y = yact;
       * 	pGraph->x = pField->x;
       * to allow relative offsets in graphics
       */

      pGraph->yPrint = pGraph->y + yact;
      pGraph->xPrint = pGraph->x + pField->x;
    }
  }
}


/******************************************************************************
 * CheckGraPrint
 *
 * DESCRIPTION:
 *
 * check if a graphics element should be printed and print it
 * 
 * PARAMETERS:
 *  stLAYINF    *pLay           - pointer to layout info structure
 *  int         iField          - number of the field
 *  int		iYPos		- current y-position
 *  FILE	*fpOut		- file pointer for output 
 *
 * RETURNS:
 * - void -
 ******************************************************************************
 */
static void CheckGraPrint (stLAYINF *pLay, int iField, int iYPos, FILE *fpOut)
{
  unsigned int 	i;
  field 	*pField;		/* simplify referencing */
  st_graphic 	*pGraph;		/* simplify referencing */
  int		iIndFld;		/* index for field */
  int		iDist;

  pField = &(pLay->paFld[iField]);

  /*
   * search for related graphics now ...
   */
  for (i = 0; i < (sizeof (pField->iaGrTo) / sizeof (int)); i++) {

    iIndFld = pField->iaGrTo[i];

    if (iIndFld == -1) {
      /* no more elements */
      break;
    }

    pGraph = &(pLay->paGra[iIndFld]);

    if ((pGraph->to_lay == iField) 	&&
        (pGraph->fAbs == FALSE)		&&
	(pGraph->type != O_LINE) 	&&
	(pGraph->type != U_LINE)) {

      /*
       * the current element is the ending point for graphics ...
       * - and no absolute element -
       * - and NO line -
       * now print the graphics
       */
      iDist = pField->max_size + pField->max_elem_dist;

      PrintGraphic (pGraph, iYPos, iDist, fpOut);
    }
  }
}


/******************************************************************************
 * CheckLinPrint
 *
 * DESCRIPTION:
 *
 * check if a graphics line should be printed and print it
 * 
 * PARAMETERS:
 *  stLAYINF    *pLay           - pointer to layout info structure
 *  int         iField          - number of the field
 *  int		iYPos		- current y-position
 *  FILE	*fpOut		- file pointer for output 
 *
 * RETURNS:
 * - void -
 ******************************************************************************
 */
static void CheckLinPrint (stLAYINF *pLay, int iField, int iYPos, FILE *fpOut)
{
  unsigned int 	i;
  field 	*pField;		/* simplify referencing */
  st_graphic 	*pGraph;		/* simplify referencing */
  int		iIndFld;		/* index for field */
  int		iDist;

  pField = &(pLay->paFld[iField]);

  /*
   * search for related graphics now ...
   */
  for (i = 0; i < (sizeof (pField->iaGrTo) / sizeof (int)); i++) {

    iIndFld = pField->iaGrTo[i];

    if (iIndFld == -1) {
      /* no more elements */
      break;
    }

    pGraph = &(pLay->paGra[iIndFld]);

    if ((pGraph->to_lay == iField) 	&&
        (pGraph->fAbs == FALSE)		&&
	((pGraph->type == O_LINE)	 	||
	 (pGraph->type == U_LINE))) {
      /*
       * the current element is the ending point for graphics ...
       * - and no absolute element -
       * - and it is a line -
       * now print the graphics
       */
      iDist = pField->max_size + pField->max_elem_dist;

      PrintGraphic (pGraph, iYPos, iDist, fpOut);
    }
  }
}


/******************************************************************************
 * PrCheckedItem
 *
 * DESCRIPTION:
 *
 * prints all text and graphics related to a single field without any checks
 * as they were done by calling function PrintItem.
 * 
 * PARAMETERS:
 *  stLAYINF    *pLay           - pointer to layout info structure
 *  int         iIndLay         - index into the lay[] array 
 *
 * RETURNS:
 * 0 = OK, everything else error. 
 ******************************************************************************
 */
static int PrCheckedItem (stLAYINF *pLay, int iIndLay, FILE *fpOut)
{
  field 	*lay;
  int 		yact_mem;		/* internal memo        */
  field 	*pField;		/* simplify referencing */
  BOOL		fOncePrinted;		/* at least on element of list was now printed */

  lay = pLay->paFld;
  pField = &lay[iIndLay];		/*init from struct      */ 

  /*
   * if max_size plus max_elem_dist is 0 this line is not correctly
   * initialised. Look in s[ ] for items mentioned in lay that are
   * probably not described in s[  ] - prevents from running into div /0 !
   */
  if (pField->max_size + pField->max_elem_dist == 0) {
    sprintf (szError, "Line not initialised: lay[%d] max_size=%d max_dist=%d",
	     iIndLay,pField->max_size,pField->max_elem_dist);
    macErrorMessage (LAY_INITLINE, WARNING, szError);
    return ((int) LAY_INITLINE);
  }

  /*
   * calculate the room needed for the simple offset of the unit
   */
  yact = yact - pField->max_dist;

  yact_mem = yact;      /* if its a unit with the INDE (=independent flag) */
                        /* the actual (yact) is NOT changed automaticly as */
                        /* usual.-have to memorise it to be able to restore*/ 
  fOncePrinted = FALSE;			/* remember nothing was printed */

  if (pField->flags & ABSO) {
    /* Its an absolute positioned layoutunit */
    yact = pField->y;
  }

  /*
   * search for related graphics now ...
   */
  CheckGraStart (pLay, pField, fpOut);

  /* insert actual output of line and loop over lists */

#if NEWLINEHDL
  while (pField->akt_pos < pField->pstLine->count) {

    if (*(pField->pstLine->my_argvs[pField->akt_pos]) == NULL) {
#else
  while (pField->akt_pos < pField->st_line.count) {

    if (*(pField->st_line.my_argvs[pField->akt_pos]) == NULL) {
#endif
      /*
       * there are still lines of this type, but they belong to
       * another group and should therefore be printed lateron.
       * So just skip the terminator (NULL-pointer) and continue
       * with the next line.
       */

      /*
       * a problem could be an already applied offset max_dist if 
       * this is the first element, but this should never happen
       */
      /*
       * B. Michler 20.06.96: remove max_dist if nothing of that
       *                      element type was printed up to now
       *                      anyway ... it happened ;-)
       */
      if (fOncePrinted == FALSE) {
	yact = yact + pField->max_dist;
      }

      pField->akt_pos++;

      /*
       * Take care of the graphics
       */
      if (fOncePrinted) {
	CheckGraPrint (pLay, iIndLay, yact, fpOut);
      }

      /*
       * end of a group reached --> continue at start
       */

      /* Independent -> restore yact */
      if (pField->flags & INDE) {
	yact = yact_mem;
      }

      /*
       * B. Michler, 06.09.1996:
       * If a list has the flag for formfeed, every break
       * of that list should start a new page
       */
      if (pField->flags & FEED) {
	  ymin_flag = 1;
      }

      if ((ISLIST (pField)) &&
	  (pField->flags & GEND)) {
	return (PIGROUP);
      } else {
	return (PILISTBROKEN);
      }
    } /* ... broken list */

    if (ISLIST (pField)) {
      /*
       * print no more list elements if we are already at the
       * end of the variable part of the page
       */
      if (yact < (ymin + pField->iAddSpace + pField->max_size)) {
	ymin_flag = 1;
	iNewPageEl = iIndLay;

	if (pField->flags & INDE) {	/* Independent -> restore yact */
	  yact = yact_mem;
	}
	return (PIPAGEEND);
      }
    }

    (void) PrintLine (pLay, yact, pField, pField->akt_pos++, fpOut);
    CheckLinPrint (pLay, iIndLay, yact, fpOut);

    /* this routine printed something */
    fOncePrinted = TRUE;

    /*
     * remember this element as the last one printed if it was
     * a variable element that occurs only once (part of a list)
     */
    if (ISLIST (pField)) {
      giLastListElem = iIndLay;
    }

    /*
     * Calculate the next yact , this is correct if the whole table fits
     * on rest of the page.
     */
    yact = yact - (pField->max_elem_dist + pField->max_size);

    /*
     * check yact against the minimum for this page (ymin)
     * print related graphics if the page ended
     */
    if (ISLIST (pField)) {
      /* below ymin already */
      if (yact < ymin) {
	CheckGraPrint (pLay, iIndLay, yact, fpOut);
      }
    }
  } /* while (pField...) */

  CheckGraPrint (pLay, iIndLay, yact, fpOut);

  if (!(pField->flags & EVPA))		/* 1   = On every page */
    pField->fDyn |= PRIN;		/* set to PRIN = done   */
  else
    pField->akt_pos = 0;		/*reset it due it has to appear on every page */


  /* formfeed-> only calculate PAGE(end) items */
  /* and stop calculating this page */
  if (pField->flags & FEED) {
    ymin_flag = 1;
  }
  if (pField->flags & INDE) {		/* Independent -> restore yact */
    yact = yact_mem;
  }

  /*
   * end of a group reached
   */
  if ((ISLIST (pField)) 	&&
      (pField->flags & GEND)) {
    return (PIGROUP);
  } else {
    return(0);
  }
}


/******************************************************************************
 * ReprintHeader
 *
 * DESCRIPTION:
 *
 * print a header again on the new page
 * works recursive for multiple headers
 *
 * PARAMETERS:
 *  stLAYINF    *pLay           - pointer to layout info structure
 *  int         iIndex       	- index into the lay[] array 
 *
 * RETURNS:
 * 0 = OK, everything else error. 
 ******************************************************************************
 */
void ReprintHeader (stLAYINF *pLay, int iIndex, FILE *fpOut)
{
  field 	*pField;		/* simplify referencing */
  int		iOld;

  pField = &(pLay->paFld[iIndex]);		/*init from struct      */ 

  if (pField->iIndHead != 0)
  {
    /* Check for an self-reference */
    if (pField->iIndHead == iIndex) {
      sprintf (szError, "Element @L%d is its own headline!!!", iIndex);
      macErrorMessage (LAY_INEXPCOUNT, WARNING, szError);
      return;
    }
    /* this element has a header */
    ReprintHeader (pLay, pField->iIndHead, fpOut);
  }

  if (ymin_flag) {
    macErrorMessage (LAY_INEXPCOUNT, WARNING,
		     "The headlines do not fit on a page!\n"
		     "Check your headline definition in the layout file!!!");
    return;
  }

  /* no more header, reprint current element */

  if (pField->akt_pos > 0)
  {
    /*
     * it was already printed, otherwise it wouldn't be a reprint
     * so set print position from my_argvs one element back and
     * reprint
     */
    iOld = pField->akt_pos;

    /*
     * if the current element was empty for the last column,
     * the last two entries are NULL (the list was broken
     * twice without adding an element in between),
     * so do nothing
     */
#if NEWLINEHDL
    if ((iOld == 1) && (*(pField->pstLine->my_argvs[iOld - 1]) == NULL))
#else
    if ((iOld == 1) && (*(pField->st_line.my_argvs[iOld - 1]) == NULL))
#endif
    {
	return;
    }
    if ((iOld > 1) 					&&
#if NEWLINEHDL
	(*(pField->pstLine->my_argvs[iOld - 1]) == NULL) &&
	(*(pField->pstLine->my_argvs[iOld - 2]) == NULL))
#else
	(*(pField->st_line.my_argvs[iOld - 1]) == NULL) &&
	(*(pField->st_line.my_argvs[iOld - 2]) == NULL))
#endif
    {
	return;
    }

    pField->akt_pos--;

    /*
     * take care of an eventually broken list
     */
#if NEWLINEHDL
    if (*(pField->pstLine->my_argvs[pField->akt_pos]) == NULL) {
#else
    if (*(pField->st_line.my_argvs[pField->akt_pos]) == NULL) {
#endif
      do {
	if (pField->akt_pos > 0) {
	  pField->akt_pos--;
	} else {
	  break;
	}
#if NEWLINEHDL
      } while (*(pField->pstLine->my_argvs[pField->akt_pos]) != NULL);
#else
      } while (*(pField->st_line.my_argvs[pField->akt_pos]) != NULL);
#endif

#if NEWLINEHDL
      if (*(pField->pstLine->my_argvs[pField->akt_pos]) == NULL) {
#else
      if (*(pField->st_line.my_argvs[pField->akt_pos]) == NULL) {
#endif
	pField->akt_pos++;
      }
    }
    /* clear flag PRIN */
    pField->fDyn = pField->fDyn & ~PRIN;

    (void) PrCheckedItem (pLay, iIndex, fpOut);

    pField->akt_pos = iOld;
  }
}


/******************************************************************************
 * PrintItem
 *
 * DESCRIPTION:
 *
 * PrintItem prints all text and graphics related to a single field.
 * This may be a complete list.
 * 
 * PARAMETERS:
 *  stLAYINF    *pLay           - pointer to layout info structure
 *  int         iIndLay         - index into the lay[] array 
 *
 * RETURNS:
 * 0 = OK, everything else error. 
 ******************************************************************************
 */
static int PrintItem (stLAYINF *pLay, int iIndLay, FILE *fpOut)
{
  field 	*lay;
  field 	*pField;		/* simplify referencing */

  lay = pLay->paFld;
  pField = &lay[iIndLay];		/*init from struct      */ 

  /*
   * Already processed, this means that there has been already
   * a postscript output for this layoutunit. If you want to process a table
   * more than once (maybe after a different sort) clear the bit.
   */

  if (pField->fDyn & PRIN)			return (0);

  /*
   * We left the variable part of the page (yact got smaller than ymin)
   * -> only layoutunits with the PAGE flag (for PAGEend) are still handled 
   */

  if (ymin_flag && !(pField->flags & PAGE))   	return (0);

  /*
   * if ymin_flag isn't set, yet items with PAGE flag are ignored.
   * this might be usefull, if you decide that something should only show
   * up as long as some other layout isn't printed.
   * if you have no need for this put layunits with PAGE to the end of the
   * array.
   */
#ifdef OLDPAGEUSAGE
  if (!ymin_flag && (pField->flags & PAGE)) 	return (0);
#endif

  /*
   * if there is nothing to print for this element, do no position
   * calculation, just stop for this element, but mark it as printed;
   * otherwise printing for this document would never stop...
   */
#if NEWLINEHDL
  if (pField->akt_pos >= pField->pstLine->count) {
#else
  if (pField->akt_pos >= pField->st_line.count)	{
#endif
    pField->fDyn |= PRIN;

    /*
     * Do a formfeed even for not-printed elements
     */
    if (pField->flags & FEED) {
      ymin_flag = 1;
    }

    /*
     * end of a group reached
     */
    if ((ISLIST (pField)) &&
	(pField->flags & GEND)) {
      return (PIGROUP);
    } else {
      return(0);
    }
  }

  /*
   * only print the element if its not a list or if it is the
   * next list element
   * or if its to be on every page
   */
  if ((ISLIST (pField)) &&
      ((pField->flags & EVPA) == 0)) {
    if (iIndLay < giLastListElem) {
      return (0);
    } else if (iNewPageEl != -1) {
      /*
       * the last page was terminated at element 'iNewPageEl',
       * we should only proceed at this element, because it
       * could be within a broken list, and the other list elements
       * do not have PRIN set, because there is still something
       * to print
       */
      if (iIndLay < iNewPageEl) {
	return (0);
      } else {
	/*
	 * This is the right position to think about printing
	 * the headline on the new page.
	 */

	if (pField->iIndHead != 0) {
	  /* element has a header, try to print it again */
	  ReprintHeader (pLay, pField->iIndHead, fpOut);
	}
	iNewPageEl = -1;
      }
    }
  }

  return (PrCheckedItem (pLay, iIndLay, fpOut));
}


/******************************************************************************
 * iOpenDocument
 *
 * DESCRIPTION:
 *
 * initialise document for printing
 *   
 * PARAMETERS:
 *  char	*szOutFile   - filename of outputfile
 *  FILE        **fpOut      - pointer to filepointer outputfile (returned)
 *  int		iCopies	     - number of copies
 *
 * RETURNS:
 *  0 if succesful
 ******************************************************************************
 */
int iOpenDocument (char *szOutFile, FILE **fpOut, int iCopies)
{
  int 	i;
  FILE *fp;   /* copy metahead into output */
  char pc;    /* helppointer */

  /*
   * check the global pointers
   */
  ASSERT (szOutFile != NULL);

  /* open the file */
  *fpOut = fopen (szOutFile, "w");

  if (*fpOut == NULL)
  {
    macErrorMessage (FILE_OPEN, WARNING,
		     "LAYOUT: could not open output file");
    return ((int) FILE_OPEN);
  }


/**************************************************************************/
/*                     produce postscript pages                           */
/**************************************************************************/

  fprintf (*fpOut,"%%!PS-Adobe-3.0\n"); 	/* Just needed by postscript */
  fprintf (*fpOut,"%%%%Creator: BGH\n");
  fprintf (*fpOut,"%%%%Version: %s\n", PSVERSION);
  fprintf (*fpOut,"%%%%Coding: ASCII\n");
  fprintf (*fpOut,"%%%%Copies: %d\n", iCopies);
  fprintf (*fpOut,"%%%%Pages: (atend)\n");
  fprintf (*fpOut,"%%%%BeginProlog\n");
  fprintf (*fpOut,"%%%%EndProlog\n");
  fprintf (*fpOut,"%%%%BeginSetup\n");


  sprintf (szFile, "%smetahead.ps", stBgh.szLayoutDir);
  fp = fopen (szFile, "r");  		/* if file doesn't exist a hardcoded */
  if (!fp)                        	/* intern meta part is attached      */
  {
    fovdPrintLog (LOG_TIMM,
		  "Couldn't open [%s], inserting buildin default\n",
		  szFile);

    for (i = 0; i < (sizeof (meta) / sizeof (char *)); i++)
    {						/* internal meta so the post -*/
      fprintf (*fpOut, "%s\n", meta[i]);        /* script printer won't crash */ 
    }
  }
  else
  {
    for( ; EOF != (pc = getc (fp)); ) 		/* Ok it's there */
      putc (pc, *fpOut);
  }

#if USEFONTCACHE
  /* print font definitions */
  vdPrintFontDef (*fpOut);
#endif

  fprintf (*fpOut,"%%%%EndSetup\n");

  return (0);
}


/******************************************************************************
 * vdCloseDocument
 *
 * DESCRIPTION:
 *
 * writes end information in a document
 *   
 * PARAMETERS:
 *  FILE        *fpOut       - filepointer outputfile
 *  int		iPages	     - number of pages in document
 *
 * RETURNS:
 *  void
 ******************************************************************************
 */
void vdCloseDocument (FILE *fpOut, int iPages)
{
  /* When all pages are printed produce postscript trailer */

  fprintf(fpOut,"%%%%Trailer\n");
  fprintf(fpOut,"%%%%Pages: %d\n", iPages);
  fprintf(fpOut,"%%%%EndComments\n");

  if (fclose (fpOut) != 0)
  {
    macErrorMessage (FILE_CLOSE, WARNING,
		     "LAYOUT: could not close Postscript file");
  }
}


/******************************************************************************
 * iPrintLayout
 *
 * DESCRIPTION:
 *
 * vdPrintLayout generates the postscript output for one (filled) complex
 * layout information structure of the type stLAYINF.
 * It first prints all fixed graphics for every page, and then the text and
 * the variable graphics. The biggest disadvantage of these method is that
 * a filled box may not be a variable graphics element, because a box covers
 * everything beneath it that was printed earlier; but variable graphics
 * can only be printed after the underlying text, because all needed positioning
 * information is only available after calculating (and printing) the text.
 *   
 * PARAMETERS:
 *  FILE        *fpOut       - filepointer outputfile
 *  stLAYINF    *pLay        - pointer to layout info structure
 *  char	*szType	     - type information for jumping pages in GhostScript
 *  int		iPage	     - current page number
 *
 * RETURNS:
 *  next page number
 ******************************************************************************
 */
int iPrintLayout (FILE  *fpOut, stLAYINF *pLay, char *szType, int iPage)
{
  field		*lay;
  field		*pField;
  int		iGrStart; 	/* start of a list grouping  */

  int		iPrRet;		/* return value from PrintItem */
  int 		iLayIndex;
  int 		page;

  int		iRemainList;	/* number of remaining lists on page */

  /*
   * check the global pointers
   */
  ASSERT (pLay != NULL);

  lay           = pLay->paFld;

  /*
   * ***********************************************************
   * ****          main loop (once for every page)        ******
   * ***********************************************************
   */
  activ_size = 0;
#if USEFONTCACHE
  iActiveFont = -1;
#else
  strcpy (activ_font,"");
#endif
  giLastListElem = 0;

  /* reset value for list grouping */
  iGrStart = 0;

  /*
   * print pages as long as there is something to print
   */
  page = 1;

  do {
    iLineWidth = -1;			/* to set it new on every page */
    yact = pLay->iPageHeight;           /* Postscript start */

    if (page == 1) {                    /* set pagelength */
      ymin = pLay->iVEndFirst;
    } else {
      ymin = pLay->iVEndMiddle;
    }
    sprintf (gszPageString, "%d",page);

    if (page > 20000) {
      macErrorMessage (LAY_INEXPCOUNT, CRITICAL,
		       "Page Overflow ? Pages > 20000");
    }

    /* reset counter for remaining lists */
    iRemainList = 0;

    /*
     * put the pageinformation in the postscript file - allows jumping pages 
     * with a previewer or print just dedicated pages.
     */

    fprintf(fpOut,"%%%%Page: (%s: %d) %d\n", szType, page, iPage);

    /*
     * print one page,
     * start with iLayIndex = 1 because the element with
     * index 0 is the dummy element which is never used
     */
    for (iLayIndex = 1, ymin_flag = 0;
	 lay[iLayIndex].iIndex != -1;
	 iLayIndex++) {

      pField = &lay[iLayIndex];

      /* don't print FOPA elements on first page */
      if ((page == 1) &&(pField->flags & FOPA)) {
	/*
	 * set EVPA,
	 * because from now on it should appear on every page
	 * and the printing routine doesn't know the page number
	 */
	pField->flags |= EVPA;

	continue;
      }

      /* count remaining lists only after pageend */
      /*#define ISLIST(a)	(!((a)->flags & (PAGE | LAPA)))*/
      if (ymin_flag && ISLIST (pField)) {
	iRemainList++;
      }

      /*
       * PrintItem returns the value PILISTBROKEN if it detects a broken
       * list i. e. a grouping of lists occured. If it is the first list,
       * its index should be remembered, because we have to continue at
       * this point if the end of the list grouping is reached
       */
      iPrRet = PrintItem (pLay, iLayIndex, fpOut);

      if ((iPrRet == PILISTBROKEN) &&
	  (iGrStart == 0)) {
	iGrStart = iLayIndex;
      } else if ((iPrRet == PIGROUP) &&
		 (iGrStart != 0)) {

	/* continue at break position */
	iLayIndex = iGrStart - 1;	/* index is increased in for-loop */
	giLastListElem = iLayIndex;
	iGrStart = 0;
      }
    }

    /* to display the page in postscript you need a "showpage" */

    fprintf(fpOut,"showpage\n");

    page++;
    iPage++;
    /*
     * as long as ymin_flag is set, there is still something
     * important to print.
     */
  } while (ymin_flag && iRemainList);

  if (iGrStart != 0) {
    macErrorMessage (LAY_INEXPCOUNT, WARNING,
		     "LAYOUT: Unterminated open text group, "
		     "check layout description!");
  }

  return (iPage);
}


/******************************************************************************
 * vdInitLayout
 *
 * DESCRIPTION:
 * searches for the biggest fontsize and distance between the items
 * of a layoutunit and initialises the corresponding these fields in "lay[]" 
 * -> if you change the fontsize of 1 item in a table the whole table will have
 * a larger spacing between the lines. Resets any remaining working flags
 * might remain from previous runs. 
 *
 * PARAMETERS:
 *  stLAYINF	*pstLay		- layout information
 *
 * RETURNS:
 *  so far always 0 (later errorcode ?)
 ******************************************************************************
 */

void vdInitLayout (stLAYINF *pstLay)
{
  int   iIndLay;		/* counter for items */

  UINT  j;
  int   item;
  field *p_lay;
  state *paState;
  field *paField;
  int	iHeader;		/* index to headline element */
  int	iCurInd;		/* index to element that has headline */
  int	iOffset;		/* additional offset ot page end for header */

  ASSERT (pstLay != NULL);

  paState = pstLay->paSta;
  paField = pstLay->paFld;

  ASSERT (paState != NULL);
  ASSERT (paField != NULL);

#if NEWLINEHDL
  vdInitLines ();
#endif

  for (iIndLay = 0; paField[iIndLay].iIndex != -1; iIndLay++) {

    p_lay = &paField[iIndLay];     /* simplifiy writing and refrencing */ 

#if NEWLINEHDL
    /* set a dummy pointer for empty elements */
    if (p_lay->pstLine == NULL) {
      p_lay->pstLine = &(pstLineElem[0]);
    }
#endif
    /* 
     * next variables are changed during processing and have to be reset
     * after (or befor) each printout (=postscript output).
     */
    p_lay->fDyn = 0;		/* clears dynamic flags                  */

    /*
     * make sure, that an item is either on every page, or
     * on follow up pages; the postscript generator sets
     * EVPA for all items with FOPA after the first page !
     * (so it gets reset here)
     */
    if (p_lay->flags & FOPA) {
      p_lay->flags &= ~EVPA;
    }

    p_lay->akt_pos =0;

    /*
     * put_next_line and (put_next_lay) can only work if block_size is at
     * least 1. block_size = how many pointers are allocated at once.
     * to memorise the lines of a lay_unit (also logical page or field)
     * after the first printout it's a pure savety if (not changed during process)
     */

#if NEWLINEHDL
#if 0
    ASSERT (p_lay->pstLine != NULL);

    if(!p_lay->pstLine->block_size) {
      p_lay->pstLine->block_size = 1;
    }
#endif
#else
    if(!p_lay->st_line.block_size) {
      p_lay->st_line.block_size = 1;
    }
#endif

    /*
     * Should never branch.    if count != 0 the dynamic builded array
     * from last call probably hasn't been returned.(should not happen
     * since I wrote the returning function myself). To garanty functionality
     * count is bruteforce set back to 0 without freeing anything here.
     * (this will waste memory till the process dies) but is probably safer
     * than risking to corrupt the memorymanagement
     * [The above comment is from Alex ;-) ]
     */
#if NEWLINEHDL
    /* it is a normal case now ... */
#else
    if(p_lay->st_line.count)
    {
      sprintf (szError,
               "In lay[%d] count was %d and not 0 as expected, "
               "there has probably memory not been released too",
               iIndLay,p_lay->st_line.count); 
      macErrorMessage (LAY_INEXPCOUNT, WARNING, szError);
      p_lay->st_line.count = 0;
    }
#endif

    /*
     * size out of all linemembers (The s Array holds layoutinfo about every
     * simple Data in the layout. =relative xpos in the line, Charsize and
     * type etc. Since this data once calculated isn't changed by the process
     * you could also calculate it only once to save time as long as the layout
     * doesn't change.
     */
    for(p_lay->max_size=0, p_lay->max_elem_dist = 0,j=0; j<p_lay->num;j++)
    {
      item = p_lay->item_list[j];        /* Maximum size has to be searched */

      /* item == -1 is a suppressed item */
      if (item >= 0) {
	if(paState[item].size > p_lay->max_size)
	  p_lay->max_size = paState[item].size;/* Find the maximum Charaktersize*/
	if(paState[item].dist_y > p_lay->max_elem_dist)
	  p_lay->max_elem_dist = paState[item].dist_y;   /* and the greatest distance */
      }
    }

    /* clear additional spaces used for headlines */
    p_lay->iAddSpace = 0;
  } /* for (...) */

  /* now the distances are calculated, set headline information */
  for (iIndLay = 0; paField[iIndLay].iIndex != -1; iIndLay++)
  {
    iHeader = paField[iIndLay].iIndHead;
    iCurInd = iIndLay;
 
    if (iHeader != 0) {
      /*
       * this element has a headline, now insert the additional distance
       * to the headline element for page breaks
       */

      j = 0;

      /* --- do it for all headlines above --- */
      do {
	/* set headline flag */
	paField[iHeader].flags |= HEAD;

	iOffset = paField[iCurInd].iAddSpace;

	if ((paField[iCurInd].flags & INDE) == 0) {
	  iOffset += paField[iCurInd].max_size      +
	             paField[iCurInd].max_elem_dist +
	             paField[iCurInd].max_dist;
	}
	if (iOffset < paField[iHeader].iAddSpace)
        {
	  /* the new distance is smaller than the already applied */
	  break;
	}
	paField[iHeader].iAddSpace = iOffset;

	/* go on to the next prior headline element */
	iCurInd = iHeader;
	iHeader = paField[iHeader].iIndHead;

	/* loop counter for detecting endless loop */
	if (j++ > 100) {
	  sprintf (szError,
		   "Endless loop in headline definition of Element @L%d,\n"
		   "Check layout file !!",
		   paField[iHeader].iIndex);
	  macErrorMessage (LAY_INITLINE, CRITICAL, szError);
	  break;
	}
      } while (iHeader);
    }
  }
}


#if NEWLINEHDL
/******************************************************************************
 * fpcAllocLay (new version of lay_alloc)
 *
 * DESCRIPTION:
 *
 * for a complex layoutunit (line or field) you need n pointers to strings
 * to memorise the n items of a line. these are allocated here. Always enough
 * to hold lay.num itempointers.
 * - I do handle everythig complex now
 * lay_alloc returns a pointer to an argv like dynamicly allocated pointerarray
 * It is always the maximum number allocated.
 *
 * NO DEFAULT ARGUMENTS ANY MORE
 *
 * PARAMETERS:
 *  - void -
 *
 * RETURNS:
 *  pointer to the allocated pointer array (argv like)
 ******************************************************************************
 */
char **fpcAllocLay (void)
{
  char 	**my_argv;		/* return pointer           */

  my_argv = (char **) calloc (NRELINLINE + 1, sizeof (char *));

  if(!my_argv)
  {
    macErrorMessage (LAY_MALLOC, CRITICAL, "lay_alloc: calloc didn't work");
    exit(-1);                              
  }

  my_argv[0] = "";		/* NULL would mean broken list! */

  return (my_argv);
}
#endif

/******************************************************************************
 * lay_alloc
 *
 * DESCRIPTION:
 *
 * for a complex layoutunit (line or field) you need n pointers to strings
 * to memorise the n items of a line. these are allocated here. Always enough
 * to hold lay.num itempointers. (I plan to handle all units complex 
 * to unify handling) -
 * - I do handle everythig complex now
 * lay_alloc returns a pointer to an argv like dynamicly allocated pointerarray
 * how much is allocated determins lay[iIndLay].num
 * 
 * PARAMETERS:
 *  iIndLay - index into the lay[] array 
 *
 * RETURNS:
 *  pointer to the allocated pointer array (argv like)
 ******************************************************************************
 */
char **lay_alloc (state *pState, field *lay, int iIndLay)
{
  UINT j;                                   /* for counting             */
  int item;                                 /* for simplify referencing */
  field *p_lay;                             /* for simplify refrencing  */
  char **my_argv;                           /* return pointer           */

  p_lay = &lay[iIndLay];                  /* for simplify referencing */ 
 
  my_argv = (char **) calloc (p_lay->num + 1, sizeof (char *));
  if(!my_argv)
  {
    macErrorMessage (LAY_MALLOC, CRITICAL, "lay_alloc: calloc didn't work");
    exit(-1);                              
  }
  for (j = 0; j < p_lay->num; j++) 	     /* Additionaly fill with      */
  {
    item = p_lay->item_list[j];              /* default information (or    */

    /* item == -1 is a suppressed item */
    if (item >= 0) {
      my_argv[j]= pState[item].info;           /* already the real if it's   */
                                               /* fixtext.                   */
    }
  }
  return (my_argv);
}


/******************************************************************************
 * put_next_line
 *
 * DESCRIPTION:
 * 
 * The putnext_line function creates an array of pointers of variable 
 * lenght. (In this case an array of pointers to "argv - main" like  
 *  structures.- needed for layout reasons - one argv = 1 Line with n Items  
 * - It's easy to use qsort on these arrays.                       
 * Examplecall ...
 * typedef struct { int count;         Incremented by this function (!Start=0)
 *                  int block_size;    Free choseable - How much room for
 *                                     the array is alloced at once - much or
 *                                     all thats needed for good performance 
 *                  char ***my_argvs;  The array is builded up here.
 *                } s_line_array;  
 *
 *     s_line_array example;
 *     char **myargv; char *pc = "H";                   Need one argv-like 
 *                                                      pointer, + Init some 
 *     struct{ char *a,*b; } st = { "ello",OA" World"};   demo info - 
 *     myargv = (char **) calloc( 3 * sizeof(char *));  room for pointers -
 *     myargv[0] = pc; myargv[1]=st.a; myargv[2]=st.b;  searching in any    
 *                                                      structur and by any 
 *                                                      criteria you like.
 *                                                      and memorise. 
 *     put_next_line(example,myargv);                   (push on stack like) 
 *                 }                                    but producing array
 *                                                      repeat this for every
 *                                                      line of a table.  
 *     And later: for(i=0;i<example.count;i++)          every line
 *                { myargv=example.myargvs[i];          one line
 *                  for(printf("\n"),j=0;myargv[j];j++) every item of the line
 *                      printf("%s ",myargv[j]); 
 *
 * PARAMETERS:
 *  s_line_array *p_so  -  
 *  char **new_my_argv  -
 *
 * RETURNS:
 * 0 = OK, everything else error. 
 ******************************************************************************
 */

int put_next_line(s_line_array *p_so,char **new_my_argv)
{
  char ***p_help;  /* Used to initialisize the pointerarray with NULL'S*/

  UINT i;           /* For counting */

  if (!p_so->block_size)        /* blocksize not initialized - should never branch */
  {
    macErrorMessage (LAY_NOBLOCKSIZE, CRITICAL, "put_next_line: no_blocksize");
    return(-1);
  }
  if(!p_so->count)
  {
    /*
    **************************  CODE REACHED ONCE ***************************
    *  First call - no argument in list -> malloc. Make room for 1+block_size 
    *  pointers.  block_size is only performance dependent. Big = lesser reallocs 
    *  but wasting more space.
    */

    /* p_help = (char ***) malloc ((1 + p_so->block_size) * sizeof (char ***)); */
    /*
     * B. Michler 04.06.96: start always with 2,
     * but increase block_size every reallocation time
     */
    p_help = (char ***) malloc (2 * sizeof (char ***));
    p_so->block_size = 1;
    p_so->my_argvs = p_help;

    if (!p_so->my_argvs) {
      macErrorMessage (LAY_MALLOC, CRITICAL, "put_next_line: allocation error");
    }
    /* 
     * Initialize array with NULL's. Due to the additional Pointer (the +1) 
     * there will always be at least one NULL at the End of the array.
     */
    for (i = 0;i < 1 + p_so->block_size; i++)
      *p_help++=NULL;

  } else {

    /* All but the first call of put_next_line land here  */

    if(!(p_so->count % (p_so->block_size))) {

      /*
      *********** CODE REACHED ONLY EVERY BLOCKSIZE TIME ********************* 
      * -> if here there's no room left in the array it has to be reallocated
      */

      /* increase block_size to a maximum of 16 */
      if (p_so->block_size < 16) {
	p_so->block_size *= 2;
      }

      p_so->my_argvs = (char ***) realloc (p_so->my_argvs,
                                           (1 + p_so->count + p_so->block_size) *
                                           sizeof (char ***));
      if (!p_so->my_argvs) {
        macErrorMessage (LAY_MALLOC, CRITICAL, "put_next_line: allocation error");
      }
      /* Again initialize with NULL */
      p_help = p_so->my_argvs + p_so->count;
      for (i =0; i < 1 + p_so->block_size; i++)
        *p_help++ = NULL;
    }
  }

  /*
  *********************** CODE REACHED EVERY TIME ************************ 
  * Put the new Pointer in place and increment the elementcounter  
  */

  *(p_so->my_argvs + p_so->count) = new_my_argv;

  p_so->count++;
  return(0);
}

#if 0
/******************************************************************************
 * fovdDumpElem
 *
 * DESCRIPTION:
 * 
 * print the contents of the line element
 *
 * PARAMETERS:
 * field *lay           pointer to layout element
 * char **my_argv (also same as in put_next_line)
 *
 * RETURNS:
 * - void -
 ******************************************************************************
 */

void fovdDumpElem (stLAYINF *pLay, int iLayIndex)
{
  int		i;
  int		count;
  s_line_array 	*p_so;

  ASSERT ((pLay != NULL) && (pLay->paFld != NULL));

#if NEWLINEHDL
  if ((iLayIndex < 0) || (iLayIndex >= MAXLINEELEM)) {
    return;
  }
  p_so = &(pstLineElem[iLayIndex]);
#else
  /*
   * check if iIndex is too big
   * (which means that this element is not in actual layout)
   */
  if (iLayIndex > pLay->iFIndMax) {
    return;
  }
  p_so = &(pLay[iLayIndex].st_line); 
#endif

  count = 0;

  for (i = 0; i < p_so->count; i++) {
    if (p_so->my_argvs[i][0] == NULL) {
      printf ("[%03d] Element %03d broken %d\n", iLayIndex, i, count);
      count++;
    } else {
      printf ("[%03d] Element %03d: \"%s\"\n", iLayIndex, i, p_so->my_argvs[i][0]);
    }
  }
}
#endif

/******************************************************************************
 * put_next_lay
 *
 * DESCRIPTION:
 * 
 * wrapper around put_next_line. Caller can forget about calculating
 * the adress of the dynamic array - just has to know iIndLay  
 * (Which unit he wants to attach data to)
 *
 * PARAMETERS:
 * field *lay           pointer to layout element
 * char **my_argv (also same as in put_next_line)
 * RETURNS:
 * int see return codes of put_next_line 
 ******************************************************************************
 */

int put_next_lay (field *play, int iLayIndex, char **new_my_argv)
{
  int rc;
  s_line_array *p_so;

#if NEWLINEHDL
  p_so = play[iLayIndex].pstLine; 
#else
  p_so = &(play[iLayIndex].st_line); 
#endif
  rc = put_next_line (p_so, new_my_argv);
  
  return (rc);
}

/******************************************************************************
 * vdAddElement
 *
 * DESCRIPTION:
 * add a line to the layout
 * iNrTexts gives the number of text-elements the line contains,
 * if a line has more elements than stated here or if a pointer to
 * an element is NULL, the defaults from the layout file are printed for
 * these elements
 * 
 *
 * PARAMETERS:
 * stLAYINF     *pLay           - pointer to layout info structure
 * int          iIndex          - index of element
 * int          iNrTexts        - number of texts to append
 * ...                          - list of character pointers
 *
 * RETURNS:
 * - void -
 *
 ******************************************************************************
 */
void vdAddElement (stLAYINF *pLay,
                   int   iIndex,
                   int   iNrTexts,
                   ...)
{
  va_list       ap;                     /* pointer to data field */
  char          **my_argv;              /* static for consecutive calls */
  char          *pTmp;
  int           j;

  fovdPushFunctionName ("vdAddElement");

  ASSERT ((pLay != NULL) && (pLay->paFld != NULL));
  ASSERT (iNrTexts > 0);

  va_start (ap, iNrTexts);

#if NEWLINEHDL
  ASSERT (iNrTexts < NRELINLINE);

  if (iIndex < MAXLINEELEM) {
    my_argv = fpcAllocLay ();

    /*
     * take one argument (char *) after the other
     * and append them to output
     */
    for (j = 0; j < iNrTexts; j++) {
      pTmp = va_arg (ap, char *);       /* fetch next parameter */
      if (pTmp != NULL) {
	my_argv[j] = pTmp;              /* assign parameter */
      }
    }
    (void) put_next_line (&(pstLineElem[iIndex]), my_argv);

    /* set flag that at least one lement is set */
    pstLineElem[iIndex].flags |= LAFLAG_SET;
  }
#else
  /*
   * do nothing if iIndex is too big
   * (which means that this element is not in actual layout)
   */
  if (iIndex <= pLay->iFIndMax) {

    int           i;

    i = pLay->paFInd[iIndex];

    my_argv = lay_alloc (pLay->paSta, pLay->paFld, i);

    /*
     * take one argument (char *) after the other
     * and append them to output
     */
    if (iNrTexts > pLay->paFld[i].num) {
      iNrTexts = pLay->paFld[i].num;
    }
    for (j = 0; j < iNrTexts; j++) {
      pTmp = va_arg (ap, char *);       /* fetch next parameter */
      if (pTmp != NULL) {
	my_argv[j] = pTmp;              /* assign parameter */
      }
    }
    (void) put_next_lay (pLay->paFld, i, my_argv);

    /* processed -> set CALC Flag */
    pLay->paFld[i].fDyn |= CALC;
  }
#endif
  va_end (ap);
  fovdPopFunctionName ();
}


/******************************************************************************
 * fofLineExists
 *
 * DESCRIPTION:
 * check if a line element exists (is in layout)
 *
 *
 * PARAMETERS:
 * stLAYINF     *pLay           - pointer to layout info structure
 * int          iIndex          - index of element
 *
 * RETURNS:
 * FALSE if it does not exist
 * TRUE  if it exists
 ******************************************************************************
 */
BOOL fofLineExists (stLAYINF *pLay, int iIndex)
{
  ASSERT ((pLay != NULL) && (pLay->paFld != NULL));
#if NEWLINEHDL
  if ((iIndex < 0) || (iIndex >= MAXLINEELEM)) {
    return (FALSE);
  }
  return (TRUE);
#else
  /*
   * check if iIndex is too big
   * (which means that this element is not in actual layout)
   */
  if (iIndex <= pLay->iFIndMax) {

    if (pLay->paFInd[iIndex] != 0) {

      return (TRUE);
    }
  }
  return (FALSE);
#endif
}


/******************************************************************************
 * fofIsPrinted
 *
 * DESCRIPTION:
 * check if an element was already prepared for printing
 *
 *
 * PARAMETERS:
 * stLAYINF     *pLay           - pointer to layout info structure
 * int          iIndex          - index of element
 *
 * RETURNS:
 * FALSE if it was not prepared for printing
 * TRUE  if it does not exist or was prepared for printing
 ******************************************************************************
 */
BOOL fofIsPrinted (stLAYINF *pLay, int iIndex)
{
  ASSERT ((pLay != NULL) && (pLay->paFld != NULL));
#if NEWLINEHDL
  if ((iIndex >= 0) && (iIndex < MAXLINEELEM)) {
    return (pstLineElem[iIndex].flags & LAFLAG_SET);
  }
#else
  /*
   * do nothing if iIndex is too big
   * (which means that this element is not in actual layout)
   */
  if (iIndex <= pLay->iFIndMax) {

    int           i;

    i = pLay->paFInd[iIndex];

    /* processed -> CALC Flag is set */
    if ((pLay->paFld[i].fDyn & CALC) == 0)
    {
      return (FALSE);
    }
  }
#endif
  return (TRUE);
}


/******************************************************************************
 * vdBreakList
 *
 * DESCRIPTION:
 * break a list:
 * all elements have to be broken
 * 
 *
 * PARAMETERS:
 * stLAYINF     *pLay           - pointer to layout info structure
 * int          iNrEleme        - number of elements to break
 * ...				- list of elements
 *
 * RETURNS:
 * - void -
 *
 ******************************************************************************
 */
void vdBreakList (stLAYINF *pLay, int iNrElem, ...)
{
  va_list       ap;                     /* pointer to data field */
  int		iElem;			/* element */
  char          **my_argv;              /* static for consecutive calls */

#if !NEWLINEHDL
  int		iInd;			/* index to element */
  field		*paField;
#endif

  ASSERT ((pLay != NULL) && (pLay->paFld != NULL));
  ASSERT (iNrElem != 0);

  va_start (ap, iNrElem);

#if NEWLINEHDL
  for (; iNrElem != 0; iNrElem--) {

    iElem = va_arg (ap, int);       	/* fetch next parameter */

    ASSERT ((iElem >= 0) && (iElem < MAXLINEELEM));

    /* break list */
    my_argv = fpcAllocLay ();
    my_argv[0] = NULL;
    (void) put_next_line (&(pstLineElem[iElem]), my_argv);
  }
#else
  paField = pLay->paFld;

  for (; iNrElem != 0; iNrElem--) {

    iElem = va_arg (ap, int);       	/* fetch next parameter */

    ASSERT (iElem >= 0);

    if (iElem > pLay->iFIndMax) {
      /*
       * index of element is greater than last element
       * -> element not in the layout information
       */
      continue;
    }

    iInd = pLay->paFInd[iElem];

    /* break list */
    my_argv = lay_alloc (pLay->paSta, pLay->paFld, iInd);
    my_argv[0] = NULL;
    (void) put_next_lay (pLay->paFld, iInd, my_argv);
  }
#endif
  va_end (ap);
}

#if 0
/******************************************************************************
 * vdBreakLay
 *
 * DESCRIPTION:
 * break a list:
 * start from the given element and search the layout upwards for the
 * beginning, breaking all elements on the way
 * 
 *
 * PARAMETERS:
 * stLAYINF  	*pLay           - pointer to layout info structure
 * int	     	iBegin    	- element to begin with
 *
 * RETURNS:
 * - void -
 *
 ******************************************************************************
 */
void vdBreakLay (stLAYINF *pLay, int iBegin)
{
  int		iInd;			/* index to element */
  char      	**my_argv;   		/* static for consecutive calls */
  field		*paField;

  ASSERT ((pLay != NULL) && (pLay->paFld != NULL));

  paField = pLay->paFld;

  ASSERT (iBegin >= 0);

  if (iBegin > pLay->iFIndMax) {
    /*
     * index of element is greater than last element
     * -> element not in the layout information
     */
    return;
  }

  iInd = pLay->paFInd[iBegin];
 
  iInd++;
  do {
    iInd--;
    /* break list */
    my_argv = lay_alloc (pLay->paSta, paField, iInd);
    my_argv[0] = NULL;
    (void) put_next_lay (paField, iInd, my_argv);
  } while ((iInd > 0) && (!(paField[iInd].flags & GBEG)));

  if (iInd < 0) {
    /* Error: group did never start */
    sprintf (szError,
	     "No corresponding 'GRBEG' for 'GROUP' at Element @L%d!\n"
	     " -- Layout corrupted --",
	     iBegin);
    macErrorMessage (LAY_INITLINE, WARNING, szError);
  }
}
#endif

/******************************************************************************
 * vdPrintForm
 *
 * DESCRIPTION:
 * print the form as defined in the *.lay files with the information about
 * the fields and states
 *
 *
 * PARAMETERS:
 * stLAYINF     *pLay           - layoutinformation
 *
 * RETURNS:
 * - void -
 ******************************************************************************
 */

void vdPrintForm (stLAYINF *pLay)
{
  int	item;
  int   iLay;
  UINT  uElem;
  state *paState;
  field *paField;
  field *paXFld;
  char  *pcDest;
  char  *pcSrc;
  char  szInfo[20];

  paState = pLay->paSta;
  paField = pLay->paFld;

  for (iLay = 0; paField[iLay].iIndex != -1; iLay++) {

    paXFld = &(paField[iLay]);

    /* insert the default text */
    vdAddElement (pLay, paXFld->iIndex, 1, NULL);

    /*
     * now replace the default text with the information about the field 
     * and the element (overwrite the default text for this purpose)
     */
    for (uElem = 0; uElem < paXFld->num; uElem++) {
      item = paXFld->item_list[uElem];
      if (item < 0)	item = 0;
      sprintf (szInfo, "@L%d-%d ",
               paXFld->iIndex,
               paState[item].iIndex);

      /* now copy the string as far as possible */
      pcDest = paState[item].info;
      pcSrc  = szInfo;

      if (pcDest != NULL) {
        while (*pcSrc != '\0') {
          if (*pcDest == '\0')          break;
          *pcDest++ = *pcSrc++;
        }
      }
    }
  }
}


/******************************************************************************
 * demo_sort_func
 *
 * DESCRIPTION:
 * 
 * For testing qsort - used by a demo qsort - type "man qsort" for details 
 * since here an array of pointers to argv like structures is sorted
 * we can adress the third row strings with *a[2] (1.Row would be *a[0] etc.)
 * the ugly (char***) cast is because qsort works with void pointers. 
 *
 * PARAMETERS: 
 *
 * RETURNS:
 * sort functions used by qsort return 0,-1 or 1 depending on the compare 
 * as strcmp does to as seen here.
 ******************************************************************************
 */
#if 0
static int demo_sort_func(const void *a,const void *b)
{ 
  /*printf("laymax: %s %s = %d\n",(*a)[2],(*b)[2],strcmp((*a)[2],(*b)[2]));*/
  return(strcmp((*(char ***)a)[2],(*(char***)b)[2])); 
} 
#endif
