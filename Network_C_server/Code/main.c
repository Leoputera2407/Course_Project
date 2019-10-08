#include "http_format.h"
#include "file_operations.h"
#include <stdio.h>

int main() {

   /*char* mime;
    const char *filenames[6] = {"hello.jPeg", "hello.Txt", "sup_homie.GiF", ".hello", "sup.sup",".jpeg"};
    int i=0;
    //TODO: Is just .extension alone e.g. ".jpeg" a valid filename?
    for(; i<6; i++){
        if(get_mimetype(mime, filenames[i])){
             printf("%d is true. Mimetype is %s\n",i, mime);
        }else{
            printf("%d failed\n", i);
        }
    }*/

    char buf[100];
    struct http_request* Request;
    if(parse_http_request("GET /hello.txt HTTP/1.1\n", Request)){
       int filefd = open_file(Request->endpoint_resource);
       off_t content_len = get_filelength(filefd);
       char* mimetype;
       if(get_mimetype(mimetype, Request->endpoint_resource)){
           get_200_OK_response_header(buf,mimetype, content_len);
       }
    }
    printf("%s", buf);
    return 0;
}