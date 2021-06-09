/**************************************************************************************************
 *                                                                                          
 * MODULE: SHDES2DES
 *                                                                                          
 * AUTHOR: N.Bondarczuk                                                
 * 
 * CREATION DATE : 30.04.98                                              
 *
 * MODIFICATION DATE 12.10.1999
 * 
 **************************************************************************************************
 */

#if 0	/* just for version.sh */
static char *SCCS_VERSION = "4.0";
#endif

char *fpsnzMapTZShdes2Des(char *);
char *fpsnzMapTTShdes2Des(char *);
char *fpsnzMapTMShdes2Des(char *);
char *fpsnzMapSPShdes2Des(char *);
char *fpsnzMapSNShdes2Des(char *);
char *fpsnzMapVPLMNShdes2Country(char *);
char *fpsnzMapPriceGroupCode2Name(char *);
toenBool foenMapPaymentCode2Type(char, tostPaymentTypeBuf *);
int foiMapMarketDes2Id(char *, int *);
int foiMapMarketDes2NetworkName(char *Market, char *Network);  /* ADDED 12.10.99 */
