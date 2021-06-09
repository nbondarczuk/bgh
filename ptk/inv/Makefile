CC = cc -c
CPPFLAGS = -I../../include -I.. -I. -I../include
CFLAGS = -g ${GLOBAL_OPT}
#CFLAGS = -O3 -fsigned-char -fwritable-strings $(GLOBAL_OPT)
#CFLAGS = -g2 -fsigned-char -fwritable-strings $(GLOBAL_OPT)

SRC = payslip_gen.c inv_fetch.c inv_gen.c vat_inv.c vat_sum.c service.c
OBJ = payslip_gen.o inv_fetch.o inv_gen.o vat_inv.o vat_sum.o service.o

all: $(OBJ)
#	ar -r libinv.a $(OBJ)

show:
	@echo CC=$(CC)
	@echo CPPFLAGS=$(CPPFLAGS)
	@echo CFLAGS=$(CFLAGS)
	@echo LDFLAGS=$(LDFLAGS)
	@echo LDLIBS=$(LDLIBS)
	@echo SYS_LIBS=$(SYS_LIBS)


clean:
	rm -f *~ *.o *.a core






