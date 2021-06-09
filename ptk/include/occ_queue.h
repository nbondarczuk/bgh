typedef struct tostOCCNode
{
  tostInvItem *spstInvItem;
  struct tostOCCNode *spstNext;

} tostOCCNode;

typedef struct tostOCCQueue
{
  tostOCCNode *spstRoot;

} tostOCCQueue;

toenBool foenOCCQueue_Init(tostOCCQueue *);
toenBool foenOCCQueue_Delete(tostOCCQueue *);
toenBool foenOCCQueue_Enqueue(tostOCCQueue *, tostInvItem *);
tostInvItem *fpstOCCQueue_Dequeue(tostOCCQueue *);
