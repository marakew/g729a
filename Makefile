EXE = G729a
CC = gcc
CXX = g++

CFLAG = -O2
CFLAGCXX = -O2

OBJS=$(patsubst %.c,%.o,$(wildcard *.c))
OBJS+=$(patsubst %.cpp,%.o,$(wildcard *.cpp))

.c.o:
	$(CC) $(CFLAG) -c $<
.cpp.o:
	$(CXX) $(CFLAGCXX) -c $<

$(EXE):$(OBJS)
	ar -crv libg729a.a $(OBJS)

test:$(OBJS)
	$(CC) $(CFLAG) -o test $(OBJS) -lm

clean:
	rm -rf *.o
	rm -rf *.a
