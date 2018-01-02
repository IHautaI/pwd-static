CXX=clang++ -std=c++14 -g -O3

all: clean main

clean:
	$$(if [ -f main ]; then rm main; fi)

main:
