#define OTNAME_MAX_LEN 100
#define OTGL_MAX_LEN   30

typedef struct tostOtEntry
{
  int soiOtSeq;                     /* load from ORDERTRAILER */
  char saszOtName  [OTNAME_MAX_LEN];/* load from ORDERTRAILER, recorde */  
  double soflOtNet;                 /* get from EDI */
  double soflOtDis;                 /* get from EDI */
  double soflOtGross;               /* load from ORDERTRAILER = OTMERCH + OTEXTTAX */
  
} tostOtEntry;

typedef struct tostOtEntrySeq
{
  struct tostOtEntry sostOtEntry;
  struct tostOtEntrySeq *spstNext;
  
} tostOtEntrySeq;

#ifdef _BGH_TRANSX_

#define DISC_ATTR_LABEL "DISC"

#define ARRAY_LEN 1024

typedef struct O_ordertrailer_arr
{
  long O_otseq     [ARRAY_LEN];
  char O_otname    [ARRAY_LEN][100];  
  double O_otmerch [ARRAY_LEN];
  double O_otexttax[ARRAY_LEN];

} O_ordertrailer_arr;

typedef enum toenServMapType
{
  UNDEFTYPE = -1,
  TM = 0,
  SP = 1,
  SN = 2,
  PL = 3,
  TT = 4,
  ZN = 5

} toenServMapType;

typedef struct tostServMapTree
{
  char *spszShdes;
  int soiCode;
  struct tostServMapTree *spstLeft;
  struct tostServMapTree *spstRight;

} tostServMapTree;

#define TERM_TAB_LEN 16

#else

int foiTransx_RoundOHEntry(double poflTotalBrutAmount, 
                           double poflBCH_TotalAmount);

int foiTransx_GetOtEntries(struct tostOtEntrySeq **pppstOtEntrySeq,
                           stTIMM *ppstInvTimm);

#endif
