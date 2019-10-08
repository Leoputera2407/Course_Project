#NAME: Hanif Leoputera Lim
#EMAIL: hanifleoputeralim@gmail.com
#ID: 504971751
CC=gcc
CFLAGS=-Wall -Wextra -std=gnu11 -pthread -g
default: build
build: lab2_add lab2_list
clean:
	rm -f *.o *.tar.gz lab2_add lab2_list 
dist: graphs
	tar -czvf lab2a-504971751.tar.gz *.c *.h Makefile README *.sh *.gp *.png *.csv
tests: build 
	./output_list.sh
	./output_add.sh
test: tests
graphs: tests
	gnuplot lab2_add.gp
	gnuplot lab2_list.gp
lab2_add:
	${CC} ${CFLAGS} lab2_add.c -o lab2_add
lab2_list:
	${CC} ${CFLAGS} lab2_list.c SortedList.c -o lab2_list
