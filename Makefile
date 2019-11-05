LIB = libg729ab.a
CC = gcc

ARCH=x86-64
#CFLAGS+=-Dsingle
CFLAGS+=-Wall -O3 -march=$(ARCH) -ffast-math -funroll-loops -fomit-frame-pointer -Werror

OBJS=$(patsubst %.c,%.o,$(wildcard *.c))

all: $(LIB)

.c.o:
	$(CC) $(CFLAGS) -c $<

$(LIB): $(OBJS)
	ar -cr $(LIB) $(OBJS)
	ranlib $(LIB)

clean:
	rm -rf $(LIB) $(OBJS)

