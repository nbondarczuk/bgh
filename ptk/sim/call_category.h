#define USAGE_CATEGORIES_NO  5

#define USAGE_CATEGORY_LOCAL 0
#define USAGE_CATEGORY_INTER 1
#define USAGE_CATEGORY_RLEG  2
#define USAGE_CATEGORY_VPLMN 3
#define USAGE_CATEGORY_SURCH 4

static char *dasnzCallCategoryLabel[5] = 
{
  "Po\xb3" "\xb1" "czenia krajowe",
  "Po\xb3" "\xb1" "czenia mi\xea" "dzynarodowe",
  "Po\xb3" "\xb1" "czenia odebrane w roamingu",
  "Op\xb3" "aty zagraniczne z tytu\xb3" "u roamingu",
  "Op\xb3" "aty administracyjne z tytu\xb3" "u roamingu"  
};

typedef struct tostCallCategory
{
  char sasnzLabel[DES_SIZE];
  double soflAmount;
  double soflSurchargeAmount;
  int soiCalls;
  
} tostCallCategory;

int foiCallCategory_Max();
toenBool foenCallCategory_Init(tostCallCategory *ppstCallCategory, int poiCategoryIndex);
toenBool foenCallCategory_Reinit(tostCallCategory *ppstCallCategory);
toenBool foenCallCategory_ParseCallSeq(struct s_group_99 *ppstG99, struct s_group_99 **pppstNextG99, 
                                       int *ppiCategoryIndex, tostCallCategory *pppstCallCategory);
toenBool foenCallCategory_Add(tostCallCategory *ppstLeft, tostCallCategory *ppstRight);
toenBool foenCallCategory_Gen(tostCallCategory *ppstCallCategory);

