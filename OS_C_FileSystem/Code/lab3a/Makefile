#Name:Hanif Leoputera Lim
#Email: hanifleoputeralim@gmail.com
#UID: 504971751
CC=gcc
CFLAGS= -g -Wall -Wextra 

default: build

build: 
	${CC} ${CFLAGS} -o lab3a lab3a.c

dist:
	tar -cvzf lab3a-504971751.tar.gz lab3a.c ext2_fs.h Makefile README

clean:
	rm -f lab3a lab3a-504971751.tar.gz
