#ifndef FILE_OPERATIONS_H
#define FILE_OPERATIONS_H
#include <sys/types.h>
#include <stdbool.h>

// changes %20 to ' '
int open_file(char* filename); // NOTE: `filename` here could be caps, or no caps. You need to normalize matching filenames by lowercasing them. Returns fildes of the file.
off_t get_filelength(const int fildes); // stat the file. This should just be a one liner of fstat() syscall.
bool get_mimetype(char* mimetype, const char* filename); //get mimetype, e.g. application/json 
// returns true if supported mimetype, false otherwise.
#endif
