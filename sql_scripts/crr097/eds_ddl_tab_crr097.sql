/* 
***************************************************************************
* NAME         : eds20000207.sql
* PROJECT NAME : PTK
* FUNCTION     : The following tables are nessesary for change request 
*                CCRR97 (WORKPARVAL) - BGH
*                All tables should be created within addons's schema on the
*		 production database and appriopriate public synonyms should
*		 be created. The table RIH_WORKPARVAL is created and
*		 maintained by PTK Centertel.
*                
* COPYRIGHT    : Copyright (c) EDS Poland Sp. z o.o.
*                All Rights Reserved.  Copying this software or parts of it
*                is a violation of Polish and International laws and will 
*                be prosecuted.
* NOTES        :
* =========================================================================
*                            C H A N G E   L O G
*
* Ver Date     Resp. Action
* No  YYYYMMDD Name  Description
* -------------------------------------------------------------------------
* 1.0 20000207 MB    Initial creation
* 
***************************************************************************
*/

/*
CREATE TABLE RIH_WORKPARVAL
(
WPARVALCODE	NUMBER(22) CONSTRAINT PKRIH_WORKPARVAL 
			PRIMARY KEY
			USING INDEX 
			STORAGE (INITIAL 1M NEXT 1M PCTINCREASE 0)
			TABLESPACE IDATA,
EVCODE		NUMBER(22) NOT NULL,
SERVCODE	NUMBER(5)  NOT NULL,
WPARCODE	NUMBER(22) NOT NULL,
CUSTOMER_ID	NUMBER(22) NOT NULL,
CO_ID		NUMBER(22),
PERIOD		NUMBER(4) NOT NULL,
WPARVAL		VARCHAR2(80),
START_DATE	DATE NOT NULL,
FINISH_DATE	DATE NOT NULL,
FIELD1		VARCHAR2(80),
FIELD2		VARCHAR2(80),
FIELD3		VARCHAR2(80),
FIELD4  	VARCHAR2(80),
FIELD5  	VARCHAR2(80),
USERNAME	VARCHAR2(20) NOT NULL,
REC_VERSION	NUMBER(22) NOT NULL
)
STORAGE (INITIAL 1M NEXT 1M PCTINCREASE 0)
TABLESPACE DATA
/

grant all on RIH_WORKPARVAL to bscs_role;
*/

CREATE TABLE EDS_BGH_WPARCODE
(
WPARCODE	NUMBER(22) NOT NULL
)
STORAGE (INITIAL 1M NEXT 1M PCTINCREASE 0)
TABLESPACE DATA
/

grant all on EDS_BGH_WPARCODE to bscs_role;
