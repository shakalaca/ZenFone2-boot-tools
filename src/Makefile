CC = gcc
AR = ar rcs
RM = rm -f

CFLAGS = -O3
LDFLAGS = -Wl

all: libmincrypt.a mkbootimg unpackbootimg

libmincrypt.a:
	make -C libmincrypt

mkbootimg: mkbootimg.o
	$(CC) -o $@ $^ -L. -lmincrypt $(LDFLAGS)

mkbootimg.o: mkbootimg.c
	$(CC) -o $@ $(CFLAGS) -c $<

unpackbootimg: unpackbootimg.o
	$(CC) -o $@ $^ $(LDFLAGS)

unpackbootimg.o: unpackbootimg.c
	$(CC) -o $@ $(CFLAGS) -c $<

clean:
	$(RM) mkbootimg mkbootimg.o unpackbootimg unpackbootimg.o
	$(RM) libmincrypt.a
	make -C libmincrypt clean
	