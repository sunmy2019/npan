VPATH = src:demo:include:build

OBJS = layers/physical_layer.o layers/internet_layer.o layers/transport_layer.o layers/application_layer.o ./data.o

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
	g++-11 -std=c++20 -Wall $(COMPFLAGS) -Iinclude -c $< -o build/$*.o 

.: npan

$(OBJS): npan.h npan-internal.h

npan: builddir lib/libnpan.a

main: npan ./main.o
	g++-11 build/main.o -Lbuild/lib -lnpan -o build/main $(LINKFLAGS)

builddir:
	@mkdir -p build/layers
	@mkdir -p build/lib

object: builddir $(OBJS)

lib/libnpan.a: $(OBJS)
	ar rvs build/$@ $(addprefix build/,$?)

run:
	build/main example_input.txt


prof: run
	gprof build/main gmon.out
	
clean:
	-rm -r *.out *.o *.a
	-rm -r build/
	
