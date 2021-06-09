/**************************************************************************/
/*  MODULE : String conversion utilities                                  */
/*                                                                        */
/*  AUTHOR : N.Bondarczuk                                                 */
/*                                                                        */
/*  CREATION DATE : 22.10.97                                              */
/*                                                                        */
/*  DESCRIPTION : Exports  function  converting strings                   */
/*                                                                        */
/**************************************************************************/
#if 0	/* just for version.sh */
static char *SCCS_VERSION = "4.1";
#endif

#define YYYY_MM_DD 102
#define YY_MM_DD 101

void fovdFormatMoney(char *);
void fovdFormatDate(char *, int);
void fovdFormatCallsNumber(char *);
void fovdFormatSimsNumber(char *);
void fovdFormatInvoiceNumber(char *);
void fovdFormatNumber(char *);
void fovdFormatCallDate(char *);
void fovdFormatCallTime(char *);
char *fpchzTranslate(char *, char *, char *);
char *fpchzFormatCity(char *);
char *fpchzFormatZip(char *);
void fovdRoundMoney(char *, double);
void fovdStr_Clean(char *);
double foflRound(double poflAmount);
