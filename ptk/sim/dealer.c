#include <stdio.h>
#include "bgh.h"
#include "protos.h"
#include "parser.h"

#include "types.h"
#include "gen.h"
#include "strutl.h"
#include "inv_item.h"
#include "inv_types.h"
#include "inv_fetch.h"
#include "sum_fetch_acc.h"
#include "sum_fetch_sim.h"
#include "dealer.h"

#if 0	/* just for version.sh */
static char *SCCS_VERSION = "1.0";
#endif

extern stBGHGLOB	stBgh;			/* structure with globals for BGH */
extern int foiInsertAirTime(char *ppchzCustomerId, char *pachzContractNo, char *pachzInvoiceNo, 
                     char *pachzInvoiceDate, char *pachzOhEntryDate, char *pachzAirTimeValue, 
                     int poiSCCODE, int poiPLCODE);

toenBool foenHandleDealerComission(struct s_TimmInter *ppstSumSheet, int poiSubscriberNo, int poiContractNo)
{
  tostContractData lostCD;
  toenBool loenStatus;
  static char lachzInvoiceNo[MAX_BUFFER];
  static char lachzAirTimeValue[MAX_BUFFER];
  char *lpchzCustomerId;
  static char lachzSim[MAX_BUFFER];
  static char lachzMarket[MAX_BUFFER];
  static char lachzInvDate[MAX_BUFFER];
  static char loszOhEntryDate[MAX_BUFFER];
  static char loszEndDate[MAX_BUFFER];
  int rc;
  int loiMarketId, loiPLMNId;

  /*
  printf("SUB:%d, CN: %d\n", poiSubscriberNo, poiContractNo);
  */
  /*
   * Set up contract no
   */

  loenStatus = foenFetchContractData(ppstSumSheet, poiSubscriberNo, poiContractNo, &lostCD);
  if (loenStatus == FALSE)
    {      
      return FALSE;
    }
  fovdPrintLog (LOG_DEBUG, "CO_ID: %s\n", lostCD.sachzContractNo);

  /*
   * Set up customer id
   */
  
  lpchzCustomerId  = ppstSumSheet->unb->v_0010;
  fovdPrintLog (LOG_DEBUG, "CUSTOMER_ID: %s\n", lpchzCustomerId);

  /*
   * Set up OHREFNUM
   */

  loenStatus = foenFetchInvoiceNo(ppstSumSheet, lachzInvoiceNo, MAX_BUFFER); 
  if (loenStatus == FALSE)
    {
      return FALSE;
    }
  fovdPrintLog (LOG_DEBUG, "INVOICE: %s\n", lachzInvoiceNo);

  /*
   * Find SIM number
   */

  loenStatus = foenFetchContractSim(ppstSumSheet, poiSubscriberNo, poiContractNo, lachzSim, MAX_BUFFER); 
  if (loenStatus == FALSE)
    {
      return FALSE;
    }

  fovdPrintLog (LOG_DEBUG, "SIM: %s\n", lachzSim);

  /*
   * Fetch market info
   */

  loenStatus = foenFetchContractMarket(ppstSumSheet, poiSubscriberNo, poiContractNo, lachzMarket, MAX_BUFFER); 
  if (loenStatus == FALSE)
    {
      return FALSE;
    }

  loiMarketId = foiMapMarketDes2Id(lachzMarket, &loiPLMNId);
  
  fovdPrintLog (LOG_DEBUG, "Market: %s, %d, %d\n", lachzMarket, loiMarketId, loiPLMNId);
  
  /*
   * Invoice date 
   */
  
  loenStatus = foenFetchInvoiceDate(ppstSumSheet, lachzInvDate, MAX_BUFFER); 
  if (loenStatus == FALSE)
    {
      return FALSE;
    }

  fovdPrintLog (LOG_DEBUG, "DATE: %s\n", lachzInvDate);


  /*
   * Oh entry  date 
   */
  
  loenStatus = foenFetchInvicePeriodEnd(ppstSumSheet, loszEndDate,  MAX_BUFFER);
  if (loenStatus == FALSE)
    {
      return FALSE;
    }

  fovdPrintLog (LOG_DEBUG, "Oh entry DATE: %s\n", loszEndDate);
  loenStatus = foenDate_AddDay(loszEndDate, loszOhEntryDate);
  if (loenStatus == FALSE)
    {
      return FALSE;
    }


  /*
   * Find air time
   */
  
  loenStatus = foenFetchAirTime(ppstSumSheet, lachzSim,  lachzAirTimeValue, MAX_BUFFER);
  if (loenStatus == FALSE)
    {
      return FALSE;
    }

  fovdPrintLog (LOG_DEBUG, "AIR: %s\n", lachzAirTimeValue);

  /*
   * Insert it
   */
  rc = foiInsertAirTime(lpchzCustomerId, 
                        lostCD.sachzContractNo, 
                        lachzInvoiceNo, 
                        lachzInvDate,
                        loszOhEntryDate,
                        lachzAirTimeValue, 
                        loiMarketId, loiPLMNId);
#if 0   /*  it was used for test of windowing only */
  rc = foiInsertAirTime(lpchzCustomerId, 
                        lostCD.sachzContractNo, 
                        lachzInvoiceNo, 
                        lachzInvDate,
                        loszOhEntryDate+2,
                        lachzAirTimeValue, 
                        loiMarketId, loiPLMNId);
#endif

    if (rc != 0)
    {
      return FALSE;
    }
  fovdPrintLog (LOG_DEBUG, "Inserted\n");

  return TRUE;
}
