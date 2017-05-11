INC_COMM_PATH = -I./

HEADERS = $(wildcard ./*.h)

LIB_COMM =

cxx = g++

CFLAGS = -g -O -Wall  $(INC_COMM_PATH)

OI_CC_OBJS  = $(patsubst %.cc,%.o, $(wildcard ./*.cc))
OI_C_OBJS  = $(patsubst %.c,%.o, $(wildcard ./*.c))

OUTPUT := libshmutil.a

all:$(OUTPUT)

.cc.o:
	$(CXX) $(CFLAGS) -o $@ -c $^ $(LIB_COMM)

.c.o:
	$(CXX) $(CFLAGS) -o $@ -c $^ $(LIB_COMM)

$(OUTPUT):$(OI_C_OBJS) $(OI_CC_OBJS)
	ar -rs $@ $^

install:
	mkdir -p ../lib
	mkdir -p ../include
	-cp -R -f $(OUTPUT)  ../lib
	-cp -R -f $(HEADERS) ../include

clean:
	rm -rf *.o *.a ../lib/ ../include/
