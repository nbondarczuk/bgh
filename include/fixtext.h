#ifndef FIXTEXT_H
#define FIXTEXT_H

/*************************************************************/
/*                                                           */
/* ABSTRACT :                                                */
/*                                                           */
/* AUTHOR   : Marcel Fehlmann, LHS International             */
/*                                                           */
/* CREATED  : 15-May-1995                                    */
/*                                                           */
/* DESCRIPTION :                                             */
/*                                                           */
/* Defines the filenames and version of the language         */
/* specific fixed strings files and the position of fixed    */
/* string entries in the file.                               */
/*                                                           */
/* N O T E !!!                                               */
/* if the number of entries changes, make clean and          */
/* and recompilation is neccessary.                          */
/*                                                           */
/*************************************************************/

#if 0 /* just for version.sh */
static char *SCCS_VERSION = "1.6";
#endif

#define ENGLTXT "english.fix"
#define ENGLVER "1.8"
#define ENGLTAG 1

#define ITALTXT "italian.fix"
#define ITALVER "1.8"
#define ITALTAG 2

#define GERMTXT "german.fix"
#define GERMVER "1.8"
#define GERMTAG 3

#define FRENTXT "french.fix"
#define FRENVER "1.8"
#define FRENTAG 4

#define ERR_FILE_OPEN 	1
#define INVOICE_ERROR 	2
#define WRONG_LANGUAGE 	3
#define EOF_READ        4
#define WRONG_VERSION   5

#define TEXTMARK        '"'  /* unique character in all     */
                             /* language specific files to  */
                             /* start and end of one entry  */
#define MAXFIXSTRNUM    799  /* highest number of fol. elem.*/
#define MAXFIXSTRLEN    256  /* max length of follow. elem. */

#define FIX_COM_FX   	102  /* telefax                     */
#define FIX_COM_TE   	100  /* telephone                   */
#define FIX_COM_TX   	101  /* telex                       */
#define FIX_TELECOM     109  /* network operator (short)    */
#define FIX_INVOICE  	110  /* Rechnung                    */
#define FIX_MSISDN     	141  /* MSISDN                      */
#define FIX_ITB      	111  /* itemized bill               */
#define FIX_SUM      	112  /* sumsheet                    */
#define FIX_PC       	103  /* Postkonto                   */
#define FIX_TO_SIM   	140  /* to sim number               */
#define FIX_MOB_CTR  	105  /* MOBILCOM CENTER             */
#define FIX_RFF_VA   	104  /* VAT number                  */
#define FIX_RFF_IT  	131  /* customer code               */
#define FIX_RFF_IV   	132  /* invoice number              */
#define FIX_RFF_IV2   	139  /* invoice number              */
#define FIX_RFF_PM_P 	  /* BAD paying                  */
#define FIX_RFF_PM_P2   137  /* BAD 2nd part                */
#define FIX_RFF_PM_D  	     /* direct debit paying         */
#define FIX_RFF_PM_R  	     /* credit card paying          */

#define FIX_FII	     	130  /* for                         */
#define FIX_DTM_3     	133  /* invoicing                   */
#define FIX_DTM_167   	134  /* period start                */
#define FIX_DTM_168   	     /* period end                  */
#define FIX_DTM_13    	135  /* due date                    */
#define FIX_DTM_156   	     /* payments considered until   */
#define FIX_DEBIT       157  /* account will be debited with*/
#define FIX_TAK_I_ACC   158  /* taken into account are ...  */

#define FIX_MOA_203   	310  /* item with VAT               */
#define FIX_MOA_203a  	311  /* item with VAT 2nd line      */
#define FIX_MOA_125   	304  /* item without VAT            */
#define FIX_MOA_125a  	305  /* item without VAT            */
#define FIX_MOA_901   	     /* threshold value             */
#define FIX_MOA_124   	308  /* VAT for item                */
#define FIX_MOA_124a  	309  /* VAT for item                */
#define FIX_MOA_52    	     /* discount                    */
#define FIX_MOA_11    	     /* paid amount                 */
#define FIX_MOA_48    	     /* deposit                     */
#define FIX_MOA_76    	     /* balance                     */
#define FIX_MOA_80    	501  /* round amount                */
#define FIX_MOA_77    	     /* invoice                     */
#define FIX_MOA_79    	500  /* netto invoice               */
#define FIX_MOA_178   	502  /* to pay                      */
#define FIX_MOA_940   	321  /* net amount charged by VPLMN */
#define FIX_MOA_940a  	322  /* net amount charged by VPLMN */
#define FIX_MOA_941   	319  /* foreign tax raised by VPLMN */
#define FIX_MOA_941a  	320  /* foreign tax raised by VPLMN */
#define FIX_MOA_942   	317  /* net amount + foreign tax    */
#define FIX_MOA_942a  	318  /* net amount + foreign tax    */
#define FIX_MOA_943   	323  /* surcharge amount by HPLMN   */
#define FIX_MOA_943a  	324  /* surcharge amount by HPLMN   */

#define FIX_SUM_SUB     520  /*subtotal for sumsheets       */
#define FIX_SUM_TOTAL   521  /* total for sumsheets         */
#define FIX_SUM_S       420  /* subscription charge         */
#define FIX_SUM_A       421  /* access charge               */
#define FIX_SUM_U       422  /* usage charge                */
#define FIX_SUM_SERV	423	/* service */
#define FIX_SUM_QTY	424	/* quantity */
#define FIX_SUM_TYPE	425	/* Type */
#define FIX_SUM_PPU	426	/* Price */
#define FIX_SUM_TTDES	427	/* Tariff Time */
#define FIX_SUM_ZNDES	428	/* Zone */



#define FIX_LIN_S       200  /* subscribtion                */
#define FIX_LIN_A       201  /* access                      */
#define FIX_LIN_UI      202  /* usage national inland       */
#define FIX_LIN_UA      204  /* usage international abroad  */
#define FIX_LIN_O       205  /* others                      */

#define FIX_IMD_SIM     220  /* SIM number for sumsheets    */
#define FIX_IMD_MSISDN  221  /* MSISDN for sumsheets        */
#define FIX_IMD_OTHER   222  /* title for other charges     */


#define FIX_LIN_SVE     300  /* title for lin services      */
#define FIX_LIN_NET     315  /* net operator                */
#define FIX_SUM_TITLE1  330  /* right side of sum header    */
#define FIX_SUM_TITLE2  331  /* right side of sum header    */
#define FIX_LIN_UR      203  /* usage international inland  */

#define FIX_XCD_TME     345  /* start time of call          */
#define FIX_XCD_AMT     346  /* amount of time              */
#define FIX_XCD_DIAL    347  /* dialed digits               */
#define FIX_XCD_ZONE    348  /* zone short description      */
#define FIX_XCD_TTME    349  /* tariff time short descr.    */
#define FIX_XCD_SVCE    350  /* service short description   */
#define FIX_XCD_RTYP    351  /* rating type indicator       */
#define FIX_XCD_MAMT    352  /* monatary amount             */
#define FIX_XCD_NET     353  /* foreign net                 */
#define FIX_XCD_FTAX         /* foreign tax                 */
#define FIX_XCD_CALL    354  /* calling number              */
#define FIX_XCD_REM     355  /* remark in VAS record        */
#define FIX_XCD_TOTAL   356  /* total for IB                */
#define FIX_XCD_TTME_2  357  /* 2nd line for tarif time     */
#define FIX_XCD_GROS_2  358  /* 2nd line for gros amount    */
#define FIX_XCD_TOTAL_2 359  /* 2nd line for total amount   */
#define FIX_VAT_RATE    360  /* VAT rate                    */
#define FIX_GROSS_VALUE 361  /* Gross value                 */


#define FIX_NL_OUT_HOME 230  /* national outbound from home */
#define FIX_IL_OUT_HOME 231  /* intnl. outbound from home   */
#define FIX_OUT_VISITOR 232  /* outbound from visitor net   */
#define FIX_IN_HOME     233  /* inbound in home             */
#define FIX_IN_VISITOR  234  /* inbound in visitor net      */
#define FIX_SERVICES    235  /* additional services         */
#define FIX_ROA_TITLE   210  /* foreign fees and tax        */
#define FIX_ROA_TOT	510  /* Total for roaming           */

#define FIX_PRI_CAL          /* calculation price           */
#define FIX_PRI_INV     301  /* invoice price               */
#define FIX_PRI_INVa    302  /* invoice price               */
#define FIX_TAX_NUM     316  /* VAT number                  */
#define FIX_TAX_7            /* tax for allowance and charge*/
#define FIX_TAX_1       306  /* individual tax for one item */
#define FIX_TAX_1a      307  /* individual tax for one item */
                             /* i.e. VAT in %               */
#define FIX_TAX_3            /* total of each tax           */
#define FIX_QTY_107     303  /* chargeable number of units  */
#define FIX_ROAMING     138  /* considered roaming calls    */

#define FIX_PAGE_1      150  /* X Seite von y               */
#define FIX_PAGE_2      151  
#define FIX_CONT        152  /* continued (next page)       */
#define FIX_THRESHOLD   153  /* threshold information       */
#define FIX_NO_CALLS_1  154  /* no call records 1           */
#define FIX_NO_CALLS_2  155  /* no call records 2           */
#define FIX_NO_CALLS_3  156  /* no call records 3           */ 

#define FIX_BAL_TITLE	700  /* Balance */
#define FIX_BAL_PAYMN	701  /*  */
#define FIX_BAL_WROFF	702
#define FIX_BAL_OINVC	703
#define FIX_BAL_CRMEM	704
#define FIX_BAL_COACC	705
#define FIX_BAL_CSHGL	706
#define FIX_BAL_PMTRI	707
#define FIX_BAL_INTRI	708
#define FIX_BAL_TROUT	709
#define FIX_BAL_ADJST	710
#define FIX_BAL_BOUNC	711
#define FIX_TRANSACTION 720
#define FIX_DATE	721
#define FIX_AMOUNT	722  /* Amount */
#define FIX_ACTION	723  /* Action */
#define FIX_REFNUM	724  /* Ref. nr. of invoice */
#define FIX_MOA_961	725  /* Previous balance */
#define FIX_MOA_962	726  /* Sum of received payments */
#define FIX_MOA_963	727  /* Sum of write off's */
#define FIX_MOA_964	728  /* Sum of other invoices */
#define FIX_MOA_965	729  /* Sum of adjustments */
#define FIX_MOA_967	730  /* Current total amount */
#define FIX_MOA_968	731  /* New balance */

#define LEG_TITLE       600  /* title of legend             */
#define LEG_HINT        601  /* to hint                     */
#define LEG_ABR_1       602  /* abreviation 1               */
#define LEG_DESC_1      603  /* description 1               */
#define LEG_ABR_2       604  /* abreviation 2               */
#define LEG_DESC_2      605  /* description 2               */
#define LEG_ABR_3       606  /* abreviation 3               */
#define LEG_DESC_3      607  /* description 3               */
#define LEG_ABR_4       608  /* abreviation 4               */
#define LEG_DESC_4      609  /* description 4               */
#define LEG_DESC_4A     610  /* description 4 2nd part      */
#define LEG_TIME        611  /* to timestamp                */
#define LEG_ABR_5       612  /* abreviation 5               */
#define LEG_DESC_5      613  /* description 5               */
#define LEG_SVCE        614  /* to service                  */
#define LEG_ABR_6       615  /* abreviation 6               */
#define LEG_DESC_6      616  /* description 6               */
#define LEG_ABR_7       617  /* abreviation 7               */
#define LEG_DESC_7      618  /* description 7               */
#define LEG_ABR_8       619  /* abreviation 8               */
#define LEG_DESC_8      620  /* description 8               */
#define LEG_ABR_9       621  /* abreviation 9               */
#define LEG_DESC_9      622  /* description 9               */
#define LEG_ABR_10      623  /* abreviation 10              */
#define LEG_DESC_10     624  /* description 10              */
#define LEG_ABR_11      625  /* abreviation 11              */
#define LEG_DESC_11     626  /* description 11              */
#define LEG_ABR_12      627  /* abreviation 12              */
#define LEG_DESC_12     628  /* description 12              */
#define LEG_ABR_13      629  
#define LEG_DESC_13     630   
#define LEG_TZONE       651  /* to tariff zone              */
#define LEG_TTIME       652  /* to tariff time              */ 
#endif
