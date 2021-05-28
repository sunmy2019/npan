physical_layer:
	g++-11 -O3 -c src/physical_layer.cpp -o build/physical_layer.o

internet_layer:
	g++-11 -O3 -c src/internet_layer.cpp -o build/internet_layer.o

transport_layer:
	g++-11 -O3 -c src/transport_layer.cpp -o build/transport_layer.o

npan: physical_layer internet_layer transport_layer
	g++-11 -O3 src/main.cpp build/physical_layer.o build/internet_layer.o build/transport_layer.o -o build/main -lfmt


.: npan