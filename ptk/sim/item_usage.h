extern toenBool foenItemUsage_FillRecord(tostItemUsage *, tostUsageRecord *); 
extern toenBool foenItemUsage_Gen(tostItemUsage *, toenPLMNType);
extern toenBool foenItemUsage_SummaryGen(tostItemUsage *, toenPLMNType);
extern tostItemUsage *fpstItemUsage_New(struct s_group_22 *);
extern toenBool foenItemUsage_Delete(tostItemUsage *);
extern toenBool foenItemUsage_Merge(tostItemUsage *, tostItemUsage *);
extern toenBool foenItemUsage_Match(tostItemUsage *, tostItemUsage *);
extern toenBool foenItemUsage_Empty(tostItemUsage *);
extern int foiItemUsage_Compare(tostItemUsage *, tostItemUsage *);
#ifdef _INVOICE_V5_
extern toenBool foenItemUsage_CallSumGen(struct s_group_22 *ppstG22, double poflBCHSumUsage, long int poilCoId);
#endif
