physical_layer: 
	g++-11 -O3 -c src/layers/physical_layer.cpp -o build/physical_layer.o

internet_layer: 
	g++-11 -O3 -c src/layers/internet_layer.cpp -o build/internet_layer.o

transport_layer: 
	g++-11 -O3 -c src/layers/transport_layer.cpp -o build/transport_layer.o

data:
	g++-11 -O3 -c src/data.cpp -o build/data.o

npan: data physical_layer internet_layer transport_layer
	g++-11 -O3 src/main.cpp build/data.o build/physical_layer.o build/internet_layer.o build/transport_layer.o -o build/main -lfmt

run: npan
	build/main example_input.txt

.: npan