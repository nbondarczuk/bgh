#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "types.h"
#include "bgh.h"
#include "line_list.h"
#include "stream.h"
#include "generator.h"

/*
 * external variables
 */

extern tostBillMedium *gpstBillMedium;

extern int goilBillMediumSize;

extern stBGHGLOB stBgh;

/*
 * static functions
 */

static toenBool foenItbLineFixedFormat(char **papsnzArgv, int poiArgs, char *ppsnzLine, int poiMaxLineLen);

static toenBool foenItbLineExcelFormat(char **papsnzArgv, int poiArgs, char *ppsnzLine, int poiMaxLineLen);

static void fovdTranslate(char *ppsnzBuf, char pochFromChr, char pochtoChr);

int foiMapBillMediumId2Index(int poiId);

char *fpsnzMapBillMediumInd2Des(int poiIndex);

tostBuf *fpstFileMap(char *spsnzHeaderPath)
{
  int fd, rc;
  struct stat s;
  tostBuf *lpstBuf;
  
  /*
   * Get info about header file
   */

  if ((rc = stat(spsnzHeaderPath, &s)) == -1)
    {
      return NULL;
    }
  
  if ((fd = open(spsnzHeaderPath, O_RDONLY)) == -1)
    {
      return NULL;
    }
  
  /*
   * Alloc and init buffer
   */
  
  if ((lpstBuf = (tostBuf *)calloc(1, sizeof(tostBuf))) == NULL)
    {
      return NULL;
    }
  
  lpstBuf->soiLen = s.st_size;

  if ((lpstBuf->spsnVal = (char *)mmap(NULL, s.st_size, PROT_READ, MAP_PRIVATE, fd, 0)) == NULL)
    {
      return NULL;
    }
  
  return lpstBuf;
}

#define LBR '('
#define RBR ')'
#define QUOTE '\\'

void fovdFormatPSArg(char *lasnzBuf, int poiBufLen, char *lpsnzArg)
{
  int i = 0, j = 0;
  char c;

  lasnzBuf[i++] = LBR;    

  do { 

    /*
     * Filter PS special characters
     */
    
    if ((c = lpsnzArg[j++]) == LBR || c == RBR)
      {
        lasnzBuf[i++] = QUOTE;  
      }
    
    lasnzBuf[i++] = c;  
    
  } while (c != '\0' && i < poiBufLen);
  
  lasnzBuf[i - 1] = RBR;    
  lasnzBuf[i] = '\0';    
}

int foiMapBillMediumId2Index(int poiId)
{
  int i;
  fovdPrintLog (LOG_DEBUG, "foiMapBillMediumId2Index BM id %d\n", poiId);
  for (i = 0; i < goilBillMediumSize; i++)
    {
      fovdPrintLog (LOG_DEBUG, "foiMapBillMediumId2Index: Testing BM: %ld, %d, %s\n", 
                    gpstBillMedium[i].soilIndex,
                    gpstBillMedium[i].soiId,
                    gpstBillMedium[i].sasnzDes);

      if (gpstBillMedium[i].soiId == poiId)
        {
		  fovdPrintLog (LOG_DEBUG, "foiMapBillMediumId2Index: BM index %ld\n",gpstBillMedium[i].soilIndex); 
          return gpstBillMedium[i].soilIndex;
        }
    }
  
  return -1;
}

char *fpsnzMapBillMediumInd2Des(int poiIndex)
{
  int i;
  fovdPrintLog (LOG_DEBUG, "fpsnzMapBillMediumInd2Des BM index %d\n", poiIndex);
  if(poiIndex >= 0 && poiIndex < goilBillMediumSize )
  {
		fovdPrintLog (LOG_DEBUG, "fpsnzMapBillMediumInd2Des: BM desc %s\n",gpstBillMedium[poiIndex].sasnzDes); 
		return gpstBillMedium[poiIndex].sasnzDes;
  }
  return NULL;
}

toenStreamType foenMapDocInv2Stream(toenDocType poenDoc, toenInvType poenInv)
{
  toenStreamType loenStream = STREAM_UNDEFINED_TYPE;

  switch (poenDoc)
    {
    case DOC_INV_TYPE:
      switch (poenInv)
        {
        case INV_MINUS_TYPE:
          loenStream = STREAM_INV_MINUS_TYPE;
          break;

        case INV_MINUSVAT_TYPE:
          loenStream = STREAM_INV_MINUSVAT_TYPE;
          break;

        case INV_DEFAULT_TYPE:
          loenStream = STREAM_INV_TYPE;
          break;

        case INV_ENCLOSURE_TYPE:
          loenStream = STREAM_ENCLOSURE_TYPE;
          break;

        case INV_PAYMENT_TYPE:
          loenStream = STREAM_PAYMENT_TYPE;
          break;	  
        }

      break;

    case DOC_DNL_TYPE:
      loenStream =  STREAM_DNL_TYPE;
      break;

    case DOC_WLL_TYPE:
      loenStream = STREAM_WLL_TYPE;
      break;

    case DOC_INL_TYPE:
      loenStream = STREAM_INL_TYPE;
      break;

    case DOC_INP_TYPE:
      loenStream = STREAM_INP_TYPE;
      break;

    case DOC_ITBEXCEL_TYPE:
      loenStream = STREAM_ITBEXECEL_TYPE;
      break;

    case DOC_ITBFIXED_TYPE:
      loenStream = STREAM_ITBFIXED_TYPE;
      break;
    }

  return loenStream;
}

char *fpsnzBillMediumDesPadded(int poiBillMedium)
{
  int i;
  char *lpsnzDes;
  static char dasnzDes[MAX_BILL_MEDIUM_DES + 8];

  /*
   * Get bill medium descr.
   */
  
  fovdPrintLog (LOG_DEBUG, "fpsnzBillMediumDesPadded: BM index %d\n", poiBillMedium);
  if ((lpsnzDes = fpsnzMapBillMediumInd2Des(poiBillMedium)) == NULL) 
    {
      return FALSE;
    }
      
  /*
   * Pad string
   */

  memset(dasnzDes, 0x00, MAX_BILL_MEDIUM_DES + 8);
  strncpy(dasnzDes, lpsnzDes, MAX_BILL_MEDIUM_DES);
  for (i = 0; i < MAX_BILL_MEDIUM_DES; i++)
    {
      if (isalnum(dasnzDes[i]) == 0)
        {
          dasnzDes[i] = BILL_MEDIUM_PAD_CHAR;
        }
    }
  
  fovdPrintLog (LOG_DEBUG, "fpsnzBillMediumDesPadded: BM  desc  %s\n", dasnzDes);
  return dasnzDes;
}

toenBool foenItbLineFormat(int poiBillMedium, char **papsnzArgv, int poiArgs, char *ppsnzLine, int poiMaxLineLen)
{
  toenBool loenStatus;

  fovdPushFunctionName ("foenItbLineFormat");

  switch (poiBillMedium)
    {
    case DISK_FIXED_BILL_MEDIUM:
      loenStatus = foenItbLineFixedFormat(papsnzArgv, poiArgs, ppsnzLine, poiMaxLineLen);
      break;

    case DISK_EXCEL_BILL_MEDIUM:
      loenStatus = foenItbLineExcelFormat(papsnzArgv, poiArgs, ppsnzLine, poiMaxLineLen);
      break;
      
    default:
      ASSERT(FALSE);
    }

  fovdPopFunctionName ();  
  return loenStatus;
}

#define DEFAULT_ARGS_NO 10
#define EXCEL_SEPARATOR ";"
#define EXCEL_END_OF_RECORD "\n"

static toenBool foenItbLineExcelFormat(char **papsnzArgv, int poiArgs, char *ppsnzLine, int poiMaxLineLen)
{
  int i, loiLen = 0;

  fovdPushFunctionName ("foenItbLineExcelFormat");

  memset(ppsnzLine, 0x00, poiMaxLineLen);

  for (i = 0; i < DEFAULT_ARGS_NO; i++)
    {
      loiLen += strlen(papsnzArgv[i]) + strlen(EXCEL_SEPARATOR);
      if (loiLen > poiMaxLineLen)
        {
          fovdPopFunctionName ();  
          return FALSE;
        }

      strcat(ppsnzLine, papsnzArgv[i]);
      if (i < poiArgs - 1)
        {
          strcat(ppsnzLine, EXCEL_SEPARATOR);
        }
    }

  strcat(ppsnzLine, EXCEL_END_OF_RECORD);

  fovdPopFunctionName ();  
  return TRUE;
}

#define FIXED_LINE_LEN 96

#define FIELD_01_OFFSET 0
#define FIELD_02_OFFSET 8
#define FIELD_03_OFFSET 16
#define FIELD_04_OFFSET 22
#define FIELD_05_OFFSET 24
#define FIELD_06_OFFSET 25
#define FIELD_07_OFFSET 41
#define FIELD_08_OFFSET 53
#define FIELD_09_OFFSET 59 
#define FIELD_10_OFFSET 68
#define FIELD_11_OFFSET 77
#define FIELD_12_OFFSET 87
#define FIELD_13_OFFSET 94
#define FIELD_14_OFFSET 95

#define FIELD_01_LEN    8
#define FIELD_02_LEN    8
#define FIELD_03_LEN    6
#define FIELD_04_LEN    2
#define FIELD_05_LEN    1
#define FIELD_06_LEN    16
#define FIELD_07_LEN    12
#define FIELD_08_LEN    6
#define FIELD_09_LEN    9
#define FIELD_10_LEN    9
#define FIELD_11_LEN    10
#define FIELD_12_LEN    7
#define FIELD_13_LEN    1
#define FIELD_14_LEN    1

#define DIRECTORY_NUMBER_INDEX 0
#define CALL_DATE_INDEX        1
#define CALL_TIME_INDEX        2
#define SERVICE_SHDES_INDEX    3
#define ZONE_DES_INDEX         4
#define CALLED_NUMBER_INDEX    5
#define DURATION_INDEX         6
#define LOCAL_PAYMENT_INDEX    7
#define INTER_PAYMENT_INDEX    8
#define SUMMARY_PAYMENT_INDEX  9
#define ROAMING_IND_INDEX      10

#define DOUBLE_ROUNDING 0.00000001

static toenBool foenItbLineFixedFormat(char **papsnzArg, int poiArgs, char *ppsnzLine, int poiMaxLineLen)
{
  static char lasnLine[FIXED_LINE_LEN + 8];
  static char lasnzBuf[FIXED_LINE_LEN + 8];
  double loflVal;
  int loiVal, i, n;

  fovdPushFunctionName ("foenItbLineFixedFormat");

  ASSERT(poiMaxLineLen > FIXED_LINE_LEN + 2);

  memset(lasnLine, '_', FIXED_LINE_LEN);

  /*
   * FIELD 01 - only last 8 digits of the number
   */

  n = strlen(papsnzArg[DIRECTORY_NUMBER_INDEX]);
  if (n > FIELD_01_LEN)
    {
      i = n - FIELD_01_LEN;
      memcpy(lasnLine + FIELD_01_OFFSET, papsnzArg[DIRECTORY_NUMBER_INDEX] + i, FIELD_01_LEN);
    }
  else
    {
      memset(lasnLine + FIELD_01_OFFSET, ' ', FIELD_01_LEN);
      memcpy(lasnLine + FIELD_01_OFFSET, papsnzArg[DIRECTORY_NUMBER_INDEX], n);
    }

  /*
   * FIELD 02 - 8 characters of date in format YYYYMMDD
   */

  memcpy(lasnLine + FIELD_02_OFFSET,     papsnzArg[CALL_DATE_INDEX], 4);
  memcpy(lasnLine + FIELD_02_OFFSET + 4, papsnzArg[CALL_DATE_INDEX] + 5, 2);
  memcpy(lasnLine + FIELD_02_OFFSET + 6, papsnzArg[CALL_DATE_INDEX] + 8, 2);

  /*
   * FIELD 03 - 6 characters in format HHMMSS
   */

  memcpy(lasnLine + FIELD_03_OFFSET,     papsnzArg[CALL_TIME_INDEX], 2);
  memcpy(lasnLine + FIELD_03_OFFSET + 2, papsnzArg[CALL_TIME_INDEX] + 3, 2);
  memcpy(lasnLine + FIELD_03_OFFSET + 4, papsnzArg[CALL_TIME_INDEX] + 6, 2);

  /*
   * FIELD 04 - 2 fixed charaters 
   */

  memcpy(lasnLine + FIELD_04_OFFSET, "00", FIELD_04_LEN);

  /*
   * FIELD 05 - 1 character
   */

  memcpy(lasnLine + FIELD_05_OFFSET,    papsnzArg[ROAMING_IND_INDEX], FIELD_05_LEN);
 
  /*
   * FIELD 06 - 16 characters of dialed digits from the front of the dialed digits string
   */
  
  memset(lasnLine + FIELD_06_OFFSET, ' ', FIELD_06_LEN);
  n = strlen(papsnzArg[CALLED_NUMBER_INDEX]);
  memcpy(lasnLine + FIELD_06_OFFSET, papsnzArg[CALLED_NUMBER_INDEX], n);

  /*
   * FIELD 07 - 12 characters of destination from the front of the string that may be longer than 12
   */
  
  memset(lasnLine + FIELD_07_OFFSET, ' ', FIELD_07_LEN);
  n = strlen(papsnzArg[ZONE_DES_INDEX]);
  if (n < FIELD_07_LEN)
    {
      memcpy(lasnLine + FIELD_07_OFFSET, papsnzArg[ZONE_DES_INDEX], n);
    }
  else
    {
      memcpy(lasnLine + FIELD_07_OFFSET, papsnzArg[ZONE_DES_INDEX], FIELD_07_LEN);
    }
  
  /*
   * FIELD 08
   */

  strcpy(lasnzBuf, papsnzArg[DURATION_INDEX]);
  sscanf(lasnzBuf, "%d", &loiVal);
  sprintf(lasnzBuf, "%06.01lf", (double)loiVal + DOUBLE_ROUNDING);
  memcpy(lasnLine + FIELD_08_OFFSET, lasnzBuf, FIELD_08_LEN);

  /*
   * FIELD 09
   */
  
  strcpy(lasnzBuf, papsnzArg[LOCAL_PAYMENT_INDEX]);
  fovdTranslate(lasnzBuf, ',', '.');
  sscanf(lasnzBuf, "%lf", &loflVal);
  sprintf(lasnzBuf, "%09.02lf", loflVal + DOUBLE_ROUNDING);
  memcpy(lasnLine + FIELD_09_OFFSET, lasnzBuf, FIELD_09_LEN);

  /*
   * FIELD 10
   */

  strcpy(lasnzBuf, papsnzArg[INTER_PAYMENT_INDEX]);
  fovdTranslate(lasnzBuf, ',', '.');
  sscanf(lasnzBuf, "%lf", &loflVal);
  sprintf(lasnzBuf, "%09.02lf", loflVal + DOUBLE_ROUNDING);
  memcpy(lasnLine + FIELD_10_OFFSET, lasnzBuf, FIELD_10_LEN);

  /*
   * FIELD 11
   */

  strcpy(lasnzBuf, papsnzArg[SUMMARY_PAYMENT_INDEX]);
  fovdTranslate(lasnzBuf, ',', '.');
  sscanf(lasnzBuf, "%lf", &loflVal);
  sprintf(lasnzBuf, "%010.02lf", loflVal + DOUBLE_ROUNDING);
  memcpy(lasnLine + FIELD_11_OFFSET, lasnzBuf, FIELD_11_LEN);
  
  /*
   * FIELD 12
   */

  
  sprintf(lasnzBuf, "%07.02lf", (loflVal * stBgh.soflItbTaxRate) + DOUBLE_ROUNDING);
  memcpy(lasnLine + FIELD_12_OFFSET, lasnzBuf, FIELD_12_LEN);

  /*
   * FIELD 13
   */
  
  lasnzBuf[2] = '\0';
  memcpy(lasnzBuf, papsnzArg[CALL_TIME_INDEX], 2);
  sscanf(lasnzBuf, "%d", &loiVal);

  if (loiVal >= 7 && loiVal < 21)
    {
      lasnLine[FIELD_13_OFFSET] = 'S';
    }
  else
    {
      lasnLine[FIELD_13_OFFSET] = 'P';
    }

  /*
   * FIELD 14
   */

  if (loiVal >= 8 && loiVal < 15)
    {
      lasnLine[FIELD_14_OFFSET] = 'S';
    }
  else
    {
      lasnLine[FIELD_14_OFFSET] = 'P';
    }

  /*
   * Fill output buffer
   */

  memcpy(ppsnzLine, lasnLine, FIXED_LINE_LEN );
  ppsnzLine[FIXED_LINE_LEN ] = '\n';
  ppsnzLine[FIXED_LINE_LEN + 1] = '\0';

  fovdPopFunctionName ();  
  return TRUE;
}

static void fovdTranslate(char *ppsnzBuf, char pochFromChr, char pochToChr)
{
  int i, n = strlen(ppsnzBuf);

  for (i = 0; i < n; i++)
    {
      if (ppsnzBuf[i] == pochFromChr)
        {
          ppsnzBuf[i] = pochToChr;
        }
    }
}

