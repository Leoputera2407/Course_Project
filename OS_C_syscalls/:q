#Name: Hanif Leoputera Lim
#Email: hanifleoputeralim@gmail.com
#ID: 504 971 751


CC=gcc
CFLAGS=-Wall -Wextra -g

default:
	${CC} ${CFLAGS} lab0.c -o lab0
clean:
	rm -f *.o *.tar.gz lab0
dist: default
	tar -czvf lab0-504971751.tar.gz lab0.c Makefile README backtrace.png breakpoint.png
check: default allTests
	
#SmokeTests
InNOutTest:
	echo "Hey hey hey!" > inFile.txt; \
	./lab0 --input inFile.txt --output outFile.txt; \
	if [[ $$? -ne 0 ]]; then \
		echo "TEST FAILED: Error copying from input to output."; \
	else \
		echo "TEST PASSED: Succesfully exit with status 0"; \
	fi 
	rm -f inFile.txt outFile.txt

InMatchOutTest:
	echo "Hey hey hey!" > inFile.txt; \
	./lab0 --input inFile.txt --output outFile.txt; \
	cmp inFile.txt outFile.txt; \
	if [[ $$? -ne 0 ]]; then \
		echo "TEST FAILED: Input doesn't match output."; \
	else \
		echo "TEST PASSED: Input matches output"; \
	fi
	rm -f inFile.txt outFile.txt
OpenTest:
	./lab0 --input nonExistentFile.txt; \
	if [[ $$? -ne 1 ]]; then \
		echo "TEST FAILED: Error handling invalid input."; \
	else	\
		echo "TEST PASSED: Invalid input properly handled. Successfully exit with status 1"; \
	fi 

CreatTest:
	echo "YO!" > inFile.txt; \
	touch outFile.txt; \
	chmod u-w outFile.txt; \
	./lab0 --input inFile.txt --output outFile.txt &> /dev/null; \
	if [[ $$? -ne 2 ]]; then \
		echo "TEST FAILED: Error handling unwritable output file."; \
	else \
		echo "TEST PASSED: Unwritable output properly handled. Successfully exit with status 2"; \
	fi
	rm -f inFile.txt outFile.txt

ArgTest:
	echo|./lab0 --invalid; \
	if [[ $$? -ne 3 ]]; then \
		echo "TEST FAILED: Error handling invalid options."; \
	else \
		echo "TEST PASSED: Invalid options are properly handled. Succesfully exit with status 3"; \
	fi 

CatchSegTest:
	echo|./lab0 --segfault --catch &> /dev/null; \
	if [[ $$? -ne 4 ]]; then \
		echo "TEST FAILED: Segmentation fault catch returns the wrong exit code"; \
	else \
		echo "TEST PASSED: Segmentation fault caught successfully, exit with status 4"; \
	fi

allTests: InNOutTest InMatchOutTest OpenTest CreatTest ArgTest CatchSegTest
	


	

		
		
		

