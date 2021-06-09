typedef struct 
{
  char sachzDate[MAX_BUFFER];
  char sachzOperation[MAX_BUFFER];
  char sachzInvoice[MAX_BUFFER];
  char sachzState[MAX_BUFFER];
  char sachzPayment[MAX_BUFFER];
  char sachzSaldo[MAX_BUFFER];  
  struct s_group_30 *spstG30;
  
} tostTurnover;


int foiTurnoverItems_Count(struct s_TimmInter *lpstTimmInter);
void fovdTurnoverList_Init(struct s_TimmInter *lpstTimmInter);
void fovdTurnoverList_Free();
toenBool foenInterestList_Gen(struct s_TimmInter *spstTimmInter);







