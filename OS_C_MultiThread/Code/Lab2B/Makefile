#NAME: Hanif Leoputera Lim
#EMAIL: hanifleoputeralim@gmail.com
#ID: 504971751
CC=gcc
CFLAGS=-Wall -Wextra -std=gnu11 -pthread -g -lprofiler
default: build
build: lab2_list
clean:
	rm -f *.o *.tar.gz lab2_list 
dist: graphs profile.out
	tar -czvf lab2b-504971751.tar.gz *.c *.h Makefile README *.sh *.gp *.png *.csv profile.out
output.sh: build
	chmod +x output.sh
	./output.sh
tests: output.sh
test: tests
graphs: tests
	gnuplot lab2_list.gp
profile.out: build 
	CPUPROFILE=profile.out ./lab2_list --threads=12 --sync=s --iterations=1000
profile:profile.out
	pprof --text ./lab2_list profile.out
lab2_list: lab2_list.c SortedList.c
	${CC} ${CFLAGS} lab2_list.c SortedList.c -o lab2_list
