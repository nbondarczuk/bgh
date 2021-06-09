#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* User includes                */
#include "bgh.h"
#include "protos.h"

extern tostBGHStat gostBGHStat;  
static time_t startup_time;
static char szTemp[128];

void fovdTimer_Init()
{
  startup_time = time(NULL);
}

void fovdTimer_Show(long golCust)
{
  time_t actual_time;
  
  actual_time = time(NULL);
  fovdPrintLog (LOG_MAX, "\nProcessed %d customers in %ld sec.\n", golCust, actual_time - startup_time);
  fovdPrintLog (LOG_MAX, "Docs Created : %d\n", gostBGHStat.soiInvProcessed);
  fovdPrintLog (LOG_MAX, "Docs Rejected: %d\n", gostBGHStat.soiInvRejected);
}
