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
 * Author(s):   A. Dreier
 *              B. Michler
 *
 * Changed  :
 * 24.05.96 	B.Michler  	added flag FOPA
 * 03.07.96 	B.Michler  	added header index in STFIELD
 *
 * Description:
 * The routines in this file read the layout description and build up
 * the arrays of structures needed by the layouter.
 *
 *******************************************************************************/

#ifndef _LAYOUT_H
#define _LAYOUT_H 1

#if 0 /* just for version.sh */
static char *SCCS_VERSION = "1.23";
#endif

#define USEFONTCACHE	1
#define NEWLINEHDL	1
#define MAXLINEELEM	500	/* ideally the correct number */


/*
 * Flags to control the layouter - if for example EVPA is set the unit 
 * is going to show up on every page. usage: See the lay[] array below
 */

/* static flags (never cleared), applied from layout */
#define EVPA	1	/* (on) EVery PAge                     	*/
#define LAPA	2   	/* (on) LAst  PAge                     	*/
#define FEED	4   	/* FORMfeed                            	*/
#define PAGE 	8   	/* End of PAGE (variable part) reached 	*/
#define ABSO	16  	/* ABSOlute coordinates               	*/
#define INDE	32  	/* INDEpendent                         	*/
#define GEND	64  	/* end of a group of lists             	*/
#define FOPA	128 	/* follow-up page (all pages but the 1.)*/
#define HEAD	256	/* element is a headline                */

/* dynamic flags, cleared after each printout */
#define CALC	1 	/* Already processed by make_pos       	*/
#define PRIN	2  	/* Already processed by make_output    	*/

/*
 * possible textattributes
 */
#define TEXTNORMAL    	0	/* no attributes  	*/
#define TEXTRIGHT     	1	/* aligned right  	*/
#define TEXTLEFT      	2	/* aligned left   	*/
#define TEXTCENTER    	3	/* centered text  	*/
#define TEXTUL        	8	/* underline text 	*/
#define TEXTPAGES    	16	/* insert nr. pages 	*/


/* This is the central struct to memorise all lists for the layout.
 * It's used in the "lay[]" array below - look at typedef "field".
 * put_next_line() - a function located in layout.c works
 * on this kind of structure.  Each call of put_next_line increases
 * the number of pointers attached at my_argvs by one and increases
 * count by one. Different numbers in blocksize should only change 
 * performance, not functionality (= How many pointers are allocated at once.)
 * A big number for blocksize should eventually increase the performance if
 * you put a lot of lines in this array. (ideal if blocksize = number of lines
 * - which you usually don't know). Don't worry too much - the processing seems
 * to be much faster than the parser anyway. 
 * Object is to search and filter information by some criteria in the build
 * up C struct from the parser - without yet thinking about sorting, how_manys, 
 * or layout questions.
 * You can call put_next_line for a specific s_line_array as often as you
 * want. The result will be a dynamically growing array of pointers to 
 * (layoutlines) which can be sorted very easily with (for example) qsort 
 * (even more than once if needed). It is now also possible to calculate 
 * the space needed by the variable lists or to determine which part still
 * fits on the page, since the number of lines is now known for every
 * layoutunit.   
 */ 

typedef struct {
  UINT	flags;			/* several flags 			*/
  UINT 	count;			/* nr of lines allocated 		*/
  UINT 	block_size;		/* nr of lines to allocate en block 	*/
  char 	***my_argvs;		/* actual pointer 			*/
} s_line_array;
/* definitions for flags */
#define LAFLAG_SET	1	/* element was once set */

/* Struct where info about simple items (textsize, alignment, texttype) is
 * stored - or in later versions loaded into. There is one array of
 * this kind of struct in this source:"s[]" were all "low_level" layout
 * information is stored. This array "state[]" is referenced by the "lay[]"  
 * array (array of the next struct type "field").
 */

typedef struct STSTATE { 
                 int iIndex;        /* index of this element           */
                 int x;             /* X position relativ to field x   */ 
                 int y;             /* y displacement (unused)         */
                 int dist_y;        /* distance of item to preccessor  */
#if USEFONTCACHE
                 int iFontIndex;    /* index to caching table          */
#else
                 char font[32];     /* Fontname */
#endif
                 int size;          /* Character size                  */
                 char *info;        /* Can hold default-dummy or fixed */
                                    /* information.                    */
                 int pos_flag;      /* For right,left,center alignment */
               } state;

/* Struct where all info about the complex layoutunits is stored (In an array
 * of this struct "= lay[]".)
 * item_list holds the references to the above "simple items" array.
 * here you can also find one of the above mentioned s_line_arrays for       
 * every layoutunit.
 */
/* max. number of elements belonging to a line type */
#define NRELINLINE	16

typedef struct STFIELD { 
  int 	iIndex;			/* index of this element               	*/
  int 	x;			/* Xposition of field or line or item  	*/
  int 	y;			/* Yposition of field or line or item  	*/
  int 	max_dist;		/* distance of field or line from prec 	*/
  UINT 	num;			/* Number of items a line has         	*/
  int 	item_list[NRELINLINE];	/* Which items are part of line        	*/
#if NEWLINEHDL
  s_line_array *pstLine;	/* pointer to complex line or field	*/
#else
  s_line_array st_line;		/*complex= line or field is stored here	*/
#endif
  int 	iaGrFrom[5];		/* related graphic elements        	*/
  int 	iaGrTo[5];		/* related graphic elements        	*/
  int 	flags;			/* see flags above         		*/
  int 	fDyn;			/* dynamic flags           		*/
  int 	iAddSpace;		/* additional offset to pageend,        */
				/* prevent single headline at pageend   */
  UINT 	akt_pos;		/* aktual line processed                */
  int 	max_size;		/* maximum charaktersize in the line    */
  int 	max_elem_dist;		/* maximum offset in the line           */
  int	iIndHead;		/* Index of header for this element     */
} field;

/* Struct where all info about graphic operations is stored .                
 *
 * which are so far under and overlining of layoutunits (!not items - which
 * is handled with a flag in the s[] array), surround something with an 
 * rectangle, and underlay something with a greytone.  
 * you can alternatively give absolute coordinates for the y coordinates
 * or use the precalculated coordinates and size of one or more layoutunits 
 * to determine the rectangle coordinates - thats important if graphic depends
 * on size of the table. 
 * ! There is no adaption to any x coordinates implemented - they always have
 * to be absolute.
 * An entrance of {10,0,0,300,19,19,G_RECT,50,5,0} would draw a rectangle
 * around the y space (height) used up by layoutunit 19. This even works
 * if the layoutunit covers more than one page, in which case more than one
 * rectangle is drawn (1 on every page with something from unit 19).
 * The 50 is the greylevel used (0-100), 5 is an additional y distance, the
 * rectangle has the x coordintes 10 to 310 (no matter if that really covers
 * all thats produced by unit19)
 */ 
#define O_LINE	1	/* overline  */ 
#define U_LINE	2   	/* underline */
#define G_RECT	3   	/* draw rectangle */
#define G_FILL	4    	/* draw filled rectangle */
#define G_PICT  5	/* draw picture */

typedef struct STGRAPH { 
                 int xPrint;
                 int yPrint;
                 int x;
                 int y;       /* if lay_unit_from or ..to are set these two */
                 int height;  /* are calculated                             */
                 int width;
                 int from_lay;
                 int to_lay; 
                 int type;
                 int black; 
                 int dist;
                 int fAbs;	/* flag for absolute positioning */
                 char *szPicName; /* pointer to string for pictures */
               } st_graphic;

/* information about the layout */
typedef struct {
  state         *paSta;         /* pointer to state array               */
  field         *paFld;         /* pointer to field array               */
  st_graphic    *paGra;         /* pointer to graphics array            */
  int           *paFInd;        /* pointer to index table               */
  int           iFIndMax;       /* maximum element of paFInd            */
  int           iVEndFirst;     /* end of var. list on first page       */
  int           iVEndMiddle;    /* end of var. list on middle pages     */
  int           iVEndLast;      /* end of var. list on last page        */
  int           iPageHeight;    /* height of page in point              */
} stLAYINF;

/* structure for font-array */
typedef struct {
  UINT		size;		/* size of the array */
  UINT		next;		/* next element in the array */
  char		*font;		/* array of font-texts */
} FONTARRAY;
#define NFONTSIZE	64
#define NALLOCFONT	10

/* ---- Prototypes ---- */ 
int put_next_line(s_line_array *p_so,char **argv);
int put_next_lay(field *, int, char **argv);


#endif
