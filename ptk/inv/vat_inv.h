#define MOA_TYPE_TO_PAY  "4"
#define MOA_TYPE_EXACT   "5"
#define MOA_TYPE_ROUNDED "19"
#define MOA_TYPE_INFO    "9"

/*
 * Only one currency allowed
 */

#define LOCAL_CURRENCY_CODE "PLN"

/*
 * memory handling
 */

#define FILL_LEN                  8

#define MAX_BUFFER_SIZE           256
#define TMP_BUFFER_SIZE           16
#define INIT_STATIC_BUF(buf) memset(buf, 0, MAX_BUFFER_SIZE)

/*
 * Static invoice categories handling
 */

#define VAT_INV_CAT_SIZE                9

#define  VAT_INV_UNKNOWN_CAT            -1
#define  VAT_INV_SUBSCRIPTION_CAT       0 /* LIN.7140 = *S */
#define  VAT_INV_ACCESS_CAT             1 /* LIN.7140 = *A */
#define  VAT_INV_LOCAL_USAGE_CAT        2 /* LIN.7140 = *U && PIA.7140 = (A | I | C).?.?.?* */
#define  VAT_INV_INTER_USAGE_CAT        3 /* LIN.7140 = *U && PIA.7140 = (A | I | C).?.?.?*M? */
#define  VAT_INV_VPLMN_USAGE_CAT        4 /* LIN.7140 = *U && PIA.7140 = (A | I | C | R).(r | m).* */
#define  VAT_INV_ROAMING_LEG_USAGE_CAT  5 /* LIN.7140 = *U && PIA.7140 = R.?.?.?* */
#define  VAT_INV_SURCHARGE_USAGE_CAT    6 /* LIN.7140 = *U && PIA.7140 = (A | I | C | R).R.* */
#define  VAT_INV_LAND_USAGE_CAT         7
#define  VAT_INV_OCC_CAT                8 /* LIN.7140 = *O */

static char *gasnzServiceStr[VAT_INV_CAT_SIZE] = 
{
  "VAT_INV_SUBSCRIPTION_CAT",
  "VAT_INV_ACCESS_CAT",
  "VAT_INV_LOCAL_USAGE_CAT",
  "VAT_INV_INTER_USAGE_CAT",
  "VAT_INV_VPLMN_USAGE_CAT",
  "VAT_INV_ROAMING_LEG_USAGE_CAT",
  "VAT_INV_SURCHARGE_USAGE_CAT",
  "VAT_INV_LAND_USAGE_CAT",
  "VAT_INV_OCC_CAT"
};

static char *gasnzLabel[VAT_INV_CAT_SIZE] =
{
  "Op\263" "aty aktywacyjne",
  "Abonamenty (\263" "\261" "cznie)",
  "Po\263" "\261" "czenia krajowe",
  "Po\263" "\261" "czenia mi\352" "dzynarodowe",
  "Op\263" "aty zagraniczne z tytu\263" "u roamingu",
  "Po\263" "\261" "czenia odebrane w roamingu",
  "Op\263" "aty administracyjne z tytu\263" "u roamingu",
  "Op\263" "aty obce za po\263" "\261" "czenia mi\xea" "dzynarodowe",
  "Inne op\263" "aty"
};  

/*
 * Service
 */

#define TAX_NAME_SIZE       35
#define TAX_RATE_SIZE       16
#define TAX_LEGAL_CODE_SIZE 40

typedef struct tostService
{
  double      soflNetAmount;                                      /* MOA.v_5004 if MOA.v_5025 == "125" && MOA.v_4405 == "5" */
  char        sasnzTaxName      [TAX_NAME_SIZE + FILL_LEN];       /* G30.TAX.v_5152 if G30.TAX.v_5153 == "VAT" */
  char        sasnzTaxRate      [TAX_RATE_SIZE + FILL_LEN];       /* G30.TAX.v_5278 if G30.TAX.v_5153 == "VAT" */
  char        sasnzTaxLegalCode [TAX_LEGAL_CODE_SIZE + FILL_LEN]; /* G30.TAX.v_5279 if G30.TAX.v_5153 == "VAT" */
  double      soflTaxRate;
  double      soflTaxAmount;                                      /* G30.MOA.v_5004 if G30.MOA.v_5025 == "124" */
  double      soflBrutAmount;                                     /* MOA.v_5004 if MOA.v_5025 == "125" && MOA.v_4405 == "5" */
  int         soiItems;
  char        sasnzLabel[MAX_BUFFER];
  struct      tostService *spstNext;
  
} tostService;

typedef struct tostServiceCat
{
  int                 soiCat;
  int                 soiSize;
  struct tostService *spstFirst;
  
} tostServiceCat;

/*
 * VAT Sum
 */

typedef struct tostVATSumNode
{
  struct tostVATSumNode *spstNext;
  double soflNetAmount;
  char   sasnzTaxName      [TAX_NAME_SIZE + FILL_LEN];
  char   sasnzTaxRate      [TAX_RATE_SIZE + FILL_LEN];
  char   sasnzTaxLegalCode [TAX_LEGAL_CODE_SIZE + FILL_LEN];
  double soflVATRate;
  double soflVATAmount;
  double soflBrutAmount;
  double soflDiscountAmount;
  double soflScale;
  
} tostVATSumNode;

typedef struct tostVATSumList
{
  int                   soiSize;
  struct tostVATSumNode *spstFirst;

} tostVATSumList;

/*
 * VAT Inv
 */

typedef struct tostVATInv
{
  struct s_TimmInter *spstTimmInv;
  
  tostServiceCat sastCatTab   [VAT_INV_CAT_SIZE];      /* BGH categories */
  tostVATSumList sostSumList;                          /* entry for each VAT rate found     */

  double soflVATNetAmount;    /* rounded sum of values for VAT item list (BGH categories)   */
  double soflVATAmount;       
  double soflVATBrutAmount;

  double soflNoVATAmount;                      /* rounded sum of values for VPLMN charges   */
  
  double soflTotalNetAmount;                           /* r(VATNetAmount) + r(NoVATAmount)  */
  double soflTotalBrutAmount;                          /* r(VATBrutAmount) + r(NoVATAmount) */

  double soflBCH_TotalAmount;
  double soflBCH_VATAmount;
  double soflBCH_PaymentAmount;                        /* BCH amount from r(MOA+77)         */
  double soflBCH_RoundingDiff;                            /* BCH rounding diff from MOA+980 */

  double soflDiscountAmount;

} tostVATInv;

/*
 * VAT Inv methods
 */

tostVATInv *fpstVATInv_New(struct s_TimmInter *ppstInv);

toenBool foenVATInv_Delete(tostVATInv *ppstInv);

toenBool foenVATInv_Gen(tostVATInv *ppstInv);

toenBool foenVATInv_Process(tostVATInv *ppstVATInv, 
                            struct s_TimmInter *ppstInv);

toenBool foenVATInv_Print(tostVATInv *ppstVATInv);

/*
 * VAT Sum List methods
 */

toenBool foenVATSumList_Init(tostVATSumList *ppstList);

toenBool foenVATSumList_Add(tostVATSumList *ppstList, 
                            struct s_group_22 *g_22);

toenBool foenVATSumList_Gen(tostVATSumList *ppstList);

toenBool foenVATSumList_Save(tostVATSumList *ppstList);

toenBool foenVATSumList_Print(tostVATSumList *ppstList);

toenBool foenVATSumList_Sum(tostVATSumList *ppstList, 
                            double *ppflNet, 
                            double *ppflVAT, 
                            double *ppflBrut);

toenBool foenVATSumList_Round(tostVATSumList *ppstList,
                              double poflDiscAmount);

toenBool foenVATSumList_GenDisc(tostVATSumList *ppstList, 
                                double poflDiscountAmount);

/*
 * Service methods
 */



tostService *fpstService_New(struct s_group_22 *g_22);

tostService *fpstService_Find(tostService *ppstFirst, 
                              struct s_group_22 *g_22);

toenBool foenService_Add(tostService *ppstFirst, 
                         struct s_group_22 *g_22);

toenBool foenService_Gen(tostServiceCat *ppstCat, 
                         tostService *ppstServ);

toenBool foenService_Print(tostService *ppstFirst);

toenBool foenService_Load(struct s_group_22 *g_22, 
                          double *ppflNet,
                          double *ppflTax, 
                          double *ppflBrut, 
                          double *ppflDiscount);

toenBool foenServiceCat_Round(tostServiceCat *ppstCat, 
                              double *ppflRound);

/*
 * Service Category methods
 */

toenBool foenServiceCat_Init(tostServiceCat *ppstCat, 
                             int poiCat);

toenBool foenServiceCat_Add(tostServiceCat *pppstCat, 
                            struct s_group_22 *g_22, 
                            tostVATSumList *ppstList);

toenBool foenServiceCat_Delete(tostServiceCat *ppstCat);

toenBool foenServiceCat_Save(tostServiceCat *ppstCat,
                             int poiCatIndex);

toenBool foenServiceCat_Gen(tostServiceCat *ppstCat);

toenBool foenServiceCat_Print(tostServiceCat *ppstCat);

int foiServiceCat_Classify(struct s_group_22 *g_22);

toenBool foenServiceCat_IsVATItem(tostServiceCat *ppstCat);

toenBool foenServiceCat_IsEmpty(tostServiceCat *ppstCat);
