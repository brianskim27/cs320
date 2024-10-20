CFLAGS = -Wall -Wextra -DDEBUG -g -std=c++14

all: Predictor.o main.o
	g++ Predictor.o main.o -o predictors
	
main.o: main.cpp
	g++ -c $(CFLAGS) -c main.cpp

Predictor.o: Predictor.cpp
	g++ -c $(CFLAGS) -c Predictor.cpp 

run: all
	./predictors

clean:
	rm Predictor.o main.o