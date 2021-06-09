/***************************************************************************** 
 * Filename:       parval.h
 *
 * Facility:       ParVal interface
 * 
 * Description:    Creation of macros from WorkParVal interface table
 *
 * Author & Date:  Leszek Dabrowski, 20/01/2000
 *
 * Modifications:
 *
 * Date       Engineer   Description
 * ---------------------------------------------------------------------------
 * 27/01/2000 NB         Initial release 
 * ---------------------------------------------------------------------------
 *
 * COPYRIGHT (c) 2000 by EDS PERSONAL COMMUNICATIONS CORPORATION
 * 
 * This software is furnished under a licence and may be used and copied only
 * in accordance with the terms of such licence and with the inclusion of the
 * above copyright notice. This software or any other copies there of may not
 * be provided or otherwise made available to to any other person. No title to
 * and ownership of the software is hereby transfered. The information in this
 * software is subject to change without notice and should not be construed as 
 * a commitment by EDS. 
 *****************************************************************************
 */

/* common section */
#define PARVALCODE_LEN 23   
#define PARVALTEXT_LEN 81
#define DATE_LEN 9

/*
 * Copy of the record's relevant part in DB WORKPARVAL table
 */

typedef struct tostWParValRec_tag
{
  char soszCustomerId[PARVALCODE_LEN];
  char soszCoId[PARVALCODE_LEN];
  char soszWParCode[PARVALCODE_LEN];
  char soszWParValFinishDate[DATE_LEN];
  char soszWParVal[PARVALTEXT_LEN];
  char soszWParField1[PARVALTEXT_LEN];
  char soszWParField2[PARVALTEXT_LEN];
  char soszWParField3[PARVALTEXT_LEN];
  char soszWParField4[PARVALTEXT_LEN];
  char soszWParField5[PARVALTEXT_LEN];
} tostWParValRec;

/*
 * DB Record list returned from the opened cursor
 */

typedef struct tostWParValRecSeq_tag
{
  struct tostWParValRec_tag *spstRec;
  struct tostWParValRecSeq_tag *spstNext; 
} tostWParValRecSeq;



/* module interface part */

int foiParVal_NextDoc(stTIMM *ppstTimm);

int foiParVal_SimGen(stTIMM *ppstTimm, int poiCoId, char *ppszDn);

int foiParVal_AccGen(stTIMM *ppstTimm);

#ifdef _PARVAL_


/* module local part */


typedef enum{PARVAL_NOT_FOUND, PARVAL_FOUND} toenParVal;



typedef struct tostWParValList_tag
{
    struct tostWParValRecSeq_tag *spstFirst;
    struct tostWParValRecSeq_tag *spstLast;
} tostWParValList;

typedef struct tostWPTreeNode_tag
{
    struct tostWPTreeNode_tag *spstLeft;
    struct tostWPTreeNode_tag *spstRight;
    void *spstNodeData;            /* the cust of: tostWPCustomerContractData* 
                                      for the customer-contract tree,
                                       tostWPTreeNode* for the date tree     */
    char *spchLabel;              /* Date for the date tree,
				     the contcatenation of the Customrer_id, 
				     the space, and the Co_id for the custome-contract tree */
}tostWPTreeNode;


toenParVal foenFindNodeLabel(tostWPTreeNode **ppstRootNode, char *poszLabel, tostWPTreeNode ***ppstFoundNode);
void fovdTreeListPreorder(tostWPTreeNode *ppstRootNode);
void fovdTreeListInorder(tostWPTreeNode *ppstRootNode, char pochMarker);
void fovdDoubleTreeListInorder(tostWPTreeNode *ppstRootNode, char pochMarker);
tostWPTreeNode *fpstTreeNode_New(char *poszLabel, int poiLabelLen);
void fovdPrintNestingLevel(void);
int foiTreeNode_Add(tostWPTreeNode **ppstRootNode, char *poszLabel, tostWPTreeNode ***ppstFoundNode);

#if 0 /* for testing */
int foiWorkParVal_LoadPart(char *poszFinishDate, tostWParValRecSeq **ppstSeqFirst);
#endif

void fovdParValSeq_Print(tostWParValRecSeq *ppstWPSecFirst);
void fovdParValSeq_Free(tostWParValRecSeq **ppstWPSecFirst);
int foiParValDate_Next(char *poszEndDate);
tostWParValList *fpstParValList_New(void);
tostWParValList *fpstParVal_Find(char *poszDate, char *poszCustomerId, char *poszCoId);
void fovdParValRec_Get(tostWParValRec *ppstDestin, tostWParValRec *ppstSource);
void fovdParValMacro_Print(tostWParValRecSeq *ppstWPSeqFirst, char *poszMacroName, char *poszLastParameter);
void fovdDate2Date8(char *poszDate6, char *poszDate8);

#endif
