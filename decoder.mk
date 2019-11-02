EXE = G729a
CC = gcc
CXX = g++

CFLAG = -O0 -g -DTEST_DECODER -DTEST_CONTROL
CFLAGCXX = -O0 -g

OBJS=$(patsubst %.c,%.o,$(wildcard *.c))
OBJS+=$(patsubst %.cpp,%.o,$(wildcard *.cpp))

.c.o:
	$(CC) $(CFLAG) -c $<
.cpp.o:
	$(CXX) $(CFLAGCXX) -c $<

$(EXE):$(OBJS)
	ar -crv libg729a.a $(OBJS)

decoder:$(OBJS)
	$(CC) $(CFLAG) -o decoder $(OBJS) -lm

clean:
	rm -rf *.o
	rm -rf *.a