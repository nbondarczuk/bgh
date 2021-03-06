SET ECHO OFF
SET TAB OFF
SET LINE 1000
SELECT MP.PPID, SP.SHDES, SN.SHDES, ZN.DES, TT.DES, MP.TYPEIND, 
   MP.UP01 / 60, MP.PP01 * 60, 
   MP.UP02 / 60, MP.PP02 * 60, 
   MP.UP03 / 60, MP.PP03 * 60, 
   MP.UP04 / 60, MP.PP04 * 60, 
   MP.UP05 / 60, MP.PP05 * 60, 
   MP.UP06 / 60, MP.PP06 * 60, 
   MP.UP07 / 60, MP.PP07 * 60, 
   MP.UP08 / 60, MP.PP08 * 60, 
   MP.UP09 / 60, MP.PP09 * 60, 
   MP.PP10 * 60
FROM MPULKPPM MP,
   MPUSPTAB SP,
   MPUSNTAB SN,
   MPUZNTAB ZN,
   MPUTTTAB TT
WHERE MP.PPCODE = 4 
  AND MP.VSCODE = 2
  AND MP.SPCODE = SP.SPCODE
  AND MP.SNCODE = SN.SNCODE
  AND (MP.ZNCODE = ZN.ZNCODE)
  AND MP.TTCODE = TT.TTCODE
UNION
SELECT MP.PPID, SP.SHDES, SN.SHDES, '------', TT.DES, MP.TYPEIND, 
   MP.UP01 / 60, MP.PP01 * 60, 
   MP.UP02 / 60, MP.PP02 * 60, 
   MP.UP03 / 60, MP.PP03 * 60, 
   MP.UP04 / 60, MP.PP04 * 60, 
   MP.UP05 / 60, MP.PP05 * 60, 
   MP.UP06 / 60, MP.PP06 * 60, 
   MP.UP07 / 60, MP.PP07 * 60, 
   MP.UP08 / 60, MP.PP08 * 60, 
   MP.UP09 / 60, MP.PP09 * 60, 
   MP.PP10 * 60
FROM MPULKPPM MP,
   MPUSPTAB SP,
   MPUSNTAB SN,
   MPUTTTAB TT
WHERE MP.PPCODE = 4 
  AND MP.VSCODE = 2
  AND MP.SPCODE = SP.SPCODE
  AND MP.SNCODE = SN.SNCODE
  AND MP.ZNCODE IS NULL
  AND MP.TTCODE = TT.TTCODE;

