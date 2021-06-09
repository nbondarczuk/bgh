#if 0	/* just for version.sh */
static char *SCCS_VERSION = "4.0";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>		/* for unlink */


/* User includes                */
#include "bgh.h"
#include "protos.h"
#include "layout.h"             /* needed for pointer to layout-arrays */

#include "types.h"
#include "gen.h"

#define INV 0
#define SUM 1

#define EQ_ADDR(a, b) \
EQ(a->v_3036 , b->v_3036 ) && \
EQ(a->v_3036a, b->v_3036a) && \
EQ(a->v_3036b, b->v_3036b) && \
EQ(a->v_3036c, b->v_3036c) && \
EQ(a->v_3036d, b->v_3036d)

#define NOT_EQ_ADDR(a, b) \
strcmp(a->v_3036 , b->v_3036 ) || \
strcmp(a->v_3036a, b->v_3036a) || \
strcmp(a->v_3036b, b->v_3036b) || \
strcmp(a->v_3036c, b->v_3036c) || \
strcmp(a->v_3036d, b->v_3036d)

#define EMPTY(a) \
a->v_3036[0]  == '\0' && \
a->v_3036a[0] == '\0' && \
a->v_3036b[0] == '\0' && \
a->v_3036c[0] == '\0' && \
a->v_3036d[0] == '\0'

#define LOG_ADDR(id, t, a) \
fovdPrintLog (LOG_DEBUG, "[%08d]NAD: %s %s:%s:%s:%s:%s\n", id, t, a->v_3036, a->v_3036a, a->v_3036b, a->v_3036c,a->v_3036d);

/* Static globals               */
static char szTemp[PATH_MAX];		/* temporay storage for error messages */

static struct s_nad_seg *fpstFindCustomerNAD(stTIMM *ppstTimm, char *pasnzType)
{
  struct s_group_2 *lpstG2;
  struct s_nad_seg *lpstNad;

  lpstG2 = ppstTimm->timm->g_2;  
  while(lpstG2) 
    {
      lpstNad = lpstG2->nad;
      if (EQ(lpstNad->v_3035, pasnzType))
        {
          return lpstNad;
        }

      lpstG2 = lpstG2->g_2_next;
    }

  return NULL;
}

/*
 * (INV.NAD+IV == SUM.NAD+IV && INV.NAD+IT == Empty) ||
 * (INV.NAD+IV != SUM.NAD+IV && INV.NAD+IT != Empty && INV.NAD+IT == SUM.NAD+IV)
 * => FALSE
 * else => TRUE
 *
 */

extern int foiIsContractAddress(int);

toenBool foenOneEnvelope(stTIMM *ppstInvTimm, stTIMM *ppstSumTimm)
{
  struct s_nad_seg *lpstNadBA, *lpstNadCA, *lpstNadTA;
  static struct s_nad_seg *fpstFindCustomerNAD(stTIMM *, char *);

#ifdef _MAILING_RULES_V0_
  return TRUE;
#endif

#ifdef _MAILING_RULES_V1_
  lpstNadBA = fpstFindCustomerNAD(ppstInvTimm, "IV");
  if (lpstNadBA == NULL)
    {
      return ERROR;
    }

  lpstNadCA = fpstFindCustomerNAD(ppstSumTimm, "IV");
  if (lpstNadCA == NULL)
    {
      return ERROR;
    }

  LOG_ADDR(atoi (ppstInvTimm->unb->v_0010) ,"BA", lpstNadBA);
  LOG_ADDR(atoi (ppstInvTimm->unb->v_0010), "CA", lpstNadCA);

  if (NOT(EQ_ADDR(lpstNadBA, lpstNadCA)) && foiIsContractAddress(atoi (ppstInvTimm->unb->v_0010)))
    {
      fovdPrintLog (LOG_DEBUG, "Separate mailing\n");
      return FALSE;
    }

  return TRUE;
#endif

#ifdef _MAILING_RULES_V2_
  lpstNadBA = fpstFindCustomerNAD(ppstInvTimm, "IV");
  if (lpstNadBA == NULL)
    {
      return ERROR;
    }

  lpstNadTA = fpstFindCustomerNAD(ppstInvTimm, "IT");
  if (lpstNadTA == NULL)
    {
      return ERROR;
    }

  lpstNadCA = fpstFindCustomerNAD(ppstSumTimm, "IV");
  if (lpstNadCA == NULL)
    {
      return ERROR;
    }

  LOG_ADDR(atoi (ppstInvTimm->unb->v_0010) ,"BA", lpstNadBA);
  LOG_ADDR(atoi (ppstInvTimm->unb->v_0010), "CA", lpstNadCA);
  LOG_ADDR(atoi (ppstInvTimm->unb->v_0010), "TA", lpstNadTA);

  if (
      EMPTY(lpstNadTA) && 
      NOT(EQ_ADDR(lpstNadBA, lpstNadCA))
      )
    {
      fovdPrintLog (LOG_DEBUG, "Separate mailing\n");
      return FALSE;
    }

  if (
      NOT(EMPTY(lpstNadTA)) && 
      EQ_ADDR(lpstNadBA, lpstNadCA) &&
      NOT(EQ_ADDR(lpstNadBA, lpstNadTA)) &&
      NOT(EQ_ADDR(lpstNadTA, lpstNadCA)) 
      )
    {
      fovdPrintLog (LOG_DEBUG, "Separate mailing\n");
      return FALSE;
    }

  if (
      NOT(EMPTY(lpstNadTA)) && 
      NOT(EMPTY(lpstNadBA)) && 
      NOT(EMPTY(lpstNadCA)) && 
      NOT(EQ_ADDR(lpstNadBA, lpstNadCA)) && 
      NOT(EQ_ADDR(lpstNadCA, lpstNadTA))
      )
    {
      fovdPrintLog (LOG_DEBUG, "Separate mailing\n");
      return FALSE;
    }

  fovdPrintLog (LOG_DEBUG, "One mailing\n");
  return TRUE;
#endif
}

toenBool foenEqualBillAddress(stTIMM *ppstSumTimm, stTIMM *ppstInvTimm)
{
  struct s_group_2 *lpstG2;
  struct s_nad_seg *lastNad[2];
  toenBool laenIsAddrFound[2] = {FALSE, FALSE};
  int i, laiDoc[2] = {INV, SUM};
  char *lasnzDocLabel[2] = {"INV", "SUM"};
  stTIMM *lastTimm[2];
  
  fovdPushFunctionName ("foenEqualBillAddress");

  for (i = 0, lastTimm[INV] = ppstSumTimm, lastTimm[SUM] = ppstInvTimm; i < 2; i++)
    {
      lpstG2 = lastTimm[laiDoc[i]]->timm->g_2;  
      while(lpstG2) 
        {
          lastNad[laiDoc[i]] = lpstG2->nad;
          if (lastNad[laiDoc[i]]) 
            {
              if (EQ(lastNad[laiDoc[i]]->v_3035, "IV")) 
                {
                  laenIsAddrFound[laiDoc[i]] = TRUE;
                  fovdPrintLog (LOG_TIMM, "%s:NAD:IV: %s\n", lasnzDocLabel[i], lastNad[laiDoc[i]]->v_3036);
                  fovdPrintLog (LOG_TIMM, "%s:NAD:IV: %s\n", lasnzDocLabel[i], lastNad[laiDoc[i]]->v_3036a);
                  fovdPrintLog (LOG_TIMM, "%s:NAD:IV: %s\n", lasnzDocLabel[i], lastNad[laiDoc[i]]->v_3036b);
                  fovdPrintLog (LOG_TIMM, "%s:NAD:IV: %s\n", lasnzDocLabel[i], lastNad[laiDoc[i]]->v_3036c);
                  fovdPrintLog (LOG_TIMM, "%s:NAD:IV: %s\n", lasnzDocLabel[i], lastNad[laiDoc[i]]->v_3036d);
                  break;
                }
            }

          lpstG2 = lpstG2->g_2_next;
        }
    }
  

  if (
      laenIsAddrFound[INV] == TRUE && 
      laenIsAddrFound[SUM] == TRUE && 
      EQ(lastNad[INV]->v_3036 , lastNad[SUM]->v_3036 ) &&
      EQ(lastNad[INV]->v_3036a, lastNad[SUM]->v_3036a) &&
      EQ(lastNad[INV]->v_3036b, lastNad[SUM]->v_3036b) &&
      EQ(lastNad[INV]->v_3036c, lastNad[SUM]->v_3036c) &&
      EQ(lastNad[INV]->v_3036d, lastNad[SUM]->v_3036d)
      )
    {
      fovdPrintLog (LOG_DEBUG, "Equality between addresses: SUM.NAD and INV.NAD \n");
      fovdPopFunctionName ();
      return TRUE;
    }
    
  fovdPrintLog (LOG_DEBUG, "No equality between addresses: SUM.NAD and INV.NAD \n");
  fovdPopFunctionName ();
  return FALSE;
}

int foiAddress_GetRuleNo(stTIMM *ppstInvTimm, stTIMM *ppstSumTimm)
{
  struct s_nad_seg *lpstNadBA;
  struct s_nad_seg *lpstNadCA;
  struct s_nad_seg *lpstNadTA;
  static struct s_nad_seg *fpstFindCustomerNAD(stTIMM *, char *);

  if (NULL == (lpstNadBA = fpstFindCustomerNAD(ppstInvTimm, "IV")))
    {
      sprintf (szTemp, "Can't get NAD+IV in INV-TIMM\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      return -1;
    }

  if (NULL == (lpstNadTA = fpstFindCustomerNAD(ppstInvTimm, "IT")))
    {
      sprintf (szTemp, "Can't get NAD+IT in INV-TIMM \n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      return -1;
    }

  if (NULL == (lpstNadCA = fpstFindCustomerNAD(ppstSumTimm, "IV")))
    {
      sprintf (szTemp, "Can't get NAD+IV in SUM-TIMM \n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      return -1;
    }

  if (EMPTY(lpstNadTA)
      && EQ_ADDR(lpstNadBA, lpstNadCA))
    {
      return 1;
    }

  if ((EMPTY(lpstNadTA) && 
       NOT(EQ_ADDR(lpstNadBA, lpstNadCA)))
      || (NOT(EMPTY(lpstNadTA))
          && EQ_ADDR(lpstNadBA, lpstNadTA)
          && NOT(EQ_ADDR(lpstNadBA, lpstNadCA))))
    {
      return 2;
    }
  
  if (NOT(EMPTY(lpstNadTA)) 
      && EQ_ADDR(lpstNadBA, lpstNadCA)
      && NOT(EQ_ADDR(lpstNadBA, lpstNadTA)) 
      && NOT(EQ_ADDR(lpstNadTA, lpstNadCA)))
    {
      return 3;
    }

  if (NOT(EMPTY(lpstNadTA))
      && NOT(EMPTY(lpstNadBA))
      && NOT(EMPTY(lpstNadCA))
      && NOT(EQ_ADDR(lpstNadBA, lpstNadCA))
      && NOT(EQ_ADDR(lpstNadCA, lpstNadTA)))
    {      
      return 4;
    }

  if (NOT(EMPTY(lpstNadTA))
      && NOT(EMPTY(lpstNadBA))
      && NOT(EMPTY(lpstNadCA))
      && NOT(EQ_ADDR(lpstNadBA, lpstNadCA))
      && EQ_ADDR(lpstNadCA, lpstNadTA))
    {      
      return 5;
    }

  return 1;  
}

