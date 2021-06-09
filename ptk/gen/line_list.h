#include <sys/uio.h>
#include <limits.h>

typedef struct tostIOVecNode
{
  int soiFreeSlot;
  struct iovec sastIOVecArr[IOV_MAX];
  struct tostIOVecNode *spstNext;

} tostIOVecNode;

typedef struct tostIOVecList
{
  int soiLen;
  struct tostIOVecNode *spstFirst, *spstLast;

} tostIOVecList;


typedef struct tostBuf
{
  int soiLen;
  char *spsnVal;
  
} tostBuf;

typedef struct tostLineNode 
{
  struct tostBuf *spstBuf;
  struct tostLineNode *spstNext;
  
} tostLineNode;

typedef struct tostLineList
{
  int soiLen;
  int soiMemoryUsage;
  tostLineNode *spstFirst, *spstLast;
  struct tostIOVecList *spstIOVecList;

} tostLineList;

tostLineList *fpstLineList_New();
toenBool foenLineList_Init(tostLineList *ppstList);
toenBool foenLineList_Write(tostLineList *ppstList, int pofilFile);
toenBool foenLineList_Delete(struct tostLineList *ppstList);
toenBool foenLineList_Append(tostLineList *ppstList, char *ppsnzLine);

