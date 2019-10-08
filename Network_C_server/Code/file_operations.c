#include <unistd.h>
#include <dirent.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/errno.h>
#include <ctype.h>

#include <stdio.h>

#include "file_operations.h"

// NOTE: `filename` here could be caps, or no caps. You need to normalize matching filenames by lowercasing them. Returns fildes of the file.
// changes %20 to ' '
int open_file(char* filename) {
    DIR* dirp = opendir(".");
    if (dirp == NULL) {
        return -1;
    }
    //replace %20 with space here.
    int filename_len = strlen(filename); // length without null byte.
    for(int i = 0; i + 2 < filename_len; ++i) {
        if (filename[i] == '%' && filename[i+1] == '2' && filename[i+2] == '0') {
            filename[i] = ' ';
            for(int j = i + 1; j + 2 < filename_len; ++j) {
                filename[j] = filename[j + 2];
            }
            filename[filename_len - 2] = '\0';
            break;
        }
    }

    struct dirent* dp;
    while ((dp = readdir(dirp))!= NULL) {
        //NOTE: for now, support only regular files.
        if(dp->d_type == DT_REG && strcasecmp(dp->d_name, filename) == 0) {
            closedir(dirp);
            return open(dp->d_name, O_RDONLY);
        }
    }
    closedir(dirp);
    errno = ENOENT;
    return -1;
}
// stat the file. This should just be a one liner of fstat() syscall.
off_t get_filelength(const int fildes) {
    struct stat stat_buf;
    int result = fstat(fildes, &stat_buf);
    if (result == -1) {
        return -1;
    }
    return stat_buf.st_size;
}

//get mimetype, e.g. application/json 
// returns true if supported mimetype, false otherwise.
/*
 - plain text files encoded in UTF-8: *.html, *.htm, and *.txt 
 - static image files: *.jpg, *.jpeg, and *.png
 - animated image files: *.gif
 */
bool get_mimetype(char* mimetype, const char* filename) {
    char *extension = strrchr(filename, '.');
    //If no extension or hidden file with no extension, assume mimetype is text
    if(!extension || extension == filename){
        strcpy(mimetype, "text/plain; charset=utf8");
        return true;
    }
    //If has extension or !hiddenfile, check is extension is allowed
    if(extension || extension != filename){
        //Lower case it so it's case insensitive
        char* current_ptr = extension + 1;
        if(strncasecmp(current_ptr,"html", 5) == 0
            || strncasecmp(current_ptr, "htm", 4) == 0) {
            strcpy(mimetype, "text/html; charset=utf8");
            return true;
        } else if(strncasecmp(current_ptr, "txt", 4) == 0) {
            strcpy(mimetype, "text/plain; charset=utf8");
            return true;
        } else if(strncasecmp(current_ptr, "jpg", 4) == 0
            || strncasecmp(current_ptr, "jpeg", 5) == 0) {
            strcpy(mimetype, "image/jpeg");
            return true;
        } else if(strncasecmp(current_ptr, "png", 4) == 0) {
            strcpy(mimetype, "image/png");
            return true;
        } else if(strncasecmp(current_ptr, "gif", 4) == 0) {
            strcpy(mimetype, "image/gif");
            return true;
        }
    } 
    return false;
}
