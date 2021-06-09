CC = cc -c
CPPFLAGS = -I. -I../include -I../../include 
CFLAGS = -g

SRC = iov_list.c stream_utl.c line_list.c \
	stream_image.c stream_single.c stream_multi.c stream.c \
	generator_doc.c generator_inv.c generator.c invoice_class.c 

OBJ = iov_list.o stream_utl.o line_list.o \
	stream_image.o stream_single.o stream_multi.o stream.o \
	generator_doc.o generator_inv.o generator.o invoice_class.o

all: $(OBJ)

show:
	@echo CC=$(CC)
	@echo CPPFLAGS=$(CPPFLAGS)
	@echo CFLAGS=$(CFLAGS)
	@echo LDFLAGS=$(LDFLAGS)
	@echo LDLIBS=$(LDLIBS)
	@echo SYS_LIBS=$(SYS_LIBS)

clean:
	rm -f *~ *.o *.a core

	
