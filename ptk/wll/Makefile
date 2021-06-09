#CC = gcc -c
CC = cc -c

CPPFLAGS = -I../../include -I.. -I. -I../inv -I../include
#CFLAGS = -O3 -fsigned-char -fwritable-strings
#CFLAGS = -g2 -fsigned-char -fwritable-strings
CFLAGS = -g

SRC = wll_fetch.c   wll_gen.c
OBJ = wll_fetch.o   wll_gen.o

all: $(OBJ)
#	ar -r libwll.a $(OBJ)

show:
	@echo CC=$(CC)
	@echo CPPFLAGS=$(CPPFLAGS)
	@echo CFLAGS=$(CFLAGS)
	@echo LDFLAGS=$(LDFLAGS)
	@echo LDLIBS=$(LDLIBS)
	@echo SYS_LIBS=$(SYS_LIBS)


clean:
	rm -f *~ *.o *.a core






