.PHONY: compile clean

all: compile

compile: main

clean:
	rm -f *.o
	rm -f main

main: main.o data_logger.o rhd2000registers.o bcm2835_chip.o
	g++ -o main main.o data_logger.o rhd2000registers.o bcm2835_chip.o bcm2835.o

main.o: main.cpp data_logger.h
	g++ -c main.cpp

data_logger.o: data_logger.cpp data_logger.h rhd2000registers.h bcm2835_chip.h
	g++ -c data_logger.cpp

rhd2000registers.o: rhd2000registers.cpp rhd2000registers.h
	g++ -c rhd2000registers.cpp

bcm2835_chip.o: bcm2835_chip.cpp bcm2835_chip.h
	g++ -c bcm2835_chip.cpp

bcm2835.o: bcm2835-1.71/src/bcm2835.c bcm2835-1.71/src/bcm2835.h
	gcc -c bcm2835-1.71/src/bcm2835.c
