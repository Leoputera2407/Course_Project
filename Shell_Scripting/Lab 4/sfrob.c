#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>

char decrypt(const char character)
{	
  return character^42;
}

int frobcmpUp (unsigned char const* a, unsigned char const* b){
    //Loop thru each frobnicated letter in the word
    //Array of letters ends with a space byte
    for(;;a++, b++){
        
        char upperA = toupper(decrypt(*a));
        char upperB = toupper(decrypt(*b));
        //Both reach end of word, means they're the same words.
        if (*a == ' ' && *b == ' '){
            return 0;
            // 'a' reaches end of word first or curr letter of 'a' is
            // smaller than 'b', menas first word argument is smaller.
        }else if (*a == ' ' || upperA < upperB) {
            return -1;
            // 'b' reached end of word first or curr letter of 'b' is
            // smaller than 'a', means first word argument is bigger.
        }else if (*b == ' ' || upperA > upperB) {
            return 1;
        }
    }

    
    
}
//Will be used by q sort later
int frobcmp (char const* a, char const* b){
	//Loop thru each frobnicated letter in the word
	//Array of letters ends with a space byte
	for(;;a++, b++){
		//Both reach end of word, means they're the same words.
		if (*a == ' ' && *b == ' '){
			return 0;
		// 'a' reaches end of word first or curr letter of 'a' is
		// smaller than 'b', menas first word argument is smaller.
		}else if (*a == ' ' || (decrypt(*a) < decrypt(*b))) {	
			return -1;
		// 'b' reached end of word first or curr letter of 'b' is 
		// smaller than 'a', means first word argument is bigger.
		}else if (*b == ' ' || (decrypt(*a) > decrypt(*b))) {
			return 1;
		}
	}
}

int cmpUp(const void* in1, const void* in2)
{
    //We cast to pointers to pointers since thats what our
    //words array holds
    unsigned const char* a = *(unsigned const char**)in1;
    unsigned const char* b = *(unsigned const char**)in2;
    return frobcmpUp(a,b);
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
    
    int flagF = 0;
    
    if(argc == 2 && strcmp(argv[1],"-f") == 0){
        flagF = 1;
    }
    
    struct stat fileStat;

    if(fstat(0, &fileStat) < 0){
        fprintf(stderr, "Input Error: %d\n", errno);
        exit(1);
    }
    
	char curr, *buffer, **word, *temp;
	int isSpace, isFrob, wordIndex, bufferIndex;
  long long fileStat_size;   
	bufferIndex = wordIndex = 0;
	//Create a buffer array to form each word
	buffer = (char*) malloc(sizeof(char));
	//Add complete words into the array below
	word = 	(char**) malloc(sizeof(char*));
    fileStat_size = fileStat.st_size;
   printf("Stdin is ye %lld big", fileStat_size);    
    
    //Make sure that fileStat is a regular file, as st_size can only read regular files
    if(S_ISREG(fileStat.st_mode)){
        temp = (char *) malloc(sizeof(char) * fileStat_size );
        read(0, &temp, fileStat_size);
        int counter;
        for(counter = 0; counter < fileStat_size; counter++){
            curr = temp[0];
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
        
        
        if(flagF == 1){
            qsort(word, wordIndex, sizeof(char*), cmpUp);
        }else{
           qsort(word, wordIndex, sizeof(char*), cmp);
        }
        
        
        
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
        free(temp);
    
    }
    
	exit(0);
}
