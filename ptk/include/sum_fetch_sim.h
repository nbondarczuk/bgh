#if 0	/* just for version.sh */
static char *SCCS_VERSION = "1.1";
#endif

typedef enum toenRecordType 
{
  RECORD_USAGE   = 0, 
  RECORD_ACCESS  = 1, 
  RECORD_SERVICE = 2, 
  RECORD_OTHER   = 3

} toenRecordType;

typedef struct tostPayment 
{
  char sasnzMonetaryAmount[MAX_BUFFER];
  char sasnzCurrency      [MAX_BUFFER];
  
} tostPayment;

typedef struct 
{
  char sasnzAmount    [MAX_BUFFER];
  char sasnzEventPrice[MAX_BUFFER];
  char sasnzEventType [4];
  char sasnzEventsNo  [MAX_BUFFER];  
  
} tostEventInfo;

typedef struct tostContractData 
{
  char sachzContractNo[MAX_BUFFER];

}  tostContractData;

typedef struct tostCoInfo 
{
  char saszMarket    [MAX_BUFFER];
  char saszMarketCode[MAX_BUFFER];
  char saszCoNo      [MAX_BUFFER];
  char saszSmNo      [MAX_BUFFER];
  char saszDnNo      [MAX_BUFFER];
  char saszSumAmt    [MAX_BUFFER];
  char saszCurrency  [MAX_BUFFER];
  struct s_group_22 *g_22;
  struct s_group_99 *g_99;
  
} tostCoInfo;

toenBool foenFetchInvoiceDate(struct s_TimmInter *, char *, int);

toenBool foenFetchInvoiceCustomerAccountNo(struct s_TimmInter *, char *, int);

toenBool foenFetchInvoiceNo(struct s_TimmInter *, char *, int);

toenBool foenFetchInvoicePeriodBegin(struct s_TimmInter *, char *, int);

toenBool foenFetchInvoicePeriodEnd(struct s_TimmInter *, char *, int);

int foiCountSubscribers(struct s_TimmInter *);

int foiCountSubscriberContracts(struct s_TimmInter *, int);

toenBool foenFetchContractSim(struct s_TimmInter *, int, int, char *, int);

toenBool foenFetchContractMarket(struct s_TimmInter *, int, int, char *, int);

toenBool foenFetchContractNumber(struct s_TimmInter *, int, int, char *, int, int *);

toenBool foenFetchSimPayment(struct s_TimmInter *, char *, tostPayment *);

int foiCountSimItems(struct s_TimmInter *, toenRecordType, char *, int);

toenBool foenFetchSimRecord(struct s_TimmInter *, char *, int, toenRecordType, char *); 

int foiCountSimCallRecords(struct s_TimmInter *, char *); 

int foiCountSimHomeCallRecords(struct s_TimmInter *, char *, int); 

int foiCountSimRoamingCallRecords(struct s_TimmInter *, char *, int); 

int loiCountSims(struct s_TimmInter *);  

toenBool foenFetchContractData(struct s_TimmInter *, int, int, tostContractData *);

toenBool foenFetchAirTime(struct s_TimmInter *, char *, char *, int);

int foiCountSimCallRecordsType(struct s_TimmInter *, char *, int, char); 

toenBool foenFetchCoServPayment(struct s_TimmInter *ppstSumSheet, 
                                struct tostCoInfo *ppstCoiInfo,
                                enum toenItemType poenType, 
                                struct tostPayment *ppstPayment); 

toenBool foenFetchCoInfo(struct s_TimmInter *ppstSumSheet,
                         int poiSubscriberNo,
                         int poiContract,
                         struct tostCoInfo *ppstCoInfo);                         

struct s_group_22 *fpstFindCoServCat(struct s_group_22 *g_22,
                                     enum toenItemType poenType);

toenBool foenFetchContractInfo(struct s_group_22 *g_22, 
                               long *ppilCoId, 
                               double *ppflBCHSumUsage, 
                               double *ppflBCHSumSubscription,
                               double *ppflBCHSumAccess, 
                               double *ppflBCHSumOCC);
