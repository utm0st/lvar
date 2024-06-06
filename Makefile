CXX=g++
FLAGS=-msse4.2 -Wall -std=c++20 -fno-rtti -fno-exceptions -fno-builtin -Iinc -ggdb

.PHONY: tests rtests

all:

tests:
	$(CXX) $(FLAGS) ./tests/test_m4.cpp -o tests/test_m4.out
	$(CXX) $(FLAGS) ./tests/test_translate.cpp -o tests/test_translate.out
	$(CXX) $(FLAGS) ./tests/test_normalise.cpp -o tests/test_normalise.out
	$(CXX) $(FLAGS) ./tests/test_cross.cpp -o tests/test_cross.out
	$(CXX) $(FLAGS) ./tests/test_dot.cpp -o tests/test_dot.out

rtests:
	./tests/test_m4.out
	./tests/test_translate.out
	./tests/test_normalise.out
	./tests/test_cross.out
	./tests/test_dot.out

clean:
	rm ./tests/*.out

cube:
	$(CXX) $(FLAGS) src/rotating_cube/main.cpp -o src/rotating_cube/main.out -lX11 -lGL

rcube:
	./src/rotating_cube/main.out
