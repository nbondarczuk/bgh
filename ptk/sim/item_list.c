#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "bgh.h"
#include "protos.h"
#include "parser.h"
#include "types.h"
#include "gen.h"
#include "item_list.h"
#include "item_access.h"
#include "item_usage.h"
#include "item_subs.h"
#include "item_occ.h"
#include "item_interval.h"
#include "timm.h"

static char szTemp[128];

extern stBGHGLOB	stBgh;			/* structure with globals for BGH */

/*
  ================================================================================================
  =                                                                                              =
  =                                                                                              =
  = DATATYPE: Item (virtual)                                                                     =
  =                                                                                              =
  =                                                                                              = 
  ================================================================================================
*/

/**************************************************************************************************
 *                                                                                          
 * METHOD : Compare
 *                                                                                          
 * DESCRIPTION : 
 *                                                                                          
 **************************************************************************************************
 */

int foiItem_Compare(void *ppstA, void *ppstB)
{
  tostItem *lpstItemA, *lpstItemB;
  int rc = 1;
  
  fovdPushFunctionName("foenItem_Compare");

  lpstItemA = (tostItem *)ppstA;
  lpstItemB = (tostItem *)ppstB;
  
  ASSERT (lpstItemA->soenType != lpstItemB->soenType);
  if (lpstItemA->soenType == USAGE_TYPE)
    {
      rc = foiItemUsage_Compare((tostItemUsage *)lpstItemA->spvItem, (tostItemUsage *)lpstItemB->spvItem);
    }
  else
    {
      rc = 1;
    }
  
  fovdPopFunctionName();
  return rc;
}

/**************************************************************************************************
 *                                                                                          
 * METHOD : New
 *                                                                                          
 * DESCRIPTION : 
 *                                                                                          
 **************************************************************************************************
 */

tostItem *fpstItem_New(struct s_group_22 *ppstG22, toenItemType poenType, struct s_group_22 *ppstPrevG22)
{
  tostItem *lpstItem;

  fovdPushFunctionName("foenItem_New");

  if ((lpstItem = (tostItem *)calloc(1, sizeof(tostItem))) == NULL)
    {
      fovdPopFunctionName();
      return NULL;
    }

  lpstItem->soenType = poenType;
  
  fovdPrintLog (LOG_DEBUG, "foenItem_New: Creating new virtual item of type: %d\n", poenType);
  switch (poenType)
    {
    case ACCESS_TYPE:
      lpstItem->spvItem = (void *)fpstItemAccess_New(ppstG22);
      break;

    case USAGE_TYPE:
      lpstItem->spvItem = (void *)fpstItemUsage_New(ppstG22);
      break;

    case SUBS_TYPE:
      lpstItem->spvItem = (void *)fpstItemSubs_New(ppstG22);
      break;

    case OCC_TYPE:
      lpstItem->spvItem = (void *)fpstItemOCC_New(ppstG22);
      break;

    case INTERVAL_TYPE:
      lpstItem->spvItem = (void *)fpstItemInterval_New(ppstG22, ppstPrevG22);
      break;      
    }

  if (lpstItem->spvItem == NULL)
    {
      sprintf (szTemp, "fpstItem_New: Can't create virtual item type: %d\n", lpstItem->soenType);
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);      
      fovdPopFunctionName();
      return NULL;
    }

  fovdPrintLog (LOG_DEBUG, "foenItem_New: New virtual item created\n");

  fovdPopFunctionName();
  return lpstItem;
}

/**************************************************************************************************
 *                                                                                          
 * METHOD : Delete
 * DESCRIPTION : 
 *                                                                                          
 * DESCRIPTION : 
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenItem_Delete(tostItem *ppstItem)
{
  toenBool loenStatus = FALSE;

  fovdPushFunctionName("foenItem_Delete");

  fovdPrintLog (LOG_DEBUG, "foenItem_Delete: Deleting item of type: %d\n", ppstItem->soenType);
  switch (ppstItem->soenType)
    {
    case ACCESS_TYPE:
      loenStatus = foenItemAccess_Delete((tostItemAccess *)ppstItem->spvItem);
      fovdPrintLog (LOG_DEBUG, "foenItem_Delete: Access item deleted\n");
      break;

    case USAGE_TYPE:
      loenStatus = foenItemUsage_Delete((tostItemUsage *)ppstItem->spvItem);
      fovdPrintLog (LOG_DEBUG, "foenItem_Delete: Usage item deleted\n");
      break;

    case SUBS_TYPE:
      loenStatus = foenItemSubs_Delete((tostItemSubs *) ppstItem->spvItem);
      fovdPrintLog (LOG_DEBUG, "foenItem_Delete: Subs. item deleted\n");
      break;

    case OCC_TYPE:
      loenStatus = foenItemOCC_Delete((tostItemOCC *)ppstItem->spvItem);
      fovdPrintLog (LOG_DEBUG, "foenItem_Delete: OCC item deleted\n");
      break;

    case INTERVAL_TYPE:
      loenStatus = foenItemInterval_Delete((tostItemInterval *)ppstItem->spvItem);
      fovdPrintLog (LOG_DEBUG, "foenItem_Delete: Interval item deleted\n");
      break;

    default:
      sprintf (szTemp, "fpstItem_Delete: Can't recognize virtual item type: %d\n", ppstItem->soenType);
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);            
      fovdPopFunctionName();
      return FALSE;
    }

  free(ppstItem);

  fovdPrintLog (LOG_DEBUG, "foenItem_Delete: Virtual item deleted\n");
  fovdPopFunctionName();
  return TRUE;
}

/**************************************************************************************************
 *                                                                                          
 * METHOD : Match
 *                                                                                          
 * DESCRIPTION : 
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenItem_Match(tostItem *ppstItemA, tostItem *ppstItemB)
{
  toenBool loenStatus;

  fovdPushFunctionName("foenItem_Match");

  fovdPrintLog (LOG_DEBUG, "foenItem_Match: Matching item of type: %d\n", ppstItemA->soenType);
  switch (ppstItemA->soenType)
    {
    case ACCESS_TYPE:
      loenStatus = foenItemAccess_Match((tostItemAccess *)ppstItemA->spvItem, (tostItemAccess *)ppstItemB->spvItem);
      break;

    case USAGE_TYPE:
      loenStatus = foenItemUsage_Match((tostItemUsage *)ppstItemA->spvItem, (tostItemUsage *)ppstItemB->spvItem);
      break;

    case SUBS_TYPE:
      loenStatus = foenItemSubs_Match((tostItemSubs *)ppstItemA->spvItem, (tostItemSubs *)ppstItemB->spvItem);
      break;

    case OCC_TYPE:
      loenStatus = foenItemOCC_Match((tostItemOCC *)ppstItemA->spvItem, (tostItemOCC *)ppstItemB->spvItem);
      break;

    case INTERVAL_TYPE:
      loenStatus = foenItemInterval_Match((tostItemInterval *)ppstItemA->spvItem, (tostItemInterval *)ppstItemB->spvItem);
      break;      
    }

  fovdPrintLog (LOG_DEBUG, "foenItem_Match: Matched item of type: %d\n", ppstItemA->soenType);
  fovdPopFunctionName();
  return loenStatus;
}

/**************************************************************************************************
 *                                                                                          
 * METHOD : Merge
 *                                                                                          
 * DESCRIPTION : 
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenItem_Merge(tostItem *ppstItemA, tostItem *ppstItemB)
{
  toenBool loenStatus;

  fovdPushFunctionName("foenItem_Merge");

  fovdPrintLog (LOG_DEBUG, "foenItem_Merge: Merging items of type: %d\n", ppstItemA->soenType);
  switch (ppstItemA->soenType)
    {
    case ACCESS_TYPE:
      loenStatus = foenItemAccess_Merge((tostItemAccess *)ppstItemA->spvItem, (tostItemAccess *)ppstItemB->spvItem);
      break;

    case USAGE_TYPE:
      loenStatus = foenItemUsage_Merge((tostItemUsage *)ppstItemA->spvItem, (tostItemUsage *)ppstItemB->spvItem);
      break;

    case SUBS_TYPE:
      loenStatus = foenItemSubs_Merge((tostItemSubs *)ppstItemA->spvItem, (tostItemSubs *)ppstItemB->spvItem);
      break;

    case OCC_TYPE:
      loenStatus = foenItemOCC_Merge((tostItemOCC *)ppstItemA->spvItem, (tostItemOCC *)ppstItemB->spvItem);
      break;

    case INTERVAL_TYPE:
      loenStatus = foenItemInterval_Merge((tostItemInterval *)ppstItemA->spvItem, (tostItemInterval *)ppstItemB->spvItem);
      break;      
    } 

  loenStatus = foenItem_Delete(ppstItemB);

  fovdPrintLog (LOG_DEBUG, "foenItem_Merge: Merged items of type: %d\n", ppstItemA->soenType);
  fovdPopFunctionName();
  return loenStatus;
}

/**************************************************************************************************
 *                                                                                          
 * METHOD : Gen
 *                                                                                          
 * DESCRIPTION : 
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenItem_Gen(tostItem *ppstItem)
{
  toenBool loenStatus;

  fovdPushFunctionName("foenItem_Gen");

  fovdPrintLog (LOG_DEBUG, "foenItem_Gen: Printing item of type: %d\n", ppstItem->soenType);
  switch (ppstItem->soenType)
    {
    case ACCESS_TYPE:
      loenStatus = foenItemAccess_Gen((tostItemAccess *)ppstItem->spvItem);
      break;

    case USAGE_TYPE:
      
      /*
       * USAGE list in not atomic - it contains sublists: VPLMN type and HPLMN type
       * and for each of them summary must be printed.
       */

      loenStatus = FALSE;
      break;

    case SUBS_TYPE:
      loenStatus = foenItemSubs_Gen((tostItemSubs *)ppstItem->spvItem);
      break;

    case OCC_TYPE:
      loenStatus = foenItemOCC_Gen((tostItemOCC *)ppstItem->spvItem);
      break;

    case INTERVAL_TYPE:
      loenStatus = foenItemInterval_Gen((tostItemInterval *)ppstItem->spvItem);
      break;
    }

  fovdPrintLog (LOG_DEBUG, "foenItem_Gen: Printed item of type: %d\n", ppstItem->soenType);
  fovdPopFunctionName();
  return loenStatus;
}

/**************************************************************************************************
 *                                                                                          
 * METHOD : Empty
 *                                                                                          
 * DESCRIPTION : Checking if item is to be processed
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenItem_Empty(tostItem *ppstItem)
{
  toenBool loenStatus;

  fovdPushFunctionName("foenItem_Empty");

  fovdPrintLog (LOG_DEBUG, "foenItem_Empty: Testing item of type: %d\n", ppstItem->soenType);
  switch (ppstItem->soenType)
    {
    case ACCESS_TYPE:
      loenStatus = foenItemAccess_Empty((tostItemAccess *)ppstItem->spvItem);
      break;

    case USAGE_TYPE:
      loenStatus = foenItemUsage_Empty((tostItemUsage *)ppstItem->spvItem);
      break;

    case SUBS_TYPE:
      loenStatus = foenItemSubs_Empty((tostItemSubs *)ppstItem->spvItem);
      break;

    case OCC_TYPE:
      loenStatus = foenItemOCC_Empty((tostItemOCC *)ppstItem->spvItem);
      break;

    case INTERVAL_TYPE:
      loenStatus = foenItemInterval_Empty((tostItemInterval *)ppstItem->spvItem);
      break;
    }

  fovdPrintLog (LOG_DEBUG, "foenItem_Empty: Tested item of type: %d\n", ppstItem->soenType);
  fovdPopFunctionName();
  return loenStatus;
}

/*
  ================================================================================================
  =                                                                                              =
  =                                                                                              =
  = DATATYPE: ItemNode (generic)                                                                 =
  =                                                                                              =
  =                                                                                              = 
  ================================================================================================
*/

/**************************************************************************************************
 *                                                                                          
 * METHOD : Compare
 *                                                                                          
 * DESCRIPTION : 
 *                                                                                          
 **************************************************************************************************
 */

int foiItemNode_Compare(const void *ppstNodeA, const void *ppstNodeB)
{
  tostItemNode *lpstNodeA, *lpstNodeB;
  int rc;

  fovdPushFunctionName("foenItemNode_Compare");

  lpstNodeA = (tostItemNode *)ppstNodeA;
  lpstNodeB = (tostItemNode *)ppstNodeB;

  rc = foiItem_Compare(lpstNodeA->spstItem, lpstNodeB->spstItem);
  
  fovdPopFunctionName();
  return rc;
}

/**************************************************************************************************
 *                                                                                          
 * METHOD : New
 *                                                                                          
 * DESCRIPTION : 
 *                                                                                          
 **************************************************************************************************
 */

static tostItemNode *fpstItemNode_New(tostItem *ppstItem)
{
  tostItemNode *lpstNode;

  fovdPushFunctionName("foenItemNode_New");

  if ((lpstNode = (tostItemNode *)calloc(1, sizeof(tostItemNode))) == NULL)
    {
      fovdPopFunctionName();
      return NULL;
    }

  /*
   * Shallow copy of item
   */

  lpstNode->spstItem = ppstItem;
  
  fovdPopFunctionName();
  return lpstNode;
}

/**************************************************************************************************
 *                                                                                          
 * METHOD : Delete
 *                                                                                          
 * DESCRIPTION : 
 *                                                                                          
 **************************************************************************************************
 */

static toenBool foenItemNode_Delete(tostItemNode *ppstNode)
{
  toenBool loenStatus;

  fovdPushFunctionName("foenItemNode_Delete");

  if (ppstNode == NULL)
    {
      fovdPopFunctionName();
      return TRUE;
    }
  else
    {
      if (ppstNode->spstItem != NULL)
        {
          loenStatus = foenItem_Delete(ppstNode->spstItem);
          if (loenStatus == FALSE)
            {
              fovdPopFunctionName();
              return FALSE;
            }
        }
      
      loenStatus = foenItemNode_Delete(ppstNode->spstNext);
      if (loenStatus == FALSE)
        {
          fovdPopFunctionName();
          return FALSE;
        }

      free(ppstNode);
    }
  
  fovdPopFunctionName();
  return loenStatus;
}

/**************************************************************************************************
 *                                                                                          
 * METHOD : Insert
 *                                                                                          
 * DESCRIPTION : 
 *                                                                                          
 **************************************************************************************************
 */

static toenBool foenItemNode_Insert(tostItemNode *ppstNode, tostItem *ppstItem)
{
  toenBool loenStatus;

  fovdPushFunctionName("foenItemNode_Insert");
  
  if ((loenStatus = foenItem_Match(ppstNode->spstItem, ppstItem)) == TRUE)
    {
      fovdPrintLog (LOG_DEBUG, "foenItemNode_Insert: Merging items\n");
      loenStatus = foenItem_Merge(ppstNode->spstItem, ppstItem);
    }    
  else if (ppstNode->spstNext == NULL)
    {
      ppstNode->spstNext = fpstItemNode_New(ppstItem);
      loenStatus = TRUE;
    }
  else
    {
      loenStatus = foenItemNode_Insert(ppstNode->spstNext, ppstItem);
    }

  fovdPopFunctionName();
  return loenStatus;
}


/**************************************************************************************************
 *                                                                                          
 * METHOD : Apply
 *                                                                                          
 * DESCRIPTION : 
 *                                                                                          
 **************************************************************************************************
 */

static toenBool foenItemNode_Apply(tostItemNode *ppstNode, toenBool (*foenFun)(tostItem *))
{
  toenBool loenStatus;

  fovdPushFunctionName("foenItemNode_Apply");

  loenStatus = foenFun(ppstNode->spstItem);
  if (loenStatus == FALSE)
    {
      fovdPopFunctionName();
      return FALSE;
    }

  if (ppstNode->spstNext != NULL)
    {
      loenStatus = foenItemNode_Apply(ppstNode->spstNext, foenFun);
      if (loenStatus == FALSE)
        {
          fovdPopFunctionName();
          return FALSE;
        }
    }

  fovdPopFunctionName();
  return loenStatus;
}

/**************************************************************************************************
 *                                                                                          
 * METHOD : Count
 *                                                                                          
 * DESCRIPTION : 
 *                                                                                          
 **************************************************************************************************
 */

static int foiItemNode_Count(tostItemNode *ppstNode)
{
  int loiCount;

  fovdPushFunctionName("foenItemNode_Count");

  if (ppstNode == NULL)
    {
      fovdPopFunctionName();
      return 0;
    }
  
  loiCount = foiItemNode_Count(ppstNode->spstNext);

  fovdPopFunctionName();
  return loiCount + 1;
}

/**************************************************************************************************
 *                                                                                          
 * METHOD : Gen
 *                                                                                          
 * DESCRIPTION : 
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenItemNode_Gen(tostItemNode *ppstNode)
{
  toenBool loenStatus;

  fovdPushFunctionName("foenItemNode_Gen");

  if (ppstNode == NULL)
    {
      fovdPopFunctionName();
      return TRUE;
    }

  loenStatus = foenItemNode_Gen(ppstNode->spstNext);
  if (loenStatus == FALSE)
    {
      fovdPopFunctionName();
      return FALSE;
    }

  loenStatus = foenItem_Gen(ppstNode->spstItem);
  if (loenStatus == FALSE)
    {
      fovdPopFunctionName();
      return FALSE;
    }

  fovdPopFunctionName();
  return loenStatus;
}

/*
  ================================================================================================
  =                                                                                              =
  =                                                                                              =
  = DATATYPE: ItemList (generic)                                                                 =
  =                                                                                              =
  =                                                                                              = 
  ================================================================================================
*/


/**************************************************************************************************
 *                                                                                          
 * METHOD : Delete
 *                                                                                          
 * DESCRIPTION : 
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenItemList_Delete(tostItemList *ppstList)
{
  toenBool loenStatus;

  fovdPushFunctionName("foenItemList_Delete");

  if (ppstList->spstRoot == NULL)
    {
      /*
       * List is empty
       */

      fovdPrintLog (LOG_DEBUG, "foenItemList_Delete: Deleting from the empty list\n");
      loenStatus = TRUE;
    }
  else
    {
      /*
       * Delete each node
       */

      fovdPrintLog (LOG_DEBUG, "foenItemList_Delete: Deleting the list\n");
      loenStatus = foenItemNode_Delete(ppstList->spstRoot);
    }

  /*
   * Dynamic structure allocated from the heap must be free
   */
  
  free(ppstList);

  fovdPopFunctionName();
  return loenStatus;
}



/**************************************************************************************************
 *                                                                                          
 * METHOD : Insert
 *                                                                                          
 * DESCRIPTION : 
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenItemList_Insert(tostItemList *ppstList, tostItem *ppstItem)
{
  toenBool loenStatus = TRUE;

  fovdPushFunctionName("foenItemList_Insert");

  if (ppstList->spstRoot == NULL)
    {
      fovdPrintLog (LOG_DEBUG, "foenItemList_Insert: Inserting to the new list of items\n");
      if ((ppstList->spstRoot = fpstItemNode_New(ppstItem)) == NULL)
        {
          loenStatus = FALSE;
        }
    }
  else
    {
      fovdPrintLog (LOG_DEBUG, "foenItemList_Insert: Inserting to the list of items\n");
      loenStatus = foenItemNode_Insert(ppstList->spstRoot, ppstItem);
    }
  
  fovdPopFunctionName();
  return loenStatus;
}

/**************************************************************************************************
 *                                                                                          
 * METHOD : New
 *                                                                                          
 * DESCRIPTION : 
 *                                                                                          
 **************************************************************************************************
 */

tostItemList *fpstItemList_New(struct s_group_22 *ppstG22, toenItemType poenType, struct s_group_22 *ppstPrevG22)
{
  struct s_group_22 *lpstG22;
  struct s_imd_seg *lpstImd;
  struct s_moa_seg *lpstMoa;
  struct s_lin_seg *lpstLin;
  tostItemList *lpstList;
  tostItem *lpstItem;
  toenBool loenStatus;
  char *lpsnzLevel;
  int n;

  fovdPushFunctionName("fpstItemList_New");

  lpstList = (tostItemList *)calloc(1, sizeof(tostItemList));
  if (lpstList == NULL)
    {
      sprintf (szTemp, "fpstItemList_New: Can't malloc memory: %d bytes\n", sizeof(tostItemList));
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      fovdPopFunctionName();
      return NULL;
    }

  lpstList->soiSize = 0;
  lpstList->spstRoot = NULL;

  if (poenType == INTERVAL_TYPE)
    {
      lpsnzLevel = "05";
    }
  else
    {
      lpsnzLevel = "04";
    }

  fovdPrintLog (LOG_DEBUG, "fpstItemList_New: New list of G22 items on level: %s\n", lpsnzLevel);
  
  lpstG22 = ppstG22->g_22_next;  
  while (lpstG22)
    {
      lpstLin = lpstG22->lin;
      
      if (EQ(lpstLin->v_1222, lpsnzLevel))
        {
          /*
           * Create item for a given type
           */
          
          fovdPrintLog (LOG_DEBUG, "fpstItemList_New: New G22 item type: %d on level: %s\n", poenType, lpsnzLevel);
          lpstItem = fpstItem_New(lpstG22, poenType, ppstPrevG22);
          if (lpstItem == NULL)
            {
              sprintf (szTemp, "fpstItemList_New: Can't create virtual item with type: %d\n", poenType);
              macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
              fovdPopFunctionName();
              return NULL;
            }
          
          if (foenItem_Empty(lpstItem) == FALSE || poenType == ACCESS_TYPE)
            {
              fovdPrintLog (LOG_DEBUG, "fpstItemList_New: Inserting new item to the list\n");
              loenStatus = foenItemList_Insert(lpstList, lpstItem);
              if (loenStatus == FALSE)
                {
                  fovdPrintLog (LOG_DEBUG, "fpstItemList_New: Deleting merged item\n");
                  loenStatus == foenItem_Delete(lpstItem);
                  if (loenStatus == FALSE)
                    {
                      sprintf (szTemp, "fpstItemList_New: Can't delete item mreged being, type: %d on level: %s\n", poenType, lpsnzLevel);
                      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
                      fovdPopFunctionName();
                      return NULL;
                    }                  
                }
              else
                {
                  fovdPrintLog (LOG_DEBUG, "fpstItemList_New: Inserted new item to the list\n");
                  lpstList->soiSize++;
                }
            }
          else
            {
              fovdPrintLog (LOG_DEBUG, "fpstItemList_New: New empty item suppressed\n");
              loenStatus = foenItem_Delete(lpstItem);
              if (loenStatus == FALSE)
                {
                  sprintf (szTemp, "fpstItemList_New: Can't delete empty item being suppressed, type: %d on level: %s\n", poenType, lpsnzLevel);
                  macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
                  fovdPopFunctionName();
                  return NULL;
                }
            }
        }
      else if (poenType == ACCESS_TYPE && EQ(lpstLin->v_1222, "05"))
        {
          /* 
           * Go to next item on the list 
           */

          fovdPrintLog (LOG_DEBUG, "fpstItemList_New: Skipping G22 item\n");
        }
      else
        {
          /*
           * Other level - out of sequence of G22 items
           */

          fovdPrintLog (LOG_DEBUG, "fpstItemList_New: Out of sequence of G22 items on level: %s\n", lpsnzLevel);
          break;
        }

      lpstG22 = lpstG22->g_22_next;
    }

  fovdPopFunctionName();
  return lpstList;  
}

/**************************************************************************************************
 *                                                                                          
 * METHOD : Apply
 *                                                                                          
 * DESCRIPTION : 
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenItemList_Apply(tostItemList *ppstList, toenBool (*foenFun)(tostItem *))
{
  toenBool loenStatus;

  if (ppstList->spstRoot == NULL)
    {
      fovdPopFunctionName();  
      return TRUE;
    }

  loenStatus = foenItemNode_Apply(ppstList->spstRoot, foenFun);

  fovdPopFunctionName();  
  return loenStatus;
}

/**************************************************************************************************
 *                                                                                          
 * METHOD : Count
 *                                                                                          
 * DESCRIPTION : 
 *                                                                                          
 **************************************************************************************************
 */

int foiItemList_Count(tostItemList *ppstList)
{
  int n;

  fovdPushFunctionName("foenItemList_Count");

  n = ppstList->soiSize;

  /*
  if (ppstList->spstRoot == NULL)
    {
      fovdPopFunctionName();  
      return 0;
    }

  n = foiItemNode_Count(ppstList->spstRoot);
  */
  
  fovdPopFunctionName();  
  return n;
}

/**************************************************************************************************
 *                                                                                          
 * METHOD : Gen
 *                                                                                          
 * DESCRIPTION : 
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenItemList_Gen(tostItemList *ppstList)
{
  toenBool loenStatus;
  tostItemNode *lpstNode;
  toenPLMNType paenType[2] = {HPLMN_TYPE, VPLMN_TYPE};
  char *lasnzPLMNTypeRep[2] = {"HPLMN", "VPLMN"};
  int i;

  fovdPushFunctionName("foenItemList_Gen");

  fovdPrintLog (LOG_DEBUG, "foenItemList_Gen: Printing item list\n");
  
  if (ppstList->spstRoot == NULL)
    {
      fovdPrintLog (LOG_DEBUG, "foenItemList_Gen: List empty\n");
      fovdPopFunctionName();  
      return TRUE;
    }
  
  lpstNode = ppstList->spstRoot;

  /*
   * Empty node (without item) is not possible state of node
   */

  ASSERT(lpstNode->spstItem != NULL);

  if (lpstNode->spstItem->soenType == USAGE_TYPE)
    {
      /*
       * There are two sublists: VPLMN and HPLMN type call category lists
       * they have summary that must be printed after the list.
       * The summary is printed with function foenItemUsage_GenSummary
       * which is type specific function for ItemUsage. The data are internal 
       * for the module ItemUsage. They are flushed by function foenItemUsage_GenSummary.
       */

      fovdPrintLog (LOG_DEBUG, "foenItemList_Gen: Printing USAGE item list\n");
      for (i = 0; i < 2; i++)
        {
          fovdPrintLog (LOG_DEBUG, "foenItemList_Gen: Printing USAGE node type: %s\n", lasnzPLMNTypeRep[i]);
          lpstNode = ppstList->spstRoot;
          while (lpstNode)
            {
              loenStatus = foenItemUsage_Gen((tostItemUsage *)lpstNode->spstItem->spvItem, paenType[i]);
              if (loenStatus == FALSE)
                {
                  fovdPopFunctionName();  
                  return FALSE;
                }
              
              lpstNode = lpstNode->spstNext;
            }      

          fovdPrintLog (LOG_DEBUG, "foenItemList_Gen: Printing USAGE item list summary\n");
          lpstNode = ppstList->spstRoot;
          loenStatus = foenItemUsage_SummaryGen((tostItemUsage *)lpstNode->spstItem->spvItem, paenType[i]);

        }

    }
  else
    {
      fovdPrintLog (LOG_DEBUG, "foenItemList_Gen: Printing not USAGE item list\n");
      loenStatus = foenItemNode_Gen(ppstList->spstRoot);
    }
  
  if (loenStatus == TRUE)
    {
      fovdPrintLog (LOG_DEBUG, "foenItemList_Gen: Printed item list\n");
    }
  else
    {
      fovdPrintLog (LOG_DEBUG, "foenItemList_Gen: Can't print item list\n");
    }

  fovdPopFunctionName();  
  return loenStatus;
}

/**************************************************************************************************
 *                                                                                          
 * METHOD : Sort
 *                                                                                          
 * DESCRIPTION : 
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenItemList_List2Array(tostItemList *ppstList, tostItemNode ***ppastNodeArray, int *ppiNodes);
toenBool foenItemList_Array2List(tostItemNode **papstNodeArray, int poiNodes, tostItemList *ppstList);

toenBool foenItemList_Sort(tostItemList *ppstList)
{
  toenBool loenStatus;
  tostItemNode **lapstNodeArray;
  int loiNodes;

  fovdPushFunctionName("foenItemList_Sort");

  fovdPrintLog (LOG_DEBUG, "foenItemList_Sort: Sorting list of items\n");

  if (ppstList->spstRoot == NULL)
    {
      fovdPrintLog (LOG_DEBUG, "foenItemList_Sort: List empty\n");
      fovdPopFunctionName();  
      return TRUE;
    }

  /*
   * Array contains pointers to nodes that may be compared
   */

  fovdPrintLog (LOG_DEBUG, "foenItemList_Sort: Creating array of item nodes\n");
  loenStatus = foenItemList_List2Array(ppstList, &lapstNodeArray, &loiNodes);
  if (loenStatus == FALSE)
    {
      fovdPopFunctionName();  
      return FALSE;
    }
  
  fovdPrintLog (LOG_DEBUG, "foenItemList_Sort: Array of nodes created with size: %d\n", loiNodes);
  
  if (loiNodes > 0)
    {
      /*
       * Sort using type specific rule 
       */

      fovdPrintLog (LOG_DEBUG, "foenItemList_Sort: Sorting array of items\n");
      qsort(lapstNodeArray, (size_t)loiNodes, sizeof(tostItemNode *), foiItemNode_Compare);

      /*
       * Convert array to new list
       */
      
      fovdPrintLog (LOG_DEBUG, "foenItemList_Sort: Creating list of item nodes\n");
      loenStatus = foenItemList_Array2List(lapstNodeArray, loiNodes, ppstList);
      if (loenStatus == FALSE)
        {
          fovdPopFunctionName();  
          return FALSE;
        }
    }
  else
    {
      fovdPrintLog (LOG_DEBUG, "foenItemList_Sort: Can't sort array with size: %d\n", loiNodes);
    }
  
  fovdPopFunctionName();  
  return loenStatus;
}


/**************************************************************************************************
 *                                                                                          
 * METHOD : List2Array
 *                                                                                          
 * DESCRIPTION : 
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenItemList_List2Array(tostItemList *ppstList, tostItemNode ***ppastNodeArray, int *ppiNodes)
{
  tostItemNode **lapstNodeArray;
  tostItemNode *lpstNode;
  int i, loiNodes;

  fovdPushFunctionName("foenItemList_List2Array");

  fovdPrintLog (LOG_DEBUG, "foenItemList_List2Array: Converting list to array\n");

  /*
   * Table allocation - deallocated in Array2List 
   */

  loiNodes = foiItemList_Count(ppstList);
  fovdPrintLog (LOG_DEBUG, "foenItemList_List2Array: List size: %d\n", loiNodes);

  if (loiNodes == 0)
    {
      fovdPrintLog (LOG_DEBUG, "foenItemList_List2Array: Empty list\n");      
      *ppastNodeArray = NULL;
      *ppiNodes = 0;

      fovdPopFunctionName();  
      return TRUE;
    }
  
  fovdPrintLog (LOG_DEBUG, "foenItemList_List2Array: Allocating memory %d bytes\n", sizeof(tostItemNode *) * loiNodes);      
  lapstNodeArray = (tostItemNode **)calloc(loiNodes, sizeof(tostItemNode *)); 
  if (lapstNodeArray == NULL)
    {
      fovdPopFunctionName();  
      return FALSE;
    }

  /*
   * Fill table with items not removing connections from nodes of the list 
   */

  i = 0;
  lpstNode = ppstList->spstRoot;
  fovdPrintLog (LOG_DEBUG, "foenItemList_List2Array: Copy each list item to table\n");      
  while (lpstNode != NULL)
    {
      lapstNodeArray[i++] = lpstNode;
      fovdPrintLog (LOG_DEBUG, "foenItemList_List2Array: Copy item: %d\n", i);
      lpstNode = lpstNode->spstNext;
    }

  /*
   * Array and its size must be returned
   */
  
  fovdPrintLog (LOG_DEBUG, "foenItemList_List2Array: Returning array ptr and size\n");      
  *ppastNodeArray = lapstNodeArray;
  *ppiNodes = i;

  fovdPopFunctionName();  
  return TRUE;
}


/**************************************************************************************************
 *                                                                                          
 * METHOD : Array2List
 *                                                                                          
 * DESCRIPTION : 
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenItemList_Array2List(tostItemNode **papstNodeArray, int poiNodes, tostItemList *ppstList)
{
  int i;
  tostItemNode *lpstNode;

  fovdPushFunctionName("foenItemList_Array2List");

  fovdPrintLog (LOG_DEBUG, "foenItemList_Array2List: Converting array to list\n");

  /*
   * List is empty but nodes aren't deallocated
   */

  ppstList->spstRoot = NULL;

  /*
   * for each item in a table 
   */

  fovdPrintLog (LOG_DEBUG, "foenItemList_Array2List: Copy each item to the list\n");
  for (i = 0; i < poiNodes; i++)
    {
      fovdPrintLog (LOG_DEBUG, "foenItemList_Array2List: Copy item: %d\n", i);
      lpstNode = papstNodeArray[i];
      ASSERT(lpstNode != NULL);
      
      /*
       * Append item
       */

      fovdPrintLog (LOG_DEBUG, "foenItemList_Array2List: Appending item: %d\n", i);
      lpstNode->spstNext = ppstList->spstRoot;
      ppstList->spstRoot = lpstNode;
    }

  /*
   * Free resources llocated in function foenItemList_List2Array
   */

  fovdPrintLog (LOG_DEBUG, "foenItemList_Array2List: Free array\n");
  free(papstNodeArray);

  fovdPopFunctionName();  
  return TRUE;
}

/**************************************************************************************************
 *                                                                                          
 * METHOD : Init
 *                                                                                          
 * DESCRIPTION : 
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenItemList_Init(tostItemList *ppstList, struct s_group_22 *ppstG22)
{
  struct s_group_22 *lpstG22;
  struct s_imd_seg *lpstImd;
  struct s_moa_seg *lpstMoa;
  struct s_lin_seg *lpstLin;
  toenBool loenStatus, loenCoIdFound = FALSE;
  int n, loiAmountsCat = 0;
  
  fovdPushFunctionName("fpstItemList_Init");

  fovdPrintLog (LOG_DEBUG, "foenItemList_Init: Loading G22\n");
  lpstG22 = ppstG22;  
  while (lpstG22) 
    {
      lpstLin = lpstG22->lin;
      
      /*
       * Fill List info fields with values found in the G22 item list
       */
      
      if (EQ(lpstLin->v_1222, "02") && loenCoIdFound == FALSE)
        {
          /*
           * Contract level
           */
          
          fovdPrintLog (LOG_DEBUG, "foenItemList_Init: Loading G22 level 02\n");
          ppstList->spstG22 = lpstG22;
          lpstImd = fpstFindItemDescription(lpstG22->imd, "CO");
          if (lpstImd != NULL)
            {
              n = sscanf(lpstImd->v_7008a, "%ld", &(ppstList->soilCoId));
              ASSERT(n == 1);
              loenCoIdFound = FALSE;
              fovdPrintLog (LOG_DEBUG, "Contract : %8ld\n", ppstList->soilCoId);
            }
        }
      
      else if (EQ(lpstLin->v_1222, "03") & loiAmountsCat < 4)
        {
          /*
           * Item level: Usage, Access, OCC, Subscription
           */
      
          fovdPrintLog (LOG_DEBUG, "foenItemList_Init: Loading G22 level 03\n");
          lpstImd = fpstFindItemDescription(lpstG22->imd, "CT");         
          lpstMoa = fpstFindPaymentSegment(lpstG22->g_23, "932", "9"); 
          if (lpstImd != NULL && lpstMoa != NULL)
            {
              switch(lpstImd->v_7008a[0])
                {
                case 'O': /* OCC */
                  n = sscanf(lpstMoa->v_5004, "%lf", &(ppstList->soflBCHSumOCC));
                  fovdPrintLog (LOG_DEBUG, "OCC      : %8.02lf\n", ppstList->soflBCHSumOCC);
                  break;
                  
                case 'U': /* Usage */
                  n = sscanf(lpstMoa->v_5004, "%lf", &(ppstList->soflBCHSumUsage));
                  fovdPrintLog (LOG_DEBUG, "Usage    : %8.02lf\n", ppstList->soflBCHSumUsage);
                  break;
              
                case 'S': /* Subscription */
                  n = sscanf(lpstMoa->v_5004, "%lf", &(ppstList->soflBCHSumSubscription));
                  fovdPrintLog (LOG_DEBUG, "Subscr.  : %8.02lf\n", ppstList->soflBCHSumSubscription);
                  break;
                  
                case 'A': /* Access */
                  n = sscanf(lpstMoa->v_5004, "%lf", &(ppstList->soflBCHSumAccess));
                  fovdPrintLog (LOG_DEBUG, "Access   : %8.02lf\n", ppstList->soflBCHSumAccess);
                  break;
                }
              
              ASSERT(n == 1);
              loiAmountsCat ++;
            }
        }

      else if (EQ(lpstLin->v_1222, "03") || EQ(lpstLin->v_1222, "02"))
        {
          break;
        }
      
      lpstG22 = lpstG22->g_22_next;
    }

  fovdPopFunctionName();
  return TRUE;  
}
