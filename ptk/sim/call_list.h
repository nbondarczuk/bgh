
/*
 * CALL RECORD
 */

typedef struct tostCallRecord 
{
  char sasnzDate[MAX_BUFFER];
  char sasnzTime[MAX_BUFFER];
  char sasnzServiceShdes[MAX_BUFFER];  
  char sasnzCallType[MAX_BUFFER];
  char sasnzPLMN[MAX_BUFFER];
  char sasnzDirectoryNumber[MAX_BUFFER];
  char sasnzDirection[MAX_BUFFER];
  char sasnzDuration[MAX_BUFFER];
  char sasnzLocalPayment[MAX_BUFFER];
  char sasnzInterPayment[MAX_BUFFER];
  char sasnzSummaryPayment[MAX_BUFFER];
  char sasnzCurrency[MAX_BUFFER];  

#ifdef US_TIMM_LAYOUT
  char soszOriginLoc[MAX_BUFFER];  
  char soszDestinLoc[MAX_BUFFER];  
  char soszTimeZone[MAX_BUFFER];  
#endif

} tostCallRecord;

typedef struct tostItemCall
{
  char sochCall; /* direction of the call: I - incbound, O - outbound */
  char sochPLMN; 
  char sochRLeg; /* if in the seq of connections RLeg part may be found then eq. R, else empty */ 
  char sasnzServiceShdes[SHDES_SIZE]; /* service name, sn + > for CFLeg or ODEBR, ODEBR* for roaming calls */
  char sasnzUnit[UNIT_SIZE];          /* name of unit for the call - Msg, Ev, Sec */
  char sasnzCallType[CALL_SEQ_SIZE];  /* call parts */ 
  char sasnzTime[TIME_SIZE];          /* date of a call */
  char sasnzPLMN[SHDES_SIZE];         /* PLMN name if roaming call, else empty */
  char sasnzDirectoryNumber[DIRECTORY_NUMBER_SIZE]; /* for inbound calls this field is empty */
  char sasnzDestZoneDes[DEST_ZONE_SIZE]; /* description of zone, empty for roaming calls,for int. calls must have format M? */
  toenBool soenIsDestZoneVPN; /* zone VPN indicator */
  int  soiDuration;                   /* duration of the call, for SMS set to 1 */
  double soflLocalPayment;
  double soflInterPayment;
  double soflTapNetPayment;
  double soflTapTaxPayment;
  char sasnzCurrency[CURRENCY_SIZE + 8];

#ifdef US_TIMM_LAYOUT
  char soszOriginLocation[LOCATION_SIZE];
  char soszDestinLocation[LOCATION_SIZE];
  char soszTimeZoneShdes[TARIFF_TIME_SHDES_LEN];
#endif

} tostItemCall;

typedef struct tostItemCallNode
{
  tostItemCall *spstItem;
  struct tostItemCallNode *spstNext;
  
} tostItemCallNode;

typedef struct tostItemCallList
{
  char soenPLMNType;
  toenPLMNType soenType;  
  tostItemCallNode *spstRoot;
  tostItemCallNode *spstLastNode;
  
  /*
   * for HPLMN originated calls
   */

  int soiCount;
  double soflAirPayment;
  double soflIntPayment;
  double soflSumPayment;

  /*
   * for calls not containing RLeg
   */

  int soiTAMOCCount;
  double soflTAMOCAirPayment;
  double soflTAMOCIntPayment;
  double soflTAMOCSumPayment;

  /*
   * for calls containing RLeg
   */

  int soiTAMTCCount;  
  double soflTAMTCAirPayment;
  double soflTAMTCIntPayment;
  double soflTAMTCSumPayment;
  
} tostItemCallList;


tostItemCallList *fpstItemCallList_New(struct s_group_99 *, toenPLMNType, char *ppsnzNumber);
toenBool foenItemCallList_Gen(tostItemCallList *, char *ppsnzNumber);
toenBool foenItemCallList_Count(tostItemCallList *);
toenBool foenItemCallList_Delete(tostItemCallList *);
