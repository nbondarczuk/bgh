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

#if 0	/* just for version.sh */
static char *SCCS_VERSION = "4.1";
#endif


enum ChargeType { 
  CHARGE_TYPE_UNKNOWN = -1,
  CHARGE_TYPE_USAGE = 0, 
  CHARGE_TYPE_ACCESS = 1,
  CHARGE_TYPE_SERVICE = 2,
  CHARGE_TYPE_OTHER = 3
};

struct charge_type {
  char *id;
  char *descr;
  enum ChargeType val;
};

char *chargeType2Id(enum ChargeType);
char *chargeType2Descr(enum ChargeType);


int isAccessType(char *);


enum UsageType {
  USAGE_TYPE_UNKNOWN = -1,
  USAGE_TYPE_NORMAL = 0,
  USAGE_TYPE_INBOUND = 1,
  USAGE_TYPE_OUTBOUND = 2
};

struct usage_type {
  char *id;
  char *descr;
  enum UsageType val;
};

struct ppu_type {
  char *id;
  enum UsageType ut;
  char *desc;
};


int isUsageType(char *);
int id2UsageType(char *);
char *usageType2Id(enum UsageType);
char *usageType2Descr(enum UsageType);

enum ConnectionType {
  CONNECTION_TYPE_UNKNOWN = -1,
  CONNECTION_TYPE_AIR = 0,
  CONNECTION_TYPE_INTERCONNECT = 1,
  CONNECTION_TYPE_CALL_FORWARD = 2,
  CONNECTION_TYPE_ROAMING = 3
};

struct connection_type {
  char *id;
  char *descr;
  enum ConnectionType val;
};

int isConnectionType(char *);
int id2ConnectionType(char *);
char *connectionType2Id(enum ConnectionType);
char *connectionType2Descr(enum ConnectionType);




