/**************************************************************************/
/*  MODULE : Enclosure Account Fetcher                                    */
/*                                                                        */
/*  AUTHOR : N.Bondarczuk                                                 */
/*                                                                        */
/*  CREATION DATE : 24.09.97                                              */
/*                                                                        */
/*  DESCRIPTION : Exports  function  creating tagged information          */
/*                necessary for creation of invoice                       */
/**************************************************************************/

#if 0	/* just for version.sh */
static char *SCCS_VERSION = "4.1.1";
#endif

/*
 * TIMM INV message header and trailer access
 */
int fetchInvoiceDate(struct s_TimmInter *, char *, int);
int fetchInvoiceCustomerName(struct s_TimmInter *, char *, int);
int fetchInvoiceCustomerAddress(struct s_TimmInter *, char *, int, char *, int, char *, int, char *, int);
int fetchInvoiceTemporaryName(struct s_TimmInter *, char *, int);
int fetchInvoiceTemporaryAddress(struct s_TimmInter *, char *, int, char *, int, char *, int, char *, int);
int fetchInvoiceNumber(struct s_TimmInter *, char *, int);
int fetchInvoiceCustomerAccountNo(struct s_TimmInter *, char *, int);
int fetchInvoiceNo(struct s_TimmInter *, char *, int);
int fetchInvoiceCustomerNIP(struct s_TimmInter *, char *, int);
int fetchInvoiceSellerName(struct s_TimmInter *, char *, int);
int fetchInvoiceSellerAddress(struct s_TimmInter *, char *, int, char *, int, char *, int, char *, int);
int fetchInvoiceSellerNIP(struct s_TimmInter *, char *, int);
int fetchInvoicePeriodEnd(struct s_TimmInter *, char *, int);
int fetchInvoicePeriodBegin(struct s_TimmInter *, char *, int);
int fetchInvoiceLetterDate(struct s_TimmInter *, char *, int); 
int fetchInvoiceAccessItem(struct s_TimmInter *, char *, int, char *, int);
int fetchInvoiceUsageItem(struct s_TimmInter *, char *, int, char *, int); 
int fetchInvoiceTotalNetValue(struct s_TimmInter *, char *, int, char *, int);
int fetchInvoiceTotalValue(struct s_TimmInter *, char *, int, char *, int);
int fetchInvoiceAmountToPay(struct s_TimmInter *, char *, int, char *, int);
int fetchInvoiceDueDate(struct s_TimmInter *, char *, int);
int fetchInvoiceBank(struct s_TimmInter *, char *, int, char *, int);
int fetchInvoiceTotalTax(struct s_TimmInter *, char *, int, char *, int);
int countInvoiceMarketingTexts(struct s_TimmInter *);
int fetchInvoiceMarketingText(struct s_TimmInter *, int, char pachzLine[5][MAX_BUFFER]);
int fetchInvoiceTaxExempt(struct s_TimmInter *, char *, int);

/*
 * TIMM INV message LIN block access
 */

int fetchInvoiceItemsNumber(struct s_TimmInter *);
int fetchInvoiceItemChargeType(struct s_TimmInter *, int);
int fetchInvoiceItemMarket(struct s_TimmInter *, int, char *, int, char *, int);
int fetchInvoiceItemTariffModel(struct s_TimmInter *, int, char *, int, char *, int);
int fetchInvoiceItemServicePackage(struct s_TimmInter *, int, char *, int, char *, int);
int fetchInvoiceItemService(struct s_TimmInter *, int, char *, int, char *, int);
int fetchInvoiceItemConnectionType(struct s_TimmInter *, int);
int fetchInvoiceItemUsageType(struct s_TimmInter *, int);
int fetchInvoiceItemBruttoValue(struct s_TimmInter *, int , char *, int, char *, int);
int fetchInvoiceItemPPUId(struct s_TimmInter *, int, char *, int);
int fetchInvoiceItemTariff(struct s_TimmInter *, int, char *, int, char *, int);
int fetchInvoiceItemTax(struct s_TimmInter *, int, char *, int, char *, int, char *, int, char *, int);
int fetchInvoiceItemQuantity(struct s_TimmInter *, int , char *, char *, int);
toenBool foenFetchInvoiceItem(struct s_TimmInter *inter, int, tostInvItem *);
toenBool foenInvFetch_CustomerGroup(struct s_TimmInter *, char *, int);
toenBool foenInvFetch_PaymentType(struct s_TimmInter *, char *, int);
