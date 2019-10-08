#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[]){
	//Make sure only 2 arguments are inputted.
	if(argc != 3){
		fprintf(stderr, "Error: wrong number of arguments");
		exit(1);
	}
	
	char* from = argv[1];
	char* to = argv[2];

	if(strlen(from) != strlen(to)){
		fprintf(stderr,"Error: from and to don't have the same length");		exit(1);
	}
	int arr;
	int checker;
	//Check whether from has a duplicate byte
	for(arr = 0; arr < strlen(from); arr++){
		for(checker = arr + 1; checker < strlen(from); checker++){
			if(from[arr] == from[checker]){
				fprintf(stderr, "Error: contain duplicate byte");				
				exit(1);
			}
		}
	}
	//Transfer "from" to "to".
	char curr;
	int idx;
	for(curr = getchar(); curr != EOF ; curr = getchar()){
		for(idx = 0; idx < strlen(from); idx++){
			if(from[idx] == curr){
				putchar(to[idx]);
				break;
			}
		}
		if(idx == strlen(from)){
			putchar(curr);
		}
	}
}
