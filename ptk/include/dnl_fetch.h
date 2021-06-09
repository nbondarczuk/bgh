/**************************************************************************/
/*  MODULE : DUNNING data fetecher                                        */
/*                                                                        */
/*  AUTHOR : N.Bondarczuk                                                 */
/*                                                                        */
/*  CREATION DATE : 25.11.97                                              */
/*                                                                        */
/*  DESCRIPTION : Exports  functions accessing information from TIMM      */
/*                DUNNING message.                                        */ 
/**************************************************************************/

#if 0	/* just for version.sh */
static char *SCCS_VERSION = "4.1";
#endif

int foiCountWelcomeLetterTexts(struct s_TimmInter *);
toenBool foenFetchDunningLetterText(struct s_TimmInter *, int, char pachzLine[5][MAX_BUFFER]);
toenBool foenFetchDunningLetterCustomerAccountNo(struct s_TimmInter *, char *, int);
toenBool foenFetchDunningLetterTerms(struct s_TimmInter *, char *, int, char *, int);
toenBool foenFetchDunningLetterLetterNo(struct s_TimmInter *, char *, int);
toenBool foenFetchDunningLetterInvoiceNo(struct s_TimmInter *, char *, int);
toenBool foenFetchDunningLetterDate(struct s_TimmInter *, char *, int);
toenBool foenFetchDunningLetterDueDate(struct s_TimmInter *, char *, int);
toenBool foenFetchDunningLevel(struct s_TimmInter *, char *, int);
