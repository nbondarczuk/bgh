/*
***************************************************************************
* NAME         : EDS_BGH_REP
* PROJECT NAME : CR68
* FUNCTION     : Change of EDS_BGH_REP table in otder to implement dual
*                market BGH features.
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


ALTER TABLE EDS_BGH_REP ADD (SCCODE NUMBER CONSTRAINT EDS_BGH_REP_SC_FK REFERENCES MPDSCTAB(SCCODE));
ALTER TABLE EDS_BGH_REP ADD (PLCODE NUMBER CONSTRAINT EDS_BGH_REP_PL_FK REFERENCES MPDPLTAB(PLCODE));

/*
***************************************************************************
* END OF 'CR68'
***************************************************************************
*/

