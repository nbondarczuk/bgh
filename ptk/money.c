#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "bgh.h"
#include "protos.h"
#include "parser.h"
#include "types.h"
#include "strutl.h"

toenBool foenMoney_Eq(double poflValA, double poflValB)
{
  toenBool loenStatus;
  
  fovdPushFunctionName("foflMoney_Eq");

  if (poflValA == poflValB)
    {
      loenStatus = TRUE;
    }
  else
    {
      loenStatus = FALSE;
    }

  fovdPopFunctionName();
  return loenStatus;
}

double foflMoney_Add(double poflValA, double poflValB)
{
  double loflVal;

  fovdPushFunctionName("foflMoney_Add");

  loflVal = poflValA + poflValB;

  fovdPopFunctionName();
  return loflVal;
}

void fovdMoney_Sprintf(char *ppsnzBuffer, double poflVal)
{
  fovdPushFunctionName("foflMoney_Sprintf");
  
  sprintf(ppsnzBuffer, "%.2lf", foflRound(poflVal));
  
  fovdPopFunctionName();
}

double foflMoney_Round(double poflVal, double *ppflRest)
{
  double loflRoundedVal;
  char lasnzVal[64];
  int n;

  fovdPushFunctionName("foflMoney_Round");  
  fovdPrintLog(LOG_DEBUG, "foflMoney_Round_begin, %24.18f\n",poflVal);
  
  sprintf(lasnzVal, "%.2lf", foflRound(poflVal));
  n = sscanf(lasnzVal, "%lf", &loflRoundedVal);
  loflRoundedVal = foflRound(loflRoundedVal);
  
  *ppflRest = poflVal - loflRoundedVal;

  fovdPrintLog(LOG_DEBUG,"foflMoney_Round_end,   %24.18f\n",loflRoundedVal);
  fovdPopFunctionName();
  return loflRoundedVal;
}

#define MAX_MONEY_BUFFER 32

toenBool foenMoney_Scan(char *ppsnzBuf, signed long *ppilVal)
{
  char lasnzBuf[MAX_MONEY_BUFFER];
  int i, j, n, rc;
  
  memset(lasnzBuf, 0x00, MAX_MONEY_BUFFER);
  for (i = 0, j = 0, n = strlen(ppsnzBuf); i < n; i++)
    {
      if (ppsnzBuf[i] != '.')
        {
          lasnzBuf[j++] = ppsnzBuf[i];
        }
    }

  if ((rc = sscanf(lasnzBuf, "%ld", ppilVal)) == 0)
    {
      return FALSE;
    }
  
  return TRUE;
}
