all:
	g++ -Wall -fPIE -std=c++11 -g -ggdb -o lang main.cpp tokenizer.cpp parser.cpp machine.cpp
