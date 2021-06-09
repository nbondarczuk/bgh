typedef struct
{
  char sasnzNetShdes[MAX_BUFFER];
  char sasnzNetDes[MAX_BUFFER];
  char sasnzAccountNo[MAX_BUFFER];
  char sasnzName[MAX_BUFFER];
  char sasnzContractNo[MAX_BUFFER];
  char sasnzContractDate[MAX_BUFFER];
  char sasnzIMSI[MAX_BUFFER];
  char sasnzSIM[MAX_BUFFER];
}
tostSimItem;

toenBool foenSimItem_Init(int);
toenBool foenSimItem_Next(int, tostSimItem *);
toenBool foenSimItem_Close(); 

