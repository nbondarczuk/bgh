/*******************************************************************************
 * LH-Specification GmbH 1995.
 *
 * All rights reserved.
 * Copying of this software or parts of this software is a violation of German
 * and International laws and will be prosecuted.
 *
 * Project  :   BGH
 *
 * File     :   bgh_prep.h
 * Created  :   Apr. 1996
 * Author(s):   M. Mustroph
 *******************************************************************************/

#ifndef BGH_PREP_H
#define BGH_PREP_H 1

#if 0 /* just for version.sh */
static char *SCCS_VERSION = "1.16";
#endif



/*************************************************************/
/* methods of payment                                        */
/*************************************************************/

#define PAYNT_METH_BAD    'D'
#define PAYNT_METH_ESR    'P'
#define PAYNT_METH_CREDIT 'C'

/*************************************************************/
/* Some defines to figure out some different content in the  */
/* same TIMM data segment                                    */
/*************************************************************/

#define QUANTITY        "107"
#define NAD_SENDER	"II"  /* Sender address              */
#define NAD_RECEIVER	"IV"  /* Receiver address            */
#define SIM_DESC	"SIM" /* a sim number is described   */
#define MSISDN_DESC     "MSISDN"
                              /* describes an msisdn number  */
#define COM_FX		"FX"  /* telefax                     */
#define COM_TE		"TE"  /* telephone                   */
#define COM_TX		"TX"  /* telex                       */
#define RFF_VA		"VA"  /* Tax rate number of sender   */
#define RFF_IT		"IT"  /* internal customer code      */
#define RFF_IV		"IV"  /* invoice number              */
#define RFF_PM 		"PM"  /* payment method              */
#define DTM_INV	        "3"   /* invoicing date              */
#define DTM_ST		"167" /* period start                */
#define DTM_EN		"168" /* period end                  */
#define DTM_DUE		"13"  /* due date                    */
#define DTM_TIL		"156" /* considered until            */
#define FII_AO		"AO"  /* invoicee                    */
#define QUANTITY	"107" /* CHANGE TO 107 LATER         */
#define LIN_NESTING_01  "01"  /* LIN nesting levels          */
#define LIN_NESTING_02  "02"  /* LIN nesting levels          */
#define LIN_NESTING_03  "03"  /* LIN nesting levels          */
#define LIN_NESTING_04  "04"  /* LIN nesting levels          */
#define LIN_NESTING_05  "05"  /* LIN nesting levels          */
#define IMD_MSISDN      "MSISDN"
#define IMD_SIM_NUM     "SIM"
#define IMD_CONTRACT    "CO"
#define IMD_SUBSCR_CHARGE "S"  /* subcription charges        */ 
#define IMD_ACCESS_CHARGE "A"  /* access charges             */
#define IMD_USAGE_CHARGE  "U"  /* usage charges              */
#define IMD_SERVICE_CONTR "CO" /* contract id                */
#define IMD_SERVICE_PACK  "SP" /* service package            */
#define IMD_SERVICE       "SN" /* service                    */
#define IMD_SERVICE_FEE   "FE" /* service fee                */

#define MOA_SUBTOTAL_CODE "931"
#define MOA_DETAIL_CODE   "932"

#define DOC_LAN_GER_C     "DE"
#define DOC_LAN_GER       "de"

#define DOC_LAN_GB_C      "EN"
#define DOC_LAN_GB        "en"

#define DOC_LAN_FR_C      "FR"
#define DOC_LAN_FR        "fr"

#define DOC_LAN_IT_C      "IT"
#define DOC_LAN_IT        "it"


#define CUSTCODE_FLAT     '1'  /* first element of the       */ 
                               /* custcode for a flat subscr.*/ 


/* some defines for the formatting of FTX segs              */

#define MAX_FTX_SEGEMENTS         10
#define MAX_FTX_TEXT_FIELDS       5
#define MAX_FTX_CHARS_PER_FIELD   70
#define MAX_FTX_CHAR_PER_LINE     65
#define MAX_FTX_BUFFER            3500

/* possible values for LIN field 7140                       */
/* "charge type"                                            */

#define CHARGE_TYPE_SUBSCRIPTION   'S'
#define CHARGE_TYPE_ACCESS        'A'
#define CHARGE_TYPE_USAGE         'U'
#define CHARGE_TYPE_OTHER         'O'


/* possible values for PIA field 7140                       */
/* "charge type indicator"                                  */

#define CHARGE_TYPE_AIR           'A'
#define CHARGE_TYPE_INTERCONNECT  'I'
#define CHARGE_TYPE_CF_LEG        'C'
#define CHARGE_TYPE_ROAMING_LEG   'R' 


/* possible values for PIA field 7140                       */
/* price per unit indicator                                 */

#define PpU_HPLMN_SURCHARGE_TA_MOC  'R'
#define PpU_VPLMN_CHARGES_TA_MOC    'r'
#define PpU_VPLMN_CHARGES_TA_MTC    'm'
#define PpU_FLAT                    'F'
#define PpU_BUNDLED                 'B'
#define PpU_SCALED                  'S'
#define PpU_TIERED                  'T'


/* possible values for PIA field 7140                       */
/* short description of tariff zone ZNDES                   */

#define ZNDES_NATIONAL1             'N'
#define ZNDES_NATIONAL2             'D'
#define ZNDES_INTERNATIONAL         'I'

#define ZNDES_INSENSITIVE           "INSEN"

/* possible values for BGM field 1000                       */

#define INVOICE       "BCH-PAYMENT"
#define ITEMIZED_BILL "BCH-SUBSCRIBER"
#define ROAMING_DOC   "BCH-ROAMING"
#define BALANCE_DOC   "BCH-BALANCE"
#define SUM_SHEET     "BCH-SUMSHEET"
#define LEGEND        "BCH-LEGEND"

/* possible values for XCD fields                           */

#define X017_OUTBOUND    "O"
#define X017_INBOUND     "I"
#define X018_HOME        "H"
#define X018_VISITOR     "V"
#define X021_NAT_PLMN    "NV"
#define X021_NAT_PSTN    "NP"
#define X021_INT_PLMN    "IV"
#define X021_INT_PSTN    "IP"
#define X021_UNDEFINED   "U"
#define X008_SERV_GEMAN  "GEMAN"
#define X008_SERV_CORON  "CORON"
#define X6411_UNIT_SEC   "Sec"
#define X6411_UNIT_EV    "Ev"
#define X012_LATE_CALL   "L"
#define X012_NORMAL_CALL "N"
#define X013_NORMAL_CALL "N"


/*  possible value for IMD field 7009                       */

#define IMD_SERVICE_CODE_ID     "SN"
#define IMD_TARIFF_TIME_CODE_ID "TT"
#define IMD_TARIFF_ZONE_CODE_ID "TZ"
#define IMD_VPLMN_CODE_ID       "VPLMN"

/*  some legal values for IMD field 7008                    */

#define IMD_GEMAN_SHORT_DES     "GEMAN"
#define IMD_CORONA_SHORT_DES    "CORON"                 

#define MAX_IMD_CODES           3
/* only SN, TT and TZ                                       */


/*  possible values for RFF field 1153                      */

#define RFF_CUST_CODE    "IT"
#define RFF_INV_NUM      "IV"
#define RFF_BILL_MEDIUM  "LY"
#define RFF_CALL_DET_MOD "CD"
#define RFF_METH_PAY     "PM"
#define RFF_TARIFF_MOD   "TM"
#define RFF_SUMSHEET_REQ "SR"
#define RFF_VAT_NUM      "VA"
#define RFF_CONTR_NUM    "NC"
#define RFF_SORT_CRIT    "BB"


/*  possible values for MOA field 5025                      */

#define MOA_THRESHOLD_VALUE "901"
#define MOA_V5004_MAX_LEN   20

/*  possible values for NAD field 3035                      */

#define NAD_V3035_INVOICEE  "IV"
#define NAD_V3035_SENDER    "II"

/*  possible values for DOC field 3453                      */

#define DOC_LAN_GERMAN      "DE"
#define DOC_LAN_ITALIAN     "IT"
#define DOC_LAN_FRENCH      "FR"
#define DOC_LAN_ENGLISH     "EN"

/* length of date fields                                    */

#define DATE_STRLEN          8
#define DUR_STRLEN           12
#define TMSTMP_STRLEN        14 


/* Define position of layout elements common for all */
#define LAY_SENDER_LINE_1           1
#define LAY_SENDER_LINE_2           2
#define LAY_SENDER_LINE_3           3
#define LAY_SENDER_LINE_4           4
#define LAY_SENDER_LINE_5           5
#define LAY_SENDER_FULLINE	    6
#define LAY_INVOICEE_FULLINE	    7
#define LAY_PAGE		    8
#define LAY_ADV_INFO		    9
#define LAY_INVOICEE_LINE_1        10
#define LAY_INVOICEE_LINE_2        11
#define LAY_INVOICEE_LINE_3        12
#define LAY_INVOICEE_LINE_4        13
#define LAY_INVOICEE_LINE_5        14
#define LAY_INVOICEE_LINE_6        15

#define LAY_ACCOUNT_NO             16
#define LAY_INVOICE_NO             17
#define LAY_INVOICE_DATE           18
#define LAY_BILLING_PERIOD         19

#define LAY_FIX_HEADER             20
#define LAY_FIX_TELECOM            21

#define LAY_FIX_STARTFIRST	   22
#define LAY_FIX_STARTFOLLOW	   23

/* Define position of layout elements for invoice */
#define LAY_INV_FIX_AUTO_PAY           30
#define LAY_INV_FIX_BALANCING          31
#define LAY_INV_FIX_HEADER_1           32
#define LAY_INV_FIX_SUBSCRIPTION       33
#define LAY_INV_ITEMS_SUBSCRIPTION     34
#define LAY_INV_FIX_ACCESS             35
#define LAY_INV_ITEMS_ACCESS           36
#define LAY_INV_FIX_NAT_USAGE          37
#define LAY_INV_ITEMS_NAT_USAGE        38
#define LAY_INV_FIX_OTHER_CHARGES      39
#define LAY_INV_ITEMS_OTHER_CHARGES    40
#define LAY_INV_FIX_INT_USAGE_INL      41
#define LAY_INV_ITEMS_INT_USAGE_INL    42
#define LAY_INV_FIX_INT_USAGE_ABROAD   43
#define LAY_INV_ITEMS_INT_USAGE_ABROAD 44
#define LAY_INV_SUBTOTAL               45
#define LAY_INV_ADJUSTMENT             46
#define LAY_INV_TOTALS                 47


/* Define position of layout elements for sumsheet */
#define LAY_SUM_SIM_NUMBER             60
#define LAY_SUM_HEADER                 61
#define LAY_SUM_SUBSCRIPTION           62
#define LAY_SUM_ACCESS                 63
#define LAY_SUM_USAGE                  64
#define LAY_SUM_SUBTOTAL               65
#define LAY_SUM_SUBADDRESS	       66
#define LAY_SUM_LIN04_S		       67
#define LAY_SUM_LIN04_A		       68
#define LAY_SUM_LIN04_U		       69

#define LAY_SUM_FIX_AMOUNT             70
#define LAY_SUM_FIX_OTHER_CHARGES      71
#define LAY_SUM_ITEMS_OTHER_CHARGES    72
#define LAY_SUM_EMPTY_LINE_1           73
#define LAY_SUM_TOTAL                  74
#define LAY_SUM_LIN04_HS	       75
#define LAY_SUM_LIN04_HA	       76
#define LAY_SUM_LIN04_HU	       77

/* Define position of layout elements for itemized bill */
#define LAY_ITB_THRESHOLD              80
#define LAY_ITB_NO_CALL_REC_12         81
#define LAY_ITB_NO_CALL_REC_3          82
#define LAY_ITB_FIX_TO_SIM             83
#define LAY_ITB_FIX_MSISDN             84
#define LAY_ITB_FIX_HEADER_1           90
#define LAY_ITB_ITEMS_TABLE_1          91
#define LAY_ITB_FIX_HEADER_2           92
#define LAY_ITB_ITEMS_TABLE_2          93
#define LAY_ITB_FIX_HEADER_3           94
#define LAY_ITB_ITEMS_TABLE_3          95
#define LAY_ITB_FIX_HEADER_4           96
#define LAY_ITB_ITEMS_TABLE_4          97
#define LAY_ITB_FIX_HEADER_5           98
#define LAY_ITB_ITEMS_TABLE_5          99
#define LAY_ITB_FIX_HEADER_6          100
#define LAY_ITB_ITEMS_TABLE_6         101

#define LAY_LEG_TITLE         	      110
#define LAY_LEG_HINT           	      111
#define LAY_LEG_INFO           	      112


/* Define position of layout elements for roaming page */
#define LAY_ROA_SIM_NR		      130
#define LAY_ROA_FIX_DESCRIPTION       131
#define LAY_ROA_ITEMS                 132
#define LAY_ROA_DUMMY                 133

#define LAY_ROA_TOTAL                 135


/* Define position of layout elements for balance */
#define LAY_BAL_IT_HEADER	      140
#define LAY_BAL_IT_ELEMENTS	      141
#define LAY_BAL_TR_HEADER	      142
#define LAY_BAL_TR_ELEMENTS	      143

#endif
