CC = cc -c
CPPFLAGS =  -I../../include -I../include -I. -I../inv -I../sim 
#CFLAGS = -O3 -fsigned-char -fwritable-strings
#CFLAGS = -g2 -fsigned-char -fwritable-strings
CFLAGS = -g

SRC = dnl_fetch.o dnl_mul_gen.c
OBJ = dnl_fetch.o dnl_mul_gen.o

all: $(OBJ)
#	ar -r libdnl.a $(OBJ)

show:
	@echo CC=$(CC)
	@echo CPPFLAGS=$(CPPFLAGS)
	@echo CFLAGS=$(CFLAGS)
	@echo LDFLAGS=$(LDFLAGS)
	@echo LDLIBS=$(LDLIBS)
	@echo SYS_LIBS=$(SYS_LIBS)


clean:
	rm -f *~ *.o *.a core






