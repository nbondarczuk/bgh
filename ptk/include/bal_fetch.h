/**************************************************************************/
/*  MODULE : BALANCE data fetecher                                        */
/*                                                                        */
/*  AUTHOR : N.Bondarczuk                                                 */
/*                                                                        */
/*  CREATION DATE : 22.10.97                                              */
/*                                                                        */
/*  DESCRIPTION : Exports  functions accessing information from TIMM      */
/*                BAL SHEET message.                                      */ 
/**************************************************************************/

#include "gen.h"

#if 0	/* just for version.sh */
static char *SCCS_VERSION = "4.1";
#endif

/*
 * EXPORT LIST
 */

int fetchBalanceTransactionsNumber(IN struct s_TimmInter *);
int fetchBalanceTransactionType(IN struct s_TimmInter *, IN int, OUT char *, IN int);
int fetchBalanceTransactionDate(IN struct s_TimmInter *, IN int, OUT char *, IN int);
int fetchBalanceTransactionCurrency(IN struct s_TimmInter *, IN int, OUT char *, IN int);
int fetchBalanceTransactionAmount(IN struct s_TimmInter *, IN int, OUT char *, IN int);

int fetchSubscriberAccessPayment(IN struct s_TimmInter *, OUT char *, IN int, OUT char *, IN int);
int fetchSubscriberUsagePayment(IN struct s_TimmInter *, OUT char *, IN int, OUT char *, IN int);
int fetchSubscriberSubscriptionPayment(IN struct s_TimmInter *, OUT char *, IN int, OUT char *, IN int);
int fetchSubscriberOthersPayment(IN struct s_TimmInter *, OUT char *, IN int, OUT char *, IN int);
int fetchSubscriberSummaryPayment(IN struct s_TimmInter *, OUT char *, IN int, OUT char *, IN int);
int fetchAccountActualSaldo(IN struct s_TimmInter *, OUT char *, IN int, OUT char *, IN int);
int fetchAccountPreviousSaldo(IN struct s_TimmInter *, OUT char *, IN int, OUT char *, IN int);
int fetchBalanceInvoicePayment(struct s_TimmInter, char *, int, char *, int);

int fetchBalanceTransactionInvoice(IN struct s_TimmInter *, IN int, OUT char *, IN int);

