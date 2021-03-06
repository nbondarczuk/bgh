/* 
***************************************************************************
* NAME         : eds20000510.sql
* PROJECT NAME : PTK
* FUNCTION     : CHANGE REQUEST 106 FOR BGH.
*                
* COPYRIGHT    : Copyright (c) EDS Poland Sp. z o.o.
*                All Rights Reserved.  Copying this software or parts of it
*                is a violation of Polish and International laws and will 
*                be prosecuted.
* NOTES        : This script should be executed within ADDONS's schema and
*		 appriopriate public synonyms should be created as well as
*		 ADDONS should be granted REFERENCES privilege to MPUSNTAB
*		 and MPUZNTAB.
* =========================================================================
*                            C H A N G E   L O G
*
* Ver Date     Resp. Action
* No  YYYYMMDD Name  Description
* -------------------------------------------------------------------------
* 1.0 20000510 MB    Initial creation
* 2.0 20000530 NB    Big change according to the new design of Loyalty mod.
* 
***************************************************************************
*/

DROP PUBLIC SYNONYM EDS_BGH_LOYALHDR;
DROP PUBLIC SYNONYM EDS_BGH_LOYAL_CONTRACT;
DROP PUBLIC SYNONYM EDS_BGH_LOYAL_CHARGE_TYPE;
DROP PUBLIC SYNONYM EDS_BGH_LOYALTRAILER;

DROP TABLE EDS_BGH_LOYALTRAILER;
DROP TABLE EDS_BGH_LOYAL_CONTRACT;
DROP TABLE EDS_BGH_LOYALHDR;
DROP TABLE EDS_BGH_LOYAL_CHARGE_TYPE;

/* EDS_BGH_LOYAL_CHARGE_TYPE */
CREATE TABLE EDS_BGH_LOYAL_CHARGE_TYPE
(
  AMOUNT_TYPE_ID	NUMBER(38)    NOT NULL ,
  SHDES   	      VARCHAR2(5)   NOT NULL,
  LIN7140         VARCHAR(35)   NOT NULL,
  PIA7140         VARCHAR(35)   NOT NULL,
  DES		          VARCHAR2(128) NOT NULL
)
TABLESPACE DATA
PCTFREE 2 PCTUSED 90
STORAGE (INITIAL 80K NEXT 80K PCTINCREASE 0);

GRANT SELECT, UPDATE, INSERT, DELETE ON EDS_BGH_LOYAL_CHARGE_TYPE TO BSCS_ROLE;


/* EDS_BGH_LOYALHDR */
CREATE TABLE EDS_BGH_LOYALHDR 
(
  OHXACT      NUMBER(38)   NOT NULL,
  CUSTOMER_ID	NUMBER(38)   NOT NULL,
  CUSTCODE    VARCHAR2(20) NOT NULL,
  OHREFNUM	  VARCHAR2(30) NOT NULL,
  OHREFDATE   DATE         NOT NULL,
  PRIMARY KEY (OHXACT) 
)
TABLESPACE DATA
PCTFREE 2 PCTUSED 90
STORAGE (INITIAL 20M NEXT 20M PCTINCREASE 0);

GRANT SELECT, UPDATE, INSERT, DELETE ON EDS_BGH_LOYALHDR TO BSCS_ROLE;

/* EDS_BGH_LOYAL_CONTRACT */
CREATE TABLE EDS_BGH_LOYAL_CONTRACT 
(
  OHXACT      NUMBER(38)   NOT NULL,
  CO_ID		    NUMBER(38)   NULL,
  MAIN_MSISDN	VARCHAR2(32) NULL,  
  UNIQUE(OHXACT, CO_ID)
)
TABLESPACE DATA
PCTFREE 2 PCTUSED 90
STORAGE (INITIAL 20M NEXT 20M PCTINCREASE 0);

GRANT SELECT, UPDATE, INSERT, DELETE ON EDS_BGH_LOYAL_CONTRACT TO BSCS_ROLE;

/* EDS_BGH_LOYALTRAILER */
CREATE TABLE EDS_BGH_LOYALTRAILER  
(
  OHXACT          NUMBER(38)   NOT NULL,
  CO_ID		        NUMBER(38)   NULL,
  AMOUNT_TYPE_ID	NUMBER(38)   NOT NULL,
  NET_AMOUNT			FLOAT(126)   NOT NULL,
  TAX_AMOUNT		  FLOAT(126)   NOT NULL,
  UNIQUE(OHXACT, CO_ID, AMOUNT_TYPE_ID)
)
TABLESPACE DATA
PCTFREE 2 PCTUSED 90
STORAGE (INITIAL 100M NEXT 100M PCTINCREASE 0);

GRANT SELECT, UPDATE, INSERT, DELETE ON EDS_BGH_LOYALTRAILER TO BSCS_ROLE;

CREATE PUBLIC SYNONYM EDS_BGH_LOYALHDR FOR EDS_BGH_LOYALHDR;
CREATE PUBLIC SYNONYM EDS_BGH_LOYAL_CONTRACT FOR EDS_BGH_LOYAL_CONTRACT;
CREATE PUBLIC SYNONYM EDS_BGH_LOYAL_CHARGE_TYPE FOR EDS_BGH_LOYAL_CHARGE_TYPE;
CREATE PUBLIC SYNONYM EDS_BGH_LOYALTRAILER FOR EDS_BGH_LOYALTRAILER;

/*
***************************************************************************
* END OF 'eds20000510.sql'
***************************************************************************
*/
