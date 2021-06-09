/**************************************************************************/
/*  MODULE : Enclosure Account Generator Header                           */
/*                                                                        */
/*  AUTHOR : N.Bondarczuk                                                 */
/*                                                                        */
/*  CREATION DATE : 22.10.97                                              */
/*                                                                        */
/*  DESCRIPTION : Exports  function  creating tagged information          */
/*                necessary for creation of invoice account enclosure     */
/**************************************************************************/
#if 0	/* just for version.sh */
static char *SCCS_VERSION = "1.1";
#endif

/*
 * EXPORT
 */

toenBool genEnclosureAccount(TYPEID poenTypeId, 
                             struct s_TimmInter *inv_ti, 
                             struct s_TimmInter *sum_ti, 
                             struct s_TimmInter *bal_ti,
                             int poiAddrRule); 






