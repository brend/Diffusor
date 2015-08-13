
CFLAGS=-g -Wall
OBJ=ipops.o ipio.o ipmem.o diff_tensor.o
LIB=-lm
EXEC=gauss_conv imginfo foo ced libip.a

all: $(EXEC)

foo: foo.c $(OBJ)
	gcc -o foo $(CFLAGS) foo.c $(OBJ) $(LIB)

ced: ced.c $(OBJ)
	gcc -o ced $(CFLAGS) ced.c $(OBJ) $(LIB)

gauss_conv: gauss_conv.c $(OBJ)
	gcc -o gauss_conv $(CFLAGS) gauss_conv.c $(OBJ) $(LIB)

imginfo: imginfo.c $(OBJ)
	gcc -o imginfo $(CFLAGS) imginfo.c $(OBJ) $(LIB)

libip.a: $(OBJ)
	ar rc libip.a $(OBJ)
	ranlib libip.a

diff_tensor.o: diff_tensor.h diff_tensor.c
	gcc -c $(CFLAGS) diff_tensor.c

ipops.o: ipops.h ipops.c
	gcc -c $(CFLAGS) ipops.c

ipmem.o: ipmem.h ipmem.c
	gcc -c $(CFLAGS) ipmem.c

ipio.o: ipio.h ipio.c ipdefs.h
	gcc -c $(CFLAGS) ipio.c

test: all
	./ip

tidy:
	rm -f $(OBJ)

clean: tidy
	rm -f $(EXEC)

sterile: clean
	rm -f *~

anew: clean all

xcode: all
	cp libip.a ../

