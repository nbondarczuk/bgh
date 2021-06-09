#include <stdlib.h>
#include <strings.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "bgh.h"
#include "protos.h"
#include "parser.h"
#include "types.h"
#include "gen.h"
#include "vat_inv.h"

#if 0	/* just for version.sh */
static char *SCCS_VERSION = "4.0";
#endif

#ifdef _MACHINE_ROUNDING_
#define ROUND(v) foflRound(v)
#else
#define ROUND(v) v
#endif

/*
 * extern functions
 */

extern void fovdFormatMoney(char *pachzMoney);

extern double foflMoney_Round(double poflAmount, 
                              double *ppflRest);

extern void fovdFormatNumber(char *poszNo);

/*
 * extern variables
 */

extern toenBool goenCustomerIsExempt;

/*
 * static functions
 */

/*
 * static variables
 */

/********************************************************************************************************************
 *
 * foenVATSumNode_Print
 *
 ********************************************************************************************************************
 */

toenBool foenVATSumNode_Print(tostVATSumNode *ppstNode)
{
  printf("Tax name: %s\n", ppstNode->sasnzTaxName);
  printf("Tax rate: %s\n", ppstNode->sasnzTaxRate);
  printf("Tax code: %s\n", ppstNode->sasnzTaxLegalCode);
  printf("Net amount : %lf\n", ppstNode->soflNetAmount);
  printf("VAT amount : %lf\n", ppstNode->soflVATAmount);  
  printf("Brut amount: %lf\n", ppstNode->soflBrutAmount);  

  return TRUE;
}

/********************************************************************************************************************
 *
 * foenVATSumNode_Sum
 *
 ********************************************************************************************************************
 */

toenBool foenVATSumNode_Sum(tostVATSumNode *ppstNode, double *ppflNet, double *ppflVAT, double *ppflBrut)
{
  fovdPushFunctionName ("foenVATSumNode_Sum");

  *ppflNet += ppstNode->soflNetAmount;
  *ppflVAT += ppstNode->soflVATAmount;  
  *ppflBrut += ppstNode->soflBrutAmount;  
  
  fovdPopFunctionName ();
  return TRUE;
}

/********************************************************************************************************************
 *
 * foenVATSumNode_Round
 *
 ********************************************************************************************************************
 */

toenBool foenVATSumNode_Round(tostVATSumNode *ppstNode, double poflDiscAmount)
{
  double loflRest;

  ppstNode->soflVATAmount = foflMoney_Round(ppstNode->soflVATAmount, 
                                            &loflRest);  

  ppstNode->soflBrutAmount = foflMoney_Round(ppstNode->soflBrutAmount,
                                             &loflRest);  
  
  ppstNode->soflNetAmount = ppstNode->soflBrutAmount - ppstNode->soflVATAmount;

  return TRUE;
}

/********************************************************************************************************************
 *
 * foenVATSumNode_Delete
 *
 ********************************************************************************************************************
 */

toenBool foenVATSumNode_Delete(tostVATSumNode *ppstNode)
{
  free(ppstNode);

  return TRUE;
}

/********************************************************************************************************************
 *
 * foenVATSumNode_Gen
 *
 ********************************************************************************************************************
 */

toenBool foenVATSumNode_Gen(tostVATSumNode *ppstNode, double poflDiscountAmount)
{
  static char lasnzNet[MAX_BUFFER];
  static char lasnzBrut[MAX_BUFFER];
  static char lasnzVAT[MAX_BUFFER];
  char lasnzCurrency[TMP_BUFFER_SIZE];

  strncpy(lasnzCurrency, LOCAL_CURRENCY_CODE, TMP_BUFFER_SIZE);

  sprintf(lasnzNet, "%lf", ppstNode->soflNetAmount);
  fovdFormatMoney(lasnzNet);
  
  sprintf(lasnzVAT, "%lf", ppstNode->soflVATAmount);
  fovdFormatMoney(lasnzVAT);

  sprintf(lasnzBrut, "%lf", ppstNode->soflBrutAmount);
  fovdFormatMoney(lasnzBrut);

  /*
  fovdFormatNumber(ppstNode->sasnzTaxRate);
  */

  fovdGen("InvoiceVATSumItem", 
          ppstNode->sasnzTaxLegalCode, ppstNode->sasnzTaxName, 
          ppstNode->sasnzTaxRate, 
          lasnzNet, lasnzVAT, lasnzBrut, lasnzCurrency, 
          EOL);

  fovdPrintLog(LOG_TIMM, "Tax rate: %s\n", ppstNode->sasnzTaxRate);
  fovdPrintLog(LOG_TIMM, "Brut sum: %s\n", lasnzBrut);
  fovdPrintLog(LOG_TIMM, "VAT sum : %s\n", lasnzVAT);
  fovdPrintLog(LOG_TIMM, "Net sum : %s\n", lasnzNet);

  return TRUE;
}

/********************************************************************************************************************
 *
 * foenVATSumNode_Add
 *
 ********************************************************************************************************************
 */

toenBool foenVATSumNode_Add(tostVATSumNode *ppstNode, struct s_group_22 *g_22)
{
  tostVATSumNode *lpstNode;
  toenBool loenStatus;
  double loflTax = 0.0;
  double loflNet = 0.0;
  double loflBrut = 0.0;
  double loflDiscount = 0.0;

  if ((loenStatus = foenService_Load(g_22, 
                                     &loflNet, 
                                     &loflTax, 
                                     &loflBrut,
                                     &loflDiscount)) == FALSE)
    {
      return FALSE;
    }
  
  ppstNode->soflNetAmount += loflNet;
  ppstNode->soflVATAmount += loflTax;
  ppstNode->soflBrutAmount += loflBrut;
  ppstNode->soflDiscountAmount += loflDiscount;
  
  return TRUE;
}

/********************************************************************************************************************
 *
 * fpstVATSumNode_New
 *
 ********************************************************************************************************************
 */

struct tostVATSumNode *fpstVATSumNode_New(struct s_group_22 *g_22)
{
  struct tostVATSumNode *lpstNode;
  toenBool loenStatus;

  if ((lpstNode = (struct tostVATSumNode *)malloc(sizeof(struct tostVATSumNode))) == NULL)
    {
      return NULL;
    }

  lpstNode->soflNetAmount = 0.0;
  lpstNode->soflVATAmount = 0.0;
  lpstNode->soflBrutAmount = 0.0;
  lpstNode->soflDiscountAmount = 0.0;
  lpstNode->soflVATRate = 0.0;

  if ((loenStatus = foenService_Load(g_22, 
                                     &(lpstNode->soflNetAmount), 
                                     &(lpstNode->soflVATAmount), 
                                     &(lpstNode->soflBrutAmount),
                                     &(lpstNode->soflDiscountAmount))) == FALSE)
    {
      return NULL;
    }
  
  strcpy(lpstNode->sasnzTaxName, g_22->g_30->tax->v_5152);
  strcpy(lpstNode->sasnzTaxRate, g_22->g_30->tax->v_5278);
  strcpy(lpstNode->sasnzTaxLegalCode, g_22->g_30->tax->v_5279);
  (void) sscanf(lpstNode->sasnzTaxRate, "%lf", &(lpstNode->soflVATRate));
  
  
  return lpstNode;
}

/********************************************************************************************************************
 *
 * fpstVATSumNode_Find
 *
 ********************************************************************************************************************
 */

tostVATSumNode *fpstVATSumNode_Find(tostVATSumNode *ppstNode, struct s_group_22 *g_22)
{
  tostVATSumNode *lpstNode;
  
  if (ppstNode == NULL)
    {
      return NULL;
    }
  
  if (EQ(ppstNode->sasnzTaxName, g_22->g_30->tax->v_5152) && 
      EQ(ppstNode->sasnzTaxRate, g_22->g_30->tax->v_5278) &&
      EQ(ppstNode->sasnzTaxLegalCode, g_22->g_30->tax->v_5279))
    {
      return ppstNode;
    }
      
  return fpstVATSumNode_Find(ppstNode->spstNext, g_22);
}


/********************************************************************************************************************
 *
 * foenVATSumList_Init
 *
 ********************************************************************************************************************
 */

toenBool foenVATSumList_Init(tostVATSumList *ppstList)
{
  ppstList->soiSize = 0;
  ppstList->spstFirst = NULL;

  return TRUE;
}

/********************************************************************************************************************
 *
 * foenVATSumList_Add
 *
 ********************************************************************************************************************
 */

toenBool foenVATSumList_Add(tostVATSumList *ppstList, 
                            struct s_group_22 *g_22)
{
  char *lpsnzTaxName, *lpsnzTaxRate, *lpsnzTaxLegalCode;
  tostVATSumNode *lpstNode, *lpstNext;
  toenBool loenStatus;

  if ((lpstNode = fpstVATSumNode_Find(ppstList->spstFirst, g_22)) == NULL)
    {
      /*
       * No item found - add new one
       */
      
      lpstNext = ppstList->spstFirst;
      if ((ppstList->spstFirst = fpstVATSumNode_New(g_22)) == NULL)
        {
          
          fovdPrintLog(LOG_DEBUG, "foenVATSumList_Add: Can't create new tax category");
          return FALSE;
        }

      ppstList->spstFirst->spstNext = lpstNext;
      ppstList->soiSize++;
    }
  else
    {
      /*
       * Update existing node
       */
      
      if ((loenStatus = foenVATSumNode_Add(lpstNode, g_22)) == FALSE)
        {
          fovdPrintLog(LOG_DEBUG, "foenVATSumList_Add: Can't update tax category");
          return FALSE;
        }
    }

  return TRUE;
}

/********************************************************************************************************************
 *
 * foenVATSumList_Gen
 *
 ********************************************************************************************************************
 */

toenBool foenVATSumList_Gen(tostVATSumList *ppstList)
{
  tostVATSumNode *lpstNode;
  char lasnzItemsNo[16];
  toenBool loenStatus;  
  double loflAmount;
  double loflSumDiscountAmount;
  double loflScale;

  sprintf(lasnzItemsNo, "%d", ppstList->soiSize);
  fovdGen("InvoiceVATSumListStart", lasnzItemsNo, EOL);

  /*
   * Print ifno about each VAT rate
   */

  lpstNode = ppstList->spstFirst;
  while (lpstNode)
    {        
      if ((loenStatus = foenVATSumNode_Gen(lpstNode, 
                                           lpstNode->soflDiscountAmount)) == FALSE)
        {
          return FALSE;
        }
      
      lpstNode = lpstNode->spstNext;
    }
  
  fovdGen("InvoiceVATSumListEnd", EOL);

  return TRUE;
}

/********************************************************************************************************************
 *
 * foenVATSumList_GenDisc
 *
 ********************************************************************************************************************
 */

toenBool foenVATSumList_GenDisc(tostVATSumList *ppstList, 
                                double poflDiscountAmount)
{
  tostVATSumNode *lpstNode;
  char lasnzItemsNo[16];
  toenBool loenStatus;  
  double loflAmount;
  double loflSumDiscountAmount;
  double loflScale;
  char laszDiscAmount[32];
  double loflRound;
  int i;

  fovdPrintLog (LOG_DEBUG, "Assign rounded disc: %lf\n", poflDiscountAmount);

  /*
   * Print info about each VAT rate
   */

  lpstNode = ppstList->spstFirst;
  while (lpstNode)
    {                   
      sprintf(laszDiscAmount, "%lf", (-1.0) * poflDiscountAmount);
      fovdFormatMoney(laszDiscAmount);
      
      /*
        fovdFormatNumber(ppstNode->sasnzTaxRate);
       */
      
      fovdGen("InvoiceVATItem",
              "Rabat",
              lpstNode->sasnzTaxLegalCode, lpstNode->sasnzTaxName, lpstNode->sasnzTaxRate, 
              "1", laszDiscAmount, 
              LOCAL_CURRENCY_CODE, 
              EOL);      
      
      lpstNode = lpstNode->spstNext;
    }
  
  return TRUE;
}

/********************************************************************************************************************
 *
 * foenVATSumList_Print
 *
 ********************************************************************************************************************
 */

toenBool foenVATSumList_Print(tostVATSumList *ppstList)
{
  tostVATSumNode *lpstNode;
  toenBool loenStatus;
  int i = 1;

  printf("VAT rates: %d\n", ppstList->soiSize);
  lpstNode = ppstList->spstFirst;
  while (lpstNode)
    {
      printf("\nVAT item: %d\n", i++);
      if ((loenStatus = foenVATSumNode_Print(lpstNode)) == FALSE)
        {
          return FALSE;
        }
      
      lpstNode = lpstNode->spstNext;
    }

  return TRUE;
}

/********************************************************************************************************************
 *
 * foenVATSumList_Sum
 *
 ********************************************************************************************************************
 */

toenBool foenVATSumList_Sum(tostVATSumList *ppstList, 
                            double *ppflNet, 
                            double *ppflVAT, 
                            double *ppflBrut)
{
  tostVATSumNode *lpstNode;
  toenBool loenStatus;

  fovdPushFunctionName ("foenVATSumList_Sum");

  lpstNode = ppstList->spstFirst;
  while (lpstNode)
    {
      if ((loenStatus = foenVATSumNode_Sum(lpstNode, 
                                           ppflNet, 
                                           ppflVAT, 
                                           ppflBrut)) == FALSE)
        {
          fovdPopFunctionName ();
          return FALSE;
        }
      
      lpstNode = lpstNode->spstNext;
    }

  fovdPopFunctionName ();
  return TRUE;
}

/********************************************************************************************************************
 *
 * foenVATSumList_Round
 *
 ********************************************************************************************************************
 */

toenBool foenVATSumList_Round(tostVATSumList *ppstList, 
                              double poflDiscAmount)
{
  tostVATSumNode *lpstNode;
  toenBool loenStatus;

  lpstNode = ppstList->spstFirst;
  while (lpstNode)
    {
      if ((loenStatus = foenVATSumNode_Round(lpstNode, 
                                             poflDiscAmount)) == FALSE)
        {
          return FALSE;
        }
      
      lpstNode = lpstNode->spstNext;
    }

  return TRUE;
}

/********************************************************************************************************************
 *
 * foenVATSumList_Delete
 *
 ********************************************************************************************************************
 */

toenBool foenVATSumList_Delete(tostVATSumList *ppstList)
{
  tostVATSumNode *lpstNode, *lpstTmp;
  toenBool loenStatus;

  lpstNode = ppstList->spstFirst;
  while (lpstNode)
    {
      lpstTmp = lpstNode->spstNext;
      
      if ((loenStatus = foenVATSumNode_Delete(lpstNode)) == FALSE)
        {
          return FALSE;
        }
      
      lpstNode = lpstTmp;
    }
  
  return TRUE;
}

/********************************************************************************************************************
 *
 * foenVATSumList_Save
 *
 ********************************************************************************************************************
 */

extern int foiVatInv_InsertTax(double poflTaxRate,
                               char pochTaxExempt,
                               double poflNetAmt,                        
                               double poflDisAmt,
                               double poflTaxAmt,
                               double poflTotAmt);

toenBool foenVATSumList_Save(tostVATSumList *ppstList)
{
  int rc = 0;
  tostVATSumNode *lpstNode;
  char lochTaxExemptCode;

  if (goenCustomerIsExempt == TRUE)
    {
      lochTaxExemptCode = 'Y';
    }
  else
    {
      lochTaxExemptCode = 'N';
    }
  
  lpstNode = ppstList->spstFirst;
  while (lpstNode)
    {      
      if ((rc = foiVatInv_InsertTax(lpstNode->soflVATRate,
                                    lochTaxExemptCode,
                                    lpstNode->soflNetAmount,
                                    lpstNode->soflDiscountAmount,
                                    lpstNode->soflVATAmount,
                                    lpstNode->soflBrutAmount)) < 0)
        {
          return FALSE;
        }
           
      lpstNode = lpstNode->spstNext;
    }
  
  return TRUE;
}
