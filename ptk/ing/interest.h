typedef struct 
{
  toenBool soenIntSeqEnd;
  char sachzDate[MAX_BUFFER];
  char sachzOperation[MAX_BUFFER];
  char sachzState[MAX_BUFFER];
  char sachzPayment[MAX_BUFFER];
  char sachzSaldo[MAX_BUFFER];
  char sachzInterestSaldo[MAX_BUFFER];
  char sachzInterest[MAX_BUFFER];
  char sachzDelay[MAX_BUFFER];

} tostInterest;

int foiInterestItems_Count(struct s_TimmInter *lpstTimmInter);
void fovdInterestList_Init(struct s_TimmInter *lpstTimmInter);
void fovdInterestList_Free();
toenBool foenInterestList_Gen(struct s_TimmInter *spstTimmInter);



