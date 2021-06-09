#if 0	/* just for version.sh */
static char *SCCS_VERSION = "1.1";
#endif

int foiGetBillMedium(struct s_TimmInter *inter);

struct s_qty_seg *fpstFindQuantity(struct s_qty_seg *ppstQty, 
                                   char *ppchzDetails, 
                                   char *ppchzUnitType); 

struct s_moa_seg *fpstFindMainPaymentSegment(struct s_group_45 *ppstG45, 
                                             char *ppchzType,
                                             char *ppchzAmountTypeId); 

struct s_moa_seg *fpstFindPaymentSegment(struct s_group_23 *ppstG23, 
                                         char *ppchzType,
                                         char *ppchzAmountTypeId);

struct s_imd_seg *fpstFindItemDescription(struct s_imd_seg *ppstImd, 
                                          char *ppszDescr);

struct s_moa_seg *fpstFindTaxPaymentSegment(struct s_group_47 *ppstG47, 
                                            char *ppsnzId, 
                                            char *ppsnzType); 

