SELECT CSTESTBILLRUN, BILLCYCLE, COUNT(*)
FROM CUSTOMER_ALL
WHERE CSTESTBILLRUN IS NOT NULL
GROUP BY CSTESTBILLRUN, BILLCYCLE
ORDER BY CSTESTBILLRUN, BILLCYCLE;


