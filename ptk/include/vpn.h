#ifndef _VPN_
/* for debugging only */
/*
#define fovdPrintLog fprintf
#define LOG_NORMAL stdout
*/
#define _FOVDGEN_
#define _VPN_

#define LEVEL(g_22, level) (EQ(g_22->lin->v_1222, level))
#define CATEGORY(g_22,ct) (EQ(g_22->imd->v_7009, "CT") && EQ(g_22->imd->v_7008a,ct))

#define VPN_EPSILON 0.00000001
#define VPN_SCALING_FACTOR 100
#define VPN_ZONE_PATTERN "VPN*"
#define VPN_SERV_DES_PATTERN "VPN*"
#define MAX_ACCOUNT_LEN 36 
#define MAX_CURRENCY_LEN 4 
#define MAX_DESCRIPTION_LEN 76 
#define MAX_MARKET_LEN 76 
#define MAX_NETWORK_LEN 76 
#define MAX_DIRNO_LEN 76 
#define MAX_VPN_ID_LEN 76 
#define MAX_BUFFER_LEN 32

enum toenVPN_SUM
{enNoVPN=0, enYesVPN, enSummary, enVPNSumPosMax};

enum toenVPN_ERR
{
    VPN_OK = 0,
    VPN_NO_CONTRACT,   /* group 22 on contract level (02 level) does not describe a contract */
    VPN_ERROR_NO_MOA,
    VPN_ERROR_NO_MEMORY,
    VPN_ERROR_OTHER
};

typedef struct tostVPNItem
{
    char soszMarket  [MAX_VPN_ID_LEN];
    char soszNetwork [MAX_VPN_ID_LEN];
    char soszDirNo   [MAX_VPN_ID_LEN];
    int soiStatus;
    long soilVPNUsageCharge;
    long soilNotVPNUsageCharge;
    long soilNotUsageCharge;
    long soilAllCharge;
    char soszCurrency[MAX_VPN_ID_LEN];

} tostVPNItem;

typedef struct tostVPNContractNode
{
    struct tostVPNContractNode *spstNext;
    struct tostVPNItem *spstItem;
} tostVPNContractNode;

typedef struct tostVPNData
{
    int soiNumberOfContracts;
    long soilVPNUsageCharge;
    long soilNotVPNUsageCharge;
    long soilNotUsageCharge;
    long soilAllCharge;
    char soszCurrency [MAX_VPN_ID_LEN];
} tostVPNData;

typedef struct tostVPNContractList
{
    int soiSubscriberStatus;
    char soszAccountNo[MAX_VPN_ID_LEN];
    struct tostVPNContractNode *spstFirst;
    struct tostVPNData sastSum[enVPNSumPosMax];
} tostVPNContractList;

typedef struct tostVPNSubscriberNode
{
    struct tostVPNSubscriberNode *spstNext;
    struct tostVPNContractList *spstCoList;
} tostVPNSubscriberNode;

typedef struct tostVPNSubscriberList
{
    int soiLASubscriberStatus;
    char soszLAccountNo[MAX_VPN_ID_LEN];
    struct tostVPNSubscriberNode *spstFirst;
    struct tostVPNData sastSum[enVPNSumPosMax];
} tostVPNSubscriberList;


int foiVPNContractList_Test(struct s_group_22 *ppstG22);


toenBool foenVPNSubscriberList_Gen(struct s_TimmInter *ppstSumTimm);
/* toenBool foenVPNSubscriberList_Gen(tostVPNSubscriberList *ppstList); */


#endif



