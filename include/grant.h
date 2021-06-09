#define GRANT_ENV_LABEL "GRANT_ENV"
#define GRANT_GL_LABEL "GRANT_GL"
#define MAX_ULFIELD_NAME_LEN 40
#define MAX_INFO_FILED_LEN 64
#define MAX_EXP_DATE_LEN 16

typedef struct tostGrantProfile
{
  int soiCustomerId;
  toenBool soenValid;
  
  char sasnzExpDate[MAX_EXP_DATE_LEN];
  double soflThresholdVal;
  double soflCalcFactor;

} tostGrantProfile;

typedef struct tostGrantEnv
{
  /*
   * Scope 1
   */

  int soiPriceGroup;

  /*
   * Scope 2
   */

  char sasnzExpDateFieldName      [MAX_ULFIELD_NAME_LEN];
  char sasnzThresholdValFieldName [MAX_ULFIELD_NAME_LEN];
  char sasnzCalcFactorFieldName   [MAX_ULFIELD_NAME_LEN];
  
  /*
   * Scope 3
   */
  
  int soiReasonCode;
  char sasnzPostPeriod[16];

  int soiGLCash;
  int soiGLDis;

} tostGrantEnv;

int foiGrant_LoadConfig();
toenBool foenGrant_Check(int poiPriceGroup);
int foiGrant_Assign(int poiCustomerId, char *ppsnzSettlPerEndDate, double *ppflBGHInvAmount);
