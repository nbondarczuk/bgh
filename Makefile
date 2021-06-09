#*************************************************************
#*                                                           *
#* ABSTRACT : MAKEFILE for BSCS BGH.                         *
#*                                                           *
#* AUTHOR  : Bernhard Michler                                *
#*                                                           *
#* CREATED  :                                                *
#*                                                           *
#* MODIFIED : 22. Oct. 1996                                  *
#*                                                           *
#* DESCRIPTION :                                             *
#*                                                           *
#* This makefile builds the Image for the BSCS BGH.          *
#*                                                           *
#* SCCS_VERSION = "1.20.1.2;bscs402/batch/kernel/src/bgh/Makefile, bscs4_bgh, bscs4_021, 97-07-18_b4021;2/26/97"
#*************************************************************

USE_ORACLE = 1

# Explicit selection of packages
# THE VALUES ARE CHECKED WITH IFDEF, SETTING THEM TO 0 HAS 
# NO EFFECT !!!
#USE_PROFILE = 1
#NO_PROC_DEFINE=1
USE_DEBUG = 1
USE_AUTO_SETTINGS = 1
USE_ORACLE_ESQL = 1

#*************************************************************
# Configuration.                                             *
#*************************************************************
include $(MPDE_MKINC)/make.rules
CPP = cpp -P
PROFLAGS:=${PROFLAGS} code=ansi_c

INC_DIR=./include

# Define macros to access sources
PROJECT 	= Makefile

BGH_LIB		= libbgh.a
BGH_C_MAIN	= bgh_main.c

BINDIR := /bscs/binEDS

BGH_LIB_SRC =	bgh_loyal.c \
		bgh_file.c \
		bgh_proc.c \
		bgh_access.c \
		bgherrh.c \
		bgh_bill.c \
		layout.c \
		bgh_form.c \
		bgh_prep.c \
		parser.c

BGH_C_SRC = 	bgh_main.c $(BGH_LIB_SRC)

BGH_H_SRC =	$(INC_DIR)/bgh.h \
		$(INC_DIR)/parser.h \
		$(INC_DIR)/protos.h \
		$(INC_DIR)/bgh_esql.h \
		$(INC_DIR)/bgh_prep.h \
		$(INC_DIR)/fixtext.h \
		$(INC_DIR)/layout.h \
		$(INC_DIR)/bgh_loyal.h

PTK_DIR = 	./ptk

PTK_INC =  	$(PTK_DIR)/include

PTK_DOC_DIR = 	$(PTK_DIR)/gen
PTK_INV_DIR = 	$(PTK_DIR)/inv
PTK_ITB_DIR = 	$(PTK_DIR)/itb
PTK_SIM_DIR = 	$(PTK_DIR)/sim
PTK_ACC_DIR = 	$(PTK_DIR)/acc
PTK_WLL_DIR = 	$(PTK_DIR)/wll
PTK_DNL_DIR = 	$(PTK_DIR)/dnl
PTK_ING_DIR = 	$(PTK_DIR)/ing

INC_ALL_PTK_DIRS = \
		-I$(PTK_DIR) \
		-I$(PTK_INC) \
		-I$(PTK_DOC_DIR) \
		-I$(PTK_INV_DIR) \
		-I$(PTK_ITB_DIR) \
		-I$(PTK_SIM_DIR) \
		-I$(PTK_ACC_DIR) \
		-I$(PTK_WLL_DIR) \
		-I$(PTK_DNL_DIR) \
		-I$(PTK_ING_DIR)

ADT_SRC = 	$(PTK_DIR)/timer.c \
		$(PTK_DIR)/env_gen.c \
		$(PTK_DIR)/queue.c \
		$(PTK_DIR)/inv_types.c \
		$(PTK_DIR)/money.c \
		$(PTK_DIR)/date.c \
		$(PTK_DIR)/timm_print.c \
		$(PTK_DIR)/address.c \
		$(PTK_DIR)/shdes2des.c

ADT_OBJ = 	$(PTK_DIR)/timer.o \
		$(PTK_DIR)/env_gen.o \
		$(PTK_DIR)/queue.o \
		$(PTK_DIR)/inv_types.o \
		$(PTK_DIR)/money.o \
		$(PTK_DIR)/date.o \
		$(PTK_DIR)/timm_print.o \
		$(PTK_DIR)/address.o \
		$(PTK_DIR)/shdes2des.o  

DOC_GEN_SRC = 	$(PTK_DOC_DIR)/iov_list.c \
		$(PTK_DOC_DIR)/generator.c \
		$(PTK_DOC_DIR)/generator_inv.c \
		$(PTK_DOC_DIR)/generator_doc.c \
		$(PTK_DOC_DIR)/line_list.c \
		$(PTK_DOC_DIR)/stream.c \
		$(PTK_DOC_DIR)/stream_image.c \
		$(PTK_DOC_DIR)/stream_multi.c \
		$(PTK_DOC_DIR)/stream_single.c \
		$(PTK_DOC_DIR)/stream_utl.c \
		$(PTK_DOC_DIR)/invoice_class.c

DOC_GEN_OBJ = 	$(PTK_DOC_DIR)/iov_list.o \
		$(PTK_DOC_DIR)/generator.o \
		$(PTK_DOC_DIR)/generator_inv.o \
		$(PTK_DOC_DIR)/generator_doc.o \
		$(PTK_DOC_DIR)/line_list.o \
		$(PTK_DOC_DIR)/stream.o \
		$(PTK_DOC_DIR)/stream_image.o \
		$(PTK_DOC_DIR)/stream_multi.o \
		$(PTK_DOC_DIR)/stream_single.o \
		$(PTK_DOC_DIR)/stream_utl.o \
		$(PTK_DOC_DIR)/invoice_class.o

INV_GEN_SRC = 	$(PTK_INV_DIR)/payslip_gen.c \
		$(PTK_INV_DIR)/service.c \
		$(PTK_INV_DIR)/vat_inv.c \
		$(PTK_INV_DIR)/vat_sum.c \
		$(PTK_INV_DIR)/inv_gen.c \
		$(PTK_INV_DIR)/inv_fetch.c

INV_GEN_OBJ = 	$(PTK_INV_DIR)/payslip_gen.o \
		$(PTK_INV_DIR)/service.o \
		$(PTK_INV_DIR)/vat_inv.o \
		$(PTK_INV_DIR)/vat_sum.o \
		$(PTK_INV_DIR)/inv_gen.o \
		$(PTK_INV_DIR)/inv_fetch.o

ACC_GEN_SRC =  	$(PTK_ACC_DIR)/occ_queue.c \
		$(PTK_ACC_DIR)/acc_enc_gen.c \
		$(PTK_ACC_DIR)/bal_fetch.c \
		$(PTK_ACC_DIR)/sum_fetch_acc.c  \
		$(PTK_ACC_DIR)/vpn.c  

ACC_GEN_OBJ =  	$(PTK_ACC_DIR)/occ_queue.o \
		$(PTK_ACC_DIR)/acc_enc_gen.o \
		$(PTK_ACC_DIR)/bal_fetch.o \
		$(PTK_ACC_DIR)/sum_fetch_acc.o  \
		$(PTK_ACC_DIR)/vpn.o  

SIM_GEN_SRC = 	$(PTK_SIM_DIR)/con_serv.c \
		$(PTK_SIM_DIR)/call_category.c \
		$(PTK_SIM_DIR)/legend.c \
		$(PTK_SIM_DIR)/call_list.c \
		$(PTK_SIM_DIR)/sim_enc_gen.c \
		$(PTK_SIM_DIR)/roa_fetch_sim.c \
		$(PTK_SIM_DIR)/sum_fetch_sim.c \
		$(PTK_SIM_DIR)/item_interval.c \
		$(PTK_SIM_DIR)/item_list.c \
		$(PTK_SIM_DIR)/item_access.c \
		$(PTK_SIM_DIR)/item_usage.c \
		$(PTK_SIM_DIR)/item_occ.c \
		$(PTK_SIM_DIR)/item_subs.c \
		$(PTK_SIM_DIR)/dealer.c  

SIM_GEN_OBJ = 	$(PTK_SIM_DIR)/con_serv.o \
		$(PTK_SIM_DIR)/call_category.o \
		$(PTK_SIM_DIR)/legend.o \
		$(PTK_SIM_DIR)/call_list.o \
		$(PTK_SIM_DIR)/sim_enc_gen.o \
		$(PTK_SIM_DIR)/roa_fetch_sim.o \
		$(PTK_SIM_DIR)/sum_fetch_sim.o \
		$(PTK_SIM_DIR)/item_interval.o \
		$(PTK_SIM_DIR)/item_list.o \
		$(PTK_SIM_DIR)/item_access.o \
		$(PTK_SIM_DIR)/item_usage.o \
		$(PTK_SIM_DIR)/item_occ.o \
		$(PTK_SIM_DIR)/item_subs.o \
		$(PTK_SIM_DIR)/dealer.o 

DNL_GEN_SRC = 	$(PTK_DNL_DIR)/dnl_mul_gen.c \
		$(PTK_DNL_DIR)/dnl_fetch.c \
		$(PTK_INC)/dnl_fetch.h

DNL_GEN_OBJ = 	$(PTK_DNL_DIR)/dnl_mul_gen.o \
		$(PTK_DNL_DIR)/dnl_fetch.o 

WLL_GEN_SRC = 	$(PTK_WLL_DIR)/wll_gen.c \
		$(PTK_WLL_DIR)/wll_fetch.c \

WLL_GEN_OBJ = 	$(PTK_WLL_DIR)/wll_gen.o \
		$(PTK_WLL_DIR)/wll_fetch.o \

ING_GEN_SRC = 	$(PTK_ING_DIR)/gen_inl.c \
		$(PTK_ING_DIR)/gen_inp.c \
		$(PTK_ING_DIR)/timm.c \
		$(PTK_ING_DIR)/interest.c \
		$(PTK_ING_DIR)/turnover.c

ING_GEN_OBJ = 	$(PTK_ING_DIR)/gen_inl.o \
		$(PTK_ING_DIR)/gen_inp.o \
		$(PTK_ING_DIR)/timm.o \
		$(PTK_ING_DIR)/interest.o \
		$(PTK_ING_DIR)/turnover.o	

PTK_SRC = 	$(ADT_SRC) \
		$(PTK_DIR)/workparval.c \
		$(PTK_DIR)/timm.c \
		$(PTK_DIR)/num_pl.c \
		$(PTK_DIR)/strutl.c \
		$(DOC_GEN_SRC) \
		$(INV_GEN_SRC) \
		$(ACC_GEN_SRC) \
		$(SIM_GEN_SRC) \
		$(DNL_GEN_SRC) \
		$(ING_GEN_SRC) \
		$(WLL_GEN_SRC) 

PTK_OBJ = 	$(ADT_OBJ) \
		$(PTK_DIR)/workparval.o \
		$(PTK_DIR)/timm.o \
		$(PTK_DIR)/num_pl.o \
		$(PTK_DIR)/strutl.o \
		$(DOC_GEN_OBJ) \
		$(INV_GEN_OBJ) \
		$(ACC_GEN_OBJ) \
		$(SIM_GEN_OBJ) \
		$(DNL_GEN_OBJ) \
		$(ING_GEN_OBJ) \
		$(WLL_GEN_OBJ) 

BGH_LIB_OBJ = 	$(BGH_LIB_SRC:.c=.o)

BGH_C_OBJ = 	$(BGH_C_SRC:.c=.o)

# Oracle section

BGH_PC_SRC = 	bgh_loyal_esql.pc \
		bgh_grant.pc \
		bgh_vatinv.pc \
		bgh_transx.pc \
		bgh_esql_$(MPDE_OSID).pc 

BGH_PC_OBJ = 	$(BGH_PC_SRC:.pc=.o)

OBJ = 		$(BGH_LIB_OBJ) $(BGH_PC_OBJ)

# Generated sources
BGH_PC_C_SRC = 	${BGH_PC_SRC:.pc=.c}

MKDEP_SRC = 	$(PROJECT) $(BGH_C_SRC) $(BGH_H_SRC) $(BGH_PC_SRC)

# Define source (for version.sh)
SRC = 		$(BGH_C_SRC) $(BGH_PC_SRC) $(BGH_H_SRC) $(PTK_SRC)

LOCAL_VERSION = ""
ifeq ($(OPSYS), SunOS)
CC = 		cc
CFLAGS = 	-g #-xarch=v9 #-misalign
MISCLIBPATH =
MISCLIB =
else
CC = 		cc
CFLAGS = 	-g3
MISCLIBPATH =
MISCLIB =
endif


#
# version control
#	

VERSION_ID  := 4.0.1.31

LEVEL_ID    := \"\@\(\#\)\ BGH\ 4.0.1.31\ 29/08/00\ \(C\)\ EDS\ Poland\"

GEN_VER     := \"2.63\"

BGH_VER     := \"4.0.1.31\"

#
# options: 
# _USE_BCH_PAYMENT_   - use amount calculated by BCH for invoice amount and payment slip, else use amount calculated by BGH
# _CLEAN_IMAGE_       - don't remove bill images in TMP directory
# _DUNSIM_ON_LEVEL_3_ - print SIM info only fordunnings on level 3, else print for all levels
# _MAILING_RULES_V2_  - use PTK specifica mailing rules
# _ENC_USE_BCH_CAT_   - use categories of usage services create by BCH, else create them using calls
# _MACHINE_ROUNDING_  - round final amounts expecting intexact results of conversion from strings to double
# _PROPOSAL_          - use traditional categorization
# _STAT_              - print statistics on TIMM level
# _GRANT_             - use granting module
# US_TIMM_LAYOUT      - for additional info for the origin and destination of a non roaming call
# _BGH_COMMIT_LIMIT_  - allow only fixed number of commit operations
# _USE_DISCOUNT_      - check the existence of Discount Model and handle specific invoices
#

COMPILE_OPT := -D_PROPOSAL_ \
	-D_CLEAN_IMAGE_ \
	-D_MACHINE_ROUNDING_ \
	-D_USE_BCH_PAYMENT_ \
	-DMAKEDIRS \
	-D_DUNSIM_ON_LEVEL_3_ \
	-D_MAILING_RULES_V2_ \
	-D_ENC_USE_BCH_CAT_ \
	-DUS_TIMM_LAYOUT \
	-D_BGH_COMMIT_LIMIT_ \
	-D_USE_DISCOUNT_

VERSION_OPT := -DLEVEL_ID=$(LEVEL_ID) -DGEN_VER=$(GEN_VER) -DBGH_VER=$(BGH_VER)

GLOBAL_OPT  := $(VERSION_OPT) $(COMPILE_OPT) 

CPPFLAGS := $(CPPFLAGS) \
	$(GLOBAL_OPT) \
	-I./include \
	-I./ptk \
	-I./ptk/include \
	-I./ptk/gen \
	-I$(SRCDIR)/include \
	-I../bat/include

$(BGH_LIB): $(BGH_LIB)($(OBJ))

#**************************************************************
# Targets.                                                    *
#**************************************************************

all:	bgh

# with libraries libcommon and libbscs
bgh: $(BGH_C_MAIN) $(BGH_LIB) $(PTK_OBJ) 
	ar -rv libptk.a $(PTK_OBJ)	
	$(CC) $(CPPFLAGS) $(GLOBAL_OPT) $(CFLAGS) $(LDFLAGS) $(BGH_C_MAIN) \
	-o bgh \
	$(LDLIBS) -L/usr/lib -L. -L./ptk $(MISCLIBPATH) \
	$(MISCLIB) -lbgh $(SYS_LIBS) -lbat -lptk -lbgh

atom:
	make bgh
	atom bgh -tool third

# show all the symbols used for linking
show:
	@echo CC=$(CC)
	@echo CPPFLAGS=$(CPPFLAGS)
	@echo CFLAGS=$(CFLAGS)
	@echo LDFLAGS=$(LDFLAGS)
	@echo LDLIBS=$(LDLIBS)
	@echo SYS_LIBS=$(SYS_LIBS)
	@echo PROFLAGS=${PROFLAGS}
	@echo VERSION_OPT=${VERSION_OPT}

# check with flex lint
# 

install: bgh $(BGH_LIB)
	strip bgh
	mv bgh $(BINDIR)

clean::
	(cd ptk; make clean)
	rm -f libptk.a	
	rm -f TAGS
	rm -f BGH*
	rm -f bgh bgh_test
	rm -f $(BGH_LIB)
	rm -f *.output
	rm -f *.tab.c
	rm -f lex.yy.c
	rm -f $(BGH_PC_C_SRC)
	rm -f *.o
	rm -f *.i
	rm -f *~
	rm -f *.out
	rm -f bgh.third bgh.3log libc.so.bgh.third

tar: clean
	(cd ptk; make clean)
	tar -cvf $(HOME)/source/bgh_$(VERSION_ID).tar .
	gzip $(HOME)/source/bgh_$(VERSION_ID).tar

xlint: $(BGH_C_MAIN) $(BGH_LIB) $(PTK_OBJ)
	lint -XE10000 \
	-I/bscs/batch/kernel/src/bat/include -I./include -I./ptk/include $(INC_ALL_PTK_DIRS) \
	`find . -name "*.c" -print | grep -v vssver.scc`  \
	> ./lint.out

cflow: $(BGH_C_MAIN) $(BGH_LIB) $(PTK_OBJ)
	cflow \
	-I/bscs/batch/kernel/src/bat/include -I./include -I./ptk/include $(INC_ALL_PTK_DIRS) \
	`find . -name "*.c" -print | grep -v vssver.scc` > ./cflow.out

xcflow: $(BGH_C_MAIN) $(BGH_LIB) $(PTK_OBJ)
	cflow -i x \
	-I/bscs/batch/kernel/src/bat/include -I./include -I./ptk/include $(INC_ALL_PTK_DIRS) \
	`find . -name "*.c" -print | grep -v vssver.scc` > ./cflow.out

tags:
	rm -f TAGS
	etags `find . -name "*[ch]" -print | grep -v vssver.scc`

include $(MPDE_MKINC)/make.misc

