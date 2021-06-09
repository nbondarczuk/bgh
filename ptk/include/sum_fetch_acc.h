/**************************************************************************/
/*  MODULE : SUM SHEET data fetecher                                      */
/*                                                                        */
/*  AUTHOR : N.Bondarczuk                                                 */
/*                                                                        */
/*  CREATION DATE : 22.10.97                                              */
/*                                                                        */
/*  DESCRIPTION : Exports  functions accessing information from TIMM      */
/*                SUM SHEET message.                                      */ 
/**************************************************************************/

#if 0	/* just for version.sh */
static char *SCCS_VERSION = "4.1";
#endif

/*
 * EXPORT LIST
 */

int fetchSubscribersNumber(struct s_TimmInter *);
int fetchSubscriberCustomerCode(struct s_TimmInter *, int, char *, int);
int findSubscriberContractsNumber(struct s_TimmInter *,  int);
int fetchSubscriberContractsNumber(struct s_TimmInter *, char *, int);

int fetchContractId( struct s_TimmInter *, int, int, char *, int);
int fetchContractMarket(struct s_TimmInter *, int, int, char *, int, char *, int);
int fetchContractStorageMedium(struct s_TimmInter *, int, int, char *, int);
int fetchContractDirectoryNumber(struct s_TimmInter *, int, int, char *, int);
int fetchContractAmount(struct s_TimmInter *, int, int, char *, int, char *, int);

int fetchAccountAccessPayment( struct s_TimmInter *,  char *,  int,  char *,  int);
int fetchAccountUsagePayment( struct s_TimmInter *,  char *,  int,  char *,  int);
int fetchAccountSubscriptionPayment( struct s_TimmInter *sum_ti,  char *,  int,  char *,  int);
int fetchAccountOthersPayment( struct s_TimmInter *,  char *,  int,  char *,  int);
int fetchAccountSummaryPayment( struct s_TimmInter *,  char *,  int,  char *,  int);

int fetchSubscriberAccountPayment(struct s_TimmInter *, int, char *, int, char *, int, char *, int, char *, int);
