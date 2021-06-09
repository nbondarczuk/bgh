#if 0   /* just for version.sh */
static char *SCCS_VERSION = "4.1";
#endif

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#define IN
#define OUT


#define EOS '\0'
#define DOT '.'
#define EOL NULL

#define EQ(a, b) !strcmp(a, b)
#define NOT(a) ((a) == FALSE)

typedef enum 
{ 
  ERROR = -1, 
  FALSE = 0, 
  TRUE = 1 

} toenBool;

typedef enum toenItemType
{ 
  ACCESS_TYPE, 
  USAGE_TYPE, 
  SUBS_TYPE, 
  OCC_TYPE, 
  INTERVAL_TYPE

} toenItemType;


typedef enum toenPLMNType 
{
  VPLMN_TYPE,
  HPLMN_TYPE

} toenPLMNType;


#define MAX_BUFFER 512
#define FIELD_SIZE 128
#define SHDES_SIZE 16
#define DES_SIZE 128
#define DATE_SIZE 16

#define UNIT_SIZE 16
#define CALL_SEQ_SIZE 512 
#define TIME_SIZE 32
#define CGI_SIZE 32
#define DIRECTORY_NUMBER_SIZE 32
#define DEST_ZONE_SIZE 64
#define CURRENCY_SIZE 16
#define LOCATION_SIZE 72
#define TARIFF_TIME_SHDES_LEN 6

typedef struct tostPaymentTypeBuf
{
  char sasnzPaymentId[MAX_BUFFER];
  char sasnzPaymentCode[MAX_BUFFER];
  char sasnzPaymentName[MAX_BUFFER];

} tostPaymentTypeBuf;


