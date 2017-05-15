DIRS = shmutil example

all:
	for d in ${DIRS};do \
		cd $${d} && make && make install && cd ..;\
	done

clean:
	for d in ${DIRS};do \
		cd $${d} && make clean && cd ..;\
	done