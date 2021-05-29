VPATH = src:build

.SUFFIXES: .cpp

builddir:
	mkdir -p build/layers

.cpp.o:
	g++-11 -std=c++20 -Wall -O3 -c $^ -o build/$*.o

OBJS = layers/physical_layer.o layers/internet_layer.o layers/transport_layer.o layers/application_layer.o ./data.o ./main.o

object: builddir $(OBJS)

npan:  object
	g++-11 $(addprefix build/,$(OBJS)) -o build/main -lfmt
	

run: npan
	build/main example_input.txt

clean:
	rm -r build/

.: npan