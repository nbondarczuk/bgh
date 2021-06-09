/***************************************************************************** 
 * Filename:      workparval.c 
 *
 * Facility:       WorkParVal table handler
 * 
 * Description:    Creation of macros from WorkParVal interface table
 *
 * Author & Date:  Leszek Dabrowski, 20/01/2000

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

#define _PARVAL_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "bgh.h"

#include "protos.h"
#include "parser.h"
#include "types.h"
#include "gen.h"

#include "workparval.h"


/*
 * external variables
 */

extern stBGHGLOB stBgh;		         /* structure with globals for BGH */

/*
 * external funcions
 */

extern int fetchInvoicePeriodEnd(struct s_TimmInter *inter, 
				 char *buffer, 
				 int buffer_len); 

extern int fetchInvoiceCustomerAccountNo(struct s_TimmInter *inter, 
					 char *buffer, 
					 int buffer_len); 

extern int  foiWorkParVal_LoadPart(char *poszFinishDate, tostWParValRecSeq **ppstSeqFirst);


static int doiNestingLevel = 0;
static int doiDNestingLevel = 0;
static int doiSearchCount = 0;
static tostWPTreeNode *dpstRoot=NULL;


#if 0 /* for testing */
#define fovdPrintLog fprintf
#define LOG_DEBUG stdout
#define LOG_NORMAL stdout
#define NDEBUG 1

FILE *we = NULL;

main(int argc, char *argv[])
{
    char loszLabel[DATE_LEN], loszCustId[PARVALCODE_LEN], loszCoId[PARVALCODE_LEN],
	 loszCustCode[PARVALCODE_LEN], loszDN[PARVALCODE_LEN] ;
    tostWPTreeNode *lpstRoot=NULL, **lpstTempNode;
    toenParVal eRet;
    int i,n, iRet;
    tostWParValRecSeq *lpstWPSeqFirst = NULL;
    tostWParValList *lpstParValList;

    printf("data %s \n", argv[argc-1]);
    
    if( argv[argc-1] != '\0')
    {
	do
	{
	    printf("podaj date\n");
	    scanf("%s",loszLabel);
	    iRet = foiParValDate_Next(loszLabel);
/*
	    iRet = foiParValDate_Next( argv[argc-1]);
*/
	    printf("iRet %d \n",iRet);
	    printf("\nInorder\n");
	    fovdTreeListInorder(dpstRoot, 'D');
	    printf("\nDoubleInorder\n");
	    fovdDoubleTreeListInorder(dpstRoot, 'D');
	}while(*loszLabel != 'k');
	do
	{
	    printf("podaj date, CustId, CoId (cxxxx), CustCode, DN\n");
	    scanf("%s %s %s %s %s",loszLabel, loszCustId, loszCoId, loszCustCode, loszDN);
	    if(NULL != (lpstParValList = fpstParVal_Find(loszLabel, loszCustId, loszCoId+1) ) )
	    {
		if('\0' == *(loszCoId+1))
		{
		    fovdParValMacro_Print(lpstParValList->spstFirst, "AccountWorkParVal", loszCustCode);
		}
		else
		{
		    fovdParValMacro_Print(lpstParValList->spstFirst, "SimWorkParVal", loszDN);
		}
	    }
	    else
	    {
		fovdPrintLog(LOG_DEBUG, "No data - no macro was generated\n");
	    }
	}while(*loszLabel != 'k');
	exit(0);
    }
}

/*****************************************************************************
 *
 * Function name:	foiWorkParVal_LoadPart
 *
 * Function call:	rc = foiWorkParVal_LoadPart(loszFinishDate, lpstSeqFirst)
 *
 * Returns:	0  if no data found in the file
 *              1  if data was loaded
 *              -1  if failure
 *
 *
 * Arguments:	char *poszFinishDate - the data for this date only will be loaded
 *
 *              tostWParValRecSeq **ppstSeqFirst - begining of the record sequence
 *
 * Description: 
 *
 *****************************************************************************
 */
int  foiWorkParVal_LoadPart(char *poszFinishDate, tostWParValRecSeq **ppstSeqFirst)
{
    tostWParValRec  lostCurrentRec, *lpstTmpRec;
    tostWParValRecSeq *lpstSeqFirst = NULL, **lpstSeqLast, *lpstSeqTmp;
    FILE *we;
    
    if(NULL == (we = fopen("parvalful.txt","r")))
    {
	printf("nie moge otworzyc pliku we\n");
	return -1;
    }

    *ppstSeqFirst = NULL;
    
    while(fscanf(we,"%s %s %s %s %s %s %s %s %s %s\n",
		 lostCurrentRec.soszWParCode,
		 lostCurrentRec.soszCustomerId,
		 lostCurrentRec.soszCoId,
		 lostCurrentRec.soszWParValFinishDate,
		 lostCurrentRec.soszWParVal,
		 lostCurrentRec.soszWParField1,
		 lostCurrentRec.soszWParField2,
		 lostCurrentRec.soszWParField3,
		 lostCurrentRec.soszWParField4,
		 lostCurrentRec.soszWParField5) == 10)
    {
	if(!strcmp(poszFinishDate,lostCurrentRec.soszWParValFinishDate))
	{
	    printf("%s %s %s %s %s %s %s %s %s %s\n",
		   lostCurrentRec.soszWParCode,
		   lostCurrentRec.soszCustomerId,
		   lostCurrentRec.soszCoId,
		   lostCurrentRec.soszWParValFinishDate,
		   lostCurrentRec.soszWParVal,
		   lostCurrentRec.soszWParField1,
		   lostCurrentRec.soszWParField2,
		   lostCurrentRec.soszWParField3,
		   lostCurrentRec.soszWParField4,
		   lostCurrentRec.soszWParField5);
	    if(NULL == (lpstSeqTmp=malloc(sizeof(tostWParValRecSeq))))
	    {
		fovdPrintLog(LOG_NORMAL, "foiWorkParVal_LoadPart: I can not get memory for the node\n");
		return -1;
	    } 
	    lpstSeqTmp->spstNext = NULL;

	    if(NULL == (lpstTmpRec=malloc(sizeof(tostWParValRec))))
	    {
		fovdPrintLog(LOG_NORMAL, "foiWorkParVal_LoadPart: I can not get memory for the record\n");
		return -1;
	    } 
	    strcpy(lpstTmpRec->soszWParCode, lostCurrentRec.soszWParCode);
	    strcpy(lpstTmpRec->soszCustomerId, lostCurrentRec.soszCustomerId);
	    strcpy(lpstTmpRec->soszCoId, lostCurrentRec.soszCoId+1);
	    strcpy(lpstTmpRec->soszWParValFinishDate, lostCurrentRec.soszWParValFinishDate);
	    strcpy(lpstTmpRec->soszWParVal, lostCurrentRec.soszWParVal);
	    strcpy(lpstTmpRec->soszWParField1, lostCurrentRec.soszWParField1);
	    strcpy(lpstTmpRec->soszWParField2, lostCurrentRec.soszWParField2);
	    strcpy(lpstTmpRec->soszWParField3, lostCurrentRec.soszWParField3);
 	    strcpy(lpstTmpRec->soszWParField4, lostCurrentRec.soszWParField4);
	    strcpy(lpstTmpRec->soszWParField5, lostCurrentRec.soszWParField5);
	    lpstSeqTmp->spstRec = lpstTmpRec;

	    if(*ppstSeqFirst == NULL)
	    {
		*ppstSeqFirst = lpstSeqTmp;
	    }
	    else
	    {
		*lpstSeqLast = lpstSeqTmp;
	    }
            lpstSeqLast = &(lpstSeqTmp->spstNext); 
	}
    }
    fseek(we,0,SEEK_SET);
    return  (NULL == *ppstSeqFirst) ? 0 : 1 ;
}
#endif  /* for testing */

/*****************************************************************************
 *
 * Function name:	foenFindNodeLabel
 *
 * Function call:	rc = foenFindNodeLabel(lpstRootNode, loszLabel, lpstFoundNode)
 *
 * Returns:	 enum{PARVAL_NOT_FOUND, PARVAL_FOUND} toenParVal
 *
 *
 * Arguments:	ppstRootNode, char *poszLabel - node label, tostWPTreeNode ***ppstFoundNode
 *
 * Description: 
 *
 *****************************************************************************
 */

toenParVal foenFindNodeLabel(tostWPTreeNode **ppstRootNode, char *poszLabel, tostWPTreeNode ***ppstFoundNode)
{
    int rc;
    
    ASSERT("foenFindNodeLabel, tree must not be empty" && ppstRootNode != NULL );
    doiSearchCount++;
    if(NULL == *ppstRootNode)
    {
        *ppstFoundNode = ppstRootNode;
        return PARVAL_NOT_FOUND;
    }
    
    if(0 == (rc = strcmp(poszLabel,(*ppstRootNode)->spchLabel)))
    {
        *ppstFoundNode = ppstRootNode;
        return PARVAL_FOUND;
    }
    if(0 > rc)
    {
        return  foenFindNodeLabel(&(*ppstRootNode)->spstLeft, poszLabel, ppstFoundNode);
    }
    else
    {
        return  foenFindNodeLabel(&(*ppstRootNode)->spstRight, poszLabel, ppstFoundNode);
    }
}



void fovdTreeListInorder(tostWPTreeNode *ppstRootNode, char pochMarker)
{
    if(NULL == ppstRootNode)
    {
        fovdPrintLog(LOG_DEBUG, "Mark %c, NULL tree \n", pochMarker);
        return;
    }

    doiNestingLevel++;
    if(NULL != ppstRootNode->spstLeft)
    {
        printf("\n%c left\n", pochMarker);
        fovdTreeListInorder(ppstRootNode->spstLeft, pochMarker);
    }
    printf("Mark %c,  nesting level %d, label %s\n", pochMarker, doiNestingLevel, ppstRootNode->spchLabel);
    if('C' == pochMarker)
    {
 	fovdParValSeq_Print( ((tostWParValList*)(ppstRootNode->spstNodeData))->spstFirst );
    } 
    if(NULL != ppstRootNode->spstRight)
    {
        printf("\n%c right\n", pochMarker);
        fovdTreeListInorder(ppstRootNode->spstRight, pochMarker);
    }
    doiNestingLevel--;
    return;
}

void fovdDoubleTreeListInorder(tostWPTreeNode *ppstRootNode, char pochMarker)
{
    if(NULL == ppstRootNode)
    {
        fovdPrintLog(LOG_DEBUG, "NULL tree \n");
        return;
    }

    doiDNestingLevel++;
    if(NULL != ppstRootNode->spstLeft)
    {
        printf("\n%c left\n", pochMarker);
        fovdDoubleTreeListInorder(ppstRootNode->spstLeft, pochMarker);
    }
    printf("Mark %c,  nesting level %d, label %s\n", pochMarker, doiDNestingLevel, ppstRootNode->spchLabel);
    fovdTreeListInorder(ppstRootNode->spstNodeData, 'C');
    
    if(NULL != ppstRootNode->spstRight)
    {
        printf("\n%c right\n", pochMarker);
        fovdDoubleTreeListInorder(ppstRootNode->spstRight, pochMarker);
    }
    doiDNestingLevel--;
    return;
}

void fovdTreeListPreorder(tostWPTreeNode *ppstRootNode)
{
    if(NULL == ppstRootNode)
    {
        fovdPrintLog(LOG_DEBUG, "NULL tree \n");
        return;
    }
    doiNestingLevel++;
    printf("nesting level %d, label %s\n", doiNestingLevel, ppstRootNode->spchLabel);
    if(NULL != ppstRootNode->spstLeft)
    {
        printf("\nleft\n");
        fovdTreeListPreorder(ppstRootNode->spstLeft);
    }
    if(NULL != ppstRootNode->spstRight)
    {
        printf("\nright\n");
        fovdTreeListPreorder(ppstRootNode->spstRight);
    }
    doiNestingLevel--;
    return;
}


/*****************************************************************************
 *
 * Function name:	fpstTreeNode_New
 *
 * Function call:	rc = fpstTreeNode_New(loszLabel, loiLabelLen)
 *
 * Returns:	Pointer to the tree node, with the label = poszLabel
 *              NULL in the case of failure
 *
 *
 * Arguments:	char *poszLabel, int poiLabelLen
 *
 * Description: 
 *
 *****************************************************************************
 */

tostWPTreeNode *fpstTreeNode_New(char *poszLabel, int poiLabelLen)
{
    int rc;
    tostWPTreeNode *lpstTreeNode;
    if(NULL == (lpstTreeNode = calloc(1,sizeof(tostWPTreeNode))) )
    {
        fovdPrintLog(LOG_NORMAL,"fpstTreeNode_New, I can not get memory\n");
        return NULL;
    }
    if(NULL == (lpstTreeNode->spchLabel = malloc(poiLabelLen+1)))
    {
        fovdPrintLog(LOG_NORMAL,"fpstTreeNode_New, I can not get memory\n");
        return NULL;
    }    
    *lpstTreeNode->spchLabel = '\0';
    strcpy(lpstTreeNode->spchLabel, poszLabel);
    return lpstTreeNode;
}

/*****************************************************************************
 *
 * Function name:	fpstTreeNode_Add
 *
 * Function call:	rc = fpstTreeNode_add(lpstRootNode, loszLabel, lpstFoundNode)
 *
 * Returns:	0  if node existed
 *              1  if node added
 *              -1  if failure
 *
 *
 * Arguments:	tostWPTreeNode **ppstRootNode - start for search
 *              char *poszLabel, 
 *              tostWPTreeNode ***ppstFoundNode  - the inserted or found node
 *
 * Description: 
 *
 *****************************************************************************
 */
int foiTreeNode_Add(tostWPTreeNode **ppstRootNode, char *poszLabel, tostWPTreeNode ***ppstFoundNode)
{
    toenParVal eRet, eRet1;
    
    fovdPrintLog(LOG_DEBUG, "foiTreeNode_Add: label %s\n",poszLabel);
    if(*ppstRootNode == NULL)
    {
	fovdPrintLog(LOG_DEBUG, "foiTreeNode_Add: label %s wil be inserted in root\n",poszLabel);
	eRet = PARVAL_NOT_FOUND;
	if(NULL == (*ppstRootNode = fpstTreeNode_New(poszLabel, strlen(poszLabel) )))
	{
	    fovdPrintLog(LOG_NORMAL, "foiTreeNode_Add: I can not introduce the root node, for the label %s\n"
			 ,poszLabel);
	    return -1;
	}
    }
    else
    {
	eRet = foenFindNodeLabel(ppstRootNode, poszLabel, ppstFoundNode);
	if(PARVAL_NOT_FOUND == eRet)
	{
	    fovdPrintLog(LOG_DEBUG, "foiTreeNode_Add: label %s not found, wil be inserted\n",poszLabel);
	    if(NULL == (**ppstFoundNode =  fpstTreeNode_New(poszLabel, strlen(poszLabel) )))
	    {
		fovdPrintLog(LOG_NORMAL, "foiTreeNode_Add: I can not introduce the node, for the label %s\n"
			 ,poszLabel);
		return -1;
	    }
	}
	else
	{
	    fovdPrintLog(LOG_DEBUG, "foiTreeNode_Add: label %s  found\n",poszLabel);
	    return 0;
	}
		
    }
    eRet1 = foenFindNodeLabel(ppstRootNode, poszLabel, ppstFoundNode);
    if(PARVAL_NOT_FOUND == eRet1)
    {
	fovdPrintLog(LOG_NORMAL, "foiTreeNode_Add: internal error, label %s not found\n",poszLabel);
	return -1;
    }
    return 1;
}


void fovdPrintNestingLevel(void)
{
    printf("searched %d levels\n", doiSearchCount);
    doiSearchCount = 0;  
}


void fovdParValSeq_Print(tostWParValRecSeq *ppstWPSeqFirst)
{

    tostWParValRec  *lpstRec;
    tostWParValRecSeq *lpstWPSeq;

    lpstWPSeq = ppstWPSeqFirst;
    if(NULL == lpstWPSeq)
    {
	fovdPrintLog(LOG_DEBUG, "fovdParValSeq_Print: WARNING!! - empty ParVal data list\n"); 
    }
    while(NULL != lpstWPSeq)
    {
	lpstRec = lpstWPSeq->spstRec;
	fovdPrintLog(LOG_DEBUG, "fovdParValSeq_Print:\n  %s %s %s %s %s %s %s %s %s %s\n",
		     lpstRec->soszWParCode,
		     lpstRec->soszCustomerId,
		     lpstRec->soszCoId,
		     lpstRec->soszWParValFinishDate,
		     lpstRec->soszWParVal,
		     lpstRec->soszWParField1,
		     lpstRec->soszWParField2,
		     lpstRec->soszWParField3,
		     lpstRec->soszWParField4,
		     lpstRec->soszWParField5);
	lpstWPSeq = lpstWPSeq->spstNext;
    }
}

void fovdParValSeq_Free(tostWParValRecSeq **ppstWPSeqFirst)
{
    tostWParValRecSeq *lpstWPSeq, *lpstTmp;

    lpstWPSeq = *ppstWPSeqFirst;

    while(NULL != lpstWPSeq)
    {
	lpstTmp = lpstWPSeq;
	free(lpstWPSeq->spstRec);
	lpstWPSeq = lpstWPSeq->spstNext;
	free(lpstTmp);
    }
    *ppstWPSeqFirst = NULL;
}

/*****************************************************************************
 *
 * Function name:	foiParValDate_Next
 *
 * Function call:	rc = foiParValDate_Next( loszEndDate)
 *
 * Returns:	0  if success

 *              -1  if failure
 *
 *
 * Arguments:	
 *              char *poszEndDate 
  *
 * Description: transfers the parval data from the sequence, obtained from the database, to the tree structure. 
 *
 *****************************************************************************
 */
int foiParValDate_Next(char *poszEndDate)
{
    tostWPTreeNode  **lpstTempDateNode,  **lpstTempCustNode, **lpstTmpTreeNode;
    void **lpvdTempPointer;
    toenParVal eRet, eRet1;
    int iRet ;
    char loszCustIdCoId[2*PARVALCODE_LEN];
    
    tostWParValRecSeq *lpstWPSeqFirst = NULL, *lpstSeq;
    tostWParValList *lpstList, *lpstTmpList;
    
    fovdPushFunctionName ("foiParValDate_Next");
    /*
     * Introduce the new date node to the tree, if necessary
     */

    if( -1 == (iRet =  foiTreeNode_Add(&dpstRoot, poszEndDate, &lpstTempDateNode) ) )
    { 
	fovdPrintLog(LOG_NORMAL, "foiParValDate_Next: I can not add the node for the date %s \n",poszEndDate);
	fovdPopFunctionName ();
	return -1;
    }
    if(0 == iRet)
    {
	fovdPrintLog(LOG_DEBUG, "foiParValDate_Next: the date %s is already in the tree\n",poszEndDate);
	fovdPopFunctionName ();
	return 0;
    }
    /*  get the data from the database
     */
    if( 0 >  (iRet = foiWorkParVal_LoadPart(poszEndDate, &lpstWPSeqFirst )))
    { 
	fovdPrintLog(LOG_NORMAL, "foiParValDate_Next: I can not load data from the database for the date %s \n",
		     poszEndDate);
	fovdPopFunctionName ();
	return -1;
    }
    if(0 == iRet)
    {
	fovdPrintLog(LOG_DEBUG, "foiParValDate_Next: No records found in the database for the date %s\n",poszEndDate);
	fovdPopFunctionName ();
	return 0;
    }
    lpstSeq = lpstWPSeqFirst;
    /*
     * for every data item
     */
    while(NULL != lpstSeq)
    {
	strcpy(loszCustIdCoId,lpstSeq->spstRec->soszCustomerId);
	strcat(loszCustIdCoId," ");
	strcat(loszCustIdCoId,lpstSeq->spstRec->soszCoId);
	/*
	 * Introduce the new customer - contract node to the tree, if necessary
	 */
	lpvdTempPointer=&((*lpstTempDateNode)->spstNodeData);
	lpstTmpTreeNode = (tostWPTreeNode**)lpvdTempPointer;	
	/*lpstTmpTreeNode = &(tostWPTreeNode*)((*lpstTempDateNode)->spstNodeData);*/

	if(-1 == (iRet =  foiTreeNode_Add(lpstTmpTreeNode, loszCustIdCoId, &lpstTempCustNode) ) ) 
	{ 
	    fovdPrintLog(LOG_NORMAL, "foiParValDate_Next: I can not add the node for the customer id - contract id %s \n",
			 loszCustIdCoId);
	    fovdPopFunctionName ();
	    return -1;
	}

	if(1 == iRet)
	{
	    /*
	     * the new ParVal list
	     */
	    if(NULL == (lpstList = fpstParValList_New() ) )
	    {
		fovdPrintLog(LOG_NORMAL, 
			     "foiParValDate_Next: I can not add the list for the customer id-contract id %s \n",
			     loszCustIdCoId);
		fovdPopFunctionName ();
		return -1;
	    } 
	    (*lpstTempCustNode)->spstNodeData = lpstList;
	}
	else
	{
	    lpstList = (*lpstTempCustNode)->spstNodeData; 
	}
	/*
	 * transfer of the data
	 */
	if(NULL == lpstList->spstFirst)
	{
	    lpstList->spstFirst = lpstSeq;
	    lpstList->spstLast = lpstSeq;
	}
	else
	{
	    (lpstList->spstLast)->spstNext = lpstSeq;
	    lpstList->spstLast = lpstSeq;
	}
	lpstSeq = lpstSeq->spstNext;
	(lpstList->spstLast)->spstNext = NULL;
    }
    fovdPopFunctionName ();
    return 0;
}

/*****************************************************************************
 *
 * Function name:	fpstParValList_New
 *
 * Function call:	rc = fpstParValList_New()
 *
 * Returns:	Pointer to the list 
 *              NULL in the case of failure
 *
 *
 * Arguments:	
 *
 * Description: 
 *
 *****************************************************************************
 */

tostWParValList *fpstParValList_New(void)
{
    int rc;
    tostWParValList *lpstHead;
    
    if(NULL == (lpstHead = calloc(1,sizeof(tostWParValList))))
    {
        fovdPrintLog(LOG_NORMAL,"fpstParValList_New, I can not get memory\n");
        return NULL;
    }
   return lpstHead;
}

/*****************************************************************************
 *
 * Function name:	fpstParVal_Find
 *
 * Function call:	ptr = fpstParVal_Find(loszDate, loszCustomerId, loszCoId)
 *
 * Returns:	Pointer to the data list 
 *              NULL if the data not found
 *
 *
 * Arguments:	char *poszDate, char *poszCustomerId, char *poszCoId
 *
 * Description: 
 *
 *****************************************************************************
 */
tostWParValList *fpstParVal_Find(char *poszDate, char *poszCustomerId, char *poszCoId)
{

    tostWPTreeNode  **lpstFoundDateNode,  **lpstFoundCustNode, **lpstTmpTreeNode;
    void **lpvdTempPointer;
    toenParVal eRet;
    char loszCustIdCoId[2*PARVALCODE_LEN];

    fovdPushFunctionName ("foiParVal_Find");
    /*
     * find date in the tree
     */
    eRet = foenFindNodeLabel(&dpstRoot, poszDate, &lpstFoundDateNode);
    if(PARVAL_NOT_FOUND == eRet)
    {
	fovdPrintLog(LOG_DEBUG, "fpstParVal_Find: date %s not found\n",poszDate);
	fovdPopFunctionName ();
	return NULL;
    }
    else
    {
	fovdPrintLog(LOG_DEBUG, "fpstParVal_Find: date %s found\n",poszDate);
    }
    /*
     *   find CustomerId - ContractId node
     */
    strcpy(loszCustIdCoId,poszCustomerId);
    strcat(loszCustIdCoId," ");
    strcat(loszCustIdCoId,poszCoId);
	lpvdTempPointer=&((*lpstFoundDateNode)->spstNodeData);
	lpstTmpTreeNode = (tostWPTreeNode**)lpvdTempPointer;
	/*lpstTmpTreeNode = &(tostWPTreeNode*)((*lpstFoundDateNode)->spstNodeData);*/
    eRet = foenFindNodeLabel(lpstTmpTreeNode, loszCustIdCoId, &lpstFoundCustNode);
    if(PARVAL_NOT_FOUND == eRet)
    {
	fovdPrintLog(LOG_DEBUG, "fpstParVal_Find: CustIdCoId '%s' not found\n",loszCustIdCoId);
	fovdPopFunctionName ();
	return NULL;
    }
    else
    {
	fovdPrintLog(LOG_DEBUG, "fpstParVal_Find: CustIdCoId %s found\n",loszCustIdCoId);
	fovdPopFunctionName ();
	return (*lpstFoundCustNode)->spstNodeData; 
    }	
}

void fovdParValRec_Get(tostWParValRec *ppstDestin, tostWParValRec *ppstSource)
{
    memcpy((void*)ppstDestin, (void*)ppstSource, sizeof(tostWParValRec));
} 

/*****************************************************************************
 *
 * Function name:	fovdParValMacro_Print
 *
 * Function call:	fovdParValMacro_Print(lpstWPSeqFirst, loszMacroName, loszLastParameter)
 *
 * Returns:	none
 *              
 *
 *
 * Arguments:	 tostWParValRecSeq *ppstWPSeqFirst -first node in the sequence
 *               char *poszMacroName,
 *               char *poszLastParameter - Customer Code or Dialed Number
 *
 * Description: 
 *
 *****************************************************************************
 */
void fovdParValMacro_Print(tostWParValRecSeq *ppstWPSeqFirst, 
			 char *poszMacroName, 
			 char *poszLastParameter)
{

    tostWParValRec  *lpstRec, lostRecBuff;
    tostWParValRecSeq *lpstWPSeq;

    fovdPushFunctionName ("fovdParValMacro_Print");
    lpstWPSeq = ppstWPSeqFirst;
    if(NULL == lpstWPSeq)
    {
	fovdPrintLog(LOG_DEBUG, "fovdParValMacro_Print: WARNING!! - empty ParVal data list\n"); 
    }
    while(NULL != lpstWPSeq)
    {
	lpstRec = lpstWPSeq->spstRec;
	/*
	 *  Just to isolate
	 */
	fovdParValRec_Get(&lostRecBuff, lpstRec);
	
	fovdPrintLog(LOG_DEBUG, "fovdParValMacro_Print:\n  %s %s %s %s %s %s %s %s %s\n",
		     poszMacroName,
		     lostRecBuff.soszWParField5,
		     lostRecBuff.soszWParField4,
		     lostRecBuff.soszWParField3,
		     lostRecBuff.soszWParField2,
		     lostRecBuff.soszWParField1,
		     lostRecBuff.soszWParVal,
		     lostRecBuff.soszWParCode,
		     poszLastParameter);

	fovdGen( poszMacroName,
		 lostRecBuff.soszWParField5,
		 lostRecBuff.soszWParField4,
		 lostRecBuff.soszWParField3,
		 lostRecBuff.soszWParField2,
		 lostRecBuff.soszWParField1,
		 lostRecBuff.soszWParVal,
		 lostRecBuff.soszWParCode,
		 poszLastParameter,
		 EOL);

	lpstWPSeq = lpstWPSeq->spstNext;
    }
    
    fovdPopFunctionName ();
    return;
}


/*****************************************************************************
 *
 * Function name:	foiParVal_NextDoc
 *
 * Function call:	rc = foiParVal_NextDoc(lpstTimm)
 *
 * Returns:	int
 *
 *              Value on success:
 *
 *                      0
 *
 *		Value on failure:
 *
 *                      -1  - can't get the invoicing period date from EDI
 *                            or another failure 
 *
 * Arguments:	ppstTimm, stTIMM * - SUM TIMM EDI interchange pointer 
 *
 * Description: Next document processing is started here. The settling period
 *              end date is known here and the selection from DB may be done.
 *
 *****************************************************************************
 */

int foiParVal_NextDoc(stTIMM *ppstTimm)
{
    int rc = 0;
    char loszDate[DATE_LEN], loszDate8[DATE_LEN];
    
    fovdPushFunctionName ("foiParVal_NextDoc");
    if (TRUE != (rc = fetchInvoicePeriodEnd(ppstTimm, loszDate, DATE_LEN)))
    {
	    fovdPrintLog(LOG_NORMAL, "foiParVal_NextDoc: I can not get period end date from EDI TIMM\n"); 
	fovdPopFunctionName ();
	return -1;
    }
    fovdDate2Date8(loszDate, loszDate8);

    rc = foiParValDate_Next(loszDate8);
    
    fovdPopFunctionName ();
    return rc;
}

/*****************************************************************************
 *
 * Function name:	foiParVal_SimGen
 *
 * Function call:	rc = foiParVal_SimGen(lpstTimm, loiCoId, loszDn )
 *
 * Returns:	int
 *
 *              Value on success:
 *
 *                      0
 *
 *		Value on failure:
 *
 *                      -1  - can't create macro
 *
 * Arguments:	ppstTimm, stTIMM * - SUM TIMM EDI interchange pointer 
 *              poiCoId, int - actual contarct id
 *              ppszDn, char * - actual directory number of the contract
 *
 * Description: 
 *
 *****************************************************************************
 */

int foiParVal_SimGen(stTIMM *ppstTimm, int poiCoId, char *ppszDn)
{
    tostWParValList *lpstParValList;

    int  rc;
    char loszCustomerId[36];
    char loszDate[DATE_LEN], loszDate8[DATE_LEN];
    char loszCoId[PARVALCODE_LEN];

    fovdPushFunctionName ("foiParVal_SimGen");

    sprintf(loszCoId,"%-d",poiCoId);

    fovdPrintLog(LOG_DEBUG, "foiParVal_SimGen: contract id: int %d string ,%s, \n", poiCoId, loszCoId);

    if (TRUE != (rc = fetchInvoicePeriodEnd(ppstTimm, loszDate, DATE_LEN)))
    {
	fovdPrintLog(LOG_NORMAL, 
		     "foiParVal_SimGen: can't get invoice period end date from SUM EDI TIMM\n");
	fovdPopFunctionName ();
	return -1;
    }
    fovdDate2Date8(loszDate, loszDate8);

    strcpy(loszCustomerId, ppstTimm->unb->v_0010);
    ASSERT(PARVALCODE_LEN > strlen(loszCustomerId));

    if(NULL != (lpstParValList = fpstParVal_Find(loszDate8, loszCustomerId, loszCoId) ) )
    {
	fovdParValMacro_Print(lpstParValList->spstFirst, "SimWorkParVal", ppszDn);
    }
    else
    {
	fovdPrintLog(LOG_DEBUG, "No data - no ParVal macro was generated\n");
    }
  
    fovdPopFunctionName ();
    return 0;
}

/*****************************************************************************
 *
 * Function name:	foiParVal_AccGen
 *
 * Function call:	rc = foiParVal_AccGen(lpstTimm)
 *
 * Returns:	int
 *
 *              Value on success:
 *
 *                      0
 *
 *		Value on failure:
 *
 *                      -1  - can't get cust code or the invoice period end date from SUM EDI TIMM
 *                             
 *
 * Arguments:	ppstTimm, stTIMM * - SUM TIMM EDI interchange pointer 
 *
 * Description: 
 *
 *****************************************************************************
 */

int foiParVal_AccGen(stTIMM *ppstTimm)
{
    tostWParValList *lpstParValList;

    int  rc;
    char loszCustomerId[36];
    char loszDate[DATE_LEN], loszDate8[DATE_LEN];
    char loszCustCode[36];

    fovdPushFunctionName ("foiParVal_AccGen");

    if (TRUE != (rc = fetchInvoicePeriodEnd(ppstTimm, loszDate, DATE_LEN)))
    {
	fovdPrintLog(LOG_NORMAL, 
		     "foiParVal_AccGen: can't get invoice period end date from SUM EDI TIMM\n");
	fovdPopFunctionName ();
	return -1;
    }
    fovdDate2Date8(loszDate, loszDate8);

    if (TRUE != (rc = fetchInvoiceCustomerAccountNo(ppstTimm, loszCustCode, 36)))
    {
	fovdPrintLog(LOG_NORMAL, "foiParVal_AccGen: can't get cust code from SUM EDI TIMM\n");
	fovdPopFunctionName ();
	return -1;
    }

    strcpy(loszCustomerId, ppstTimm->unb->v_0010);
    ASSERT(PARVALCODE_LEN > strlen(loszCustomerId));

    if(NULL != (lpstParValList = fpstParVal_Find(loszDate8, loszCustomerId, "") ) )
    {
	fovdParValMacro_Print(lpstParValList->spstFirst, "AccountWorkParVal", loszCustCode);
    }
    else
    {
	fovdPrintLog(LOG_DEBUG, "No data - no ParVal macro was generated\n");
    }
  
    fovdPopFunctionName ();
    return 0;
}


void fovdDate2Date8(char *poszDate6, char *poszDate8)
{
    char loszDate8[DATE_LEN];

    if(6 == strlen(poszDate6))
    {
	if(0 > strncmp(poszDate6,"70",2))
	{
	    strcpy(loszDate8,"20");
	}
	else
	{
	    strcpy(loszDate8,"19");
	}
	strcat(loszDate8,poszDate6);
	strcpy(poszDate8,loszDate8);
    }
    else
    {
	strcpy(poszDate8,poszDate6);
    }


}
