/**************************************************************************/
/*  MODULE : ROAMING data fetecher                                        */
/*                                                                        */
/*  AUTHOR : N.Bondarczuk                                                 */
/*                                                                        */
/*  CREATION DATE : 10.11.97                                              */
/*                                                                        */
/*  DESCRIPTION : Exports  functions accessing information from TIMM      */
/*                ROAMING   message.                                      */ 
/**************************************************************************/

#include <stdio.h>
#include <string.h>
#include "bgh.h"
#include "protos.h"
#include "parser.h"

#include "types.h"
#include "gen.h"
#include "sum_fetch_sim.h"
#include "roa_fetch_sim.h"

#if 0	/* just for version.sh */
static char *SCCS_VERSION = "1.1";
#endif

extern struct s_imd_seg *lpstFindItemDescription(struct s_imd_seg *, char *);
extern struct s_moa_seg *lpstFindPaymentSegment(struct s_group_23 *, char *);

#ifdef _DEBUG_ROA_FETCH_SIM_
#define _DEBUG_
#endif


/*
 * Of no use now
 */

int foiCountRoamingContracts(struct s_TimmInter *spstRaomingSheet)
{
  return 0;
}

toenBool foenFetchRoamingSim(struct s_TimmInter *spstRoamingSheet, int poiIndex, char *pachzSim, int poiSimLen)
{
  return FALSE;
}


int foiCountRoamingContractVPLMNs(struct s_TimmInter *ppstRoamingSheet, char *pachzSim, int poiSimLen)
{
  struct s_lin_seg *lpstLin;
  struct s_group_22 *lpstG22;
  struct s_group_22 *lpstFindRoamingContractSegment(struct s_group_22 *, char *);
  int loiItems;

  fovdPushFunctionName("foiCountRoamingContractVPLMNs");

  /*
   * Find  block on level 01 identified by SIM number
   */

  lpstG22 = ppstRoamingSheet->timm->g_22;
  lpstG22 = lpstFindRoamingContractSegment(lpstG22, pachzSim);
  if (lpstG22 == NULL)
    {
      fovdPopFunctionName();
      return 0;
    }

  /* 
   *Count all blocks level 02
   */

  lpstG22 = lpstG22->g_22_next;
  loiItems = 0;
  while (lpstG22)
    {
      lpstLin = lpstG22->lin;      
      if (lpstLin && lpstLin->v_1222)
        {
          if (EQ(lpstLin->v_1222, "02"))
            {
              loiItems++;
            }
          else
            {
              break;
            }
        }
      lpstG22 = lpstG22->g_22_next;
    }

  fovdPopFunctionName();
  return loiItems;
}

toenBool foenFetchRoamingContractVPLMN(struct s_TimmInter *spstRoamingSheet, char *pachzSim, int poiSimLen, int poiIndex, tostVPLMN *postVPLMN)
{
  struct s_group_22 *lpstG22;
  struct s_group_23 *lpstG23;
  int loiItems, loiIndex;
  struct s_moa_seg *lpstMoa;
  struct s_imd_seg *lpstImd;
  struct s_lin_seg *lpstLin;
  struct s_group_22 *lpstFindRoamingContractSegment(struct s_group_22 *, char *);


  fovdPushFunctionName("foenFetchRomaingContractVPLMN");

 
  lpstG22 = lpstFindRoamingContractSegment(spstRoamingSheet->timm->g_22, pachzSim);
  if (lpstG22 == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }
  fovdPrintLog (LOG_DEBUG, "SIM: %s, %d\n", pachzSim, poiIndex);

  /*
   * Find VPLMN description block 
   */
  lpstG22 = lpstG22->g_22_next;
  loiIndex = 0;
  while (lpstG22)
    {
      fovdPrintLog (LOG_DEBUG, "G22: %d\n", loiIndex);
      lpstLin = lpstG22->lin;
      if (EQ(lpstLin->v_1222, "01"))
        {
          fovdPrintLog (LOG_DEBUG, "OUT\n");
          break;
       }
      else if (EQ(lpstLin->v_1222, "02") && loiIndex == poiIndex)
        {
          fovdPrintLog (LOG_DEBUG, "FOUND\n");
          break;
        }
      else
        {
          fovdPrintLog (LOG_DEBUG, "NEXT\n");
          loiIndex++;
          lpstG22 = lpstG22->g_22_next;
        }
    }

  if (loiIndex != poiIndex || EQ(lpstLin->v_1222, "01"))
    {
      fovdPopFunctionName();
      return FALSE;
    }
  
  /*
   * VPLMN
   */
  
  lpstImd = lpstG22->imd;
  lpstImd = lpstFindItemDescription(lpstImd, "VPLMN");
  if (lpstImd == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }
  strncpy(postVPLMN->laszName, lpstImd->v_7008a, MAX_BUFFER);
  fovdPrintLog (LOG_DEBUG, "NAME: %s\n", postVPLMN->laszName);

  
  lpstG23 = lpstG22->g_23;

  /*
   * net amount charged by VPLMN for TAMOCS
   */
  lpstMoa = lpstFindPaymentSegment(lpstG23, "940");
  if (lpstMoa == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }
  strncpy(postVPLMN->laszNetInboundAmount, lpstMoa->v_5004, MAX_BUFFER);
  strncpy(postVPLMN->laszNetInboundCurrency, lpstMoa->v_6345, MAX_BUFFER);
  fovdPrintLog (LOG_DEBUG, "MOA+940\n");

  /*
   * net amount charged by VPLMN for TAMTC
   */
  lpstMoa = lpstFindPaymentSegment(lpstG23, "944");
  if (lpstMoa == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }
  strncpy(postVPLMN->laszNetOutboundAmount, lpstMoa->v_5004, MAX_BUFFER);
  strncpy(postVPLMN->laszNetOutboundCurrency, lpstMoa->v_6345, MAX_BUFFER);
  fovdPrintLog (LOG_DEBUG, "MOA+944\n");

  /*
   * tax amount charged by VPLMN
   */
  lpstMoa = lpstFindPaymentSegment(lpstG23, "941");
  if (lpstMoa == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }
  strncpy(postVPLMN->laszChargedTaxAmount, lpstMoa->v_5004, MAX_BUFFER);
  strncpy(postVPLMN->laszChargedTaxCurrency, lpstMoa->v_6345, MAX_BUFFER);
  fovdPrintLog (LOG_DEBUG, "MOA+941\n");  

  /*
   *  usage amount 
   */
  lpstMoa = lpstFindPaymentSegment(lpstG23, "942");
  if (lpstMoa == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }
  strncpy(postVPLMN->laszUsageAmount, lpstMoa->v_5004, MAX_BUFFER);
  strncpy(postVPLMN->laszUsageCurrency, lpstMoa->v_6345, MAX_BUFFER);
  fovdPrintLog (LOG_DEBUG, "MOA+942\n");

  /*
   * surcharge amount
   */
  lpstMoa = lpstFindPaymentSegment(lpstG23, "943");
  if (lpstMoa == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }
  strncpy(postVPLMN->laszSurchargeAmount, lpstMoa->v_5004, MAX_BUFFER);
  strncpy(postVPLMN->laszSurchargeCurrency, lpstMoa->v_6345, MAX_BUFFER);
  fovdPrintLog (LOG_DEBUG, "MOA+943\n");  

  fovdPopFunctionName();
  return TRUE;
}



/*
 * LOCAL
 */

struct s_group_22 *lpstFindRoamingContractSegment(struct s_group_22 *ppstG22, char *ppszSim)
{
  struct s_group_22 *lpstG22;
  struct s_lin_seg *lpstLin;
  struct s_imd_seg *lpstImd;

  fovdPushFunctionName("lpstFindRoamingContractSegment");

  lpstG22 = ppstG22;

  while (lpstG22)
    {
      fovdPrintLog (LOG_DEBUG, "LIN block\n");
      lpstLin = lpstG22->lin;
      if (lpstLin && lpstLin->v_1222 && EQ(lpstLin->v_1222, "01"))
        {
          fovdPrintLog (LOG_DEBUG, "IMD block on level 01\n");
          lpstImd = lpstFindItemDescription(lpstG22->imd, "SMNUM");
          if (lpstImd != NULL && EQ(lpstImd->v_7008a, ppszSim))
            {
              fovdPrintLog (LOG_DEBUG, "SMNUM found : %s\n", ppszSim);              
              fovdPopFunctionName();
              return lpstG22;
            }
        }
      lpstG22 = lpstG22->g_22_next;
    }

  fovdPopFunctionName();
  return NULL;
}


