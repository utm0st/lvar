CXX=g++
FLAGS=-msse4.2 -Wall -std=c++20 -fno-rtti -fno-exceptions -O3 -fno-builtin -Iinc

all:

ctest:
	$(CXX) $(FLAGS) tests/test_m4.cpp -o tests/test_m4.out

rtest:
	./tests/test_m4.out

clean:
	rm ./tests/*.out
