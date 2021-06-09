#include <string.h>
#include <stdlib.h>

#include "parser.h"
#include "types.h"
#include "gen.h"
#include "shdes2des.h"
#include "item_list.h"
#include "money.h"
#include "call_category.h"

static int foiCallCategory(struct s_group_99 *ppstG99, int poiSeqNo);

static char szTemp[128];


/*
  ================================================================================================
  =                                                                                              =
  =                                                                                              =
  = DATATYPE: CallCategory                                                                       =
  =                                                                                              =
  =                                                                                              = 
  ================================================================================================
*/

/**************************************************************************************************
 *                                                                                          
 * METHOD : foiCallCategory_Max
 *                                                                                          
 * DESCRIPTION : 
 *
 * Returning -1 means internal error during allocation of categories. Else returned is maximal 
 * number of category allowed. It is used in iteration loop for each category.
 *                                                                                          
 **************************************************************************************************
 */

int foiCallCategory_Max()
{
  return USAGE_CATEGORIES_NO - 1;
}

/**************************************************************************************************
 *                                                                                          
 * METHOD : foenCallCategory_Init
 *                                                                                          
 * DESCRIPTION : 
 *
 *
 * Initialization of a given category for a given number of category.
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenCallCategory_Init(tostCallCategory *ppstCallCategory, int poiCategoryIndex)
{
  fovdPushFunctionName("foenCallCategory_Init");

  strncpy(ppstCallCategory->sasnzLabel, dasnzCallCategoryLabel[poiCategoryIndex], DES_SIZE);      
  ppstCallCategory->soflSurchargeAmount = 0.0;
  ppstCallCategory->soflAmount = 0.0;
  ppstCallCategory->soiCalls = 0;

  fovdPopFunctionName();
  
  return TRUE;
}

/**************************************************************************************************
 *                                                                                          
 * METHOD : foenCallCategory_Reinit
 *                                                                                          
 * DESCRIPTION : 
 *
 * Reinitialization of a given category for a given number of category.
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenCallCategory_Reinit(tostCallCategory *ppstCallCategory)
{
  ppstCallCategory->soflSurchargeAmount = 0.0;
  ppstCallCategory->soflAmount = 0.0;
  ppstCallCategory->soiCalls = 0;
  
  return TRUE;
}

/**************************************************************************************************
 *                                                                                          
 * METHOD : foenCallCategory_Add
 *                                                                                          
 * DESCRIPTION : 
 *
 * Add fields of Right to Left structure.
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenCallCategory_Add(tostCallCategory *ppstLeft, tostCallCategory *ppstRight)
{  
  ppstLeft->soflSurchargeAmount += ppstRight->soflSurchargeAmount;
  ppstLeft->soflAmount += ppstRight->soflAmount;
  ppstLeft->soiCalls += ppstRight->soiCalls;

  return TRUE;
}

/**************************************************************************************************
 *                                                                                          
 * METHOD : foenCallCategory_Gen
 *                                                                                          
 * DESCRIPTION : 
 * 
 * Generate PS macro in the input file.
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenCallCategory_Gen(tostCallCategory *ppstCallCategory)
{
  static char lasnzAmount[MAX_BUFFER];
  static char lasnzCalls[MAX_BUFFER];
  
  fovdPushFunctionName("foenCallCategory_Gen");
  
  memset(lasnzAmount, 0, MAX_BUFFER);
  memset(lasnzCalls, 0, MAX_BUFFER);

  sprintf(lasnzCalls, "%d", ppstCallCategory->soiCalls);  
  sprintf(lasnzAmount, "%.02lf", ppstCallCategory->soflAmount);
  fovdFormatMoney(lasnzAmount);

  fovdGen("SimCallCategory", ppstCallCategory->sasnzLabel, lasnzAmount, lasnzCalls, EOL);

  fovdPopFunctionName();
  return TRUE;
}

/**************************************************************************************************
 *                                                                                          
 * METHOD : foenCallCategory_ParseCallSeq
 *                                                                                          
 * DESCRIPTION : 
 *
 * Treat the next seq of XCD blocks in subsequent G99 groups as one call and return it's summary 
 * value and number of occurences.
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenCallCategory_ParseCallSeq(struct s_group_99 *ppstG99, struct s_group_99 **pppstNextG99, 
                                       int *ppiCategoryIndex, tostCallCategory *ppstCallCategory)
{
  struct s_group_99 *lpstG99;
  struct s_xcd_seg *lpstXcd;
  int n, loiSeqNo, loiIndex = -1, loiVal;
  double loflAmount = 0.0, loflTAPNetAmount = 0.0, loflTAPTaxAmount = 0.0;
  double loflSumAmount = 0.0, loflSumTAPNetAmount = 0.0, loflSumTAPTaxAmount = 0.0;
  double loflSurchargeAmount = 0.0;
  
  fovdPushFunctionName("foenCallCategory_ParseCallSeq");

  lpstG99 = ppstG99;
  n = sscanf(lpstG99->xcd->v_X001, "%d", &loiSeqNo);
  if ((loiIndex = foiCallCategory(lpstG99, loiSeqNo)) == -1)
    {
      sprintf (szTemp, "foenCallCategory_ParseCallSeq: Category of call is not assigned\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName();
      return FALSE;
    }
  
  while (lpstG99)
    {
      n = sscanf(lpstG99->xcd->v_X001, "%d", &loiVal);
      if (loiVal == loiSeqNo)
        {          
          /*
           * In the seq of XCD items
           */

          lpstXcd = lpstG99->xcd;

          if ((n = sscanf(lpstXcd->v_5004, "%lf", &loflAmount)) != 1)
            {
              sprintf (szTemp, "foenCallCategory_ParseCallSeq: Can't scan Call Value\n");
              macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
              fovdPopFunctionName();
              return FALSE;
            }
          
          switch(loiIndex)
            {
            case USAGE_CATEGORY_LOCAL:
            case USAGE_CATEGORY_INTER:
            case USAGE_CATEGORY_RLEG:
              loflSumAmount += loflAmount;              
              break;
              
            case USAGE_CATEGORY_VPLMN:
              loflSurchargeAmount += loflAmount;

              if ((n = sscanf(lpstXcd->v_5004a,"%lf", &loflTAPNetAmount)) != 1)
                {
                  sprintf (szTemp, "foenCallCategory_ParseCallSeq: Can't scan TAP Net Call Value\n");
                  macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
                  fovdPopFunctionName();
                  return FALSE;
                }

              loflSumTAPNetAmount += loflTAPNetAmount; 
              
              if ((n = sscanf(lpstXcd->v_5004b,"%lf", &loflTAPTaxAmount)) != 1)
                {
                  sprintf (szTemp, "foenCallCategory_ParseCallSeq: Can't scan TAP Tax Call Value\n");
                  macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
                  fovdPopFunctionName();
                  return FALSE;
                }
              
              loflSumTAPTaxAmount += loflTAPTaxAmount;                        
              break;

            default:
              return FALSE;
            }
        }
      else
        {
          /*
           * Out of the seq of XCD items
           */
          
          break;
        }

      lpstG99 = lpstG99->g_99_next;
    }

  /*
   * Set summary value of the call
   */

  ppstCallCategory->soflSurchargeAmount = loflSurchargeAmount;
  ppstCallCategory->soflAmount = loflSumAmount + loflSumTAPNetAmount + loflSumTAPTaxAmount;

  /*
   * Only non-zero value calls should be counted
   */

  if (ppstCallCategory->soflAmount > 0.0)
    {
      ppstCallCategory->soiCalls = 1;
    }
  else
    {
      ppstCallCategory->soiCalls = 0;
    }

  fovdPrintLog (LOG_DEBUG, "foenCallCategory_ParseCallSeq: Value: %8.02lf, Surch: %8.02lf, Count: %d\n", 
                ppstCallCategory->soflAmount, ppstCallCategory->soflSurchargeAmount, ppstCallCategory->soiCalls);

  /*
   * If end of seq or NULL pointer found
   */

  *pppstNextG99 = lpstG99;
  *ppiCategoryIndex = loiIndex;
  
  fovdPopFunctionName();
  return TRUE;
}

static int foiCallCategory(struct s_group_99 *ppstG99, int poiSeqNo)
{
  struct s_group_99 *lpstG99;
  struct s_xcd_seg *lpstXcd;
  int loiIndex = -1, loiVal, n;

  lpstG99 = ppstG99;
  while (lpstG99)
    {
      n = sscanf(lpstG99->xcd->v_X001, "%d", &loiVal);
      if (loiVal == poiSeqNo)
        {          
          lpstXcd = lpstG99->xcd;
          
          if (lpstXcd->v_X019[0] == 'R')
            {
              return USAGE_CATEGORY_RLEG;
            }
          
          else if (lpstXcd->v_X028[0] == 'V')
            {
              loiIndex = USAGE_CATEGORY_VPLMN;
            }
          
          else if (strlen(lpstXcd->v_X015) >= 2 &&
                   lpstXcd->v_X015[strlen(lpstXcd->v_X015) - 2] == 'M' && 
                   isdigit(lpstXcd->v_X015[strlen(lpstXcd->v_X015) - 1]))
            {
              return USAGE_CATEGORY_INTER;
            }
          
          else
            {
              loiIndex = USAGE_CATEGORY_LOCAL;
            }
        }
      else
        {
          break;
        }
      
      lpstG99 = lpstG99->g_99_next;
    }

  return loiIndex;
}

