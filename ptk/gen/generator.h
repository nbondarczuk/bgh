#define UNDEFINED_BILL_MEDIUM -1

#define MAX_POSTSCRIPT_ARG 32
#define MAX_GEN_ARG_LEN 256
#define MAX_GEN_LINE_LEN 1024

#ifndef GEN_VER
#define GEN_VER ""
#endif
#ifndef BGH_VER
#define BGH_VER ""
#endif

/*
 * This is type of generator
 */

typedef enum
{
  GEN_DOC_TYPE  = 0,
  GEN_INV_TYPE  = 1
  
} toenGenType;

/*
 * This is the definition of interface between BGH environment and the module
 */

typedef enum 
{
  DOC_UNDEFINED_TYPE = 0xff,
  
  DOC_INV_TYPE       = 0x00,
  DOC_DNL_TYPE       = 0x01,
  DOC_WLL_TYPE       = 0x02,
  DOC_INL_TYPE       = 0x03,
  DOC_INP_TYPE       = 0x04,
  DOC_ITBEXCEL_TYPE  = 0x05,
  DOC_ITBFIXED_TYPE  = 0x06  

} toenDocType;

/*
 * This is the definition of BGH mode of work
 */

typedef enum
{
  GEN_SINGLE_MODE  = 0,
  GEN_MULTI_MODE   = 1,
   
} toenGenMode;

/*
 * This is the definition of allowed types of invoices produced with given bill media
 */

#define INV_TYPES_NO 5

typedef enum
{
  INV_UNDEFINED_TYPE = 0xff,

  INV_MINUS_TYPE     = 0x00, /* Invoice with negative amount, Credit Memo */
  INV_MINUSVAT_TYPE  = 0x01, /* Invoice with positive amount, but VAT amount is less than 0 */ 
  INV_DEFAULT_TYPE   = 0x02, /* */
  INV_ENCLOSURE_TYPE = 0x03, /* */
  INV_PAYMENT_TYPE   = 0x04  /* Was supposed to be implemented in CR100 but ... */
  
} toenInvType;

/*
 * This is disk type
 */

#define DISK_TYPES_NO 2

typedef enum
{
  DISK_EXCEL_FORMAT = 0,
  DISK_FIXED_FORMAT = 1

} toenDiskType;

#define DISK_EXCEL_BILL_MEDIUM 2
#define DISK_FIXED_BILL_MEDIUM 3

/*
 * This is generator for non bill media documents like DNL, WLL, INP, INL
 */

typedef struct tostDocGen 
{
  toenGenType soenGen;

  tostStream *spstStream;

} tostDocGen;

/*
 * This is generator for bill media documents like INV
 */

typedef struct tostStreamTab
{
  int soiTabSize;
  tostStream **sapstStream;

} tostStreamTab;

typedef struct tostStreamSeq
{
  tostStream *spstStream;
  struct tostStreamSeq *spstNext;

} tostStreamSeq;

typedef struct tostInvGen 
{
  toenGenType soenGen;

  tostStreamTab *sapstStreamTab[INV_TYPES_NO];  
  tostSingleStream *sapstItbStream[DISK_TYPES_NO];
  tostImageStream *spstImage;
  tostStreamSeq *spstUsedStreams;
  
  /*
   * Dynamic part
   */

  tostStream *spstStream;
  tostSingleStream *spstItbStream;
  int soiCustomerId;
  int soiBillMedium;
  char sasnzCustCode[MAX_CUSTCODE_LEN];      
  int soiImgTypeId;

} tostInvGen;
