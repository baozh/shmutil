LIB_INSTALL_COMM_PATH = ../lib

INC_COMM_PATH = -I./   -I../include

LIB_COMM = -lpthread -L$(LIB_INSTALL_COMM_PATH) -lshmutil

.SUFFIXES: .o .cc

C_ARGS := -g -O -Wall -fno-common

cxx = g++

CFLAGS = -g -O -Wall  $(INC_COMM_PATH)

BINARY_READER := reader
OI_OBJS_READER  = hashtable_reader.o

BINARY_WRITER := writer
OI_OBJS_WRITER = hashtable_writer.o

all:$(BINARY_READER) $(BINARY_WRITER)

.cc.o:
	$(CXX) $(CFLAGS) -c $^ -o ./$@ $(LIB_COMM)

$(BINARY_READER):$(OI_OBJS_READER)
	$(CXX) $(C_ARGS) -o $@ $^ $(LIB_COMM)

$(BINARY_WRITER):$(OI_OBJS_WRITER)
	$(CXX) $(C_ARGS) -o $@ $^ $(LIB_COMM)

clean:
	rm -f *.o $(BINARY_READER) $(BINARY_WRITER)
