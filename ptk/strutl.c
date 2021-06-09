/**************************************************************************/
/*  MODULE : String conversion utilities                                  */
/*                                                                        */
/*  AUTHOR : N.Bondarczuk                                                 */
/*                                                                        */
/*  CREATION DATE : 22.10.97                                              */
/*                                                                        */
/*  DESCRIPTION : Exports  function  converting strings                   */
/*                                                                        */
/**************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "bgh.h"
#include "parser.h"
#include "strutl.h"
#include "types.h"
#include "gen.h"

#if 0	/* just for version.sh */
static char *SCCS_VERSION = "4.1.2";
#endif

extern stBGHGLOB stBgh;

double foflRound(double poflAmount);

/*
 * 29.01.98 N.Bondarczuk : Change label for connections
 * 29.01.98 N.Bondarczuk : Fixed up the problem with formating date in format yyyymmdd to yyyy.mm.dd 
 */

#define SUBFIELD_END '.'
#define FIELD_END ':'
#define BLOCK_END '\''
#define EOS '\0'
#define TRUE 1
#define FALSE 0
#define ASCII2NUM(a) (a - '0')

int field2str(char *source, int slen, char *buffer, int blen) 
{
  strcpy(buffer, source);
  return TRUE;
}


void fovdFormatMoney(char *pachzMoney)
{
  int i, j, n;
  double loflValue;

  fovdPushFunctionName("fovdFormatMoney");
  
  sscanf(pachzMoney, "%lf", &loflValue);
  
  fovdRoundMoney(pachzMoney, loflValue);
  
  n = strlen(pachzMoney);
  for (i = 0; i < n; i++)
    {
      if (pachzMoney[i] == '.')
        {
          pachzMoney[i] = ',';
        }
    }
  
  fovdPopFunctionName();
}

void add_blank(char *str) 
{
  int blank, i, j;
  static char tmp[MAX_BUFFER];

  blank = 1;
  j = 0;
  for (i = strlen(str) - 1; i >= 0; i--)
    {
      tmp[j++] = str[i];
      if (blank == 3) 
        {
          tmp[j++] = 32;
          blank = 0;
        }
      blank++;
    }
  tmp[j] = '\0';
  
  j = 0;
  for (i = strlen(tmp) - 1; i >= 0; i--)
    {
      str[j++] = tmp[i];
    }
  str[j] = '\0';
}  

void fovdFormatDate(char *pachzDate, int loiDateFormat)
{
  static char tmp[64];
  int n;

  fovdPushFunctionName("fovdFormatDate");
  
  if ((n = strlen(pachzDate)) == 8)
    {
      tmp[0] = pachzDate[0];
      tmp[1] = pachzDate[1];
      tmp[2] = pachzDate[2];
      tmp[3] = pachzDate[3];
      tmp[4] = '.';
      tmp[5] = pachzDate[4];
      tmp[6] = pachzDate[5];
      tmp[7] = '.';
      tmp[8] = pachzDate[6];
      tmp[9] = pachzDate[7];
      tmp[10] = '\0';
    }
  else if (n == 6)
    {
      if (pachzDate[0] == '9')
        {
          tmp[0] = '1';
          tmp[1] = '9';
        }
      else
        {
          tmp[0] = '2';
          tmp[1] = '0';
        }
      
      tmp[2] = pachzDate[0];
      tmp[3] = pachzDate[1];
      tmp[4] = '.';
      tmp[5] = pachzDate[2];
      tmp[6] = pachzDate[3];
      tmp[7] = '.';
      tmp[8] = pachzDate[4];
      tmp[9] = pachzDate[5];
      tmp[10] = '\0';
    }
  else
    {}

  strcpy(pachzDate, tmp);

  fovdPopFunctionName();
}

void fovdFormatCallsNumber(char *calls)
{
  int n, m;

  n = atoi(calls);
  if (n == 1)
    {
      strcat(calls, " po\xb3" "\xb1" "czenie");
    }
  else if (n > 0 && (m = ASCII2NUM(calls[strlen(calls) - 1])) >= 2 && m <= 4 && (n > 14 || n < 12))
    {
      strcat(calls,  " po\xb3" "\xb1" "czenia");
    }
  else
    {
      strcat(calls,  " po\xb3" "\xb1" "cze\xf1");
    }
}

void fovdFormatSimsNumber(char *sims)
{
  int n, m;

  n = atoi(sims);
  if (n == 1)
    {
      strcat(sims, " telefon");
    }
  else if (n > 0 && (m = ASCII2NUM(sims[strlen(sims) - 1])) >= 2 && m <= 4 && (n > 14 || n < 12))
    {
      strcat(sims, " telefony");
    }
  else
    {
      strcat(sims, " telefon\xf3w");
    }
}


void fovdFormatInvoiceNumber(char *poszInvNo)
{
  
}

void fovdFormatNumber(char *poszNo)
{
  int i, n;

  n = strlen(poszNo);
  for (i = 0; i < n; i++)
    {
      if (poszNo[i] == '.')
        {
          poszNo[i] = ',';
        }
    }
}

/*
 * input date format : YYMMDDHHMMSS or YYYYMMDDHHMMSS
 */

void fovdFormatCallDate(char *pachzDate)
{
  char tmp[32];

  if (strlen(pachzDate) == 12)
    {
      memcpy(tmp, pachzDate, 6);
      tmp[6] = '\0'; 
      fovdFormatDate(tmp, YY_MM_DD);  
    }
  else 
    {
      memcpy(tmp, pachzDate, 8);
      tmp[8] = '\0';
      fovdFormatDate(tmp, YYYY_MM_DD);  
    }

  strcpy(pachzDate, tmp);
}

/*
 * input date format : YYMMDDHHMMSS
 */

void fovdFormatCallTime(char *pachzDate)
{
  static char tmp[32];
  
  if (strlen(pachzDate) == 12)
    {
      tmp[0] = pachzDate[6];
      tmp[1] = pachzDate[7];
      tmp[2] = ':';
      tmp[3] = pachzDate[8];
      tmp[4] = pachzDate[9];
      tmp[5] = ':';
      tmp[6] = pachzDate[10];
      tmp[7] = pachzDate[11];
      tmp[8] = '\0';
    }
  else 
    {
      tmp[0] = pachzDate[8];
      tmp[1] = pachzDate[9];
      tmp[2] = ':';
      tmp[3] = pachzDate[10];
      tmp[4] = pachzDate[11];
      tmp[5] = ':';
      tmp[6] = pachzDate[12];
      tmp[7] = pachzDate[13];
      tmp[8] = '\0';
    }
  
  strcpy(pachzDate, tmp);
}


char *fpchzTranslate(char *ppchzStr, char *ppchFromStr, char *ppchToStr)
{
  static char lpchzStr[MAX_BUFFER];
  int loiLetter;

  for (loiLetter = 0; loiLetter < strlen(ppchzStr); loiLetter++)
    {
      if (ppchFromStr[0] == ppchzStr[loiLetter])
        {
          lpchzStr[loiLetter] = ppchToStr[loiLetter];
        }
      else
        {
          lpchzStr[loiLetter] = ppchzStr[loiLetter];
        }
    }
  lpchzStr[loiLetter] = '\0';

  return lpchzStr;
}
  
char *fpchzFormatCity(char *ppchzCCLine4)
{
  char *lpchzStr;
  static char lachzToken[64];
  int loiCounter;

  lachzToken[0] = '\0';
  lpchzStr = strtok(ppchzCCLine4, " ");
  if (lpchzStr != NULL)
    {
      if (atoi(lpchzStr)  > 0) /* ZIP number */
        {
          lpchzStr = strtok((char *)NULL, " ");          
          if (lpchzStr != NULL)
            {
              strncpy(lachzToken, lpchzStr, 40);
            }
          else
            {
              lachzToken[0] = '\0';
            }
        }
      else
        {
          strncpy(lachzToken, ppchzCCLine4, 40);
        }
    }

  return lachzToken;
}

char *fpchzFormatZip(char *ppchzCCLine4)
{
  char *lpchzStr;
  static char lachzToken[40];
  int loiCounter;

  lachzToken[0] = '\0';
  lpchzStr = strtok(ppchzCCLine4, " ");
  if (lpchzStr != NULL)
    {
      if (atoi(lpchzStr)  > 0)
        {
          strncpy(lachzToken, lpchzStr, 40);
          lachzToken[2] = '-'; lachzToken[3] = '\0';
          strncat(lachzToken, lpchzStr+2, 36);
        }
      else
        {
          lachzToken[0] = '\0';
        }
    }

  return lachzToken;

}

void fovdRoundMoney(char *pachzMoney, double pofValue)
{
  int n;

  fovdPushFunctionName("fovdRoundMoney");  
  sprintf(pachzMoney, "%.02lf", foflRound(pofValue));
  fovdPopFunctionName();
}


int foiScanMoa(char *ppstMoa, int n)
{
  char lastMoa[16];
  char *lpstPref, *lpstSuf;
  int loiPref, loiSuf;

  strncpy(lastMoa, ppstMoa, 16);
  
  lpstPref = lastMoa;
  lpstSuf = strchr(lastMoa, '.');
  *lpstSuf = '\0';
  lpstSuf++;  
  lpstSuf[n - 1] = '\0';
  
  sscanf(lpstPref, "%d", &loiPref);
  sscanf(lpstSuf, "%d", &loiSuf);

  return (loiPref * 100) + loiSuf;
}

extern toenBool foenCheckNIP(char *customer_nip)
{
  int i, n, zero_no = 0, digit_no = 0, minus_no = 0;
      
  n = strlen(customer_nip);
  for (i = 0; i < n; i++)
    {
      switch (customer_nip[i])
        {
        case '0':
          zero_no++;
          digit_no++;
          break;
          
        case '-':
          minus_no++;
          break;
          
        default:
          if (isdigit(customer_nip[i]))
            {
              digit_no++;
            }          
        }
    }

  /*
   * NIP is filled with 0 signs or no digits found
   */
  
  if (digit_no == zero_no)
    {
      return FALSE;
    }

  return TRUE;
}

char *fpsnzFormatAccessDays(char pochType, int poiDays)
{
  static char dasnzStr[MAX_BUFFER];

  fovdPrintLog (LOG_DEBUG, "fpsnzFormatAccessDays: %c %d\n", pochType, poiDays);
  if (pochType == 'A' || pochType == 'P')
    {
      sprintf(dasnzStr, "%d", poiDays);
      if (poiDays == 1)
        {
          strcat(dasnzStr, " dzien");
        }
      else 
        {
          strcat(dasnzStr,  " dni");
        }
    }
  else if (pochType == 'C')
    {
      dasnzStr[0] = '\0';
    }
  else
    {
      dasnzStr[0] = '\0';
    }

  fovdPrintLog (LOG_DEBUG, "fpsnzFormatAccessDays: Access charging: %c %s\n", pochType, dasnzStr);

  return dasnzStr;
}

void fovdStr_Clean(char *str)
{
  static char tmp[MAX_BUFFER];
  int i, j, n;

  n = strlen(str);
  for (i = 0, j = 0; i < n && str[i] != '\0'; i++)
    {
      if (isalnum(str[i]))
        {
          tmp[j++] = str[i];
        }
    }

  tmp[j] = '\0';
  strcpy(str, tmp);
}

/*
 * foflRound(-3.0, 10, 5, 1, loflSum)
 */

double foflDoubleRound(double poflExp, int poiLen, int poiMinLen, short posRoundUp, double poflVal)
{
#define EPSILON_MULT 100

    int loiIntNo, loiRmd;
    double loflIntLen = pow(10.0, poflExp);
    double loflSign, lofoEpsilon;

/*    printf("\nfoflDoubleRound_begin, %26.19f\n",poflVal); 
 */
    if (poflVal < 0.0)
    {
	lofoEpsilon = -poflVal + nextafter(poflVal, poflVal - 1.0);
/*
        poflVal = nextafter(poflVal, poflVal - 1.0);
*/
	/*
	  loflSign = -1.0;
	  poflVal = -poflVal;
	  */
    }
    else if (poflVal > 0)
    {
	lofoEpsilon = -poflVal + nextafter(poflVal, poflVal + 1.0);
/*
	poflVal = nextafter(poflVal, poflVal + 1.0);
*/
	/*
	  loflSign = 1.0;
	  */
    }
    else
    {
	return 0.0;
    }
    poflVal +=  EPSILON_MULT * lofoEpsilon;

#undef EPSILON_MULT 
    /*
      loiIntNo = rint(poflVal / loflIntLen);
      loiRmd = loiIntNo % poiLen;
      loiIntNo -= loiRmd;
      if (loiRmd == poiMinLen)
      {
      if (posRoundUp)
      {
      loiIntNo += poiLen;
      }
      }
      else if (loiRmd > poiMinLen)
      {
      loiIntNo += poiLen;
      }

      return loiIntNo * loflIntLen * loflSign;
      */

/*    printf("foflDoubleRound_end  , %26.19f\n",poflVal); 
 */
    return poflVal;
}

/*
 * machine rounding 
 */

double foflRound(double poflAmount)
{
  return foflDoubleRound(-3.0, 10, 5, 1, poflAmount);
}



