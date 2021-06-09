/**************************************************************************/
/*  MODULE : WELCOME data fetecher                                        */
/*                                                                        */
/*  AUTHOR : N.Bondarczuk                                                 */
/*                                                                        */
/*  CREATION DATE : 25.11.97                                              */
/*                                                                        */
/*  DESCRIPTION : Exports  functions accessing information from TIMM      */
/*                WELCOME   message.                                      */ 
/**************************************************************************/
#if 0	/* just for version.sh */
static char *SCCS_VERSION = "4.1";
#endif

int foiCountWelcomeLetterTexts(struct s_TimmInter *);
toenBool foenFetchWellcomeLetterText(struct s_TimmInter *, int, char pachzLine[5][MAX_BUFFER]);
toenBool foenFetchWelcomeLetterCustomerAccountNo(struct s_TimmInter *, char *, int);
toenBool foenFetchWelcomeLetterSellerNIP(struct s_TimmInter *ppstWelcomeLetter, char *pachzNIP, int poiNIPLen);
