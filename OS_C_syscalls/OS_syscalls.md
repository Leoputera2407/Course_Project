lab0.c:

This is the source file. The program copies from the stdin to stdout, or input file to output file, if they are specified using the --input and --output flag respectively. The copying is done by using syscalls read() and write(). 

The program in total has 5 optional flags: the input and output flags, previously mentioned, and also 3 more-- segfault, catch and dump-core.

Options are implemented using the getup-long API. 

Using the --segfault command, a seg fault can be aritifically induced. This is implemented by trying to access a NULLPTR; while, --catch is used to register the signal handler for this segfault.

Any error will result in the program exitting with their respective exit code as specified in the spec.

Usage will be printed if  there is an invalid invocation of the command( ie invalid arguments, no source file, etc..), happened.

| Score Attained |
| -------------- |
| 100/100        |

Makefile:

The makefile has these targets:

 * default:
    * Build the program's executable with -Wall -Wextra -g flags
* clean
  * Removes tarballs and executables
* dist
  * Creates the tarball that is to be submitted. It contains the source file, Makefile, README, and 2 images.
* check
  * Runs some smoke-tests for the program. Some tests are:
    * InNOutTest
      * Checks if program exit with status code 0 after copying input to output.
    * InMatchOutTest
      * Checks if the input is copied correctly to output by comparing them.
    * OpenTest
      * Checks if program exit with status code 1 when it can't open the input file.
    * CreatTest
      * Checks if program exit with status code 2 when it can't write into the output file.
    * ArgTest
      * Checks if program exit with status code 3 when an invalid option was passed.
    * CatchSegTest
      * Checks if program exit with status code 4 when a segfault is caught.



README:

It contains the description of each of the files submitted and sources that are used to do the lab.



backtrace.png:

Screenshot to gdb backtrace of the segfault we induced using --segfault.



breakpoint.png:

Screenshot of the breakpoint to the bad assigment, as we discovered in backtrace.png. The program runs to that breakpoint, and the nullptr was inspected to confirm that it's NULL in gdb.



Some problems:

I was clueless on how to output the core-dump. I can't seem to find a method in C to do it. 

The other functionalities can be figured out from reading their man-page:

* File descriptor: http://web.cs.ucla.edu/classes/winter19/cs111/labs/fd_juggling.html
* getopt: http://man7.org/linux/man-pages/man3/getopt.3.html
* signal: http://man7.org/linux/man-pages/man2/signal.2.html
* strerror: http://man7.org/linux/man-pages/man3/strerror.3.html





​	
