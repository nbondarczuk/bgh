/**************************************************************************************************
 *                                                                                          
 * MODULE: TIMM
 *                                                                                          
 * AUTHOR: N.Bondarczuk                                                
 * 
 * CREATION DATE : 08.01.98                                              
 *
 **************************************************************************************************
 */

/*
  History: 
  NB 990311 : added functionality for checking reverses done by corrective invoice

 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>

#include "bgh.h"
#include "protos.h"
#include "parser.h"
#include "types.h"
#include "gen.h"
#include "strutl.h"
#include "records.h"
#include "timm.h"

#if 0	/* just for version.sh */
static char *SCCS_VERSION = "1.1";
#endif

extern stTariffZone *pstTariffZone;
extern long glTZCount;

extern stTariffTime *pstTariffTime;
extern long glTTCount;

extern stTariffModel *pstTariffModel;
extern long glTMCount;

extern stServicePackage *pstServicePackage;
extern long glSPCount;

char *fpchzGetField(int, char *);

struct s_imd_seg *fpstFindItemDescription(struct s_imd_seg *, char *);

struct s_qty_seg *fpstFindQuantity(struct s_qty_seg *, char *, char *);

char *lpchzGetField(int, char *);

struct s_imd_seg *lpstFindItemDescription(struct s_imd_seg *,  char *);

struct s_qty_seg *lpstFindQuantity(struct s_qty_seg *, char *, char *);

struct s_pia_seg *lpstFindProductId(struct s_pia_seg *, toenRecordType);

/*
 * static functions
 */

static int compareSP(const void *, const void *);
static int compare(const void *, const void *);
static int compareSN(const void *, const void *);

struct s_group_2 *fpstFindPartyBlock(struct s_group_2 *ppstG2, 
                                     char *ppchzLabel) 
{
  struct s_group_2 *lpstG2;
  struct s_nad_seg *nad;

  lpstG2 = ppstG2;
  while (lpstG2) 
    {
      nad = lpstG2->nad;
      if (EQ(nad->v_3035, ppchzLabel))
        {
          return lpstG2;
        }
      lpstG2 = lpstG2->g_2_next;
    }
  
  return NULL;
}

struct s_moa_seg *fpstFindMainPaymentSegment(struct s_group_45 *ppstG45, 
                                             char *ppchzType,
                                             char *ppchzAmountTypeId) 
{
  struct s_moa_seg *lpstMoa;
  struct s_group_45 *lpstG45;
  
  lpstG45 = ppstG45;
  while (lpstG45)
    {
      lpstMoa = lpstG45->moa;
      if (EQ(lpstMoa->v_5025, ppchzType) 
          && EQ(lpstMoa->v_4405, ppchzAmountTypeId))
        {
          return lpstMoa;
        }
      
      lpstG45 = lpstG45->g_45_next;
    }

  return NULL;
}

struct s_moa_seg *fpstFindPaymentSegment(struct s_group_23 *ppstG23, 
                                         char *ppchzType,
                                         char *ppchzAmountTypeId) 
{
  struct s_moa_seg *lpstMoa;
  struct s_group_23 *lpstG23;
  
  lpstG23 = ppstG23;
  while (lpstG23)
    {
      lpstMoa = lpstG23->moa;
      if (EQ(lpstMoa->v_5025, ppchzType) 
          && EQ(lpstMoa->v_4405, ppchzAmountTypeId))
        {
          return lpstMoa;
        }
      
      lpstG23 = lpstG23->g_23_next;
    }

  return NULL;
}

static char *spszPatternTab[] = {"U", "A", "S", "O"};

struct s_group_22 *fpstFindChargeSegment(struct s_group_22 *ppstG22, 
                                         toenRecordType poenRecordType) 
{
  struct s_group_22 *lpstG22;
  struct s_lin_seg *lpstLin;
  struct s_imd_seg *lpstImd;
  char *lpszPattern;

  fovdPushFunctionName("fpstFindChargeSegment");
  
  lpszPattern = spszPatternTab[poenRecordType];
  
  lpstG22 = ppstG22;
  while (lpstG22)
    {
      lpstLin = lpstG22->lin;
      if (lpstLin && lpstLin->v_1222) 
        {
          if (EQ(lpstLin->v_1222, "03"))
            {
              lpstImd = lpstG22->imd;
              while (lpstImd)
                {
                  if (EQ(lpstImd->v_7009, "CT") && EQ(lpstImd->v_7008a, lpszPattern))
                    {
                      fovdPopFunctionName();
                      return lpstG22;
                    }
                  lpstImd = lpstImd->imd_next;
                }
            }
        }
      lpstG22 = lpstG22->g_22_next;
    }

  fovdPopFunctionName();
  return NULL;
}



struct s_imd_seg *fpstFindItemDescription(struct s_imd_seg *ppstImd, 
                                          char *ppszDescr)
{
  struct s_imd_seg *lpstImd;

  lpstImd = ppstImd;
  while (lpstImd) 
    {
      if (EQ(lpstImd->v_7009, ppszDescr))
        {
          return lpstImd;
        }
      lpstImd = lpstImd->imd_next;
    }

  return NULL;
}

struct s_rff_seg *fpstFindReference(struct s_group_3 *ppstG3, 
                                    char *ppszType) 
{
  struct s_group_3 *lpstG3;  
  struct s_rff_seg *lpstRff;

  lpstG3 = ppstG3;
  while (lpstG3) 
    {
      lpstRff = lpstG3->rff;
      if (EQ(lpstRff->v_1153, ppszType))
        {
          return lpstRff;
        }

      lpstG3 = lpstG3->g_3_next;
    }

  return NULL;
}


struct s_qty_seg *fpstFindQuantity(struct s_qty_seg *ppstQty, 
                                   char *ppchzDetails, 
                                   char *ppchzUnitType) 
{
  struct s_qty_seg *lpstQty;
  fovdPushFunctionName ("lpstFindQuantity");

  lpstQty = ppstQty;
  while (lpstQty) 
    {
      if (EQ(lpstQty->v_6063, ppchzDetails) 
          && EQ(lpstQty->v_6411, ppchzUnitType))
        {
          fovdPopFunctionName ();
          return lpstQty;
        }
      lpstQty = lpstQty->qty_next;
    }

  fovdPopFunctionName ();
  return NULL;
}

struct s_xcd_seg *fpstFindXCDSegment(struct s_group_99 *ppstG99, int poiIndex)
{
  struct s_xcd_seg *lpstXcd;
  struct s_group_99 *lpstG99;
  int loiIndex;

  loiIndex = 0;
  lpstG99 = ppstG99;
  while (lpstG99) 
    {
      if (loiIndex == poiIndex) 
        {
          return lpstG99->xcd;
        }
      else
        {
          loiIndex++;
        }
      lpstG99 = lpstG99->g_99_next;
    }

  return NULL;
}


struct s_pia_seg *fpstFindProductId(struct s_pia_seg *ppstPia, 
                                    toenRecordType poenRecordType) 
{
  return ppstPia;
};


/**************************************************************************************************
 *                                                                                          
 * FUNCTION : foenFillRecordUsage             
 *                                                                                          
 * LOCAL FUNCTIONS USED:                                                                    
 *                                                                                          
 * EXTERNAL FUNCTIONS USED:                                                                 
 * MODULE:
 *                                                                                          
 **************************************************************************************************
 */

static char *spszConnectionTypeTab[4] = {"AIR", "INTERCONNECT", "CALL_FORWARDING", "ROAMING"};

toenBool foenTimmFillRecordUsage(struct s_group_22 *ppstG22, tostUsageRecord *ppstRecord) {
  struct s_lin_seg *lpstLin;
  struct s_imd_seg *lpstImd;  
  struct s_qty_seg *lpstQty;
  struct s_pia_seg *lpstPia;
  struct s_moa_seg *lpstMoa;
  int loiConnection, i;
  char *lpszPriceId, *lpszConnectionType, *lpszTTDES, *lpszZNDES, *lpszVPLMN;
  char *lpchzTMShdes, *lpchzTMVer;
  int loiTMVer;
  char *p;
  stServicePackage *lpstServicePackage;

  fovdPushFunctionName("foenFillRecordUsage");

  /*
   * LIN block values
   */
  fovdPrintLog (LOG_DEBUG, "LIN\n");
  lpstLin = ppstG22->lin;
  if (lpstLin == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }

  /* 
   * Find name of tariff model
   */

  lpchzTMVer = lpchzGetField(1, lpstLin->v_7140);
  loiTMVer = atoi(lpchzTMVer);
  lpchzTMShdes = lpchzGetField(0, lpstLin->v_7140);
  fovdPrintLog (LOG_DEBUG, "PIA: %d, %s in %s\n", loiTMVer, lpchzTMShdes, lpstLin->v_7140);

  for (i = 0; i < glTMCount; i++)
    {
      fovdPrintLog (LOG_DEBUG, "FIND: %d, %s from %s in %d, %s\n", 
                    loiTMVer, lpchzTMShdes, lpstLin->v_7140,
                    pstTariffModel[i].lTariffModelVersion, pstTariffModel[i].szShdes);
      
      if (loiTMVer == pstTariffModel[i].lTariffModelVersion && EQ(lpchzTMShdes, pstTariffModel[i].szShdes))
        {
          strcpy(ppstRecord->sasnzTariffModel, pstTariffModel[i].szDes);
          break;
        }
    }
  if (i == glTMCount)
    {
      strcpy(ppstRecord->sasnzTariffModel, lpchzTMShdes);
    }

  /*
   * Service package
   */

  p = (char *)bsearch((char *)lpchzGetField(2, lpstLin->v_7140), 
                      pstServicePackage, 
                      glSPCount, 
                      sizeof(stServicePackage), 
                      compareSP);
  if (p != NULL)
    {
      lpstServicePackage = (stServicePackage *)p;
      strcpy(ppstRecord->sasnzServicePackage, lpstServicePackage->szDes);
    }
  else
    {
      strcpy(ppstRecord->sasnzServicePackage, lpchzGetField(2, lpstLin->v_7140));
    }


  /*
   * Serivce code 
   */
  strncpy(ppstRecord->sasnzServiceCode, lpchzGetField(3, lpstLin->v_7140), MAX_BUFFER);

  /*
   * IMD block values
   */
  fovdPrintLog (LOG_DEBUG, "IMD\n");
  lpstImd = lpstFindItemDescription(ppstG22->imd, "SN");
  if (lpstImd == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }
  strncpy(ppstRecord->sasnzServiceName, lpstImd->v_7008a, MAX_BUFFER);

  /*
   * PIA block values
   */
  fovdPrintLog (LOG_DEBUG, "PIA\n");
  lpstPia = ppstG22->pia;
  if (lpstLin == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }

  lpszConnectionType = lpchzGetField(0, lpstPia->v_7140);
  if (lpszConnectionType == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }
  switch(lpszConnectionType[0])
    {
    case 'A': loiConnection = 0; break;
    case 'I': loiConnection = 1; break;
    case 'C': loiConnection = 2; break;
    case 'R': loiConnection = 3; break;
    default:
      fovdPopFunctionName(); 
      return FALSE;
    }
  /* We have AIR, INTERCONNECT, CALLFORWARDING, ROAMING copied */
  strncpy(ppstRecord->sasnzConnectionType, spszConnectionTypeTab[loiConnection], MAX_BUFFER);


  lpszPriceId = lpchzGetField(1, lpstPia->v_7140);
  if (lpszPriceId == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }
  /* We can have INROAMING, OUTROAMING, NORMAL copied */
  switch(lpszPriceId[0]) 
    {
    case 'F':
    case 'B':
    case 'S':
    case 'T':
    case 'E':
      strncpy(ppstRecord->sasnzUsageType, "NORMAL", MAX_BUFFER);
      /*
       * The following items may be missing if usage is not splitted by TT and TZ
       * (no exetended GL package in use) 
       */

      /*
       * TARIFF TIME
       */
      lpszTTDES =  lpchzGetField(4, lpstPia->v_7140);
      if (lpszTTDES == NULL)
        {
          ppstRecord->sasnzTariffTime[0] = '\0';
        }
      else
        {
          /*
           * pstTariffTime : CODE -> NAME
           */
          for (i = 0; i < glTTCount; i++)
            {
              if (EQ(lpszTTDES, pstTariffTime[i].szShdes))
                {
                  strncpy(ppstRecord->sasnzTariffTime, pstTariffTime[i].szDes, MAX_BUFFER);
                  break;
                }
            }
          if (i == glTTCount)
            {
              strncpy(ppstRecord->sasnzTariffTime, lpszTTDES, MAX_BUFFER);
            }
        }

      /*
       * TARIFF ZONE
       */ 
      lpszZNDES =  lpchzGetField(5, lpstPia->v_7140);
      if (lpszZNDES == NULL || strlen(lpszZNDES) == 0)
        {
          strncpy(ppstRecord->sasnzTariffZone, lpstImd->v_7008a, MAX_BUFFER);
        }
      else
        {
          /*
           * pstTariffZone : CODE -> NAME
           */
          for (i = 0; i < glTZCount; i++)
            {
              if (EQ(lpszZNDES, pstTariffZone[i].szShdes))
                {
                  strncpy(ppstRecord->sasnzTariffZone, pstTariffZone[i].szDes, MAX_BUFFER);
                  break;
                }
            }
          if (i == glTZCount)
            {
              strncpy(ppstRecord->sasnzTariffZone, lpszZNDES, MAX_BUFFER);
            }
        }
      break;

    case 'R':
    case 'r':
      strncpy(ppstRecord->sasnzUsageType, "OUTROAMING", MAX_BUFFER);
      lpszVPLMN =  lpchzGetField(2, lpstPia->v_7140);
      if (lpszVPLMN == NULL)
        {
          fovdPopFunctionName();
          return FALSE;
        }
      strncpy(ppstRecord->sasnzPLMN, lpszVPLMN, MAX_BUFFER);
      break;

    case 'm':
      strncpy(ppstRecord->sasnzUsageType, "INROAMING", MAX_BUFFER);
      lpszVPLMN =  lpchzGetField(2, lpstPia->v_7140);
      if (lpszVPLMN == NULL)
        {
          fovdPopFunctionName();
          return FALSE;
        }
      strncpy(ppstRecord->sasnzPLMN, lpszVPLMN, MAX_BUFFER);
      break;
    default:
      fovdPopFunctionName();
      return FALSE;
    }


  /*
  strcpy(ppstRecord->sasnzTariffTime, lpszTTDES = lpchzGetField(5, lpstPia->v_7140));
  strcpy(ppstRecord->sasnzTariffZone, lpszZNDES = lpchzGetField(6, lpstPia->v_7140));  
  */

  /*
   * QTY block values
   */
  fovdPrintLog (LOG_DEBUG, "QTY\n");
  lpstQty = lpstFindQuantity(ppstG22->qty, "107", "UNI");
  if (lpstQty == NULL)
    {
      /*
       * If found usage charge accounted twice
       */
      lpstQty = lpstFindQuantity(ppstG22->qty, "108", "UNI");
      if (lpstQty == NULL)
        {
          fovdPopFunctionName();
          return FALSE;
        }
      else
        {
          strncpy(ppstRecord->sasnzQuantity, "0", MAX_BUFFER);
        }
    }
  else
    {
      strncpy(ppstRecord->sasnzQuantity, lpstQty->v_6060, MAX_BUFFER);
    }

  /*
   * MOA+125 block values
   */

  fovdPrintLog (LOG_DEBUG, "MOA\n");
  lpstMoa = fpstFindPaymentSegment(ppstG22->g_23, "125", "5");
  if (lpstMoa == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }
  strncpy(ppstRecord->sasnzMonetaryAmount, lpstMoa->v_5004, MAX_BUFFER);
  strncpy(ppstRecord->sasnzCurrency, lpstMoa->v_6345, MAX_BUFFER);
  
  fovdPopFunctionName();
  return TRUE;
}; 





/**************************************************************************************************
 *                                                                                          
 * FUNCTION : foenFillRecordAccess             
 *                                                                                          
 * LOCAL FUNCTIONS USED:                                                                    
 *                                                                                          
 * EXTERNAL FUNCTIONS USED:                                                                 
 * MODULE:
 *                                                                                          
 **************************************************************************************************
 */

static char *spszAccessTypeTab[] = {"FULL", "PRORATE"};
static char *spszChargeTypeTab[] = {"PAST", "ADVANCE", "CRDIT"};

toenBool foenTimmFillRecordAccess(struct s_group_22 *ppstG22, tostAccessRecord *ppstRecord) 
{
  struct s_lin_seg *lpstLin;
  struct s_imd_seg *lpstImd;  
  struct s_qty_seg *lpstQty;
  struct s_pia_seg *lpstPia;
  struct s_moa_seg *lpstMoa;
  char *lpszChargeType, *lpszAccessType;
  int loiAccess, loiCharge;
  char *lpchzTMShdes, *lpchzTMVer;
  int loiTMVer, i;
  char *p;
  stServicePackage *lpstServicePackage;

  fovdPushFunctionName("foenFillRecordAccess");

  /*
   * LIN block values
   */

  lpstLin = ppstG22->lin;
  if (lpstLin == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }

  /* 
   * Find name of tariff model
   */

  lpchzTMVer = lpchzGetField(1, lpstLin->v_7140);
  loiTMVer = atoi(lpchzTMVer);
  lpchzTMShdes = lpchzGetField(0, lpstLin->v_7140);
  fovdPrintLog (LOG_DEBUG, "PIA: %d, %s in %s\n", loiTMVer, lpchzTMShdes, lpstLin->v_7140);

  for (i = 0; i < glTMCount; i++)
    {
      fovdPrintLog (LOG_DEBUG, "FIND: %d, %s from %s in %d, %s\n", 
                    loiTMVer, lpchzTMShdes, lpstLin->v_7140,
                    pstTariffModel[i].lTariffModelVersion, pstTariffModel[i].szShdes);
      
      if (loiTMVer == pstTariffModel[i].lTariffModelVersion && EQ(lpchzTMShdes, pstTariffModel[i].szShdes))
        {
          strcpy(ppstRecord->sasnzTariffModel, pstTariffModel[i].szDes);
          break;
        }
    }
  if (i == glTMCount)
    {
      strcpy(ppstRecord->sasnzTariffModel, lpchzTMShdes);
    }

  /*
   * Service package
   */

  p = (char *)bsearch((char *)lpchzGetField(2, lpstLin->v_7140), 
                      pstServicePackage, 
                      glSPCount, 
                      sizeof(stServicePackage), 
                      compareSP);
  if (p != NULL)
    {
      lpstServicePackage = (stServicePackage *)p;
      strcpy(ppstRecord->sasnzServicePackage, lpstServicePackage->szDes);
    }
  else
    {
      strcpy(ppstRecord->sasnzServicePackage, lpchzGetField(2, lpstLin->v_7140));
    }

  /*
   * Service code
   */

  strncpy(ppstRecord->sasnzServiceCode, lpchzGetField(3, lpstLin->v_7140), MAX_BUFFER);

  /*
   * IMD block values
   */

  lpstImd = lpstFindItemDescription(ppstG22->imd, "SN");
  if (lpstImd == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }
  strncpy(ppstRecord->sasnzServiceName, lpstImd->v_7008a, MAX_BUFFER);

  /*
   * PIA block values
   */

  lpstPia = lpstFindProductId(ppstG22->pia, RECORD_ACCESS);
  if (lpstLin == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }

  lpszAccessType = lpchzGetField(0, lpstPia->v_7140);
  if (lpszAccessType == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }
  /* We have PRORATE, FULL values */
  switch (lpszAccessType[0])
    {
    case 'F': loiAccess = 0; break;
    case 'P': loiAccess = 1; break;
    default: return FALSE;
    }
  strncpy(ppstRecord->sasnzAccessType, spszAccessTypeTab[loiAccess], MAX_BUFFER);

  lpszChargeType = lpchzGetField(1, lpstPia->v_7140);
  if (lpszChargeType == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }
  /* We have PAST, ADVANCE, CREDIT values */
  switch (lpszChargeType[0]) 
    {
    case 'P': loiCharge = 0; break;
    case 'A': loiCharge = 1; break;
    case 'C': loiCharge = 2; break;
    default: 
      fovdPopFunctionName();
      return FALSE;
    }
  strncpy(ppstRecord->sasnzChargeType, spszChargeTypeTab[loiCharge], MAX_BUFFER);
  
  /*
   * QTY block values
   */
  lpstQty = lpstFindQuantity(ppstG22->qty, "109", "DAY");
  if (lpstQty == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }
  strncpy(ppstRecord->sasnzQuantity, lpstQty->v_6060, MAX_BUFFER);
  
  /*
   * MOA+125 block values
   */
 
  lpstMoa = fpstFindPaymentSegment(ppstG22->g_23, "125", "5");
  if (lpstMoa == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }
  strncpy(ppstRecord->sasnzMonetaryAmount, lpstMoa->v_5004, MAX_BUFFER);
  strncpy(ppstRecord->sasnzCurrency, lpstMoa->v_6345, MAX_BUFFER);
  
  fovdPopFunctionName();
  return TRUE;
}; 


/**************************************************************************************************
 *                                                                                          
 * FUNCTION : foenFillRecordService             
 *                                                                                          
 * LOCAL FUNCTIONS USED:                                                                    
 *                                                                                          
 * EXTERNAL FUNCTIONS USED:                                                                 
 * MODULE:
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenTimmFillRecordService(struct s_group_22 *ppstG22, tostServiceRecord *ppstRecord) 
{
  struct s_lin_seg *lpstLin;
  struct s_imd_seg *lpstImd;  
  struct s_qty_seg *lpstQty;
  struct s_moa_seg *lpstMoa;
  char *lpchzTMShdes, *lpchzTMVer;
  int loiTMVer, i;
  char *p;
  stServicePackage *lpstServicePackage;


  fovdPushFunctionName("foenFillRecordService");

  /*
   * LIN block values
   */

  lpstLin = ppstG22->lin;
  if (lpstLin == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }

  /* 
   * Find name of tariff model
   */

  lpchzTMVer = lpchzGetField(1, lpstLin->v_7140);
  loiTMVer = atoi(lpchzTMVer);
  lpchzTMShdes = lpchzGetField(0, lpstLin->v_7140);
  fovdPrintLog (LOG_DEBUG, "PIA: %d, %s in %s\n", loiTMVer, lpchzTMShdes, lpstLin->v_7140);

  for (i = 0; i < glTMCount; i++)
    {
      fovdPrintLog (LOG_DEBUG, "FIND: %d, %s from %s in %d, %s\n", 
                    loiTMVer, lpchzTMShdes, lpstLin->v_7140,
                    pstTariffModel[i].lTariffModelVersion, pstTariffModel[i].szShdes);
      
      if (loiTMVer == pstTariffModel[i].lTariffModelVersion && EQ(lpchzTMShdes, pstTariffModel[i].szShdes))
        {
          strcpy(ppstRecord->sasnzTariffModel, pstTariffModel[i].szDes);
          break;
        }
    }
  if (i == glTMCount)
    {
      strcpy(ppstRecord->sasnzTariffModel, lpchzTMShdes);
    }

  /*
   * Service package
   */

  p = (char *)bsearch((char *)lpchzGetField(2, lpstLin->v_7140), 
                      pstServicePackage, 
                      glSPCount, 
                      sizeof(stServicePackage), 
                      compareSP);
  if (p != NULL)
    {
      lpstServicePackage = (stServicePackage *)p;
      strcpy(ppstRecord->sasnzServicePackage, lpstServicePackage->szDes);
    }
  else
    {
      strcpy(ppstRecord->sasnzServicePackage, lpchzGetField(2, lpstLin->v_7140));
    }


  /*
   * Service code
   */

  strncpy(ppstRecord->sasnzServiceCode, lpchzGetField(3, lpstLin->v_7140), MAX_BUFFER);

  /*
   * IMD block values
   */

  lpstImd = lpstFindItemDescription(ppstG22->imd, "SN");
  if (lpstImd == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }
  strncpy(ppstRecord->sasnzServiceName, lpstImd->v_7008a, MAX_BUFFER);

  /*
   * QTY block values
   */
  lpstQty = lpstFindQuantity(ppstG22->qty, "107", "UNI");
  if (lpstQty == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }
  strncpy(ppstRecord->sasnzQuantity, lpstQty->v_6060, MAX_BUFFER);
  
  /*
   * MOA+125 block values
   */
  lpstMoa = fpstFindPaymentSegment(ppstG22->g_23, "125", "5");
  if (lpstMoa == NULL)
    {
      fovdPopFunctionName();
      return FALSE;
    }
  strncpy(ppstRecord->sasnzMonetaryAmount, lpstMoa->v_5004, MAX_BUFFER);
  strncpy(ppstRecord->sasnzCurrency, lpstMoa->v_6345, MAX_BUFFER);
  
  fovdPopFunctionName();
  return TRUE;
}; 


/**************************************************************************************************
 *                                                                                          
 * FUNCTION : foenTimmFillRecordOther             
 *                                                                                          
 * LOCAL FUNCTIONS USED:                                                                    
 *                                                                                          
 * EXTERNAL FUNCTIONS USED:                                                                 
 * MODULE:
 *                                                                                          
 **************************************************************************************************
 */

toenBool foenTimmFillRecordOther(struct s_group_22 *ppstG22, tostOtherRecord *ppstRecord) {
  struct s_lin_seg *lpstLin;
  struct s_imd_seg *lpstImd;  
  struct s_qty_seg *lpstQty;
  struct s_moa_seg *lpstMoa;
  char *lpchzTMShdes, *lpchzTMVer;
  int loiTMVer, i;
  char *p;
  stServicePackage *lpstServicePackage;


  fovdPushFunctionName("foenFillRecordOther");

  /*
   * LIN block values
   */

  lpstLin = ppstG22->lin;
  if (lpstLin == NULL)
    {
      fovdPrintLog (LOG_CUSTOMER, "LIN block not found\n");      
      fovdPopFunctionName();
      return FALSE;
    }

  /* 
   * Find name of tariff model
   */

  lpchzTMVer = lpchzGetField(1, lpstLin->v_7140);
  loiTMVer = atoi(lpchzTMVer);
  lpchzTMShdes = lpchzGetField(0, lpstLin->v_7140);
  fovdPrintLog (LOG_DEBUG, "PIA: %d, %s in %s\n", loiTMVer, lpchzTMShdes, lpstLin->v_7140);

  for (i = 0; i < glTMCount; i++)
    {
      fovdPrintLog (LOG_DEBUG, "FIND: %d, %s from %s in %d, %s\n", 
                    loiTMVer, lpchzTMShdes, lpstLin->v_7140,
                    pstTariffModel[i].lTariffModelVersion, pstTariffModel[i].szShdes);
      
      if (loiTMVer == pstTariffModel[i].lTariffModelVersion && EQ(lpchzTMShdes, pstTariffModel[i].szShdes))
        {
          strcpy(ppstRecord->sasnzTariffModel, pstTariffModel[i].szDes);
          break;
        }
    }
  if (i == glTMCount)
    {
      strcpy(ppstRecord->sasnzTariffModel, lpchzTMShdes);
    }

  /*
   * Service package
   */

  p = (char *)bsearch((char *)lpchzGetField(2, lpstLin->v_7140), 
                      pstServicePackage, 
                      glSPCount, 
                      sizeof(stServicePackage), 
                      compareSP);
  if (p != NULL)
    {
      lpstServicePackage = (stServicePackage *)p;
      strcpy(ppstRecord->sasnzServicePackage, lpstServicePackage->szDes);
    }
  else
    {
      strcpy(ppstRecord->sasnzServicePackage, lpchzGetField(2, lpstLin->v_7140));
    }

  /*
   * Service code
   */

  strncpy(ppstRecord->sasnzServiceCode, lpchzGetField(3, lpstLin->v_7140), MAX_BUFFER);

  /*
   * IMD block values
   */

  lpstImd = lpstFindItemDescription(ppstG22->imd, "FE");
  if (lpstImd == NULL)
    {
      fovdPrintLog (LOG_CUSTOMER, "IMD FE block not found\n");      
      fovdPopFunctionName();
      return FALSE;
    }
  strncpy(ppstRecord->sasnzServiceName, lpstImd->v_7008a, MAX_BUFFER);

  /*
   * QTY block values
   */
  lpstQty = lpstFindQuantity(ppstG22->qty, "107", "UNI");
  if (lpstQty == NULL)
    {
      fovdPrintLog (LOG_CUSTOMER, "QTY block not found\n");      
      fovdPopFunctionName();
      return FALSE;
    }
  strncpy(ppstRecord->sasnzQuantity, lpstQty->v_6060, MAX_BUFFER);
  
  /*
   * MOA+125 block values
   */

  lpstMoa = fpstFindPaymentSegment(ppstG22->g_23, "125", "5");
  if (lpstMoa == NULL)
    {
      fovdPrintLog (LOG_CUSTOMER, "MOA+125 block not found\n");      
      fovdPopFunctionName();
      return FALSE;
    }
  strncpy(ppstRecord->sasnzMonetaryAmount, lpstMoa->v_5004, MAX_BUFFER);
  strncpy(ppstRecord->sasnzCurrency, lpstMoa->v_6345, MAX_BUFFER);
  
  fovdPopFunctionName();
  return TRUE;
}; 

char *fpchzGetField(int poiFieldNo, char *pochzField)
{
  static char lpchzOutput[MAX_BUFFER];
  char *lpchzToken;
  char lpchzField[MAX_BUFFER];
  int loiTokenNo;
  int loiFound;
  
  strncpy(lpchzField, pochzField, MAX_BUFFER);
  loiTokenNo = 0;

  loiFound = TRUE;
  lpchzToken = strtok(lpchzField, ".");
  while (loiTokenNo < poiFieldNo)
    {
      lpchzToken = strtok(NULL, ".");
      if (lpchzToken == NULL)
        {
          lpchzOutput[0] = '\0';
          loiFound = FALSE;
          break;
        }
      loiTokenNo++;
    }
  
  if (loiFound)
    {
      strncpy(lpchzOutput, lpchzToken, MAX_BUFFER);
    }

  return lpchzOutput;
}


extern stDest *pstDest;
extern stVPLMN *pstVPLMN;
extern long lVPLMNCount;
extern stServiceName *pstServiceName;
extern long glSNCount;

char *fpchzTranslate(char *, char *, char *);
struct s_xcd_seg *lpstFindXCDSegment(struct s_group_99 *, int);
char *lpchzFindCountryOfVPLMN(char *);

toenBool foenTimmFillSimCallRecord(struct s_group_99 *ppstG99, tostCallRecord *postCallRecord, float pofThreshold)
{
  struct s_group_22 *lpstG22;
  struct s_group_23 *lpstG23;
  struct s_group_99 *lpstG99, *lpstG99Seq;  
  struct s_moa_seg *lpstMoa;
  struct s_lin_seg *lpstLin;
  struct s_xcd_seg *lpstXcd,  *lpstXcdSeq;
  int loiRecordNo, i, loiFound, n, loiIndex, loiNumberSeq, loiNumber;
  toenBool loenRecordFound, loenInSeq;
  char *lpchzCountry;
  stVPLMN *pstActVPLMN;
  stServiceName *lpstServiceName;
  char *p;
  float lofThreshold;
  float lofAirtimePayment, lofAirtimePaymentSeq;
  float lofInterconnectPayment, lofInterconnectPaymentSeq;

  fovdPushFunctionName ("foenTimmFillSimCallRecord");

  /*
   * Set up local variables
   */

  lpstG99 = ppstG99;
  lpstXcd = ppstG99->xcd;
  pofThreshold = pofThreshold;
  fovdPrintLog (LOG_DEBUG, "CALL\n");

  /*
   * Mapping XCD -> CallRecord
   */

  strcpy(postCallRecord->sasnzType, lpstXcd->v_X019);
  strcpy(postCallRecord->sasnzCurrency, lpstXcd->v_6345);
  strcpy(postCallRecord->sasnzDate, lpstXcd->v_X004);
  strcpy(postCallRecord->sasnzTime, lpstXcd->v_X004);
  strcpy(postCallRecord->sasnzNumber, lpstXcd->v_X043);
  strcpy(postCallRecord->sasnzDuration, lpstXcd->v_X010);
  
  /*
   * Mapping special number -> * suffix
   */
  
  if (fnmatch(SPECIAL_NUMBER_PREFIX, postCallRecord->sasnzNumber, 0) == 0)
    {
      fovdPrintLog (LOG_DEBUG, "MATCH SPECIAL NUMBER: %s\n", postCallRecord->sasnzNumber);
      strcpy(postCallRecord->sasnzNumber + 1, postCallRecord->sasnzNumber + SPECIAL_NUMBER_PREFIX_INDEX);
      postCallRecord->sasnzNumber[0] = '*';
      fovdPrintLog (LOG_DEBUG, "RECODED TO NUMBER: %s\n", postCallRecord->sasnzNumber);
    }

  /*
   * Classification into ROAMING and NORMAL calls
   */
  if (lpstXcd->v_X028[0] == 'V')
    {
      /*
       * ROAMING
       */
      strcpy(postCallRecord->sasnzType, "R");
      strcpy(postCallRecord->sasnzPLMN, fpchzTranslate(lpstXcd->v_X029, "\n", "\0"));
      fovdPrintLog (LOG_DEBUG, "RAOMING CALL\n");

      /*
       * Mapping VPLMN -> Country (binary search in table pstVPLMN by index szShdes)
       */

      p = (char *)bsearch(postCallRecord->sasnzPLMN, pstVPLMN, lVPLMNCount, sizeof(stVPLMN), compare);
      if (p != NULL)
        {
          pstActVPLMN = (stVPLMN *)p;
          strcpy(postCallRecord->sasnzCountry, pstActVPLMN->szCountry);
        }
      else
        {
          strcpy(postCallRecord->sasnzCountry, "");
        }
    }
  else
    {
      /*
       * LOCAL
       */
      strcpy(postCallRecord->sasnzType, "L");
      fovdPrintLog (LOG_DEBUG, "LOCAL CALL\n");
    }
  

  /*
   * This is first part of call record (if IN leg exists)
   * and should not be included if summ of all parts
   * is below threshold.
   */
  n = sscanf(lpstXcd->v_X001, "%d", &loiNumber);                  
  if (n != 1)
    {
      return FALSE;
    }

  fovdPrintLog (LOG_DEBUG, "CALL %d\n", loiNumber);
  
  lofAirtimePayment = 0.0;
  lofInterconnectPayment = 0.0;  
  lpstG99Seq = lpstG99;
  while (lpstG99Seq)
    {
      lpstXcdSeq = lpstG99Seq->xcd;
      n = sscanf(lpstXcdSeq->v_X001, "%d", &loiNumberSeq);                  
      if (n != 1)
        {
          return FALSE;
        }

      if (loiNumberSeq != loiNumber)
        {
          /*
           * End of sequence of subparts of one call
           */
          break;
        }
      else
        {
          /*
           * Sum up found values
           */
          fovdPrintLog (LOG_DEBUG, "SUBCALL %s %s VAL = %s\n", 
                        lpstXcdSeq->v_X001, lpstXcdSeq->v_X002, lpstXcdSeq->v_5004);
          lofAirtimePaymentSeq = 0.0;
          lofInterconnectPaymentSeq = 0.0;
          if (lpstXcdSeq->v_X019[0] == 'I')
            {
              n = sscanf(lpstXcdSeq->v_5004, "%f", &lofInterconnectPaymentSeq);            
            }
          else
            {
              n = sscanf(lpstXcdSeq->v_5004, "%f", &lofAirtimePaymentSeq);            
            }
          if (n != 1)
            {
              return FALSE;
            }
          lofInterconnectPayment = lofInterconnectPayment + lofInterconnectPaymentSeq;
          lofAirtimePayment = lofAirtimePayment + lofAirtimePaymentSeq;
        }
      lpstG99Seq = lpstG99Seq->g_99_next;
    }
  
  sprintf(postCallRecord->sasnzLocalPayment, "%.2f", lofAirtimePayment);
  sprintf(postCallRecord->sasnzInternationalPayment, "%.2f", lofInterconnectPayment);
  sprintf(postCallRecord->sasnzSummaryPayment,"%.2f", lofAirtimePayment + lofInterconnectPayment );
  
  fovdPrintLog (LOG_DEBUG, "PAYMENT: %s, %s\n",
                postCallRecord->sasnzLocalPayment,
                postCallRecord->sasnzInternationalPayment);

  /*
   * Direction of called number - may be missing if RTX is for VAS service
   */

  strcpy(postCallRecord->sasnzDirection, lpstXcd->v_X045);
  fovdPrintLog (LOG_DEBUG, "CALL DIRECTION : %s\n", lpstXcd->v_X045);

  /*
   * Type of call
   */
  strcpy(postCallRecord->sasnzCallType, lpstXcd->v_X019);
  fovdPrintLog (LOG_DEBUG, "CALL TYPE : %s\n", lpstXcd->v_X019);

  /*
   * Sevice full name na dshort description
   */

  p = (char *)bsearch(lpstXcd->v_X013, pstServiceName, glSNCount, sizeof(stServiceName), compareSN);
  if (p != NULL)
    {
      lpstServiceName = (stServiceName *)p;
      strcpy(postCallRecord->sasnzServiceNameDes, lpstServiceName->szDes);
    }
  strcpy(postCallRecord->sasnzServiceNameShdes,  lpstXcd->v_X013);
  fovdPrintLog (LOG_DEBUG, "CALL SERVICE : %s\n", lpstXcd->v_X043);

  /*
   * Connection type
   */
  strcpy(postCallRecord->sasnzConnectionType,  lpstXcd->v_X019);
  fovdPrintLog (LOG_DEBUG, "CALL CONNECTION TYPE : %s\n", lpstXcd->v_X019);
  

  fovdPopFunctionName ();
  return TRUE;
}

static int compare(const void *key, const void *r)
{
  return strcasecmp(key, ((stVPLMN *)r)->szShdes);
}

static int compareSN(const void *key, const void *r)
{
  return strcasecmp(key, ((stServiceName *)r)->szShdes);
}

static int compareSP(const void *key, const void *r)
{
  return strcasecmp(key, ((stServicePackage *)r)->szShdes);
}


/*
 *
 * Count the number of call items in a ITB message
 *
 */

int foiCountITB_CallItems(struct s_TimmInter *ppstTimmInter, char *ppchzMode, int poiDropZeroValue) 
{
  struct s_group_22 *lpstG22;
  struct s_group_23 *lpstG23;
  struct s_group_99 *lpstG99, *lpstG99Seq;  
  struct s_moa_seg *lpstMoa;
  struct s_lin_seg *lpstLin;
  struct s_xcd_seg *lpstXcd, *lpstXcdSeq;
  int loiCalls, loiSubNumber, n, loiNumber, loiNumberSeq;
  float lofThreshold;
  float lofAirtimePayment, lofAirtimePaymentSeq;
  float lofInterconnectPayment, lofInterconnectPaymentSeq;

  fovdPushFunctionName ("foiCountITB_CallItems");

  /*
   * G22 segment on level 02
   */

  lpstG22 = ppstTimmInter->timm->g_22;
  if (lpstG22 == NULL)
    {
      fovdPopFunctionName ();
      return FALSE;
    }

  /*
   * G22 segment on level 03 with 
   * - IMD block empty 
   * - MOA+903 in group G23
   * - RFF+IC in group G23
   * - G99 not empty
   * Let's find any block with G99 not equal NULL
   */

  lpstG99 = NULL;
  lpstG22 = lpstG22->g_22_next;
  while (lpstG22)
    {
      lpstLin = lpstG22->lin;

      /*
       * Next contract or customer
       */
      if (EQ(lpstLin->v_1222, "02") || EQ(lpstLin->v_1222, "01"))
        {
          lpstG99 = NULL;
          break;
        }

      /*
       * Not broken loop so we are in a contract
       */
      if (lpstG22->g_99)
        {
          fovdPrintLog (LOG_DEBUG, "G99\n");
          lpstG23 = lpstG22->g_23;
          lofThreshold = 0.0;
          if (lpstG23)
            {
              fovdPrintLog (LOG_DEBUG, "G23\n");
              lpstMoa = lpstG23->moa;
              if (EQ(lpstMoa->v_5025, "901"))
                {
                  n = sscanf(lpstMoa->v_5004, "%f", &lofThreshold);
                  if (n != 1)
                    {
                      lofThreshold = 0.0;
                    }
                }
            }

          lpstG99 = lpstG22->g_99;
          break;
        }
      
      lpstG22 = lpstG22->g_22_next;
    }
  
  fovdPrintLog (LOG_DEBUG, "THRESHOLD : %f\n", lofThreshold);

  /*
   * Is it a correct G99 segment that was previously found for a given contract ?
   */

  if (lpstG99 != NULL)
    {
      loiCalls = 0;
      while (lpstG99) 
        {
          lpstXcd = lpstG99->xcd;
          n = sscanf(lpstXcd->v_X002, "%d", &loiSubNumber);
          if (EQ(lpstXcd->v_X028, ppchzMode) && loiSubNumber == 0)
            { 
              /*
               * First chunk of the sequence
               */

              if (poiDropZeroValue)          
                {
                  /*
                   * This is first part of call record (if IN leg exists)
                   * and should not be included if summ of all parts
                   * is below threshold.
                   */
                  n = sscanf(lpstXcd->v_X001, "%d", &loiNumber);                  
                  lofAirtimePayment = 0.0;
                  lofInterconnectPayment = 0.0;

                  lpstG99Seq = lpstG99;
                  while (lpstG99Seq)
                    {
                      lpstXcdSeq = lpstG99Seq->xcd;
                      n = sscanf(lpstXcdSeq->v_X001, "%d", &loiNumberSeq);                  
                      
                      if (loiNumberSeq != loiNumber)
                        {
                          /*
                           * End of sequence of subparts of one call
                           */
                          break;
                        }
                      else
                        {
                          /*
                           * Sum up found values
                           */
                          lofAirtimePaymentSeq = 0.0;
                          lofInterconnectPaymentSeq = 0.0;
                          if (lpstXcdSeq->v_X019[0] == 'I')
                            {
                              n = sscanf(lpstXcdSeq->v_5004, "%f", &lofInterconnectPaymentSeq);            
                            }
                          else
                            {
                              n = sscanf(lpstXcdSeq->v_5004, "%f", &lofAirtimePaymentSeq);            
                            }
                          lofInterconnectPayment =+ lofInterconnectPaymentSeq;
                          lofAirtimePayment =+ lofAirtimePaymentSeq;
                        }
                      lpstG99Seq = lpstG99Seq->g_99_next;
                    }

                  /*
                   * Recount summary value of a call
                   */
                  
                  /*
                   * Use threshold 
                   */
                  if (lofAirtimePayment + lofInterconnectPayment > lofThreshold)
                    {
                      loiCalls++;                                                      
                    }
                }
              else
                {
                  /*
                   * Include each item
                   */
                  loiCalls++;              
                }
            }

          lpstG99 = lpstG99->g_99_next;
        }
    }
  else
    {
      loiCalls = 0;
    }

  fovdPopFunctionName ();
  return loiCalls;
}

int foiGetBillMedium(struct s_TimmInter *inter) 
{
  int is_found, n, loiBillMedium;
  struct s_group_2 *g2;
  struct s_group_3 *g3;
  struct s_nad_seg *nad;
  struct s_rff_seg *rff;

  fovdPushFunctionName ("foiGetBillMedium");
  
  g2 = inter->timm->g_2;
  is_found = FALSE;
  while (g2) 
    {
      nad = g2->nad;
      if (EQ(nad->v_3035, "IV")) 
        {
          is_found = TRUE;
          break;
        }
      
      g2 = g2->g_2_next;
    }
  
  if (is_found == FALSE)
    {
      fovdPopFunctionName ();
      return -1;
    }
  
  g3 = g2->g_3;
  while (g3) 
    {
      rff = g3->rff;
      if (EQ(rff->v_1153, "LY")) 
        {
          is_found = TRUE;
          break;
        }

      g3 = g3->g_3_next;
    }
  
  if (is_found == TRUE) 
    {
      rff = g3->rff;
      n = sscanf(rff->v_1154, "%d", &loiBillMedium);
    }
  else
    {
      fovdPopFunctionName ();
      return -1;
    }
  
  fovdPopFunctionName ();
  return loiBillMedium;
}

/******************************************************************************************************
 *
 * FUNCTION: fovdSumSheet_CheckCallList
 *
 ******************************************************************************************************
 */

static struct s_group_99 *fpstNextCallSeq(struct s_group_99 *ppstG99, char *ppsnzCallNo, struct s_group_99 **ppstLastG99);
static toenBool foenEqSeqNo(struct s_group_99 *ppstG99, struct s_group_99 *ppstNextG99);
static toenBool foenEqCallDate(struct s_group_99 *ppstG99, struct s_group_99 *ppstNextG99);
static toenBool foenEqCallType(struct s_group_99 *ppstG99, struct s_group_99 *ppstNextG99);
static void fovdSetCallNo(struct s_group_99 *ppstG99, int poiCallno, int poiCallSubNo);
static void fovdSwapCalls(struct s_group_99 *ppstPrevG99, struct s_group_99 *ppstNextG99);

void fovdSumSheet_CheckCallList(stTIMM *ppstTimmInter)
{
  struct s_group_22 *lpstG22;
  struct s_group_99 *lpstG99, *lpstNextG99, *lpstPrevG99;
  
  if (ppstTimmInter == NULL || ppstTimmInter->timm == NULL)
    {
      return;
    }

  fovdPrintLog (LOG_TIMM, "Correcting all calls\n");
  lpstG22 = ppstTimmInter->timm->g_22;
  while (lpstG22)
    {
      /*
       * If call seq found in actual G22 block
       */
      
      if (lpstG22->g_99 != NULL)
        {
          lpstG99 = lpstG22->g_99;

          /*
           * For each call containing many connections
           */

          fovdPrintLog (LOG_DEBUG, "Call seq found\n");
          while (lpstG99) 
            {
              /*
               * Skip to the next call - sequence of connections
               */

              if ((lpstNextG99 = fpstNextCallSeq(lpstG99, lpstG99->xcd->v_X001, &lpstPrevG99)) != NULL)
                {
                  /*
                   * NULL may be returned if at the end of all calls
                   */
                  /*
                  fovdPrintLog (LOG_DEBUG, "Testing calls: %s %s with date: %s %s\n", 
                                lpstPrevG99->xcd->v_X001, lpstNextG99->xcd->v_X001,
                                lpstPrevG99->xcd->v_X004, lpstNextG99->xcd->v_X004);
		  */
                  if (foenEqSeqNo(lpstG99, lpstNextG99) == FALSE 
                      && foenEqCallDate(lpstPrevG99, lpstNextG99) == TRUE
                      && foenEqCallType(lpstPrevG99, lpstNextG99) == TRUE
                      && lpstPrevG99->xcd->v_X028[0] == 'V'
                      && lpstNextG99->xcd->v_X028[0] == 'V'
                      && lpstNextG99->xcd->v_X026[0] != 'O'
                      && lpstPrevG99->xcd->v_X019[0] == 'I')
                    {
                      /*
                       * This is the brink of the call but part of the call skipped to the previous call
                       */
                      
                      fovdPrintLog (LOG_CUSTOMER, "Correcting call with [type: %s%s%s, val: %s, date: %s]: %s.%s -> %d.%d\n", 
                                    lpstPrevG99->xcd->v_X019, lpstPrevG99->xcd->v_X026, lpstPrevG99->xcd->v_X028, 
                                    lpstPrevG99->xcd->v_5004, 
                                    lpstPrevG99->xcd->v_X004, 
                                    lpstPrevG99->xcd->v_X001, lpstPrevG99->xcd->v_X002,
                                    atoi(lpstNextG99->xcd->v_X001), atoi(lpstNextG99->xcd->v_X002) + 1);
                      
                      fovdSetCallNo(lpstPrevG99, atoi(lpstNextG99->xcd->v_X001), atoi(lpstNextG99->xcd->v_X002) + 1);
                      fovdSwapCalls(lpstPrevG99, lpstNextG99);
                    }
                }
              
              /*
               * This is the end of sequence of connections
               */

              lpstG99 = lpstNextG99;
            }
        }

      /*
       * Go to the next G22 item
       */

      lpstG22 = lpstG22->g_22_next;
    }
}

static struct s_group_99 *fpstNextCallSeq(struct s_group_99 *ppstG99, char *ppsnzCallNo, struct s_group_99 **ppstLastG99)
{
  struct s_group_99 *lpstG99;

  lpstG99 = ppstG99;
  while (lpstG99) 
    {
      if (NOT(EQ(lpstG99->xcd->v_X001, ppsnzCallNo)))
        {
          break;
        }
      
      *ppstLastG99 = lpstG99;
      lpstG99 = lpstG99->g_99_next;
    }

  return lpstG99;
}

static toenBool foenEqSeqNo(struct s_group_99 *ppstG99, struct s_group_99 *ppstNextG99)
{
  if (EQ(ppstG99->xcd->v_X001, ppstNextG99->xcd->v_X001))
    {
      return TRUE;
    }

  return FALSE;
}

static toenBool foenEqCallDate(struct s_group_99 *ppstG99, struct s_group_99 *ppstNextG99)
{
  if (EQ(ppstG99->xcd->v_X004, ppstNextG99->xcd->v_X004))
    {
      return TRUE;
    }

  return FALSE;
}

static toenBool foenEqCallType(struct s_group_99 *ppstG99, struct s_group_99 *ppstNextG99)
{
  /*
   * Unit of Measurement
   */

  if (NOT(EQ(ppstG99->xcd->v_6411, ppstNextG99->xcd->v_6411)))
    {
      return FALSE;
    }
  
  /*
   * Service
   */
  
  if (NOT(EQ(ppstG99->xcd->v_X013, ppstNextG99->xcd->v_X013)))
    {
      return FALSE;
    }

  /*
   * PLMN indicator
   */

  if (NOT(EQ(ppstG99->xcd->v_X028, ppstNextG99->xcd->v_X028)))
    {
      return FALSE;
    }

  return TRUE;
}

#define CALL_NO_BUFFER_LEN 64
#define TIMM_CALL_MAIN_NO_LEN 18
#define TIMM_CALL_SUBNO_LEN 18

static void fovdSetCallNo(struct s_group_99 *ppstG99, int poiCallNo, int poiCallSubNo)
{
  char lasnzCallNo[CALL_NO_BUFFER_LEN], lasnzCallSubNo[CALL_NO_BUFFER_LEN];
  
  sprintf(lasnzCallNo, "%d", poiCallNo);
  sprintf(lasnzCallSubNo, "%d", poiCallSubNo);
  
  strncpy(ppstG99->xcd->v_X001, lasnzCallNo, TIMM_CALL_MAIN_NO_LEN);
  strncpy(ppstG99->xcd->v_X002, lasnzCallNo, TIMM_CALL_SUBNO_LEN);
}

static void fovdSwapCalls(struct s_group_99 *ppstPrevG99, struct s_group_99 *ppstNextG99)
{
  struct s_xcd_seg *xcd;

  xcd = ppstPrevG99->xcd;
  ppstPrevG99->xcd = ppstNextG99->xcd;
  ppstNextG99->xcd = xcd;
}

/******************************************************************************************************
 *
 * FUNCTION: fovdBalSheet_CheckReverse
 *
 ******************************************************************************************************
 */

static struct s_group_22 *fpstSearchPaymentTransferPair(struct s_group_22 *ppstFirstG22, struct s_group_22 *ppstG22);
static struct s_group_22 *fpstSearchReversePair(struct s_group_22 *ppstFirstG22, struct s_group_22 *ppstG22);
static toenBool foenRemoveItemG22(struct s_group_22 **pppstG22, struct s_group_22 *ppstTmpG22);
static char *fpsnzTransxValue(struct s_group_22 *ppstTmpG22);
static char *fpsnzTransxDate(struct s_group_22 *ppstTmp);              
static char *fpsnzTransxRefNo(struct s_group_22 *ppstG22);
static char *fpsnzTransxType(struct s_group_22 *ppstG22);
static void fovdTransxSetType(struct s_group_22 *ppstG22, char *ppsnzOldType,  char *ppsnzNewType);
extern void free_g22 (struct s_group_22 *a_g22);
static int match(char *pattern, char *string);

void fovdBalSheet_CheckReverse(stTIMM *ppstTimmInter)
{
  struct s_group_22 *lpstG22, *lpstTmpG22;
  char *lpsnzValue, *lpsnzType, *lpsnzRefNo;
  double loflTmpValue, loflValue;
  int n;
  toenBool loenRemoved;

  /*
   * Check arguments
   */

  ASSERT(ppstTimmInter != NULL);
  ASSERT(ppstTimmInter->timm != NULL);

  /*
   * Find items to be deleted
   */

  fovdPrintLog (LOG_TIMM, "Correcting all transactions\n");
  lpstG22 = ppstTimmInter->timm->g_22;
  while (lpstG22)
    {
      /*
       * Get item properties
       */

      lpsnzType = fpsnzTransxType(lpstG22);
      ASSERT(lpsnzType != NULL);
      
      lpsnzValue = fpsnzTransxValue(lpstG22);
      ASSERT(lpsnzValue != NULL);
      n = sscanf(lpsnzValue, "%lf", &loflValue);
      ASSERT(n > 0);

      fovdPrintLog (LOG_DEBUG, "fovdBalSheet_CheckReverse: Checking item: %s, %s\n", lpsnzType, lpsnzValue);
      
      /*
       * Check rule: transaction is payment with value > 0.0
       */

      if (EQ("PAYMN", lpsnzType) && loflValue > 0.0)
        {
          fovdPrintLog (LOG_DEBUG, "fovdBalSheet_CheckReverse: Starting CI payment transfer correction\n");
          if ((lpstTmpG22 = fpstSearchPaymentTransferPair(ppstTimmInter->timm->g_22, lpstG22)) != NULL)
            {
              loenRemoved = foenRemoveItemG22(&(ppstTimmInter->timm->g_22), lpstG22);
              ASSERT(loenRemoved == TRUE);
              loenRemoved = foenRemoveItemG22(&(ppstTimmInter->timm->g_22), lpstTmpG22);
              ASSERT(loenRemoved == TRUE);
              lpstG22 = ppstTimmInter->timm->g_22;
            }
          else
            {
              lpstG22 = lpstG22->g_22_next;
            }
        }
      else if (EQ("OINVC", lpsnzType) && loflValue > 0.0)
        {
          fovdPrintLog (LOG_DEBUG, "fovdBalSheet_CheckReverse: Starting CI reverse correction\n");
          if ((lpstTmpG22 = fpstSearchReversePair(ppstTimmInter->timm->g_22, lpstG22)) != NULL)
            {
              loenRemoved = foenRemoveItemG22(&(ppstTimmInter->timm->g_22), lpstG22);
              ASSERT(loenRemoved == TRUE);
              loenRemoved = foenRemoveItemG22(&(ppstTimmInter->timm->g_22), lpstTmpG22);
              ASSERT(loenRemoved == TRUE);
              lpstG22 = ppstTimmInter->timm->g_22;
            }
          else
            {
              lpstG22 = lpstG22->g_22_next;
            }          
        }
      else
        {
          lpstG22 = lpstG22->g_22_next;
        }
    }

  /*
   * Handle new transactions: ININV, COINV
   */

  lpstG22 = ppstTimmInter->timm->g_22;
  while (lpstG22)
    {
      /*
       * Get item properties
       */
      
      lpsnzType = fpsnzTransxType(lpstG22);
      ASSERT(lpsnzType != NULL);
      
      if (EQ("OINVC", lpsnzType))
        {
          lpsnzRefNo = fpsnzTransxRefNo(lpstG22);
          ASSERT(lpsnzRefNo != NULL);
          
          if (memcmp(lpsnzRefNo, "CO", 2) == 0)
            {
              fovdTransxSetType(lpstG22, "OINVC", "COINV");
            }
          
          if (memcmp(lpsnzRefNo + 4, "/NL/", 4) == 0 && 
              isdigit(lpsnzRefNo[0]) && isdigit(lpsnzRefNo[1]) && isdigit(lpsnzRefNo[2]) && isdigit(lpsnzRefNo[3]))
            {
              fovdTransxSetType(lpstG22, "OINVC", "NLINV");
            }
          
          if (memcmp(lpsnzRefNo + 4, "/NP/", 4) == 0 &&
              isdigit(lpsnzRefNo[0]) && isdigit(lpsnzRefNo[1]) && isdigit(lpsnzRefNo[2]) && isdigit(lpsnzRefNo[3]))
            {
              fovdTransxSetType(lpstG22, "OINVC", "NPINV");
            }
        }
      
      lpstG22 = lpstG22->g_22_next;
    }
}

static struct s_group_22 *fpstSearchPaymentTransferPair(struct s_group_22 *ppstFirstG22, struct s_group_22 *ppstG22)
{
  struct s_group_22 *lpstG22;
  char *lpsnzValue, *lpsnzDate, *lpsnzTmpType, *lpsnzTmpValue, *lpsnzTmpDate;
  double loflTmpValue, loflValue;
  int n;

  lpsnzDate = fpsnzTransxDate(ppstG22);              
  ASSERT(lpsnzDate != NULL);
  
  lpsnzValue = fpsnzTransxValue(ppstG22);
  ASSERT(lpsnzValue != NULL);
  n = sscanf(lpsnzValue, "%lf", &loflValue);
  ASSERT(n > 0);

  fovdPrintLog (LOG_DEBUG, "fpstSearchReversePair: Look for the pair of: PAYMN, %s, %s\n", lpsnzDate, lpsnzValue);
  
  lpstG22 = ppstFirstG22;
  while (lpstG22)
    {
      lpsnzTmpType = fpsnzTransxType(lpstG22);
      ASSERT(lpsnzTmpType != NULL);
      
      if (EQ("PAYMN", lpsnzTmpType) && lpstG22 != ppstG22)
        {
          lpsnzTmpDate = fpsnzTransxDate(lpstG22);              
          ASSERT(lpsnzTmpDate != NULL);
          
          lpsnzTmpValue = fpsnzTransxValue(lpstG22);
          ASSERT(lpsnzTmpValue != NULL);
          n = sscanf(lpsnzTmpValue, "%lf", &loflTmpValue);
          ASSERT(n > 0);
          
          if (EQ(lpsnzTmpDate, lpsnzDate) && ((-1.0) * loflTmpValue) == loflValue)
            {
              /*
               * Bingo
               */

              return lpstG22;
            }
        }
 
      lpstG22 = lpstG22->g_22_next;
    }

  return NULL;
}

static struct s_group_22 *fpstSearchReversePair(struct s_group_22 *ppstFirstG22, struct s_group_22 *ppstG22)
{
  struct s_group_22 *lpstG22;
  char *lpsnzValue, *lpsnzDate, *lpsnzRefNo, *lpsnzTmpType, *lpsnzTmpValue, *lpsnzTmpDate, *lpsnzTmpRefNo;
  double loflTmpValue, loflValue;
  int n;
  
  lpsnzDate = fpsnzTransxDate(ppstG22);              
  ASSERT(lpsnzDate != NULL);
  
  lpsnzValue = fpsnzTransxValue(ppstG22);
  ASSERT(lpsnzValue != NULL);
  n = sscanf(lpsnzValue, "%lf", &loflValue);
  ASSERT(n > 0);
  
  lpsnzRefNo = fpsnzTransxRefNo(ppstG22);
  ASSERT(lpsnzRefNo != NULL);

  fovdPrintLog (LOG_DEBUG, "fpstSearchReversePair: Look for the pair of: OINVC, %s, %s, %s\n", lpsnzDate, lpsnzValue, lpsnzRefNo);

  lpstG22 = ppstFirstG22;
  while (lpstG22)
    {
      lpsnzTmpType = fpsnzTransxType(lpstG22);
      ASSERT(lpsnzTmpType != NULL);

      if (EQ("OINVC", lpsnzTmpType) && lpstG22 != ppstG22)
        {
          lpsnzTmpDate = fpsnzTransxDate(lpstG22);              
          ASSERT(lpsnzTmpDate != NULL);
          
          lpsnzTmpValue = fpsnzTransxValue(lpstG22);
          ASSERT(lpsnzTmpValue != NULL);
          n = sscanf(lpsnzTmpValue, "%lf", &loflTmpValue);
          ASSERT(n > 0);

          lpsnzTmpRefNo = fpsnzTransxRefNo(ppstG22);
          ASSERT(lpsnzTmpRefNo != NULL);
          
          fovdPrintLog (LOG_DEBUG, "fpstSearchReversePair: Check: OINVC, %s, %s, %s\n", lpsnzTmpDate, lpsnzTmpValue, lpsnzTmpRefNo);

          if (EQ(lpsnzRefNo, lpsnzTmpRefNo) && EQ(lpsnzTmpDate, lpsnzDate) && ((-1.0) * loflTmpValue) == loflValue)
            {
              /*
               * Bingo
               */

              fovdPrintLog (LOG_DEBUG, "fpstSearchReversePair: Bingo\n");
              return lpstG22;
            }
        }
 
      lpstG22 = lpstG22->g_22_next;
    }

  return NULL;
}

static char *fpsnzTransxValue(struct s_group_22 *ppstG22)
{
  struct s_moa_seg *moa;
  struct s_group_23 *g_23;
  
  g_23 = ppstG22->g_23;
  while (g_23)
    {
      moa = g_23->moa;
      if (EQ(moa->v_5025, "960")) 
        {
          return moa->v_5004;
        }
          
      g_23 = g_23->g_23_next;
    }
  
  return NULL;
}

static char *fpsnzTransxDate(struct s_group_22 *ppstG22)
{
  struct s_dtm_seg *dtm;

  dtm = ppstG22->dtm;
  while (dtm) 
    {
      if (EQ(dtm->v_2005, "900")) 
        {
          return dtm->v_2380;
        }
      
      dtm = dtm->dtm_next;
    }
  
  return NULL;
}

static char *fpsnzTransxRefNo(struct s_group_22 *ppstG22)
{
  struct s_rff_seg *rff;
  struct s_group_26 *g_26;
  
  g_26 = ppstG22->g_26;
  while (g_26)
    {
      rff = g_26->rff;
      if (EQ(rff->v_1153, "RF")) 
        {
          return rff->v_1154;
        }
      
      g_26 = g_26->g_26_next;
    }
  
  return NULL;
}

static char *fpsnzTransxType(struct s_group_22 *ppstG22)
{
  struct s_imd_seg *imd;
  
  imd = ppstG22->imd;
  while (imd) 
    {
      if (imd->v_7009 != NULL && strlen(imd->v_7009) > 0) 
        {
          return imd->v_7009;
        }
      
      imd = imd->imd_next;
    }

  return NULL;
}

static void fovdTransxSetType(struct s_group_22 *ppstG22, char *ppsnzOldType,  char *ppsnzNewType)
{
  struct s_imd_seg *imd;
  
  imd = ppstG22->imd;
  while (imd) 
    {
      if (EQ(imd->v_7009, ppsnzOldType))
        {
          memcpy(imd->v_7009, ppsnzNewType, 5);
        }
      
      imd = imd->imd_next;
    }  
}

static toenBool foenRemoveItemG22(struct s_group_22 **pppstG22, struct s_group_22 *ppstTmpG22)
{
  struct s_group_22 *lpstG22;

  lpstG22 = *pppstG22;
  if (lpstG22 != NULL && lpstG22 == ppstTmpG22)
    {
      *pppstG22 = ppstTmpG22->g_22_next;
      ppstTmpG22->g_22_next = NULL;
      free_g22(ppstTmpG22);
      return TRUE;
    }
  else 
    {
      return foenRemoveItemG22(&(lpstG22->g_22_next), ppstTmpG22);
    }

  return FALSE;
}

toenBool foenTimm_GetDiscAmount(stTIMM *ppstTimm,
                                double *ppflDiscAmount)
{
  struct s_moa_seg *moa;
  
  /* get MOA+920 from group 45 */
  
  if (NULL == (moa = fpstFindMainPaymentSegment(ppstTimm->timm->g_45, 
                                                "920",
                                                "9")))
    {
      return FALSE;
    }
  
  sscanf(moa->v_5004, "%lf", ppflDiscAmount);
  
  return TRUE;
}

struct s_moa_seg *fpstFindTaxPaymentSegment(struct s_group_47 *ppstG47, 
                                            char *ppsnzId, 
                                            char *ppsnzType) 
{
  struct s_moa_seg *lpstMoa;
  struct s_group_47 *lpstG47;
  
  lpstG47 = ppstG47;
  while (lpstG47)
    {
      lpstMoa = lpstG47->moa;
      if (EQ(lpstMoa->v_5025, ppsnzId) && EQ(lpstMoa->v_4405, ppsnzType))
        {
          return lpstMoa;
        }

      lpstG47 = lpstG47->g_47_next;
    }

  return NULL;
}
