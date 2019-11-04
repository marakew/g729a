EXE = G729a
CC = gcc
CXX = g++

CFLAG = -O0 -g
CFLAGCXX = -O0 -g

OBJS=$(patsubst %.c,%.o,$(wildcard *.c))
OBJS+=$(patsubst %.cpp,%.o,$(wildcard *.cpp))

.c.o:
	$(CC) $(CFLAG) -c $<
.cpp.o:
	$(CXX) $(CFLAGCXX) -c $<

codera.o: codera.c
	$(CC) $(CFLAG) -DTEST_ENCODER -DTEST_CONTROL -o $@ -c $<

bits.o: bits.c
	$(CC) $(CFLAG) -DTEST_CONTROL -o $@ -c $<

$(EXE):$(OBJS)
	ar -crv libg729a.a $(OBJS)

coder:$(OBJS)
	$(CC) $(CFLAG) -o coder $(OBJS) -lm

clean:
	rm -rf *.o
	rm -rf *.a
