#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "bgh.h"
#include "protos.h"
#include "parser.h"
#include "types.h"
#include "gen.h"
#include "call_list.h"
#include "con_serv.h"
#include "timm.h"


/*
 * extern functions
 */

extern void fovdFormatMoney(char *pachzMoney);

extern int foiItemCallList_CountLocalCalls(tostItemCallList *ppstList);

extern int foiItemCallList_CountRoamingCalls(tostItemCallList *ppstList);


/*
 * static functions
 */

static int foiSumSheetItem_UsageClass(struct s_group_22 *g_22, 
				      char *ppchLeg);

static toenBool foenSumSheetItem_Load(struct s_group_22 *g_22, 
				      double *ppflNet, 
				      int *ppiItems);

static int foiSumSheetItem_CallClass(struct s_group_99 *ppstG99, 
				     int poiSeqNo, 
				     struct s_group_99 **pppstNextG99, 
				     double *ppflVal);

static int foiSumSheetItem_Level(struct s_group_22 *ppstG22);

static toenBool foenConServ_Init(tostConServ *ppstServ, 
				 int poiIndex);

static toenBool foenConServ_Gen(tostConServ *ppstServ);

static toenBool foenSumSheetItem_SuppressCall(struct s_group_99 *ppstG99, 
					      int poiSeqNo, 
					      char *ppchType, 
					      struct s_group_99 **pppstNextG99);

/*
 * extern variables
 */

extern int goiContractNo;
extern long goilCustomerId;

/*
 * static variables
 */

static char szTemp[128];

/*
 * global variables exported
 */

double goflSimUsage_Local;
double goflSimUsage_Roaming;
double goflSimUsage_Surcharge;

/********************************************************************************************************************
 *
 * foenSumSheetItem_ConnectionValue
 *
 * DESCRIPTION:
 * Returns index of category for a given G99 item seq with Usage info. If type no recognized then returns ENC_UNKNOWN_CAT. 
 *
 ********************************************************************************************************************
 */

toenBool foenSumSheetItem_ConnectionValue(struct s_xcd_seg *ppstXcd, double *ppflVal)
{
  int n;
  double loflAmount, loflTAPNetAmount, loflTAPTaxAmount;

  fovdPushFunctionName("foenSumSheetItem_ConnectionValue");

  if ((n = sscanf(ppstXcd->v_5004c, "%lf", &loflAmount)) != 1)
    {
      loflAmount = 0.0;
    }
  
  if ((n = sscanf(ppstXcd->v_5004a,"%lf", &loflTAPNetAmount)) != 1)
    {
      loflTAPNetAmount = 0.0;
    }

  if ((n = sscanf(ppstXcd->v_5004b,"%lf", &loflTAPTaxAmount)) != 1)
    {
      loflTAPTaxAmount = 0.0;
    }

  *ppflVal += loflAmount + loflTAPNetAmount + loflTAPTaxAmount;

  fovdPopFunctionName();
  return TRUE;
}

/********************************************************************************************************************
 *
 * foflSumSheetItem_CallValue
 *
 * DESCRIPTION:
 *
 ********************************************************************************************************************
 */

double foflSumSheetItem_CallValue(struct s_group_99 *ppstG99, int poiCallSeqNo)
{
  struct s_group_99 *lpstG99 = ppstG99;
  struct s_xcd_seg *lpstXcd;
  int loiVal, n;
  double loflVal = 0.0;
  toenBool loenStatus;

  while (lpstG99)
    {
      n = sscanf(lpstG99->xcd->v_X001, "%d", &loiVal);
      if (loiVal != poiCallSeqNo)
        {
          break;
        }

      lpstXcd = lpstG99->xcd;    
      if ((loenStatus = foenSumSheetItem_ConnectionValue(lpstXcd, &loflVal)) == FALSE)
        {
          return 0.0;
        }

      lpstG99 = lpstG99->g_99_next;
    }
      

  return loflVal;
}

/********************************************************************************************************************
 *
 * foiSumSheetItem_CallClass
 *
 * DESCRIPTION:
 * Returns index of category for a given G99 item seq with Usage info. If type no recognized then returns ENC_UNKNOWN_CAT. 
 *
 ********************************************************************************************************************
 */

static toenBool foenSumSheetItem_SuppressCall(struct s_group_99 *ppstG99, 
					      int poiSeqNo, 
					      char *ppchType, 
					      struct s_group_99 **pppstNextG99)
{
  struct s_group_99 *lpstG99;
  struct s_xcd_seg *lpstXcd;
  int loiIndex = -1, loiVal, n;
  double loflVal, loflSumVal = 0.0;
  toenBool loenStatus;
  int laiIndex[ENC_CAT_TAB_SIZE];
  int loiType = ENC_LOCAL_USAGE_CAT;

  fovdPushFunctionName("foenSumSheetItem_SuppressCall");

  memset(laiIndex, 0, sizeof (int) * ENC_CAT_TAB_SIZE);

  lpstG99 = ppstG99;
  while (lpstG99)
    {
      n = sscanf(lpstG99->xcd->v_X001, "%d", &loiVal);
      ASSERT(n == 1);
      if (loiVal != poiSeqNo)
        {         
          /*
           * Next connection seq - next call
           */
          
          break;
        }
          
      lpstXcd = lpstG99->xcd;

      /*
       * Get summary call value
       */

      loflVal = 0.0;
      if ((loenStatus = foenSumSheetItem_ConnectionValue(lpstXcd, &loflVal)) != TRUE)
        {
          fovdPopFunctionName();
          return -1;
        }
      
      loflSumVal += loflVal;

      /*
       * Get call type
       */
      
      if (lpstXcd->v_X019[0] == 'R')
        {
          laiIndex[ENC_ROAMING_LEG_USAGE_CAT]++;
        }

      else if (lpstXcd->v_X028[0] == 'V')       
        {
          laiIndex[ENC_VPLMN_USAGE_CAT]++;
        }
      
      else
        {
          laiIndex[ENC_LOCAL_USAGE_CAT]++;
        }
      
      lpstG99 = lpstG99->g_99_next;
    }
  
  *pppstNextG99 = lpstG99;
  
  /*
   * Apply rules for seq of connections
   */
  
  if (laiIndex[ENC_VPLMN_USAGE_CAT] > 0 && laiIndex[ENC_ROAMING_LEG_USAGE_CAT] == 0)
    {
      fovdPopFunctionName();
      return FALSE;
    }

  fovdPopFunctionName();
  return FALSE;
}

static int foiSumSheetItem_CallClass(struct s_group_99 *ppstG99, 
				     int poiSeqNo, 
				     struct s_group_99 **pppstNextG99, 
				     double *ppflVal)
{
  struct s_group_99 *lpstG99;
  struct s_xcd_seg *lpstXcd;
  int loiIndex = -1, loiVal, n;
  double loflVal, loflSumVal = 0.0;
  toenBool loenStatus;
  int laiIndex[ENC_CAT_TAB_SIZE];
  int loiType = ENC_LOCAL_USAGE_CAT;

  fovdPushFunctionName("foenSumSheetItem_CallClass");

  memset(laiIndex, 0, sizeof (int) * ENC_CAT_TAB_SIZE);

  lpstG99 = ppstG99;
  while (lpstG99)
    {
      n = sscanf(lpstG99->xcd->v_X001, "%d", &loiVal);
      ASSERT(n == 1);
      if (loiVal != poiSeqNo)
        {         
          /*
           * Next connection seq - next call
           */
          
          break;
        }
          
      lpstXcd = lpstG99->xcd;

      /*
       * Get summary call value
       */

      loflVal = 0.0;
      if ((loenStatus = foenSumSheetItem_ConnectionValue(lpstXcd, &loflVal)) != TRUE)
        {
          fovdPopFunctionName();
          return -1;
        }
      
      loflSumVal += loflVal;

      /*
       * Get call type
       */
      
      if (lpstXcd->v_X019[0] == 'R')
        {
          laiIndex[ENC_ROAMING_LEG_USAGE_CAT]++;
        }

      else if (lpstXcd->v_X028[0] == 'V')       
        {
          laiIndex[ENC_VPLMN_USAGE_CAT]++;
        }
      
      else
        {
          laiIndex[ENC_LOCAL_USAGE_CAT]++;
        }
      
      lpstG99 = lpstG99->g_99_next;
    }

  *ppflVal = loflSumVal;
  *pppstNextG99 = lpstG99;

  /*
   * Apply rules for seq of connections
   */
  
  if (laiIndex[ENC_VPLMN_USAGE_CAT] > 0 && laiIndex[ENC_ROAMING_LEG_USAGE_CAT] == 0)
    {
      fovdPrintLog (LOG_DEBUG, "foiSumSheetItem_CallClass: Type: VPLMN\n"); 
      fovdPopFunctionName();
      return ENC_VPLMN_USAGE_CAT;
    }

  fovdPopFunctionName();
  return ENC_LOCAL_USAGE_CAT;
}


/********************************************************************************************************************
 *
 * foiSumSheetItem_UsageClass
 *
 * DESCRIPTION:
 * Returns index of category for a given G22 item with Usage info. If type no recognized then returns ENC_UNKNOWN_CAT. 
 * The next value returned is connection leg type with allowed values {A, C, I, R} if pia block indested by BCH.
 *
 ********************************************************************************************************************
 */

static int foiSumSheetItem_UsageClass(struct s_group_22 *ppstG22, char *ppchLeg)
{
  fovdPushFunctionName("foenSumSheetItem_UsageClass");

  if (ppstG22->lin->v_7140[strlen(ppstG22->lin->v_7140) - 1] == 'U')
    {
      if (ppstG22->pia != NULL)
        {
          ASSERT(ppstG22->pia->v_7140 != NULL);
          fovdPrintLog (LOG_DEBUG, "foiSumSheetItem_UsageClass: Testing: %s %s\n", 
                        ppstG22->lin->v_7140, 
                        ppstG22->pia->v_7140);
          
          *ppchLeg = ppstG22->pia->v_7140[0];

          if (ppstG22->pia->v_7140[2] == 'm' || ppstG22->pia->v_7140[2] == 'r')
            {
              fovdPopFunctionName();
              return ENC_VPLMN_USAGE_CAT;
            }
          
          if (ppstG22->pia->v_7140[2] == 'R')
            {
              fovdPopFunctionName();
              return ENC_SURCHARGE_USAGE_CAT;
            }
        }
      else
        {
          *ppchLeg = 0;
        }

      fovdPopFunctionName();
      return ENC_LOCAL_USAGE_CAT;
    }
  
  fovdPopFunctionName();
  return ENC_UNKNOWN_CAT;
}


/********************************************************************************************************************
 *
 * foiSumSheetItem_Level
 *
 * DESCRIPTION:
 * Loads G22 item level and returns it. If some error then returns -1.
 *
 ********************************************************************************************************************
 */

static int foiSumSheetItem_Level(struct s_group_22 *ppstG22)
{
  int n, loiVal;

  fovdPushFunctionName("foenSumSheetItem_Level");

  if ((n = sscanf(ppstG22->lin->v_1222, "%d", &loiVal)) != 1)
    {
      fovdPopFunctionName();
      return -1;
    }

  fovdPopFunctionName();
  return loiVal;
}

/********************************************************************************************************************
 *
 * foenSumSheetItem_Load
 *
 * DESCRIPTION:
 * Loading exact (not rounded) Net Amount info from G22 item of any type (Usage, Access, OCC, Subs.).
 *
 ********************************************************************************************************************
 */

static toenBool foenSumSheetItem_Load(struct s_group_22 *g_22, double *ppflNet, int *ppiItems)
{
  struct s_group_23 *g_23;
  struct s_group_30 *g_30;
  struct s_moa_seg *moa;
  struct s_tax_seg *tax;
  struct s_qty_seg *qty;
  int n, i = 0;

  fovdPushFunctionName("foenSumSheetItem_Level");
  
  ASSERT(g_22 != NULL);
  
  /*
   * Load MOA items from G_23 of G_22 block
   */
  
  g_23 = g_22->g_23;
  while (g_23)
    {
      moa = g_23->moa;
      ASSERT(moa != NULL);
      
      /*
       * MOA+125 -> NetAmount
       */
      
      fovdPrintLog (LOG_DEBUG, "MOA+%s\n", moa->v_5025);
      if (EQ(moa->v_5025, "125") && EQ(moa->v_4405, "19"))
        {
          n = sscanf(moa->v_5004, "%lf", ppflNet);
          ASSERT(n == 1);
          i++;
          break;
        }
      
      /*
       * Go to next group G23 item
       */
      
      g_23 = g_23->g_23_next;
    }
  
  if (i != 1)
    {
      /*
       * Try using exact value
       */

      i = 0;
      g_23 = g_22->g_23;
      while (g_23)
        {
          moa = g_23->moa;
          ASSERT(moa != NULL);
          
          /*
           * MOA+125 -> NetAmount
           */
      
          fovdPrintLog (LOG_DEBUG, "MOA+%s\n", moa->v_5025);
          if (EQ(moa->v_5025, "125") && EQ(moa->v_4405, "9"))
            {
              n = sscanf(moa->v_5004, "%lf", ppflNet);
              ASSERT(n == 1);
              i++;
              break;
            }
          
          /*
           * Go to next group G23 item
           */
          
          g_23 = g_23->g_23_next;
        }
      
      if (i != 1)
        {
          
          sprintf (szTemp, "foenSumSheetItem_Load: Can't find MOA+125 block\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          fovdPopFunctionName();
          return FALSE;
        }
    }      
  
  /*
   * Load number of calls from QTY block
   */
  
  if ((qty = fpstFindQuantity(g_22->qty, "107", "UNI")) != NULL)
    {
      n = sscanf(qty->v_6060, "%d", ppiItems);
      ASSERT(n == 1);
    }
  else if ((qty = fpstFindQuantity(g_22->qty, "108", "UNI")) != NULL)
    {
      n = sscanf(qty->v_6060, "%d", ppiItems);
      ASSERT(n == 1);
    }
  else
    {
      *ppiItems = 0;
    }

  fovdPopFunctionName();
  return TRUE;
}

/********************************************************************************************************************
 *
 * fpstConServ_Init
 *
 * DESCRIPTION:
 *
 ********************************************************************************************************************
 */

static toenBool foenConServ_Init(tostConServ *ppstServ, int poiIndex)
{
  fovdPushFunctionName("foenConServ_Init");

  strncpy(ppstServ->sasnzLabel, gasnzServLabel[poiIndex], MAX_BUFFER_SIZE);
  ppstServ->soflNet = 0.0;        
  ppstServ->soiItems = 0;
  ppstServ->soiServNo = poiIndex;

  fovdPopFunctionName();
  return TRUE;
}

/********************************************************************************************************************
 *
 * foenConServ_IsEmpty
 *
 * DESCRIPTION:
 *
 ********************************************************************************************************************
 */

static toenBool foenConServ_IsEmpty(tostConServ *ppstServ)
{ 
  if (ppstServ->soflNet >= 0.001 || ppstServ->soiItems > 0)
    {
      return FALSE;
    }

  return TRUE;
}

/********************************************************************************************************************
 *
 * foenConServ_Gen
 *
 * DESCRIPTION:
 *
 ********************************************************************************************************************
 */

static toenBool foenConServ_Gen(tostConServ *ppstServ)
{ 
  int n;
  static char lasnzNet[MAX_BUFFER_SIZE];
  static char lasnzItems[MAX_BUFFER_SIZE];

  fovdPushFunctionName("foenConServ_Gen");

  INIT_STATIC_BUF(lasnzNet);
  INIT_STATIC_BUF(lasnzItems);
  
  /*
   * Print number of items
   */
  
  if (ppstServ->soiServNo != ENC_SURCHARGE_USAGE_CAT && ppstServ->soiServNo != ENC_LAND_USAGE_CAT)
    {
      /*
       * -1 means not printing of number of calls
       */
      
      n = sprintf(lasnzItems, "%d", ppstServ->soiItems);
      ASSERT(n > 0);  
    }

  /*
   * Print Net value for a category of calls
   */

  n = sprintf(lasnzNet, "%.02lf", ppstServ->soflNet);
  ASSERT(n > 0);
  fovdFormatMoney(lasnzNet);

  /*
   * Print macro
   */

  fovdGen("SimUsageCategory", ppstServ->sasnzLabel, lasnzNet, lasnzItems, EOL);
  fovdPrintLog (LOG_TIMM, "Usage: %d\t%lf\t%s\n", ppstServ->soiItems, ppstServ->soflNet, ppstServ->sasnzLabel);
  
  fovdPopFunctionName();
  return TRUE;
}


/********************************************************************************************************************
 *
 * fpstConServTab_New
 *
 * DESCRIPTION:
 * Creates and loads a table with Usage categories for a given G22 item seq. G22 given in argument is Contract
 * Info item on level 2.
 *
 ********************************************************************************************************************
 */

static void fovdGetCallStat(struct s_group_99 *ppstG99);

tostConServTab *fpstConServTab_New(struct s_group_22 *g_22, 
                                   tostItemCallList *ppstCallList_Local, 
                                   tostItemCallList *ppstCallList_Roaming)
{
  struct s_group_22 *lpstG22;
  struct s_group_99 *lpstG99 = NULL, *lpstStartG99 = NULL;
  struct s_xcd_seg *lpstXcd;
  tostConServTab *lpstTab;
  int i, n, loiIndex, loiLevel, loiSeqNo, loiItems, loiIntZeroCalls, loiLandSeqNo = 0, loiRLegSeqNo = 0;
  char lochLeg, lochType;
  double loflNet = 0.0, loflVal = 0.0;
  toenBool loenStatus;

  fovdPushFunctionName("foenConServTab_New");

  /*
   * Allocate memory and init table
   */

  if ((lpstTab = (tostConServTab *) calloc(1, sizeof(tostConServTab))) == NULL)
    {
      sprintf (szTemp, "fpstConServTab_New: Can't calloc memory\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);   
      fovdPopFunctionName();
      return NULL;
    }

  for (i = 0; i < ENC_CAT_TAB_SIZE; i++)
    {
      if ((loenStatus = foenConServ_Init(&(lpstTab->sastConServ[i]), i)) == FALSE)
        {
          sprintf (szTemp, "fpstConServTab_New: Can't init service: %d\n", i);
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);   
          fovdPopFunctionName();
          return NULL;
        }
    }

  /*
   * Load all Net amounts for Usage Charges starting from G22 item seq 
   * after Contract G22 item
   */

  lpstG22 = g_22->g_22_next;
  while (lpstG22)
    {
      /*
       * Stop processing G22 seq if next contract found
       */

      if ((loiLevel = foiSumSheetItem_Level(lpstG22)) == 2)
        {
          break;
        }

      /*
       * Only G22 items on level 4 and Usage type are counted
       */

      
      if (loiLevel == 4 && (loiIndex = foiSumSheetItem_UsageClass(lpstG22, &lochLeg)) != ENC_UNKNOWN_CAT)
        {          
          fovdPrintLog (LOG_DEBUG, "fpstConServTab_New: Class: %d, %s\n", loiIndex, gasnzServLabel[loiIndex]);
          fovdPrintLog (LOG_DEBUG, "fpstConServTab_New: Con. Leg: %c\n", lochLeg);

          if ((loenStatus = foenSumSheetItem_Load(lpstG22, &loflNet, &loiItems)) == FALSE)
            {
              sprintf (szTemp, "fpstConServTab_New: Can't load service, category: %d, leg: %c\n", loiIndex, lochLeg);
              macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);   
              fovdPopFunctionName();
              return NULL;
            }
          
          fovdPrintLog (LOG_DEBUG, "fpstConServTab_New: Net value: %lf\n", loflNet);
          fovdPrintLog (LOG_DEBUG, "fpstConServTab_New: Items: %d\n", loiItems);
          
          lpstTab->sastConServ[loiIndex].soflNet += loflNet;

          /*
           * Number of connections
           */
          
          switch(loiIndex)
            {
            case ENC_LOCAL_USAGE_CAT:       /* only Air, CF parts with all destinations not like M? */
              if (lochLeg == 'A' || lochLeg == 'C')
                {
                  lpstTab->sastConServ[loiIndex].soiItems += loiItems;
                }
              break;
              
            case ENC_INTER_USAGE_CAT:       /* only Interconnect part with destination shdes like M? */             
              if (lochLeg == 'I')
                {
                  lpstTab->sastConServ[loiIndex].soiItems += loiItems;
                }
              break;
              
            case ENC_VPLMN_USAGE_CAT:       /* only Air part */
              if (lochLeg == 'A' || lochLeg == 'C')
                {
                  lpstTab->sastConServ[loiIndex].soiItems += loiItems;
                }
              break;
              
            case ENC_ROAMING_LEG_USAGE_CAT: /* all items with this part, may be accounted twice */
              lpstTab->sastConServ[loiIndex].soiItems += loiItems;
              break;
              
            case ENC_SURCHARGE_USAGE_CAT:   /* number of calls where surcharge was added, may be accounted twice */
              lpstTab->sastConServ[loiIndex].soiItems += loiItems;
              break;
            }
        }
      
      /*
       * Calls are used for counting number of non zero value calls in each category
       */
      
      if (loiLevel == 3 && lpstG22->g_99 != NULL)
        {
          lpstStartG99 = lpstG22->g_99;
        }
      
      lpstG22 = lpstG22->g_22_next;
    }

  fovdPrintLog (LOG_DEBUG, "fpstConServTab_New: Scanning calls for ENC_LOCAL_USAGE_CAT\n");    
  
  lpstTab->sastConServ[ENC_LOCAL_USAGE_CAT].soiItems = foiItemCallList_CountLocalCalls(ppstCallList_Local);
  lpstTab->sastConServ[ENC_LOCAL_USAGE_CAT].soiItems += foiItemCallList_CountLocalCalls(ppstCallList_Roaming);
  lpstTab->sastConServ[ENC_VPLMN_USAGE_CAT].soiItems = foiItemCallList_CountRoamingCalls(ppstCallList_Roaming);

  goflSimUsage_Local = lpstTab->sastConServ[ENC_LOCAL_USAGE_CAT].soflNet;  
  goflSimUsage_Roaming = lpstTab->sastConServ[ENC_VPLMN_USAGE_CAT].soflNet;  
  goflSimUsage_Surcharge = lpstTab->sastConServ[ENC_SURCHARGE_USAGE_CAT].soflNet;  

  /*
   * Save original call list pointer for filtering of some call subcategoies
   */

  for (i = 0; i < ENC_CAT_TAB_SIZE; i++)
    {
      lpstTab->sastConServ[i].spstG99 = lpstStartG99;
    }
  
#ifdef _STAT_
  fovdGetCallStat(lpstStartG99);
#endif

  fovdPopFunctionName();
  return lpstTab;
}

/********************************************************************************************************************
 *
 * foenConServTab_Gen
 *
 * DESCRIPTION:
 *
 ********************************************************************************************************************
 */

toenBool foenConServTab_Gen(tostConServTab *ppstTab)
{ 
  int i;
  toenBool loenStatus;
  toenBool loenMiscGen = FALSE;

  fovdPushFunctionName("foenConServ_New");

  /*
   * Print each category of calls
   */

  for (i = 0; i < ENC_CAT_TAB_SIZE; i++)
    {
      if ((loenStatus = foenConServ_IsEmpty(&(ppstTab->sastConServ[i]))) == FALSE)
        {
          if ((loenStatus = foenConServ_Gen(&(ppstTab->sastConServ[i]))) != TRUE)
            {
              fovdPopFunctionName();
              return FALSE;
            }
        }
    }
  
  fovdPopFunctionName();
  return TRUE;
}

/********************************************************************************************************************
 *
 * foenConServTab_Delete
 *
 * DESCRIPTION:
 *
 ********************************************************************************************************************
 */

toenBool foenConServTab_Delete(tostConServTab *ppstTab)
{
  fovdPushFunctionName("foenConServ_Delete");

  free(ppstTab);

  fovdPopFunctionName();
  return TRUE;
}

/********************************************************************************************************************
 *
 * foiConServTab_Size
 *
 * DESCRIPTION:
 *
 ********************************************************************************************************************
 */

int foiConServTab_Size(tostConServTab *ppstTab)
{
  int i, loiItems;
  toenBool loenStatus;

  for (i = 0, loiItems = 0; i < ENC_CAT_TAB_SIZE; i++)
    {
      if ((loenStatus = foenConServ_IsEmpty(&(ppstTab->sastConServ[i]))) == FALSE)
        {
          loiItems++;
        }
    }

  return loiItems;
}

#ifdef _STAT_
static void fovdGetCallStat(struct s_group_99 *ppstG99)
{
  struct s_group_99 *lpstG99;
  int loiClicksNo, laiAllClicksNo[2], n;
  double loflValue, laflAllValue[2];
  struct s_xcd_seg *lpstXcd;

  /*
   * Get clicks and amounts for all calls
   */
  
  laiAllClicksNo[0] = 0;
  laiAllClicksNo[1] = 0;
  laflAllValue[0] = 0.0;
  laflAllValue[1] = 0.0;

  lpstG99 = ppstG99;
  while (lpstG99)
    {
      lpstXcd = lpstG99->xcd;           
      
      /*
       * Clicks
       */

      sscanf(lpstXcd->v_X011, "%d", &loiClicksNo);
      laiAllClicksNo[0] += loiClicksNo;      
      
      sscanf(lpstXcd->v_X012, "%d", &loiClicksNo);
      laiAllClicksNo[1] += loiClicksNo;      

      /*
       * Value
       */
      
      sscanf(lpstXcd->v_5004c, "%lf", &loflValue);
      laflAllValue[0] += loflValue;

      sscanf(lpstXcd->v_5004, "%lf", &loflValue);
      laflAllValue[1] += loflValue;


      /*
       * Go to the next XCD block
       */
      
      lpstG99 = lpstG99->g_99_next;
    }  
  
  fovdPrintLog (LOG_TIMM, "Customer[%08d]: Clicks: %08d, %08d\n", goilCustomerId, laiAllClicksNo[0], laiAllClicksNo[1]);
  fovdPrintLog (LOG_TIMM, "Customer[%08d]: Value : %012.06lf, %012.06lf\n", goilCustomerId, laflAllValue[0], laflAllValue[1]);
}
#endif
