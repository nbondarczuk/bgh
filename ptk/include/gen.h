/**************************************************************************/
/*  MODULE : data generator                                               */
/*                                                                        */
/*  AUTHOR : N.Bondarczuk                                                 */
/*                                                                        */
/*  CREATION DATE : 25.10.97                                              */
/*                                                                        */
/*  DESCRIPTION : Generates output                                        */
/*                                                                        */
/**************************************************************************/

#include <stdio.h>

#include "bgh.h"

#if 0	/* just for version.sh */
static char *SCCS_VERSION = "4.1";
#endif


#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#ifndef EQ
#define EQ(a, b) !strcmp(a, b)
#endif

#ifndef NOT
#define NOT(a) ((a) == FALSE)
#endif

typedef enum {START, NEXTSET, NEXTDOCCOPY, NEXTDOCNOCOPY, STOP} toenOutputState;

#define MAX_LINE 256
#define MAX_FILE 256

void fovdGen();
void fovdGenItb();

