#include <strings.h>
#include "types.h"
#include "gen.h"
#include "ing_timm.h"
#include "ing_gen.h"
#include "strutl.h"
#include "num_pl.h"
#include "interest.h"
#include "turnover.h"

toenBool foenINPEnvelopeGen();
toenBool foenINPHeaderGen();
toenBool foenINPTrailerGen();

static struct s_TimmInter *spstTimmInter;

/*
 * Main procedure of generator
 */
     
toenBool foenINPGen(struct s_TimmInter *ppstTimmInter, toenBool poenAddEnvelope) 
{  
  toenBool loenStatus;
  
  /*
   * Is it a correct TIMM message?
   */

  if (NOT(EQ(ppstTimmInter->timm->bgm->v_1000 ,"INH-INP")))
    {
      return FALSE;
    }

  /*
   * set up module variables
   */

  spstTimmInter = ppstTimmInter;

  if (poenAddEnvelope == TRUE) 
    {
      fovdPrintLog (LOG_DEBUG, "Starting envelope\n");
      if ((loenStatus = foenINPEnvelopeGen()) == FALSE)
        {          
          return FALSE;
        }
    }

  fovdGen("InterestNotePeriodicStart", EOL); 

  /*
   * Header
   */
  fovdPrintLog (LOG_DEBUG, "Starting header\n");
  if ((loenStatus = foenINPHeaderGen()) == FALSE)
    {
      return FALSE;
    }

  /*
   * Trailer
   */
  fovdPrintLog (LOG_DEBUG, "Starting trailer\n");
  if ((loenStatus = foenINPTrailerGen()) == FALSE)
    {
      return FALSE;
    }
 
  /*
   * Interest table
   */
  fovdPrintLog (LOG_DEBUG, "Starting  interest list\n");
  if ((loenStatus = foenInterestList_Gen(ppstTimmInter)) == FALSE)
    {
      return FALSE;
    }

  /*
   * Turnover table
   */
  fovdPrintLog (LOG_DEBUG, "Starting turnover list\n");
  if ((loenStatus = foenTurnoverList_Gen(ppstTimmInter)) == FALSE)
    {
      return FALSE;
    }


  fovdGen("InterestNotePeriodicEnd", EOL); 

  return TRUE;
}


toenBool foenINPEnvelopeGen()
{
  struct s_group_2 *lpstG2;
  struct s_nad_seg *lpstNAD;

  lpstG2 = spstTimmInter->timm->g_2;
  while (lpstG2 != NULL)
    {
      lpstNAD = lpstG2->nad;
      if (EQ(lpstNAD->v_3035, "IT"))
        {
          break;
        }

      lpstG2 = lpstG2->g_2_next;
    }
  if (lpstNAD == NULL)
    {
      return FALSE;
    }

  fovdGen("EnvelopeStart", EOL);
  
  fovdGen("EnvelopeAddress", 
          /*
          lpstNAD->v_3036, 
          */
          lpstNAD->v_3036a, 
          lpstNAD->v_3036b, 
          lpstNAD->v_3036c, 
          lpstNAD->v_3036d, 
          EOL);
  
  fovdGen("EnvelopeEnd", EOL);

  return TRUE;
}

/*
 * Header of document
 */

toenBool foenINPHeaderGen()
{
  struct s_group_2 *lpstG2;
  struct s_group_22 *lpstG22;
  struct s_nad_seg *lpstNAD;
  struct s_fii_seg *lpstFII;
  struct s_rff_seg *lpstRFF;
  struct s_dtm_seg *lpstDTM;

  static char lpchzDate[MAX_BUFFER];

  /*
   * XARGS = Find DTM+990 with date of letter
   */

  lpstDTM = spstTimmInter->timm->dtm;
  while (lpstDTM != NULL)
    {
      if (EQ(lpstDTM->v_2005, "990"))
        {
          break;
        }

      lpstDTM = lpstDTM->dtm_next;
    }

  if (lpstDTM == NULL)
    {
      return FALSE;
    }
  
  strcpy(lpchzDate, lpstDTM->v_2380);
  fovdFormatDate(lpchzDate, YY_MM_DD);
  fovdGen("InterestNotePeriodicDate", lpchzDate, EOL);
  
  /*
   * XARGS = Find NAD+IT of sender
   */

  lpstG2 = spstTimmInter->timm->g_2;
  while (lpstG2 != NULL)
    {
      lpstNAD = lpstG2->nad;
      if (EQ(lpstNAD->v_3035, "II"))
        {
          break;
        }

      lpstG2 = lpstG2->g_2_next;
    }

  if (lpstNAD == NULL)
    {
      return FALSE;
    }
  
  fovdGen("InterestNotePeriodicSender", 
          lpstNAD->v_3036, 
          lpstNAD->v_3036a, 
          lpstNAD->v_3036b, 
          lpstNAD->v_3036c, 
          lpstNAD->v_3036d, EOL);
  
  /*
   * XARGS = Find FII+RH in G2 which contains NAD+IT
   */

  lpstFII = lpstG2->fii;
  if (lpstFII == NULL)
    {
      return FALSE;
    }
  
  if (NOT(EQ(lpstFII->v_3035, "RH")))
    {
      return FALSE;
    }

  fovdGen("InterestNotePeriodicBank", 
          lpstFII->v_3194, 
          lpstFII->v_3432, 
          lpstFII->v_3436, EOL);

  /*
   * XARGS = Find NAD+IV of customer
   */
  
  lpstG2 = spstTimmInter->timm->g_2;
  while (lpstG2 != NULL)
    {
      lpstNAD = lpstG2->nad;
      if (EQ(lpstNAD->v_3035, "IV"))
        {
          break;
        }

      lpstG2 = lpstG2->g_2_next;
    }

  if (lpstNAD == NULL)
    {
      return FALSE;
    }

  fovdGen("InterestNotePeriodicCustomer", 
          lpstNAD->v_3036, 
          lpstNAD->v_3036a, 
          lpstNAD->v_3036b, 
          lpstNAD->v_3036c, 
          lpstNAD->v_3036d, EOL);

  /*
   * XARGS = Find RFF+IR - note number
   */

  lpstRFF = fpstFindReference(lpstG2->g_3, "IR"); 
  if (lpstRFF == NULL)
    {
      return FALSE;
    }
  
  fovdGen("InterestNotePeriodicNumber", lpstRFF->v_1154, EOL);

  /*
   * XARGS = Find RFF+IT - customer's account 
   */

  lpstRFF = fpstFindReference(lpstG2->g_3, "IT"); 
  if (lpstRFF == NULL)
    {
      return FALSE;
    }

  fovdGen("InterestNotePeriodicAccount", lpstRFF->v_1154, EOL);

  return TRUE;
}

toenBool foenINPTrailerGen()
{
  struct s_group_45 *lpstG45;
  struct s_moa_seg *lpstMOA;
  static char lpchzAmount[MAX_BUFFER], lpchzWords[MAX_BUFFER];
  int rc;
  
  /*
   * XARGS = Find MOA+1010 in G45
   */
  
  lpstMOA = fpstFindMainPaymentSegment(spstTimmInter->timm->g_45, "610");
  if (lpstMOA == NULL)
    {
      return FALSE;
    }
  
  strcpy(lpchzAmount, lpstMOA->v_5004);
  fovdFormatMoney(lpchzAmount);
  fovdGen("InterestNotePeriodicPayment", lpchzAmount, lpstMOA->v_6345, EOL);
  
  strcpy(lpchzAmount, lpstMOA->v_5004);
  rc = moa2str_pl(lpchzAmount, MAX_BUFFER, lpchzWords, MAX_BUFFER);
  fovdGen("InterestNotePeriodicPaymentWords", lpchzWords, EOL);

  return TRUE;
}
