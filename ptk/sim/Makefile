CC = cc -c
CPPFLAGS = -I../../include -I.. -I. -I../inv -I../acc -I../include		
CFLAGS = -g

SRC = con_serv.c call_category.c call_list.c legend.c item_list.c item_access.c item_usage.c item_subs.c item_occ.c \
	dealer.c roa_fetch_sim.c sim_enc_gen.c sum_fetch_sim.c 
OBJ = con_serv.o call_category.o call_list.o legend.o item_list.o item_access.o item_usage.o item_subs.o item_occ.o \
	dealer.o roa_fetch_sim.o sim_enc_gen.o sum_fetch_sim.o	

all: $(OBJ)
#	ar -r libsim.a $(OBJ)

show:
	@echo CC=$(CC)
	@echo CPPFLAGS=$(CPPFLAGS)
	@echo CFLAGS=$(CFLAGS)
	@echo LDFLAGS=$(LDFLAGS)
	@echo LDLIBS=$(LDLIBS)
	@echo SYS_LIBS=$(SYS_LIBS)

clean:
	rm -f *~ *.o *.a core






