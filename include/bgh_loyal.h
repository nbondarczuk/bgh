typedef struct tostLoyalCoServKey
{
  long solAmtTypeId;
  char saszSubType[6];
  char saszLin7140[36];
  char saszPia7140[36];
  char saszDes[96];

} tostLoyalCoServKey;

typedef struct tostLoyalCoServKeySeq
{
  struct tostLoyalCoServKey *spstKey;
  struct tostLoyalCoServKeySeq *spstNext;

} tostLoyalCoServKeySeq;

typedef struct tostLoyalCoServVal
{
  double soflNetAmt;
  double soflTaxAmt;

} tostLoyalCoServVal;

typedef struct tostLoyalCoServ
{
  struct tostLoyalCoServKey *spstKey;
  struct tostLoyalCoServVal sostVal;

} tostLoyalCoServ;

typedef struct tostLoyalCoServNode
{
  struct tostLoyalCoServ *spstServ;
  struct tostLoyalCoServNode *spstLeft;
  struct tostLoyalCoServNode *spstRight;
  
} tostLoyalCoServNode;

typedef struct tostLoyalCoServTree
{
  struct tostLoyalCoServVal sostSumVal;
  struct tostLoyalCoServNode *spstRoot;
  
} tostLoyalCoServTree;

typedef struct tostLoyalCoInfo
{
  long solLoyalCoId;
  long solCoId;
  char *spszDnNum;
  struct tostLoyalCoServTree sostCoServTree;
  
} tostLoyalCoInfo;

typedef struct tostLoyalCoNode
{
  struct tostLoyalCoInfo sostCoInfo;
  struct tostLoyalCoNode *spstLeft;
  struct tostLoyalCoNode *spstRight;
  
} tostLoyalCoNode;

typedef struct tostLoyalInvInfo
{
  long solLoyalHdrId;
  long solCustId;
  char *spszCustCode;
  long solOhxact;
  char *spszOhRefNum;
  char *spszOhRefDate;
  struct tostLoyalCoServVal sostDis;  
  
} tostLoyalInvInfo;

typedef struct tostLoyalCoTree
{
  struct tostLoyalInvInfo sostInvInfo;
  struct tostLoyalCoNode *spstRoot;
  
} tostLoyalCoTree;

#define LEN_ANYSTRING 64

int foiBghLoyalHdr_Insert(struct tostLoyalInvInfo *ppstInfo);

int foiEdsBghLoyalCo_Insert(struct tostLoyalInvInfo *ppstInvInfo,
                            struct tostLoyalCoInfo *ppstCoInfo);

int foiEdsBghLoyalTrl_SumInsert(struct tostLoyalInvInfo *ppstInvInfo,
                                struct tostLoyalCoInfo *ppstCoInfo,
                                int polTypeId);

int foiEdsBghLoyalTrl_ServInsert(struct tostLoyalInvInfo *ppstInvInfo,
                                 struct tostLoyalCoInfo *ppstCoInfo,
                                 struct tostLoyalCoServ *ppstServ);

int foiEdsBghLoyalChargeType_Load( struct tostLoyalCoServKeySeq **ppstSeq);

int foiEdsBghLoyal_Clean(struct tostLoyalInvInfo *ppstInfo);
