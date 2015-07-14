CC = gcc
AR = ar rcs
RM = rm -f
CP = cp

CFLAGS = -O3
LIB = libmincrypt.a
LIB_OBJS = rsa.o sha.o sha256.o
INC  = -I..

all:$(LIB)

clean:
	$(RM) $(LIB_OBJS) $(LIB)

$(LIB): $(LIB_OBJS)
	$(AR) $@ $^
	$(CP) $@ ..

%.o: %.c
	$(CC) -o $@ $(CFLAGS) -c $< $(INC)
