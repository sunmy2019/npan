VPATH = src:demo:include:build

OBJS = layers/physical_layer.o layers/internet_layer.o layers/transport_layer.o layers/application_layer.o ./data.o
OUTDIR = build/layers build/lib

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

npan: $(OUTDIR) lib/libnpan.a

$(OBJS): npan.h npan-internal.h

$(OUTDIR):
	mkdir -p $@

.PHONY: builddir
builddir: $(OUTDIR)

lib/libnpan.a: $(OBJS)
	ar rvs build/$@ $(addprefix build/,$?)

build/main: $(OUTDIR) lib/libnpan.a ./main.o
	g++-11 build/main.o -Lbuild/lib -lnpan -o $@ $(LINKFLAGS)

build/test-k12: $(OUTDIR) lib/libnpan.a ./test-k12.o
	g++-11 build/test-k12.o -Lbuild/lib -lnpan -o $@ $(LINKFLAGS)


.PHONY: run
run: main
	build/main example_input.txt

.PHONY: prof
prof: run
	gprof build/main gmon.out

.PHONY: clean
clean:
	-rm -rf *.out *.o *.a
	-rm -rf build/
	
