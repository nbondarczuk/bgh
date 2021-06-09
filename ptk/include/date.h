#define SECMIN 60
#define MINHOU 60
#define HOUDAY 24
#define SECDAY SECMIN * MINHOU * HOUDAY

typedef struct tostDate
{
  time_t time;
  struct tm tm;

} tostDate;

tostDate *fpstDate_New(char *ppsnzStr);
toenBool foenDate_Print(tostDate *ppstDate, char *lasznzBuf);
int foiDate_DaysBetween(tostDate *ppstDateA, tostDate *ppstDateB);
toenBool foenDate_AddDays(tostDate *ppstDate, int poiDays);
int foiDate_AddMonths(char *ppsnzDate, int poiMonths, char *ppsnzOutDate);
toenBool foenDate_AddDay(char *ppsnzDate, char *ppsnzOutDate);

