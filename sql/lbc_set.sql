UPDATE CUSTOMER_ALL SET LBC_DATE = '&2'
WHERE CUSTOMER_ID = '&1';

UPDATE CONTR_SERVICES 
SET CS_DATE_BILLED = '&2'
WHERE CO_ID IN (SELECT CO_ID 
                  FROM CONTRACT_ALL
                  WHERE CUSTOMER_ID = '&1');