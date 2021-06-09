/*******************************************************************************
 * LH-Specification GmbH 1996.
 *
 * All rights reserved.
 * Copying of this software or parts of this software is a violation of German
 * and International laws and will be prosecuted.
 *
 * Project  :   BGH
 *
 * File     :   pars.h
 * Created  :   Feb. 1996
 * Author(s):   A. Dreier, T. Rotaru
 *              adapted from pars210.h Apr.  1996
 *******************************************************************************/

#ifndef TIMM_PARSER_H
#define TIMM_PARSER_H 1

#if 0 /* just for version.sh */
static char *SCCS_VERSION = "1.6";
#endif

#define VERBOSE 0

#define UNUSED 0

#define m_char char

/****************************************/
/*                                      */
/* Segmente                             */
/*                                      */
/****************************************/

struct s_unb_seg
{
  m_char           v_0001[5];
  m_char           v_0002[2];
  m_char           v_0004[36];
  m_char           v_0010[36];
  m_char           v_0017[9];
  m_char           v_0019[5];
  m_char           v_0020[15];
#if UNUSED
  m_char           v_0022[15];
  m_char           v_0025[3];
  m_char           v_0026[15];
  m_char           v_0029[2];
  m_char           v_0031[2];
  m_char           v_0032[36];
  m_char           v_0035[2];
#endif
};

struct s_unz_seg
{
  m_char           v_0036[7];
  m_char           v_0020[15];
};

struct s_unh_seg
{
  int		   version;
  int		   release;
  m_char           v_0062[15];
  m_char           v_0065[7];
  m_char           v_0052[4];
  m_char           v_0054[4];
  m_char           v_0051[3];
  m_char           v_0057[7];
};

struct s_unt_seg
{
  m_char           v_0074[7];
  m_char           v_0062[15];
};

struct s_uns_seg
{
  m_char           v_0081[2];
};

struct s_alc_seg
{
  m_char           v_5463[4];
};

struct s_bgm_seg
{
  m_char           v_1001[4];
  m_char           v_1000[36];
  m_char           v_1004[40];
  m_char           v_1225[4];
};

struct s_cnt_seg
{
  struct s_cnt_seg *cnt_next;
  m_char           v_6069[4];
  m_char           v_6066[19];
  m_char           v_6411[4];
};

struct s_com_seg
{
  struct s_com_seg *com_next;
  m_char           v_3148[26];
  m_char           v_3155[4];
};

struct s_cta_seg
{
  m_char           v_3139[4];
  m_char           v_3413[18];
  m_char           v_3412[36];
};

struct s_doc_seg
{
   m_char	   v_1001[4];
#if UNUSED
   m_char	   v_1373[4];
#endif
   m_char	   v_3453[4];
};

struct s_cux_seg
{
  m_char           v_6347[4];
  m_char           v_6345[4];
#if UNUSED
  m_char           v_6343[4];
#endif
};

struct s_dtm_seg
{
  struct s_dtm_seg *dtm_next;
  m_char           v_2005[4];
  m_char           v_2380[36];
  m_char           v_2379[4];
};

struct s_fii_seg
{
  struct s_fii_seg *fii_next;
  m_char           v_3035[4];
  m_char           v_3194[18];
  m_char           v_3192[41], 
                   v_3192a[41];
  m_char           v_3433[12];
  m_char           v_3432[71];
  m_char           v_3436[71];
#if UNUSED
  m_char           v_3207[4];
#endif
};

struct s_ftx_seg
{
  struct s_ftx_seg *ftx_next;
  m_char           v_4451[4];
#if UNUSED
  m_char           v_4453[4];
#endif
  m_char           v_4441[4];
  m_char           v_4440[71],
                   v_4440a[71],
                   v_4440b[71],
                   v_4440c[71],
                   v_4440d[71],
                   v_4440e[71];
};

struct s_imd_seg
{
  struct s_imd_seg *imd_next;
  m_char           v_7081[4];
  m_char           v_7009[8];
  m_char           v_7008[76],
                   v_7008a[76];
};

struct s_lin_seg
{
  m_char           v_1082[7];
  m_char           v_7140[36];
  m_char           v_7143[4];
  m_char           v_1222[3];
};

struct s_loc_seg
{
  struct s_loc_seg *loc_next;
  m_char	   v_3227[4];
#if UNUSED
  m_char	   v_3225[26];  /* unused in 302 */
  m_char	   v_1131[4];   /* unused in 302 */
  m_char	   v_3055[4];   /* unused in 302 */
  m_char	   v_3224[18];  /* unused in 302 */
#endif
  m_char	   v_3223[26];
#if UNUSED
  m_char	   v_1131a[4];  /* unused in 302 */
  m_char	   v_3055a[4];  /* unused in 302 */
#endif
  m_char	   v_3222[71];  
#if UNUSED
  m_char	   v_3233[71];  /* unused + only [26] in 302 */
  m_char	   v_1131b[4];  /* unused in 302 */
  m_char	   v_3055b[4];  /* unused in 302 */
  m_char	   v_3232[71];  /* unused in 302 */
  m_char	   v_5479[4];   /* unused in 302 */
#endif
};

struct s_moa_seg
{
  struct s_moa_seg *moa_next;
  m_char           v_5025[4];
  m_char           v_5004[19];
  m_char           v_6345[4];
  m_char           v_6343[4];
  m_char           v_4405[4];
};

struct s_nad_seg
{
  m_char           v_3035[4];
#if UNUSED
  m_char           v_3124[36],      /* unused in 302 */ 
                   v_3124a[36],     /* unused in 302 */
                   v_3124b[36],     /* unused in 302 */
                   v_3124c[36],     /* unused in 302 */
                   v_3124d[36];     /* unused in 302 */
#endif
  m_char           v_3036[76],
                   v_3036a[76],
                   v_3036b[76],
                   v_3036c[76],
                   v_3036d[76];
#if UNUSED
  m_char           v_3042[36],      /* unused in 302 */ 
                   v_3042a[36],     /* unused in 302 */
                   v_3042b[36];     /* unused in 302 */
#endif
  m_char           v_3164[76];
  m_char           v_3251[16];      /* changed from [10] */
  m_char           v_3207[4];   
};

struct s_pat_seg
{
  m_char           v_4279[4];
  m_char           v_2475[4];
  m_char           v_2009[4];
  m_char           v_2151[4];
  m_char           v_2152[4];
};

struct s_pcd_seg
{
  m_char           v_5245[4];
  m_char           v_5482[9];
};

struct s_pia_seg
{
  struct s_pia_seg *pia_next;
  m_char	   v_4347[4];
  m_char	   v_7140[36];
};

struct s_pri_seg
{
  m_char           v_5125[4];
  m_char           v_5118[16];
  m_char           v_5375[4];
};

struct s_qty_seg
{
  struct s_qty_seg *qty_next;
  m_char           v_6063[4];
  m_char           v_6060[16];
  m_char           v_6411[4];
};

struct s_rff_seg
{
  m_char           v_1153[4];
  m_char           v_1154[36];
};

struct s_tax_seg
{
  m_char           v_5283[4];
  m_char           v_5153[4];
  m_char           v_5152[36];
  m_char           v_5279[41];
  m_char           v_5278[18];
  m_char           v_5273[13];
  m_char           v_5305[4];
};

struct s_xcd_seg
{
  /* struct s_xcd_seg *xcd_left;*/
  /* struct s_xcd_seg *xcd_right; */
  
  m_char	   v_X001[19];		/* running main number */
  m_char	   v_X002[19];		/* running sub number */
  m_char	   v_X003[4];		/* market-id / call type */
  m_char	   v_X004[15];		/* call timestamp */
  m_char	   v_X005[15];		/* CR timestamp */
  m_char	   v_X005a[13];
  m_char	   v_X006[19];		/* rated call volume */
  m_char	   v_X006a[19];	/* rounded call volume */
  m_char    v_6411[4];		/* measure unit qualifier */
  m_char	   v_X007[19];		/* clicks */
  
  m_char	   v_5004c[19];   /* new in 302 */
  m_char    v_6345c[4];    /* new in 302 */
  m_char	   v_6343c[4];    /* new in 302 */

  m_char    v_5004[19];		/* monetary amount */
  m_char    v_6345[4];		/* currency coded */
  m_char    v_6343[4];		/* currency qualifier */

  m_char	   v_X008[6];		/* service */
  m_char	   v_X009[6];		/* tariff time */
  m_char	   v_X009a[6];		/* tariff zone */
  m_char	   v_X010[6];		/* split call indic. */
  m_char	   v_X011[6];		/* type indic. */
  m_char	   v_X012[6];		/* late call ind. */
  m_char	   v_X013[6];		/* rating type ind. */
  m_char	   v_X014[6];		/* call action ind. */
  m_char	   v_X015[6];		/* call completion ind. */
  m_char	   v_X016[6];		/* microcell ind. */
  m_char	   v_X017[6];		/* type */
  m_char	   v_X017a[2];		/* rated party TON */
  m_char	   v_X018[2];		/* PLMN ind. */
  m_char	   v_X019[11];		/* PLMN name */
  m_char	   v_X019a[19];   /* new in 302 */
  m_char           v_5004a[19];   	/* VPLMN tap net rate */ /* located behind X035 in 302 */
  m_char           v_6345a[4];    	/* currency ind. */      /* located behind X035 in 302 */
  m_char           v_6343a[4];    	/* currency qualifier */ /* located behind X035 in 302 */
  m_char           v_5004b[19];		/* VPLMN tap tax rate */
  m_char           v_6345b[4];    	/* currency ind.  */     /* located behind X035 in 302 */
  m_char           v_6343b[4];    	/* currency qualifier */ /* located behind X035 in 302 */
  m_char	   v_X024[31];		/* VPLMN VAT Nr. */
  m_char	   v_X020[19];		/* dialled digits */
  m_char	   v_X021[3];		/* destination */
  m_char	   v_X022[6];     /* only [2] in 302, but [6] in <= 209 */
  m_char	   v_X023[36];		/* zonepoint short desc. */
  m_char	   v_X024a[36];		/* CGI indicator */
  m_char	   v_X025[36];		/* calling number */
  m_char	   v_X026[81];		/* remark */
  m_char	   v_X027[2];     /* new in 302 */
  m_char	   v_X028[2]; 	  /* new in 302 */
  m_char	   v_X029[11];	  /* new in 302 */
#ifdef US_TIMM_LAYOUT
  m_char	   v_X030[36];	  /* new in 302 */
  m_char	   v_X031[36];	  /* new in 302 */
  m_char           v_X032[36];    /* new in 302 */
  m_char	   v_X033[36];    /* new in 302 */
  m_char           v_X034[36];    /* new in 302 */
  m_char   	   v_X035[36];    /* new in 302 */
                                  /* look at 5004a etc. above */
  m_char	   v_X036[4];     /* new in 302 */
  m_char	   v_X037[2];     /* new in 302 */ 
  m_char	   v_X038[36];	  /* new in 302 */
  m_char           v_X039[36];    /* new in 302 */
  m_char  	   v_X040[6];     /* new in 302 */
  m_char           v_X041[6];     /* new in 302 */
#endif
  m_char	   v_X042[31];    /* new in 302 */
  m_char	   v_X043[19];    /* new in 302 */
  m_char	   v_X044[3];     /* new in 302 */
  m_char	   v_X045[36];    /* new in 302 */
  m_char	   v_X046[14];    /* new in 302 */
  m_char	   v_X047[19];    /* new in 302 */
  m_char	   v_X048[81];    /* new in 302 */
};

/****************************************/
/*                                      */
/* Gruppen mit Level 3                  */
/*                                      */
/****************************************/

struct s_group_37
{
  struct s_pcd_seg *pcd;
};

struct s_group_38
{
  struct s_group_38 *g_38_next;
  struct s_moa_seg *moa;
};

struct s_group_40
{
  struct s_group_40 *g_40_next;
  struct s_tax_seg *tax;
  struct s_moa_seg *moa;
};

/****************************************/
/*                                      */
/* Gruppen mit Level 2                  */
/*                                      */
/****************************************/

struct s_group_3
{
   struct s_group_3 *g_3_next;
   struct s_rff_seg *rff;
};

struct s_group_4
{
   struct s_group_4 *g_4_next;
   struct s_doc_seg *doc;
};

struct s_group_5
{
  struct s_group_5 *g_5_next;
  struct s_cta_seg *cta;
  struct s_com_seg *com;
};

struct s_group_23
{
  struct s_group_23 *g_23_next;
  struct s_moa_seg  *moa;
};

struct s_group_25
{
  struct s_group_25 *g_25_next;
  struct s_pri_seg  *pri;
};

struct s_group_26
{
  struct s_group_26 *g_26_next;
  struct s_rff_seg  *rff;
};

struct s_group_30
{
  struct s_group_30 *g_30_next;
  struct s_tax_seg  *tax;
  struct s_moa_seg  *moa;
};

struct s_group_31
{
  struct s_group_31 *g_31_next;
  struct s_nad_seg  *nad;
  struct s_loc_seg  *loc;
};

struct s_group_35
{
  struct s_group_35 *g_35_next;
  struct s_alc_seg  *alc;
  struct s_group_37 *g_37;
  struct s_group_38 *g_38;
  struct s_group_40 *g_40;
};

struct s_group_99
{ 
  struct s_group_99 *g_99_next;
  struct s_xcd_seg *xcd;
};

/****************************************/
/*                                      */
/* Gruppen mit Level 1                  */
/*                                      */
/****************************************/

struct s_group_1
{
  struct s_group_1 *g_1_next;
  struct s_rff_seg *rff;
};

struct s_group_2
{
  struct s_group_2 *g_2_next;
  struct s_nad_seg *nad;
  struct s_fii_seg *fii;
  struct s_group_3 *g_3;
  struct s_group_4 *g_4;
  struct s_group_5 *g_5;
};

struct s_group_6
{
  struct s_group_6 *g_6_next;
  struct s_tax_seg *tax;
};

struct s_group_7
{
  struct s_group_7 *g_7_next;
  struct s_cux_seg *cux;
};

struct s_group_8
{
  struct s_group_8 *g_8_next;
  struct s_pat_seg *pat;
  struct s_dtm_seg *dtm;
};

struct s_group_22
{
  struct s_group_22 *g_22_next;
  struct s_lin_seg  *lin;
  struct s_pia_seg  *pia;
  struct s_imd_seg  *imd;
  struct s_qty_seg  *qty;
  struct s_dtm_seg  *dtm;
  struct s_ftx_seg  *ftx;
  struct s_group_23 *g_23;
  struct s_group_25 *g_25;
  struct s_group_26 *g_26;
  struct s_group_30 *g_30;
  struct s_group_31 *g_31;
  struct s_group_35 *g_35;
  struct s_group_99 *g_99;
};

struct s_group_45
{
  struct s_group_45 *g_45_next;
  struct s_moa_seg  *moa;
};

struct s_group_47
{
  struct s_group_47 *g_47_next;
  struct s_tax_seg  *tax;
  struct s_moa_seg  *moa;
};

/****************************************/
/*                                      */
/* Level 0                              */
/*                                      */
/****************************************/

struct s_timm_message
{
  struct s_timm_message *timm_next;
  struct s_unh_seg  *unh;
  struct s_bgm_seg  *bgm;
  struct s_dtm_seg  *dtm;
  struct s_ftx_seg  *ftx;
  struct s_group_1  *g_1;
  struct s_group_2  *g_2;
  struct s_group_6  *g_6;
  struct s_group_7  *g_7;
  struct s_group_8  *g_8;
  struct s_group_22 *g_22;
  struct s_uns_seg  *uns;
  struct s_cnt_seg  *cnt;
  struct s_group_45 *g_45;
  struct s_group_47 *g_47;
  struct s_unt_seg  *unt;
};

struct s_TimmInter
{
  struct s_TimmInter    *next;
  struct s_unb_seg      *unb;
  struct s_timm_message *timm;
  struct s_unz_seg   	*unz;
};


#define TIMM_MESSAGE struct s_timm_message
#define TIMM_INTERCHANGE struct s_TimmInter

struct s_TimmInter *parse_timm (char *);
void *timm_malloc(size_t);
void test_print(struct s_TimmInter *);
void free_timm(struct s_TimmInter *);
int hook_is_undefined(char *text);
int anz_error(int erl,int vorh,char *name);
int copy_str(char *item,int allowed_len,int state);
int bgh_yyinput(char *buf, int max_size);
int unpossible_error(char *text);

#endif /* ifndef TIMM_PARSER_H */
