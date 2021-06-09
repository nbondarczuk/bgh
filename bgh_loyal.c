/***************************************************************************** 
 * Filename:       bgh_loyal.c 
 *
 * Facility:       BGH customer loayality stat system
 * 
 * Description:    Saving stat. info from g_22 items of SUM-SHEET TIMM message
 *
 * Author & Date:  Norbert Bondarczuk, 23/05/2000
 *
 * Modifications:
 *
 * Date       Engineer   Description
 * ---------------------------------------------------------------------------
 * 23/05/00   NB         Initial version
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fnmatch.h>
#include "bgh.h"
#include "protos.h"
#include "types.h"
#include "bgh_loyal.h"

/*
 * Extern variables
 */

extern stBGHGLOB	stBgh;

extern long O_ohxact;

extern short O_ohxact_ind;     /* NULL indicator */

toenBool foenTimm_GetDiscAmount(struct s_TimmInter *ppstInv, 
                                double *flDisAmt);

/* 
 * Static variables
 */

static char szTemp[PATH_MAX];		/* temporay storage for error messages */

static int foiLoyal_ProcDiscount(struct s_TimmInter *ppstInv,
                                 struct tostLoyalInvInfo *ppstInfo)
{
  int rc = 0;
  struct s_moa_seg *moa = NULL;
  struct s_group_45 *g_45;
  int n;
  double loflAmt = 0.0;
  
  g_45 = ppstInv->timm->g_45;
  while (g_45)
    {
      if (EQ(g_45->moa->v_5025, "920") 
          && EQ(g_45->moa->v_4405, "9"))
        {
          moa = g_45->moa;
          break;
        }
      
      g_45 = g_45->g_45_next;
    }

  if (moa == NULL)
    {
      sprintf (szTemp, "Can't find MOA+920...+9 in INV TIMM message\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);                  
      rc = -1;
    }

  if (rc == 0)
    {
      loflAmt = 0.0;
      n = sscanf(moa->v_5004, "%lf", &loflAmt);
      if (n != 1)
        {
          sprintf (szTemp, "Can't scan value: %s\n", moa->v_5004);
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);                  
          rc = -2;
        }
      else
        {
          if (loflAmt != 0.0)
            {
              ppstInfo->sostDis.soflNetAmt = loflAmt;
              ppstInfo->sostDis.soflTaxAmt = loflAmt * 0.22;
              
              rc = foiEdsBghLoyalTrl_SumInsert(ppstInfo,
                                               NULL,
                                               -1L);
              if (rc != 0)
                {
                  sprintf (szTemp, "Can't save discount info\n");
                  macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);                  
                  rc = -3;
                }
            }
        }
    }

  return rc;
}


/*****************************************************************************
 *
 * Function name: fpszTimm_GetDate
 *
 * Function call: 
 *
 * Returns:	      char *
 *
 *                Value on success:
 *
 *                Pointer to the field found
 *
 *		            Value on failure:
 *
 *                NULL
 *
 * Arguments:     ppstInv, struct s_TimmInter *
 *                ppszTypeId, char *
 *
 * Description: 
 *
 *****************************************************************************
 */

static char *fpszTimm_GetDate(struct s_TimmInter *ppstInv, 
                              char *ppszTypeId)
{
  struct s_dtm_seg *dtm;

  dtm = ppstInv->timm->dtm;
  while (dtm != NULL)
    {
      if (0 == strcmp(dtm->v_2005, ppszTypeId))
        {
          return dtm->v_2380;
        }

      dtm = dtm->dtm_next;
    }

  sprintf (szTemp, "Can't find DTM sgment with id: %s\n", ppszTypeId);
  macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);

  return NULL;
}

/*****************************************************************************
 *
 * Function name: fpszTimm_GetCustInfo
 *
 * Function call: 
 *
 * Returns:	      char *
 *
 *                Value on success:
 *
 *                Pointer to the field found
 *
 *		            Value on failure:
 *
 *                NULL
 *
 * Arguments:     ppstInv, struct s_TimmInter *
 *                ppszTypeId, char *
 *
 * Description: 
 *
 *****************************************************************************
 */

static char *fpszTimm_GetCustInfo(struct s_TimmInter *ppstInv, 
                                  char *ppszTypeId)
{
  struct s_group_2 *g_2;
  struct s_group_3 *g_3;
  char *lpszVal = NULL;

  g_2 = ppstInv->timm->g_2;
  g_3 = NULL;
  while (g_2 != NULL)
    {
      if (0 == strcmp(g_2->nad->v_3035, "IV"))
        {
          g_3 = g_2->g_3;
          break;
        }

      g_2 = g_2->g_2_next;
    }
  
  if (g_3 != NULL)
    {
      while (g_3 != NULL)
        {
          if (0 == strcmp(g_3->rff->v_1153, ppszTypeId))
            {
              lpszVal = g_3->rff->v_1154;
            }
          
          g_3 = g_3->g_3_next;
        }
    }
  
  return lpszVal;
}

/*****************************************************************************
 *
 * Function name: fpszTimm_GetAmtType
 *
 * Function call: 
 *
 * Returns:	      char *
 *
 *                Value on success:
 *
 *                Pointer to the structure found
 *
 *		            Value on failure:
 *
 *                NULL
 *
 * Arguments:     g_22, struct s_group_22 *
 *                ppszAmtType, char *
 *                ppszAmtTypeId, char *
 *
 * Description: 
 *
 *****************************************************************************
 */

static char *fpszTimm_GetAmtType(struct s_group_22 *g_22, 
                                 char *ppszAmtType, 
                                 char *ppszAmtTypeId)
{
  struct s_moa_seg *moa;
  struct s_group_23 *g_23;
  
  g_23 = g_22->g_23;
  while (g_23 != NULL)
    {      
      moa = g_23->moa;
      if (0 == strcmp(moa->v_5025, ppszAmtType) 
          && 0 == strcmp(moa->v_4405, ppszAmtTypeId))
        {
          return moa->v_5004;
        }
      
      g_23 = g_23->g_23_next;
    }

  sprintf (szTemp, "Can't find MOA with id: %s, type: %s\n", ppszAmtType, ppszAmtTypeId);           
  macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);

  return NULL;
}

/*****************************************************************************
 *
 * Function name: fpszTimm_GetTaxRate
 *
 * Function call: 
 *
 * Returns:	  char *
 *
 *                Value on success:
 *
 *                Pointer to the structure found
 *
 *		  Value on failure:
 *
 *                NULL
 *
 * Arguments:     g_22, struct s_group_22 *
 *
 * Description: 
 *
 *****************************************************************************
 */

static char *fpszTimm_GetTaxRate(struct s_group_22 *g_22)
{
  return g_22->g_30->tax->v_5278;
}

/*****************************************************************************
 *
 * Function name: fpstTimm_GetMatchServ
 *
 * Function call: 
 *
 * Returns:	      struct s_group_22 *
 *
 *                Value on success:
 *
 *                Pointer to the structure found
 *
 *		            Value on failure:
 *
 *                NULL
 *
 * Arguments:     inv_g_22, struct s_group_22 *
 *                sum_g_22, struct s_group_22 *
 *
 * Description: 
 *
 *****************************************************************************
 */

static struct s_group_22 *fpstTimm_GetMatchServ(struct s_group_22 *inv_g_22, 
                                                struct s_group_22 *sum_g_22)
{
  while (inv_g_22 != NULL)
    {
      if (inv_g_22->pia != NULL)
        {
          fovdPrintLog (LOG_DEBUG, "Matching: LIN: %s, PIA: %s\n", 
                        inv_g_22->lin->v_7140,
                        inv_g_22->pia->v_7140);
        }
      else
        {
          fovdPrintLog (LOG_DEBUG, "Matching: LIN: %s\n", 
                        inv_g_22->lin->v_7140);
        }

      if (0 == strcmp(inv_g_22->lin->v_7140, sum_g_22->lin->v_7140))
        {
          if (inv_g_22->pia == NULL && sum_g_22->pia == NULL)
            {
              return inv_g_22;
            }
          else if (inv_g_22->pia == NULL || sum_g_22->pia == NULL)
            {
              /* nop*/
            }
          else if (0 == strcmp(inv_g_22->pia->v_7140, sum_g_22->pia->v_7140))
            {
              return inv_g_22;
            }
          else
            {
              /* nop */
            }
        }

      inv_g_22 = inv_g_22->g_22_next;
    }

  if (sum_g_22->pia != NULL)
    {
      sprintf (szTemp, "Can't match g_22, LIN: %s, PIA: %s\n",
               sum_g_22->lin->v_7140, 
               sum_g_22->pia->v_7140);
    }
  else
    {
      sprintf (szTemp, "Can't match g_22, LIN: %s\n",
               sum_g_22->lin->v_7140);               
    }

  fovdPrintLog(LOG_NORMAL, "%s\n",szTemp);
/*
  macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);  
*/ 
  return NULL;
}

/*****************************************************************************
 *
 * Function name: foiLoyalInvInfo_Init
 *
 * Function call: 
 *
 * Returns:	      int
 *
 *                Value on success:
 *
 *                0
 *
 *		            Value on failure:
 *
 *                -1 -
 *                -2 -
 *                -3 -
 *                -4 -
 *
 * Arguments:     ppstInfo, struct tostLoyalInvInfo *
 *                ppstInv, struct s_TimmInter *
 *
 * Description: 
 *
 *****************************************************************************
 */

static foiLoyalInvInfo_Init(struct tostLoyalInvInfo *ppstInfo,
                            struct s_TimmInter *ppstInv)
{
  int rc = 0;
  int n;
  char *lpszVal;

  /* blindly init structure */
  memset(ppstInfo, 0x00, sizeof(struct tostLoyalInvInfo));

  /* get customer id */
  n = sscanf(ppstInv->unb->v_0010, "%ld", &(ppstInfo->solCustId));
  if (n != 1)
    {
      sprintf (szTemp, "Can't scan UNB segment getting customer_id\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);  
      rc = -1;
    }

  /* get cust code */
  if (rc == 0)
    {
      lpszVal = fpszTimm_GetCustInfo(ppstInv, "IT");
      if (lpszVal != NULL)
        {
          ppstInfo->spszCustCode = lpszVal;
        }
      else
        {
          sprintf (szTemp, "Can't get custcode\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);  
          rc = -2;
        }
    }

  /* get ohrefnum */
  if (rc == 0)
    {
      lpszVal = fpszTimm_GetCustInfo(ppstInv, "IV");
      if (lpszVal != NULL)
        {
          ppstInfo->spszOhRefNum = lpszVal;
        }
      else
        {
          sprintf (szTemp, "Can't get ohrefnum\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);  
          rc = -3;
        }
    }

  /* get ohrefdate */
  if (rc == 0)
    {
      lpszVal = fpszTimm_GetDate(ppstInv, "3");
      if (lpszVal != NULL)
        {
          ppstInfo->spszOhRefDate = lpszVal;
        }
      else
        {
          sprintf (szTemp, "Can't get ohrefdate\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);  
          rc = -4;
        }
    }

  /* get ohxact */
  if (O_ohxact_ind == -1)
    {
      ppstInfo->solOhxact = -1;
    }
  else
    {
      ppstInfo->solOhxact = O_ohxact;
    }

  return rc;
}

/*****************************************************************************
 *
 * Function name: fpstLoyalCoTree_New
 *
 * Function call: 
 *
 * Returns:	      struct tostLoyalCoTree *
 *
 *                Value on success:
 *
 *                Pointer to new allocated structure
 *
 *	     	        Value on failure:
 *
 *                NULL
 *
 * Arguments:     ppstInv, struct s_TimmInter *
 *
 * Description: 
 *
 *****************************************************************************
 */

static struct tostLoyalCoTree *fpstLoyalCoTree_New(struct s_TimmInter *ppstInv)
{
  int rc = 0;
  int n;
  struct tostLoyalCoTree *lpstTree;

  lpstTree = (struct tostLoyalCoTree *)malloc(sizeof(struct tostLoyalCoTree));
  if (lpstTree == NULL)
    {
      sprintf (szTemp, "Can't malloc memory\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);  
      return NULL;
    }
  else
    {
      /* empty tree */
      lpstTree->spstRoot = NULL;
      
      /* info fields */
      rc = foiLoyalInvInfo_Init(&(lpstTree->sostInvInfo), ppstInv);
      if (rc != 0)
        {
          sprintf (szTemp, "Can't load invoice infor from TIMM INV message\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);  
          return NULL;
        }
    }

  return lpstTree;
}

/*****************************************************************************
 *
 * Function name: foiGetCoInfo
 *
 * Function call: 
 *
 * Returns:	      int
 *
 *                Value on success:
 *
 *                0
 *
 *		            Value on failure:
 *
 *                -1 - 
 *                -2 -
 *                -3 -
 *
 * Arguments:     g_22, struct s_group_22 *
 *                ppstInfo, struct tostLoyalCoInfo *
 *
 * Description: 
 *
 *****************************************************************************
 */

static int foiGetCoInfo(struct s_group_22 *g_22, 
                        struct tostLoyalCoInfo *ppstInfo)
{
  int rc = 0;
  int n;
  struct s_imd_seg *imd;
  
  memset(ppstInfo, 0x00, sizeof(struct tostLoyalCoInfo));
  
  ppstInfo->solCoId = -1L;
  
  ppstInfo->spszDnNum = NULL;
  
  if (g_22->lin->v_7140[strlen(g_22->lin->v_7140) - 1] == 'O')
    {
      ppstInfo->solCoId = -1;
      ppstInfo->spszDnNum = "";
    }
  else
    {
      imd = g_22->imd;
      while (imd != NULL)
        {
          if (0 == strcmp(imd->v_7009, "CO"))
            {
              n = sscanf(imd->v_7008a, "%ld", &(ppstInfo->solCoId));          
              if (n != 1)
                {
                  sprintf (szTemp, "Can't scan field IMD: %s\n", imd->v_7008a);
                  macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);               
                  rc = -1;
                }
            }      
          else if (0 == strcmp(imd->v_7009, "DNNUM"))
            {
              ppstInfo->spszDnNum = imd->v_7008a;
            }
          else
            {}
          
          if (rc != 0)
            {
              break;
            }
          
          imd = imd->imd_next;
        }

      if (ppstInfo->solCoId == -1L)
        {
          sprintf (szTemp, "Can't find contract id\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);               
          rc = -2;
        }
      else if (ppstInfo->spszDnNum == NULL)
        {
          sprintf (szTemp, "Can't find directory number\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);               
          rc = -3;
        }
      else
        {}
    }

  return rc;
}

/*****************************************************************************
 *
 * Function name: fpstLoyalCoNode_New
 *
 * Function call: 
 *
 * Returns:       struct tostLoyalCoNode *
 *
 *                Value on success:
 *
 *                Pointer to new allocated structure
 *
 *                Value on failure:
 *
 *                NULL
 *
 * Arguments:     ppstInfo, struct tostLoyalCoInfo *
 *
 * Description:   
 *
 *****************************************************************************
 */

static struct tostLoyalCoNode *fpstLoyalCoNode_New(struct tostLoyalCoInfo *ppstInfo)
{
  struct tostLoyalCoNode *lpstNode;

  lpstNode = (struct tostLoyalCoNode *)malloc(sizeof(struct tostLoyalCoNode));
  if (lpstNode == NULL)
    {
      sprintf (szTemp, "Can't malloc memory\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);               
      return NULL;
    }
  else
    {
      /* assure empty substructures */
      memset(lpstNode, 0x00, sizeof(struct tostLoyalCoNode));
      
      /* fill direct fields */
      lpstNode->sostCoInfo.solCoId = ppstInfo->solCoId;
      lpstNode->sostCoInfo.spszDnNum = ppstInfo->spszDnNum;
      lpstNode->spstLeft = NULL;
      lpstNode->spstRight = NULL;
      
      /* fill values not set during Info filling */
      lpstNode->sostCoInfo.sostCoServTree.spstRoot = NULL;
      lpstNode->sostCoInfo.sostCoServTree.sostSumVal.soflNetAmt = 0.0;
      lpstNode->sostCoInfo.sostCoServTree.sostSumVal.soflTaxAmt = 0.0;
    }

  return lpstNode;
}

/*****************************************************************************
 *
 * Function name:  fpstLoyalCoNode_Find
 *
 * Function call: 
 *
 * Returns:        struct tostLoyalCoServTree *
 *
 *                 Value on success:
 *
 *                 Pointer to new allocated structure or the old one
 *
 *                 Value on failure:
 *
 *                 NULL
 *
 * Arguments:      ppstNode, struct tostLoyalCoNode **
 *                 ppstInfo, struct tostLoyalCoInfo *
 *
 * Description:   
 *
 *****************************************************************************
 */

static struct tostLoyalCoServTree *fpstLoyalCoNode_Find(struct tostLoyalCoNode **ppstNode,
                                                        struct tostLoyalCoInfo *ppstInfo)
{
  struct tostLoyalCoNode *lpstNode;
  struct tostLoyalCoServTree *lpstTree;

  lpstNode = *ppstNode;

  if (lpstNode == NULL)
    {
      /* create new node */
      lpstNode = fpstLoyalCoNode_New(ppstInfo);
      if (lpstNode == NULL)
        {
          sprintf (szTemp, "Can't create new loyal contract node\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);               
          return NULL;
        }
      else
        {
          *ppstNode = lpstNode;
          lpstTree = &(lpstNode->sostCoInfo.sostCoServTree);
        }
    }
  else
    {      
      if (ppstInfo->solCoId < lpstNode->sostCoInfo.solCoId)
        {
          /* left */
          lpstTree = fpstLoyalCoNode_Find(&(lpstNode->spstLeft), ppstInfo);
        }
      else if (ppstInfo->solCoId > lpstNode->sostCoInfo.solCoId)
        {
          /* right */
          lpstTree = fpstLoyalCoNode_Find(&(lpstNode->spstRight), ppstInfo);
        }
      else
        {
          /* found */
          lpstTree = &(lpstNode->sostCoInfo.sostCoServTree);
        }
    }

  return lpstTree;
}

/*****************************************************************************
 *
 * Function name:  fpstLoyalCoTree_Find
 *
 * Function call: 
 *
 * Returns:	       struct tostLoyalCoServTree *
 *
 *                 Value on success:
 * 
 *                 Pointer to new allocated structure or the old one
 *
 *                 Value on failure:
 *
 *                 NULL
 *
 * Arguments:      ppstCoTree, struct tostLoyalCoTree *
 *                 ppstInfo, struct tostLoyalCoInfo *
 *
 * Description:   
 *
 *****************************************************************************
 */

static struct tostLoyalCoServTree *fpstLoyalCoTree_Find(struct tostLoyalCoTree *ppstCoTree,
                                                        struct tostLoyalCoInfo *ppstInfo)
{
  struct tostLoyalCoServTree *lpstTree = NULL;
  
  lpstTree = fpstLoyalCoNode_Find(&(ppstCoTree->spstRoot), ppstInfo);
  if (lpstTree == NULL)
    {
      sprintf (szTemp, "Can't find loyal contract node\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);               
      return NULL;
    }
  else
    {
      /* */
    }

  return lpstTree;
}


/*****************************************************************************
 *
 * Function name:  foiGetTypeId
 *
 * Function call: 
 *
 * Returns:	       int
 *
 *                 Value on success:
 *
 *                 0
 *
 *		             Value on failure:
 *
 *                 -1 -
 *                 -2 -
 *
 * Arguments:      ppstKey, (struct tostLoyalCoServKey **
 *                 g_22, struct s_group_22 *
 *
 * Description:    Encoding from EDI TIMM format of Group 22 to Serv 
 *
 *****************************************************************************
 */

int foiGetTypeId(struct tostLoyalCoServKey **ppstKey, 
                 struct s_group_22 *g_22)
{
  int rc = 0;  
  static struct tostLoyalCoServKeySeq *dpstKeySeq = NULL; /* list of rules */
  struct tostLoyalCoServKeySeq *lpstSeq = NULL;
  char *lpszLin7140 = NULL;
  char *lpszPia7140 = NULL;
  char *lpszPatLin7140 = NULL;
  char *lpszPatPia7140 = NULL;

  if (dpstKeySeq == NULL)
    {
      /* load list of rules from DB */
      rc = foiEdsBghLoyalChargeType_Load(&lpstSeq);
      if (rc != 0)
        {
          sprintf (szTemp, "Can't load list of patterns from DB\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);          
          rc = -1;
        }
      else
        {
          dpstKeySeq = lpstSeq;
        }
    }
  
  /* search the list of rules matching LIN + PIA pattern */
  if (rc == 0)
    {
      lpszLin7140 = g_22->lin->v_7140;
      if (g_22->pia != NULL)
        {
          lpszPia7140 = g_22->pia->v_7140;
        }
      else
        {
          lpszPia7140 = "";
        }

      fovdPrintLog (LOG_DEBUG, "Matching: LIN: %s, PIA: %s\n", 
                    lpszLin7140,
                    lpszPia7140);
      
      lpstSeq = dpstKeySeq;
      while (lpstSeq != NULL)
        {
          lpszPatLin7140 = lpstSeq->spstKey->saszLin7140;
          lpszPatPia7140 = lpstSeq->spstKey->saszPia7140;
          
          if (0 == fnmatch(lpszPatLin7140, lpszLin7140, FNM_PERIOD)
              && 0 == fnmatch(lpszPatPia7140, lpszPia7140, FNM_PERIOD))
            {
              break;
            }
          
          lpstSeq = lpstSeq->spstNext;
        }
    }

  /* must find some pattern (even the most general one) else error */
  if (rc == 0)
    {
      if (lpstSeq != NULL)
        {
          *ppstKey = lpstSeq->spstKey;
        }
      else
        {
          sprintf (szTemp, "Can't find type id\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);        
          rc = -2;
        }
    }

  return rc;
}

/*****************************************************************************
 *
 * Function name:  fpstLoyalCoServ_New
 *
 * Function call: 
 *
 * Returns:	       struct tostLoyalCoServ *
 *
 *                 Value on success:
 *
 *                 Pointer to new allocated structure or the old one
 *
 *		             Value on failure:
 *
 *                 NULL
 *
 * Arguments:      g_22, struct s_group_22 *
 *                 ppstInv, struct s_TimmInter *
 *
 * Description:    Encoding from EDI TIMM format 
 *
 *****************************************************************************
 */

static struct tostLoyalCoServ *fpstLoyalCoServ_New(struct s_group_22 *g_22,
                                                   struct s_TimmInter *ppstInv)
{
  int rc = 0;
  struct tostLoyalCoServ *lpstServ;
  char *lpszAmtLabel;
  double loflNetAmt;
  double loflTaxRate;
  char *lpszTaxRate;
  int n;
  struct s_group_22 *inv_g_22;
  
  lpstServ = (struct tostLoyalCoServ *)malloc(sizeof(struct tostLoyalCoServ));
  if (lpstServ == NULL)
    {
      sprintf (szTemp, "Can't malloc memory\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);               
      return NULL;
    }
  else
    {
      memset(lpstServ, 0x00, sizeof(struct tostLoyalCoServ));
    }

  /* get service net amount from actual g_22 item */
  lpszAmtLabel = fpszTimm_GetAmtType(g_22, "125", "9");
  if (lpszAmtLabel == NULL)
    {
      sprintf (szTemp, "Can't find net amount in SUM-SHEET\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);               
      return NULL;
    }
  else
    {
      n = sscanf(lpszAmtLabel, "%lf", &loflNetAmt);
      if (n != 1)
        {
          sprintf (szTemp, "Can't scan value: %s\n", lpszAmtLabel);
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);                         
          return NULL;
        }
      else
        {
          lpstServ->sostVal.soflNetAmt = loflNetAmt;
        }
    }

  if (loflNetAmt == 0.0)
    {
      lpstServ->sostVal.soflTaxAmt = 0.0;
    }
  else
  {
      /* find service in INV identified by LIN and PIA - OTNAME like */
      inv_g_22 = fpstTimm_GetMatchServ(ppstInv->timm->g_22, g_22);
      if (inv_g_22 == NULL)
      {
	  /* Tax will be 0 in this case */
	  /* The error is reported, but the processing is not interrupted */
	  
	  sprintf (szTemp, "Can't match g_22 service in TIMM INV\n");
	  fovdPrintLog(LOG_NORMAL, "%s zero tax rate assumed for the service category\n",szTemp);
	  
	  lpstServ->sostVal.soflTaxAmt = 0.0;
/*
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);               
          return NUL
*/
      }
      else
      {

	  /* get tax rate - only one for one g_22 item */
	  lpszTaxRate = fpszTimm_GetTaxRate(inv_g_22);
	  if (lpszTaxRate == NULL)
	  {
	      sprintf (szTemp, "Can't get tax rate\n");
	      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);               
	      return NULL;
	  }
	  else
	  {
	      n = sscanf(lpszTaxRate, "%lf", &loflTaxRate);
	      if (n != 1)
	      {
		  sprintf (szTemp, "Can't scan value: %s\n", lpszTaxRate);
		  macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);                         
		  return NULL;
	      }
	      else
	      {
		  lpstServ->sostVal.soflTaxAmt = loflNetAmt * loflTaxRate * 0.01;
	      }
	  }
      }
  }
  
  /* get type id searching rule list */  
  rc = foiGetTypeId(&(lpstServ->spstKey), g_22);
  if (rc != 0)
    {
      if (g_22->pia == NULL)
        {
          sprintf (szTemp, "Can't get type id for LIN: %s\n", 
                   g_22->lin->v_7140);
        }
      else
        {
          sprintf (szTemp, "Can't get type id for LIN: %s, PIA: %s\n",
                   g_22->lin->v_7140, 
                   g_22->pia->v_7140);
        }
      
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);                         
      return NULL;    
    }
  else
    {
      fovdPrintLog (LOG_DEBUG, "Assigned type: %ld\n", lpstServ->spstKey->solAmtTypeId);    
    }

  return lpstServ;
}

/*****************************************************************************
 *
 * Function name:  foiLoyalCoServNode_Add
 *
 * Function call: 
 *
 * Returns:	       int
 *
 *                 Value on success:
 *
 *                 0
 *
 *		             Value on failure:
 *
 *                 -1 -
 *                 -2 -
 *
 * Arguments:      ppstNode, struct tostLoyalCoNode *
 *                 ppstServ, struct tostLoyalCoServ *
 *
 * Description: 
 *
 *****************************************************************************
 */

int foiLoyalCoServNode_Add(struct tostLoyalCoServNode **ppstNode, 
                           struct tostLoyalCoServ *ppstServ)
{
  int rc = 0;
  int d;
  struct tostLoyalCoServNode *lpstNode;
  
  if (*ppstNode == NULL)
    {
      lpstNode = (struct tostLoyalCoServNode *)malloc(sizeof(struct tostLoyalCoServNode));
      if (lpstNode == NULL)
        {
          rc = -1;
        }
      else
        {
          memset(lpstNode, 0x00, sizeof(struct tostLoyalCoServNode));

          /* explicitely init */
          lpstNode->spstLeft = lpstNode->spstRight = NULL;
          lpstNode->spstServ = (struct tostLoyalCoServ *)malloc(sizeof(struct tostLoyalCoServ));
          if (lpstNode->spstServ == NULL)
            {
              rc = -2;              
            }
          else
            {
              /* copy all values, init charges, set key ptr  */
              memcpy(lpstNode->spstServ, ppstServ, sizeof(struct tostLoyalCoServ));
              /* use new node */
              *ppstNode = lpstNode;
            }
        }
    }
  else
    {
      /* use serv type id as the key */
      d = (*ppstNode)->spstServ->spstKey->solAmtTypeId - ppstServ->spstKey->solAmtTypeId;
      if (d < 0)
        {
          /* go left */
          rc = foiLoyalCoServNode_Add(&((*ppstNode)->spstLeft), ppstServ);
        }
      else if (d > 0)
        {
          /* go right */
          rc = foiLoyalCoServNode_Add(&((*ppstNode)->spstRight), ppstServ);
        }
      else
        {
          /* key conflict - node found */
          (*ppstNode)->spstServ->sostVal.soflNetAmt += ppstServ->sostVal.soflNetAmt;
          (*ppstNode)->spstServ->sostVal.soflTaxAmt += ppstServ->sostVal.soflTaxAmt;          
          free(ppstServ);
        }
    }

  return rc;
}

/*****************************************************************************
 *
 * Function name:  foiLoyalCoServTree_AddServ
 *
 * Function call: 
 *
 * Returns:	       int
 *
 *                 Value on success:
 *
 *                 0
 *
 *		             Value on failure:
 *
 *                 -1 -
 *                 -2 -
 *
 * Arguments:      ppstCoTree, struct tostLoyalCoTree *
 *                 ppstServ, struct tostLoyalCoServ *
 *
 * Description: 
 *
 *****************************************************************************
 */

static int foiLoyalCoServTree_AddServ(struct tostLoyalCoServTree *ppstTree,
                                      struct tostLoyalCoServ *ppstServ)
{
  int rc = 0;

  ppstTree->sostSumVal.soflNetAmt += ppstServ->sostVal.soflNetAmt;
  ppstTree->sostSumVal.soflTaxAmt += ppstServ->sostVal.soflTaxAmt;

  /* simplified version - no serv tree insert */
  if (stBgh.sochLoyalReportLevel == '0')
    {
      free(ppstServ);
    }
  else if (stBgh.sochLoyalReportLevel == '1')
    {
      /* extended version - insert to contract service tree */
      rc = foiLoyalCoServNode_Add(&(ppstTree->spstRoot), ppstServ);
      if (rc != 0)
        {
          sprintf (szTemp, "Can't add new service\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);              
          rc = -1;
        }
    }
  else
    {
      sprintf (szTemp, "Wrong report level: %c\n", stBgh.sochLoyalReportLevel);
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);              
      rc = -2;
    }

  return rc;
}

/*****************************************************************************
 *
 * Function name: foiSetOccAmt
 *
 * Function call: 
 *
 * Returns:	      int
 *
 *                Value on success:
 *
 *                0
 *
 *		            Value on failure:
 *
 *                -1 -
 *                -2 -
 *
 * Arguments:     ppstTree, struct tostLoyalCoServTree *
 *                ppstInfo, struct tostLoyalCoInfo *
 *
 * Description: 
 *
 *****************************************************************************
 */

static int foiSetOccAmt(struct tostLoyalCoServTree *ppstTree, 
                        struct s_group_22 *g_22,
                        struct s_TimmInter *ppstInv)
{
  int rc = 0;
  struct tostLoyalCoServ *lpstServ;

  /* create arificial service and add it to the tree */
  lpstServ = fpstLoyalCoServ_New(g_22, ppstInv);
  if (lpstServ == NULL)
    {
      rc = -1;
    }

  if (rc == 0)
    {
      rc = foiLoyalCoServTree_AddServ(ppstTree,
                                      lpstServ);
      if (rc != 0)
        {
          rc = -2;
        }
      else
        {
          fovdPrintLog (LOG_DEBUG, "Added OCC service\n");
        }
    }
  
  return rc;
}

/*****************************************************************************
 *
 * Function name:  foiLoyalCoServNode_Save
 *
 * Function call: 
 *
 * Returns:	       int
 *
 *                 Value on success:
 *
 *                 0
 *
 *		             Value on failure:
 *
 *                 -1 -
 *                 -2 -
 *                 -3 -
 *
 * Arguments:      ppstNode, struct tostLoyalCoServNode *
 *                 ppstInvInfo, struct tostLoyalInvInfo *
 *                 ppstCoInfo, struct tostLoyalCoInfo *
 *
 * Description: 
 *
 *****************************************************************************
 */

int foiLoyalCoServNode_Save(struct tostLoyalCoServNode *ppstNode,
                            struct tostLoyalInvInfo *ppstInvInfo,
                            struct tostLoyalCoInfo *ppstCoInfo)
{
  int rc = 0;

  if (ppstNode == NULL)
    {
      return rc;
    }
  else
    {
      rc = foiEdsBghLoyalTrl_ServInsert(ppstInvInfo, 
                                        ppstCoInfo, 
                                        ppstNode->spstServ);
      if (rc != 0)
        {
          rc = -1;
        }
      else
        {
          rc = foiLoyalCoServNode_Save(ppstNode->spstLeft, 
                                       ppstInvInfo, 
                                       ppstCoInfo);
          if (rc != 0)
            {
              rc = -2;
            }
          else
            {
              rc = foiLoyalCoServNode_Save(ppstNode->spstRight, 
                                           ppstInvInfo, 
                                           ppstCoInfo);
              if (rc != 0)
                {
                  rc = -3;
                }      
            }
        }
    }

  return rc;
}

/*****************************************************************************
 *
 * Function name:  foiLoyalCoServTree_Save
 *
 * Function call: 
 *
 * Returns:	       int
 *
 *                 Value on success:
 *
 *                 0
 *
 *		             Value on failure:
 *
 *                 -1
 *
 * Arguments:      ppstTree, struct tostLoyalCoServTree *
 *                 ppstInvInfo, struct tostLoyalInvInfo *
 *                 ppstCoInfo, struct tostLoyalCoInfo *
 *
 * Description: 
 *
 *****************************************************************************
 */

int foiLoyalCoServTree_Save(struct tostLoyalCoServTree *ppstTree,
                            struct tostLoyalInvInfo *ppstInvInfo,
                            struct tostLoyalCoInfo *ppstCoInfo)
{
  int rc = 0;

  rc = foiLoyalCoServNode_Save(ppstTree->spstRoot, ppstInvInfo, ppstCoInfo);
  if (rc != 0)
    {
      rc = -1;
    }

  return rc;
}

/*****************************************************************************
 *
 * Function name:  foiLoyalCoNode_Save
 *
 * Function call: 
 *
 * Returns:	       int
 *
 *                 Value on success:
 *
 *                 0
 *
 *		             Value on failure:
 *
 *                 -1 -
 *                 -2 -
 *                 -3 -
 *                 -4 -
 *                 -5 -
 *                 -6 -
 *
 * Arguments:      polLoyalHdrId, long
 *                 ppstCoTree, struct tostLoyalCoTree *
 *
 * Description: 
 *
 *****************************************************************************
 */

static int foiLoyalCoNode_Save(struct tostLoyalInvInfo *ppstInvInfo,
                               struct tostLoyalCoNode *ppstNode)
{
  int rc = 0;
  
  if (ppstNode == NULL)
    {
      return rc;
    }
  else
    {
      fovdPrintLog (LOG_DEBUG, "Node:\n");

      fovdPrintLog (LOG_DEBUG, "Node Co Info co id    : %ld\n", 
                    ppstNode->sostCoInfo.solCoId);

      fovdPrintLog (LOG_DEBUG, "Node Co Info dn num   : %s\n",  
                    ppstNode->sostCoInfo.spszDnNum);

      fovdPrintLog (LOG_DEBUG, "Node Co Serv Tree net : %lf\n", 
                    ppstNode->sostCoInfo.sostCoServTree.sostSumVal.soflNetAmt);

      fovdPrintLog (LOG_DEBUG, "Node Co Serv Tree tax : %lf\n", 
                    ppstNode->sostCoInfo.sostCoServTree.sostSumVal.soflTaxAmt);

      rc = foiEdsBghLoyalCo_Insert(ppstInvInfo,
                                   &(ppstNode->sostCoInfo));
      if (rc != 0)
        {
          rc = -1;
        }
      else
        {
          if (stBgh.sochLoyalReportLevel == '0')
            {
              /* save contract summary charges */
              rc = foiEdsBghLoyalTrl_SumInsert(ppstInvInfo,
                                               &(ppstNode->sostCoInfo),
                                               0L);
              if (rc != 0)
                {
                  rc = -2;
                }
            }
          else if (stBgh.sochLoyalReportLevel == '1')
            {
              /* usage extended grouping configured with DB */
              if (rc == 0)
                {
                  rc = foiLoyalCoServTree_Save(&(ppstNode->sostCoInfo.sostCoServTree),
                                               ppstInvInfo,
                                               &(ppstNode->sostCoInfo));
                  if (rc != 0)
                    {
                      rc = -3;
                    }
                }
            }          
          else
            {
              sprintf (szTemp, "Wrong report level: %c\n", stBgh.sochLoyalReportLevel);
              macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);              
              rc = -4;              
            }

          if (rc == 0)
            {
              rc = foiLoyalCoNode_Save(ppstInvInfo,
                                       ppstNode->spstLeft);
              if (rc != 0)
                {
                  rc = -5;
                }
              else
                {
                  rc = foiLoyalCoNode_Save(ppstInvInfo,
                                           ppstNode->spstRight);
                  if (rc != 0)
                    {
                      rc = -6;
                    }
                }
            }
        }
    }
  
  return rc;
}

/*****************************************************************************
 *
 * Function name:  foiLoyalCoTree_Save
 *
 * Function call: 
 *
 * Returns:	       int
 *
 *                 Value on success:
 *
 *                 0
 *
 *		             Value on failure:
 *
 *                 -1 -
 *                 -2 -
 *
 * Arguments:      ppstCoTree, struct tostLoyalCoTree *
 *
 * Description: 
 *
 *****************************************************************************
 */

static int foiLoyalCoTree_Save(struct tostLoyalCoTree *ppstCoTree)
{
  int rc = 0;
  
  fovdPrintLog (LOG_DEBUG, "Tree:\n");

  fovdPrintLog (LOG_DEBUG, "Tree Inv Info customer id: %ld\n", 
                ppstCoTree->sostInvInfo.solCustId);

  fovdPrintLog (LOG_DEBUG, "Tree Inv Info custcode   : %s\n", 
                ppstCoTree->sostInvInfo.spszCustCode);

  fovdPrintLog (LOG_DEBUG, "Tree Inv Info ohrefnum   : %s\n", 
                ppstCoTree->sostInvInfo.spszOhRefNum);

  fovdPrintLog (LOG_DEBUG, "Tree Inv Info ohrefdate  : %s\n", 
                ppstCoTree->sostInvInfo.spszOhRefDate);
  
  rc = foiBghLoyalHdr_Insert(&(ppstCoTree->sostInvInfo));
  if (rc != 0)
    {
      rc = -1;
    }
  else
    {
      rc = foiLoyalCoNode_Save(&(ppstCoTree->sostInvInfo),
                               ppstCoTree->spstRoot);
      if (rc != 0)
        {
          rc = -2;
        }
    }

  return rc;
}

/*****************************************************************************
 *
 * Function name:  foiLoyalCoServNode_Delete
 *
 * Function call: 
 *
 * Returns:	       int
 *
 *                 Value on success:
 *
 *                 0
 *
 *		             Value on failure:
 *
 *                 -1 -
 *                 -2 -
 *
 * Arguments:     ppstNode, struct tostLoyalCoServNode *
 *
 * Description: 
 *
 *****************************************************************************
 */

int foiLoyalCoServNode_Delete(struct tostLoyalCoServNode *ppstNode)
{
  int rc = 0;

  if (ppstNode == NULL)
    {
      return rc;
    }
  else
    {
      rc = foiLoyalCoServNode_Delete(ppstNode->spstLeft);
      if (rc != 0)
        {
          rc = -1;
        }
      else
        {
          rc = foiLoyalCoServNode_Delete(ppstNode->spstRight);
          if (rc != 0)
            {
              rc = -2;
            }
        }
    }

  if (rc == 0)
    {
      free(ppstNode);
    }

  return rc;
}

/*****************************************************************************
 *
 * Function name:  foiLoyalCoNode_Delete
 *
 * Function call: 
 *
 * Returns:	       int
 *
 *                 Value on success:
 *
 *                 0
 *
 *		             Value on failure:
 *
 *                 -1 -
 *                 -2 -
 *                 -3 -
 *
 * Arguments:     ppstCoTree, struct tostLoyalCoTree *
 *
 * Description: 
 *
 *****************************************************************************
 */

static int foiLoyalCoNode_Delete(struct tostLoyalCoNode *ppstNode)
{
  int rc = 0;
  
  if (ppstNode == NULL)
    {
      return rc;
    }
  else
    {
      rc = foiLoyalCoServNode_Delete(ppstNode->sostCoInfo.sostCoServTree.spstRoot);
      if (rc != 0)
        {
          rc = -1;
        }
      else
        {
          rc = foiLoyalCoNode_Delete(ppstNode->spstLeft);      
          if (rc != 0)
            {
              rc = -2;
            }
          else
            {
              rc = foiLoyalCoNode_Delete(ppstNode->spstRight);
              if (rc != 0)
                {
                  rc = -3;
                }
            }
        }
    }

  if (rc == 0)
    {
      free(ppstNode);
    }
  
  return rc;
}

/*****************************************************************************
 *
 * Function name:  foiLoyalCoTree_Delete
 *
 * Function call: 
 *
 * Returns:	   int
 *
 *                 Value on success:
 *
 *                 0
 *
 *		   Value on failure:
 *
 *                 -1
 *
 * Arguments:     ppstCoTree, struct tostLoyalCoTree *
 *
 * Description: 
 *
 *****************************************************************************
 */

static int foiLoyalCoTree_Delete(struct tostLoyalCoTree *ppstCoTree)
{
  int rc = 0;

  rc = foiLoyalCoNode_Delete(ppstCoTree->spstRoot);
  if (rc != 0)
    {
      rc = -1;
    }

  if (rc == 0)
    {
      free(ppstCoTree);
    }

  return rc;
}

/*****************************************************************************
 *
 * Function name: foiLoyal_ProcServInfo
 *
 * Function call: 
 *
 * Returns:	  int
 *
 *                Value on success:
 *
 *                0
 *
 *		  Value on failure:
 *
 *                -1 - 
 *                -2 -
 *                -3 -
 *                -4 -
 *                -5 -
 *                -6 -
 *                -7 -
 *                -8 -
 *                -9 -
 *
 * Arguments:     pstColumn, stDBCOL *
 *                ppstInv, struct s_TimmInter *
 *                ppstSum, struct s_TimmInter *
 *
 * Description: 
 *
 *****************************************************************************
 */

int foiLoyal_ProcServInfo(stDBCOL *pstColumn,
                          struct s_TimmInter *ppstInv, 
                          struct s_TimmInter *ppstSum)
{
  int rc = 0;
  struct s_group_22 *g_22;
  struct s_lin_seg *lin;
  struct tostLoyalCoTree *lpstCoTree = NULL;
  struct tostLoyalCoServTree *lpstCoServTree = NULL;
  struct tostLoyalCoServ *lpstServ = NULL;
  struct tostLoyalCoInfo lostCoInfo;
  double loflDisAmt = 0.0;
  toenBool loenFound;

  if (stBgh.sochLoyalReportLevel != '0' && stBgh.sochLoyalReportLevel != '1')
    {
      return rc;
    }

  /* creating empty contract tree */
  lpstCoTree = fpstLoyalCoTree_New(ppstInv);
  if (lpstCoTree == NULL)
    {
      sprintf (szTemp, "Can't create new loyal contract tree\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      rc = -1;
    }
  else
    {
      fovdPrintLog (LOG_DEBUG, 
                    "Proc loyal info for customer: %ld, %s, %s, %s\n",
                    lpstCoTree->sostInvInfo.solCustId,
                    lpstCoTree->sostInvInfo.spszCustCode,
                    lpstCoTree->sostInvInfo.spszOhRefNum,
                    lpstCoTree->sostInvInfo.spszOhRefDate);
    }
  
  /* for each group 22 find contract serv tree and add all services */
  if (rc == 0)
    {
      g_22 = ppstSum->timm->g_22;
      while (g_22)
        {
          lin = g_22->lin;      
          switch (lin->v_1222[1])
            {
            case '2': /* contract or customer OCC level */
              /* fill the structure with TIMM info */
              rc = foiGetCoInfo(g_22, &lostCoInfo);
              if (rc != 0)
                {
                  sprintf (szTemp, "Can't get contract info from g_22: %s, %s, %s\n", 
                           g_22->lin->v_1082,
                           g_22->lin->v_7140,
                           g_22->lin->v_1222);
                  
                  macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
                  rc = -2;
                }
              else
                {
                  fovdPrintLog (LOG_DEBUG, "New contract: %ld, dnnum: %s\n", 
                                lostCoInfo.solCoId,
                                lostCoInfo.spszDnNum);

                  /* recursively search the tree of contracts */
                  /* copy info to the new co node and creates */
                  /* new empty tree */
                  lpstCoServTree = fpstLoyalCoTree_Find(lpstCoTree,
                                                        &lostCoInfo);
                  if (lpstCoServTree == NULL)
                    {
                      sprintf (szTemp, "Can't find contract node in contract tree\n");
                      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
                      rc = -3;
                    }
                }

              if (rc == 0)
                {
                  if (lostCoInfo.solCoId == -1) /* payment resp. OCC */
                    {
                      rc = foiSetOccAmt(lpstCoServTree, g_22, ppstInv);
                      if (rc != 0)
                        {
                          sprintf (szTemp, "Can't get payment responsible OCC info\n");
                          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);               
                          rc = -4;
                        }
                    }
                }

              break;
              
            case '4': /* current contract service */
              lpstServ = fpstLoyalCoServ_New(g_22, ppstInv);
              if (lpstServ == NULL)
                {
                  sprintf (szTemp, "Can't create new service info\n");
                  macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
                  rc = -5;
                }
              else
                {
                  if (g_22->pia != NULL)
                    {
                      fovdPrintLog (LOG_DEBUG, "New service LIN: %s, PIA: %s\n",
                                    g_22->lin->v_7140,
                                    g_22->pia->v_7140);
                    }
                  else
                    {
                      fovdPrintLog (LOG_DEBUG, "New service LIN: %s\n",
                                    g_22->lin->v_7140);                                    
                    }

                  /* adds new service or updates the existing one */
                  /* the structure may be deallocated in the func */
                  rc = foiLoyalCoServTree_AddServ(lpstCoServTree,
                                                  lpstServ);
                  if (rc != 0)
                    {
                      sprintf (szTemp, "Can't add service info\n");
                      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
                      rc = -6;
                    }
                  else
                    {
                      fovdPrintLog (LOG_DEBUG, "Added new service\n");
                    }
                }
              
              break;
            } /* switch */
      
          if (rc != 0)
            {
              break;
            }
          
          g_22 = g_22->g_22_next;
        } /* while */
    }
  
  /* try to clean previous results */
  if (rc == 0)
    {
      rc = foiEdsBghLoyal_Clean(&(lpstCoTree->sostInvInfo));
      if (rc != 0)
        {
          sprintf (szTemp, "Can't clean Loyal tables\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          rc = -7;
        }
    }
    

  /* recursively save the co tree to the DB */  
  if (rc == 0)
    {
      rc = foiLoyalCoTree_Save(lpstCoTree);
      if (rc != 0)
        {
          sprintf (szTemp, "Can't save loyal contract tree\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          rc = -8;
        }
      else
        {
          fovdPrintLog (LOG_DEBUG, "Saved loyal contract tree\n");
        }
    }

  /* handle discount */

  if (rc == 0)
    {
      rc = foiLoyal_ProcDiscount(ppstInv, &(lpstCoTree->sostInvInfo));
      if (rc != 0)
        {
          sprintf (szTemp, "Can't process discount\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          rc = -9;
        }
    }

  /* destroy the tree */
  if (rc == 0)
    {
      rc = foiLoyalCoTree_Delete(lpstCoTree);
      if (rc != 0)
        {
          sprintf (szTemp, "Can't delete loyal contract tree\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          rc = -10;
        }
      else
        {
          fovdPrintLog (LOG_DEBUG, "Deleted loyal contract tree\n");
        }
    }
  
  return rc; 
}





