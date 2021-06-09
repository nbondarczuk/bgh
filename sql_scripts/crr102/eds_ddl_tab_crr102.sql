/*
***************************************************************************
* NAME         : EDS_BGH_REP
* PROJECT NAME : CR102
* FUNCTION     : Create new tables for saving the invoice first page
*                structure
*                These tables should be created within ADDONS schema and
*                appriopriate public synonyms should be created. 
*                User ADDONS has to have grant REFERENCES to ORDERHDR_ALL
*                and TAX_ALL
* COPYRIGHT    : Copyright (c) EDS Poland Sp. z o.o.
*                All Rights Reserved.  Copying this software or parts of it
*                is a violation of Polish and International laws and will 
*                be prosecuted.
* NOTES        :
* =========================================================================
*                            C H A N G E   L O G
*
* Ver Date     Resp.  Action
* No  YYYYMMDD Name   Description
* -------------------------------------------------------------------------
* 1.0 20000214 NB, BK Initial creation
* 2.0 20000223 NB     New table OT, removed tables LABEL & SERV
* 
***************************************************************************
*/

/*
***************************************************************************
*
* NAME       : EDS_VATINVOTR
*
* DESCRIPTION: This is the mirror of ORDERTRAILER table with OTSEQ filled
*              with the number loaded by BGH from DB using the 1:1 mapping 
*              between OTNAME and LIN + PIA texts from EDI TIMM SUM SHHET
*              message. The SHDES of all strings used in EDI must be unique.
*              The following tables are affected:
*              MPUTMTAB
*              MPUSPTAB
*              MPUSNTAB
*              MPUZNTAB
*              MPUTTTAB
*              MPDPLTAB
*              The amounts are exact - not rounded. NETAMT is the netto 
*              before discounting (info not available in ORDERTRAILER).
*              DISAMT is the discount assigned by BCH to a given booking.
*              Columns with CUR infix wull be used by CIN. BGH will not fill
*              them.
*
***************************************************************************
*/

CREATE TABLE EDS_VATINVOTR
(
  OTXACT      NUMBER NOT NULL REFERENCES SYSADM.ORDERHDR_ALL(OHXACT),
  OTSEQ       NUMBER NOT NULL,
  NETAMT      NUMBER NOT NULL,
  DISAMT      NUMBER NOT NULL,
  NETCURAMT   NUMBER,
  DISCURAMT   NUMBER,
  REC_VERSION NUMBER,
  PRIMARY KEY (OTXACT, OTSEQ)
  USING INDEX TABLESPACE IDATA1 STORAGE (INITIAL 20M NEXT 20M PCTINCREASE 0)
)
TABLESPACE DATA1
STORAGE (INITIAL 100M NEXT 100M PCTINCREASE 0);

GRANT ALL ON EDS_VATINVOTR TO BSCS_ROLE;

/*
***************************************************************************
*
* NAME       : EDS_VATINVTAX
*
* DESCRIPTION: Summary info about each tax rate with the discount amount
*              calculatedfor each tax rate. Tax exemption is ON when
*              TAX_EXEMPT = 'X' and OFF when TAX_EXEMPT = 'N' . 
*              NETAMT is the netto before discounting.
*              DISAMT is the discount assigned by BCH to a given booking
*              TAXAMT is calculated after discounting.
*              Columns with CUR infix wull be used by CIN. BGH will not fill
*              them.
*
***************************************************************************
*/

CREATE TABLE EDS_VATINVTAX
(
  OHXACT      NUMBER NOT NULL REFERENCES SYSADM.ORDERHDR_ALL(OHXACT),
  TAX_RATE    NUMBER NOT NULL REFERENCES SYSADM.TAX_ALL(TAX_RATE),
  TAX_EXEMPT  CHAR(1) NOT NULL,
  NETAMT      NUMBER NOT NULL,
  DISAMT      NUMBER NOT NULL,
  TAXAMT      NUMBER NOT NULL,
  TOTAMT      NUMBER NOT NULL,
  NETCURAMT   NUMBER,
  DISCURAMT   NUMBER,
  TAXCURAMT   NUMBER,
  TOTCURAMT   NUMBER,
  REC_VERSION NUMBER,
  PRIMARY KEY (OHXACT, TAX_RATE, TAX_EXEMPT)
USING INDEX TABLESPACE IDATA1 STORAGE (INITIAL 20M NEXT 20M PCTINCREASE 0)
)
TABLESPACE DATA1
STORAGE (INITIAL 100M NEXT 100M PCTINCREASE 0);

GRANT ALL ON EDS_VATINVTAX TO BSCS_ROLE;

/*
***************************************************************************
* END OF 'CR102'
***************************************************************************
*/

