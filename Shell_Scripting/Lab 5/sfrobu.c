#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>

int FLAGF = 0;

char decrypt(const char character)
{	
  return character^42;
}

//Will be used by q sort later
int frobcmp (char const* a, char const* b){
	//Loop thru each frobnicated letter in the word
	//Array of letters ends with a space byte
	for(;;a++, b++){
            if (*a == ' ' && *b == ' '){
          	 return 0;
            // 'a' reaches end of word first or curr letter of 'a' is
            // smaller than 'b', menas first word argument is smaller.
       	    }else if (*a == ' ' && *b != ' ') {
           	 return -1;
            // 'b' reached end of word first or curr letter of 'b' is
            // smaller than 'a', means first word argument is bigger.
       	     }else if (*a != ' ' && *b == ' ') {
            	return 1;
	     }
	   char unfrob_a = decrypt(*a);
	   char unfrob_b = decrypt(*b);
	   if(FLAGF == 1){

		if (unfrob_a == EOF || (unfrob_a >= 0 && unfrob_a <= UCHAR_MAX))
			unfrob_a = toupper(unfrob_a);
		if(unfrob_b == EOF || (unfrob_b >= 0 && unfrob_b <= UCHAR_MAX))
			unfrob_b = toupper(unfrob_b);
	   }
	   if(unfrob_a < unfrob_b){
		return -1;
	   }else if (unfrob_a > unfrob_b){
		return 1;
	   }
	}
	return 0;			
}



int cmp(const void* in1, const void* in2)
{
  //We cast to pointers to pointers since thats what our
  //words array holds
      const char* a = *(const char**)in1;
      const char* b = *(const char**)in2;
      return frobcmp(a,b);
}

void checkReadErr(FILE *p){
	if(ferror(p)){
		fprintf(stderr, "IO Error: %d\n", errno);
		exit(1);
	}
}

void checkAllocErr(void* ptr)
{	
    if (ptr == NULL){
	fprintf(stderr, "Memory Allocation Error: %d\n", errno);
        exit(1);   
    }
}

int main(int argc, char **argv){
	    
    
	
    if(argc >2){
	fprintf(stderr, "Too many arguments pass: %d\n", errno);
	exit(1);
    }
 
    if(argc == 2 && strcmp(argv[1],"-f") == 0){
        FLAGF = 1;
    }
    
    struct stat fileStat;
    
    if(fstat(0, &fileStat) < 0){
        fprintf(stderr, "Input Error: %d\n", errno);
        exit(1);
    }

    
	char curr, *buffer, **word, *temp;
	int isSpace, wordIndex, bufferIndex;
	bufferIndex = wordIndex = 0;
	//Create a buffer array to form each word
	buffer = (char*) malloc(0);
	//Add complete words into the array below
	word = 	(char**) malloc(0);
    	curr = ' ';
   
    //Make sure that fileStat is a regular file, as st_size can only read regular files
    if(S_ISREG(fileStat.st_mode)){
	if(fileStat.st_size == 0){
		free(buffer);
		free(word);
		exit(0);
	}
	long long fileStat_size;
	fileStat_size = fileStat.st_size;	
        temp = (char *) malloc(sizeof(char) * fileStat_size);	
	if(read(0, temp, fileStat_size) < 0){
		fprintf(stderr,"Read Error: %d\n", errno);
		exit(1);
	}
	int isReading = 1;
	while(isReading){
		fileStat_size = fileStat_size + 64;
		temp = realloc(temp, sizeof(char) * fileStat_size);
		int byte_read = read(0, temp, 64);
		if(byte_read == 0){
			fileStat_size = fileStat_size - 64;
			temp = realloc(temp, sizeof(char) * fileStat_size);
			isReading = 0;
			break;
		}
		if(byte_read < 0){
			fprintf(stderr,"Read Error: %d\n", errno);
               		 exit(1);
		}
		int extraSpace = 64 - byte_read;
		fileStat_size = fileStat_size - extraSpace;
		temp = realloc(temp, sizeof(char) * fileStat_size);		
	}
        int counter;
        for(counter = 0; counter < fileStat_size; counter++){
            curr = temp[counter];
            isSpace = (curr == ' ');
            //Add Frob letters into buffer
            buffer = realloc(buffer, (bufferIndex + 1) * sizeof(char));
            checkAllocErr(buffer);
            buffer[bufferIndex] = curr;
            bufferIndex++;
            
            //If a Frob letter is a space, word is complete.
            if(isSpace){
                word = realloc(word, (wordIndex + 1) * sizeof(char*));
                checkAllocErr(word);
                word[wordIndex] = buffer;
                wordIndex++;
                //Reset buffer
                bufferIndex = 0;
                buffer = NULL;
                continue; 
            }
        }
        
     }else{
	while(read(0, &curr, sizeof(char)) > 0){
	     isSpace = (curr == ' ');
            //Add Frob letters into buffer
             buffer = realloc(buffer, (bufferIndex + 1) * sizeof(char));
             checkAllocErr(buffer);
             buffer[bufferIndex] = curr;
             bufferIndex++;		
            
	    //If a Frob letter is a space, word is complete.
            if(isSpace){
                word = realloc(word, (wordIndex + 1) * sizeof(char*));
                checkAllocErr(word);
                word[wordIndex] = buffer;
                wordIndex++;
                //Reset buffer
                bufferIndex = 0;
                buffer = NULL;
                continue; 
	    }
     	}
    }
	
        //After EOF, the latest buffer not added yet
        if(bufferIndex != 0){
            buffer = realloc(buffer, (bufferIndex + 1) * sizeof(char));
            checkAllocErr(buffer);
            //Add Space, since it's be used later to delimit words.
            buffer[bufferIndex] = ' ';
            bufferIndex++;
            //Add completed word
            word = realloc(word, (wordIndex + 1) * sizeof(char*));
            checkAllocErr(word);
            word[wordIndex] = buffer;
            wordIndex++;
            //Reset buffer
            bufferIndex = 0;
            buffer = NULL;
        }

        qsort(word, wordIndex, sizeof(char*), cmp);
         
        int wordCount = 0;
        while(wordCount < wordIndex){
            int wordSize = 0;
            int i;
            for(i = 0; ; i++){
                wordSize++;
                if(word[wordCount][i] == ' '){
                    break;
                }
            }
            if(write(1, word[wordCount], wordSize) == 0){
                fprintf(stderr, "Error while writing!");
                exit(1);
            }
            wordCount++;
        }
        
        int k;
        //Free up mem used
        for(k = 0; k< wordIndex; k++){
            free(word[k]);
        }
        free(word);
	if(S_ISREG(fileStat.st_mode)){
        	free(temp);
	}
	exit(0);
}
