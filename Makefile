PROGRAMS=diff_bpp diff_bpp_decode \

CFLAGS=-W -Wall -O2 -g -fPIC -DBUILD_MAIN=1

all: $(PROGRAMS)

diff_bpp: diff_bpp.o common.o
	$(CC) $(CFLAGS) -o $@ $^

diff_bpp_decode: diff_bpp_decode.o common.o
	$(CC) $(CFLAGS) -o $@ $^

common.o: common.c common.h

tags: *.c *.h 
	ctags $^

clean:
	rm -f $(PROGRAMS)
	rm -f *.o
	rm -f *~
	rm -f core
	rm -f tags
