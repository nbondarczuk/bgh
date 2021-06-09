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
* 
***************************************************************************
*/

INSERT INTO EDS_BGH_LOYAL_CHARGE_TYPE VALUES (0, 'KONTR', '*',   '*',     'Summary contract charge');
INSERT INTO EDS_BGH_LOYAL_CHARGE_TYPE VALUES (-1,'RABAT', '*',   '*',     'Summary of discounts');

INSERT INTO EDS_BGH_LOYAL_CHARGE_TYPE VALUES (1, 'INNE',  '*',   '*',     'Most general rule');
INSERT INTO EDS_BGH_LOYAL_CHARGE_TYPE VALUES (2, 'AKTY',  '*.S', '*',     'Subscription fee');
INSERT INTO EDS_BGH_LOYAL_CHARGE_TYPE VALUES (3, 'ABON',  '*.A', '*',     'Access fee');
INSERT INTO EDS_BGH_LOYAL_CHARGE_TYPE VALUES (4, 'POLA',  '*.U', 'A.*',   'Air usage charges');
INSERT INTO EDS_BGH_LOYAL_CHARGE_TYPE VALUES (5, 'POLI',  '*.U', 'I.*',   'Interconect usage charges');
INSERT INTO EDS_BGH_LOYAL_CHARGE_TYPE VALUES (6, 'POLC',  '*.U', 'C.*',   'CF usage charges');
INSERT INTO EDS_BGH_LOYAL_CHARGE_TYPE VALUES (7, 'POLR',  '*.U', 'R.*',   'RLeg usage charges');
INSERT INTO EDS_BGH_LOYAL_CHARGE_TYPE VALUES (8, 'ROAM',  '*.U', '*.r.*', 'Roaming usage charges');
INSERT INTO EDS_BGH_LOYAL_CHARGE_TYPE VALUES (9, 'RADM',  '*.U', '*.R.*', 'Roaming surcharge');
INSERT INTO EDS_BGH_LOYAL_CHARGE_TYPE VALUES (10,'OCC',   '*O',  '*',     'All OCC');
INSERT INTO EDS_BGH_LOYAL_CHARGE_TYPE VALUES (11,'TMOCC', '*.O', '*',     'Tariff model related OCC');


/*
***************************************************************************
* END OF 'eds20000510.sql'
***************************************************************************
*/
