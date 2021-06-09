/**************************************************************************/
/*  MODULE : WELCOME data fetecher                                        */
/*                                                                        */
/*  AUTHOR : N.Bondarczuk                                                 */
/*                                                                        */
/*  CREATION DATE : 25.11.97                                              */
/*                                                                        */
/*  DESCRIPTION : Exports  functions accessing information from TIMM      */
/*                WELCOME   message.                                      */ 
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

int foiCountWelcomeLetterTexts(struct s_TimmInter *ti)
{
  struct s_ftx_seg *ftx;
  int loiText;

  loiText = 0;
  ftx = ti->timm->ftx;
  while (ftx)
    {
      loiText++;
      ftx = ftx->ftx_next;
    }

  return loiText;
}


toenBool foenFetchWelcomeLetterText(struct s_TimmInter *ti, int poiTextNo, char pachzLine[5][MAX_BUFFER])
{
  struct s_ftx_seg *ftx;
  int loiText;

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
  
  return FALSE;
}


toenBool foenFetchWelcomeLetterCustomerAccountNo(struct s_TimmInter *ppstWelcomeLetter, 
                                                 char *pachzAccountNo, int poiAccountNoLen)
{
  struct s_group_1 *g_1;
  struct s_rff_seg *rff;
  struct s_rff_seg *lpstFindMainReference(struct s_group_1 *, char *);   

  rff = lpstFindMainReference(ppstWelcomeLetter->timm->g_1, "IT");
  if (rff == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }
  
  strncpy(pachzAccountNo, rff->v_1154, poiAccountNoLen);

  fovdPopFunctionName();
  return TRUE;
}

toenBool foenFetchWelcomeLetterSellerNIP(struct s_TimmInter *ppstWelcomeLetter, char *pachzNIP, int poiNIPLen)
{
  struct s_group_1 *g_1;
  struct s_rff_seg *rff;
  struct s_rff_seg *lpstFindMainReference(struct s_group_1 *, char *);   

  rff = lpstFindMainReference(ppstWelcomeLetter->timm->g_1, "VA");
  if (rff == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }
  
  strncpy(pachzNIP, rff->v_1154, poiNIPLen);

  fovdPopFunctionName();
  return TRUE;
}




struct s_rff_seg *lpstFindMainReference(struct s_group_1 *ppstG1, char *ppszType) 
{
  struct s_group_1 *lpstG1;  
  struct s_rff_seg *lpstRff;

  lpstG1 = ppstG1;
  while (lpstG1) 
    {
      lpstRff = lpstG1->rff;
      if (EQ(lpstRff->v_1153, ppszType))
        {
          return lpstRff;
        }

      lpstG1 = lpstG1->g_1_next;
    }

  return NULL;
}

