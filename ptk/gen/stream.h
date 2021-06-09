#define STREAM_TYPES_NO 12

typedef enum 
{
  STREAM_UNDEFINED_TYPE    = 0xff,

  STREAM_INV_TYPE          = 0x00, 
  STREAM_WLL_TYPE          = 0x01,
  STREAM_DNL_TYPE          = 0x02,
  STREAM_INL_TYPE          = 0x03,
  STREAM_INP_TYPE          = 0x04,
  STREAM_ENCLOSURE_TYPE    = 0x05,
  STREAM_INV_MINUS_TYPE    = 0x06,
  STREAM_INV_MINUSVAT_TYPE = 0x07,
  STREAM_ITBEXECEL_TYPE    = 0x08,
  STREAM_ITBFIXED_TYPE     = 0x09,
  STREAM_IMAGE_TYPE        = 0x0a,
  STREAM_PAYMENT_TYPE      = 0x0b

} toenStreamType;

static char *dasnzStreamTypePrefix[STREAM_TYPES_NO] =
{
  "inv",
  "wll",
  "dnl",
  "inl",
  "inp",
  "enc",
  "MINUS.inv",
  "MINUSVAT.inv",
  "",
  "",
  "image.",
  "pay"
};

#define MAX_PATH_NAME_LEN 256
#define STREAM_FILE_ACCESS_RIGHTS 0660
#define BILL_MEDIUM_PAD_CHAR '0'
#define MAX_BILL_MEDIUM_DES 16
#define MAX_CUSTCODE_LEN 64

typedef struct tostStream
{
  tostLineList *spstLineList;
  char sasnzPathName[MAX_PATH_NAME_LEN]; /* directory name of output stream */
  int sofilFile;                             /* file descriptor of open output stream */

} tostStream;

typedef struct tostImageStream
{
  toenStreamType soenStream;
  tostLineList *spstLineList;
  char sasnzPathName[MAX_PATH_NAME_LEN]; /* directory name of output stream */

  /*
   * Dynamic part
   */

  int sofilFile;                             /* file descriptor of open output stream */
  char *spszFileName;                       /* file name of the atual document */
  int soiCustomerId;                         /* CUSTOMER_ID */
  
} tostImageStream;

typedef struct tostSingleStream
{
  toenStreamType soenStream;  
  int soiBillMediumIndex; 
  tostLineList *spstLineList;
  char sasnzPathName[MAX_PATH_NAME_LEN]; /* directory name of output stream */

  /*
   * Dynamic part
   */

  int sofilFile;                             /* file descriptor of open output stream */
  char *spszFileName;                       /* file name of the atual document */
  int soiCustomerId;                         /* CUSTOMER_ID */
  char sasnzCustCode[MAX_CUSTCODE_LEN];      

} tostSingleStream;

typedef struct tostMultiStream
{
  toenStreamType soenStream;
  int soiBillMediumIndex; 
  tostLineList *spstLineList;
  char sasnzPathName[MAX_PATH_NAME_LEN];/* file name of output stream */

  /*
   * Dynamic part
   */

  int sofilFile;                            /* file descriptor of open output stream */
  off_t soilFileOffset;                     /* previous document beginning file offset */
  char *spszFileName;                       /* file name of the atual document */
  int soiCustomerId;                        /* actual CUSTOMER_ID */
  char sasnzCustCode[MAX_CUSTCODE_LEN];      
  int soiPID;                               /* PID of process using output stream */
  int soiSeqNo;                             /* number of file in sequence of files */
  int soiDocNo;                             /* number of document in one file */
  int soiMaxDocNo;                          /* max number of documents in one file */
    
} tostMultiStream;


