/**************************************************************************/
/*  MODULE : Invoice Item Handler                                         */
/*                                                                        */
/*  AUTHOR : N.Bondarczuk                                                 */
/*                                                                        */
/*  CREATION DATE : 22.04.98                                              */
/*                                                                        */
/*  DESCRIPTION : Defines type used for printing TIMM contents            */
/*                                                                        */
/**************************************************************************/


#if 0	/* just for version.sh */
static char *SCCS_VERSION = "4.0";
#endif

typedef struct 
{
  char sasnzType[MAX_BUFFER];
  char sasnzTM[MAX_BUFFER];
  char sasnzSN[MAX_BUFFER];
  char sasnzQuantity[MAX_BUFFER];
  char sasnzPrice[MAX_BUFFER];
  char sasnzAccessType[MAX_BUFFER];
  char sasnzAccessSubtype[MAX_BUFFER];
  char sasnzUsageType[MAX_BUFFER];
  char sasnzTT[MAX_BUFFER];
  char sasnzTZ[MAX_BUFFER];
  char sasnzFE[MAX_BUFFER];
  char sasnzVPLMN[MAX_BUFFER];
  char sasnzNetAmount[MAX_BUFFER];
  char sasnzTaxName[MAX_BUFFER];
  char sasnzTaxAmount[MAX_BUFFER];
  char sasnzBrutAmount[MAX_BUFFER];
}
tostInvItem; 
