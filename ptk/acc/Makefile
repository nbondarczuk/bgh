CC = cc -c
CPPFLAGS = -I../../include -I.. -I. -I../inv -I../sim -I../include
CFLAGS = -g
#CFLAGS = -O3 -fsigned-char -fwritable-strings
#CFLAGS = -g2 -fsigned-char -fwritable-strings
SRC = occ_queue.c acc_enc_gen.c bal_fetch.c sum_fetch_acc.c vpn.c
OBJ = occ_queue.o acc_enc_gen.o bal_fetch.o sum_fetch_acc.o vpn.o

all: $(OBJ)
#	ar -r libacc.a $(OBJ)

show:
	@echo CC=$(CC)
	@echo CPPFLAGS=$(CPPFLAGS)
	@echo CFLAGS=$(CFLAGS)
	@echo LDFLAGS=$(LDFLAGS)
	@echo LDLIBS=$(LDLIBS)
	@echo SYS_LIBS=$(SYS_LIBS)


clean:
	rm -f *~ *.o *.a core






