/*
***************************************************************************
* NAME         : EDS_BGH_REP
* PROJECT NAME : CR68
* FUNCTION     : 1. Setup of old values of attributes in table EDS_BGH_REP.
*                2. Setup of value used as scaling factor for call item gross 
*                   value.
*                3. change of default bill medium description and adding new 
*                   values to the table BILL_MEDIUM.
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
* 1.0 19990420 NB    Initial creation
* 
***************************************************************************
*/

/*
 * New attributes must have default values
 * valid for DCS market. In dual market varsion 
 * SCCODE and PLCODE will have values used to
 * distinguaish between DCS and NMT markets.
 */
UPDATE EDS_BGH_REP SET SCCODE = 1, PLCODE = 2000;

/*
 * MPSCFTAB must have new entry with CFDCODE
 * eq 6201 which is the itemized bill gross
 * value scaling factor itemized bill in the file.
 */
INSERT INTO MPSCFTAB VALUES (6201, '0.22', 'ITB scaling factor', 0);

/*
 * Table BILL_MEDIUM must be changed in order to support
 * many bill mediums. Actually it contains only one row
 * but at last 3 ones must be defined.
 */
DELETE FROM BILL_MEDIUM WHERE BM_ID <> 1;
UPDATE BILL_MEDIUM SET BM_DES = 'Standard' WHERE BM_ID = 1;
INSERT INTO BILL_MEDIUM VALUES (2, 'DYSK', NULL, 0);
INSERT INTO BILL_MEDIUM VALUES (3, 'DYSK_TVP', NULL, 0);
INSERT INTO BILL_MEDIUM VALUES (4, 'Wylapka A', NULL, 0);
