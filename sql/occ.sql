SET TAB OFF
SELECT SEQNO, FEE_TYPE, AMOUNT, CO_ID, VALID_FROM, GLCODE, SUBSTR(REMARK, 1, 80)
FROM FEES
WHERE CUSTOMER_ID = &1
ORDER BY SEQNO;
