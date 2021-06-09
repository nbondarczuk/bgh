/**************************************************************************/
/*  MODULE : ROAMING data fetecher                                        */
/*                                                                        */
/*  AUTHOR : N.Bondarczuk                                                 */
/*                                                                        */
/*  CREATION DATE : 10.11.97                                              */
/*                                                                        */
/*  DESCRIPTION : Exports  functions accessing information from TIMM      */
/*                ROAMING   message.                                      */ 
/**************************************************************************/

#if 0	/* just for version.sh */
static char *SCCS_VERSION = "1.1";
#endif

typedef struct tostVPLMN 
{
  char laszName[MAX_BUFFER];  
  
  char laszNetInboundAmount[MAX_BUFFER];
  char laszNetInboundCurrency[MAX_BUFFER];

  char laszNetOutboundAmount[MAX_BUFFER];
  char laszNetOutboundCurrency[MAX_BUFFER];

  char laszChargedTaxName[MAX_BUFFER];
  char laszChargedTaxAmount[MAX_BUFFER];
  char laszChargedTaxCurrency[MAX_BUFFER];
  
  char laszUsageAmount[MAX_BUFFER];
  char laszUsageCurrency[MAX_BUFFER];
  
  char laszSurchargeAmount[MAX_BUFFER];
  char laszSurchargeCurrency[MAX_BUFFER];
  
} tostVPLMN;

int foiCountRoamingContracts(struct s_TimmInter *);

toenBool foenFetchRoamingSim(struct s_TimmInter *, int, char *, int);

int foiCountRoamingContractVPLMNs(struct s_TimmInter *, char *, int);

toenBool foenFetchRoamingContractVPLMN(struct s_TimmInter *, char *, int, int, tostVPLMN *);



