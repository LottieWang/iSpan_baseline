exe = ispan

cc = "$(shell which g++)" 
#flags = -I. -fopenmp -march=athlon64 -O3
flags = -I. -fopenmp -O3 -fcilkplus -DCILK -march=native
#flags += -std=c++11
INCLUDE_PATH = -I../../../parlaylib/include/

ifeq ($(debug), 1)
	flags+= -DDEBUG 
endif

objs = $(patsubst %.cpp,%.o,$(wildcard ../../lib/*.cpp))\
			$(patsubst %.cpp,%.o,$(wildcard *.cpp))

deps = $(wildcard ../../lib/*.h) \
	   			$(wildcard ./*.hpp) \
				$(wildcard *.h) \
				Makefile

%.o:%.cpp $(deps)
	$(cc) -c $< -o $@ $(flags) $(INCLUDE_PATH)

$(exe):$(objs)
	$(cc) $(objs) -o $(exe) $(flags) $(INCLUDE_PATH)


test:$(exe)
	./scc_cpu /mnt/raid0_huge/yuede/data/pokec/fw_begin.bin /mnt/raid0_huge/yuede/data/pokec/fw_adjacent.bin /mnt/raid0_huge/yuede/data/pokec/bw_begin.bin /mnt/raid0_huge/yuede/data/pokec/bw_adjacent.bin 

clean:
	rm -rf $(exe) $(objs) 
