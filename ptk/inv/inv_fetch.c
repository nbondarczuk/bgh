/**************************************************************************/
/*  MODULE : Enclosure Account Fetcher                                    */
/*                                                                        */
/*  AUTHOR : N.Bondarczuk                                                 */
/*                                                                        */
/*  CREATION DATE : 24.09.97                                              */
/*                                                                        */
/*  DESCRIPTION : Exports  function  creating tagged information          */
/*                necessary for creation of invoice                       */
/**************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "bgh.h"
#include "protos.h"
#include "parser.h"
#include "types.h"
#include "gen.h"
#include "inv_types.h"
#include "inv_item.h"
#include "inv_fetch.h"


#if 0	/* just for version.sh */
static char *SCCS_VERSION = "4.1.1";
#endif

#ifdef _DEBUG_INV_FETCH_
#define _DEBUG_
#endif

extern int field2str(char *, int, char *, int);

#define INVOICEE_CODE_STR "IV"
#define SELLER_CODE_STR "II"
#define NIP_CODE_STR "SC"
#define TAXREG_CODE_STR "VA"
#define PERBEG_CODE_STR "167"
#define PEREND_CODE_STR "168"

/*******************************************************
 *                                                    
 * FUNCTION : fetchInvoiceDate                        
 *
 *******************************************************/

#define FIELD_V2005_LEN 3
#define INVOICE_DATE_CODE_STR "3"
#define FIELD_V2380_LEN 35

int fetchInvoiceDate(struct s_TimmInter *inter, char *buffer, int buffer_len) 
{
  int is_found;
  struct s_dtm_seg *dtm;  

  dtm = inter->timm->dtm;  
  is_found = FALSE;
  while (dtm) 
    {
      if (EQ(dtm->v_2005, INVOICE_DATE_CODE_STR)) 
        {
          is_found = TRUE;
          break;
        }
        
      dtm = dtm->dtm_next;
    }

  if (is_found) 
    {
      strncpy(buffer,  dtm->v_2380, buffer_len);
    }
  else
    {
      return FALSE;
    }

  return TRUE;
}



/*******************************************************/
/* FUNCTION : fetchInvoiceNumber                       */
/* ARGUMENTS : IN : inter                              */
/*             OUT : NIP                               */
/*******************************************************/

#define INVOICE_NUMBER_CODE_STR "IV"

int fetchInvoiceNumber(struct s_TimmInter *inter, char *invoice_number_buf, int invoice_number_buf_len) 
{
  int is_found, rc;
  struct s_group_2 *g2;
  struct s_group_3 *g3;
  struct s_nad_seg *nad;
  struct s_rff_seg *rff;

  /*
   * Find g2 block with information about customer
   */

  g2 = inter->timm->g_2;
  is_found = FALSE;
  while (g2) 
    {
      nad = g2->nad;
      if (nad) 
        {
          if (EQ(nad->v_3035, INVOICEE_CODE_STR)) 
            {
              is_found = TRUE;
              break;
            }
        }
      
      g2 = g2->g_2_next;
    }

  if (!is_found)
    {
      return FALSE;
    }

  /*
   * g3 is a subgroup of g2 so we have to find RFF segment with code IV
   */

  g3 = g2->g_3;
  is_found = FALSE;
  while (g3) 
    {
      rff = g3->rff;
      if (rff) 
        {
          if (EQ(rff->v_1153, INVOICE_NUMBER_CODE_STR)) 
            {
              is_found = TRUE;
              break;
            }
        }

      g3 = g3->g_3_next;
    }
  
  /*
   * IT'S DONE !
   */
  
  if (is_found) 
    {
      rff = g3->rff;
      if (rff) 
        {
          strncpy(invoice_number_buf, rff->v_1154, invoice_number_buf_len);
          return TRUE;
        }
      else
        {
          return FALSE;
        }
    }

  return FALSE;
}

/*******************************************************/
/* FUNCTION : fetchInvoiceCustomerName                 */
/* ARGUMENTS : IN : inter                              */
/*             OUT : name                              */
/*******************************************************/

int fetchInvoiceCustomerName(struct s_TimmInter *inter, char *buffer, int buffer_len) 
{
  int is_found, rc;
  char party_type[MAX_BUFFER];
  char customer_name[MAX_BUFFER];
  struct s_group_2 *g2;
  struct s_nad_seg *nad;

  g2 = inter->timm->g_2;
  is_found = FALSE;
  while(g2) 
    {
      nad = g2->nad;
      if (nad) 
        {
          rc =  field2str(nad->v_3035, 3, party_type, MAX_BUFFER);
          if (EQ(party_type, INVOICEE_CODE_STR)) 
            {
              is_found = TRUE;
              break;
            }
        }

      g2 = g2->g_2_next;
    }

  if (is_found) 
    {
      rc = field2str(nad->v_3036, 75, customer_name, MAX_BUFFER);
      strncpy(buffer, customer_name, buffer_len);
    }
    
  return is_found;
}


/*******************************************************/
/* FUNCTION : fetchInvoiceCustomerName                 */
/* ARGUMENTS : IN : inter                              */
/*             OUT : name                              */
/*******************************************************/

int fetchInvoiceTemporaryName(struct s_TimmInter *inter, char *buffer, int buffer_len) 
{
  int is_found, rc;
  char party_type[MAX_BUFFER];
  char customer_name[MAX_BUFFER];
  struct s_group_2 *g2;
  struct s_nad_seg *nad;
  
  g2 = inter->timm->g_2;
  is_found = FALSE;
  while(g2) 
    {
      nad = g2->nad;
      if (nad) 
        {
          if (EQ(nad->v_3035, "IT")) 
            {
              is_found = TRUE;
              break;
            }
        }
      g2 = g2->g_2_next;
    }
  
  if (is_found) 
    {
      strncpy(buffer, nad->v_3036, buffer_len);
    }
    
  return is_found;
}


/*******************************************************/
/* FUNCTION : fetchInvoiceTemporaryAddress             */
/* ARGUMENTS : IN : inter                              */
/*             OUT : address line A                    */
/*             OUT : address line B                    */
/*             OUT : address line C                    */
/*             OUT : address line D                    */
/*******************************************************/

int fetchInvoiceTemporaryAddress(struct s_TimmInter *inter, 
                                 char *bufa, int bufa_len,
                                 char *bufb, int bufb_len,
                                 char *bufc, int bufc_len,
                                 char *bufd, int bufd_len) 
{
  int is_found, i, rc;
  char party_type[MAX_BUFFER];
  char customer_address[MAX_BUFFER];
  struct s_group_2 *g2;
  struct s_nad_seg *nad;
  
  g2 = inter->timm->g_2;
  is_found = FALSE;
  while(g2) 
    {
      nad = g2->nad;
      if (nad) 
        {
          if (EQ(nad->v_3035, "IT")) 
            {
              is_found = TRUE;
              break;
            }
        }

      g2 = g2->g_2_next;
    }
  
  if (is_found) 
    {  
      strcpy(bufa, nad->v_3036a);
      strcpy(bufb, nad->v_3036b);
      strcpy(bufc, nad->v_3036c);
      strcpy(bufd, nad->v_3036d);
    }
  
  return is_found;
}


/*******************************************************/
/* FUNCTION : fetchInvoiceCustomerAddress              */
/* ARGUMENTS : IN : inter                              */
/*             OUT : address line A                    */
/*             OUT : address line B                    */
/*             OUT : address line C                    */
/*             OUT : address line D                    */
/*******************************************************/

int fetchInvoiceCustomerAddress(struct s_TimmInter *inter, 
                                char *bufa, int bufa_len,
                                char *bufb, int bufb_len,
                                char *bufc, int bufc_len,
                                char *bufd, int bufd_len) 
{
  int is_found, i, rc;
  struct s_group_2 *g2;
  struct s_nad_seg *nad;
  
  g2 = inter->timm->g_2;
  is_found = FALSE;
  while(g2) 
    {
      nad = g2->nad;
      if (nad) 
        {
          if (EQ(nad->v_3035, INVOICEE_CODE_STR)) 
            {
              is_found = TRUE;
              break;
            }
        }

      g2 = g2->g_2_next;
    }
  
  if (is_found) 
    {  
      strcpy(bufa, nad->v_3036a);
      strcpy(bufb, nad->v_3036b);
      strcpy(bufc, nad->v_3036c);
      strcpy(bufd, nad->v_3036d);
    }
    
  return is_found;
}


/*******************************************************/
/* FUNCTION : fetchInvoiceCustomerNIP                  */
/* ARGUMENTS : IN : inter                              */
/*             OUT : NIP                               */
/*******************************************************/

int fetchInvoiceCustomerNIP(struct s_TimmInter *inter, char *buffer, int buffer_len) 
{
  int is_found;
  struct s_group_2 *g2;
  struct s_group_3 *g3;
  struct s_nad_seg *nad;
  struct s_rff_seg *rff;

  g2 = inter->timm->g_2;
  is_found = FALSE;
  while(g2) 
    {
      nad = g2->nad;
      if (EQ(nad->v_3035, INVOICEE_CODE_STR)) 
        {
          is_found = TRUE;
          break;
        }
      else
        {
          g2 = g2->g_2_next;
        }
    }

  if (is_found == FALSE)
    {
      return FALSE;
    }

  g3 = g2->g_3;
  while (g3) 
    {
      rff = g3->rff;
      if (EQ(rff->v_1153, NIP_CODE_STR)) 
        {
          is_found = TRUE;
          break;
        }
      
      g3 = g3->g_3_next;      
    }

  if (is_found == TRUE) 
    {
      rff = g3->rff;
      strncpy(buffer, rff->v_1154, buffer_len);
    }
  else
    {
      return FALSE;
    }

  return TRUE;
}


/*******************************************************/
/* FUNCTION : fetchInvoiceCustomerAccountNo            */
/* ARGUMENTS : IN : inter                              */
/*             OUT : account no                        */
/*******************************************************/

#define INVOICE_CODE_STR "IV"

int fetchInvoiceNo(struct s_TimmInter *inter, char *buffer, int buffer_len) 
{
  int is_found;
  struct s_group_2 *g2;
  struct s_group_3 *g3;
  struct s_nad_seg *nad;
  struct s_rff_seg *rff;

  g2 = inter->timm->g_2;
  is_found = FALSE;
  while(g2) 
    {
      nad = g2->nad;
      if (EQ(nad->v_3035, INVOICEE_CODE_STR)) 
        {
          is_found = TRUE;
          break;
        }
      
      g2 = g2->g_2_next;
    }
  
  if (is_found == FALSE)
    {
      return FALSE;
    }
  
  g3 = g2->g_3;
  while (g3) 
    {
      rff = g3->rff;
      if (EQ(rff->v_1153, INVOICE_CODE_STR)) 
        {
          is_found = TRUE;
          break;
        }

      g3 = g3->g_3_next;
    }
  
  if (is_found == TRUE) 
    {
      rff = g3->rff;
      strncpy(buffer, rff->v_1154, buffer_len);
    }
  else
    {
      return FALSE;
    }
  
  return TRUE;
}


/*******************************************************/
/* FUNCTION : fetchInvoiceCustomerAccountNo            */
/* ARGUMENTS : IN : inter                              */
/*             OUT : account no                        */
/*******************************************************/

int fetchInvoiceCustomerAccountNo(struct s_TimmInter *inter, char *buffer, int buffer_len) 
{
  int is_found;
  struct s_group_2 *g2;
  struct s_group_3 *g3;
  struct s_nad_seg *nad;
  struct s_rff_seg *rff;
  
  g2 = inter->timm->g_2;
  is_found = FALSE;
  while(g2) 
    {
      nad = g2->nad;      
      if (EQ(nad->v_3035, INVOICEE_CODE_STR)) 
        {
          is_found = TRUE;
          break;
        }
      
      g2 = g2->g_2_next;
    }
  
  if (is_found == FALSE)
    {
      return FALSE;
    }

  g3 = g2->g_3;
  while (g3) 
    {
      rff = g3->rff;
      if (EQ(rff->v_1153, "IT")) 
        {
          is_found = TRUE;
          break;
        }

      g3 = g3->g_3_next;
    }

  if (is_found == TRUE) 
    {
      rff = g3->rff;
      strncpy(buffer, rff->v_1154, buffer_len);
    }
  else
    {
      return FALSE;
    }

  return TRUE;
}

/*******************************************************/
/* FUNCTION : fetchInvoiceSellerName                   */
/* ARGUMENTS : IN : inter                              */
/*             OUT : name                              */
/*******************************************************/

int fetchInvoiceSellerName(struct s_TimmInter *inter, char *buffer, int buffer_len) 
{
  int is_found;
  char party_type[MAX_BUFFER];
  char seller_name[MAX_BUFFER];
  struct s_group_2 *g2;
  struct s_nad_seg *nad;

  g2 = inter->timm->g_2;
  is_found = FALSE;
  while(g2) 
    {
      nad = g2->nad;
      field2str(nad->v_3035, 3, party_type, MAX_BUFFER);
      if (EQ(party_type, SELLER_CODE_STR)) 
        {
          is_found = TRUE;
          break;
        }

      g2 = g2->g_2_next;
    }

  if (is_found) 
    {
      field2str(nad->v_3036, 75, seller_name, MAX_BUFFER);
      strcpy(buffer, seller_name);
    }
  else
    {
      return FALSE;
    }

  return TRUE;
}


/*******************************************************/
/* FUNCTION : fetchInvoiceSellerAddress                */
/* ARGUMENTS : IN : inter                              */
/*             OUT : address line A                    */
/*             OUT : address line B                    */
/*             OUT : address line C                    */
/*             OUT : address line D                    */
/*******************************************************/

#define SENDER_CODE_STR "II"

int fetchInvoiceSellerAddress(struct s_TimmInter *inter, 
				char *bufa, int bufa_len,
				char *bufb, int bufb_len,
				char *bufc, int bufc_len,
				char *bufd, int bufd_len) 
{
  int is_found, i;
  char party_type[MAX_BUFFER];
  char seller_address[MAX_BUFFER];
  struct s_group_2 *g2;
  struct s_nad_seg *nad;

  g2 = inter->timm->g_2;
  is_found = FALSE;
  while(g2) 
    {
      nad = g2->nad;
      field2str(nad->v_3035, 3, party_type, MAX_BUFFER);
      if (EQ(party_type, SENDER_CODE_STR)) 
        {
          is_found = TRUE;
          break;
        }
      
      g2 = g2->g_2_next;
    }

  if (is_found) 
    {  
      field2str(nad->v_3036a, 75, seller_address, MAX_BUFFER);
      strcpy(bufa, seller_address);
      field2str(nad->v_3036b, 75, seller_address, MAX_BUFFER); 
      strcpy(bufb, seller_address);
      field2str(nad->v_3036c, 75, seller_address, MAX_BUFFER); 
      strcpy(bufc, seller_address);
      field2str(nad->v_3036d, 75, seller_address, MAX_BUFFER); 
      strcpy(bufd, seller_address);
    }
  else
    {
      return FALSE;
    }
    
  return TRUE;
}


/*******************************************************/
/* FUNCTION : fetchInvoiceSellerNIP                    */
/* ARGUMENTS : IN : inter                              */
/*             OUT : NIP                               */
/*******************************************************/

int fetchInvoiceSellerNIP(struct s_TimmInter *inter, char *buffer, int buffer_len) 
{
  int is_found;
  struct s_group_2 *g2;
  struct s_group_3 *g3;
  struct s_nad_seg *nad;
  struct s_rff_seg *rff;

  fovdPushFunctionName("fetchInvoiceSellerNIP");

  g2 = inter->timm->g_2;
  is_found = FALSE;
  while(g2) 
    {
      nad = g2->nad;
      if (EQ(nad->v_3035, SELLER_CODE_STR)) 
        {
          is_found = TRUE;
          break;
        }

      g2 = g2->g_2_next;
    }

  if (!is_found)
    {
      return FALSE;
    }

  g3 = g2->g_3;
  while (g3) 
    {
      rff = g3->rff;      
      if (EQ(rff->v_1153, TAXREG_CODE_STR)) 
        {
          is_found = TRUE;
          break;
        }

      g3 = g3->g_3_next;
    }

  if (is_found) 
    {
      strncpy(buffer, rff->v_1154, buffer_len);
    }

  fovdPopFunctionName();
  return is_found;
}


/*******************************************************/
/* FUNCTION : fetchInvoicePeriodBegin                  */
/* ARGUMENTS : IN : inter                              */
/*             OUT : period beginning                  */
/*******************************************************/

int fetchInvoicePeriodBegin(struct s_TimmInter *inter, char *buffer, int buffer_len) 
{
  int is_found;
  struct s_dtm_seg *dtm;  
  
  dtm = inter->timm->dtm;  
  is_found = FALSE;
  while (dtm) 
    {
      if (EQ(dtm->v_2005, PERBEG_CODE_STR)) 
        {
          is_found = TRUE;
          break;
        }

      dtm = dtm->dtm_next;
    }

  if (is_found) 
    {
      strncpy(buffer, dtm->v_2380, buffer_len);
    }
  
  return is_found;
}

/*******************************************************/
/* FUNCTION : fetchInvoicePeriodEnd                    */
/* ARGUMENTS : IN : inter                              */
/*             OUT : period end                        */
/*******************************************************/

int fetchInvoicePeriodEnd(struct s_TimmInter *inter, char *buffer, int buffer_len) 
{
  int is_found;
  struct s_dtm_seg *dtm;  
  
  dtm = inter->timm->dtm;  
  is_found = FALSE;
  while (dtm) 
    {
      if (EQ(dtm->v_2005, PEREND_CODE_STR)) 
        {
          is_found = TRUE;
          break;
        }

      dtm = dtm->dtm_next;
    }
  
  if (is_found) 
    {
      strncpy(buffer, dtm->v_2380, buffer_len);
    }

  return is_found;
}



/*******************************************************/
/* FUNCTION : fetchInvoiceLetterDate                   */
/* ARGUMENTS : IN : inter                              */
/*             OUT : period end                        */
/*******************************************************/

int fetchInvoiceLetterDate(struct s_TimmInter *inter, char *buffer, int buffer_len) 
{
  int is_found;
  struct s_dtm_seg *dtm;  
  
  fovdPushFunctionName("fetchInvoiceLetterDate");
  
  dtm = inter->timm->dtm;  
  is_found = FALSE;
  while (dtm) 
    {
      if (EQ(dtm->v_2005, "3")) 
        {
          is_found = TRUE;
          break;
        }

      dtm = dtm->dtm_next;
    }
  
  if (is_found) 
    {
      strncpy(buffer, dtm->v_2380, buffer_len);
    }
  
  fovdPopFunctionName();
  return is_found;
}

/*******************************************************/
/* FUNCTION : fetchInvoiceNetValue                     */
/* ARGUMENTS : IN : inter                              */
/*             OUT : netto_value                       */
/*******************************************************/

#define NETTO_INV_AMOUNT_STR "79"
#define ROUNDED_AMOUNT_STR "19"
#define FIELD_V5025_LEN 3
#define FIELD_V4405_LEN 3
#define FIELD_V5004_LEN 18

int fetchInvoiceTotalNetValue(struct s_TimmInter *inter, 
                           char *netto_value_buf, int netto_value_buf_len, char *currency_buf, int currency_buf_len) 
{
  int ok, rc;
  struct s_group_45 *g_45;
  struct s_moa_seg *moa;
  
  g_45 = inter->timm->g_45;
  while (g_45) 
    {
      moa = g_45->moa;
      if (moa) 
        {
          if (EQ(moa->v_5025, NETTO_INV_AMOUNT_STR) && EQ(moa->v_4405, ROUNDED_AMOUNT_STR)) 
            {
              strncpy(netto_value_buf, moa->v_5004, netto_value_buf_len);
              strncpy(currency_buf, moa->v_6345, currency_buf_len);
              
              return TRUE;
            }
        }
      
      g_45 = g_45 -> g_45_next;
    }
  
  return FALSE;
}

/*******************************************************/
/* FUNCTION : fetchInvoiceValue                        */
/* ARGUMENTS : IN : inter                              */
/*             OUT : total_value                       */
/*******************************************************/

int fetchInvoiceTotalValue(struct s_TimmInter *inter, 
                           char *value_buf, int value_buf_len, 
                           char *currency_buf, int currency_buf_len) 
{
  int ok, rc;  
  struct s_group_45 *g_45;
  struct s_moa_seg *moa;

  g_45 = inter->timm->g_45;
  ok = FALSE;
  while (g_45) 
    {
      moa = g_45->moa;
      if (moa) 
        {
          if (EQ(moa->v_5025, "77")) 
            {
              if (moa -> v_5004) 
                {
                  strncpy(value_buf, moa->v_5004, value_buf_len);
                  strncpy(currency_buf, moa->v_6345, currency_buf_len);
                  
                  return TRUE;                  
                }
            }
        }
      
      g_45 = g_45 -> g_45_next;
    }
  
  return ok;
}

/*******************************************************/
/* FUNCTION : fetchInvoiceAmountToPay                  */
/* ARGUMENTS : IN : inter                              */
/*             OUT : amount_to_pay                     */
/*******************************************************/

int fetchInvoiceAmountToPay(struct s_TimmInter *inter, 
                           char *amount_buf, int amount_buf_len, 
                            char *currency_buf, int currency_buf_len) 
{
  int ok, rc;
  struct s_group_45 *g_45;
  struct s_moa_seg *moa;

  g_45 = inter->timm->g_45;
  ok = FALSE;
  while (g_45) 
    {
      moa = g_45->moa;
      if (moa) 
        {
          if (EQ(moa->v_5025, "178") && EQ(moa->v_4405, "4")) 
            {
              if (moa -> v_5004) 
                {
                  strncpy(amount_buf, moa->v_5004, amount_buf_len);
                  strncpy(currency_buf, moa->v_6345, currency_buf_len);
                  ok = TRUE;
                  break;
                }
            }
        }

      g_45 = g_45 -> g_45_next;
    }
  
  return ok;
}

/*******************************************************/
/* FUNCTION : fetchInvoiceDueDate                      */
/* ARGUMENTS : IN : inter                              */
/*             OUT : date                              */
/*******************************************************/

#define DUE_DATE_CODE_STR "13"
#define FIELD_V2005_LEN 3
#define FIELD_V2380_LEN 35

int fetchInvoiceDueDate(struct s_TimmInter *inter, char *buffer, int buffer_len) 
{
  int is_found, rc;
  struct s_group_8 *g_8;
  struct s_dtm_seg *dtm;  
  static char date_type[MAX_BUFFER], date[MAX_BUFFER];

  g_8 = inter->timm->g_8;  
  is_found = FALSE;
  while (g_8) 
    {
      dtm = g_8->dtm;
      if (dtm) 
        {
          if (EQ(dtm->v_2005, DUE_DATE_CODE_STR)) 
            {
              is_found = TRUE;
              break;
            }
        }
      
      g_8 = g_8->g_8_next;
    }
  
  if (is_found) 
    {      
      strncpy(buffer, dtm->v_2380, MAX_BUFFER);
    }
  else
    {
      return FALSE;
    }

  return TRUE;
}


/*******************************************************/
/* FUNCTION : fetchInvoiceBank                         */
/* ARGUMENTS : IN : inter                              */
/*             OUT : bank_name                         */
/*             OUT : account no                        */
/*******************************************************/

#define NETWORK_OPERATOR_STR "RH"
#define FIELD_V3432_LEN 70
#define FIELD_V3192_LEN 40
#define FIELD_V3035_LEN 3

int fetchInvoiceCustomerBank(struct s_TimmInter *inter, 
                             char *bank_name_buf, int bank_name_buf_len,
                             char *account_buf, int account_buf_len) 
{
  int found, rc;
  struct s_group_2 *g_2;
  struct s_fii_seg *fii;
  
  g_2 = inter->timm->g_2;  
  found = FALSE;
  while (g_2 && found == FALSE) 
    {
      fii = g_2->fii;
      while (fii) 
        {
          if (fii->v_3035) 
            {
              if (EQ(fii->v_3035, "AO"))
                {
                  found = TRUE;
                  break;
                }
            }
          
          fii = fii->fii_next;
        }

      g_2 = g_2->g_2_next;
    }
  
  if (found) 
    {
      /*
       * Bank name and bank branch name
       */

      bank_name_buf[0] = '\0';

      if (strlen(fii->v_3432) > 0)
        {
          strcpy(bank_name_buf, fii->v_3432);
          strcat(bank_name_buf, " ");
        }
      
      strncat(bank_name_buf, fii->v_3436, MAX_BUFFER);
      
      /*
       * Account no
       */

      strcpy(account_buf, fii->v_3194); /* Account No */
      strcat(account_buf, fii->v_3192); /* Account Holder Name */
    }
  
  return found;
}


/*******************************************************/
/* FUNCTION : fetchInvoiceBank                         */
/* ARGUMENTS : IN : inter                              */
/*             OUT : bank_name                         */
/*             OUT : account no                        */
/*******************************************************/

#define NETWORK_OPERATOR_STR "RH"
#define FIELD_V3432_LEN 70
#define FIELD_V3192_LEN 40
#define FIELD_V3035_LEN 3

int fetchInvoiceBank(struct s_TimmInter *inter, 
                     char *bank_name_buf, int bank_name_buf_len,
                     char *account_buf, int account_buf_len) 
{
  int found, rc;
  struct s_group_2 *g_2;
  struct s_fii_seg *fii;
  static char type[MAX_BUFFER], bank_name[MAX_BUFFER], account[MAX_BUFFER];
  
  g_2 = inter->timm->g_2;  
  found = FALSE;
  while (g_2) 
    {
      fii = g_2->fii;
      while (fii) 
        {
          if (fii->v_3035) 
            {
              if (EQ(fii->v_3035, NETWORK_OPERATOR_STR))
                {
                  found = TRUE;
                  break;
                }
            }
          
          fii = fii->fii_next;
        }
      
      if (found)
        {
          break;
        }
      else
        {
          g_2 = g_2->g_2_next;
        }
    }
  
  if (found) 
    {
      strncpy(bank_name, fii->v_3432, MAX_BUFFER);
      strncpy(account, fii->v_3194, MAX_BUFFER);
      strcat(account, "-");
      strcat(account, fii->v_3433);

      /*
       * return formatted strings
       */

      strncpy(bank_name_buf, bank_name, bank_name_buf_len);
      strncpy(account_buf, account, account_buf_len);
    }
  
  return found;
}

/*******************************************************/
/* FUNCTION : fetchInvoiceAccessItem                   */
/* ARGUMENTS : IN : inter                              */
/*             OUT : label                             */
/*             OUT : currency                          */
/*             OUT : value                             */
/*******************************************************/

#define FIELD_V1222_LEN 2
#define BLOCK_LEVEL_ONE "01"
#define FIELD_V7140_LEN 35
#define FIELD_V7009_LEN 7
#define SUBSCRIPTION_CODE_STR "S"
#define ACCESS_CODE_STR "A"
#define SERVICE(sc) EQ(sc, "CT")
#define FIELD_V7008_LEN 36
#define FIELD_V5025_LEN 3
#define ITEM_WITH_VAT_STR "203"
#define FIELD_V5004_LEN 18


int fetchInvoiceAccessItem(struct s_TimmInter *inter, 
                           char *buffer_label, int buffer_label_len,
                           char *amount_buf, int amount_buf_len) {
  int rc, is_item_found, is_amount_found;
  struct s_group_22 *g_22;
  struct s_imd_seg *imd;  
  struct s_lin_seg *lin;
  struct s_moa_seg *moa;
  struct s_group_23 *g_23;
  char article_code[MAX_BUFFER], charge_type[MAX_BUFFER], is_service_code[MAX_BUFFER];
  char item_short_description[MAX_BUFFER], level[MAX_BUFFER];
  char amount_id[MAX_BUFFER], amount[MAX_BUFFER];

  g_22 = inter->timm->g_22;    
  while (g_22) {
    is_item_found = FALSE;
    lin = g_22->lin;
    if (lin) {
      rc = field2str(lin->v_1222, FIELD_V1222_LEN, level, MAX_BUFFER);
      if (EQ(level, BLOCK_LEVEL_ONE)) {
        rc = field2str(lin->v_7140, FIELD_V7140_LEN, article_code, MAX_BUFFER);
        rc = getChargeType(article_code, MAX_BUFFER, charge_type, MAX_BUFFER);     
        if (EQ(charge_type, ACCESS_CODE_STR)) {        
          imd = g_22->imd;
          is_item_found = FALSE;
          while (imd) {
            rc = field2str(imd->v_7009, FIELD_V7009_LEN, is_service_code, MAX_BUFFER);
            if (EQ(is_service_code, "SN") && imd->v_7008) {
              rc = field2str(imd->v_7008a, FIELD_V7008_LEN, item_short_description, MAX_BUFFER);
              strcpy(buffer_label, item_short_description);
              is_item_found = TRUE;
              break;
            }
            imd = imd->imd_next;                          
          }
        }
        if (is_item_found)
          break; 
      }
    }

    g_22 = g_22->g_22_next;
  }

  if (is_item_found) {
    is_amount_found = FALSE;
    g_23 = g_22->g_23;
    while (g_23) {
      moa = g_23->moa;
      while (moa) {
        rc = field2str(moa->v_5025, FIELD_V5025_LEN, amount_id, MAX_BUFFER);
        if (EQ(amount_id, ITEM_WITH_VAT_STR)) {
          rc = field2str(moa->v_5004, FIELD_V5004_LEN, amount, MAX_BUFFER);
          strncpy(amount_buf, amount, amount_buf_len);
          is_amount_found = TRUE;
          break;
        }
        moa = moa->moa_next;
      }
      if (is_amount_found)
        break;
      g_23 = g_23->g_23_next;
    }
  }

  return is_item_found;
}

/*
 * article_code ~= <TMDES>.<VSCODE>.<SPDES>.<SNDES>.<Charge Type>
 * charge_type = <Charge Type>
 */

int getChargeType(char *article_code, int artcbuf_len, char *charge_type, int ctbuf_len) {
  int i = 0, pd = 0, ok = TRUE, j, end;

  /*
   * Loop until fourth '.' occurence found
   */

  while (pd < 4 && ok) 
    if (article_code[i] == '.') {
      pd++;
      i++;
    }
    else if (article_code[i] == '\0')
      ok = FALSE;
    else
      i++;
    
  /*
   * RETURN iff (1) EOS found in article_code,
   *            (2) input buffer overflow
   *            (3) not enough space in charge_type buffer
   */

  if (! ok || i == artcbuf_len || i > ctbuf_len)
    return FALSE;

  /*
   * Copy characters from article_code to charge_type
   * until EOS found, or buffer overflow.
   */

  j = 0; end = FALSE;
  while (!end && ok) {
    charge_type[j] = article_code[i];
    if (i > artcbuf_len || j > ctbuf_len)
      ok = FALSE;
    else if (article_code[i] == '\0')
      end = TRUE; 
    else {
      i++;
      j++;
    }
  }

  return TRUE;
}


/*******************************************************/
/* FUNCTION : fetchInvoiceServiceItem                  */
/* ARGUMENTS : IN : inter                              */
/*             OUT : label                             */
/*             OUT : currency                          */
/*             OUT : value                             */
/*******************************************************/

#define SERVICE_CODE_STR "S"

int fetchInvoiceServiceItem(struct s_TimmInter *inter, 
                            char *buffer_label, int buffer_label_len,
                            char *amount_buf, int amount_buf_len) {
  int rc, is_item_found, is_amount_found;
  struct s_group_22 *g_22;
  struct s_imd_seg *imd;  
  struct s_lin_seg *lin;
  struct s_moa_seg *moa;
  struct s_group_23 *g_23;
  char article_code[MAX_BUFFER], charge_type[MAX_BUFFER], is_service_code[MAX_BUFFER];
  char item_short_description[MAX_BUFFER], level[MAX_BUFFER];
  char amount_id[MAX_BUFFER], amount[MAX_BUFFER];

  g_22 = inter->timm->g_22;    
  while (g_22) {
    is_item_found = FALSE;
    lin = g_22->lin;
    if (lin) {
      rc = field2str(lin->v_1222, FIELD_V1222_LEN, level, MAX_BUFFER);
      if (EQ(level, BLOCK_LEVEL_ONE)) {
        rc = field2str(lin->v_7140, FIELD_V7140_LEN, article_code, MAX_BUFFER);
        rc = getChargeType(article_code, MAX_BUFFER, charge_type, MAX_BUFFER);     
        if (EQ(charge_type, SERVICE_CODE_STR)) {        
          imd = g_22->imd;
          is_item_found = FALSE;
          while (imd) {
            rc = field2str(imd->v_7009, FIELD_V7009_LEN, is_service_code, MAX_BUFFER);
            if (EQ(is_service_code, "SN") && imd->v_7008) {
              rc = field2str(imd->v_7008a, FIELD_V7008_LEN, item_short_description, MAX_BUFFER);
              strcpy(buffer_label, item_short_description);
              is_item_found = TRUE;
              break;
            }
            imd = imd->imd_next;                          
          }
        }
        if (is_item_found)
          break; 
      }
    }
    g_22 = g_22->g_22_next;
  }

  if (is_item_found) {
    is_amount_found = FALSE;
    g_23 = g_22->g_23;
    while (g_23) {
      moa = g_23->moa;
      while (moa) {
        rc = field2str(moa->v_5025, FIELD_V5025_LEN, amount_id, MAX_BUFFER);
        if (EQ(amount_id, ITEM_WITH_VAT_STR)) {
          rc = field2str(moa->v_5004, FIELD_V5004_LEN, amount, MAX_BUFFER);
          strncpy(amount_buf, amount, amount_buf_len);
          is_amount_found = TRUE;
          break;
        }
        moa = moa->moa_next;
      }
      if (is_amount_found)
        break;
      g_23 = g_23->g_23_next;
    }
  }

  return is_item_found;
}


/*******************************************************/
/* FUNCTION : fetchInvoiceItemsNumber                  */
/* ARGUMENTS : IN : inter                              */
/*             OUT : items number                      */
/*******************************************************/

#define FIELD_V7140 36

int fetchInvoiceItemsNumber(struct s_TimmInter *inter) 
{
  int i;
  struct s_group_22 *g_22;
  struct s_lin_seg *lin;
  
  fovdPushFunctionName ("fetchInvoiceItemsNumber");
  
  i = 0;
  g_22 = inter->timm->g_22;    
  while (g_22) 
    {
      lin = g_22->lin;
      if (lin) 
        {
          if (lin->v_1222 && EQ(lin->v_1222, "01")) 
            {
              i++;
            }
        }
      
      g_22 = g_22->g_22_next;
    }
  
  fovdPopFunctionName ();
  return i;
}

int fetchInvoiceItemsTypeNumber(struct s_TimmInter *inter, char pochType) 
{
  int i, n;
  char lochType;
  struct s_group_22 *g_22;
  struct s_imd_seg *imd;  
  struct s_lin_seg *lin;
  
  fovdPushFunctionName ("fetchInvoiceItemsNumber");
  
  i = 0;
  g_22 = inter->timm->g_22;    
  while (g_22) 
    {
      lin = g_22->lin;
      if (lin) 
        {
          n = strlen(lin->v_7140);
          if (n > 0)
            {
              lochType = lin->v_7140[n - 1];
              if (lin->v_1222 && EQ(lin->v_1222, "01") && lochType == pochType) 
                {
                  i++;
                }
            }
        }

      g_22 = g_22->g_22_next;
    }
  
  fovdPopFunctionName ();
  return i;
}

/*******************************************************/
/* FUNCTION : fetchInvoiceItem                         */
/* ARGUMENTS : IN : inter                              */
/*             IN : item number                        */
/*             OUT : item type                         */
/*             OUT : item tarif model name             */
/*             OUT : item service package name         */
/*             OUT : item service name                 */
/*             OUT : item brutto value                 */
/*******************************************************/

#define TARIFF_MODEL_CODE_STR "TM"
#define SERVICE_PACKAGE_CODE_STR "SP"
#define SERVICE_NAME_CODE_STR "SN"

#define FIELD_V1082_LEN 6
#define nop {}

int fetchInvoiceItem(struct s_TimmInter *inter,
                     int number,
                     char *item_type_buf, int item_type_buf_len,
                     char *item_tmn_buf, int item_tmn_buf_len,
                     char *item_spn_buf, int item_spn_buf_len,
                     char *item_sn_buf, int item_sn_buf_len,
                     char *brutto_value_buf, int brutto_value_buf_len) {
  int items, amount_found, found, rc, is_item_found, is_amount_found;
  struct s_group_22 *g_22;
  struct s_imd_seg *imd;  
  struct s_lin_seg *lin;
  struct s_moa_seg *moa;
  struct s_group_23 *g_23;
  char level[MAX_BUFFER], item_number[MAX_BUFFER];
  char amount_id[MAX_BUFFER], amount[MAX_BUFFER];
  char article_code[MAX_BUFFER], charge_type[MAX_BUFFER];
  char service_code_id[MAX_BUFFER], item_full_description[MAX_BUFFER];

  found = FALSE;
  g_22 = inter->timm->g_22;    
  while (g_22) {
    is_item_found = FALSE;
    lin = g_22->lin;
    if (lin) {
      rc = field2str(lin->v_1222, FIELD_V1222_LEN, level, MAX_BUFFER);
      if (EQ(level, BLOCK_LEVEL_ONE)) {
        rc = field2str(lin->v_1082, FIELD_V1082_LEN, item_number, MAX_BUFFER);
        if (atoi(item_number) == number) {
          found = TRUE;
          break;
        }
      }
    }
    g_22 = g_22->g_22_next;
  }

  if (found) {
    /*
     * Now we have LIN block with service code : ACCESS, USAGE, SERVICE, OTHER  
     */
    rc = field2str(lin->v_7140, FIELD_V7140_LEN, article_code, MAX_BUFFER);
    rc = getChargeType(article_code, MAX_BUFFER, charge_type, MAX_BUFFER);     
    strncpy(item_type_buf, charge_type, item_type_buf_len);

    /*
     * Find descriptions of Tariff Model, Service Package, Service from sequence 
     * of IMD blocks
     */
    imd = g_22->imd;
    items = 0;
    while (imd && items < 3) {
      rc = field2str(imd->v_7009, FIELD_V7009_LEN, service_code_id, MAX_BUFFER);
      if (imd->v_7008) {
        if (EQ(service_code_id, TARIFF_MODEL_CODE_STR)) {
          rc = field2str(imd->v_7008a, FIELD_V7008_LEN, item_full_description, MAX_BUFFER);          
          strncpy(item_tmn_buf, item_full_description, item_tmn_buf_len);
          items++;
        }
        else if (EQ(service_code_id, SERVICE_PACKAGE_CODE_STR)) {
          rc = field2str(imd->v_7008a, FIELD_V7008_LEN, item_full_description, MAX_BUFFER);          
          strncpy(item_spn_buf, item_full_description, item_spn_buf_len);
          items++;
        }
        else if (EQ(service_code_id, SERVICE_NAME_CODE_STR)) {
          rc = field2str(imd->v_7008a, FIELD_V7008_LEN, item_full_description, MAX_BUFFER);          
          strncpy(item_sn_buf, item_full_description, item_sn_buf_len);
          items++;
        }
        else
          nop;
      }
      imd = imd->imd_next;                          
    }
    
    /*
     * Find amount with VAT in sequence of MOA blocks being a group 23
     */
    amount_found = FALSE;
    g_23 = g_22->g_23;
    while (g_23) {
      moa = g_23->moa;
      while (moa) {
        rc = field2str(moa->v_5025, FIELD_V5025_LEN, amount_id, MAX_BUFFER);
        if (EQ(amount_id, ITEM_WITH_VAT_STR)) {
          rc = field2str(moa->v_5004, FIELD_V5004_LEN, amount, MAX_BUFFER);
          strncpy(brutto_value_buf, amount, brutto_value_buf_len);
          amount_found = TRUE;
          break;
        }
        moa = moa->moa_next;
      }
      if (amount_found)
        break;
      g_23 = g_23->g_23_next;
    }
  }

  return found && items == 3 && amount_found;
}
  

/*******************************************************/
/* FUNCTION : fetchInvoiceItemMarket                   */
/* ARGUMENTS : IN : inter                              */
/*             IN : item number                        */
/*             OUT : item market code                  */
/*             OUT : item marker description           */
/*******************************************************/

#define MARKET_CODE_STR "MRKT"

int fetchInvoiceItemMarket(struct s_TimmInter *inter,
                           int number,
                           char *market_code_buf, int market_code_buf_len,
                           char *market_desc_buf, int market_desc_buf_len) {
  int found, rc;
  struct s_group_22 *g_22;
  struct s_imd_seg *imd;  
  struct s_lin_seg *lin;
  char level[MAX_BUFFER], item_number[MAX_BUFFER];
  char service_code_id[MAX_BUFFER]; 
  char market_code[MAX_BUFFER], market_desc[MAX_BUFFER];
  
  found = FALSE;
  g_22 = inter->timm->g_22;    
  while (g_22) {
    found = FALSE;
    lin = g_22->lin;
    if (lin) {
      rc = field2str(lin->v_1222, FIELD_V1222_LEN, level, MAX_BUFFER);
      if (EQ(level, BLOCK_LEVEL_ONE)) {
        rc = field2str(lin->v_1082, FIELD_V1082_LEN, item_number, MAX_BUFFER);
        if (atoi(item_number) == number) {
          found = TRUE;
          break;
        }
      }
    }
    g_22 = g_22->g_22_next;
  }

  if (found) {
    /*
     * Find descriptions of Tariff Model, Service Package, Service from sequence 
     * of IMD blocks
     */
    imd = g_22->imd;
    found = FALSE;
    while (imd) {
      rc = field2str(imd->v_7009, FIELD_V7009_LEN, service_code_id, MAX_BUFFER);
      if (imd->v_7008) {
        if (EQ(service_code_id, MARKET_CODE_STR)) {
          rc = field2str(imd->v_7008, FIELD_V7008_LEN, market_code, MAX_BUFFER);          
          strncpy(market_code_buf, market_code, market_code_buf_len);

          rc = field2str(imd->v_7008a, FIELD_V7008_LEN, market_desc, MAX_BUFFER);          
          strncpy(market_desc_buf, market_desc, market_desc_buf_len);
          found = TRUE;
          break;
        }
      }
      imd = imd->imd_next;                          
    }
  }
    
  return found;
}


/*******************************************************/
/* FUNCTION : fetchInvoiceItemTariffModel              */
/* ARGUMENTS : IN : inter                              */
/*             IN : item number                        */
/*             OUT : item tariff model code            */
/*             OUT : item tariff model  description    */
/*******************************************************/

int fetchInvoiceItemTariffModel(struct s_TimmInter *inter,
                                int number,
                                char *tm_code_buf, int tm_code_buf_len,
                                char *tm_desc_buf, int tm_desc_buf_len) {
  int found, rc;
  struct s_group_22 *g_22;
  struct s_imd_seg *imd;  
  struct s_lin_seg *lin;
  char level[MAX_BUFFER], item_number[MAX_BUFFER];
  char service_code_id[MAX_BUFFER]; 
  char tm_code[MAX_BUFFER], tm_desc[MAX_BUFFER];
  
  found = FALSE;
  g_22 = inter->timm->g_22;    
  while (g_22) {
    found = FALSE;
    lin = g_22->lin;
    if (lin) {
      rc = field2str(lin->v_1222, FIELD_V1222_LEN, level, MAX_BUFFER);
      if (EQ(level, BLOCK_LEVEL_ONE)) {
        rc = field2str(lin->v_1082, FIELD_V1082_LEN, item_number, MAX_BUFFER);
        if (atoi(item_number) == number) {
          found = TRUE;
          break;
        }
      }
    }
    g_22 = g_22->g_22_next;
  }

  if (found) {
    imd = g_22->imd;
    found = FALSE;
    while (imd) {
      rc = field2str(imd->v_7009, FIELD_V7009_LEN, service_code_id, MAX_BUFFER);
      if (imd->v_7008) {
        if (EQ(service_code_id, TARIFF_MODEL_CODE_STR)) {
          rc = field2str(imd->v_7008, FIELD_V7008_LEN, tm_code, MAX_BUFFER);          
          strncpy(tm_code_buf, tm_code, tm_code_buf_len);

          rc = field2str(imd->v_7008a, FIELD_V7008_LEN, tm_desc, MAX_BUFFER);          
          strncpy(tm_desc_buf, tm_desc, tm_desc_buf_len);
          found = TRUE;
          break;
        }
      }
      imd = imd->imd_next;                          
    }
  }
    
  return found;
}



/*******************************************************/
/* FUNCTION : fetchInvoiceItemServcePackage            */
/* ARGUMENTS : IN : inter                              */
/*             IN : item number                        */
/*             OUT : item service package code         */
/*             OUT : item service package description  */
/*******************************************************/

int fetchInvoiceItemServicePackage(struct s_TimmInter *inter,
                                   int number,
                                   char *sp_code_buf, int sp_code_buf_len,
                                   char *sp_desc_buf, int sp_desc_buf_len) {
  int found, rc;
  struct s_group_22 *g_22;
  struct s_imd_seg *imd;  
  struct s_lin_seg *lin;
  char level[MAX_BUFFER], item_number[MAX_BUFFER];
  char service_code_id[MAX_BUFFER]; 
  char sp_code[MAX_BUFFER], sp_desc[MAX_BUFFER];
  
  found = FALSE;
  g_22 = inter->timm->g_22;    
  while (g_22) {
    found = FALSE;
    lin = g_22->lin;
    if (lin) {
      rc = field2str(lin->v_1222, FIELD_V1222_LEN, level, MAX_BUFFER);
      if (EQ(level, BLOCK_LEVEL_ONE)) {
        rc = field2str(lin->v_1082, FIELD_V1082_LEN, item_number, MAX_BUFFER);
        if (atoi(item_number) == number) {
          found = TRUE;
          break;
        }
      }
    }
    g_22 = g_22->g_22_next;
  }

  if (found) {
    imd = g_22->imd;
    found = FALSE;
    while (imd) {
      rc = field2str(imd->v_7009, FIELD_V7009_LEN, service_code_id, MAX_BUFFER);
      if (imd->v_7008) {
        if (EQ(service_code_id, SERVICE_PACKAGE_CODE_STR)) {
          rc = field2str(imd->v_7008, FIELD_V7008_LEN, sp_code, MAX_BUFFER);          
          strncpy(sp_code_buf, sp_code, sp_code_buf_len);
          rc = field2str(imd->v_7008a, FIELD_V7008_LEN, sp_desc, MAX_BUFFER);          
          strncpy(sp_desc_buf, sp_desc, sp_desc_buf_len);
          found = TRUE;
          break;
        }
      }
      imd = imd->imd_next;                          
    }
  }
    
  return found;
}



/*******************************************************/
/* FUNCTION : fetchInvoiceItemServce                   */
/* ARGUMENTS : IN : inter                              */
/*             IN : item number                        */
/*             OUT : item service code                 */
/*             OUT : item service description          */
/*******************************************************/

int fetchInvoiceItemService(struct s_TimmInter *inter,
                            int number,
                            char *s_code_buf, int s_code_buf_len,
                            char *s_desc_buf, int s_desc_buf_len) {
  int found, rc;
  struct s_group_22 *g_22;
  struct s_imd_seg *imd;  
  struct s_lin_seg *lin;
  char level[MAX_BUFFER], item_number[MAX_BUFFER];
  char service_code_id[MAX_BUFFER]; 
  char s_code[MAX_BUFFER], s_desc[MAX_BUFFER];
  
  found = FALSE;
  g_22 = inter->timm->g_22;    
  while (g_22) {
    found = FALSE;
    lin = g_22->lin;
    if (lin) {
      rc = field2str(lin->v_1222, FIELD_V1222_LEN, level, MAX_BUFFER);
      if (EQ(level, BLOCK_LEVEL_ONE)) {
        rc = field2str(lin->v_1082, FIELD_V1082_LEN, item_number, MAX_BUFFER);
        if (atoi(item_number) == number) {
          found = TRUE;
          break;
        }
      }
    }
    g_22 = g_22->g_22_next;
  }

  if (found) {
    imd = g_22->imd;
    found = FALSE;
    while (imd) {
      rc = field2str(imd->v_7009, FIELD_V7009_LEN, service_code_id, MAX_BUFFER);
      if (imd->v_7008) {
        if (EQ(service_code_id, SERVICE_NAME_CODE_STR)) {
          rc = field2str(imd->v_7008, FIELD_V7008_LEN, s_code, MAX_BUFFER);          
          strncpy(s_code_buf, s_code, s_code_buf_len);

          rc = field2str(imd->v_7008a, FIELD_V7008_LEN, s_desc, MAX_BUFFER);          
          strncpy(s_desc_buf, s_desc, s_desc_buf_len);
          found = TRUE;
          break;
        }
      }
      imd = imd->imd_next;                          
    }
  }
    
  return found;
}


/*******************************************************/
/* FUNCTION : fetchInvoiceItemChargeType               */
/* ARGUMENTS : IN : inter                              */
/*             IN : item number                        */
/*             OUT : item type (USE, ACC, SERV, OTH)   */
/*******************************************************/

#ifndef ENV
#define ENV "fetchInvoiceChargeType"
#endif

extern struct charge_type charge_types_assoc[4];

int fetchInvoiceItemChargeType(struct s_TimmInter *inter, int number) 
{
  int i, items, amount_found, found, rc, is_item_found, is_amount_found;
  struct s_group_22 *g_22;
  struct s_imd_seg *imd;  
  struct s_lin_seg *lin;
  struct s_moa_seg *moa;
  struct s_group_23 *g_23;
  char level[MAX_BUFFER], item_number[MAX_BUFFER], type[MAX_BUFFER];
  char article_code[MAX_BUFFER];
  char *str;
  int ct;

  fovdPushFunctionName ("foenGenInvoiceItemChargeType");

  found = FALSE;
  g_22 = inter->timm->g_22;    
  while (g_22) 
    {
      is_item_found = FALSE;
      lin = g_22->lin;
      if (lin) 
        {
          if (lin->v_1222 && EQ(lin->v_1222, "01")) 
            {
              if (lin->v_1082 && atoi(lin->v_1082) == number) 
                {
                  found = TRUE;
                  break;
                }
            }
        }
      g_22 = g_22->g_22_next;
    }

  if (found) 
    {
      str = lin->v_7140;
      switch(str[strlen(str)-1])
        {
        case 'A': 
          ct = CHARGE_TYPE_ACCESS;
          break;
        case 'S': 
          ct = CHARGE_TYPE_SERVICE;
          break;
        case 'U':
          ct =  CHARGE_TYPE_USAGE;
          break;
        case 'O': 
          ct = CHARGE_TYPE_OTHER;
          break;
        default:
          ct = CHARGE_TYPE_UNKNOWN;
        }
    }
  else
    {
      ct = CHARGE_TYPE_UNKNOWN;
    }
  
  fovdPopFunctionName ();
  return ct;
}

#ifdef ENV
#undef ENV
#endif

/*************************************************************/
/* FUNCTION : fetchInvoiceItemConnectionType                 */
/* ARGUMENTS : IN : inter                                    */
/*             IN : item number                              */
/*             OUT : connection type (AIT, INTER, CF, ROA)   */
/*************************************************************/

#define USAGE_CODE_STR "U"
extern struct connection_type connection_types_assoc[4];
int getConnectionType(char *, int, char *, int);

int fetchInvoiceItemConnectionType(struct s_TimmInter *inter, int number) {
  int i, items, amount_found, found, rc, is_item_found, is_amount_found;
  struct s_group_22 *g_22;
  struct s_imd_seg *imd;  
  struct s_lin_seg *lin;
  struct s_moa_seg *moa;
  struct s_pia_seg *pia;
  char level[MAX_BUFFER], item_number[MAX_BUFFER], type[MAX_BUFFER];
  char article_code[MAX_BUFFER], subarticle_number[MAX_BUFFER];

  found = FALSE;
  g_22 = inter->timm->g_22;    
  while (g_22) {
    lin = g_22->lin;
    if (lin) {
      rc = field2str(lin->v_1222, FIELD_V1222_LEN, level, MAX_BUFFER);
      if (EQ(level, BLOCK_LEVEL_ONE)) {
        rc = field2str(lin->v_1082, FIELD_V1082_LEN, item_number, MAX_BUFFER);
        if (atoi(item_number) == number) {
          found = TRUE;
          break;
        }
      }
    }
    g_22 = g_22->g_22_next;
  }

  /*
   * We have the proper LIN block in group 22
   */
  if (found) {
    rc = field2str(lin->v_7140, FIELD_V7140_LEN, article_code, MAX_BUFFER);
    rc = getChargeType(article_code, MAX_BUFFER, type, MAX_BUFFER);     

    /*
     * Checking PIA block only if LIN block is a USAGE block
     */

    if (EQ(type, USAGE_CODE_STR)) {
      /*
       * PIA is a sequence of blocks
       */

      pia = g_22->pia;
      found = FALSE;
      while (pia) {
        rc = field2str(pia->v_7140, FIELD_V7140, subarticle_number, MAX_BUFFER);
        rc = getConnectionType(subarticle_number, MAX_BUFFER, type, MAX_BUFFER);
        if (isConnectionType(type)) {
          return id2ConnectionType(type);
        }
        else
          pia = pia->pia_next;
      }
    }
    else
      return CONNECTION_TYPE_UNKNOWN;
  }

  return CONNECTION_TYPE_UNKNOWN;
}


int getConnectionType(char *sub_num_buf, int sub_num_buf_len, char *type_buf, int type_buf_len) {
  int i, ok;

  /*
   * Copy characters until first the dot is found or EOS 
   */
  ok = TRUE;
  for (i = 0; i < sub_num_buf_len && ok; i++)
    if (i < type_buf_len) {
      switch (sub_num_buf[i]) {
      case EOS:
        /*
         * Be robust even if found error in the input data.
         */
        type_buf[i] = EOS;
        ok = FALSE;
        break;
      case '.':
        /*
         * We have found EOF(ield) !
         */
        type_buf[i] = EOS;
        return ok;
      default:
        type_buf[i] = sub_num_buf[i];
        break;
      }
    }
    else {
      ok = FALSE;
      break;
    }

  return ok && i < sub_num_buf_len;
}


/*******************************************************/
/* FUNCTION : fetchInvoiceItemUsageType                */
/* ARGUMENTS : IN : inter                              */
/*             IN : item number                        */
/*             OUT : usage type (IN, OUT, NORMAL)      */
/*******************************************************/

int getPPUId(char *, int, char *, int);

int fetchInvoiceItemUsageType(struct s_TimmInter *inter, int number) {
  int i, items, amount_found, found, rc, is_item_found, is_amount_found;
  struct s_group_22 *g_22;
  struct s_imd_seg *imd;  
  struct s_lin_seg *lin;
  struct s_moa_seg *moa;
  struct s_pia_seg *pia;
  char level[MAX_BUFFER], item_number[MAX_BUFFER], type[MAX_BUFFER];
  char article_code[MAX_BUFFER], subarticle_number[MAX_BUFFER];

  found = FALSE;
  g_22 = inter->timm->g_22;    
  while (g_22) {
    lin = g_22->lin;
    if (lin) {
      rc = field2str(lin->v_1222, FIELD_V1222_LEN, level, MAX_BUFFER);
      if (EQ(level, BLOCK_LEVEL_ONE)) {
        rc = field2str(lin->v_1082, FIELD_V1082_LEN, item_number, MAX_BUFFER);
        if (atoi(item_number) == number) {
          found = TRUE;
          break;
        }
      }
    }
    g_22 = g_22->g_22_next;
  }

  /*
   * We have the proper LIN block in group 22
   */
  if (found) {
    rc = field2str(lin->v_7140, FIELD_V7140_LEN, article_code, MAX_BUFFER);
    rc = getChargeType(article_code, MAX_BUFFER, type, MAX_BUFFER);     

    /*
     * Checking PIA block only if LIN block is a USAGE block
     */

    if (EQ(type, USAGE_CODE_STR)) {
      /*
       * PIA is a sequence of blocks
       */

      pia = g_22->pia;
      found = FALSE;
      while (pia) {
        rc = field2str(pia->v_7140, FIELD_V7140, subarticle_number, MAX_BUFFER);
        rc = getPPUId(subarticle_number, MAX_BUFFER, type, MAX_BUFFER);
        if (isUsageType(type)) {
          return id2UsageType(type);
        }
        else
          pia = pia->pia_next;
      }
    }
    else
      return USAGE_TYPE_UNKNOWN;
  }

  return USAGE_TYPE_UNKNOWN;
}

int getPPUId(char *sub_num_buf, int sub_num_buf_len, char *ppuid_buf, int ppuid_buf_len) {
  int i, j, k, ok;

  /*
   * Go through the first part of input string.
   */
  ok = FALSE;
  for (i = 0; i < sub_num_buf_len; i++)
    if (sub_num_buf[i] == DOT) {
      ok = TRUE;
      i++;
      break;
    }
    else if (!isascii(sub_num_buf[i])) {
      ok = FALSE;
      break;
    }
    else
      nop;
  
  /*
   * Copy characters until first the dot is found or EOS 
   * i is now the index of first character of the ppuid
   */
  if (ok)
    for (j = 0; i < sub_num_buf_len && j < ppuid_buf_len; i++, j++)
      if (sub_num_buf[i] == DOT) {
        ppuid_buf[j] = EOS;
        break;
      }
      else if (sub_num_buf[i] == EOS) {
        ppuid_buf[j] = EOS;
        ok = FALSE;
        break;
      }
      else
        ppuid_buf[j] = sub_num_buf[i];
  
  return ok;
}


int fetchInvoiceItemQuantity(struct s_TimmInter *inter, int number, char *qualifier, char *quantity_buf, int quantity_buf_len)
{
  int found;
  struct s_group_22 *g_22;
  struct s_imd_seg *imd;  
  struct s_lin_seg *lin;
  struct s_qty_seg *qty;

  found = FALSE;
  g_22 = inter->timm->g_22;    
  while (g_22) 
    {
      lin = g_22->lin;
      if (lin) 
        {
          if (EQ(lin->v_1222, "01")) 
            {
              if (atoi(lin->v_1082) == number) 
                {
                  found = TRUE;
                  break;
                }
            }
        }
      g_22 = g_22->g_22_next;
    }

  if (found == FALSE)
    {
      return FALSE;
    }

  qty = g_22->qty;
  while (qty)
    {
      if (EQ(qty->v_6411, qualifier))
        {
          strncpy(quantity_buf, qty->v_6060, quantity_buf_len);
          return TRUE;
        }
      qty = qty->qty_next;
    }

  return FALSE;
}



/*******************************************************/
/* FUNCTION : fetchInvoiceItemNettoValue               */
/* ARGUMENTS : IN : inter                              */
/*             IN : item number                        */
/*             OUT : item brutto value                 */
/*******************************************************/

#define ITEM_WITHOUT_VAT_STR "125"

int fetchInvoiceItemNettoValue(struct s_TimmInter *inter,
                               int number,
                               char *brutto_value_buf, int brutto_value_buf_len, char *currency_buf, int currency_buf_len) {
  int items, amount_found, found, rc, is_item_found, is_amount_found;
  struct s_group_22 *g_22;
  struct s_imd_seg *imd;  
  struct s_lin_seg *lin;
  struct s_moa_seg *moa;
  struct s_group_23 *g_23;
  char level[MAX_BUFFER], item_number[MAX_BUFFER];
  char amount_id[MAX_BUFFER], amount[MAX_BUFFER];
  char article_code[MAX_BUFFER], charge_type[MAX_BUFFER];
  char service_code_id[MAX_BUFFER], item_full_description[MAX_BUFFER];

  found = FALSE;
  g_22 = inter->timm->g_22;    
  while (g_22) {
    is_item_found = FALSE;
    lin = g_22->lin;
    if (lin) {
      rc = field2str(lin->v_1222, FIELD_V1222_LEN, level, MAX_BUFFER);
      if (EQ(level, BLOCK_LEVEL_ONE)) {
        rc = field2str(lin->v_1082, FIELD_V1082_LEN, item_number, MAX_BUFFER);
        if (atoi(item_number) == number) {
          found = TRUE;
          break;
        }
      }
    }
    g_22 = g_22->g_22_next;
  }

  if (found) {
    /*
     * Find amount with VAT in sequence of MOA blocks being a group 23
     */
    amount_found = FALSE;
    g_23 = g_22->g_23;
    while (g_23) {
      moa = g_23->moa;
      while (moa) {
        rc = field2str(moa->v_5025, FIELD_V5025_LEN, amount_id, MAX_BUFFER);
        if (EQ(amount_id, ITEM_WITHOUT_VAT_STR)) {
          strncpy(brutto_value_buf, moa->v_5004, brutto_value_buf_len);
          strncpy(currency_buf, moa->v_6345, currency_buf_len);

          amount_found = TRUE;
          break;
        }
        moa = moa->moa_next;
      }
      if (amount_found)
        break;
      g_23 = g_23->g_23_next;
    }
  }

  return found && amount_found;
}


/*******************************************************/
/* FUNCTION : fetchInvoiceItemBruttoValue              */
/* ARGUMENTS : IN : inter                              */
/*             IN : item number                        */
/*             OUT : item brutto value                 */
/*******************************************************/

int fetchInvoiceItemBruttoValue(struct s_TimmInter *inter,
                                int number,
                                char *brutto_value_buf, int brutto_value_buf_len, char *currency_buf, int currency_buf_len) {
  int items, amount_found, found, rc, is_item_found, is_amount_found;
  struct s_group_22 *g_22;
  struct s_imd_seg *imd;  
  struct s_lin_seg *lin;
  struct s_moa_seg *moa;
  struct s_group_23 *g_23;
  char level[MAX_BUFFER], item_number[MAX_BUFFER];
  char amount_id[MAX_BUFFER], amount[MAX_BUFFER];
  char article_code[MAX_BUFFER], charge_type[MAX_BUFFER];
  char service_code_id[MAX_BUFFER], item_full_description[MAX_BUFFER];

  found = FALSE;
  g_22 = inter->timm->g_22;    
  while (g_22) {
    is_item_found = FALSE;
    lin = g_22->lin;
    if (lin) {
      rc = field2str(lin->v_1222, FIELD_V1222_LEN, level, MAX_BUFFER);
      if (EQ(level, BLOCK_LEVEL_ONE)) {
        rc = field2str(lin->v_1082, FIELD_V1082_LEN, item_number, MAX_BUFFER);
        if (atoi(item_number) == number) {
          found = TRUE;
          break;
        }
      }
    }
    g_22 = g_22->g_22_next;
  }

  if (found) {
    /*
     * Find amount with VAT in sequence of MOA blocks being a group 23
     */
    amount_found = FALSE;
    g_23 = g_22->g_23;
    while (g_23) {
      moa = g_23->moa;
      while (moa) {
        rc = field2str(moa->v_5025, FIELD_V5025_LEN, amount_id, MAX_BUFFER);
        if (EQ(amount_id, ITEM_WITH_VAT_STR)) {
          strncpy(brutto_value_buf, moa->v_5004, brutto_value_buf_len);
          strncpy(currency_buf, moa->v_6345, currency_buf_len);
          amount_found = TRUE;
          break;
        }
        moa = moa->moa_next;
      }
      if (amount_found)
        break;
      g_23 = g_23->g_23_next;
    }
  }

  return found && amount_found;
}



/*******************************************************/
/* FUNCTION : fetchInvoiceItemPPUId                    */
/* ARGUMENTS : IN : inter                              */
/*             IN : item number                        */
/*             OUT : ppuid                             */
/*******************************************************/

int getPPUId(char *, int, char *, int);

int fetchInvoiceItemPPUId(struct s_TimmInter *inter, int number, char *ppuid_buf, int ppuid_buf_len) {
  int i, items, amount_found, found, rc, is_item_found, is_amount_found;
  struct s_group_22 *g_22;
  struct s_imd_seg *imd;  
  struct s_lin_seg *lin;
  struct s_moa_seg *moa;
  struct s_pia_seg *pia;
  char level[MAX_BUFFER], item_number[MAX_BUFFER], type[MAX_BUFFER];
  char article_code[MAX_BUFFER], subarticle_number[MAX_BUFFER];

  found = FALSE;
  g_22 = inter->timm->g_22;    
  while (g_22) {
    lin = g_22->lin;
    if (lin) {
      rc = field2str(lin->v_1222, FIELD_V1222_LEN, level, MAX_BUFFER);
      if (EQ(level, BLOCK_LEVEL_ONE)) {
        rc = field2str(lin->v_1082, FIELD_V1082_LEN, item_number, MAX_BUFFER);
        if (atoi(item_number) == number) {
          found = TRUE;
          break;
        }
      }
    }
    g_22 = g_22->g_22_next;
  }

  /*
   * We have the proper LIN block in group 22
   */
  if (found) {
    rc = field2str(lin->v_7140, FIELD_V7140_LEN, article_code, MAX_BUFFER);
    rc = getChargeType(article_code, MAX_BUFFER, type, MAX_BUFFER);     

    /*
     * Checking PIA block only if LIN block is a USAGE block
     */

    if (EQ(type, USAGE_CODE_STR)) {
      /*
       * PIA is a sequence of blocks
       */

      pia = g_22->pia;
      found = FALSE;
      while (pia) {
        rc = field2str(pia->v_7140, FIELD_V7140, subarticle_number, MAX_BUFFER);
        rc = getPPUId(subarticle_number, MAX_BUFFER, type, MAX_BUFFER);
        if (isUsageType(type)) {
          strncpy(ppuid_buf, type, ppuid_buf_len);
          return TRUE;
        }
        else
          pia = pia->pia_next;
      }
    }
    else
      return FALSE;
  }

  return FALSE;
}


/*******************************************************
 * FUNCTION : fetchInvoiceItemTax
 * ARGUMENTS : IN : inter        
 *            IN : item number  
 *            OUT: tax id
 *            OUT: tax rate
 *            OUT: tax_value       
 *******************************************************
 */

int fetchInvoiceItemTax(struct s_TimmInter *inter, int index, 
                        char *tax_id_buf, int tax_id_buf_len,
                        char *tax_rate_buf, int tax_rate_buf_len,
                        char *tax_amount_buf, int tax_amount_buf_len,
                        char *tax_currency_buf, int tax_currency_buf_len) 
{
  int found;
  struct s_group_22 *g_22;
  struct s_group_30 *g_30;
  struct s_lin_seg *lin;
  struct s_moa_seg *moa;
  struct s_tax_seg *tax;
  
  fovdPushFunctionName ("fetchInvoiceItemTax");

  found = FALSE;
  g_22 = inter->timm->g_22;    
  while (g_22) 
    {
      lin = g_22->lin;
      if (lin) 
        {
          if (EQ(lin->v_1222, "01")) 
            {
              if (atoi(lin->v_1082) == index) 
                {
                  found = TRUE;
                  break;
                }
            }
        }
      g_22 = g_22->g_22_next;
    }

  /*
   * We have the proper LIN block in group 22
   */
  if (found == FALSE) 
    {
      fovdPopFunctionName ();
      return FALSE;
    }

  g_30 = g_22->g_30;
  while (g_30) 
    {
      tax = g_30->tax;
      moa = g_30->moa;
      /*
       * individual tax for one item
       */
      if (tax != NULL && moa != NULL && EQ(tax->v_5283, "1") && EQ(tax->v_5153, "VAT") && EQ(moa->v_5025, "124"))
        {
          strncpy(tax_id_buf, tax->v_5152, tax_id_buf_len);
          strncpy(tax_rate_buf, tax->v_5278, tax_rate_buf_len);
          strncpy(tax_amount_buf, moa->v_5004, tax_amount_buf_len);
          strncpy(tax_currency_buf, moa->v_6345, tax_currency_buf_len);

          fovdPopFunctionName ();
          return TRUE;
        }
      g_30 = g_30->g_30_next;
    }

  fovdPopFunctionName ();
  return FALSE;
}


/*******************************************************/
/* FUNCTION : fetchInvoiceItemTariff                   */
/* ARGUMENTS : IN : inter                              */
/*             IN : item number                        */
/*             OUT : tariff time                       */
/*             OUT : tariff zone                       */
/*******************************************************/

int getTariffTime(char *, int, char *, int);
int getTariffZone(char *, int, char *, int);


int fetchInvoiceItemTariff(struct s_TimmInter *inter, int number, 
                           char *ttime_buf, int ttime_buf_len,
                           char *tzone_buf, int tzone_buf_len) {
  int i, items, amount_found, found, rc, is_item_found, is_amount_found;
  struct s_group_22 *g_22;
  struct s_imd_seg *imd;  
  struct s_lin_seg *lin;
  struct s_moa_seg *moa;
  struct s_pia_seg *pia;
  char level[MAX_BUFFER], item_number[MAX_BUFFER], tzone[MAX_BUFFER], ttime[MAX_BUFFER];
  char article_code[MAX_BUFFER], subarticle_number[MAX_BUFFER], type[MAX_BUFFER];

  found = FALSE;
  g_22 = inter->timm->g_22;    
  while (g_22) {
    lin = g_22->lin;
    if (lin) {
      rc = field2str(lin->v_1222, FIELD_V1222_LEN, level, MAX_BUFFER);
      if (EQ(level, BLOCK_LEVEL_ONE)) {
        rc = field2str(lin->v_1082, FIELD_V1082_LEN, item_number, MAX_BUFFER);
        if (atoi(item_number) == number) {
          found = TRUE;
          break;
        }
      }
    }
    g_22 = g_22->g_22_next;
  }

  /*
   * We have the proper LIN block in group 22
   */

  if (found) {
    rc = field2str(lin->v_7140, FIELD_V7140_LEN, article_code, MAX_BUFFER);
    rc = getChargeType(article_code, MAX_BUFFER, type, MAX_BUFFER);     

    /*
     * Checking PIA block only if LIN block is a USAGE block
     */

    if (EQ(type, USAGE_CODE_STR)) {
      /*
       * PIA is a sequence of blocks
       */

      pia = g_22->pia;
      found = FALSE;
      while (pia) {
        rc = field2str(pia->v_7140, FIELD_V7140, subarticle_number, MAX_BUFFER);
        rc = getPPUId(subarticle_number, MAX_BUFFER, type, MAX_BUFFER);
        if (isUsageType(type)) {
          rc = getTariffTime(subarticle_number, MAX_BUFFER, ttime, MAX_BUFFER);
          strncpy(ttime_buf, ttime, ttime_buf_len);
          rc = getTariffZone(subarticle_number, MAX_BUFFER, tzone, MAX_BUFFER);
          strncpy(tzone_buf, tzone, tzone_buf_len);

          return TRUE;
        }
        else
          pia = pia->pia_next;
      }
    }
    else
      return FALSE;
  }

  return FALSE;
}


/*
 * PPUID = <TI>.<PPUID>.<RIVER>.<EGVER>.<TTDES>.<ZNDES>
 */

int getTariffTime(char *ppuid, int ppuid_len, char *ttdes, int ttdes_len) {
  int i = 0, pd = 0, ok = TRUE, j, end;

  /*
   * Loop until fourth '.' occurence found
   */
  while (pd < 4 && ok) 
    if (ppuid[i] == '.') {
      pd++;
      i++;
    }
    else if (ppuid[i] == '\0')
      ok = FALSE;
    else
      i++;
    
  /*
   * RETURN iff (1) EOS found in article_code,
   *            (2) input buffer overflow
   *            (3) not enough space in charge_type buffer
   */
  if (! ok || i == ppuid_len || i > ttdes_len)
    return FALSE;

  /*
   * Copy characters from article_code to charge_type
   * until EOS found, or buffer overflow.
   */
  j = 0; end = FALSE;
  while (!end && ok) {
    ttdes[j] = ppuid[i];
    if (i > ppuid_len || j > ttdes_len)
      ok = FALSE;
    else if (ppuid[i] == '\0')
      end = TRUE; 
    else if (ppuid[i] == DOT) {
      end = TRUE;
      ttdes[j] = '\0';
    }
    else {
      i++;
      j++;
    }
  }

  return TRUE;
}



/*
 * PPUID = <TI>.<PPUID>.<RIVER>.<EGVER>.<TTDES>.<ZNDES>
 */

int getTariffZone(char *ppuid, int ppuid_len, char *ttdes, int ttdes_len) {
  int i = 0, pd = 0, ok = TRUE, j, end;

  /*
   * Loop until fourth '.' occurence found
   */
  while (pd < 5 && ok) 
    if (ppuid[i] == '.') {
      pd++;
      i++;
    }
    else if (ppuid[i] == '\0')
      ok = FALSE;
    else
      i++;
    
  /*
   * RETURN iff (1) EOS found in article_code,
   *            (2) input buffer overflow
   *            (3) not enough space in charge_type buffer
   */
  if (! ok || i == ppuid_len || i > ttdes_len)
    return FALSE;

  /*
   * Copy characters from article_code to charge_type
   * until EOS found, or buffer overflow.
   */
  j = 0; end = FALSE;
  while (!end && ok) {
    ttdes[j] = ppuid[i];
    if (i > ppuid_len || j > ttdes_len)
      ok = FALSE;
    else if (ppuid[i] == '\0')
      end = TRUE; 
    else {
      i++;
      j++;
    }
  }

  return TRUE;
}



/*******************************************************/
/* FUNCTION : fetchInvoiceItemAccessType               */
/* ARGUMENTS : IN : inter                              */
/*             IN : item number                        */
/*             OUT : access type                       */
/*             OUT : access subtype                    */
/*******************************************************/

int getAccesType(char *, int, char *, int);
int getAccesSubtype(char *, int, char *, int);

int fetchInvoiceItemAccessType(struct s_TimmInter *inter, int number, 
                               char *access_type_buf, int access_type_buf_len,
                               char *access_subtype_buf, int access_subtype_buf_len) {
  int i, items, amount_found, found, rc, is_item_found, is_amount_found;
  struct s_group_22 *g_22;
  struct s_imd_seg *imd;  
  struct s_lin_seg *lin;
  struct s_moa_seg *moa;
  struct s_pia_seg *pia;
  char level[MAX_BUFFER], item_number[MAX_BUFFER], tzone[MAX_BUFFER], ttime[MAX_BUFFER];
  char article_code[MAX_BUFFER], subarticle_number[MAX_BUFFER], type[MAX_BUFFER];
  char access_type[MAX_BUFFER], access_subtype[MAX_BUFFER];

  found = FALSE;
  g_22 = inter->timm->g_22;    
  while (g_22) {
    lin = g_22->lin;
    if (lin) {
      rc = field2str(lin->v_1222, FIELD_V1222_LEN, level, MAX_BUFFER);
      if (EQ(level, BLOCK_LEVEL_ONE)) {
        rc = field2str(lin->v_1082, FIELD_V1082_LEN, item_number, MAX_BUFFER);
        if (atoi(item_number) == number) {
          found = TRUE;
          break;
        }
      }
    }
    g_22 = g_22->g_22_next;
  }

  /*
   * We have the proper LIN block in group 22
   */
  if (found) {
    rc = field2str(lin->v_7140, FIELD_V7140_LEN, article_code, MAX_BUFFER);
    rc = getChargeType(article_code, MAX_BUFFER, type, MAX_BUFFER);     

    /*
     * Checking PIA block only if LIN block is a USAGE block
     */
    if (EQ(type, ACCESS_CODE_STR)) {
      /*
       * PIA is a sequence of blocks
       */

      pia = g_22->pia;
      found = FALSE;
      while (pia) {
        rc = field2str(pia->v_7140, FIELD_V7140, subarticle_number, MAX_BUFFER);
        rc = getPPUId(subarticle_number, MAX_BUFFER, type, MAX_BUFFER);

        if (isAccessType(type)) {
          rc = getAccessType(subarticle_number, MAX_BUFFER, access_type, MAX_BUFFER);
          strncpy(access_type_buf, access_type, access_type_buf_len);
          rc = getAccessSubtype(subarticle_number, MAX_BUFFER, access_subtype, MAX_BUFFER);
          strncpy(access_subtype_buf, access_subtype, access_subtype_buf_len);

          return TRUE;
        }
        else
          pia = pia->pia_next;
      }
    }
    else
      return FALSE;
  }

  return FALSE;
}



/*
 * 
 */

int getAccessType(char *ppuid, int ppuid_len, char *type, int type_len) {
  int i = 0, pd = 0, ok = TRUE, j, end;

  /*
   * Copy characters from article_code to charge_type
   * until EOS found, or buffer overflow.
   */
  i = 0; j = 0; end = FALSE;
  while (!end && ok) {
    type[j] = ppuid[i];
    if (i > ppuid_len || j > type_len)
      ok = FALSE;
    else if (ppuid[i] == '\0')
      end = TRUE; 
    else if (ppuid[i] == DOT) {
      end = TRUE;
      type[j] = '\0';
    }   
    else {
      i++;
      j++;
    }
  }
  
  return ok;
}

/*
 *
 */

int getAccessSubtype(char *ppuid, int ppuid_len, char *subtype, int subtype_len) 
{
  int i = 0, pd = 0, ok = TRUE, j, end;

  /*
   * Loop until first '.' occurence found
   */
  while (pd < 1 && ok) 
    {
      if (ppuid[i] == '.') 
        {
          pd++;
          i++;
        }
      else if (ppuid[i] == '\0')
        {
          ok = FALSE;
        }
      else
        {
          i++;
        }
    }

  /*
   * RETURN iff (1) EOS found in article_code,
   *            (2) input buffer overflow
   *            (3) not enough space in charge_type buffer
   */
  if (! ok || i == ppuid_len || i > subtype_len)
    {
      return FALSE;
    }

  /*
   * Copy characters from article_code to charge_type
   * until EOS found, or buffer overflow.
   */
  j = 0; end = FALSE;
  while (!end && ok) 
    {
      subtype[j] = ppuid[i];
      if (i > ppuid_len || j > subtype_len)
        {
          ok = FALSE;
        }
      else if (ppuid[i] == '\0')
        {
          end = TRUE; 
        }
      else if (ppuid[i] == DOT) 
        {        
          end = TRUE;
          subtype[j] = '\0';
        }
      else {
        i++;
        j++;
      }
    }

  return ok;
}

/*******************************************************/
/* FUNCTION : fetchInvoiceTotalTax                     */
/* ARGUMENTS : IN : inter                              */
/*             OUT : tax value                         */
/*******************************************************/

#define FIELD_V5283_LEN 3
#define FIELD_V5278_LEN 17
#define TOTAL_EACH_TAX_STR "3" 
#define TOTAL_VAT_STR "124"

int fetchInvoiceTotalTax(struct s_TimmInter *inter, char *tax_amount_buf, int tax_amount_buf_len, char *currency_buf, int currency_buf_len) 
{
  int found, rc;
  struct s_group_47 *g_47;
  struct s_tax_seg *tax;
  struct s_moa_seg *moa;

  g_47 = inter->timm->g_47;
  found = FALSE;
  while (g_47) 
    {
      tax = g_47->tax;
      if (tax) 
        {
          if (EQ(tax->v_5283, TOTAL_EACH_TAX_STR))
            {
              found = TRUE;
            }
        }
      
      if (found) 
        {
          found = FALSE;
          moa = g_47->moa;
          if (moa) 
            {
              if (EQ(moa->v_5025, TOTAL_VAT_STR) && moa -> v_5004)
                {
                  strncpy(tax_amount_buf, moa->v_5004, tax_amount_buf_len);
                  strncpy(currency_buf, moa->v_6345, currency_buf_len);                  
                  found = TRUE;
                  break;
                }
            }
        }

      g_47 = g_47 -> g_47_next;
    }
  
  return found;
}


int countInvoiceMarketingTexts(struct s_TimmInter *ti)
{
  struct s_ftx_seg *ftx;
  int loiText;

  loiText = 0;
  ftx = ti->timm->ftx;
  while (ftx)
    {
      loiText++;
      fovdPrintLog (LOG_DEBUG, "Marketing Text no %d\n", loiText);
      ftx = ftx->ftx_next;
    }

  fovdPrintLog (LOG_DEBUG, "Marketing Texts no %d\n", loiText);

  return loiText;
}


int fetchInvoiceMarketingText(struct s_TimmInter *ti, int poiTextNo, char pachzLine[6][MAX_BUFFER])
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
              strncpy(pachzLine[5], ftx->v_4440e, MAX_BUFFER);
              fovdPrintLog (LOG_DEBUG, "Text no %d is : %s", loiText, pachzLine[0]);
              return TRUE;
            }
          else
            {
              pachzLine[0][0] = '\0'; 
              pachzLine[1][0] = '\0'; 
              pachzLine[2][0] = '\0'; 
              pachzLine[3][0] = '\0'; 
              pachzLine[4][0] = '\0'; 
              pachzLine[5][0] = '\0'; 
            }
          break;
        }
      loiText++;
      ftx = ftx->ftx_next;
    }
  
  return FALSE;
}


int fetchInvoiceTaxExempt(struct s_TimmInter *ti, char *pachzTermLen, int poiTermLen)
{
  int is_found, rc;
  struct s_group_2 *g2;
  struct s_group_3 *g3;
  struct s_nad_seg *nad;
  struct s_rff_seg *rff;

  /*
   * Find g2 block with information about customer
   */

  g2 = ti->timm->g_2;
  is_found = FALSE;
  while(g2) 
    {
      nad = g2->nad;
      if (nad) 
        {
          /*
           * This is invoicee info block
           */

          if (EQ(nad->v_3035, "IV")) 
            {
              is_found = TRUE;
              break;
            }
        }

      g2 = g2->g_2_next;
    }

  if (!is_found)
    {
      return FALSE;
    }

  /*
   * g3 is a subgroup of g2 so we have to find RFF segment with code GC
   */

  g3 = g2->g_3;
  while (g3) 
    {
      rff = g3->rff;
      if (rff) 
        {
          /*
           * Referential information 
           */

          if (EQ(rff->v_1153, "GC")) 
            {
              strncpy(pachzTermLen, rff->v_1154, poiTermLen);
              return TRUE;
            }
        }

      g3 = g3->g_3_next;
    }
  
  return FALSE;
}

/**************************************************************************************
 *                                                    
 * FUNCTION : foenFetchInvoiceItem                        
 *
 * DESCRIPTION : For a given item no loading full information about item on a main 
 *               page of invoice. It fills fields of structure tostInvItem with strings
 *               assuming that it is initialized with zero values. Processed are all 
 *               fields of G22.
 *
 **************************************************************************************/

extern struct s_imd_seg *lpstFindItemDescription(struct s_imd_seg *, char *);
extern char *lpchzGetField(int, char *);
extern struct s_moa_seg *lpstFindPaymentSegment(struct s_group_23 *, char *); 
extern struct s_qty_seg *lpstFindQuantity(struct s_qty_seg *, char *, char *); 

int foiFieldsNo(char *);

static char szTemp[128];

toenBool foenFetchInvoiceItem(struct s_TimmInter *ppstTimmInter, int poiItemNo, tostInvItem *ppstInvItem)
{
  struct s_group_22 *lpstG22;
  struct s_imd_seg *lpstImd;  
  struct s_lin_seg *lpstLin;
  struct s_moa_seg *lpstMoa;
  struct s_qty_seg *lpstQty;
  struct s_tax_seg *lpstTax;
  struct s_pri_seg *lpstPri;
  struct s_group_23 *lpstG23;
  struct s_group_25 *lpstG25;
  struct s_group_30 *lpstG30;
  toenBool loenFound;
  int i;
  char *ptr;

  /*
   * Looking for a item with no eq poiItemNo
   */

  lpstG22 = ppstTimmInter->timm->g_22;    
  loenFound = FALSE;
  i = 0;
  while (lpstG22) 
    {      
      lpstLin = lpstG22->lin;
      if (poiItemNo == i++)
        {
          loenFound = TRUE;
          break;
        }

      lpstG22 = lpstG22->g_22_next;
    }

  if (loenFound == FALSE)
    {
      sprintf (szTemp, "foenFetchInvoiceItem: No item found\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      return FALSE;
    }

  /*
   * Item in G22 is found 
   */

  /* 
     lostItem.sasnzType - Access, Usage, Subscription, OCC 
     from G22.LIN.v_7140.[strlen(G22.LIN.v_7140)]
  */

  ppstInvItem->sasnzType[0] = lpstG22->lin->v_7140[strlen(lpstG22->lin->v_7140) - 1];
  ppstInvItem->sasnzType[1] = '\0';

  if (ppstInvItem->sasnzType[0] != 'O')
    {
      /* 
         lostItem.sasnzTM - Tariff Modell 
         from G22.IMD[i].v_7008a 
         where G22.IMD.v_7009 = 'TM'
      */
  
      lpstImd = lpstFindItemDescription(lpstG22->imd, "TM");
      if (lpstImd == NULL)
        {
          sprintf (szTemp, "foenFetchInvoiceItem: Can't find TM for Item\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          return FALSE;
        }
      else
        {
          strcpy(ppstInvItem->sasnzTM, lpstImd->v_7008);
          strcat(ppstInvItem->sasnzTM, " v.");
          strcat(ppstInvItem->sasnzTM, lpchzGetField(1, lpstG22->lin->v_7140));
          if (ppstInvItem->sasnzType[0] == 'U')
            {
              if (foiFieldsNo(lpstG22->pia->v_7140) >= 3)
                {
                  strcat(ppstInvItem->sasnzTM, lpchzGetField(2, lpstG22->pia->v_7140));              
                  if (foiFieldsNo(lpstG22->pia->v_7140) >= 4)
                    {
                      strcat(ppstInvItem->sasnzTM, lpchzGetField(3, lpstG22->pia->v_7140));
                    }
                }
            }          
        }

      /* 
         lostItem.sasnzSN - Service Name 
         from G22.IMD[i].v_7008a 
         where G22.IMD.v_7009 = 'SN'
      */

      lpstImd = lpstFindItemDescription(lpstG22->imd, "SN");
      if (lpstImd == NULL)
        {
          sprintf (szTemp, "foenFetchInvoiceItem: Can't find SN for Item\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          return FALSE;
        }
      else
        {
          strcpy(ppstInvItem->sasnzSN, lpstImd->v_7008a);
        }      
    }

  

  switch(ppstInvItem->sasnzType[0])
    {
    case 'U':
      
      /* 
         lostItem.sasnzUsageType - Air, Interconnect, CF, Roaming, MOC, MTC 
         from G22.PIA[0].v_7140.0 
      */

      switch(lpstG22->pia->v_7140[2])
        {
          /*
           * TA MOC callrecords
           */

        case 'R':
        case 'r':
          strcpy(ppstInvItem->sasnzUsageType, "MOC");
          break;

          /*
           * TA MTC callrecords
           */

        case 'm':
          strcpy(ppstInvItem->sasnzUsageType, "MTC");
          break;

          /*
           * Normal usage
           */

        default:
          ppstInvItem->sasnzUsageType[0] = lpstG22->pia->v_7140[0];
          ppstInvItem->sasnzUsageType[1] = '\0';

        }


      /* 
         lostItem.sasnzTT - Tariff Time if Usage 
         from G22.PIA[0].v_7140.4 
         where G22.LIN.v_7140.[strlen(G22.LIN.v_7140)] = 'U' 
      */
      
      strcpy(ppstInvItem->sasnzTT, lpchzGetField(4, lpstG22->pia->v_7140));

      /* 
         lostItem.sasnzTZ - Tariff Zone 
         from G22.PIA[0].v_7140.5 
         where G22.LIN.v_7140.[strlen(G22.LIN.v_7140)] = 'U' 
      */

      strcpy(ppstInvItem->sasnzTZ, lpchzGetField(5, lpstG22->pia->v_7140));

      /* 
         lostItem.sasnzVPLMN - VPLMN name if Usage Roaming 
         from G22.IMD[i].v_7008a 
         where G22.IMD.v_7009 = 'VPLMN' 
         and G22.LIN.v_7140.[strlen(G22.LIN.v_7140)] = 'U'
      */

      switch(lpstG22->pia->v_7140[2])
        {
          /*
           * MOC or MTC callrecords
           */

        case 'm':
        case 'R':
        case 'r':
          strcpy(ppstInvItem->sasnzVPLMN, lpchzGetField(2, lpstG22->pia->v_7140));

        }

      break;

    case 'A':
    case 'S':

      /* 
         lostItem.sasnzQty - Quantity if Access or Sub
         from G22.QTY[i].v_6060 
         where G22.QTY.v_6063 = '109' 
      */

      lpstQty = lpstFindQuantity(lpstG22->qty, "107", "UNI"); 
      if (lpstQty == NULL)
        {
          sprintf (szTemp, "foenFetchInvoiceItem: Can't find QTY.107.UNI for Access Item\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          return FALSE;
        }
      else
        {
          strcpy(ppstInvItem->sasnzQuantity, lpstQty->v_6060);
        }

      /*
       * PRI
       */
     
      lpstG25 = lpstG22->g_25;
      while (lpstG25)
        {
          lpstPri = lpstG25->pri;
          if (lpstPri == NULL)
            {
              return FALSE;
            }
      
          if (EQ(lpstPri->v_5125, "CAL") && EQ(lpstPri->v_5375, "INV"))
            {
              strcpy(ppstInvItem->sasnzPrice, lpstPri->v_5118);
              break;
            }
          
          lpstG25 = lpstG25->g_25_next;
        }

      break;

    case 'O':

      /* 
         lostItem.sasnzFE - Fee description if OCC 
         from G22.IMD[i].v_7008a 
         where G22.IMD.v_7009 = 'FE'
      */

      lpstImd = lpstFindItemDescription(lpstG22->imd, "FE");
      if (lpstImd == NULL)
        {
          sprintf (szTemp, "foenFetchInvoiceItem: Can't find FE for ACC Item\n");
          macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
          return FALSE;
        }
      else
        {
          strcpy(ppstInvItem->sasnzFE, lpstImd->v_7008a);
        }

      break;

    default:
      sprintf (szTemp, "foenFetchInvoiceItem: Unknown type of G22 Item\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      return FALSE;

    }
  
  if (ppstInvItem->sasnzType[0] == 'A')
    {
      if (strlen(lpstG22->pia->v_7140) >= 1)
        {
          ppstInvItem->sasnzAccessType[0] = lpstG22->pia->v_7140[0];
          ppstInvItem->sasnzAccessType[1] = '\0';
        }
      
      if (strlen(lpstG22->pia->v_7140) >= 3)
        {
          ppstInvItem->sasnzAccessSubtype[0] = lpstG22->pia->v_7140[2];      
          ppstInvItem->sasnzAccessSubtype[1] = '\0';
        }
    }

  /* 
     lostItem.sasnzNetAmount - Item net amount 
     G22.G23[i].MOA[j].v_5004 
     where G22.G23[i].MOA[j].v_5025 = '125' 
  */ 

  lpstMoa = lpstFindPaymentSegment(lpstG22->g_23, "125"); 
  if (lpstMoa == NULL)
    {
      sprintf (szTemp, "foenFetchInvoiceItem: Can't find MOA:125 for Item\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      return FALSE;
    }
  else
    {
      strcpy(ppstInvItem->sasnzNetAmount, lpstMoa->v_5004);
    }

  /* 
     lostItem.sasnzBrutAmount - Item brut amount 
     G22.G23[i].MOA[j].v_5004 
     where G22.G23[i].MOA[j].v_5025 = '203' 
  */          

  lpstMoa = lpstFindPaymentSegment(lpstG22->g_23, "203"); 
  if (lpstMoa == NULL)
    {
      sprintf (szTemp, "foenFetchInvoiceItem: Can't find MOA:203 for Item\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);
      return FALSE;
    }
  else
    {
      strcpy(ppstInvItem->sasnzBrutAmount, lpstMoa->v_5004);
    }

  /* 
     lostItem.sasnzTaxName - Item tax name
     G22.G30[i].TAX.v_5152
     where G22.G30[i].TAX.v_5283 = '1'
  */

  /* 
     BETTER : lostItem.sasnzTaxName - Tax rate
     G22.G30[i].TAX.v_5278
     where G22.G30[i].TAX.v_5283 = '1'
  */
  
  lpstG30 = lpstG22->g_30;

  lpstTax = lpstG30->tax;
  strcpy(ppstInvItem->sasnzTaxName, lpstTax->v_5278);
  
  /* 
     lostItem.sasnzTaxAmount - Item tax amount 
     G22.G30[i].MOA.v_5004 
     where G22.G30[i].TAX.v_5283 = '1'
  */

  lpstMoa = lpstG30->moa;
  strcpy(ppstInvItem->sasnzTaxAmount, lpstMoa->v_5004);
    
  return TRUE;
}

int foiFieldsNo(char *str)
{
  int n, i, j;

  n = strlen(str);
  for (i = 0, j = 0; i < n; i++)
    {
      if (str[i] == '.')
        {
          j++;
        }
    }

  return j;
}

/*****************************************************************************************************************
 *
 * FUNCTION: foenInvFetch_CustomerGroup
 *
 * DESCRIPTION: CR42 - loading info about customer's group
 *
 *****************************************************************************************************************
 */

#define CUSTOMER_GROUP_CODE_STR "PG"

toenBool foenInvFetch_CustomerGroup(struct s_TimmInter *ppstTimm, char *pasnzGroup, int poiBufferSize)
{
  struct s_group_2 *g2;
  struct s_group_3 *g3;
  struct s_nad_seg *nad;
  struct s_rff_seg *rff;
  toenBool loenIsFound;

  g2 = ppstTimm->timm->g_2;
  loenIsFound = FALSE;
  while (g2) 
    {
      nad = g2->nad;
      if (EQ(nad->v_3035, INVOICEE_CODE_STR)) 
        {
          loenIsFound = TRUE;
          break;
        }
      else
        {
          g2 = g2->g_2_next;
        }
    }

  if (loenIsFound == FALSE)
    {
      return FALSE;
    }
  
  g3 = g2->g_3;
  while (g3) 
    {
      rff = g3->rff;
      if (EQ(rff->v_1153, CUSTOMER_GROUP_CODE_STR)) 
        {
          loenIsFound = TRUE;
          break;
        }
      else
        {
          g3 = g3->g_3_next;
        }
    }
  
  if (loenIsFound == TRUE) 
    {
      rff = g3->rff;
      strncpy(pasnzGroup, rff->v_1154, poiBufferSize);
    }
  else
    {
      return FALSE;
    }

  return TRUE;
}


/*****************************************************************************************************************
 *
 * FUNCTION: foenInvFetch_PaymentType
 *
 * DESCRIPTION: CR42 - loading info about customer's payment type
 *
 *****************************************************************************************************************
 */

#define PAYMENT_TYPE_CODE_STR "PM"

toenBool foenInvFetch_PaymentType(struct s_TimmInter *ppstTimm, char *pasnzType, int poiTypeBufLen)
{
  struct s_group_2 *g2;
  struct s_group_3 *g3;
  struct s_nad_seg *nad;
  struct s_rff_seg *rff;
  toenBool loenIsFound;

  g2 = ppstTimm->timm->g_2;
  loenIsFound = FALSE;
  while (g2) 
    {
      nad = g2->nad;
      if (EQ(nad->v_3035, INVOICEE_CODE_STR)) 
        {
          loenIsFound = TRUE;
          break;
        }
      else
        {
          g2 = g2->g_2_next;
        }
    }

  if (loenIsFound == FALSE)
    {
      return FALSE;
    }
  
  g3 = g2->g_3;
  while (g3) 
    {
      rff = g3->rff;
      if (EQ(rff->v_1153, PAYMENT_TYPE_CODE_STR)) 
        {
          loenIsFound = TRUE;
          break;
        }
      else
        {
          g3 = g3->g_3_next;
        }
    }
  
  if (loenIsFound == TRUE) 
    {
      rff = g3->rff;
      strncpy(pasnzType, rff->v_1154, poiTypeBufLen);
    }
  else
    {
      return FALSE;
    }

  return TRUE;
}
