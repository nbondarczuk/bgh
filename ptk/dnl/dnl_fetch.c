/**************************************************************************/
/*  MODULE : DUNNING data fetecher                                        */
/*                                                                        */
/*  AUTHOR : N.Bondarczuk                                                 */
/*                                                                        */
/*  CREATION DATE : 25.11.97                                              */
/*                                                                        */
/*  DESCRIPTION : Exports  functions accessing information from TIMM      */
/*                DUNNING message.                                        */ 
/**************************************************************************/

#include <stdio.h>
#include <string.h>
#include "bgh.h"
#include "protos.h"
#include "parser.h"

#include "types.h"
#include "gen.h"
#if 0	/* just for version.sh */
static char *SCCS_VERSION = "4.1";
#endif

int foiCountDunningLetterTexts(struct s_TimmInter *ti)
{
  struct s_ftx_seg *ftx;
  int loiText;

  fovdPushFunctionName("foenCountDunningLetterTexts");

  loiText = 0;
  ftx = ti->timm->ftx;
  while (ftx)
    {
      loiText++;
      ftx = ftx->ftx_next;
    }

  fovdPopFunctionName();

  return loiText;
}


toenBool foenFetchDunningLetterText(struct s_TimmInter *ti, int poiTextNo, char pachzLine[5][MAX_BUFFER])
{
  struct s_ftx_seg *ftx;
  int loiText;

  fovdPushFunctionName("foenFetchDunningLetterText");

  loiText = 0;
  ftx = ti->timm->ftx;
  while (ftx)
    {
      if (loiText == poiTextNo)
        {
          if (ti->timm->ftx)
            {
              strncpy(pachzLine[0], ftx->v_4440, MAX_BUFFER);
              strncpy(pachzLine[1], ftx->v_4440a, MAX_BUFFER);
              strncpy(pachzLine[2], ftx->v_4440b, MAX_BUFFER);
              strncpy(pachzLine[3], ftx->v_4440c, MAX_BUFFER);
              strncpy(pachzLine[4], ftx->v_4440d, MAX_BUFFER);

              fovdPopFunctionName();

              return TRUE;
            }
          else
            {
              pachzLine[0][0] = '\0'; 
              pachzLine[1][0] = '\0'; 
              pachzLine[2][0] = '\0'; 
              pachzLine[3][0] = '\0'; 
              pachzLine[4][0] = '\0'; 
            }
          break;
        }
      loiText++;
      ftx = ftx->ftx_next;
    }

  fovdPopFunctionName();
  
  return FALSE;
}


toenBool foenFetchDunningLetterCustomerAccountNo(struct s_TimmInter *ppstDunningLetter, char *pachzAccountNo, int poiAccountNoLen)
{
  struct s_group_1 *g_1;
  struct s_rff_seg *rff;
  struct s_rff_seg *lpstFindMainReference(struct s_group_1 *, char *);   

  fovdPushFunctionName("foenFetchDunningLetterCustomerAccountNo");

  rff = lpstFindMainReference(ppstDunningLetter->timm->g_1, "IT");
  if (rff == NULL)
    {
      fovdPopFunctionName();
      
      return FALSE;
    }
  
  strncpy(pachzAccountNo, rff->v_1154, poiAccountNoLen);
  
  fovdPopFunctionName();

  return TRUE;
}


toenBool foenFetchDunningLevel(struct s_TimmInter *ppstDunningLetter, char *pachzLevel, int poiLevelLen)
{
  struct s_rff_seg *rff;
  struct s_rff_seg *lpstFindMainReference(struct s_group_1 *, char *);   

  fovdPushFunctionName("foenFetchDunningLetterCustomerAccountNo");

  rff = lpstFindMainReference(ppstDunningLetter->timm->g_1, "ND");
  if (rff == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }
  
  strncpy(pachzLevel, rff->v_1154, poiLevelLen);
  
  fovdPopFunctionName();
  return TRUE;
}



toenBool foenFetchDunningLetterInvoiceNo(struct s_TimmInter *ppstDunningLetter, char *pachzInvoiceNo, int poiInvoiceNoLen)
{
  struct s_group_1 *g_1;
  struct s_rff_seg *rff;
  struct s_rff_seg *lpstFindMainReference(struct s_group_1 *, char *);   

  fovdPushFunctionName("foenFetchDunningLetterInvoiceNo");

  rff = lpstFindMainReference(ppstDunningLetter->timm->g_1, "IV");
  if (rff == NULL)
    {
      fovdPopFunctionName();
      
      return FALSE;
    }
  
  strncpy(pachzInvoiceNo, rff->v_1154, poiInvoiceNoLen);
  
  fovdPopFunctionName();

  return TRUE;
}


toenBool foenFetchDunningLetterInvoiceAmount(struct s_TimmInter *ppstDunningLetter, char *pachzAmount, int poiAmountLen, char *pachzCurrency, int poiCurrencyLen)
{
  struct s_moa_seg *lpstMoa;
  struct s_moa_seg *lpstFindMainPaymentSegment(struct s_group_45 *, char *); 

  fovdPushFunctionName("foenFetchDunningLetterAmount");

  lpstMoa = lpstFindMainPaymentSegment(ppstDunningLetter->timm->g_45, "77");
  if (lpstMoa == NULL)
    {
      fovdPopFunctionName();

      return FALSE;
    }

  strncpy(pachzAmount, lpstMoa->v_5004, poiAmountLen);
  strncpy(pachzCurrency, lpstMoa->v_6345, poiCurrencyLen);

  fovdPopFunctionName();

  return TRUE;
}


toenBool foenFetchDunningLetterTerms(struct s_TimmInter *ppstDunningLetter, char *pachzDaysLeft, int poiDaysLeftLen, char *pachzDueDate, char poiDueDateLen)
{
  struct s_group_8 *lpstG8;
  struct s_dtm_seg *lpstDtm;
  struct s_pat_seg *lpstPat;

  fovdPushFunctionName("foenFetchDunningLetterTerms");

  lpstG8 = ppstDunningLetter->timm->g_8;
  while (lpstG8)
    {
      lpstPat = lpstG8->pat;
      lpstDtm = lpstG8->dtm;
      fovdPrintLog (LOG_TIMM, "DTM : %s, %s, %s\n", lpstDtm->v_2005, lpstDtm->v_2380, lpstDtm->v_2379);
      fovdPrintLog (LOG_TIMM, "PAT : %s, %s, %s\n", lpstPat->v_2475, lpstPat->v_2009, lpstPat->v_2151);
      /*
       * payment date is relative to invoice date
       * relative to date
       * calendar day
       */
      if (EQ(lpstPat->v_2475, "5") && EQ(lpstPat->v_2009, "1") && EQ(lpstPat->v_2151, "CD"))
        {
          strncpy(pachzDaysLeft, lpstPat->v_2152, poiDaysLeftLen);
          strcpy(pachzDueDate, lpstDtm->v_2380);

          fovdPrintLog (LOG_TIMM, "DTM : %s, %s, %s -> %s\n", lpstDtm->v_2005, lpstDtm->v_2380, lpstDtm->v_2379, pachzDueDate);
          fovdPrintLog (LOG_TIMM, "PAT : %s, %s, %s -> %s\n", lpstPat->v_2475, lpstPat->v_2009, lpstPat->v_2151, pachzDaysLeft);
         
          fovdPopFunctionName();          

          return TRUE;
        }
      lpstG8 = lpstG8->g_8_next;
    }

  fovdPopFunctionName();

  return FALSE;
}
  

toenBool foenFetchDunningLetterDate(struct s_TimmInter *ppstDuningLetter, char *pachzDate, int poiDateLen)
{
  struct s_dtm_seg *lpstDtm;

  fovdPushFunctionName("foenFetchDunningLetterDate");

  lpstDtm = ppstDuningLetter->timm->dtm;
  while (lpstDtm)
    {
      if (EQ(lpstDtm->v_2005, "3"))
        {
          strncpy(pachzDate, lpstDtm->v_2380, poiDateLen);
          
          fovdPopFunctionName();
          
          return TRUE;
        }

      lpstDtm = lpstDtm->dtm_next;
    }

  fovdPopFunctionName();

  return FALSE;
}

toenBool foenFetchDunningLetterDueDate(struct s_TimmInter *ppstDuningLetter, char *pachzDate, int poiDateLen)
{
  struct s_dtm_seg *lpstDtm;

  fovdPushFunctionName("foenFetchDunningLetterDate");

  lpstDtm = ppstDuningLetter->timm->dtm;
  while (lpstDtm)
    {
      if (EQ(lpstDtm->v_2005, "13"))
        {
          strncpy(pachzDate, lpstDtm->v_2380, poiDateLen);          
          fovdPopFunctionName();
          return TRUE;
        }

      lpstDtm = lpstDtm->dtm_next;
    }

  fovdPopFunctionName();

  return FALSE;
}


struct s_moa_seg *lpstFindMainPaymentSegment(struct s_group_45 *ppstG45, char *pachzType) 
{
  struct s_moa_seg *lpstMoa;
  struct s_group_45 *lpstG45;
  
  lpstG45 = ppstG45;
  while (lpstG45)
    {
      lpstMoa = lpstG45->moa;
      if (EQ(lpstMoa->v_5025, pachzType))
        {
          return lpstMoa;
        }
      lpstG45 = lpstG45->g_45_next;
    }

  return NULL;
}


