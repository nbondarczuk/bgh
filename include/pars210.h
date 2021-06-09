/*******************************************************************************
 * LH-Specification GmbH 1996.
 *
 * All rights reserved.
 * Copying of this software or parts of this software is a violation of German
 * and International laws and will be prosecuted.
 *
 * Project  :   BGH
 *
 * File     :   timm_parser_210.c
 * Created  :   Feb. 1996
 * Author(s):   A. Dreier, T. Rotaru
 *		B. Michler 03.06.1996: 	added new segment CNT and up to now
 *					unused elements for DCH
 *******************************************************************************/

#ifndef TIMM_PARSER_210
#define TIMM_PARSER_210 1

#define VERSION "2.10"
#define VERBOSE 0

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
  m_char           v_0017[7];
  m_char           v_0019[5];
  m_char           v_0020[15];
  m_char           v_0022[15];
  m_char           v_0025[3];
  m_char           v_0026[15];
  m_char           v_0029[2];
  m_char           v_0031[2];
  m_char           v_0032[36];
  m_char           v_0035[2];
};

struct s_unz_seg
{
  m_char           v_0036[7];
  m_char           v_0020[15];
};

struct s_unh_seg
{
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
  m_char           v_1004[36];
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
   m_char	   v_1373[4];
   m_char	   v_3453[4];
};

struct s_cux_seg
{
  m_char           v_6347[4];
  m_char           v_6345[4];
  m_char           v_6343[4];
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
  m_char           v_3207[4];
};

struct s_ftx_seg
{
  struct s_ftx_seg *ftx_next;
  m_char           v_4451[4];
  m_char           v_4453[4];
  m_char           v_4441[4];
  m_char           v_4440[71],
                   v_4440a[71],
                   v_4440b[71],
                   v_4440c[71],
                   v_4440d[71];
};

struct s_imd_seg
{
  struct s_imd_seg *imd_next;
  m_char           v_7081[4];
  m_char           v_7009[8];
  m_char           v_7008[36],
                   v_7008a[36];
};

struct s_lin_seg
{
  m_char           v_1082[7];
  m_char           v_7140[36];
  m_char           v_7143[4];
  m_char           v_3055[4];
  m_char           v_1222[3];
};

struct s_loc_seg
{
  struct s_loc_seg *loc_next;
  m_char	   v_3227[4];
  m_char	   v_3225[26];
  m_char	   v_1131[4];
  m_char	   v_3055[4];
  m_char	   v_3224[18];
  m_char	   v_3223[26];
  m_char	   v_1131a[4];
  m_char	   v_3055a[4];
  m_char	   v_3222[71];
  m_char	   v_3233[71];
  m_char	   v_1131b[4];
  m_char	   v_3055b[4];
  m_char	   v_3232[71];
  m_char	   v_5479[4];
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
  m_char           v_3124[36], 
                   v_3124a[36], 
                   v_3124b[36], 
                   v_3124c[36], 
                   v_3124d[36];
  m_char           v_3036[76],
                   v_3036a[76],
                   v_3036b[76],
                   v_3036c[76],
                   v_3036d[76];
  m_char           v_3045[4];
  m_char           v_3042[36],
                   v_3042a[36],
                   v_3042b[36];
  m_char           v_3164[76];
  m_char           v_3251[10];
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
  m_char           v_5278[18];
  m_char           v_5305[4];
};

struct s_xcd_seg
{
  /* struct s_xcd_seg *xcd_left;*/
  /* struct s_xcd_seg *xcd_right; */
  
  m_char	   v_X001[19];
  m_char	   v_X002[19];
  m_char	   v_X003[4];
  m_char	   v_X004[13];
  m_char	   v_X005[13];
  m_char	   v_X006[19];
  m_char	   v_X006a[19];
  m_char           v_6411[4];
  m_char	   v_X007[19];
  m_char           v_5004[19];
  m_char           v_6345[4];
  m_char           v_6343[4];
  m_char	   v_X008[6];
  m_char	   v_X009[6];
  m_char	   v_X009a[6];
  m_char	   v_X010[2];
  m_char	   v_X011[2];
  m_char	   v_X012[2];
  m_char	   v_X013[2];
  m_char	   v_X014[2];
  m_char	   v_X015[2];
  m_char	   v_X016[2];
  m_char	   v_X017[2];
  m_char	   v_X017a[2];
  m_char	   v_X018[2];
  m_char	   v_X019[11];
  m_char           v_5004a[19];
  m_char           v_6345a[4];
  m_char           v_6343a[4];
  m_char           v_5004b[19];
  m_char           v_6345b[4];
  m_char           v_6343b[4];
  m_char	   v_X024[31];
  m_char	   v_X020[19];
  m_char	   v_X021[3];
  m_char	   v_X023[6];
  m_char	   v_X024a[31];
  m_char	   v_X025[19];
  m_char	   v_X026[81];
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

struct s_timm_message_210
{
  struct s_timm_message_210 *timm_next;
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

struct s_interchange_210
{
  struct s_interchange_210  *next;
  struct s_unb_seg          *unb;
  struct s_timm_message_210 *timm;
  struct s_unz_seg          *unz;
};


#define TIMM_MESSAGE_210 struct s_timm_message_210
#define TIMM_INTERCHANGE_210 struct s_interchange_210

struct s_interchange_210 *parse_timm_210(char *);
void *timm_malloc(size_t);
void test_print(struct s_interchange_210 *);
void free_timm_210(struct s_interchange_210 *);
int hook_is_undefined(char *text);
int anz_error(int erl,int vorh,char *name);
int copy_str(char *item,int allowed_len,int state);
int bgh_yyinput(char *buf, int max_size);
int unpossible_error(char *text);

#endif /* ifndef TIMM_PARSER_210_H */
