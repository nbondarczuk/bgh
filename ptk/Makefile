CPPFLAGS = -I../include -I./include -I. 
CFLAGS = $(GLOBAL_OPT) -g
#-xarch=v9
#-misalign

GEN_SRC = gen/iov_list.c gen/stream_utl.c gen/line_list.c \
	gen/stream_image.c gen/stream_single.c gen/stream_multi.c gen/stream.c \
	gen/generator_doc.c gen/generator_inv.c gen/generator.c 

GEN_OBJ = gen/iov_list.o gen/stream_utl.o gen/line_list.o \
	gen/stream_image.o gen/stream_single.o gen/stream_multi.o gen/stream.o \
	gen/generator_doc.o gen/generator_inv.o gen/generator.o

INV_SRC = inv/payslip_gen.c inv/vat_inv.c inv/vat_sum.c inv/service.c inv/inv_fetch.c inv/inv_gen.c 
INV_OBJ = inv/payslip_gen.o inv/vat_inv.o inv/vat_sum.o inv/service.o inv/inv_fetch.o inv/inv_gen.o

ACC_SRC = acc/occ_queue.c acc/acc_enc_gen.c acc/bal_fetch.c acc/sum_fetch_acc.c acc/vpn.c
ACC_OBJ = acc/occ_queue.o acc/acc_enc_gen.o acc/bal_fetch.o acc/sum_fetch_acc.o acc/vpn.o

SIM_SRC = sim/con_serv.c sim/call_category.c sim/call_list.c sim/legend.c \
	sim/item_interval.c sim/item_list.c sim/item_access.c sim/item_usage.c sim/item_subs.c sim/item_occ.c \
	sim/dealer.c sim/roa_fetch_sim.c sim/sim_enc_gen.c sim/sum_fetch_sim.c

SIM_OBJ = sim/con_serv.o sim/call_category.o sim/call_list.o sim/legend.o \
	sim/item_interval.o sim/item_list.o sim/item_access.o sim/item_usage.o sim/item_subs.o sim/item_occ.o \
	sim/dealer.o sim/roa_fetch_sim.o sim/sim_enc_gen.o sim/sum_fetch_sim.o

WLL_SRC = wll/wll_fetch.c   wll/wll_gen.c
WLL_OBJ = wll/wll_fetch.o   wll/wll_gen.o

DNL_SRC = dnl/dnl_fetch.c dnl/dnl_mul_gen.c
DNL_OBJ = dnl/dnl_fetch.o dnl/dnl_mul_gen.o

ING_SRC = ing/gen_inl.c ing/gen_inp.c ing/timm.c ing/interest.c ing/turnover.c
ING_OBJ = ing/gen_inl.o ing/gen_inp.o ing/timm.o ing/interest.o ing/turnover.o

SRC = timer.c timm.c money.c \
	queue.c inv_types.c date.c shdes2des.c address.c num_pl.c strutl.c timm_print.c \
	env_gen.c \
	$(GEN_SRC) $(INV_SRC) $(ACC_SRC) $(SIM_SRC) $(DNL_SRC) $(WLL_SRC) $(ING_SRC)

OBJ = timer.o timm.o money.o \
	queue.o inv_types.o date.o shdes2des.o address.o num_pl.o strutl.o timm_print.o \
	env_gen.o \
	$(GEN_OBJ) $(INV_OBJ) $(ACC_OBJ) $(SIM_OBJ) $(DNL_OBJ) $(WLL_OBJ) $(ING_OBJ)

all: $(OBJ) 
	cd gen ; make all
	cd inv ; make all
	cd acc ; make all
	cd sim ; make all
	cd wll ; make all
	cd dnl ; make all
	cd ing ; make all
	ar -r libptk.a $(OBJ)

gen:
	(cd gen ; make all)

inv:
	(cd inv ; make all)

acc:
	(cd acc ; make all)	

sim:
	(cd sim ; make all)

itb:
	(cd itb ; make all)

wll:
	(cd wll ; make all)

dnl:
	(cd dnl ; make all)

ing:
	(cd ing; make all)

show:
	@echo CC=$(CC)
	@echo CPPFLAGS=$(CPPFLAGS)
	@echo CFLAGS=$(CFLAGS)
	@echo LDFLAGS=$(LDFLAGS)
	@echo LDLIBS=$(LDLIBS)
	@echo SYS_LIBS=$(SYS_LIBS)

clean:
	rm -f *~ *.o *.a core
	(cd gen; make clean)
	(cd inv; make clean)
	(cd acc; make clean)
	(cd sim; make clean)
	(cd wll; make clean)
	(cd dnl; make clean)
	(cd ing; make clean)

