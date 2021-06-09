/*
 * memory handling
 */

#define FILL_LEN                  8

#define MAX_BUFFER_SIZE           128
#define TMP_BUFFER_SIZE           16
#define INIT_STATIC_BUF(buf) memset(buf, 0, MAX_BUFFER_SIZE)

/*
 * Static invoice categories handling
 */

#define ENC_CAT_TAB_SIZE            6

#define  ENC_UNKNOWN_CAT            -1
#define  ENC_LOCAL_USAGE_CAT        0 /* LIN.7140 = *U && PIA.7140 = (A | I | C).?.?.?* */
#define  ENC_INTER_USAGE_CAT        1 /* LIN.7140 = *U && PIA.7140 = (A | I | C).?.?.?*M? */
#define  ENC_ROAMING_LEG_USAGE_CAT  2 /* LIN.7140 = *U && PIA.7140 = R.?.?.?* */
#define  ENC_VPLMN_USAGE_CAT        3 /* LIN.7140 = *U && PIA.7140 = (A | I | C | R).(r | m).* */
#define  ENC_SURCHARGE_USAGE_CAT    4 /* LIN.7140 = *U && PIA.7140 = (A | I | C | R).R.* */
#define  ENC_LAND_USAGE_CAT         5

static char *gasnzServStr[ENC_CAT_TAB_SIZE] = 
{
  "ENC_LOCAL_USAGE_CAT",
  "ENC_INTER_USAGE_CAT",
  "ENC_ROAMING_LEG_USAGE_CAT",
  "ENC_VPLMN_USAGE_CAT",
  "ENC_SURCHARGE_USAGE_CAT",
  "ENC_LAND_USAGE_CAT"
};



  
static char *gasnzServLabel[ENC_CAT_TAB_SIZE] =
{
  "Po\263" "\261" "czenia krajowe",
  "Po\263" "\261" "czenia mi\xea" "dzynarodowe",
  "Po\263" "\261" "czenia odebrane w roamingu",
  "Op\263" "aty zagraniczne z tytu\263" "u roamingu",
  "Op\263" "aty administracyjne z tytu\263" "u roamingu",
  "Op\263" "aty obce za po\263" "\261" "czenia mi\xea" "dzynarodowe"
};  



/*
 * Contract Service
 */

typedef struct tostConServ
{
  int         soiServNo;
  char        sasnzLabel[MAX_BUFFER_SIZE];
  double      soflNet;
  int         soiItems;
  struct s_group_99 *spstG99;
    
} tostConServ;

typedef struct tostConServTab
{
  tostConServ sastConServ[ENC_CAT_TAB_SIZE];
  
} tostConServTab;

tostConServTab *fpstConServTab_New(struct s_group_22 *g_22, 
                                   tostItemCallList *ppstCallList_Local, 
                                   tostItemCallList *ppstCallList_Roaming);

toenBool foenConServTab_Gen(tostConServTab *ppstTab);

toenBool foenConServTab_Delete(tostConServTab *ppstTab);

int foiConServTab_Size(tostConServTab *ppstTab);
