/********************************************************************************
 *
 * Filename:       vpn.c 
 *
 * Facility:       VPN list hadling
 *
 * Description:    Creates list of VPN contracts and prints it using BGH generator
 *                 interface
 *
 * Author & Date:  Norbert Bondarczuk 11/08/1999
 *
 * Modifications:  Leszek Dabrowski 1/10/1999
 *
 * Date        Engineer   Description
 * ------------------------------------------------------------------------------
 * 11/08/1999  NB         Initial version
 * ------------------------------------------------------------------------------
 *
 * COPYRIGHT (c) 1999 by EDS PERSONAL COMMUNICATIONS CORPORATION, WARSAW, POLAND
 *
 * This software is furnished under a licence and may be used and copied only
 * in accordance with the terms of such licence and with the inclusion of the
 * above copyright notice. This software or any other copies there of may not
 * be provided or otherwise made available to to any other person. No title to
 * and ownership of the software is hereby transfered. The information in this
 * software is subject to change without notice and should not be construed as 
 * a commitment by EDS. EDS assumes no responsibility for the use or reliability
 * of its software on equipment which is not supplied EDS .
 *
 ********************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fnmatch.h>
#include <assert.h>
#include "parser.h"
#include "gen.h"
#include "types.h"
#include "protos.h"
#include "vpn.h"
/*
#include "shdes2des.h"
*/

/*
 * static functions of the VPN module
 */

static int foiVPNServices_Count(struct s_group_22 *ppstG22);

static int foiIsVPNContract(struct s_group_22 *ppstG22);

static  toenBool foenIsDestinVPN(struct s_group_22 *ppstG22);

static enum toenVPN_ERR foenVPNContractList_New(struct s_group_22 *ppstG22, 
						tostVPNContractList **ppstList, 
						char *poszAccountNo, 
						int *ppiSubscriberStatus);

static enum toenVPN_ERR foenVPNItem_New(struct s_group_22 *ppstG22, 
					tostVPNItem **ppstItem, 
					int *ppiSubscrStatus, 
					tostVPNData *pastSum);

static int foiMoney_Long2Str(long poilCharge, char *paszCharge);

static int foiMoney_Str2Long(struct s_moa_seg *ppstMoa, long *ppilCharge);

static struct s_moa_seg *fpstGetCharge(struct s_group_23 *ppstG23, char *ppszAmountId, char *ppszAmountTypeId);

static tostVPNSubscriberList *fpstVPNSubscriberList_New(struct s_group_22 *ppstG22);

static toenBool foenVPNContractList_Gen(tostVPNContractList *ppstList);

static int foenVPNSubscriberSummary_Gen(tostVPNData *ppstSumData, 
					char *poszAccountNumber, 
					char *poszMacroName,
                                        char *poszFormat);

static void fovdPrintSum(tostVPNData *pastSum);

static void fovdPrintItem(tostVPNItem *pastItem);

static int foiMoney_Long2StrForm(long poilCharge, char *ppszCharge, char *poszFormat);

static void fovdSetFormatFrScaleFactor(char *poszFormatStr);

static toenBool foenFindAccountNo(struct s_group_26 *ppstG26, char *poszAccountNo);

static int foiNumOfDots(char * ppszStr);

/*******************************************************************************
 *
 * Function name:	foiIsVPNSubscriber
 * 
 * Function call:	rc = foiIsVPNSubscriber( lpstTimm->inter->g_22 );
 *
 * Returns:	int
 *
 * 	Value on success:
 *
 *       1 if the customer has got some VPN service
 *       0 if the customer does not posses any  VPN service assigned
 *
 *		Value on failure:
 *
 *       -1
 *
 * Arguments:	ppstG22, struct s_group_22 * - pointer to the first Group 22 element in
 *             TIMM SUM-SHEET		
 *
 * Description: For each Group22 item on contract level - 02 level do test
 *              if some of Group22 item on level 04 contains  charge for VPN 
 *              like service. Return TRUE if at last one service like VPN is 
 *              found in the list of charged services.
 *
 *******************************************************************************
 */

int foiIsVPNSubscriber(struct s_group_22 *ppstG22)
{
    struct s_group_22 *lpstG22;
    int loiFound = 0;
  
    /* for each G22 item on contract level */
    lpstG22 = ppstG22;
    while (lpstG22 != NULL)
    {
        if (LEVEL(lpstG22, "02"))
        {
            /* number of VPN service access charges found in the G22 item sublist */
            loiFound = foiIsVPNContract(lpstG22->g_22_next);
            if(1 == loiFound)
            {
                return 1;
            }
        }
      
        /* go to the next item */
        lpstG22 = lpstG22->g_22_next;
    }

    return 0;
}

/*******************************************************************************
 *
 * Function name:	foiIsVPNContract
 * 
 * Function call:	loiCount = foiIsVPNContract( g_22 );
 *
 * Returns:	int
 *
 * 	Value on success:
 *
 *       1  VPN service access charge found
 *       0                       not  found
 *
 *		Value on failure:
 *
 *       -1
 *
 * Arguments:	ppstG22, struct s_group_22 * - pointer to the Group 22 element on level
 *             03, next to the contract level item
 *
 *******************************************************************************
 */

static int foiIsVPNContract(struct s_group_22 *ppstG22)
{
    struct s_group_22 *lpstG22;
    toenBool loenAccess = FALSE;
    struct s_imd_seg *lpstImd;
  
    /* find the access charge on level 04 where service is like VPN */
    /* until the NULL found (end of list) or G22 item on lower than 03 level */
  
    lpstG22 = ppstG22;
    while (lpstG22 != NULL)
    {
        /* subscriber or next contract level */
        if (LEVEL(lpstG22, "01") || LEVEL(lpstG22, "02"))
        {
            /* this is the end of the G22 items sequence */
            break;
        }
      
        /* charge class level */
        fovdPrintLog( LOG_DEBUG, "foiIsVPNContract, level .%s.\n",lpstG22->lin->v_1222);
      
        if( LEVEL(lpstG22, "03"))
        {
            if (NULL != lpstG22->imd && CATEGORY(lpstG22, "A"))
            {
                loenAccess = TRUE;
            }
            else
            {
                loenAccess = FALSE;
            }
	  
            fovdPrintLog( LOG_DEBUG, "foiIsVPNContract, level 03 .%s. Access: %d\n",lpstG22->lin->v_1222, loenAccess);
        }

        /* access charge class items on single charge level */
        if (loenAccess == TRUE && LEVEL(lpstG22, "04"))
        {
            /* get IMD block with service description */
            lpstImd = lpstG22->imd;
            while (lpstImd != NULL)
            {
                /*
                 * printf("imd addr .%x.\n", lpstImd);
                 * printf("service name .%s. .%s.\n", lpstImd->v_7009, lpstImd->v_7008a);
                 */

                if (0 == strcmp(lpstImd->v_7009, "SN"))
                {
                    break;
                }
              
                lpstImd = lpstImd->imd_next;
            }

            fovdPrintLog(LOG_DEBUG, "foiIsVPNContract, choosen service name .%s. .%s. .%s.\n", 
                         lpstImd->v_7009, 
                         lpstImd->v_7008, 
                         lpstImd->v_7008a);
	  
            /* this block must be found */
            ASSERT(lpstImd != NULL);
	  
            /* service short description matches the pattern with VPN string */
            if (0 == fnmatch(VPN_SERV_DES_PATTERN, lpstImd->v_7008, FNM_PERIOD))
            {
                /*                
                 * printf("VPN\n");
                 */

                return 1;
            }
        }
      
        /* go to the next item */
        lpstG22 = lpstG22->g_22_next;
    }

    return 0;
}

/*******************************************************************************
 *
 * Function name:	fpstVPNSubscriberList_New
 * 
 * Function call:	lpstVPNSubscriberList = fpstVPNSubscriberList_New( lpstTimm->inter->g_22 );
 *
 * Returns:	tostVPNSubscriberList *
 *
 * 	Value on success:
 *
 *       pointer to the new list if the list can be created
 *
 *		Value on failure:
 *
 *       NULL, if the list can not be created
 *
 * Arguments:	ppstG22, struct s_group_22 * - pointer to the first Group 22 element in
 *             TIMM SUM-SHEET
 *
 * Description: For each Subscriber create a node in the list, process usage charges
 *              searching for the VPN like zone calls and update the statistic 
 *              info in the node and in the list header.
 *
 *******************************************************************************
 */

static tostVPNSubscriberList *fpstVPNSubscriberList_New(struct s_group_22 *ppstG22)
{
    enum toenVPN_ERR loenVPNError;
    tostVPNSubscriberList *lpstList;
    struct s_group_22 *lpstG22;
    tostVPNContractList *lpstContractList;
    tostVPNSubscriberNode **lpstLastNode;
    tostVPNSubscriberNode *lpstNode;
    int loiCount = 0, loiNumOfDots, loiNOD;
    int i, rc;
    char loszAccountNo[MAX_VPN_ID_LEN];

    /* allocate memory for the list structure header */
    lpstList = (tostVPNSubscriberList *)malloc(sizeof(tostVPNSubscriberList));
    if (lpstList == NULL)
    {
        return NULL;
    }
  
    /* init fields of VPN list header */
    /*
     * ASSERT(ppszAccountNo != NULL);
     */

    strcpy(lpstList->soszLAccountNo, ".....");
    lpstList->soiLASubscriberStatus = 0;
  
    lpstList->spstFirst = NULL;
    lpstLastNode = &lpstList->spstFirst;
  
    for (i = 0; i < enVPNSumPosMax; i++)
    {
        lpstList->sastSum[i].soilVPNUsageCharge = 0L;
        lpstList->sastSum[i].soilNotVPNUsageCharge = 0L;
        lpstList->sastSum[i].soilNotUsageCharge = 0L;
        lpstList->sastSum[i].soilAllCharge = 0L;
        lpstList->sastSum[i].soiNumberOfContracts = 0; 
        strcpy(lpstList->sastSum[i].soszCurrency,"PLN");
    }

    lpstG22 = ppstG22;
    if( ( FALSE == foenFindAccountNo(lpstG22->g_26,lpstList->soszLAccountNo)) )
    {
        fovdPrintLog(LOG_DEBUG, "fpstVPNSubscriberList_New, no AccountNo found\n");
        return NULL;
    }

    loiNumOfDots = foiNumOfDots(lpstList->soszLAccountNo);
    /* for each G22 item on subscriber level */
    while (lpstG22 != NULL)
    {
        if (LEVEL(lpstG22, "01"))
        {
            /* find acount number */
            if( ( FALSE == foenFindAccountNo(lpstG22->g_26,loszAccountNo)) )
            {
                fovdPrintLog(LOG_DEBUG, "fpstVPNSubscriberList_New, no AccountNo found\n");
                return NULL;
            }

            /* eventualy get the payment responsible account*/
            if(( loiNOD = foiNumOfDots(loszAccountNo ) ) < loiNumOfDots && strlen(loszAccountNo) > 0)
            {
                strcpy(lpstList->soszLAccountNo, loszAccountNo);
		loiNumOfDots = loiNOD;
            }

            /* create new VPN info container updating the statistic info */
            loenVPNError =  foenVPNContractList_New(lpstG22->g_22_next, 
                                                    &lpstContractList, 
                                                    loszAccountNo,
                                                    &lpstList->soiLASubscriberStatus); 
            if (VPN_NO_CONTRACT == loenVPNError)
            {
                lpstG22 = lpstG22->g_22_next;
                continue;
            }
            else if ( VPN_OK != loenVPNError )
            {
                fovdPrintLog( LOG_DEBUG, "fpstVPNSubscriberList_New, loenVPNError %d\n",loenVPNError);
                return NULL;
            }
	  
            /* allocate memory for new VPN ContractList */
            lpstNode = (tostVPNSubscriberNode *)malloc(sizeof(tostVPNSubscriberNode));
            if (lpstNode == NULL)
            {
                return NULL;
            }
	  
            /* init new VPN ContractList */
            lpstNode->spstNext = NULL;
            lpstNode->spstCoList = lpstContractList;
	  
            /* append new node to the list of VPN contract lists (at the begin)*/
            /* CAUTION! if the order of macros were incorrect, append at the end! */

            /*            
             * lpstNode->spstNext = lpstList->spstFirst;
             * lpstList->spstFirst = lpstNode;
             */            
	  
            *lpstLastNode = lpstNode;
            lpstLastNode = &lpstNode->spstNext;

            /* update payment responsible subscriber's summary */
            for (i = 0; i < enVPNSumPosMax; i++)
            {
                lpstList->sastSum[i].soilVPNUsageCharge    += lpstContractList->sastSum[i].soilVPNUsageCharge;
                lpstList->sastSum[i].soilNotVPNUsageCharge += lpstContractList->sastSum[i].soilNotVPNUsageCharge;
                lpstList->sastSum[i].soilNotUsageCharge    += lpstContractList->sastSum[i].soilNotUsageCharge;
                lpstList->sastSum[i].soilAllCharge         += lpstContractList->sastSum[i].soilAllCharge;
                lpstList->sastSum[i].soiNumberOfContracts  += lpstContractList->sastSum[i].soiNumberOfContracts; 
                strcpy(lpstList->sastSum[i].soszCurrency,lpstContractList->sastSum[i].soszCurrency);
            }	  
        }

        /* go to the next item */
        lpstG22 = lpstG22->g_22_next;
    }
  
    return lpstList;
}

/*******************************************************************************
 *
 * Function name:	foenVPNContractList_New
 * 
 * Function call:	lpstVPNContractList = foenVPNContractList_New( lpstTimm->inter->g_22, );
 *
 * Returns:	tostVPNContractList *
 *
 * 	Value on success:
 *
 *       pointer to the new list if the list can be created
 *
 *		Value on failure:
 *
 *       NULL, if the list can not be created
 *
 * Arguments:	ppstG22, struct s_group_22 * - pointer to the first Group 22 element in
 *             TIMM SUM-SHEET
 *
 * Description: For each contract create a node in the list, process usage charges
 *              searching for the VPN like zone calls and update the statistic 
 *              info in the node and in the list header.
 *
 *******************************************************************************
 */

static enum toenVPN_ERR foenVPNContractList_New(struct s_group_22 *ppstG22, 
						tostVPNContractList **ppstList, 
						char *poszAccountNo,
						int *ppiSubscriberStatus)
{
    enum toenVPN_ERR loenVPNError;
    tostVPNContractList *lpstList;
    struct s_group_22 *lpstG22;
    tostVPNItem *lpstItem;
    tostVPNContractNode *lpstNode;
    tostVPNContractNode **lpstLastNode;
    int loiCount = 0;
    int i, rc;
  
    /* allocate memory for the list structure header */
    lpstList = (tostVPNContractList *)malloc(sizeof(tostVPNContractList));
    if (lpstList == NULL)
    {
        return VPN_ERROR_NO_MEMORY;
    }
  
    /* init fields of VPN list header */
    ASSERT(poszAccountNo != NULL);
    memset(lpstList->soszAccountNo, 0x00, MAX_ACCOUNT_LEN);
    strcpy(lpstList->soszAccountNo, poszAccountNo);
    lpstList->soiSubscriberStatus = 0;
  
    lpstList->spstFirst = NULL;
    lpstLastNode = &lpstList->spstFirst;

    for (i = 0; i < enVPNSumPosMax; i++)
    {
        lpstList->sastSum[i].soilVPNUsageCharge = 0L;
        lpstList->sastSum[i].soilNotVPNUsageCharge = 0L;
        lpstList->sastSum[i].soilNotUsageCharge = 0L;
        lpstList->sastSum[i].soilAllCharge = 0L;
        lpstList->sastSum[i].soiNumberOfContracts = 0; 
        strcpy(lpstList->sastSum[i].soszCurrency, "PLN");
    }

    /* check, whether this is VPN subscriber */
/*    if( -1 == (rc = foiIsVPNSubscriber(ppstG22->g_22_next)))
 */
    if( -1 == (rc = foiIsVPNSubscriber(ppstG22)))
    {
        return VPN_ERROR_OTHER;
    }

    lpstList->soiSubscriberStatus = rc;
    if(rc)
    {
        *ppiSubscriberStatus = 1;
    }
  
    /* for each G22 item on contract level */
    lpstG22 = ppstG22;
    while (lpstG22 != NULL && !LEVEL(lpstG22,"01"))
    {
        if (LEVEL(lpstG22, "02"))
        {
            /* create new VPN info container updating the statistic info */
            loenVPNError = foenVPNItem_New(lpstG22, 
                                           &lpstItem, 
                                           &lpstList->soiSubscriberStatus, 
                                           lpstList->sastSum);

            if (VPN_NO_CONTRACT == loenVPNError)
            {
                lpstG22 = lpstG22->g_22_next;
                continue;
            }
            else if ( VPN_OK != loenVPNError )
            {
                fovdPrintLog( LOG_DEBUG, "foenVPNContractList_New, loenVPNError %d\n",loenVPNError);
                return loenVPNError;
            }

            /* allocate memory for new VPN item */
            lpstNode = (tostVPNContractNode *)malloc(sizeof(tostVPNContractNode));
            if (lpstNode == NULL)
            {
                return VPN_ERROR_NO_MEMORY;
            }
	  
            /* init new VPN item */
            lpstNode->spstNext = NULL;
            lpstNode->spstItem = lpstItem;
	  
            /* append new node to the list of VPN services (at the begin)*/
            /* CAUTION! if the order of macros were incorrect, append at the end! */
            /*            
             * lpstNode->spstNext = lpstList->spstFirst;
             * lpstList->spstFirst = lpstNode;
             */

            *lpstLastNode = lpstNode;
            lpstLastNode = &lpstNode->spstNext;
        }
      
        /* go to the next item */
        lpstG22 = lpstG22->g_22_next;
    }

    *ppstList = lpstList;
    return VPN_OK;
}

/*******************************************************************************
 *
 * Function name:	foenVPNSubscriberList_Gen
 * 
 * Function call:	rc = foiVPNSubscriberList_Gen( lpstVPNSubscriberList );
 *
 * Returns:	toenBool
 *
 * 	Value on success:
 *
 *       TRUE
 *
 *		Value on failure:
 *
 *       FALSE
 *
 * Arguments:	ppstVPNSubscriberList, tostVPNSubscriberList * - pointer to the list of VPN items
 *             being the Subscriber info nodes of customer with VPN service.
 *
 * Description: The VPN list is printed to the BGH output and memory is deallocated.
 *
 *******************************************************************************
 */

toenBool foenVPNSubscriberList_Gen(struct s_TimmInter *ppstSumTimm)
{
    tostVPNSubscriberList *lpstList;
    tostVPNSubscriberNode *lpstNode, *lpstTmp;  
    static char loszStatusS[MAX_BUFFER_LEN];
    static char loszFormat[MAX_BUFFER_LEN];
    int rc = 0;
  
    /* create VPN lists */
    if(NULL == (lpstList = fpstVPNSubscriberList_New(ppstSumTimm->timm->g_22)))
    {
        return FALSE;
    }
  
    memset(loszStatusS, 0x00, MAX_BUFFER_LEN);

    rc = sprintf(loszStatusS, "%d", lpstList->soiLASubscriberStatus);
    ASSERT(rc > 0);

    fovdSetFormatFrScaleFactor(loszFormat);
    ASSERT(strlen(loszFormat) < MAX_BUFFER_LEN);
#ifdef _FOVDGEN_
    fovdGen("VPNPhonePaymentListStart", lpstList->soszLAccountNo, loszStatusS, EOL);
#else
    printf("%s (%s) (%s)\n","VPNPhonePaymentListStart", lpstList->soszLAccountNo, loszStatusS);
#endif
  
    lpstNode = lpstList->spstFirst; 
    while(lpstNode != NULL)
    {
        foenVPNContractList_Gen(lpstNode->spstCoList);
      
        /* free the allocated data structures */
        lpstTmp = lpstNode;
      
        /* go to the next item */      
        lpstNode = lpstNode->spstNext;
      
        /* previous node may be dealoocated */
		free(lpstTmp);
    }

    /*   summary for the payment responible subscriber */
    rc = foenVPNSubscriberSummary_Gen(&lpstList->sastSum[enYesVPN], 
                                      lpstList->soszLAccountNo, 
                                      "YesVPNSummaryPayment", 
                                      loszFormat);  
    ASSERT(rc == 0);

    rc = foenVPNSubscriberSummary_Gen(&lpstList->sastSum[enNoVPN],
                                      lpstList->soszLAccountNo, 
                                      "NoVPNSummaryPayment", 
                                      loszFormat);
    ASSERT(rc == 0);

    rc = foenVPNSubscriberSummary_Gen(&lpstList->sastSum[enSummary],
                                      lpstList->soszLAccountNo, 
                                      "SumVPNSummaryPayment", 
                                      loszFormat);
    ASSERT(rc == 0);
  
#ifdef _FOVDGEN_
    fovdGen("VPNPhonePaymentListEnd", lpstList->soszLAccountNo, loszStatusS, EOL);
#else
    printf("%s (%s) (%s)\n","VPNPhonePaymentListEnd", lpstList->soszLAccountNo, loszStatusS); 
#endif
    /* free the allocated list */
    free(lpstList);
  
    return TRUE;
}

/*******************************************************************************
 *
 * Function name:	foenVPNContractList_Gen
 * 
 * Function call:	rc = foiVPNContractList_Gen( lpstVPNContractList );
 *
 * Returns:	toenBool
 *
 * 	Value on success:
 *
 *       TRUE
 *
 *		Value on failure:
 *
 *       FALSE
 *
 * Arguments:	ppstVPNContractList, tostVPNContractList * - pointer to the list of VPN items
 *             being the contract info nodes of customer with VPN service.
 *
 * Description: The VPN list is printed to the BGH output and memory is deallocated.
 *
 *******************************************************************************
 */

static toenBool foenVPNContractList_Gen(tostVPNContractList *ppstList)
{
    tostVPNContractNode *lpstNode, *lpstTmp;  
    static char loszStatusS           [MAX_BUFFER_LEN];
    static char loszStatusC           [MAX_BUFFER_LEN];
    static char loszVPNUsageCharge   [MAX_BUFFER_LEN];
    static char loszNotVPNUsageCharge[MAX_BUFFER_LEN];
    static char loszNotUsageCharge   [MAX_BUFFER_LEN];
    static char loszCharge           [MAX_BUFFER_LEN];
    char loszFormat[MAX_BUFFER_LEN];
    int rc = 0;

    memset(loszStatusS, 0x00, MAX_BUFFER_LEN);
    rc = sprintf(loszStatusS, "%d", ppstList->soiSubscriberStatus);
    ASSERT(rc > 0);
    fovdSetFormatFrScaleFactor(loszFormat);
    ASSERT(strlen(loszFormat) < MAX_BUFFER_LEN);
    /*fovdGen("VPNPhonePaymentListStart", ppstList->soszAccountNo, loszStatusS, EOL);
    printf("%s (%s) (%s)\n","VPNPhonePaymentListStart", ppstList->soszAccountNo, loszStatusS);
    */
    lpstNode = ppstList->spstFirst; 
    while(lpstNode != NULL)
    {
        memset(loszStatusC, 0x00, MAX_BUFFER_LEN);
        rc = sprintf(loszStatusC,"%-d",lpstNode->spstItem->soiStatus);
        ASSERT(rc != -1);

        memset(loszVPNUsageCharge, 0x00, MAX_BUFFER_LEN);
        rc = foiMoney_Long2StrForm(lpstNode->spstItem->soilVPNUsageCharge, loszVPNUsageCharge, loszFormat);
        ASSERT(rc != -1);

        memset(loszNotVPNUsageCharge, 0x00, MAX_BUFFER_LEN);
        rc = foiMoney_Long2StrForm(lpstNode->spstItem->soilNotVPNUsageCharge, loszNotVPNUsageCharge, loszFormat);
        ASSERT(rc != -1);
      
        memset(loszNotUsageCharge, 0x00, MAX_BUFFER_LEN);
        rc = foiMoney_Long2StrForm(lpstNode->spstItem->soilNotUsageCharge, loszNotUsageCharge, loszFormat);
        ASSERT(rc != -1);
            
        memset(loszCharge, 0x00, MAX_BUFFER_LEN);
        rc = foiMoney_Long2StrForm(lpstNode->spstItem->soilAllCharge, loszCharge, loszFormat);
        ASSERT(rc != -1);
      
#ifdef _FOVDGEN_
        fovdGen("VPNPhonePaymentItem",
           lpstNode->spstItem->soszMarket,
           lpstNode->spstItem->soszNetwork,
           lpstNode->spstItem->soszDirNo,
           loszStatusC,
           loszVPNUsageCharge,
           loszNotVPNUsageCharge,
           loszNotUsageCharge,
           loszCharge,
           lpstNode->spstItem->soszCurrency,
           EOL);
#else
        printf("%s (%s) (%s) (%s) (%s) (%s) (%s) (%s) (%s) (%s)\n",
               "VPNPhonePaymentItem",
               lpstNode->spstItem->soszMarket,
               lpstNode->spstItem->soszNetwork,
               lpstNode->spstItem->soszDirNo,
               loszStatusC,
               loszVPNUsageCharge,
               loszNotVPNUsageCharge,
               loszNotUsageCharge,
               loszCharge,
               lpstNode->spstItem->soszCurrency);
#endif
        /* free the allocated data structures */
        free(lpstNode->spstItem);
        lpstTmp = lpstNode;

        /* go to the next item */      
        lpstNode = lpstNode->spstNext;

        /* previous node may be dealoocated */
        free(lpstTmp);
    }
    /*   summary for the flat subscriber */
    rc = foenVPNSubscriberSummary_Gen(&ppstList->sastSum[enYesVPN], 
                                      ppstList->soszAccountNo, "YesVPNSubscriberPayment", loszFormat);
    ASSERT(rc == 0);

    rc = foenVPNSubscriberSummary_Gen(&ppstList->sastSum[enNoVPN],
                                      ppstList->soszAccountNo, "NoVPNSubscriberPayment", loszFormat);
    ASSERT(rc == 0);
    rc = foenVPNSubscriberSummary_Gen(&ppstList->sastSum[enSummary],
                                      ppstList->soszAccountNo, "SumVPNSubscriberPayment", loszFormat);
    ASSERT(rc == 0);

    /* fovdGen("VPNPhonePaymentListEnd", ppstList->soszAccountNo, loszStatusS, EOL);
    printf("%s (%s) (%s)\n","VPNPhonePaymentListEnd", ppstList->soszAccountNo, loszStatusS); 
    */
    /* free the allocated list */
    free(ppstList);

    return TRUE;
}

/*******************************************************************************
 *
 * Function name:	foiVPNServices_Count
 * 
 * Function call:	loiCount = foiVPNServices_Count( g_22 );
 *
 * Returns:	int
 *
 * 	Value on success:
 *
 *       number of VPN service access charges found
 *
 *		Value on failure:
 *
 *       -1
 *
 * Arguments:	ppstG22, struct s_group_22 * - pointer to the Group 22 element on level
 *             03, next to the contract level item
 *
 *******************************************************************************
 */

static int foiVPNServices_Count(struct s_group_22 *ppstG22)
{
    struct s_group_22 *lpstG22;
    toenBool loenAccess = FALSE;
    int loiCount = 0;
    struct s_imd_seg *lpstImd;
  
    /* count the number of access charges on level 04 where service is like VPN */
    /* until the NULL found (end of list) or G22 item on lower than 03 level */

    lpstG22 = ppstG22;
    while (lpstG22 != NULL)
    {
        /* subscriber or next contract level */
        if (LEVEL(lpstG22, "01") || LEVEL(lpstG22, "02"))
        {
            /* this is the end of the G22 items sequence */
            break;
        }

        /* charge class level */
        if (LEVEL(lpstG22, "03"))
        {
            if (CATEGORY(lpstG22, "A"))
            {
                loenAccess = TRUE;
            }
            else
            {
                loenAccess = FALSE;
            }
        }

        /* access charge class items on single charge level */
        if (loenAccess == TRUE && LEVEL(lpstG22, "04"))
        {
            /* get IMD block with service description */
            lpstImd = lpstG22->imd;
            while (lpstImd != NULL)
            {
                if (0 == strcmp(lpstImd->v_7009, "SN"))
                {
                    break;
                }
              
                lpstImd = lpstImd->imd_next;
            }

            /* this block must be found */
            ASSERT(lpstImd != NULL);

            /* service long description matches the pattern with VPN string */
            if (0 == fnmatch(VPN_SERV_DES_PATTERN, lpstImd->v_7008a, FNM_PERIOD))
            {
                loiCount++;
            }
        }
          
        /* go to the next item */
        lpstG22 = lpstG22->g_22_next;
    }
  
    return loiCount;
}

/*******************************************************************************
 *
 * Function name:	foenVPNItem_New
 * 
 * Function call:	loenErr = foenVPNItem_New( g_22, VPNItem, SubscriberStatus, Sum );
 *
 * Returns:	tostVPNItem *
 *
 * 	Value on success:
 *
 *       new item allocated
 *
 *		Value on failure:
 *
 *       NULL
 *
 * Arguments:	ppstG22, struct s_group_22 * - pointer to the Group 22 element on level
 *             02 with the contract level item,
 *             pastSum, tostVPNData ** - table of 3 structures with statistic info
 *             to be updated during the creation of the list
 *
 *******************************************************************************
 */

static enum toenVPN_ERR foenVPNItem_New(struct s_group_22 *ppstG22, 
					tostVPNItem **ppstItem,
					int *ppiSubscrStatus, tostVPNData *pastSum)
{
    tostVPNItem *lpstItem;
    toenBool loenUsage = FALSE;
    char *lpszId;
    int i, n, loiDotOff;
    struct s_moa_seg *lpstMoa;
    struct s_imd_seg *lpstImd;
    struct s_group_22 *lpstG22;
    int loiFields = 0;
    int rc;
    int lofForVPN;
    int loiIsNotContract = 0;
    long lolTempCharge;
    enum toenVPN_SUM loenSumIndex; 
  
    lpstG22 = ppstG22;
    fovdPrintLog(LOG_DEBUG, "foenVPNItem_New, %x, %s %s %s\n", 
                 ppstG22, ppstG22->lin->v_1082,  
                 ppstG22->lin->v_7140,  
                 ppstG22->lin->v_1222); 

    /* allocate some memory for a new VPN item */
    lpstItem = (tostVPNItem*)malloc(sizeof(tostVPNItem));
    if (lpstItem == NULL)
    {
        *ppstItem = NULL;
        return VPN_ERROR_NO_MEMORY;
    }
  
    /* fill the fields of the VPN item using G22 values */

    /* check, whether this is VPN contract */
    if( -1 == (rc = foiIsVPNContract(ppstG22->g_22_next)))
    {
        *ppstItem = NULL;
        return VPN_ERROR_OTHER;
    }
  
    lpstItem->soiStatus = rc;
    if(1 == rc)
    {
        *ppiSubscrStatus = 1;
        loenSumIndex = enYesVPN;
    }
    else
    {
        loenSumIndex = enNoVPN;
    }

    fovdPrintLog( LOG_DEBUG, "foenVPNItem_New, indeks NO-VPN %d\n",loenSumIndex);

    pastSum[loenSumIndex].soiNumberOfContracts ++ ;
    pastSum[enSummary].soiNumberOfContracts ++ ;
    lpstImd = ppstG22->imd;
    while (lpstImd != NULL)
    {
        if (EQ(lpstImd->v_7009, "MRKT"))
        {
            /* get full descrition of the market */
            ASSERT(strlen(lpstImd->v_7008a) < MAX_VPN_ID_LEN);
            strcpy(lpstItem->soszMarket, lpstImd->v_7008a);
            loiFields++;
	  
            /* get network shdes using the market description */
	  
            strcpy(lpstItem->soszNetwork, lpstImd->v_7008);
            loiFields++;
        }
      
        else if (EQ(lpstImd->v_7009, "DNNUM"))
        {
            /* get directory number of the contract */
            ASSERT(strlen(lpstImd->v_7008a) < MAX_VPN_ID_LEN);
            strcpy(lpstItem->soszDirNo, lpstImd->v_7008a);
            loiFields++;
        }
        else if (EQ(lpstImd->v_7009, "FE"))
        {
            /* this group does not describe a contract  */
            loiIsNotContract++;
        }
      
        /* go to the next item on IMD sequence */
        lpstImd = lpstImd->imd_next;
    }

    fovdPrintLog( LOG_DEBUG, "foenVPNItem_New, loiFields %d, loiIsNotContract %d\n",loiFields, loiIsNotContract);

    if((3 != loiFields ) && (0 < loiIsNotContract ))
    {
        /* not a contract group */
        pastSum[loenSumIndex].soiNumberOfContracts -- ;
        pastSum[enSummary].soiNumberOfContracts -- ;
        *ppstItem = NULL;
        return VPN_NO_CONTRACT;
    } 
  
    assert (loiFields == 3);
  
    /* set other fields to a default value */
  
    lpstItem->soilVPNUsageCharge = 0L;
    lpstItem->soilNotVPNUsageCharge = 0L;
    lpstItem->soilNotUsageCharge = 0L;  
    lpstItem->soilAllCharge = 0L;  
    strcpy(lpstItem->soszCurrency,"PLN");
  
    /* get sum of contract all charges */
    fovdPrintLog(LOG_DEBUG, "foenVPNItem_New 2, %x, .%s. .%s. .%s.\n", 
                 lpstG22, 
                 lpstG22->lin->v_1082,  
                 lpstG22->lin->v_7140,  
                 lpstG22->lin->v_1222); 
  
    /*    if(NULL != (lpstMoa = fpstGetCharge(lpstG22->g_23, "931", "19"))
     * || NULL != (lpstMoa = fpstGetCharge(lpstG22->g_23, "931", "9")) && 0.0 == atof(lpstMoa->v_5004))
     */

    if(NULL != (lpstMoa = fpstGetCharge(lpstG22->g_23, "931", "9")))
    {
        rc = foiMoney_Str2Long(lpstMoa, &(lpstItem->soilAllCharge));
        ASSERT(rc != -1);

        /*        
         *  pastSum[loenSumIndex].soilAllCharge += lpstItem->soilAllCharge;
         */

        pastSum[enSummary].soilAllCharge += lpstItem->soilAllCharge;
        memset(lpstItem->soszCurrency, 0x00, MAX_CURRENCY_LEN);
        strcpy(lpstItem->soszCurrency, lpstMoa->v_6345);
        strcpy(pastSum[loenSumIndex].soszCurrency, lpstMoa->v_6345);
        strcpy( pastSum[enSummary].soszCurrency, lpstMoa->v_6345);
    }
    else
    {
        fovdPrintLog(LOG_DEBUG, "foenVPNItem_New, No total charge for the item\n");
        lpstItem->soilAllCharge = 0;
        strcpy(lpstItem->soszCurrency, "PLN");
    }
  
    fovdPrintSum(pastSum);
  
    /* calculate VPN charges from the sequence of G22 blocks until NULL found */
    /* or G22 item on level lower than 03 */
    /* for each G22 item on 04 level with Usage charges next to the contract */
    /* item on level 02 */
    lpstG22 = ppstG22->g_22_next;
    while (lpstG22 != NULL)
    {
        fovdPrintLog(LOG_DEBUG, "foenVPNItem_New_3, %x, .%s. .%s. .%s.\n", 
                     lpstG22, 
                     lpstG22->lin->v_1082,  
                     lpstG22->lin->v_7140,  
                     lpstG22->lin->v_1222); 

        /* subscriber or next contract level */
        if (LEVEL(lpstG22, "01") || LEVEL(lpstG22, "02"))
        {
            /* this is the end of the G22 items sequence */
            break;
        }
      
        /* charge class level */
        if (LEVEL(lpstG22, "03") )
        {
            if ( NULL != lpstG22->imd && CATEGORY(lpstG22, "U"))
            {
                loenUsage = TRUE;
            }
            else
            {
                loenUsage = FALSE;
            }
        }
      
        /* usage charge class items on single charge level */
        if (LEVEL(lpstG22, "04"))
        {
            ASSERT(lpstG22->g_23 != NULL);
            if(NULL != (lpstMoa = fpstGetCharge(lpstG22->g_23, "125", "19"))
               || (NULL !=(lpstMoa = fpstGetCharge(lpstG22->g_23, "125", "9")))) /* && 0.0 == atof(lpstMoa->v_5004)))*/
            {
                /*                
                 * lpstMoa = fpstGetCharge(lpstG22->g_23, "125", "19");
                 * ASSERT(lpstMoa != NULL);
                 */
	      
                ASSERT(EQ(lpstItem->soszCurrency, lpstMoa->v_6345));
                rc = foiMoney_Str2Long(lpstMoa, &(lolTempCharge));
                ASSERT(rc != -1);
                if (loenUsage == TRUE )
                {
                    lofForVPN = foenIsDestinVPN(lpstG22);    
                    if (TRUE == lofForVPN)
                    {
                        /* this is VPN usage charge */
                        lpstItem->soilVPNUsageCharge += lolTempCharge;
                        pastSum[loenSumIndex].soilVPNUsageCharge += lolTempCharge;
                        pastSum[enSummary].soilVPNUsageCharge += lolTempCharge;
                        pastSum[loenSumIndex].soilAllCharge += lolTempCharge;
                    }
                    else
                    {
                        /* this is not VPN usage charge */              
                        lpstItem->soilNotVPNUsageCharge += lolTempCharge;
                        pastSum[loenSumIndex].soilNotVPNUsageCharge += lolTempCharge;
                        pastSum[enSummary].soilNotVPNUsageCharge += lolTempCharge;
                        pastSum[loenSumIndex].soilAllCharge += lolTempCharge;
                    }
                }
                else
                {
                    /* this is not usage charge */
                    lpstItem->soilNotUsageCharge += lolTempCharge;          
                    pastSum[loenSumIndex].soilNotUsageCharge += lolTempCharge;
                    pastSum[enSummary].soilNotUsageCharge += lolTempCharge;
                    pastSum[loenSumIndex].soilAllCharge += lolTempCharge;
                }
	      
                /*            
                 * pastSum[loenSumIndex].soilAllCharge += lolTempCharge;
                 * exit(0);
                 */
	      
            }
            /*
              else
              {
              fovdPrintLog(LOG_NORMAL, "foenVPNItem_New, no proper MOA segment\n");
              *ppstItem = NULL;
              return VPN_ERROR_NO_MOA;
              }
            */
        }

        lpstG22 = lpstG22->g_22_next;
        /* go to the next item */
    }
  
    fovdPrintSum(pastSum);
  
    fovdPrintItem(lpstItem);

    *ppstItem = lpstItem;

    return VPN_OK;
}

/*******************************************************************************
 *
 * Function name:	foiMoney_Long2Str
 * 
 * Function call:	rc = foiMoney_Long2Str( loilCharge, ppszCharge );
 *
 * Returns:	int
 *
 * 	Value on success:
 *
 *       0
 *
 *		Value on failure:
 *
 *       -1
 *
 * Arguments:	loilCharge, long - charge to be converted to the money string,
 *             ppszCharge, char * - output buffer with the money string in country
 *             dependent format
 *
 *******************************************************************************
 */

static int foiMoney_Long2Str(long poilCharge, char *ppszCharge)
{
    int rc;
    char loszMsp[64];
    char loszLsp[64];

    rc = sprintf(loszMsp, "%d", poilCharge / VPN_SCALING_FACTOR);
    ASSERT(rc > 0);

#if  (VPN_SCALING_FACTOR == 100)
    rc = sprintf(loszLsp, "%02ld", poilCharge % VPN_SCALING_FACTOR);
#elif (VPN_SCALING_FACTOR == 1000)
    rc = sprintf(loszLsp, "%03ld",  poilCharge % VPN_SCALING_FACTOR);
#elif (VPN_SCALING_FACTOR == 10000)
    rc = sprintf(loszLsp, "%04ld", poilCharge % VPN_SCALING_FACTOR);
#endif
    ASSERT(rc > 0);

    strcpy(ppszCharge, loszMsp);
    strcat(ppszCharge, ",");
    strcat(ppszCharge, loszLsp);
    return 0;
}

/*******************************************************************************
 *
 * Function name:	foiMoney_Str2Long
 * 
 * Function call:	rc = foiMoney_Str2Long( ppszCharge, &loilCharge );
 *
 * Returns:	tostVPNItem *
 *
 * 	Value on success:
 *
 *       0
 *
 *		Value on failure:
 *
 *       -1
 *
 * Arguments:  ppstMoa, struct s_moa_seg * - pointer to TIMM MOA block 
 *           	ppilCharge, long * - charge to be converted from the money string
 *             in MOA.5004
 *
 *******************************************************************************
 */

static int foiMoney_Str2Long(struct s_moa_seg *ppstMoa, long *ppilCharge)
{
    int rc = 0;
    double loflVal = 0.0, lofoNotRound, lofoSign;
  
    if(NULL ==  ppstMoa->v_5004 )
    {
        fovdPrintLog(LOG_DEBUG, " foiMoney_Str2Long, NULL  ppstMoa->v_5004\n");
        return -1;
    }
  
    if(-1 == (rc = sscanf(ppstMoa->v_5004, "%lf", &loflVal)))
    {
        fovdPrintLog(LOG_DEBUG, " foiMoney_Str2Long, I can not convert MOA amount\n");
        return -1;
    }

    lofoSign = loflVal>0 ? 1.0 : -1.0 ;
    *ppilCharge = (long)((loflVal + VPN_EPSILON * lofoSign) * VPN_SCALING_FACTOR );

    /*    lofoNotRound = atof(ppstMoa->v_5004);
          if(*ppilCharge != (long)(lofoNotRound * VPN_SCALING_FACTOR))
          {
          printf(" foiMoney_Str2Long, istotny epsilon,  %lf, %lf, %lf\n", loflVal, lofoNotRound,
          (loflVal - lofoNotRound)* VPN_SCALING_FACTOR);
          printf(" foiMoney_Str2Long, istotny epsilon,  :%s:, %lf, %lf\n", ppstMoa->v_5004,loflVal, lofoNotRound);
          }
    */

    fovdPrintLog( LOG_DEBUG, " foiMoney_Str2Long, money_2_Long %s %12.4f %ld\n",ppstMoa->v_5004,  loflVal, *ppilCharge);
  
    return 0;
}

/*******************************************************************************
 *
 * Function name:	fpstGetCharge
 * 
 * Function call:	lpstMoa = fpstGetCharge( g_23, "999", "19" );
 *
 * Returns:	struct moa_seg *
 *
 * 	Value on success:
 *
 *       pointer to MOA item found on the charge list n G23 sequence
 *
 *		Value on failure:
 *
 *       NULL
 *
 * Arguments:	ppstG23, struct s_group_23 * - pointer to the sequence of g23 items
 *             ppszAmountId, char * - amount id fromMOA.5025
 *             ppszAmountTypeId, char * - amount type id from MOA.4405
 *
 *******************************************************************************
 */

static struct s_moa_seg *fpstGetCharge(struct s_group_23 *ppstG23, 
				       char *ppszAmountId, 
				       char *ppszAmountTypeId)
{
    struct s_group_23 *lpstG23;
    struct s_moa_seg *lpstMoa;
  
    /* for each g23 item of the sequence */
    lpstG23 = ppstG23;
  
    fovdPrintLog(LOG_DEBUG, "fpstGetCharge: AmountId: %s, AmountTypeId: %s\n", ppszAmountId, ppszAmountTypeId);

    while (lpstG23 != NULL)
    {
        lpstMoa = lpstG23->moa;
        ASSERT(NULL != lpstMoa);      
      
        fovdPrintLog(LOG_DEBUG, "fpstGetCharge: MOA: v_5025: %s,  v_4405: %s,  v_6345: %s, v_5004: %s\n", 
                     lpstMoa->v_5025, 
                     lpstMoa->v_4405, 
                     lpstMoa->v_6345, 
                     lpstMoa->v_5004);

        if (EQ(lpstMoa->v_5025, ppszAmountId) && EQ(lpstMoa->v_4405, ppszAmountTypeId))
        {          
            break;
        }
	
        /* go to the next item of G23 sequence */
        lpstG23 = lpstG23->g_23_next;
    }

    /* the end of G23 sequence reached but no item found */
    if (lpstG23 == NULL)
    {
        return NULL;
    }

    fovdPrintLog(LOG_DEBUG, "fpstGetCharge: Found MOA: v_5025: %s, v_4405: %s,  v_6345: %s, v_5004: %s\n", 
                 lpstMoa->v_5025, 
                 lpstMoa->v_4405, 
                 lpstMoa->v_6345, 
                 lpstMoa->v_5004);
  
    return lpstG23->moa;
}

/*******************************************************************************
 *
 * Function name:	foiIsDestinVPN
 * 
 * Function call:	rc = foenIsDestinVPN( lpstTimm->inter->g_22 );
 *
 * Returns:	int
 *
 * 	Value on success:
 *
 *       TRUE if the destination address of the call is VPN 
 *       FALSE if the converse is true
 *
 *
 * Arguments:	ppstG22, struct s_group_22 * - pointer to the  Group 22 element in
 *             TIMM SUM-SHEET		
 *
 * Description: For the Group22 item on usage level - 04 level  test if 
 *              the destination zone short description has a prefix VPN
 *              Return TRUE if the destination is VPN, FALSE if not.
 *
 *******************************************************************************
 */
static  toenBool foenIsDestinVPN(struct s_group_22 *ppstG22)
{
    int i, n, loiDotOff;
    char *lpszId;
  
    fovdPrintLog( LOG_DEBUG, "foenIsDestinVPN, %s\n",ppstG22->pia->v_7140);
    /* zone short description must contain the string VPN */
    n = strlen(ppstG22->pia->v_7140);
    loiDotOff = 0;
    for (i = 0; i < n; i++)
    {
        if (ppstG22->pia->v_7140[i] == '.')
        {
            loiDotOff = i;
        }
    }

    /* next character after the last dot will be used as string to be checked */
    lpszId = ppstG22->pia->v_7140 + loiDotOff + 1;
  
    /* determine id the charge is VPN or  not VPN usage charge */
    if (0 == fnmatch(VPN_ZONE_PATTERN, lpszId, FNM_PERIOD))
    {
        /* this is VPN usage charge */
        return TRUE;
    }
    else
    {
        /* this is not VPN usage charge */              
        return FALSE;
    }
}

/*******************************************************************************
 *
 * Function name:	foenVPNSubscriberSummary_Gen
 * 
 * Function call:	rc = foenVPNSubscriberSummary_Gen( lpstSumData, loszAccountNumber, loszMacroName );
 *
 * Returns:	toenBool
 *
 * 	Value on success:
 *
 *       TRUE
 *
 *		Value on failure:
 *
 *       FALSE
 *
 * Arguments:	ppstSumData, tostVPNData * - pointer to the table of summary information 
 *              poszAccountNumber - the customer code
 *              poszMacroName - the name of macro to be generated
 *
 * Description: The VPN summary information is printed to the BGH output.
 *
 *******************************************************************************
 */

static int foenVPNSubscriberSummary_Gen(tostVPNData *ppstSumData, char *poszAccountNumber, char *poszMacroName, char *poszFormat)
{
    char loszVPNUsageCharge   [MAX_BUFFER_LEN];
    char loszNotVPNUsageCharge[MAX_BUFFER_LEN];
    char loszNotUsageCharge   [MAX_BUFFER_LEN];
    char loszCharge           [MAX_BUFFER_LEN];
    char loszNumberOfContracts[MAX_BUFFER_LEN];
    int rc = 0;
  
    rc = foiMoney_Long2StrForm(ppstSumData->soilVPNUsageCharge, loszVPNUsageCharge, poszFormat);
    ASSERT(rc != -1);
  
    rc = foiMoney_Long2StrForm(ppstSumData->soilNotVPNUsageCharge, loszNotVPNUsageCharge, poszFormat);
    ASSERT(rc != -1);
  
    rc = foiMoney_Long2StrForm(ppstSumData->soilNotUsageCharge, loszNotUsageCharge, poszFormat);
    ASSERT(rc != -1);
  
    rc = foiMoney_Long2StrForm(ppstSumData->soilAllCharge, loszCharge, poszFormat);
    ASSERT(rc != -1);
  
    rc = sprintf(loszNumberOfContracts, "%d", ppstSumData->soiNumberOfContracts);
    ASSERT(rc > 0);
  
#ifdef _FOVDGEN_
    fovdGen(poszMacroName,
            poszAccountNumber,
            loszNumberOfContracts,
            loszVPNUsageCharge,
            loszNotVPNUsageCharge,
            loszNotUsageCharge,
            loszCharge,
            ppstSumData->soszCurrency,
            EOL);
#else
    printf("%s (%s) (%s) (%s) (%s) (%s) (%s) (%s)\n",
           poszMacroName,
           poszAccountNumber,
           loszNumberOfContracts,
           loszVPNUsageCharge,
           loszNotVPNUsageCharge,
           loszNotUsageCharge,
           loszCharge,
           ppstSumData->soszCurrency);
#endif

    return 0;
} 

void fovdPrintSum(tostVPNData *pastSum)
{
    int i;
    fovdPrintLog( LOG_DEBUG, "fovdPrintSum\n");
  
    for(i=0;i< enVPNSumPosMax; i++)
    {
        fovdPrintLog( LOG_DEBUG, "%d\n %d %ld %ld %ld %ld %s\n",i,
                      pastSum[i].soiNumberOfContracts,
                      pastSum[i].soilVPNUsageCharge,
                      pastSum[i].soilNotVPNUsageCharge,
                      pastSum[i].soilNotUsageCharge,
                      pastSum[i].soilAllCharge,
                      pastSum[i].soszCurrency);
    }
}

void fovdPrintItem(tostVPNItem *postItem)
{
  fovdPrintLog( LOG_DEBUG, "fovdPrintItem\n");
  {
    fovdPrintLog( LOG_DEBUG, " %d %ld %ld %ld %ld %s\n",
		 postItem->soiStatus,
		 postItem->soilVPNUsageCharge,
		 postItem->soilNotVPNUsageCharge,
		 postItem->soilNotUsageCharge,
		 postItem->soilAllCharge,
		 postItem->soszCurrency);
  }
}

static void fovdSetFormatFrScaleFactor(char *poszFormatStr)
{
  int n;
  long k =  VPN_SCALING_FACTOR;
  char loszBuf[20];

  sprintf(loszBuf,"%ld",k);
  n = strlen(loszBuf);
  ASSERT(n < 20);
  
  strcpy(poszFormatStr, "%0");
  sprintf(poszFormatStr+2,"%d",n-1);
  strcat(poszFormatStr,"ld");

  fovdPrintLog( LOG_DEBUG, " fovdSetFormatFrScaleFactor, Format %s\n",poszFormatStr);

  return;
}

/*******************************************************************************
 *
 * Function name:	foiMoney_Long2Str
 * 
 * Function call:	rc = foiMoney_Long2Str( loilCharge, ppszCharge );
 *
 * Returns:	int
 *
 * 	Value on success:
 *
 *       0
 *
 *		Value on failure:
 *
 *       -1
 *
 * Arguments:	loilCharge, long - charge to be converted to the money string,
 *             ppszCharge, char * - output buffer with the money string in country
 *             dependent format
 *
 *******************************************************************************
 */

static int foiMoney_Long2StrForm(long poilCharge, char *ppszCharge, char *poszFormat)
{
  int rc;
  char loszMsp[64];
  char loszLsp[64];
  
  rc = sprintf(loszMsp, "%d", poilCharge / VPN_SCALING_FACTOR);
  ASSERT(rc > 0);

  rc = sprintf(loszLsp, poszFormat, abs(poilCharge) % VPN_SCALING_FACTOR);
  ASSERT(rc > 0);
  
  strcpy(ppszCharge, loszMsp);
  strcat(ppszCharge, ",");
  strcat(ppszCharge, loszLsp);
  return 0;
}


static toenBool foenFindAccountNo(struct s_group_26 *ppstG26, char *poszAccountNo)
{
    struct s_group_26 *lpstG26;
  
    lpstG26 = ppstG26;
    while(NULL != lpstG26)
    {
        if(NULL != lpstG26->rff)
        {
            if(!strcmp(lpstG26->rff->v_1153,"IT"))
            {
                strcpy(poszAccountNo, lpstG26->rff->v_1154);
                return TRUE;
            }
        }
    } 
  
    return FALSE;
}

static int foiNumOfDots(char * ppszStr)
{
  char *ptr;
  int n = 0;
  
  ptr = ppszStr;
  while('\0' != *ptr)
    if('.' == *(ptr++))
      n++;
  return n;
}





