typedef enum toenMailingType
{
  MAILING_INV,
  MAILING_ENC,
  MAILING_PAY

} toenMailingType;

toenBool foenGenEnvelope(struct s_TimmInter *ppstInvTimm,
                         struct s_TimmInter *ppstSumTimm,
                         enum toenMailingType poenMailingType,
                         int poiAddRuleNo);

