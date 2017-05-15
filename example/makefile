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

BINARY_SMQ_CON := sconsumer
OI_OBJS_SMQ_CON  = mq_single_consumer.o

BINARY_SMQ_PRO := sproducer
OI_OBJS_SMQ_PRO = mq_single_producer.o

BINARY_MMQ_CON := mconsumer
OI_OBJS_MMQ_CON  = mq_multi_consumer.o

BINARY_MMQ_PRO := mproducer
OI_OBJS_MMQ_PRO = mq_multi_producer.o

all:$(BINARY_READER) $(BINARY_WRITER) $(BINARY_SMQ_CON) $(BINARY_SMQ_PRO) $(BINARY_MMQ_CON) $(BINARY_MMQ_PRO)

.cc.o:
	$(CXX) $(CFLAGS) -c $^ -o ./$@ $(LIB_COMM)

$(BINARY_READER):$(OI_OBJS_READER)
	$(CXX) $(C_ARGS) -o $@ $^ $(LIB_COMM)

$(BINARY_WRITER):$(OI_OBJS_WRITER)
	$(CXX) $(C_ARGS) -o $@ $^ $(LIB_COMM)

$(BINARY_SMQ_CON):$(OI_OBJS_SMQ_CON)
	$(CXX) $(C_ARGS) -o $@ $^ $(LIB_COMM)

$(BINARY_SMQ_PRO):$(OI_OBJS_SMQ_PRO)
	$(CXX) $(C_ARGS) -o $@ $^ $(LIB_COMM)

$(BINARY_MMQ_CON):$(OI_OBJS_MMQ_CON)
	$(CXX) $(C_ARGS) -o $@ $^ $(LIB_COMM)

$(BINARY_MMQ_PRO):$(OI_OBJS_MMQ_PRO)
	$(CXX) $(C_ARGS) -o $@ $^ $(LIB_COMM)

install:

clean:
	rm -f *.o *.lock $(BINARY_READER) $(BINARY_WRITER) $(BINARY_SMQ_CON) $(BINARY_SMQ_PRO) $(BINARY_MMQ_CON) $(BINARY_MMQ_PRO)
