/**************************************************************************/
/*  MODULE : Invoice types interface                                      */
/*                                                                        */
/*  AUTHOR : N.Bondarczuk                                                 */
/*                                                                        */
/*  CREATION DATE : 24.09.97                                              */
/*                                                                        */
/*  DESCRIPTION : Contains functions for presentation of informations     */
/*                from TIMM messages                                      */
/**************************************************************************/

#include "types.h"
#include "gen.h"
#include "inv_types.h"

#if 0	/* just for version.sh */
static char *SCCS_VERSION = "4.1";
#endif


struct charge_type charge_types_assoc[] = {
  {"U", "USAGE", CHARGE_TYPE_USAGE},
  {"A", "ACCESS", CHARGE_TYPE_ACCESS},
  {"S", "SERVICE", CHARGE_TYPE_SERVICE},
  {"O", "OTHER", CHARGE_TYPE_OTHER}
};

char *chargeType2Id(enum ChargeType ct) {
  int i;

  for (i = 0; i < sizeof(charge_types_assoc); i++)
    if (charge_types_assoc[i].val == ct)
      return charge_types_assoc[i].id;

  return (char *)0;
}

char *chargeType2Descr(enum ChargeType ct) {
  int i;

  for (i = 0; i < sizeof(charge_types_assoc); i++)
    if (charge_types_assoc[i].val == ct)
      return charge_types_assoc[i].descr;

  return (char *)0;
}

int isAccessType(char *str) {
  if (str[0] == 'P' || str[0] == 'A' || str[0] == 'C')
    return TRUE;
  return FALSE;
}

struct usage_type usage_types_assoc[] = {
  {"I", "INBOUND", USAGE_TYPE_INBOUND},
  {"O", "OUTBOUND", USAGE_TYPE_OUTBOUND},
  {"N", "NORMAL", USAGE_TYPE_NORMAL}
};

struct ppu_type ppuid_types_assoc[] = {
  {"R", USAGE_TYPE_INBOUND, "SURCHARGE"},
  {"r", USAGE_TYPE_INBOUND, "VPLMN CHARGE"},
  {"m", USAGE_TYPE_OUTBOUND, "VPLMN"},
  {"F", USAGE_TYPE_NORMAL, "FLAT"},
  {"B", USAGE_TYPE_NORMAL, "BUNDLED"},
  {"S", USAGE_TYPE_NORMAL, "SCALED"},
  {"T", USAGE_TYPE_NORMAL, "TIERED"},
  {"E", USAGE_TYPE_NORMAL, "EXTERNAL"}
};

int isUsageType(char *str) {
  int i;
  
  for (i = 0; i < sizeof(ppuid_types_assoc); i++)
    if (EQ(ppuid_types_assoc[i].id, str))
      return TRUE;
  
  return FALSE;
}

int id2UsageType(char *str) {
  int i;
  
  for (i = 0; i < sizeof(ppuid_types_assoc); i++)
    if (EQ(ppuid_types_assoc[i].id, str))
      return ppuid_types_assoc[i].ut;
  
  return USAGE_TYPE_UNKNOWN;
}

char *usageType2Id(enum UsageType ut) {
  int i;

  for (i = 0; i < sizeof(usage_types_assoc); i++)
    if (usage_types_assoc[i].val == ut)
      return usage_types_assoc[i].id;

  return (char *)0;
}

char *usageType2Descr(enum UsageType ut) {
  int i;

  for (i = 0; i < sizeof(usage_types_assoc); i++)
    if (usage_types_assoc[i].val == ut)
      return usage_types_assoc[i].descr;

  return (char *)0;
}

struct connection_type connection_types_assoc[] = {
  {"A", "AIR", CONNECTION_TYPE_AIR},
  {"I", "INTERCONNECT", CONNECTION_TYPE_INTERCONNECT},
  {"C", "CALL_FORWARD", CONNECTION_TYPE_CALL_FORWARD},
  {"R", "ROAMING", CONNECTION_TYPE_ROAMING}
};


int isConnectionType(char *id) {
  int i;
  
  for (i = 0; i < sizeof(connection_types_assoc); i++)
    if (EQ(connection_types_assoc[i].id, id))
      return TRUE;
  
  return FALSE;
}
  
int id2ConnectionType(char *id) {
  int i;
  
  for (i = 0; i < sizeof(connection_types_assoc); i++)
    if (EQ(connection_types_assoc[i].id, id))
      return connection_types_assoc[i].val;
  
  return CONNECTION_TYPE_UNKNOWN;
}

char *connectionType2Id(enum ConnectionType ct) {
  int i;

  for (i = 0; i < sizeof(connection_types_assoc); i++)
    if (connection_types_assoc[i].val == ct)
      return connection_types_assoc[i].id;

  return (char *)0;
}

char *connectionType2Descr(enum ConnectionType ct) {
  int i;

  for (i = 0; i < sizeof(connection_types_assoc); i++)
    if (connection_types_assoc[i].val == ct)
      return connection_types_assoc[i].descr;

  return (char *)0;
}

