INC_COMM_PATH = -I./

HEADERS = $(wildcard ./*.h)

LIB_COMM =

cxx = g++

CFLAGS = -g -O -Wall  $(INC_COMM_PATH)

SRCS=$(wildcard *.c) $(wildcard *.cc) $(wildcard *.cpp)
OBJS=$(addsuffix .o, $(basename $(SRCS)))

OUTPUT := libshmutil.a

all:$(OUTPUT)

.cc.o:
	$(CXX) $(CFLAGS) -o $@ -c $^ $(LIB_COMM)

.c.o:
	$(CXX) $(CFLAGS) -o $@ -c $^ $(LIB_COMM)

$(OUTPUT):$(OBJS)
	ar -rs $@ $^

install:
	mkdir -p ../lib
	mkdir -p ../include
	-cp -R -f $(OUTPUT)  ../lib
	-cp -R -f $(HEADERS) ../include

clean:
	rm -rf *.o *.a ../lib/ ../include/
