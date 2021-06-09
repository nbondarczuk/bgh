#include <stdlib.h>
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

/*
 * extern variables
 */

extern toenBool goenCustomerIsExempt;

/*
 * extern functions
 */

extern void fovdFormatMoney(char *pachzMoney);

extern double foflMoney_Round(double poflNet, 
                              double *ppflRest);

/*
 * static functions
 */

/*
 * static variables
 */


/********************************************************************************************************************
 *
 * foenService_Load
 *
 ********************************************************************************************************************
 */

toenBool foenService_Load(struct s_group_22 *g_22, 
                          double *ppflNet, 
                          double *ppflTax, 
                          double *ppflBrut,
                          double *ppflDiscount)
{
  struct s_group_23 *g_23;
  struct s_group_30 *g_30;
  struct s_moa_seg *moa;
  struct s_tax_seg *tax;
  int n, i = 0;

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
       * MOA+125 -> NetAmount (exact value)
       */

      if (EQ(moa->v_5025, "125") && EQ(moa->v_4405, "5"))
        {
          n = sscanf(moa->v_5004, "%lf", ppflNet);
          i++;
        }
      
      /*
       * MOA+203 -> BrutAmount (eaxact value)
       */
      
      if (EQ(moa->v_5025, "203") && EQ(moa->v_4405, "5"))
        {
          n = sscanf(moa->v_5004, "%lf", ppflBrut);          
          i++;
        }

      /* 
       * MOA+919 -> Discount (exact value)
       */
      
      if (EQ(moa->v_5025, "919") && EQ(moa->v_4405, "5"))
        {
          n = sscanf(moa->v_5004, "%lf", ppflDiscount);          
          i++;
        }      

      /*
       * Go to next group G23 item
       */

      g_23 = g_23->g_23_next;
    }
  
  /* 3 amounts must be available: MOA+125, MOA+203, MOA+919 */
  ASSERT(i == 3);

  /*
   * Load group G_30 with TAX info
   */
  
  g_30 = g_22->g_30;
  while (g_30)
    {
      tax = g_22->g_30->tax;
      moa = g_22->g_30->moa;
      ASSERT(tax != NULL);
      ASSERT(moa != NULL);
      if (EQ(tax->v_5283, "1") && EQ(tax->v_5153, "VAT"))
        {
          if (EQ(moa->v_5025, "124"))
            {
              n = sscanf(moa->v_5004, "%lf", ppflTax);
              break;
            }
        }
      
      /*
       * Go to next group G30 item
       */
      
      g_30 = g_30->g_30_next;
    }

  ASSERT(g_30 != NULL);

  return TRUE;
}


/********************************************************************************************************************
 *
 * fpstService_New
 *
 * DESCRITION:
 * Alloc memory and fill structure Service with values from G22 item.
 *
 ********************************************************************************************************************
 */

tostService *fpstService_New(struct s_group_22 *g_22)
{
  tostService *lpstServ;
  struct s_group_23 *g_23;
  struct s_group_30 *g_30;
  struct s_moa_seg *moa;
  struct s_tax_seg *tax;
  struct s_qty_seg *qty;
  int n, i = 0, loiCat;
  char lpsnzLabel;
  toenBool loenRound = FALSE;
  double loflBrut = 0.0, loflNet = 0.0, loflRest = 0.0;

  ASSERT(g_22 != NULL);

  /*
   * Get category label
   */

  if ((loiCat = foiServiceCat_Classify(g_22)) == VAT_INV_UNKNOWN_CAT)
    {
      return NULL;
    }

#ifdef _ROUND_MOA_
  if (loiCat == VAT_INV_VPLMN_USAGE_CAT)
    {
      loenRound = TRUE;
    }
#endif

  if ((lpstServ = (tostService *)malloc(sizeof(tostService))) == NULL)
  {
      return NULL;
  }
  lpstServ->soiItems = 0; 
  /*
   * Load group G_30 with TAX info
   */
  
  g_30 = g_22->g_30;
  while (g_30)
    {
      tax = g_22->g_30->tax;
      moa = g_22->g_30->moa;
      ASSERT(tax != NULL);
      ASSERT(moa != NULL);      
      if (EQ(tax->v_5283, "1") && EQ(tax->v_5153, "VAT"))
        {
          strncpy(lpstServ->sasnzTaxName, tax->v_5152, TAX_NAME_SIZE);
          strncpy(lpstServ->sasnzTaxRate, tax->v_5278, TAX_RATE_SIZE);
          strncpy(lpstServ->sasnzTaxLegalCode, tax->v_5279, TAX_LEGAL_CODE_SIZE);
          if (EQ(moa->v_5025, "124"))
            {
              n = sscanf(moa->v_5004, "%lf", &(lpstServ->soflTaxAmount));          
              break;
            }
        }
      
      /*
       * Go to next group G30 item
       */
      
      g_30 = g_30->g_30_next;
    }

  ASSERT(g_30 != NULL);

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

      if (EQ(moa->v_5025, "125") && EQ(moa->v_4405, "5"))
        {
          n = sscanf(moa->v_5004, "%lf", &loflNet);
          if (loenRound == TRUE)
            {
              lpstServ->soflNetAmount = foflMoney_Round(loflNet, &loflRest);
            }
          else
            {
              lpstServ->soflNetAmount = loflNet;
            }
          i++;
        }
      
      /*
       * MOA+203 -> BrutAmount
       */
      
      if (EQ(moa->v_5025, "203") && EQ(moa->v_4405, "5"))
        {
          n = sscanf(moa->v_5004, "%lf", &loflBrut);
          if (loenRound == TRUE)
            {
              lpstServ->soflBrutAmount = foflMoney_Round(loflBrut, &loflRest);
            }
          else
            {
              lpstServ->soflBrutAmount = loflBrut;
            }
          i++;
        }
      
      /*
       * Go to next group G23 item
       */

      g_23 = g_23->g_23_next;
    }
  
  ASSERT(i == 2);

  /*
   * Service item loaded with data from G22
   */      
  
  strcpy(lpstServ->sasnzLabel, gasnzLabel[loiCat]);

  sscanf(lpstServ->sasnzTaxRate, "%lf", &(lpstServ->soflTaxRate));
  
  return lpstServ;
}


/********************************************************************************************************************
 *
 * foenService_Add
 *
 ********************************************************************************************************************
 */

toenBool foenService_Add(tostService *ppstServ, struct s_group_22 *g_22)
{
  struct s_group_23 *g_23;
  struct s_group_30 *g_30;
  struct s_moa_seg *moa;
  struct s_tax_seg *tax;
  struct s_qty_seg *qty;
  int n, i = 0, loiItems, loiCat;
  double loflTax = 0.0;
  double loflNet = 0.0;
  double loflBrut = 0.0;
  toenBool loenRound = FALSE;
  double loflRest = 0.0;  

  ASSERT(g_22 != NULL);
  
  if ((loiCat = foiServiceCat_Classify(g_22)) == VAT_INV_UNKNOWN_CAT)
    {
      return FALSE;
    }

#ifdef _ROUND_MOA_
  if (loiCat == VAT_INV_VPLMN_USAGE_CAT)
    {
      loenRound = TRUE;
    }
#endif

  /*
   * Load group G_30 with TAX info
   */
  
  g_30 = g_22->g_30;
  while (g_30)
    {
      tax = g_22->g_30->tax;
      moa = g_22->g_30->moa;
      ASSERT(tax != NULL);
      ASSERT(moa != NULL);
      if (EQ(tax->v_5283, "1") && EQ(tax->v_5153, "VAT"))
        {
          n = sscanf(moa->v_5004, "%lf", &loflTax);
          ppstServ->soflTaxAmount += loflTax;
          break;
        }
      
      /*
       * Go to next group G30 item
       */
      
      g_30 = g_30->g_30_next;
    }

  ASSERT(g_30 != NULL);
    
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

      if (EQ(moa->v_5025, "125") && EQ(moa->v_4405, "5"))
        {
          n = sscanf(moa->v_5004, "%lf", &loflNet);
          if (loenRound == TRUE)
            {
              ppstServ->soflNetAmount += foflMoney_Round(loflNet, &loflRest);  
            }
          else
            {
              ppstServ->soflNetAmount += loflNet;  
            }
          i++;
        }
      
      /*
       * MOA+203 -> BrutAmount
       */
      
      if (EQ(moa->v_5025, "203") && EQ(moa->v_4405, "5"))
        {
          n = sscanf(moa->v_5004, "%lf", &loflBrut);
          if (loenRound == TRUE)
            {
              ppstServ->soflBrutAmount += foflMoney_Round(loflBrut, &loflRest);  
            }
          else
            {
              ppstServ->soflBrutAmount += loflBrut;  
            }
          i++;
        }
      
      /*
       * Go to next group G23 item
       */

      g_23 = g_23->g_23_next;
    }
  
  ASSERT(i == 2);

  return TRUE;
}

/********************************************************************************************************************
 *
 * fpstService_Find
 *
 * DESCRIPTION:
 * Find service on the list using VAT rate from G22 item. NULL service list means return NULL.
 *
 ********************************************************************************************************************
 */

tostService *fpstService_Find(tostService *ppstServ, struct s_group_22 *g_22)
{
  struct s_tax_seg *tax;

  if (ppstServ == NULL)
    {
      return NULL;
    }

  tax = g_22->g_30->tax;
  if (EQ(ppstServ->sasnzTaxName, tax->v_5152) && 
      EQ(ppstServ->sasnzTaxRate, tax->v_5278) &&
      EQ(ppstServ->sasnzTaxLegalCode, tax->v_5279))
    {
      return ppstServ;
    }
  
  return ppstServ->spstNext;
}

/********************************************************************************************************************
 *
 * foenService_Gen
 *
 ********************************************************************************************************************
 */

toenBool foenService_Gen(tostServiceCat *ppstCat, tostService *ppstServ)
{
  char lasnzCurrency[TMP_BUFFER_SIZE];
  int loiCat;
  char *lpsnzPSMacro;
  toenBool loenStatus;
  static char lasnzNet  [MAX_BUFFER_SIZE];
  static char lasnzItems[MAX_BUFFER_SIZE];
  static char lasnzTaxRate[MAX_BUFFER_SIZE];
  static char lasnzTaxName[MAX_BUFFER_SIZE];

  /*
   * Init
   */

  INIT_STATIC_BUF(lasnzNet);
  INIT_STATIC_BUF(lasnzItems);
  INIT_STATIC_BUF(lasnzTaxRate);
  INIT_STATIC_BUF(lasnzTaxName);
  strncpy(lasnzCurrency, LOCAL_CURRENCY_CODE, TMP_BUFFER_SIZE);

  /*
   * Net value
   */

  sprintf(lasnzNet, "%lf", ppstServ->soflNetAmount);
  fovdFormatMoney(lasnzNet);

  /*
   * Optional numbers of items
   */

  if (ppstServ->soiItems > 0)
    {
      sprintf(lasnzItems, "%d", ppstServ->soiItems);
    }

  /*
   * Tax
   */

  /*
   * Tax exemption
   */

  strcpy(lasnzTaxName, ppstServ->sasnzTaxName);
  if (goenCustomerIsExempt == TRUE && ppstServ->soflBrutAmount == ppstServ->soflNetAmount)
    {
      strcpy(lasnzTaxRate, "ZW");
    }
  else
    {
      strcpy(lasnzTaxRate, ppstServ->sasnzTaxRate);
      /*
      fovdFormatNumber(lasnzTaxRate);
      */
    }

  /*
   * Tax exclusion
   */

  if ((loenStatus = foenServiceCat_IsVATItem(ppstCat)) == FALSE)
    {
      lpsnzPSMacro = "InvoiceNoVATItem";      
      strcpy(lasnzTaxRate, "nie podlega VAT");
    }
  else
    {
      lpsnzPSMacro = "InvoiceVATItem";
    }

  /*
  fovdFormatNumber(lasnzTaxRate);
  */

  fovdGen(lpsnzPSMacro,
          ppstServ->sasnzLabel,
          ppstServ->sasnzTaxLegalCode, lasnzTaxName, lasnzTaxRate, 
          lasnzItems, lasnzNet, 
          lasnzCurrency, 
          EOL);

  fovdPrintLog(LOG_TIMM, "Service: %s\n", ppstServ->sasnzLabel);
  fovdPrintLog(LOG_TIMM, "Value  : %s\n", lasnzNet);
  fovdPrintLog(LOG_TIMM, "Tax    : %s\n", lasnzTaxRate);

  return TRUE;
}

/********************************************************************************************************************
 *
 * foenService_Print
 *
 ********************************************************************************************************************
 */

toenBool foenService_Print(tostService *ppstServ)
{
  return TRUE;
}

/********************************************************************************************************************
 *
 * foenService_Sum
 *
 ********************************************************************************************************************
 */

toenBool foenService_Sum(tostService *ppstServ, double *ppflNet, double *ppflVAT, double *ppflBrut)
{
  fovdPushFunctionName ("foenService_Sum");

  *ppflNet += ppstServ->soflNetAmount;
  *ppflVAT += ppstServ->soflTaxAmount;  
  *ppflBrut += ppstServ->soflBrutAmount;  

  fovdPopFunctionName ();
  return TRUE;
}

/********************************************************************************************************************
 *
 * foenService_Round
 *
 ********************************************************************************************************************
 */

toenBool foenService_Round(tostService *ppstServ, double *ppflRound)
{
  fovdPushFunctionName ("foenService_Round");

  ppstServ->soflNetAmount = foflMoney_Round(ppstServ->soflNetAmount, ppflRound);
  ppstServ->soflTaxAmount = foflMoney_Round(ppstServ->soflTaxAmount, ppflRound);
  ppstServ->soflBrutAmount = foflMoney_Round(ppstServ->soflBrutAmount, ppflRound);
  
  fovdPopFunctionName ();
  return TRUE;
}

/********************************************************************************************************************
 *
 * fpstServiceCat_Init
 *
 ********************************************************************************************************************
 */

toenBool foenServiceCat_Init(tostServiceCat *ppstCat, int poiCat)
{
  ppstCat->soiCat = poiCat;
  ppstCat->soiSize = 0;
  ppstCat->spstFirst = NULL;

  return TRUE;
}

/********************************************************************************************************************
 *
 * foenServiceCat_Delete
 *
 ********************************************************************************************************************
 */

toenBool foenServiceCat_Delete(tostServiceCat *ppstCat)
{
  tostService *lpstServ, *lpstTmp;

  lpstServ = ppstCat->spstFirst;
  while (lpstServ)
    {
      lpstTmp = lpstServ;
      lpstServ = lpstServ->spstNext;
      free(lpstTmp);
    }
  
  return TRUE;
}

/********************************************************************************************************************
 *
 * foenServiceCat_Gen
 *
 ********************************************************************************************************************
 */

toenBool foenServiceCat_Gen(tostServiceCat *ppstCat)
{
  tostService *lpstServ;
  toenBool loenStatus;

  lpstServ = ppstCat->spstFirst;
  while (lpstServ)
    {
      if ((loenStatus = foenService_Gen(ppstCat, lpstServ)) == FALSE)
        {
          return FALSE;
        }

      lpstServ = lpstServ->spstNext;
    }
  
  return TRUE;
}

/********************************************************************************************************************
 *
 * foenServiceCat_Print
 *
 ********************************************************************************************************************
 */

toenBool foenServiceCat_Print(tostServiceCat *ppstCat)
{
  tostService *lpstServ;
  toenBool loenStatus;
  int i = 1;

  printf("Services: %d\n", ppstCat->soiSize);
  
  lpstServ = ppstCat->spstFirst;
  while (lpstServ)
    {
      printf("\nService item: %d\n", i++);
      if ((loenStatus = foenService_Print(lpstServ)) == FALSE)
        {
          return FALSE;
        }

      lpstServ = lpstServ->spstNext;
    }

  return TRUE;
}

/********************************************************************************************************************
 *
 * foenServiceCat_Sum
 *
 ********************************************************************************************************************
 */

toenBool foenServiceCat_Sum(tostServiceCat *ppstCat, 
                            double *ppflNet, 
                            double *ppflVAT, 
                            double *ppflBrut)
{
  tostService *lpstServ;
  toenBool loenStatus;
  
  fovdPushFunctionName ("foenServiceCat_Sum");

  lpstServ = ppstCat->spstFirst;
  while (lpstServ)
    {
      if ((loenStatus = foenService_Sum(lpstServ, 
                                        ppflNet, 
                                        ppflVAT, 
                                        ppflBrut)) == FALSE)
        {
          fovdPopFunctionName ();
          return FALSE;
        }
      
      lpstServ = lpstServ->spstNext;
    }

  fovdPopFunctionName ();
  return TRUE;  
}

/********************************************************************************************************************
 *
 * foenServiceCat_Round
 *
 ********************************************************************************************************************
 */

toenBool foenServiceCat_Round(tostServiceCat *ppstCat, double *ppflRound)
{
  tostService *lpstServ;
  toenBool loenStatus;
  
  fovdPushFunctionName ("foenServiceCat_Sum");

  lpstServ = ppstCat->spstFirst;
  while (lpstServ)
    {      
      if ((loenStatus = foenService_Round(lpstServ, ppflRound)) == FALSE)
        {
          fovdPopFunctionName ();
          return FALSE;
        }
      
      lpstServ = lpstServ->spstNext;
    }

  fovdPopFunctionName ();
  return TRUE;  
}

/********************************************************************************************************************
 *
 * foenServiceCat_IsVATItem
 *
 ********************************************************************************************************************
 */

toenBool foenServiceCat_IsVATItem(tostServiceCat *ppstCat)
{
  /*
   * TAP charges are treated as services with no VAT charged (if M2 billing stratedy used)
   */

  if (ppstCat->soiCat == VAT_INV_VPLMN_USAGE_CAT && ppstCat->soiSize > 0)
    {
      return FALSE;
    }
  
  return TRUE;
}

/********************************************************************************************************************
 *
 * foenServiceCat_IsEmpty
 *
 ********************************************************************************************************************
 */

toenBool foenServiceCat_IsEmpty(tostServiceCat *ppstCat)
{
  if (ppstCat->soiSize == 0)
    {
      return TRUE;
    }
  
  return FALSE;
}

/********************************************************************************************************************
 *
 * foenServiceCat_Add
 * 
 * DESCRIPTION: 
 * Add to the given list of services G22 item. Services list contains services grouped by VAT tax rate.
 * If some rate is not registered or list is empty then add new node to the list or services.
 *
 ********************************************************************************************************************
 */

toenBool foenServiceCat_Add(tostServiceCat *ppstCat, struct s_group_22 *g_22, tostVATSumList *ppstList)
{
  tostService *lpstServ;
  toenBool loenStatus;

  /*
   * Update list of services grouped by Tax Rate
   */

  if ((lpstServ = fpstService_Find(ppstCat->spstFirst, g_22)) == NULL)
    {
      /*
       * No VAT rate matching service found
       */
      
      if ((lpstServ = fpstService_New(g_22)) == NULL)
        {
          fovdPrintLog(LOG_DEBUG, "foenServiceCat_Add: Can't create new service");
          return FALSE;
        }
      
      lpstServ->spstNext = ppstCat->spstFirst;      
      ppstCat->spstFirst = lpstServ;
      ppstCat->soiSize++;      
    }
  else
    {      
      /*
       * Update existing service item returned from method Find
       */
      
      if ((loenStatus = foenService_Add(lpstServ, g_22)) == FALSE)
        {
          fovdPrintLog(LOG_DEBUG, "foenServiceCat_Add: Can't add new service to category service list");
          return FALSE;
        }
    }

  /*
   * Update Tax list with new item but only for VAT categories
   */
  
  if ((loenStatus = foenServiceCat_IsVATItem(ppstCat)) == TRUE)
    {
      if ((loenStatus = foenVATSumList_Add(ppstList, g_22)) == FALSE)
        {
          fovdPrintLog(LOG_DEBUG, "foenServiceCat_Add: Can't add item to tax list");
          return FALSE;
        }
    }

  return TRUE;
}

/********************************************************************************************************************
 *
 * foiServiceCat_Classify
 *
 * DESCRIPTION:
 * Returns index of category casting enum type toenServCat to int 
 *
 ********************************************************************************************************************
 */

#ifdef _PROPOSAL_

int foiServiceCat_Classify(struct s_group_22 *g_22)
{
  if (g_22->lin->v_7140[strlen(g_22->lin->v_7140) - 1] == 'A')
    {
      return VAT_INV_ACCESS_CAT;
    }

  if (g_22->lin->v_7140[strlen(g_22->lin->v_7140) - 1] == 'S')
    {
      return VAT_INV_SUBSCRIPTION_CAT;
    }

  if (g_22->lin->v_7140[strlen(g_22->lin->v_7140) - 1] == 'O')
    {
      return VAT_INV_OCC_CAT;
    }

  if (g_22->lin->v_7140[strlen(g_22->lin->v_7140) - 1] == 'U')
    {
      if (g_22->pia != NULL)
        {
          if (g_22->pia->v_7140[2] == 'm' || g_22->pia->v_7140[2] == 'r')
            {
              return VAT_INV_VPLMN_USAGE_CAT;
            }
          
          if (g_22->pia->v_7140[2] == 'R')
            {
              return VAT_INV_SURCHARGE_USAGE_CAT;
            }
        }

      return VAT_INV_LOCAL_USAGE_CAT;
    }

  return VAT_INV_UNKNOWN_CAT;
}

#else
int foiServiceCat_Classify(struct s_group_22 *g_22)
{
  if (g_22->lin->v_7140[strlen(g_22->lin->v_7140) - 1] == 'A')
    {
      return VAT_INV_ACCESS_CAT;
    }

  if (g_22->lin->v_7140[strlen(g_22->lin->v_7140) - 1] == 'S')
    {
      return VAT_INV_SUBSCRIPTION_CAT;
    }

  if (g_22->lin->v_7140[strlen(g_22->lin->v_7140) - 1] == 'O')
    {
      return VAT_INV_OCC_CAT;
    }

  if (g_22->lin->v_7140[strlen(g_22->lin->v_7140) - 1] == 'U')
    {
      if (g_22->pia != NULL)
        {
          if (g_22->pia->v_7140[0] == 'I')
            {
              /*
              return VAT_INV_LAND_USAGE_CAT;
              */
              return VAT_INV_INTER_USAGE_CAT;
            }

          if (g_22->pia->v_7140[2] == 'm' || g_22->pia->v_7140[2] == 'r')
            {
              return VAT_INV_VPLMN_USAGE_CAT;
            }
          
          if (g_22->pia->v_7140[2] == 'R')
            {
              return VAT_INV_SURCHARGE_USAGE_CAT;
            }
          
          if (g_22->pia->v_7140[0] == 'R')
            {
              return VAT_INV_ROAMING_LEG_USAGE_CAT;
            }
          
          if (g_22->pia->v_7140[strlen(g_22->pia->v_7140) - 2] == 'M' && isdigit(g_22->pia->v_7140[strlen(g_22->pia->v_7140) - 1]))
            {
              return VAT_INV_INTER_USAGE_CAT;
            }
        }

      return VAT_INV_LOCAL_USAGE_CAT;
    }

  return VAT_INV_UNKNOWN_CAT;
}
#endif


