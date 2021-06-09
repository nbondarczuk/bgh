#if 0	/* just for version.sh */
static char *SCCS_VERSION = "1.1";
#endif


typedef enum toenRecordType 
{
  RECORD_USAGE = 0, 
  RECORD_ACCESS = 1, 
  RECORD_SERVICE = 2, 
  RECORD_OTHER = 3
} toenRecordType;

typedef struct tostPayment 
{
  char sasnzMonetaryAmount[MAX_BUFFER];
  char sasnzCurrency[MAX_BUFFER];
} tostPayment;

typedef struct tostUsageRecord {
  char sasnzUsageType[MAX_BUFFER];
  char sasnzConnectionType[MAX_BUFFER];
  char sasnzPriceId[MAX_BUFFER];
  char sasnzTariffTime[MAX_BUFFER];
  char sasnzTariffZone[MAX_BUFFER];
  char sasnzPLMN[MAX_BUFFER];
  char sasnzTariffModel[MAX_BUFFER];
  char sasnzServiceCode[MAX_BUFFER];
  char sasnzServiceName[MAX_BUFFER];
  char sasnzServicePackage[MAX_BUFFER];
  char sasnzQuantity[MAX_BUFFER];
  char sasnzMonetaryAmount[MAX_BUFFER];
  char sasnzCurrency[MAX_BUFFER];
} tostUsageRecord;

typedef struct tostAccessRecord {
  char sasnzAccessType[MAX_BUFFER];
  char sasnzChargeType[MAX_BUFFER];
  char sasnzMarket[MAX_BUFFER];
  char sasnzTariffModel[MAX_BUFFER];
  char sasnzServicePackage[MAX_BUFFER];    
  char sasnzServiceCode[MAX_BUFFER];
  char sasnzServiceName[MAX_BUFFER];
  char sasnzQuantity[MAX_BUFFER];
  char sasnzMonetaryAmount[MAX_BUFFER];
  char sasnzCurrency[MAX_BUFFER];
} tostAccessRecord;

typedef struct tostServiceRecord {
  char sasnzTariffModel[MAX_BUFFER];
  char sasnzServicePackage[MAX_BUFFER];    
  char sasnzServiceCode[MAX_BUFFER];
  char sasnzServiceName[MAX_BUFFER];
  char sasnzQuantity[MAX_BUFFER];
  char sasnzMonetaryAmount[MAX_BUFFER];
  char sasnzCurrency[MAX_BUFFER];
} tostServiceRecord;

typedef struct tostOtherRecord {
  char sasnzTariffModel[MAX_BUFFER];
  char sasnzServicePackage[MAX_BUFFER];    
  char sasnzServiceCode[MAX_BUFFER];
  char sasnzServiceName[MAX_BUFFER];
  char sasnzQuantity[MAX_BUFFER];
  char sasnzMonetaryAmount[MAX_BUFFER];
  char sasnzCurrency[MAX_BUFFER];
} tostOtherRecord;

typedef struct tostCallRecord {
  char sasnzServiceNameShdes[MAX_BUFFER];
  char sasnzServiceNameDes[MAX_BUFFER];
  char sasnzCallType[MAX_BUFFER]; 
  char sasnzType[MAX_BUFFER];
  char sasnzDate[MAX_BUFFER];
  char sasnzTime[MAX_BUFFER];
  char sasnzPLMN[MAX_BUFFER];
  char sasnzCountry[MAX_BUFFER];
  char sasnzNumber[MAX_BUFFER];
  char sasnzDuration[MAX_BUFFER];
  char sasnzLocalPayment[MAX_BUFFER];
  char sasnzInternationalPayment[MAX_BUFFER];
  char sasnzSummaryPayment[MAX_BUFFER];
  char sasnzCurrency[MAX_BUFFER];
  char sasnzDirection[MAX_BUFFER];
  char sasnzThreshold[MAX_BUFFER];
  char sasnzConnectionType[MAX_BUFFER];
} tostCallRecord;









