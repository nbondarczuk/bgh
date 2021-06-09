toenBool foenTIMMTypeCheck(struct s_TimmInter *, char *);
struct s_imd_seg *fpstFindItemDescription(struct s_imd_seg *, char *);
struct s_qty_seg *fpstFindQuantity(struct s_qty_seg *, char *, char *);
struct s_moa_seg *fpstFindPaymentSegment(struct s_group_23 *, char *);
struct s_group_2 *fpstFindPartyBlock(struct s_group_2 *, char *); 
struct s_moa_seg *fpstFindPaymentSegment(struct s_group_23 *, char *); 
struct s_imd_seg *fpstFindItemDescription(struct s_imd_seg *, char *);
struct s_rff_seg *fpstFindReference(struct s_group_3 *, char *); 
struct s_qty_seg *fpstFindQuantity(struct s_qty_seg *, char *, char *); 
struct s_xcd_seg *fpstFindXCDSegment(struct s_group_99 *, int);
char *fpchzGetField(int, char *);
struct s_moa_seg *fpstFindMainPaymentSegment(struct s_group_45 *, char *);
struct s_rff_seg *fpstFindItemReference(struct s_group_26 *, char *);

