#include <stdio.h>
#include <string.h>
#include <fnmatch.h>
#include <stdlib.h>


#include "bgh.h"
#include "protos.h"
#include "parser.h"

#include "types.h"
#include "gen.h"
#include "money.h"
#include "call_list.h"
#include "shdes2des.h"
#include "vpn.h"
#include "legend.h"

/*
 * extern variables
 */

extern double goflSimUsage_Local;
extern double goflSimUsage_Roaming;
extern double goflSimUsage_Surcharge;
extern int goiBillMedium;
extern stBGHGLOB       stBgh;                  /* structure with globals for BGH */

/*
 * static variables
 */

/*
 * Init in method Gen 
 * Updated in method FillRecord
 * Printed in method Gen after update
 */

static double doflOutAirPayment = 0.0;
static double doflOutIntPayment = 0.0;
static double doflOutSumPayment = 0.0;
static double doflOutRoamingLegPayment = 0.0;
static char szTemp[128];

static void fovdItemCall_FillRecord(tostItemCall *, tostCallRecord *);
static int fovdFormatDialedDigits(char *, int);
static toenBool foenItemCall_SuppressItem(struct s_xcd_seg *, tostItemCall *, char *);
static toenBool foenItemCall_Delete(tostItemCall *);
static tostItemCall *fpstItemCall_New(struct s_xcd_seg *);
static tostItemCallNode *fpstItemCallNode_New(struct s_group_99 *, int);

#ifdef US_TIMM_LAYOUT
static toenBool foenCommaSepString(char *ppszFirst,  char *ppszSecond, int poiOutLen, char *ppszOut);
#endif

#define _MIN_

/********************************************************************************************************************
 * 
 * METHOD: foenFormatDialedDigits
 *
 * DESCRIPTION: Number formating function using FIH specific rules
 *
 ********************************************************************************************************************
 */

#define SPN                            "501000000"
#define MAX_DIALED_DIGITS_LEN          32

static int fovdFormatDialedDigits(char *ppsnzDigits, int poiBufSize)
{
  static char lasnzTmp[MAX_DIALED_DIGITS_LEN];
  char *lpchDigitsSuffix;
  int loiSpecLen;

  fovdPushFunctionName("foenFormatDialedDigits");

  memset(lasnzTmp, 0, MAX_DIALED_DIGITS_LEN);

  if((ppsnzDigits != '\0'))    /* CR113 - special numbers 501999000 and 501999001 */
  {
      lpchDigitsSuffix = ppsnzDigits;
      if(0 == memcmp(ppsnzDigits,"0048",4))
      {
	  lpchDigitsSuffix = ppsnzDigits + 4;
      }
      else 
      {
	  if(0 == memcmp(ppsnzDigits,"48",2))
	  {
	      lpchDigitsSuffix = ppsnzDigits + 2;
	  }
      }
      if( ppsnzDigits != lpchDigitsSuffix ) 
      {
	  if( (loiSpecLen = strlen("501999000")) == strlen(lpchDigitsSuffix) ) 
	  {
	      if( (0 == memcmp(lpchDigitsSuffix, "501999000", loiSpecLen) ) ||
		  (0 == memcmp(lpchDigitsSuffix, "501999001", loiSpecLen) ) ) 
	      {
		  *ppsnzDigits = '\0';
		  return 1;
	      }
	  }
      }
  }

  if (memcmp(ppsnzDigits, "004890499999", strlen("004890499999")) == 0) /* CR066 */
    {
      strcpy(lasnzTmp, "90");
      strcat(lasnzTmp, ppsnzDigits + strlen("004890499999"));
      strcpy(ppsnzDigits, lasnzTmp);
      return 0;
    }

  if (memcmp(ppsnzDigits, "004890499998", strlen("004890499998")) == 0) /* CR066 */
    {
      strcpy(lasnzTmp, "501");
      strcat(lasnzTmp, ppsnzDigits + strlen("004890499998"));
      strcpy(ppsnzDigits, lasnzTmp);
      return 0;
    }
  
  if (memcmp(ppsnzDigits, "0501996666", strlen("0501996666")) == 0)    /* CR064 -  Personal Number */
    {
      if (memcmp(ppsnzDigits, "05019966660188", strlen("05019966660188")) == 0)
        {
          strcpy(lasnzTmp, "5");
        }
      
      strcat(lasnzTmp, ppsnzDigits + strlen("0501996666"));
      strcpy(ppsnzDigits, lasnzTmp);
      return 0;
    }

  /* 
   * CR ??? - FCC normalization 
   */

  if (fnmatch("0048*", ppsnzDigits, 0) == 0)
    {
      strcpy(lasnzTmp, ppsnzDigits + 4);
      strcpy(ppsnzDigits, lasnzTmp);
    }
  else if (ppsnzDigits[0] == '0')                     /* remove leading zero */
    {
      strcpy(lasnzTmp, ppsnzDigits + 1);
      strcpy(ppsnzDigits, lasnzTmp);
    }      

  /*
   * Special numbers
   */

  if (fnmatch("501998888*", ppsnzDigits, 0) == 0)     /* CR ??? */
    {
      strcpy(lasnzTmp, ppsnzDigits + 9);
      strcpy(ppsnzDigits, lasnzTmp);
    }
  else if (fnmatch(SPN "22", ppsnzDigits, 0) == 0)    /* CR ??? */
    {
      ppsnzDigits[0] = '*';
      strcpy(ppsnzDigits + 1, "22");
    }
  else if (fnmatch(SPN "123", ppsnzDigits, 0) == 0)   /* CR ??? */
    {
      ppsnzDigits[0] = '*';
      strcpy(ppsnzDigits+ 1, "122");
    }
  else if (fnmatch(SPN "100", ppsnzDigits, 0) == 0)   /* CR ??? */
    {
      ppsnzDigits[0] = '*';
      strcpy(ppsnzDigits + 1, "100");
    }
  else if (fnmatch(SPN "99", ppsnzDigits, 0) == 0)   /* CR ??? */
    {
      ppsnzDigits[0] = '*';
      strcpy(ppsnzDigits + 1, "99");
    }
  else if (fnmatch(SPN "501", ppsnzDigits, 0) == 0)  /* CR ??? */
    {
      ppsnzDigits[0] = '*';
      strcpy(ppsnzDigits + 1, "501");
    }  
  else if (fnmatch(SPN "98", ppsnzDigits, 0) == 0)   /* CR ??? */
    {
      ppsnzDigits[0] = '*';
      strcpy(ppsnzDigits + 1, "98");
    }
  else if (fnmatch(SPN "555", ppsnzDigits, 0) == 0)  /* CR ??? */
    {
      ppsnzDigits[0] = '*';
      strcpy(ppsnzDigits + 1, "555");
    }  
  else if (fnmatch(SPN "242", ppsnzDigits, 0) == 0)  /* CR ??? */
    {
      ppsnzDigits[0] = '*';
      strcpy(ppsnzDigits + 1, "242");
    }
  else if (fnmatch(SPN "888", ppsnzDigits, 0) == 0)  /* CR ??? */
    {
      ppsnzDigits[0] = '*';
      strcpy(ppsnzDigits + 1, "888");
    }
  else
    {
      /*
       * Digits string is ok
       */
    }

  fovdPopFunctionName();
  return 0;
}

/********************************************************************************************************************
 * 
 * METHOD: foenItemCallList_Update
 *
 * DESCRIPTION: 
 *
 ********************************************************************************************************************
 */

static toenBool foenItemCallList_Update(tostItemCallList *ppstList, tostItemCall *ppstItem)
{
  if (ppstItem->sochPLMN == 'V')
    {
      if (ppstItem->sochCall == 'I')
        {          
          ppstList->soiTAMTCCount++;
          ppstList->soflTAMTCAirPayment += ppstItem->soflLocalPayment;
          ppstList->soflTAMTCIntPayment += ppstItem->soflInterPayment  + ppstItem->soflTapNetPayment + ppstItem->soflTapTaxPayment;
          ppstList->soflTAMTCSumPayment = ppstList->soflTAMTCAirPayment + ppstList->soflTAMTCIntPayment;
        }
      else
        {
          ppstList->soiTAMOCCount++;
          ppstList->soflTAMOCAirPayment += ppstItem->soflLocalPayment;
          ppstList->soflTAMOCIntPayment += ppstItem->soflInterPayment  + ppstItem->soflTapNetPayment + ppstItem->soflTapTaxPayment;
          ppstList->soflTAMOCSumPayment = ppstList->soflTAMOCAirPayment + ppstList->soflTAMOCIntPayment;
        }

      ppstList->soflAirPayment += ppstItem->soflLocalPayment;
      ppstList->soflIntPayment += ppstItem->soflInterPayment + ppstItem->soflTapNetPayment + ppstItem->soflTapTaxPayment;
      ppstList->soflSumPayment = ppstList->soflAirPayment + ppstList->soflIntPayment;
      ppstList->soiCount++;     
    }
  else
    {
      ppstList->soflAirPayment += ppstItem->soflLocalPayment;
      ppstList->soflIntPayment += ppstItem->soflInterPayment + ppstItem->soflTapNetPayment + ppstItem->soflTapTaxPayment;
      ppstList->soflSumPayment = ppstList->soflAirPayment + ppstList->soflIntPayment;
      ppstList->soiCount++;
    }

  fovdPrintLog (LOG_DEBUG, "fpstItemCallList_Update: %lf, %lf\n", ppstList->soflAirPayment, ppstList->soflIntPayment);
  
  return TRUE;
}

/********************************************************************************************************************
 * 
 * METHOD: fpstItemCall_FillRecord
 *
 * DESCRIPTION: Formating function: item -> record
 *
 ********************************************************************************************************************
 */

static void fovdItemCall_FillRecord(tostItemCall *ppstItem, tostCallRecord *ppstRecord)
{
  toenBool loenStatus;
  double loflPayment;
  char *ptr;
  int n, i, loiZonePatternLen;
  fovdPushFunctionName("foenItemCall_FillRecord");
  
  loiZonePatternLen = strlen(VPN_ZONE_PATTERN);
  fovdPrintLog (LOG_DEBUG, "fpstItemCall_FillRecord: Filling record with call item\n");
  memset(ppstRecord, 0, sizeof(tostCallRecord));

  /*
   * Call time info
   */

  strcpy(ppstRecord->sasnzDate, ppstItem->sasnzTime);
  fovdFormatCallDate(ppstRecord->sasnzDate);
  fovdPrintLog (LOG_DEBUG, "fpstItemCall_FillRecord: Date     : %s\n", ppstRecord->sasnzDate);

  strcpy(ppstRecord->sasnzTime, ppstItem->sasnzTime);
  fovdFormatCallTime(ppstRecord->sasnzTime);
  fovdPrintLog (LOG_DEBUG, "fpstItemCall_FillRecord: Time     : %s\n", ppstRecord->sasnzTime);

#ifdef US_TIMM_LAYOUT
  strcpy(ppstRecord->soszTimeZone, ppstItem->soszTimeZoneShdes);
  fovdPrintLog (LOG_DEBUG, "fpstItemCall_FillRecord: Time zone : %s\n", ppstRecord->soszTimeZone);
#endif

  /*
   * Call volume rounding rules - duration is 0 when SMS 
   */

  if (ppstItem->soiDuration == 0)
    {
      strcpy(ppstRecord->sasnzDuration, "1");
    }
  else
    {
#ifdef _ROUND_CALL_DURATION_
      if (ppstItem->soiDuration % 60 != 0)
        {
          /*
           * Use BGH rounding rule from CR 31
           */
          
          if (ppstItem->soiDuration % 60 != 0)
            {
              sprintf(ppstRecord->sasnzDuration, "%d", (ppstItem->soiDuration / 60) + 1);
            }
          else
            {
              sprintf(ppstRecord->sasnzDuration, "%d", ppstItem->soiDuration / 60);
            }
        }
      else
        {
          sprintf(ppstRecord->sasnzDuration, "%d", ppstItem->soiDuration / 60);
        }
#else
      sprintf(ppstRecord->sasnzDuration, "%d", ppstItem->soiDuration);
#endif
    }
  
  fovdPrintLog (LOG_DEBUG, "fpstItemCall_FillRecord: Duration : %s\n", ppstRecord->sasnzDuration);
  
  /*
   * Call type: seq of AI or A or C
   */
  
  strncpy(ppstRecord->sasnzCallType, ppstItem->sasnzCallType, MAX_BUFFER);
  fovdPrintLog (LOG_DEBUG, "fpstItemCall_FillRecord: Call type: %s\n", ppstRecord->sasnzCallType);  

#ifdef US_TIMM_LAYOUT
  /*
   * Call origin info
   */
  strncpy(ppstRecord->soszOriginLoc, ppstItem-> soszOriginLocation, MAX_BUFFER);
  fovdPrintLog (LOG_DEBUG, "fpstItemCall_FillRecord: Origin location: %s\n", ppstRecord->soszOriginLoc);
#endif

  /*
   * Call dest. info
   */

  strncpy(ppstRecord->sasnzDirection, ppstItem->sasnzDestZoneDes, MAX_BUFFER);
  fovdPrintLog (LOG_DEBUG, "fpstItemCall_FillRecord: Direction: %s\n", ppstRecord->sasnzDirection);

#ifdef US_TIMM_LAYOUT
  strncpy(ppstRecord->soszDestinLoc, ppstItem-> soszDestinLocation, MAX_BUFFER);
  fovdPrintLog (LOG_DEBUG, "fpstItemCall_FillRecord: Destination location: %s\n", ppstRecord->soszDestinLoc);
#endif

  /*
   * Special recoding rule for DN
   */

  strncpy(ppstRecord->sasnzDirectoryNumber, ppstItem->sasnzDirectoryNumber, MAX_BUFFER);
  if( 1 == fovdFormatDialedDigits(ppstRecord->sasnzDirectoryNumber, MAX_BUFFER))
  {
        /*
         * change the Direction if CR113 is in efect
         */
      strcpy(ppstRecord->sasnzDirection,"Koszt po\263" ". przych.");
      fovdPrintLog (LOG_DEBUG, "fpstItemCall_FillRecord: Direction changed due to CR113: %s\n", 
		    ppstRecord->sasnzDirection);
      fovdPrintLog (LOG_DEBUG, "fpstItemCall_FillRecord: Dialed digits wiped due to CR113\n");
  }
  fovdPrintLog (LOG_DEBUG, "fpstItemCall_FillRecord: Dialed   : %s\n", ppstRecord->sasnzDirectoryNumber);

  /*
   * setup of service name
   */ 

  if (ppstItem->sochPLMN == 'V' && ppstItem->sochCall == 'I' && (ppstItem->sochRLeg == 'R' || ppstItem->soflTapNetPayment == 0.0))
    {
      /*
       * RCF if source is VPLMN, call is Inbound, has RLeg or TAP rate is 0.0 when no RLeg defined
       */
      
      strcpy(ppstRecord->sasnzServiceShdes, "ODEBR");
    }
  else if (ppstItem->sochPLMN == 'V' && ppstItem->sochCall == 'I' && ppstItem->soflTapNetPayment > 0.0)
    {
      /*
       * TA-MTC
       */

      strcpy(ppstRecord->sasnzServiceShdes, "ODEBR*");
    }
  else
    {
      strncpy(ppstRecord->sasnzServiceShdes, ppstItem->sasnzServiceShdes, MAX_BUFFER);

      /*
       * CF Service must have a special label
       */
      
      if (ppstItem->sasnzCallType[0] == 'C' || ppstItem->sasnzCallType[1] == 'C')
        {
          strcat(ppstRecord->sasnzServiceShdes, ">");
        }
    }

  fovdPrintLog (LOG_DEBUG, "fpstItemCall_FillRecord: Service  : %s\n", ppstRecord->sasnzServiceShdes);

  /*
   * Payment info: not strict checking
   */

  fovdStr_Clean(ppstItem->sasnzPLMN);
  ptr = fpsnzMapVPLMNShdes2Country(ppstItem->sasnzPLMN);
  if (ptr == NULL)
    {
      ptr = ppstItem->sasnzPLMN;
    }

  strncpy(ppstRecord->sasnzPLMN, ptr, MAX_BUFFER);

  fovdPrintLog (LOG_DEBUG, "fpstItemCall_FillRecord: PLMN     : %s\n", ppstRecord->sasnzPLMN);
  if (ppstItem->sasnzPLMN[0] == '\0')
    {
      /*
       * Payment section: PTK = Air, CF, Rleg, OTHER = Incterconnect
       */
      
      fovdMoney_Sprintf(ppstRecord->sasnzLocalPayment, ppstItem->soflLocalPayment);
      fovdMoney_Sprintf(ppstRecord->sasnzInterPayment, ppstItem->soflInterPayment);
      loflPayment = foflMoney_Add(ppstItem->soflLocalPayment, ppstItem->soflInterPayment);
      fovdMoney_Sprintf(ppstRecord->sasnzSummaryPayment, loflPayment);
      
      /*
       * Summary items
       */
     
      doflOutAirPayment = foflMoney_Add(doflOutAirPayment, ppstItem->soflLocalPayment);
      doflOutIntPayment = foflMoney_Add(doflOutIntPayment, ppstItem->soflInterPayment);      
      doflOutSumPayment = foflMoney_Add(doflOutSumPayment, loflPayment);      
    }
  else
    {
      /*
       * Payment section: PTK = 0, OTHER = Tap Net + Tap Tax
       */
      
      /*
       * Internal payment
       */

      fovdMoney_Sprintf(ppstRecord->sasnzLocalPayment, ppstItem->soflLocalPayment);
      doflOutAirPayment += ppstItem->soflLocalPayment;

      /*
       * External payment
       */

      loflPayment = foflMoney_Add(ppstItem->soflTapNetPayment, ppstItem->soflTapTaxPayment);
      loflPayment = foflMoney_Add(loflPayment, ppstItem->soflInterPayment);
      fovdMoney_Sprintf(ppstRecord->sasnzInterPayment, loflPayment);
      doflOutIntPayment += loflPayment;

      /*
       * Summary payment
       */

      loflPayment = foflMoney_Add(loflPayment, ppstItem->soflLocalPayment);
      fovdMoney_Sprintf(ppstRecord->sasnzSummaryPayment, loflPayment);
      doflOutSumPayment += loflPayment;
    }
  
  fovdFormatMoney(ppstRecord->sasnzLocalPayment);
  fovdFormatMoney(ppstRecord->sasnzInterPayment);
  fovdFormatMoney(ppstRecord->sasnzSummaryPayment);
  
  fovdPrintLog (LOG_DEBUG, "fpstItemCall_FillRecord: Air      : %s\n", ppstRecord->sasnzLocalPayment);
  fovdPrintLog (LOG_DEBUG, "fpstItemCall_FillRecord: Interc   : %s\n", ppstRecord->sasnzInterPayment);
  fovdPrintLog (LOG_DEBUG, "fpstItemCall_FillRecord: Sum      : %s\n", ppstRecord->sasnzSummaryPayment);
  fovdPrintLog (LOG_DEBUG, "fpstItemCall_FillRecord: Part Sum : %.2lf %.2lf %.2lf\n", 
                doflOutAirPayment, doflOutIntPayment, doflOutSumPayment);
  ASSERT( MAX_BUFFER > strlen(ppstItem->sasnzCurrency) + loiZonePatternLen -1);
  if(TRUE == ppstItem->soenIsDestZoneVPN)
  {
      fovdPrintLog (LOG_DEBUG, "fpstItemCall_FillRecord: destination zone is VPN\n");
      strncpy(ppstRecord->sasnzCurrency, VPN_ZONE_PATTERN, loiZonePatternLen-1);
      strncat(ppstRecord->sasnzCurrency, ppstItem->sasnzCurrency, MAX_BUFFER);
  }
  else
  {
      strncpy(ppstRecord->sasnzCurrency, ppstItem->sasnzCurrency, MAX_BUFFER);
  }    
  fovdPrintLog (LOG_DEBUG, "fpstItemCall_FillRecord: Currency : %s\n", ppstRecord->sasnzCurrency);

  fovdPopFunctionName();
}


/********************************************************************************************************************
 * 
 * METHOD: foenItemCall_SuppressItem
 *
 * DESCRIPTION: Number formating function using FIH specific rules
 *
 ********************************************************************************************************************
 */

#define NUMBER_MAX_LEN 32

static toenBool foenItemCall_SuppressItem(struct s_xcd_seg *xcd, tostItemCall *ppstItem, char *ppsnzNumber)
{
  double loflHPLMNPayment, loflVPLMNPayment, loflPayment;
  char lasnzNumber[NUMBER_MAX_LEN];
  int off;

  loflVPLMNPayment = ppstItem->soflTapNetPayment + ppstItem->soflTapTaxPayment;
  loflHPLMNPayment = ppstItem->soflLocalPayment + ppstItem->soflInterPayment;
  loflPayment = loflHPLMNPayment + loflVPLMNPayment;
  
  /*
   * Don't show free CF calls to Voice Mail
   */

  /*
  strcpy(lasnzNumber, "004850188");
  strncat(lasnzNumber, ppsnzNumber + 3);
  if (EQ(ppstItem->sasnzDirectoryNumber, lasnzNumber) &&  ppstItem->sochRLeg == 'C' && foenMoney_Eq(loflPayment, 0.0) == TRUE)
    {
      return TRUE;
    }
  */

  /*
   * if EVENT && val = 0 then suppress
   */
  
  if (EQ(ppstItem->sasnzUnit, "Ev") && foenMoney_Eq(loflPayment, 0.0) == TRUE)
    {
      return TRUE;
    }

  /*
   * SMS have Duration = 0, all of them should be shown
   */
  
  if (EQ(ppstItem->sasnzUnit, "Msg"))
    {
      return FALSE;
    }  
  
  /*
   * if CALL && rounded_time_befor_disc = 0 then suppress 
   */

  /*
  if (ppstItem->soiDuration == 0)
    {
      return TRUE;
    }
  */

  if (loflPayment == 0.0 && ppstItem->soiDuration == 0)
    {
      return TRUE;
    }

  return FALSE;
}

/********************************************************************************************************************
 * 
 * METHOD: fpstItemCall_Delete
 *
 * DESCRIPTION: Delete item - only free it
 *
 ********************************************************************************************************************
 */

static toenBool foenItemCall_Delete(tostItemCall *ppstItem)
{
  fovdPushFunctionName("foenItemCall_Delete");

  free(ppstItem);
  
  fovdPopFunctionName();
  return TRUE;
}

/********************************************************************************************************************
 * 
 * METHOD: fpstItemCall_New
 *
 * DESCRIPTION: New item from XCD segment
 *
 ********************************************************************************************************************
 */

static tostItemCall *fpstItemCall_New(struct s_xcd_seg *ppstXCD)
{
  tostItemCall *lpstItem;
  int loiDuration;

  fovdPushFunctionName("foenItemCall_New");
  
  fovdPrintLog (LOG_DEBUG, "fpstItemCall_New: Creating call item\n");
  
  lpstItem = (tostItemCall *)calloc(1, sizeof(tostItemCall));
  if (lpstItem == NULL)
    {
      fovdPopFunctionName();
      return NULL;
    }
  
  /*
   * Fill item structure
   */

  if (ppstXCD->v_X028[0] == 'V' && ppstXCD->v_X026[0] != 'O')
    {
      lpstItem->sochCall = 'I';
    }
  else
    {
      lpstItem->sochCall = ppstXCD->v_X026[0];
    }
  
  strncpy(lpstItem->sasnzServiceShdes, ppstXCD->v_X013, SHDES_SIZE); 
  strncpy(lpstItem->sasnzUnit, ppstXCD->v_6411, UNIT_SIZE);
  strncpy(lpstItem->sasnzTime, ppstXCD->v_X004, TIME_SIZE);
  strncpy(lpstItem->sasnzDirectoryNumber, ppstXCD->v_X043, DIRECTORY_NUMBER_SIZE);
  strncpy(lpstItem->sasnzDestZoneDes, ppstXCD->v_X045, DEST_ZONE_SIZE);

  /*
   * Info
   */

  /*
    strncpy(lpstItem->sasnzCurrency, ppstXCD->v_6345c, CURRENCY_SIZE);
  */

  lpstItem->sasnzCurrency[0] = ppstXCD->v_X021[0];
  lpstItem->sasnzCurrency[1] = '\0';
  strcat(lpstItem->sasnzCurrency, ppstXCD->v_X016);


  /*
   * Roaming info
   */

  lpstItem->sochPLMN = ppstXCD->v_X028[0];
  if (ppstXCD->v_X028[0] == 'V')
    {
      strncpy(lpstItem->sasnzPLMN, ppstXCD->v_X029, SHDES_SIZE);
    }
  else
    {
      lpstItem->sasnzPLMN[0] = '\0';
    }

  fovdPrintLog (LOG_DEBUG, "fpstItemCall_New: Service        : %s\n", lpstItem->sasnzServiceShdes);  
  fovdPrintLog (LOG_DEBUG, "fpstItemCall_New: Unit           : %s\n", lpstItem->sasnzUnit);  
  fovdPrintLog (LOG_DEBUG, "fpstItemCall_New: Time           : %s\n", lpstItem->sasnzTime);  
  fovdPrintLog (LOG_DEBUG, "fpstItemCall_New: PLMN           : %s\n", lpstItem->sasnzPLMN);  
  fovdPrintLog (LOG_DEBUG, "fpstItemCall_New: Number         : %s\n", lpstItem->sasnzDirectoryNumber);  
  fovdPrintLog (LOG_DEBUG, "fpstItemCall_New: Zone           : %s\n", lpstItem->sasnzDestZoneDes);  
  fovdPrintLog (LOG_DEBUG, "fpstItemCall_New: Type           : %c\n", ppstXCD->v_X019[0]);  
  fovdPrintLog (LOG_DEBUG, "fpstItemCall_New: Tariff time    : %s\n", ppstXCD->v_X014);  

#ifdef US_TIMM_LAYOUT
  fovdPrintLog (LOG_DEBUG, "fpstItemCall_New: Origin city    : %s\n", ppstXCD->v_X030);  
  fovdPrintLog (LOG_DEBUG, "fpstItemCall_New: Origin state   : %s\n", ppstXCD->v_X031);  
  fovdPrintLog (LOG_DEBUG, "fpstItemCall_New: Destin city    : %s\n", ppstXCD->v_X033);  
  fovdPrintLog (LOG_DEBUG, "fpstItemCall_New: Destin state   : %s\n", ppstXCD->v_X034);  
  /*
   * Origin & Destination Location info
   */

  *(lpstItem->soszOriginLocation) = '\0';      
  *(lpstItem->soszDestinLocation) = '\0';      
  if (ppstXCD->v_X019[0] != 'I')
  {
      foenCommaSepString(ppstXCD->v_X030, ppstXCD->v_X031, LOCATION_SIZE, lpstItem->soszOriginLocation);      
      foenCommaSepString(ppstXCD->v_X033, ppstXCD->v_X034, LOCATION_SIZE, lpstItem->soszDestinLocation);      
  }
  ASSERT(strlen((ppstXCD->v_X014)) < TARIFF_TIME_SHDES_LEN );
  strcpy(lpstItem->soszTimeZoneShdes,ppstXCD->v_X014);
#endif /* US_TIMM_LAYOUT */

  /*
   * Load payment info
   */

  /*
   * Local payment
   */
  
  if (ppstXCD->v_X019[0] == 'I')
    {
      /*
       * Payment type: Interconnect
       */
      
      sscanf(ppstXCD->v_5004, "%lf", &(lpstItem->soflInterPayment));
    }
  else 
    {
      /*
       * Payment type: Air, Call Forwarding, Roaming leg
       */
      
      sscanf(ppstXCD->v_5004, "%lf", &(lpstItem->soflLocalPayment));      
    }

  lpstItem->sochRLeg = ppstXCD->v_X019[0];

  /*
   * Roaming payment - if D2 strategy selected
   */
  
  if (ppstXCD->v_X028[0] == 'V')
    {      
      sscanf(ppstXCD->v_5004a, "%lf", &(lpstItem->soflTapNetPayment));
      sscanf(ppstXCD->v_5004b, "%lf", &(lpstItem->soflTapTaxPayment));
    }
  else
    {
      lpstItem->soflTapNetPayment = 0.0;
      lpstItem->soflTapTaxPayment = 0.0;
    }
  
  fovdPrintLog (LOG_DEBUG, "fpstItemCall_New: Local Payment  : %lf\n", lpstItem->soflLocalPayment);  
  fovdPrintLog (LOG_DEBUG, "fpstItemCall_New: Inter Payment  : %lf\n", lpstItem->soflInterPayment);  
  fovdPrintLog (LOG_DEBUG, "fpstItemCall_New: Tap Net Payment: %lf\n", lpstItem->soflTapNetPayment);  
  fovdPrintLog (LOG_DEBUG, "fpstItemCall_New: Tap Tax Payment: %lf\n", lpstItem->soflTapTaxPayment);  
  
  /*
   * If FUM used then must be volume before disc. used
   */
  
  if (ppstXCD->v_X021[0] == 'F' || ppstXCD->v_X021[0] == 'P')     
    {
      loiDuration = atoi(ppstXCD->v_X009);
    }
  else
    {      
      loiDuration = atoi(ppstXCD->v_X010);
    }

  /*
   * Strange roaming SMS have had duration 0
   */

  /*
  if (loiDuration == 0)
    {
      loiDuration = 1;
    }
  */

  lpstItem->soiDuration = loiDuration;
  fovdPrintLog (LOG_DEBUG, "fpstItemCall_New: Duration       : %d (%s, %s)\n", 
                lpstItem->soiDuration, ppstXCD->v_X009, ppstXCD->v_X010);  

  /* VPN destination zone */
  fovdPrintLog (LOG_DEBUG, "fpstItemCall_New: Destination zone: %s\n",ppstXCD->v_X015);
  if(0 == fnmatch(VPN_ZONE_PATTERN, ppstXCD->v_X015, FNM_PERIOD))
  {
      lpstItem->soenIsDestZoneVPN = TRUE;
  }    
  else
  {
      lpstItem->soenIsDestZoneVPN = FALSE;
  }    
 
  fovdPopFunctionName();
  return lpstItem;
}

/********************************************************************************************************************
 * 
 * METHOD: fpstItemCallNode_New
 *
 * DESCRIPTION: For a subsequence of XCD records create one call item.
 *
 ********************************************************************************************************************
 */

static tostItemCallNode *fpstItemCallNode_New(struct s_group_99 *ppstG99, int poiNumber)
{
  int i, loiDuration, n;
  struct s_group_99 *lpstG99;
  struct s_xcd_seg *lpstXCD;
  tostItemCall *lpstItem;
  tostItemCallNode *lpstNode;
  double loflPayment;

  fovdPushFunctionName("foenItemCallNode_New");

  fovdPrintLog (LOG_DEBUG, "fpstItemCallNode_New: Creating call list node for XCD no: %d\n", poiNumber);

  i = 0;
  lpstG99 = ppstG99;
  while (lpstG99)
  {
      if (poiNumber == atoi(lpstG99->xcd->v_X001))
      {
          fovdPrintLog (LOG_DEBUG, "fpstItemCallNode_New: Using XCD: %s %s\n", lpstG99->xcd->v_X001, lpstG99->xcd->v_X002);
          
          /*
           * Sum values for items from one subsequence with a given call number
           */

          lpstXCD = lpstG99->xcd;
          if (i == 0)
            {
              fovdPrintLog (LOG_DEBUG, "fpstItemCallNode_New: Seq XCD: %s start\n", lpstG99->xcd->v_X001);
              
              /*
               * Create new item
               */
              
              lpstItem = fpstItemCall_New(lpstXCD);
              if (lpstItem == NULL)
                {
                  fovdPopFunctionName();
                  return NULL;
                }
            }
          else
            {              
              fovdPrintLog (LOG_DEBUG, "fpstItemCallNode_New: Seq XCD: %s item no: %d\n", lpstG99->xcd->v_X001, i);
              
              /*
               * Load payment info for the next part of the call
               */
              
              /*
               * Local payment
               */
              
              fovdPrintLog (LOG_DEBUG, "fpstItemCallNode_New: Loading payment\n");
              if (lpstXCD->v_X019[0] == 'I')
                {
                  /*
                   * Payment type: Interconnect
                   */
                  
                  n = sscanf(lpstXCD->v_5004, "%lf", &loflPayment);
                  ASSERT(n == 1);
                  lpstItem->soflInterPayment = foflMoney_Add(lpstItem->soflInterPayment, loflPayment);
                  fovdPrintLog (LOG_DEBUG, "fpstItemCallNode_New: Land payment: %lf\n", lpstItem->soflInterPayment);
                }
              else 
                {
                  /*
                   * Payment type: Air, Call Forwarding, Roaming leg
                   */

                  n = sscanf(lpstXCD->v_5004, "%lf", &loflPayment);
                  ASSERT(n == 1);
                  lpstItem->soflLocalPayment = foflMoney_Add(lpstItem->soflLocalPayment, loflPayment);
                  fovdPrintLog (LOG_DEBUG, "fpstItemCallNode_New: Local payment: %lf\n", lpstItem->soflInterPayment);

#ifdef US_TIMM_LAYOUT
                  /*
                   * Origin & Destination Location info update
                   */

                  if( '\0' == *(lpstItem->soszOriginLocation) && '\0' == *(lpstItem->soszDestinLocation) )
                  {
                      foenCommaSepString(lpstXCD->v_X030, lpstXCD->v_X031, LOCATION_SIZE, lpstItem->soszOriginLocation);      
                      foenCommaSepString(lpstXCD->v_X033, lpstXCD->v_X034, LOCATION_SIZE, lpstItem->soszDestinLocation);      
                  }
#endif /* US_TIMM_LAYOUT */

                  
                }

              /*
               * VPLMN payment
               */

              fovdPrintLog (LOG_DEBUG, "fpstItemCallNode_New: Loading tap payment\n");
              if (lpstXCD->v_X028[0] == 'V')
                {
                  /*
                   * Roaming payment - if D2 strategy selected
                   */
                  
                  n = sscanf(lpstXCD->v_5004a, "%lf", &loflPayment);
                  ASSERT(n == 1);
                  lpstItem->soflTapNetPayment = foflMoney_Add(lpstItem->soflTapNetPayment, loflPayment);
                  fovdPrintLog (LOG_DEBUG, "fpstItemCallNode_New: Tap net payment: %lf\n", lpstItem->soflTapNetPayment);
                  
                  n = sscanf(lpstXCD->v_5004b, "%lf", &loflPayment);
                  ASSERT(n == 1);
                  lpstItem->soflTapTaxPayment = foflMoney_Add(lpstItem->soflTapTaxPayment, loflPayment);
                  fovdPrintLog (LOG_DEBUG, "fpstItemCallNode_New: Tap tax payment: %lf\n", lpstItem->soflTapTaxPayment);
                }
            }

          /*
           * XCD seq no
           */

          lpstItem->sasnzCallType[i++] = lpstXCD->v_X019[0];
          if(CALL_SEQ_SIZE/4 <= i)
          {
             fovdPrintLog (LOG_CUSTOMER, "fpstItemCallNode_New: Number of XCD subitems large: %d\n",i);
          }
          ASSERT("Number of XCD subitems large" && i < CALL_SEQ_SIZE);
      }
      else
      {
          fovdPrintLog (LOG_DEBUG, "fpstItemCallNode_New: End of seq for XCD: %s\n", lpstG99->xcd->v_X001);
          break;
      }

      lpstG99 = lpstG99->g_99_next;
    }

  /*
   * End of call sequence like AI or A or CI or C or AIA or IR
   */

  lpstItem->sasnzCallType[i + 1] = '\0';
  fovdPrintLog (LOG_DEBUG, "fpstItemCallNode_New: Item type: %s\n", lpstItem->sasnzCallType);

  /*
   * New node for new item
   */

  fovdPrintLog (LOG_DEBUG, "fpstItemCallNode_New: Creating call list node\n");
  if ((lpstNode = (tostItemCallNode *)calloc(1, sizeof(tostItemCallNode))) == NULL)
    {
      fovdPopFunctionName();
      return NULL;
    }

  lpstNode->spstItem = lpstItem;

  fovdPrintLog (LOG_DEBUG, "fpstItemCallNode_New: Call list node created\n");

  fovdPopFunctionName();
  return lpstNode;
}

/********************************************************************************************************************
 *
 * METHOD: fpstItemCallList_Append
 *
 * DESCRIPTIN: 
 *
 ********************************************************************************************************************
 */

toenBool foenItemCallList_Append(tostItemCallList *ppstList, tostItemCallNode *ppstNode)
{
  toenBool loenStatus;

  fovdPushFunctionName("foenItemCallList_Append");

  fovdPrintLog (LOG_DEBUG, "fpstItemCallList_Append: Appending item node\n");
  if (ppstList->spstRoot == NULL)
    {
      fovdPrintLog (LOG_DEBUG, "fpstItemCallList_Append: First node of the list\n");
      ppstList->spstLastNode = ppstNode;
      ppstList->spstRoot = ppstNode;
    }
  else
    {
      fovdPrintLog (LOG_DEBUG, "fpstItemCallList_Append: Next node of the list\n");
      ppstList->spstLastNode->spstNext = ppstNode;
      ppstList->spstLastNode = ppstNode;
    }
  
  if ((loenStatus = foenItemCallList_Update(ppstList, ppstNode->spstItem)) == FALSE)
    {
      fovdPopFunctionName();
      return FALSE;
    }

  fovdPrintLog (LOG_DEBUG, "fpstItemCallList_Append: List item count: %d\n", ppstList->soiCount);

  fovdPopFunctionName();
  return TRUE;
}

/********************************************************************************************************************
 *
 * METHOD: fpstItemCallList_New
 *
 * DESCRIPTIN: Insert each subsequence of XCD records to a list of call items.
 *
 ********************************************************************************************************************
 */

#define XCD_SUBSEQ_START 0

tostItemCallList *fpstItemCallList_New(struct s_group_99 *ppstG99, toenPLMNType poenType, char *ppsnzNumber)
{
  tostItemCallList *lpstList;
  tostItemCallNode *lpstNode;
  struct s_group_99 *lpstG99;
  char pochType;
  toenBool loenStatus;

  fovdPushFunctionName("foenItemCallList_New");
  
  fovdPrintLog (LOG_DEBUG, "fpstItemCallList_New: Creating call item list\n");

  if ((lpstList = (tostItemCallList *)calloc(1, sizeof (tostItemCallList))) == NULL)
    {
      sprintf (szTemp, "fpstItemCallList_New: Can't alloc memory\n");
      macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);                    
      fovdPopFunctionName();
      return NULL;
    }

  lpstList->soiTAMTCCount = 0;
  lpstList->soflTAMTCAirPayment = 0.0;
  lpstList->soflTAMTCIntPayment = 0.0;
  lpstList->soflTAMTCSumPayment = 0.0; 

  lpstList->soiTAMOCCount = 0;
  lpstList->soflTAMOCAirPayment = 0.0;
  lpstList->soflTAMOCIntPayment = 0.0;
  lpstList->soflTAMOCSumPayment = 0.0;

  lpstList->soflAirPayment = 0.0;
  lpstList->soflIntPayment = 0.0;
  lpstList->soflSumPayment = 0.0;
  lpstList->soiCount = 0;

  fovdPrintLog (LOG_DEBUG, "fpstItemCallList_New: New call list created\n");

  /*
   * Define the type of XCD record to be processed
   */

  if (poenType == HPLMN_TYPE)
    {
      pochType = 'H';
    }
  else
    {
      pochType = 'V';
    }

  fovdPrintLog (LOG_DEBUG, "fpstItemCallList_New: Type of list: %c\n", pochType);

  /*
   * Init list
   */

  lpstList->spstRoot = NULL;
  lpstList->soenType = poenType;

  /*
   * For each sequence of items
   */

  fovdPrintLog (LOG_DEBUG, "fpstItemCallList_New: Start XCD list loading\n");
  lpstG99 = ppstG99;
  while (lpstG99)
    { 
      if (atoi(lpstG99->xcd->v_X002) == XCD_SUBSEQ_START && lpstG99->xcd->v_X028[0] == pochType)
        {
          fovdPrintLog (LOG_DEBUG, "fpstItemCallList_New: XCD: %s %s\n", lpstG99->xcd->v_X001, lpstG99->xcd->v_X002);

          /*
           * Filter rule: 
           * New node with item created from the new sequence of XCD records
           * beginning with running sub number and with specified PLMN type 
           */

          if ((lpstNode = fpstItemCallNode_New(lpstG99, atoi(lpstG99->xcd->v_X001))) == NULL)
            {
              sprintf (szTemp, "fpstItemCallList_New: Can't create new call list node\n");
              macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);              
              fovdPopFunctionName();
              return NULL;
            }
          
          fovdPrintLog (LOG_DEBUG, "fpstItemCallList_New: seq XCD: %s %s in a list node\n", 
                        lpstG99->xcd->v_X001, lpstG99->xcd->v_X002);
          
          /*
           * Link new node to the end of a list
           */
          
          if (foenItemCall_SuppressItem(lpstG99->xcd, lpstNode->spstItem, ppsnzNumber) == FALSE)
            { 
              fovdPrintLog (LOG_DEBUG, "fpstItemCallList_New: XCD: %s %s accepted\n", 
                            lpstG99->xcd->v_X001, lpstG99->xcd->v_X002);
              if ((loenStatus = foenItemCallList_Append(lpstList, lpstNode)) == FALSE)
                {
                  sprintf (szTemp, "fpstItemCallList_New: Can't append node\n");
                  macErrorMessage (BILL_NO_OUTPUT_GENERATED, WARNING, szTemp);              
                  fovdPopFunctionName();
                  return NULL;
                }
              
              fovdPrintLog (LOG_DEBUG, "fpstItemCallList_New: XCD: %s %s appended\n", 
                            lpstG99->xcd->v_X001, lpstG99->xcd->v_X002);
            }
          else
            {
              fovdPrintLog (LOG_DEBUG, "fpstItemCallList_New: XCD: %s %s suppressed\n", 
                            lpstG99->xcd->v_X001, lpstG99->xcd->v_X002);
              free(lpstNode->spstItem);
              free(lpstNode);
            }
        }

      lpstG99 = lpstG99->g_99_next;
    }

  fovdPopFunctionName();
  return lpstList;
}


/********************************************************************************************************************
 *
 * METHOD: fpstItemCallList_Gen
 *
 * DESCRIPTIN: 
 *
 ********************************************************************************************************************
 */

static char *dapsnzStartTag[2] = {"SimCallListStart", "SimRoamingCallListStart"};
static char *dapsnzEndTag[2]   = {"SimCallListEnd", "SimRoamingCallListEnd"};
static char *dapsnzTag[2]      = {"SimCall", "SimRoamingCall"};
static char *dapsnzSumTag[2]   = {"SimCallListSum", "SimRoamingCallListSum"};

#define HPLMN_CALL_INDEX 0
#define VPLMN_CALL_INDEX 1

static tostCallRecord dostRecord;

toenBool foenItemCallList_GenOld(tostItemCallList *ppstList, char *ppsnzNumber)
{
  int loiCount, i, n, j = 0;
  tostItemCallNode *lpstNode, *lpstTmp;
  tostItemCall *lpstItem;
  char *lpsnzTag, *lpsnzType;
  toenBool loenStatus;

  char lasnzCount[32];
  char lasnzAirPayment[32];
  char lasnzIntPayment[32];
  char lasnzSumPayment[32];

  char lasnzTAMOCCount[32];
  char lasnzTAMOCAirPayment[32];
  char lasnzTAMOCIntPayment[32];
  char lasnzTAMOCSumPayment[32];
  
  char lasnzTAMTCCount[32];
  char lasnzTAMTCAirPayment[32];
  char lasnzTAMTCIntPayment[32];
  char lasnzTAMTCSumPayment[32];
  
  fovdPushFunctionName("foenItemCallList_Gen");

  /*
   * Zero size list isn't to be printed
   */

  if (ppstList->soiCount == 0)
    {
      fovdPopFunctionName();
      return TRUE;
    }

  fovdPrintLog (LOG_DEBUG, "fpstItemCallList_Gen: Printing list type: %d with size: %d\n", 
                ppstList->soenType,
                ppstList->soiCount);
  
  /*
   * Which tag group is to be used
   */

  if (ppstList->soenType == HPLMN_TYPE)
    {
      i = 0;
    }
  else
    {
      i = 1;
    }

  if (goiBillMedium != 2 && goiBillMedium != 3)
    {
      if (FALSE == stBgh.soContractCallDetailListExist)
	{
	  if (0.0 == stBgh.sofoThresholdValue)
	    {
	      fovdGen("SimListPrintITB", "1", EOL);
	      stBgh.soSubscriberCallDetailListPrinted = TRUE;
	    }
	  else
	    {
	      fovdGen("SimListPrintITB", "0", EOL );
	    }
	}
      
      stBgh.soContractCallDetailListExist = TRUE;
      sprintf(lasnzCount, "%d", ppstList->soiCount);
      fovdGen(dapsnzStartTag[i], lasnzCount, EOL);
    }

  /*
   * Convert each item to record (formating) and print 
   */
  
  fovdPrintLog (LOG_DEBUG, "fpstItemCallList_Gen: Printing list\n");
  lpstNode = ppstList->spstRoot;
  while (lpstNode)
    {
      lpstItem = lpstNode->spstItem;
      if (goiBillMedium == 2 || goiBillMedium == 3)
        {
          /*
           * Itemized bill in separate file 
           */

          fovdPrintLog (LOG_DEBUG, "fpstItemCallList_Gen: Call list item: %d\n", j++);
          fovdItemCall_FillRecord(lpstItem, &dostRecord);

          fovdPrintLog (LOG_DEBUG, "fpstItemCallList_Gen: DISK %s %s %s\n",  
                        dostRecord.sasnzDate,
                        dostRecord.sasnzTime,
                        dostRecord.sasnzServiceShdes);
          
          if (ppstList->soenType == VPLMN_TYPE)  
            {
              lpsnzType = "N";
            }
          else
            {
              lpsnzType = "T";
            }
          /* dostRecord - struktura z poformatowane (null terminated szczegoly rozmow */
          fovdGenItb("", 
                     ppsnzNumber,                    /* directory number */
                     dostRecord.sasnzDate,           /* date of the call */
                     dostRecord.sasnzTime,           /* time of the call */
                     dostRecord.sasnzServiceShdes,   /* service abbreviation */
                     dostRecord.sasnzDirection,      /* zone description */
                     dostRecord.sasnzDirectoryNumber,/* called number */              
                     dostRecord.sasnzDuration,       /* billed minutes */
                     dostRecord.sasnzLocalPayment,   /* PTK charges */
                     dostRecord.sasnzInterPayment,   /* TPSA charges */
                     dostRecord.sasnzSummaryPayment, /* total charges */                
                     lpsnzType,
                     EOL);
        }
      else
        {                
          /*
           * Itemized bill on the paper
           */

          fovdPrintLog (LOG_DEBUG, "fpstItemCallList_Gen: Call list item: %d\n", j++);
          fovdItemCall_FillRecord(lpstItem, &dostRecord);
          
          /*
           * Print formated record
           */
          
          fovdPrintLog (LOG_DEBUG, "fpstItemCallList_Gen: Printing record\n");

#ifdef US_TIMM_LAYOUT
          if(!strcmp(dapsnzTag[i],"SimCall"))
          {
              fovdGen("SimCall", 
                      dostRecord.sasnzServiceShdes,
                      dostRecord.sasnzCallType,
                      dostRecord.sasnzDate,
                      dostRecord.sasnzTime,
                      dostRecord.sasnzPLMN,
                      dostRecord.sasnzDirection,
                      dostRecord.sasnzDirectoryNumber,              
                      dostRecord.sasnzDuration,
                      dostRecord.sasnzLocalPayment,
                      dostRecord.sasnzInterPayment,
                      dostRecord.sasnzSummaryPayment,
                      dostRecord.sasnzCurrency,
                      dostRecord.soszTimeZone,
                      dostRecord.soszDestinLoc,
                      dostRecord.soszOriginLoc,
                      EOL);
          }
          else
#endif
          {
              fovdGen(dapsnzTag[i], 
                      dostRecord.sasnzServiceShdes,
                      dostRecord.sasnzCallType,
                      dostRecord.sasnzDate,
                      dostRecord.sasnzTime,
                      dostRecord.sasnzPLMN,
                      dostRecord.sasnzDirection,
                      dostRecord.sasnzDirectoryNumber,              
                      dostRecord.sasnzDuration,
                      dostRecord.sasnzLocalPayment,
                      dostRecord.sasnzInterPayment,
                      dostRecord.sasnzSummaryPayment,
                      dostRecord.sasnzCurrency,
                      EOL);
          }
          
          /*
           * Legend label 
           */
          
          fovdPrintLog (LOG_DEBUG, "fpstItemCallList_Gen: Adding label: %s\n", lpstNode->spstItem->sasnzServiceShdes);
          loenStatus = foenLegend_AddLabel(dostRecord.sasnzServiceShdes,      /* service - may be modified by flags */
                                           lpstNode->spstItem->sasnzCallType, /* seq of conn. types */
                                           lpstNode->spstItem->soenIsDestZoneVPN, /*  */
                                           dostRecord.sasnzCurrency);         /* here is info about FUM usage */
          if (loenStatus == FALSE)
            {
              fovdPopFunctionName();
              return FALSE;
            }
        }

      /*
       * Next one
       */

      lpstNode = lpstNode->spstNext;
    }
  
  /*
   * End of list
   */

  if (goiBillMedium != 2 && goiBillMedium != 3)
    {
      fovdGen(dapsnzEndTag[i], EOL);
    }

  /*
   * Print summary
   */

  if (goiBillMedium != 2 && goiBillMedium != 3)
    {
      fovdPrintLog (LOG_DEBUG, "fpstItemCallList_Gen: Creating call list summary\n");
      if (ppstList->soenType == VPLMN_TYPE)  
        { 
          if (ppstList->soiTAMOCCount > 0)
            {
              sprintf(lasnzTAMOCCount, "%d", ppstList->soiTAMOCCount);
              
              /*
               * Adjust TA-MOC values
               */

              /*
                extern double goflSimUsage_Local;
                extern double goflSimUsage_Roaming;
                extern double goflSimUsage_Surcharge;
              */
              
              /*
                sprintf(lasnzTAMOCAirPayment, "%.2lf", goflSimUsage_Surcharge);
                sprintf(lasnzTAMOCIntPayment, "%.2lf", goflSimUsage_Roaming);
                sprintf(lasnzTAMOCSumPayment, "%.2lf", goflSimUsage_Roaming + goflSimUsage_Surcharge);
              */
              
              sprintf(lasnzTAMOCAirPayment, "%.2lf", ppstList->soflTAMOCAirPayment);
              sprintf(lasnzTAMOCIntPayment, "%.2lf", ppstList->soflTAMOCIntPayment);
              sprintf(lasnzTAMOCSumPayment, "%.2lf", ppstList->soflTAMOCSumPayment);
              
              fovdFormatMoney(lasnzTAMOCAirPayment);
              fovdFormatMoney(lasnzTAMOCIntPayment);
              fovdFormatMoney(lasnzTAMOCSumPayment);  
              
              fovdGen("SimOutboundCallSum", lasnzTAMOCCount, 
                      lasnzTAMOCAirPayment, lasnzTAMOCIntPayment, lasnzTAMOCSumPayment, 
                      EOL);
            }
      
          if (ppstList->soiTAMTCCount > 0)
            {
              sprintf(lasnzTAMTCCount, "%d", ppstList->soiTAMTCCount);          
              
              /*
               * Adjust TA-MTC values
               */ 
              /*
                extern double goflSimUsage_Local;
                extern double goflSimUsage_Roaming;
                extern double goflSimUsage_Surcharge;
              */
              
              sprintf(lasnzTAMTCAirPayment, "%.2lf", ppstList->soflTAMTCAirPayment);
              sprintf(lasnzTAMTCIntPayment, "%.2lf", ppstList->soflTAMTCIntPayment);
              sprintf(lasnzTAMTCSumPayment, "%.2lf", ppstList->soflTAMTCSumPayment);
              
              fovdFormatMoney(lasnzTAMTCAirPayment);
              fovdFormatMoney(lasnzTAMTCIntPayment);
              fovdFormatMoney(lasnzTAMTCSumPayment);  
          
              fovdGen("SimInboundCallSum", lasnzTAMTCCount, 
                      lasnzTAMTCAirPayment, lasnzTAMTCIntPayment, lasnzTAMTCSumPayment, 
                      EOL);      
            }
        }
      
      n = sprintf(lasnzAirPayment, "%.2lf", ppstList->soflAirPayment);
      n = sprintf(lasnzIntPayment, "%.2lf", ppstList->soflIntPayment);
      n = sprintf(lasnzSumPayment, "%.2lf", ppstList->soflSumPayment);
      
      fovdFormatMoney(lasnzAirPayment);
      fovdFormatMoney(lasnzIntPayment);
      fovdFormatMoney(lasnzSumPayment);  
  
      fovdGen("SimCallListSum", lasnzCount, lasnzAirPayment, lasnzIntPayment, lasnzSumPayment, EOL);
    }

  fovdPopFunctionName();
  return TRUE;
}

toenBool foenItemCallList_Gen(tostItemCallList *ppstList, char *ppsnzNumber)
{
    int loiCount, i, n, j = 0;
    tostItemCallNode *lpstNode, *lpstTmp;
    tostItemCall *lpstItem;
    char *lpsnzTag, *lpsnzType;
    toenBool loenStatus;

    char lasnzCount[32];
    char lasnzAirPayment[32];
    char lasnzIntPayment[32];
    char lasnzSumPayment[32];

    char lasnzTAMOCCount[32];
    char lasnzTAMOCAirPayment[32];
    char lasnzTAMOCIntPayment[32];
    char lasnzTAMOCSumPayment[32];
  
    char lasnzTAMTCCount[32];
    char lasnzTAMTCAirPayment[32];
    char lasnzTAMTCIntPayment[32];
    char lasnzTAMTCSumPayment[32];
  
    fovdPushFunctionName("foenItemCallList_Gen");

    /*
     * Zero size list isn't to be printed
     */

    if (ppstList->soiCount == 0)
    {
        fovdPopFunctionName();
        return TRUE;
    }

    fovdPrintLog (LOG_DEBUG, "fpstItemCallList_Gen: Printing list type: %d with size: %d\n", 
                  ppstList->soenType,
                  ppstList->soiCount);
  
    /*
     * Which tag group is to be used
     */
  
    if (ppstList->soenType == HPLMN_TYPE)
    {
        i = 0;
    }
    else
    {
        i = 1;
    }

    if (FALSE == stBgh.soContractCallDetailListExist)
    {
        fovdPrintLog (LOG_DEBUG, "fpstItemCallList_Gen: threshold %f, bill medium %d\n",
                       stBgh.sofoThresholdValue,goiBillMedium);
        if (0.0 == stBgh.sofoThresholdValue )
        {
            fovdGen("SimListPrintITB", "1", EOL);
            stBgh.soSubscriberCallDetailListPrinted = TRUE;
        }
        else
        {
            fovdGen("SimListPrintITB", "0", EOL );
        }
    }
      
    stBgh.soContractCallDetailListExist = TRUE;
    n = sprintf(lasnzCount, "%d", ppstList->soiCount);
    fovdGen(dapsnzStartTag[i], lasnzCount, EOL);

    /*
     * Convert each item to record (formating) and print 
     */
  
    fovdPrintLog (LOG_DEBUG, "fpstItemCallList_Gen: Printing list\n");
    lpstNode = ppstList->spstRoot;
    while (lpstNode)
    {
        lpstItem = lpstNode->spstItem;
        if (goiBillMedium == 2 || goiBillMedium == 3)
        {
            /*
             * Itemized bill in separate file 
             */

            fovdPrintLog (LOG_DEBUG, "fpstItemCallList_Gen: Call list item: %d\n", j++);
            fovdItemCall_FillRecord(lpstItem, &dostRecord);
	  
            fovdPrintLog (LOG_DEBUG, "fpstItemCallList_Gen: DISK %s %s %s\n",  
                          dostRecord.sasnzDate,
                          dostRecord.sasnzTime,
                          dostRecord.sasnzServiceShdes);
          
            if (ppstList->soenType == VPLMN_TYPE)  
            {
                lpsnzType = "N";
            }
            else
            {
                lpsnzType = "T";
            }

            /* dostRecord - struktura z poformatowane (null terminated szczegoly rozmow */
            fovdGenItb("", 
                       ppsnzNumber,                    /* directory number */
                       dostRecord.sasnzDate,           /* date of the call */
                       dostRecord.sasnzTime,           /* time of the call */
                       dostRecord.sasnzServiceShdes,   /* service abbreviation */
                       dostRecord.sasnzDirection,      /* zone description */
                       dostRecord.sasnzDirectoryNumber,/* called number */              
                       dostRecord.sasnzDuration,       /* billed minutes */
                       dostRecord.sasnzLocalPayment,   /* PTK charges */
                       dostRecord.sasnzInterPayment,   /* TPSA charges */
                       dostRecord.sasnzSummaryPayment, /* total charges */                
                       lpsnzType,
                       EOL);
        }

        /*
         * Itemized bill on the paper - for everyone
         */
      
        fovdPrintLog (LOG_DEBUG, "fpstItemCallList_Gen: Call list item: %d\n", j++);
        fovdItemCall_FillRecord(lpstItem, &dostRecord);
      
        /*
         * Print formated record
         */
      
        fovdPrintLog (LOG_DEBUG, "fpstItemCallList_Gen: Printing record\n");

#ifdef US_TIMM_LAYOUT
          if(!strcmp(dapsnzTag[i],"SimCall"))
          {
              fovdGen("SimCall", 
                      dostRecord.sasnzServiceShdes,
                      dostRecord.sasnzCallType,
                      dostRecord.sasnzDate,
                      dostRecord.sasnzTime,
                      dostRecord.sasnzPLMN,
                      dostRecord.sasnzDirection,
                      dostRecord.sasnzDirectoryNumber,              
                      dostRecord.sasnzDuration,
                      dostRecord.sasnzLocalPayment,
                      dostRecord.sasnzInterPayment,
                      dostRecord.sasnzSummaryPayment,
                      dostRecord.sasnzCurrency,
                      dostRecord.soszTimeZone,
                      dostRecord.soszDestinLoc,
                      dostRecord.soszOriginLoc,
                      EOL);
          }
          else
#endif
          {
              fovdGen(dapsnzTag[i], 
                      dostRecord.sasnzServiceShdes,
                      dostRecord.sasnzCallType,
                      dostRecord.sasnzDate,
                      dostRecord.sasnzTime,
                      dostRecord.sasnzPLMN,
                      dostRecord.sasnzDirection,
                      dostRecord.sasnzDirectoryNumber,              
                      dostRecord.sasnzDuration,
                      dostRecord.sasnzLocalPayment,
                      dostRecord.sasnzInterPayment,
                      dostRecord.sasnzSummaryPayment,
                      dostRecord.sasnzCurrency,
                      EOL);
          }

        /*
         * Legend label 
         */
      
        fovdPrintLog (LOG_DEBUG, "fpstItemCallList_Gen: Adding label: %s\n", lpstNode->spstItem->sasnzServiceShdes);
        loenStatus = foenLegend_AddLabel(dostRecord.sasnzServiceShdes,      /* service - may be modified by flags */
                                         lpstNode->spstItem->sasnzCallType, /* seq of conn. types */
                                         lpstNode->spstItem->soenIsDestZoneVPN, /*  */
                                         dostRecord.sasnzCurrency);         /* here is info about FUM usage */
        if (loenStatus == FALSE)
        {
            fovdPopFunctionName();
            return FALSE;
        }

        /*
         * Next call
         */
      
        lpstNode = lpstNode->spstNext;
    }
  
    /*
     * End of list
     */
  
    fovdGen(dapsnzEndTag[i], EOL);

    /*
     * Print summary
     */

    fovdPrintLog (LOG_DEBUG, "fpstItemCallList_Gen: Creating call list summary\n");
    if (ppstList->soenType == VPLMN_TYPE)  
    { 
        if (ppstList->soiTAMOCCount > 0)
        {
            sprintf(lasnzTAMOCCount, "%d", ppstList->soiTAMOCCount);
	  
            /*
             * Adjust TA-MOC values
             */
	  
            sprintf(lasnzTAMOCAirPayment, "%.2lf", ppstList->soflTAMOCAirPayment);
            sprintf(lasnzTAMOCIntPayment, "%.2lf", ppstList->soflTAMOCIntPayment);
            sprintf(lasnzTAMOCSumPayment, "%.2lf", ppstList->soflTAMOCSumPayment);
	  
            fovdFormatMoney(lasnzTAMOCAirPayment);
            fovdFormatMoney(lasnzTAMOCIntPayment);
            fovdFormatMoney(lasnzTAMOCSumPayment);  
	  
            fovdGen("SimOutboundCallSum", lasnzTAMOCCount, 
                    lasnzTAMOCAirPayment, lasnzTAMOCIntPayment, lasnzTAMOCSumPayment, 
                    EOL);
        }
      
        if (ppstList->soiTAMTCCount > 0)
        {
            sprintf(lasnzTAMTCCount, "%d", ppstList->soiTAMTCCount);          
	  
            /*
             * Adjust TA-MTC values
             */ 
	  
            sprintf(lasnzTAMTCAirPayment, "%.2lf", ppstList->soflTAMTCAirPayment);
            sprintf(lasnzTAMTCIntPayment, "%.2lf", ppstList->soflTAMTCIntPayment);
            sprintf(lasnzTAMTCSumPayment, "%.2lf", ppstList->soflTAMTCSumPayment);
	  
            fovdFormatMoney(lasnzTAMTCAirPayment);
            fovdFormatMoney(lasnzTAMTCIntPayment);
            fovdFormatMoney(lasnzTAMTCSumPayment);  
          
            fovdGen("SimInboundCallSum", lasnzTAMTCCount, 
                    lasnzTAMTCAirPayment, lasnzTAMTCIntPayment, lasnzTAMTCSumPayment, 
                    EOL);      
        }
    }
  
    n = sprintf(lasnzAirPayment, "%.2lf", ppstList->soflAirPayment);
    n = sprintf(lasnzIntPayment, "%.2lf", ppstList->soflIntPayment);
    n = sprintf(lasnzSumPayment, "%.2lf", ppstList->soflSumPayment);
  
    fovdFormatMoney(lasnzAirPayment);
    fovdFormatMoney(lasnzIntPayment);
    fovdFormatMoney(lasnzSumPayment);  
  
    fovdGen("SimCallListSum", lasnzCount, lasnzAirPayment, lasnzIntPayment, lasnzSumPayment, EOL);
  
    fovdPopFunctionName();
    return TRUE;
}

/********************************************************************************************************************
 *
 * METHOD: fpstItemCallList_Delete
 *
 * DESCRIPTIN: 
 *
 ********************************************************************************************************************
 */

toenBool foenItemCallList_Delete(tostItemCallList *lpstList)
{
  tostItemCallNode *lpstNode, *lpstTmp;  
  toenBool loenStatus;
  int i = 0;

  fovdPushFunctionName("foenItemCallList_Delete");

  fovdPrintLog (LOG_DEBUG, "fpstItemCallList_Delete: Deleting list\n");

  /*
   * Delete each node of the list
   */
  
  lpstNode = lpstList->spstRoot;
  while (lpstNode)
    {
      fovdPrintLog (LOG_DEBUG, "fpstItemCallList_Delete: Deleting node: %d\n", i++);
      if (lpstNode->spstItem != NULL)
        {
          free(lpstNode->spstItem);
        }

      lpstTmp = lpstNode;
      lpstNode = lpstNode->spstNext;

      if (lpstTmp != NULL)
        {
          free(lpstTmp);
        }
    }

  /*
   * List was allocated from heap so it must be destoyed
   */
  
  free(lpstList);

  fovdPopFunctionName();
  return TRUE;
}

int foiItemCallList_CountLocalCalls(tostItemCallList *ppstList)
{
  tostItemCallNode *lpstNode;
  tostItemCall *lpstCall;
  int loiCallsNo = 0;
  
  if (ppstList == NULL)
    {
      return 0;
    }

  lpstNode = ppstList->spstRoot;
  while (lpstNode)
    {
      lpstCall = lpstNode->spstItem;
      switch (ppstList->soenType)
        {
        case HPLMN_TYPE:
          if (lpstCall->sasnzCallType[0] == 'C' 
              && strlen(lpstCall->sasnzCallType) == 1 
              && lpstCall->soflLocalPayment == 0.0
              && lpstCall->soflInterPayment == 0.0
              && lpstCall->soflTapNetPayment == 0.0
              && lpstCall->soflTapTaxPayment == 0.0)
            {
              /* */
            }
          else
            {
              loiCallsNo++;
            }

          break;

        case VPLMN_TYPE:
          if (lpstCall->sochPLMN == 'V' 
              && lpstCall->sochCall == 'I' 
              && (lpstCall->sochRLeg == 'R' || lpstCall->soflTapNetPayment == 0.0))
            {
              loiCallsNo++;
            }

          break;
        }

      lpstNode = lpstNode->spstNext;
    }

  return loiCallsNo;
}

int foiItemCallList_CountRoamingCalls(tostItemCallList *ppstList)
{
  tostItemCallNode *lpstNode;
  tostItemCall *lpstCall;
  int loiCallsNo = 0;

  if (ppstList == NULL)
    {
      return 0;
    }

  if (ppstList->soenPLMNType != VPLMN_TYPE)
    {
      return 0;
    }

  lpstNode = ppstList->spstRoot;
  while (lpstNode)
    {
      lpstCall = lpstNode->spstItem;
      if (lpstCall->sochPLMN == 'V' 
          && lpstCall->sochCall == 'I' 
          && (lpstCall->sochRLeg == 'R' || lpstCall->soflTapNetPayment == 0.0))
        {
          /* */
        }
      else
        {
          loiCallsNo++;
        }

      lpstNode = lpstNode->spstNext;
    }

  return loiCallsNo;
}



#ifdef US_TIMM_LAYOUT
static toenBool foenCommaSepString(char *ppszFirst,  char *ppszSecond, int poiOutLen, char *ppszOut)
{
    int loiLen;

    ASSERT ((loiLen = strlen(ppszFirst)) + strlen(ppszSecond) + 1 < poiOutLen);
    if('\0' != *ppszFirst || '\0' != *ppszSecond)
    {
        strcpy(ppszOut,ppszFirst);
        *(ppszOut + loiLen++) = ',';
        strcpy(ppszOut+loiLen, ppszSecond);
    }
    return TRUE;
}
#endif
