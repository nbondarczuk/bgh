#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <string.h>

#include "bgh.h"
#include "types.h"
#include "date.h"

#define DATE_BUF_LEN 8
#define DATE_YYYYMMDD_LEN 8
#define DATE_YYMMDD_LEN 6
#define YYYY_OFF 0
#define YYYY_LEN 4
#define YY_OFF 0
#define YY_LEN 2
#define MM_OFF 4
#define MM_LEN 2
#define DD_OFF 6
#define DD_LEN 2
#define CENT_OFF 1900
#define DECEMBER_CODE 11

int d[12][2] = 
{
  {31, 31},/* January */
  {28, 29},/* February */
  {31, 31},/* March */
  {30, 30},/* April */  
  {31, 31},/* May */  
  {30, 30},/* June */  
  {31, 31},/* July */  
  {31, 31},/* August */  
  {30, 30},/* September */  
  {31, 31},/* October */  
  {30, 30},/* November */  
  {31, 31} /* December */  
};

#if 0
int main(int argc, char *argv[])
{
    int noofdays, i;
	time_t t1,t2;
	clock_t ct1,ct2;
	char InDate[16], OutDate[16];
	if( argc > 0 )
	strcpy(InDate,argv[1]);
	printf("we %s\n",InDate);
	ct1 = clock();
/*	for(i=1;i<=100000;i++) */
		noofdays = foiDate_AddMonth(InDate,  OutDate);
    ct2 = clock(); 
/*	printf("used %ld seconds\n", (long*)((ct2-ct1)/CLOCKS_PER_SEC)); */ 
	printf("wy %s %d\n",OutDate, noofdays);
	return 0;
}
#define fovdPrintLog printf
#endif
/**************************************************************************************************
 *
 * fpstDate_New      
 *
 * DESCRIPTION: Parse given date in the string with format YYYYMMDD to the internal
 *              UNIX format hidden in the structur.
 *
 **************************************************************************************************
 */

tostDate *fpstDate_New(char *ppsnzStr)
{
  static char lasnzYYYY[DATE_BUF_LEN], lasnzMM[DATE_BUF_LEN], lasnzDD[DATE_BUF_LEN];
  struct tm tm;
  time_t time;
  int n;
  struct tostDate *lpstDate;
  int p, loiYear;

  assert (ppsnzStr != NULL);
  
  memset(lasnzYYYY, 0, DATE_BUF_LEN);
  memset(lasnzMM, 0, DATE_BUF_LEN);
  memset(lasnzDD, 0, DATE_BUF_LEN);
  
  if ((n = strlen(ppsnzStr)) == DATE_YYYYMMDD_LEN)
    {
      memcpy(lasnzYYYY, ppsnzStr, 4);
      memcpy(lasnzMM, ppsnzStr + 4, 2);
      memcpy(lasnzDD, ppsnzStr + 6, 2);
    }
  else if (n == DATE_YYMMDD_LEN)
    {
      strcpy(lasnzYYYY, "19");
      memcpy(lasnzYYYY + 2, ppsnzStr, 2);
      memcpy(lasnzMM, ppsnzStr + 2, 2);
      memcpy(lasnzDD, ppsnzStr + 4, 2);
    }
  else
    {
      fovdPrintLog (LOG_DEBUG, "fpstDate_New: Incorrect date format: %s\n", ppsnzStr);
      return NULL;
    }

  memset(&tm, 0 , sizeof(struct tm));

  tm.tm_isdst = -1;
  tm.tm_sec = 0;
  tm.tm_min = 0;
  tm.tm_hour = 2;
  n = sscanf(lasnzDD, "%d", &tm.tm_mday);
  n = sscanf(lasnzMM, "%d", &tm.tm_mon);
  tm.tm_mon--;
  n = sscanf(lasnzYYYY, "%d", &tm.tm_year);
  tm.tm_year -= CENT_OFF;

  if (tm.tm_year < 0)
    {
      return NULL;
    }

  if (tm.tm_mon > 11 || tm.tm_mon < 0)
    {
      return NULL;
    }

  loiYear = tm.tm_year + 1900;
  if (((loiYear % 4 == 0) && ((loiYear % 100) != 0)) || (loiYear % 1000 == 0))
    {
      p = 1;
    }
  else
    {
      p = 0;
    }

  if (tm.tm_mday > d[tm.tm_mon][p] || tm.tm_mday < 1)
    {
      return NULL;
    }

  if ((time = mktime(&tm)) == -1)
    {
      fovdPrintLog (LOG_DEBUG, "fpstDate_New: Can't make time\n");
      return NULL;
    }

  if ((lpstDate = (tostDate *)calloc(1, sizeof(tostDate))) == NULL)
    {
      fovdPrintLog (LOG_DEBUG, "fpstDate_New: Can't malloc memory\n");
      return NULL;
    }

  lpstDate->time = time;
  memcpy(&(lpstDate->tm), &tm, sizeof(struct tm));

  return lpstDate;
}

/**************************************************************************************************
 *
 * foenDate_Print
 *
 * DESCRIPTION: Show given parsed date in the format YYYYMMDD
 *
 **************************************************************************************************
 */

toenBool foenDate_Print(tostDate *ppstDate, char *pasnzBuf)
{
  char lasnzBuf[DATE_BUF_LEN];

  sprintf(lasnzBuf, "%04d", ppstDate->tm.tm_year + CENT_OFF);
  memcpy(pasnzBuf, lasnzBuf, 4);

  sprintf(lasnzBuf, "%02d",  ppstDate->tm.tm_mon + 1);
  memcpy(pasnzBuf + 4, lasnzBuf, 2);

  sprintf(lasnzBuf, "%02d",  ppstDate->tm.tm_mday);
  memcpy(pasnzBuf + 6, lasnzBuf, 2);  

  return TRUE;
}

/**************************************************************************************************
 *
 * foenDate_DaysBetween
 *
 * DESCRIPTION: Return the number of days between two parsed dates.
 *
 **************************************************************************************************
 */

int foiDate_DaysBetween(tostDate *ppstDateA, tostDate *ppstDateB)
{
  return (ppstDateA->time - ppstDateB->time) / (60 * 60 * 24);
}

/**************************************************************************************************
 *
 * foenDate_AddDays
 *
 * DESCRIPTION: To the parsed date add given number of days changing input argument value.
 *
 **************************************************************************************************
 */

toenBool foenDate_AddDays(tostDate *ppstDate, int poiDays)
{
  struct tm *tm;

  ppstDate->time += (poiDays * 60 * 60 *24);
  tm = localtime(&(ppstDate->time));
  memcpy(&(ppstDate->tm), tm, sizeof(struct tm));

  return TRUE;
}

/**************************************************************************************************
 *
 * foiDate_AddMonth
 *
 * DESCRIPTION: To the given date in string format YYYYMMDD add one month
 *              returning result as the string with output date and value being the number 
 *              of days between two dates:input and output.
 *
 **************************************************************************************************
 */

int foiDate_AddMonth(char *ppsnzDate, char *ppsnzOutDate)
{
  tostDate *lpstDate, *lpstStartDate;
  int loiDays;
  toenBool loenStatus;
  int p, loiYear;

  /*
   * Produce date one month later
   */
  
  if ((lpstDate = fpstDate_New(ppsnzDate)) == NULL)
    {
      fovdPrintLog (LOG_DEBUG, "foiDate_AddMonth: Can't parse date: %s\n", ppsnzDate);
      return -1;
    }

  /* leap year */
  loiYear = lpstDate->tm.tm_year + 1900;
  if (((loiYear % 4 == 0) && ((loiYear % 100) != 0)) || (loiYear % 1000 == 0))
    {
      p = 1;
    }
  else
    {
      p = 0;
    }

  /* day in the next month */
  if ((lpstDate->tm.tm_mday ) > d[(lpstDate->tm.tm_mon + 1) % 12][p])
    {
      lpstDate->tm.tm_mday = d[(lpstDate->tm.tm_mon + 1) % 12][p];
    }

  /* next month */
  lpstDate->tm.tm_mon = (lpstDate->tm.tm_mon + 1) % 12;

  /* next year if January */
  if (lpstDate->tm.tm_mon == 0)
    {
      lpstDate->tm.tm_year++;
    }
  
  if ((lpstDate->time = mktime(&(lpstDate->tm))) == -1)
    {
      fovdPrintLog (LOG_DEBUG, "foiDate_AddMonth: Can't make time\n");
      return -2;
    }

  /*
   * Prepare number of days between two dates to be returned as value
   * of the function
   */

  if ((lpstStartDate = fpstDate_New(ppsnzDate)) == NULL)
    {
      fovdPrintLog (LOG_DEBUG, "foiDate_AddMonth: Can't parse date: %s\n", ppsnzDate);
      return -3;
    }

  loiDays = foiDate_DaysBetween(lpstDate, lpstStartDate);

  /*
   * Show output date
   */

  memset(ppsnzOutDate, 0, 16);
  if ((loenStatus = foenDate_Print(lpstDate, ppsnzOutDate)) == FALSE)
    {
      fovdPrintLog (LOG_DEBUG, "foiDate_AddMonth: Can't print date\n");
      return -4;
    }

  free(lpstDate);
  free(lpstStartDate);

  return loiDays;
}

/**************************************************************************************************
 *
 * foenDate_AddDay
 *
 * DESCRIPTION: Adding one day to the given date and returning result as string in the
 *              output variable.
 *
 **************************************************************************************************
 */

toenBool foenDate_AddDay(char *ppsnzDate, char *ppsnzOutDate)
{
  tostDate *lpstDate;
  toenBool loenStatus;

  if ((lpstDate = fpstDate_New(ppsnzDate)) == NULL)
    {
      return FALSE;
    }

  if ((loenStatus = foenDate_AddDays(lpstDate, 1)) == FALSE)
    {
      return FALSE;
    }

  if ((loenStatus = foenDate_Print(lpstDate, ppsnzOutDate)) == FALSE)
    {
      return FALSE;
    }
  
  free(lpstDate);

  return TRUE;
}
