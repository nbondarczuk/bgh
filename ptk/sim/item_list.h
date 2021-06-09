/*
 * ACCESS
 */

typedef struct tostAccessRecord 
{
  char sasnzMarket[MAX_BUFFER];
  char sasnzTariffModel[MAX_BUFFER];
  char sasnzServicePackage[MAX_BUFFER];    
  char sasnzService[MAX_BUFFER];
  
  char sasnzAccessType[MAX_BUFFER];
  char sasnzChargeType[MAX_BUFFER];
  
  char sasnzDays[MAX_BUFFER];
  char sasnzQuantity[MAX_BUFFER];
  char sasnzMonetaryAmount[MAX_BUFFER];
  char sasnzCurrency[MAX_BUFFER];
  char sasnzPrice[MAX_BUFFER];

} tostAccessRecord;

typedef struct tostItemAccess
{
  struct s_group_22 *spstG22;

  char sasnzTMShdes  [SHDES_SIZE];    /* from lin.v_7140.0 */
  char sasnzTMDes    [DES_SIZE];
  long soilTMVersion;                 /* from lin.v_7140.1 */
  char sasnzSPShdes  [SHDES_SIZE];    /* from lin.v_7140.2 */
  char sasnzSPDes    [DES_SIZE];   
  char sasnzSNShdes  [SHDES_SIZE];    /* from lin.v_7140.3 */
  char sasnzSNDes    [DES_SIZE];      /* from imd+SN */

  char sochChargeType;     /* from pia.v_7140.0 */
  char sochChargeSubtype;  /* from pia.v_7140.1 */
  char sochOverwriteInd;   /* from pia.v_7140.2 */
  
  int  soiDays;            /* from qty+107 */
  int  soiQuantity;
  double soflMoa;          /* from G23.moa+125 */
  char sasnzCurrency[CURRENCY_SIZE];
  double soflPrice;        /* from G25.pri */
  
  struct tostItemList *spstListInterval;

} tostItemAccess;

/*
 * INTERVAL
 */

typedef struct tostIntervalRecord 
{
  char sasnzState[MAX_BUFFER];
  char sasnzStartDate[MAX_BUFFER];
  char sasnzEndDate[MAX_BUFFER];

} tostIntervalRecord;

typedef struct tostIntervalItem
{
  struct s_group_22 *spstG22;
  
  char sochSign;
  char sochAccessSubtype;
  char sochState;
  char sasnzEndDate[DATE_SIZE];
  char sasnzStartDate[DATE_SIZE];

} tostItemInterval;

/*
 * USAGE
 */

typedef struct tostUsageRecord 
{
  char sasnzTariffModel[MAX_BUFFER];
  char sasnzServicePackage[MAX_BUFFER];  
  char sasnzService[MAX_BUFFER];

  char sasnzUsageType[MAX_BUFFER];
  char sasnzConnectionType[MAX_BUFFER];
  char sasnzVPLMN[MAX_BUFFER];
  char sasnzTariffTime[MAX_BUFFER];
  char sasnzTariffZone[MAX_BUFFER];

  char sasnzQuantity[MAX_BUFFER];
  char sasnzMonetaryAmount[MAX_BUFFER];
  char sasnzCurrency[MAX_BUFFER];
  char sasnzTime[MAX_BUFFER];
  char sasnzFullPrice[MAX_BUFFER]; 
  char sasnzUnitPrice[MAX_BUFFER];   
  char sasnzUnits[MAX_BUFFER]; 

} tostUsageRecord;


typedef struct tostItemUsage
{
  struct s_group_22 *spstG22;
  struct s_group_99 *spstG99;

  char sasnzTMShdes[SHDES_SIZE];
  long soilTMVersion;
  char sasnzSPShdes[SHDES_SIZE];
  char sasnzSNShdes[SHDES_SIZE];
  char sasnzSNDes[DES_SIZE];
  char sasnzVPLMN[SHDES_SIZE];
  char sochType;
  char sochRatingModel;
  long soilRIPVersion;
  long soilEGLPVersion;
  char sasnzTTShdes[SHDES_SIZE];
  char sasnzZNShdes[SHDES_SIZE];
  int  soiCalls;
  double soflMoa;
  char sasnzCurrency[CURRENCY_SIZE];

} tostItemUsage;


/*
 * ROAMING USAGE
 */

typedef struct tostItemRoamingUsage
{
  struct s_group_22 *spstG22;
  struct s_group_99 *spstG99;

  char sasnzTMShdes[SHDES_SIZE];
  long soilTMVersion;
  char sasnzSPShdes[SHDES_SIZE];
  char sasnzSNShdes[SHDES_SIZE];
  char sasnzSNDes[DES_SIZE];
  char sochType;  
  char sasnzVPLMN[SHDES_SIZE];  
  int  soiCalls;
  double soflMoa;
  char sasnzCurrency[CURRENCY_SIZE];

} tostItemRoamingUsage;


/*
 * SUBS
 */

typedef struct tostSubsRecord 
{
  char sasnzTariffModel[MAX_BUFFER];
  char sasnzServicePackage[MAX_BUFFER];  
  char sasnzService[MAX_BUFFER];
  char sasnzQuantity[MAX_BUFFER];
  char sasnzMonetaryAmount[MAX_BUFFER];
  char sasnzCurrency[MAX_BUFFER];
  char sasnzPrice[MAX_BUFFER];
} 
tostSubsRecord;

typedef struct tostItemSubs
{
  struct s_group_22 *spstG22;

  char sasnzTMShdes[SHDES_SIZE];
  long soilTMVersion;
  char sasnzSPShdes[SHDES_SIZE];
  char sasnzSNShdes[SHDES_SIZE];
  char sasnzSNDes[DES_SIZE];
  int  soiQuantity;
  double soflMoa;
  char sasnzCurrency[CURRENCY_SIZE];
  double soflPrice;

} tostItemSubs;


/*
 * OCC
 */

typedef struct tostOCCRecord 
{
  char sasnzTariffModel[MAX_BUFFER];
  char sasnzServicePackage[MAX_BUFFER];  
  char sasnzService[MAX_BUFFER];
  char sasnzQuantity[MAX_BUFFER];
  char sasnzPrice[MAX_BUFFER];
  char sasnzMonetaryAmount[MAX_BUFFER];
  char sasnzCurrency[MAX_BUFFER];
} 
tostOCCRecord;

typedef struct tostItemOCC
{
  struct s_group_22 *spstG22;
  
  char sasnzTMShdes[SHDES_SIZE];
  long soilTMVersion;
  char sasnzSPShdes[SHDES_SIZE];
  char sasnzSNShdes[SHDES_SIZE];
  char sasnzSNDes[DES_SIZE];
  char sasnzFE[DES_SIZE];  
  int  soiQuantity;
  double soflMoa;
  char sasnzCurrency[CURRENCY_SIZE];
  double soflPrice;

} tostItemOCC;


/*
 * ITEM
 */

typedef struct tostItem
{

  toenItemType soenType;
  void *spvItem;
  
} tostItem;

/*
 * LIST
 */

typedef struct tostItemNode
{
  struct tostItem *spstItem;  
  struct tostItemNode *spstNext;

} tostItemNode;

typedef struct tostItemList
{
  struct s_group_22 *spstG22;
  long soilCoId;
  double soflBCHSumUsage;
  double soflBCHSumAccess;
  double soflBCHSumOCC;
  double soflBCHSumSubscription;
  int soiSize;
  
  struct tostItemNode *spstRoot;
  
} tostItemList;


extern toenBool foenItemList_Gen(tostItemList *);
extern int foiItemList_Count(tostItemList *);
extern toenBool foenItemList_Apply(tostItemList *, toenBool (*)(tostItem *));
extern tostItemList *fpstItemList_New(struct s_group_22 *, toenItemType, struct s_group_22 *);
extern toenBool foenItemList_Insert(tostItemList *ppstList, tostItem *);
extern toenBool foenItemList_Delete(tostItemList *);
toenBool foenItemList_Sort(tostItemList *);
toenBool foenItemList_Init(tostItemList *ppstList, struct s_group_22 *ppstG22);



