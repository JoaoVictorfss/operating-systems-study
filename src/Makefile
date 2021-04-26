all:
	gcc -c main.c queue.c statistics.c
	gcc -o main queue.o main.o statistics.o -lpthread -lrt
	clear
	./main
