VPATH = src:build

OBJS = layers/physical_layer.o layers/internet_layer.o layers/transport_layer.o layers/application_layer.o ./data.o ./main.o

DEBUG ?= 0
ifeq ($(DEBUG), 1)
    COMPFLAGS = -g -fsanitize=address
	LINKFLAGS = -lfmt -lasan
else
    COMPFLAGS = -O3
	LINKFLAGS = -lfmt
endif

PROF ?= 0
ifeq ($(PROF), 1)
    COMPFLAGS += -pg
	LINKFLAGS += -pg
endif

.SUFFIXES: .cpp

.cpp.o:
	g++-11 -std=c++20 -Wall $(COMPFLAGS) -c $^ -o build/$*.o 

.: npan

npan:  object
	g++-11 $(addprefix build/,$(OBJS)) -o build/main $(LINKFLAGS)

builddir:
	mkdir -p build/layers

object: builddir $(OBJS)


run:
	build/main example_input.txt

clean:
	rm -r build/

prof: run
	gprof build/main gmon.out