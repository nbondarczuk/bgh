#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "bgh.h"
#include "protos.h"
#include "parser.h"
#include "types.h"
#include "gen.h"
#include "strutl.h"
#include "vpn.h"
#include "legend.h"


extern stServiceName *pstServiceName;
extern long glSNCount;
extern stBGHGLOB       stBgh;                  /* structure with globals for BGH */
toenBool goenOdebr = FALSE;
toenBool goenRLegOdebr = FALSE;
toenBool goenFUM = FALSE;
toenBool goenVPNFUM = FALSE;

/**************************************************************************************************
 *                                                                                          
 * METHOD: Init
 *                                                                                          
 *
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenLegend_Init()
{
  register int i;

  /*
   *  set up table stServiceName
   */

  goenOdebr = FALSE;
  goenRLegOdebr = FALSE;
  goenFUM = FALSE;
  goenVPNFUM = FALSE;

  for (i = 0; i < glSNCount; i++)
    {
/*      pstServiceName[i].lMult = 0;
      pstServiceName[i].lCFMult = 0;
*/
      pstServiceName[i].soenWasVPNCall = 0;
      pstServiceName[i].soenWasNormalCall = 0;
      pstServiceName[i].soenWasVPNCFCall = 0;
      pstServiceName[i].soenWasNormalCFCall = 0;
    }

  return TRUE;
}

/**************************************************************************************************
 *                                                                                          
 * METHOD: AddLabel
 *                                                                                          
 *
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenLegend_AddLabel(char *ppsnzShdes, char *ppsnzSuffix, toenBool poenIsVPN, char *ppsnzFreeInd)
{  
    int i, n, loiVPNPrefixLen;
    toenBool loenCFLeg = FALSE;
    
    fovdPrintLog(LOG_DEBUG, "foenLegend_AddLabel, Shdes: %s, IsVPN: %d\n", ppsnzShdes, poenIsVPN);
    if (EQ(ppsnzShdes, "ODEBR"))
    {
        goenOdebr = TRUE;
        return TRUE;
    }
  
    if (EQ(ppsnzShdes, "ODEBR*"))
    {
        goenRLegOdebr = TRUE;
        return TRUE;
    }

    i = 0;
    loiVPNPrefixLen = strlen(VPN_ZONE_PATTERN)-1;
    if(!strncmp(ppsnzFreeInd,VPN_ZONE_PATTERN, loiVPNPrefixLen))
    {
        i +=  loiVPNPrefixLen ; 
    }
    if (ppsnzFreeInd[i] != 'N' || ppsnzFreeInd[i+1] != '\0')
    {
        if(0 == i)
        {
            goenFUM = TRUE;
        }
        else
        {
            goenVPNFUM = TRUE;
        }
    }

    fovdPrintLog(LOG_DEBUG, "goenFUM %d, goenVPNFUM %d\n", goenFUM, goenVPNFUM);
    n = strlen(ppsnzShdes);
    for (i = 0; i < n; i++)
    {
        if (ppsnzShdes[i] == '>')
        {
            ppsnzShdes[i] = '\0';
            loenCFLeg = TRUE;
        }
    }

    for (i = 0; i < glSNCount; i++)
    {
        if (EQ(pstServiceName[i].szShdes, ppsnzShdes))
        {
            if(TRUE == poenIsVPN)
            {
                pstServiceName[i].soenWasVPNCall = 1;
                if (loenCFLeg == TRUE)
                {
                    pstServiceName[i].soenWasVPNCFCall = 1;
                }
            }
            else
            {
                pstServiceName[i].soenWasNormalCall = 1;
                if (loenCFLeg == TRUE)
                {
                    pstServiceName[i].soenWasNormalCFCall = 1;
                }
            }
/*
  pstServiceName[i].lMult++;
  if (loenCFLeg == TRUE)
  {
  pstServiceName[i].lCFMult++;
  break;
  }
*/    
            fovdPrintLog(LOG_DEBUG, " foenLegend_AddLabel: found: Shdes %s, VPN %d, VPNCF %d, Normal %d, NormalCF %d\n"
                         ,pstServiceName[i].szShdes,
                         pstServiceName[i].soenWasVPNCall, pstServiceName[i].soenWasVPNCFCall,
                         pstServiceName[i].soenWasNormalCall, pstServiceName[i].soenWasNormalCFCall);
            break;
        }
    }

    return TRUE;
}

/**************************************************************************************************
 *                                                                                          
 * METHOD: Gen
 *                                                                                          
 *
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenLegend_Gen() 
{
  int i, loiItems;
  char lpchzItems[32];
  static char lasnzDes[MAX_BUFFER];
  static char lasnzShdes[MAX_BUFFER];

  /*
   * Count items number
   */

  loiItems = 0;
  for (i = 0; i < glSNCount; i++)
  {      
/*      if (pstServiceName[i].lMult > 0)
      {
*/
          if(1 == pstServiceName[i].soenWasVPNCall)
          {
              loiItems++;
          }                   
          if(1 == pstServiceName[i].soenWasNormalCall)
          {
              loiItems++;
          }                   
/*
      }
      if (pstServiceName[i].lCFMult > 0)
      {
*/
          if(1 == pstServiceName[i].soenWasVPNCFCall)
          {
              loiItems++;
          }                   
          if(1 == pstServiceName[i].soenWasNormalCFCall)
          {
              loiItems++;
          }                   
/*
      }
*/
  }

  if (goenOdebr == TRUE)
    {
      loiItems++;
    }

  if (goenRLegOdebr == TRUE)
    {
      loiItems++;
    }
      
  if (goenFUM == TRUE)
    {
      loiItems++;
    }

  if (loiItems == 0)
    {
      return TRUE;
    }

  sprintf(lpchzItems, "%d", loiItems);
  if(TRUE == stBgh.soSubscriberCallDetailListPrinted)
  {
	  fovdGen("SimLegendPrint","1", EOL);
  }
  else
  {
	  fovdGen("SimLegendPrint","0", EOL);
  }
  fovdGen("SimLegendListStart", lpchzItems, EOL);

  if (goenFUM == TRUE)
    {
      fovdGen("SimLegendBoldItem", "Po\263" "\261" "czenia telefoniczne w ramach puli darmowych minut", "TELEF", EOL);
    }

  if (goenVPNFUM == TRUE)
    {
      fovdGen("SimLegendBoldItem", 
              "Po\263" "\261" "czenia telefoniczne w ramach puli darmowych minut, w ramavh VPN", "TELEF", EOL);
    }

  if (goenOdebr == TRUE)
    {
      fovdGen("SimLegendItem", "Po\263" "\261" "czenia odebrane w roamingu", "ODEBR", EOL);
    }
  
  if (goenRLegOdebr == TRUE)
    {
      fovdGen("SimLegendItem", "Po\263" "\261" "czenia odebrane w roamingu - op\263" "aty obce", "ODEBR*", EOL);
    }

  /*
   * Print found service names and codes
   */

  for (i = 0; i < glSNCount; i++)
  {
/*
      if (pstServiceName[i].lMult > 0)
      {
*/
          if(1 == pstServiceName[i].soenWasVPNCall)
          {
              strcpy(lasnzDes,pstServiceName[i].szDes);
              strcat(lasnzDes, " w ramach VPN");
              fovdGen( "SimLegendItem", lasnzDes, pstServiceName[i].szShdes, EOL);          
          }                   
          if(1 == pstServiceName[i].soenWasNormalCall)
          {
              fovdGen( "SimLegendItem", pstServiceName[i].szDes, pstServiceName[i].szShdes, EOL);          
          }                   
/*
      }
      if (pstServiceName[i].lCFMult > 0)
      {
*/
          if(1 == pstServiceName[i].soenWasVPNCFCall)
          {
              sprintf(lasnzDes, "%s przekierowanie w ramach VPN", pstServiceName[i].szDes);
              sprintf(lasnzShdes, "%s>", pstServiceName[i].szShdes);
              fovdGen("SimLegendItem", lasnzDes, lasnzShdes, EOL);
          }                   
          if(1 == pstServiceName[i].soenWasNormalCFCall)
          {
              sprintf(lasnzDes, "%s przekierowanie", pstServiceName[i].szDes);
              sprintf(lasnzShdes, "%s>", pstServiceName[i].szShdes);
              fovdGen("SimLegendItem", lasnzDes, lasnzShdes, EOL);
          }                   
/*
      }
*/
  }
  
  fovdGen( "SimLegendListEnd", EOL);

  return TRUE;
}







