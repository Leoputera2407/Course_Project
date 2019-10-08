/*
 *Name: Hanif Leoputera Lim
  Email: hanifleoputeralim@gmail.com
  ID: 504 971 751
*/
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

void signal_handler(int sig_num){
  if(sig_num == SIGSEGV){
    fprintf(stderr, "Segmentation fault caught. %s \n", strerror(errno));
    exit(4);
  }
}

void print_usage(){
  printf("Usage: lab0 [scd] --input filename --output filename \n");
}

int main(int argc, char **argv){
  char* infile = NULL;
  char* outfile = NULL;
  int seg_flag = 0;
  int ifd, ofd;

  //An array of 'struct option'
  static struct option long_opt[] = {
    {"input", required_argument, 0, 'i'},
    {"output", required_argument, 0, 'o'},
    {"segfault", no_argument, 0, 's'},
    {"catch", no_argument, 0, 'c'},
    {"dump-core", no_argument, 0, 'd'}
  };

  int opt;
  while((opt = getopt_long(argc, argv, "i:o:scd", long_opt, NULL)) != -1){
    switch(opt){
      case 'i':
        infile = optarg;
        break;
      case 'o':
        outfile = optarg;
        break;
      case 's':
        seg_flag = 1;
        break;
      case 'c':
        signal(SIGSEGV, signal_handler);
        break;
      case 'd':
        break;
      default:
        print_usage();
        exit(3);
    }
  }
  //Seg_flag take precedence
  if(seg_flag){
    //Artificially cause a seg fault by assigning a nullptr
    char* null_ptr = NULL;
    *null_ptr = 's';
  }

  //if input file is specified
  if(infile){
    ifd = open(infile, O_RDONLY);
    if(ifd >= 0){
      //Empty out file descriptor 0 first
      close(0);
      //dup will assign ifd the lowest descriptor 0
      dup(ifd);
      //Free up ifd
      close(ifd);
    }else{
      fprintf(stderr, "Problem with --input.Error from opening file %s. Error: %s \n", infile, strerror(errno));
      exit(1);
    }
  }

  //if output file is specified
  if(outfile){
    ofd = creat(outfile, 0666);
    if(ofd >= 0){
      close(1);
      dup(ofd);
      close(ofd);
    }else{
      fprintf(stderr, "Problem with --output.Error from creating file %s. Error:%s \n", outfile, strerror(errno));
      exit(2);
    }
  }

  //Copies stdin to stdout
  char* buffer;
  buffer = (char*) malloc(sizeof(char));
  while(read(0,buffer,1) > 0){
    write(1, buffer, 1);
  }
  free(buffer);
  exit(0);

}
