/**************************************************************************************************
 *                                                                                          
 * MODULE: TIMM
 *                                                                                          
 * AUTHOR: N.Bondarczuk                                                
 * 
 * CREATION DATE : 08.01.98                                              
 *
 **************************************************************************************************
 */

#include <stdio.h>
#include <string.h>
#include "bgh.h"
#include "protos.h"
#include "parser.h"

#include "types.h"
#include "gen.h"
#include "strutl.h"

#if 0	/* just for version.sh */
static char *SCCS_VERSION = "4.1";
#endif

toenBool foenTIMMTypeCheck(struct s_TimmInter *pstTimmInter, char *pachzType)
{
  return EQ(pstTimmInter->timm->bgm->v_1000, pachzType);
}

struct s_rff_seg *fpstFindItemReference(struct s_group_26 *ppstG26, char *ppszType)
{
  struct s_group_26 *lpstG26;  
  struct s_rff_seg *lpstRff;

  lpstG26 = ppstG26;
  while (lpstG26) 
    {
      lpstRff = lpstG26->rff;
      if (EQ(lpstRff->v_1153, ppszType))
        {
          return lpstRff;
        }

      lpstG26 = lpstG26->g_26_next;
    }

  return NULL;
}

