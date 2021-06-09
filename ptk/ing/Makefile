CC = cc -c
#ALPHA = -assume noaligned_objects
ALPHA = -misalign
CPPFLAGS = -I../../include -I../include -I. 
CFLAGS =  $(ALPHA) -g

SRC = gen_inl.c gen_inp.c turnover.c interest.c
OBJ = gen_inl.o gen_inp.o turnover.o interest.o

all: $(OBJ)
	ar -r libinp.a $(OBJ)

show:
	@echo CC=$(CC)
	@echo CPPFLAGS=$(CPPFLAGS)
	@echo CFLAGS=$(CFLAGS)
	@echo LDFLAGS=$(LDFLAGS)
	@echo LDLIBS=$(LDLIBS)
	@echo SYS_LIBS=$(SYS_LIBS)


clean:
	rm -f *~ *.o *.a core






