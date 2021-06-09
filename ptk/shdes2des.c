/**************************************************************************************************
 *                                                                                          
 * MODULE: SHDES2DES
 *                                                                                          
 * AUTHOR: N.Bondarczuk                                                
 * 
 * CREATION DATE : 30.04.98                                              
 *
 * MODIFICATION DATE 12.10.1999
 * 
 * ADDED int foiMapMarketDes2NetworkName(char *ppszMarket, char  *ppszNetwork)
 **************************************************************************************************
 */

#include <stdio.h>
#include <string.h>
#include "bgh.h"
#include "protos.h"
#include "parser.h"
#include "types.h"
#include "gen.h"
#include "fnmatch.h"


#if 0	/* just for version.sh */
static char *SCCS_VERSION = "4.0";
#endif

extern stTariffZone *pstTariffZone;
extern long glTZCount;

extern stTariffTime *pstTariffTime;
extern long glTTCount;

extern stTariffModel *pstTariffModel;
extern long glTMCount;

extern stServicePackage *pstServicePackage;
extern long glSPCount;

extern stServiceName *pstServiceName;
extern long glSNCount;

extern stVPLMN *pstVPLMN;
extern long lVPLMNCount;

extern tostPriceGroup *gpstPriceGroup;
extern long goilPriceGroupSize;

extern int goilPaymentTypeSize;
extern tostPaymentType *gpstPaymentType;

/****************************************************************************
 *
 * FUNCTION: fpsnzMapTZShdes2Des
 *
 * DESCRIPTION: Mapping of TZ short description to full description
 *              Returned is pointer to string in global table.
 *
 ***************************************************************************/

char *fpsnzMapTZShdes2Des(char *pasnzShdes)
{
  int i;

  for (i = 0; i < glTZCount; i++)
    {
      if (EQ(pasnzShdes, pstTariffZone[i].szShdes))
        {
          return pstTariffZone[i].szDes;
        }
    }

  /*
   * Nothing found
   */

  return NULL;
}

/****************************************************************************
 *
 * FUNCTION: fpsnzMapTTShdes2Des
 *
 * DESCRIPTION: Mapping of TT short description to full description
 *              Returned is pointer to string in global table.
 *
 ***************************************************************************/

char *fpsnzMapTTShdes2Des(char *pasnzShdes)
{
  int i;

  for (i = 0; i < glTTCount; i++)
    {
      if (EQ(pasnzShdes, pstTariffTime[i].szShdes))
        {
          return pstTariffTime[i].szDes;
        }
    }

  /*
   * Nothing found
   */

  return NULL;
}

/****************************************************************************
 *
 * FUNCTION: fpsnzMapTMShdes2Des
 *
 * DESCRIPTION: Mapping of TM short description to full description
 *              Returned is pointer to string in global table.
 *
 ***************************************************************************/

char *fpsnzMapTMShdes2Des(char *pasnzShdes)
{
  int i;

  for (i = 0; i < glTMCount; i++)
    {
      if (EQ(pasnzShdes, pstTariffModel[i].szShdes))
        {
          return pstTariffModel[i].szDes;
        }
    }

  /*
   * Nothing found
   */

  return NULL;
}

/****************************************************************************
 *
 * FUNCTION: fpsnzMapSPShdes2Des
 *
 * DESCRIPTION: Mapping of TZ short description to full description
 *              Returned is pointer to string in global table.
 *
 ***************************************************************************/

char *fpsnzMapSPShdes2Des(char *pasnzShdes)
{
  int i;

  for (i = 0; i < glSPCount; i++)
    {
      if (EQ(pasnzShdes, pstServicePackage[i].szShdes))
        {
          return pstServicePackage[i].szDes;
        }
    }

  /*
   * Nothing found
   */

  return NULL;
}

/****************************************************************************
 *
 * FUNCTION: fpsnzMapSNShdes2Des
 *
 * DESCRIPTION: Mapping of SN short description to full description
 *              Returned is pointer to string in global table.
 *
 ***************************************************************************/

char *fpsnzMapSNShdes2Des(char *pasnzShdes)
{
  int i;

  for (i = 0; i < glSNCount; i++)
    {
      if (EQ(pasnzShdes, pstServiceName[i].szShdes))
        {
          return pstServiceName[i].szDes;
        }
    }

  /*
   * Nothing found
   */

  return NULL;
}

/****************************************************************************
 *
 * FUNCTION: fpsnzMapVPLMNShdes2Des
 *
 * DESCRIPTION: Mapping of SN short description to full description
 *              Returned is pointer to string in global table.
 *
 ***************************************************************************/

/*
  typedef struct {
  char szShdes[7];
  char szPlmnName[13];
  char szCountry[21];
  } stVPLMN;

extern stVPLMN *pstVPLMN;
extern long lVPLMNCount;

*/

char *fpsnzMapVPLMNShdes2Des(char *pasnzShdes)
{
  int i;

  for (i = 0; i < lVPLMNCount; i++)
    {
      if (EQ(pasnzShdes, pstVPLMN[i].szShdes))
        {
          return pstVPLMN[i].szPlmnName;
        }
    }

  /*
   * Nothing found
   */

  return NULL;
}

char *fpsnzMapVPLMNShdes2Country(char *pasnzShdes)
{
  int i;

  for (i = 0; i < lVPLMNCount; i++)
    {
      if (EQ(pasnzShdes, pstVPLMN[i].szShdes))
        {
          return pstVPLMN[i].szCountry;
        }
    }

  /*
   * Nothing found
   */

  return NULL;
}

/****************************************************************************
 *
 * FUNCTION: fpsnzMapPriceGroupCode2Name
 *
 * DESCRIPTION: Mapping of PG code to name
 *
 ***************************************************************************/

char *fpsnzMapPriceGroupCode2Name(char *pasnzCode)
{
  int i;

  for (i = 0; i < goilPriceGroupSize; i++)
    {
      if (EQ(pasnzCode, gpstPriceGroup[i].sasnzCode))
        {
          return gpstPriceGroup[i].sasnzName;
        }
    }

  /*
   * Nothing found
   */

  return NULL;
}

/****************************************************************************
 *
 * FUNCTION: foenMapPaymentCode2Type
 *
 * DESCRIPTION: Mapping of PG code to name
 *
 ***************************************************************************/

toenBool foenMapPaymentCode2Type(char pochCode, tostPaymentTypeBuf *ppstPaymentType)
{
  int i;

  for (i = 0; i < goilPaymentTypeSize; i++)
    {
      if (pochCode == gpstPaymentType[i].sochPaymentCode)
        {
          sprintf(ppstPaymentType->sasnzPaymentId, "%d", gpstPaymentType[i].soiPaymentId);
          sprintf(ppstPaymentType->sasnzPaymentCode, "%c", gpstPaymentType[i].sochPaymentCode);
          strncpy(ppstPaymentType->sasnzPaymentName, gpstPaymentType[i].sasnzPaymentName, MAX_BUFFER);
          
          return TRUE;
        }
    }
     
  return FALSE;
}

/*
 * Map market DES 2 market id with table MPDSCTAB,  ADDED 12.10.99 
*/

extern int goiMarketTabSize;
extern tostMarket *gpstMarket;

int foiMapMarketDes2Id(char *ppchzMarket, int *ppiPLMNId)
{
  int i;

  for (i = 0; i < goiMarketTabSize; i++)
    {
      if (EQ(gpstMarket[i].sasnzMarketDes, ppchzMarket))
        {
          *ppiPLMNId = gpstMarket[i].soiPLMNId;          
          return gpstMarket[i].soiMarketId;
        }
    }

  return -1;
}

/*
 * Map the market DES 2 the network name (PLMNNAME) with the table MPDSCTAB
 */


int foiMapMarketDes2NetworkName(char *poszMarket, char  *poszNetwork)
{
  int i;

  for (i = 0; i < goiMarketTabSize; i++)
    {
      if (EQ(gpstMarket[i].sasnzMarketDes, poszMarket))
        {
          strcpy(poszNetwork, gpstMarket[i].soszNetworkName);          
          return gpstMarket[i].soiMarketId;
        }
    }

  return -1;
}

