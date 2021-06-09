#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <varargs.h>

#include "parser.h"
#include "timm.h"
#include "types.h"
#include "bgh.h"
#include "line_list.h"
#include "stream.h"
#include "generator.h"

/*
 * extern functions
 */

extern toenBool foenMoney_Scan(char *ppsnzBuf, signed long *ppilVal);
extern int fetchInvoiceTotalTax(struct s_TimmInter *inter, 
                                char *tax_amount_buf, int tax_amount_buf_len, 
                                char *currency_buf, int currency_buf_len); 
extern int fetchInvoiceTotalValue(struct s_TimmInter *inter, 
                                  char *value_buf, int value_buf_len, 
                                  char *currency_buf, int currency_buf_len); 

/*
  INV_MINUS_TYPE
  INV_MINUSVAT_TYPE
  INV_DEFAULT_TYPE
*/

toenInvType foenInvoiceClass(stTIMM *pstTimm, stTIMM *pstBalTimm, stTIMM *pstSumTimm)
{
  static char dasnzCurrency   [MAX_BUFFER];
  static char dasnzTotalAmount[MAX_BUFFER];
  double loflTotalAmount;
  static char dasnzVATAmount  [MAX_BUFFER];
  double loflVATAmount;
  toenBool loenStatus;
  int ok, n;
  
  /*
   * Get value of the invoice from first MOA+77 item
   */

  if ((ok = fetchInvoiceTotalValue(pstTimm, dasnzTotalAmount, MAX_BUFFER, dasnzCurrency, MAX_BUFFER)) == FALSE)
    {
      return INV_UNDEFINED_TYPE;
    }

  n = sscanf(dasnzTotalAmount, "%lf", &loflTotalAmount);
  ASSERT(n != 0);

  /*
   * Get tax amount from MOA+124 in first G47 item
   */

  if ((ok = fetchInvoiceTotalTax(pstTimm, dasnzVATAmount, MAX_BUFFER, dasnzCurrency, MAX_BUFFER)) == FALSE)
    {
      return INV_UNDEFINED_TYPE;
    }
  
  n = sscanf(dasnzVATAmount, "%lf", &loflVATAmount);
  ASSERT(n != 0);

  /*
   * Apply rules
   */

  if (loflVATAmount <= 0.0 && loflTotalAmount >= 0.0)
    {
      return INV_MINUSVAT_TYPE;
    }
  
  if (loflTotalAmount <= 0.0)
    {
      return INV_MINUS_TYPE;
    }

  return INV_DEFAULT_TYPE;
}
