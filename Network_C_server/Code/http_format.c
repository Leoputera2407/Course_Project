#include <stdio.h>
#include <string.h> 
#include <stdbool.h>
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include "http_format.h"
#include <stdlib.h>
/*
 * NOTE: if the request is for path '/', then r->endpoint_resource will be NULL.
 */
 
bool parse_http_request(char* request_str, struct http_request* r){
    // set null for method and endpoint resource.
    r->method = NULL;
    r->endpoint_resource = NULL;
    if(request_str == NULL){
        return false;
    }
    const char *end_of_method = strchr(request_str, ' ');

    //If method is not delimited properly, BADARG
    if(end_of_method == NULL) {
        return false;
    }
    long method_len = end_of_method - request_str; 
    char method[method_len + 1];
    strncpy(method, request_str, method_len);
    method[sizeof(method) - 1]  = 0;


    //Parse the filename
    const char *start_of_filename = strchr(end_of_method, '/') + 1;
    if(start_of_filename == NULL) {
        return false;
    }
    //If dist between method and path (start_of_filename -1) is != 1, BADARG
    //If "/" is not found, BADARG 
    if(((start_of_filename - 1) - end_of_method) != 1 || start_of_filename == NULL){
        return false;
    }
    const char *end_of_filename = strchr(start_of_filename, ' ');
    //If filename is not delimited properly, BADARG
    if(end_of_filename == NULL){
       return false;
    }
    long filename_len = end_of_filename - start_of_filename; // NOT including null byte!
    if (filename_len == 0) { // i.e. if request is just GET /
        r->endpoint_resource = NULL;
    } else {
        char endpoint[filename_len + 1];
        strncpy(endpoint, start_of_filename, filename_len);
        endpoint[filename_len] = 0;
        r->endpoint_resource = strdup(endpoint);
    }
    r->method = strdup(method);
    return true;
}

int get_error_response_header(char* buf, int http_response_code) {
    char* msg_str;
    int msg_len;
    switch(http_response_code) {
        // note: these  include the null byte.
        case 400:
            msg_str = "Bad request";
            break;
        case 403:
            msg_str = "Resource forbidden";
            break;
        case 404:
            msg_str = "Resource not found";
            break;
        case 405:
            msg_str = "Method not allowed";
            break;
        case 415:
            msg_str = "Media not supported";
            break;
        case 500:
            msg_str = "Internal Server error";
            break;
        default:
            msg_str = "Bad request";
            http_response_code = 400;
            break;
    }
    msg_len = strlen(msg_str);
    char *fmt = "HTTP/1.1 %d %s\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: %d\r\n"
                "\r\n"
                "%s";
    int chars_printed = sprintf(buf, fmt, http_response_code, msg_str, msg_len, msg_str);
    return chars_printed;
}

/* e.g. buf could have:
HTTP/1.1 200 Ok
Content-Length:4

OK
*/
int get_200_OK_response_header(char* buf, const char* mimetype, off_t content_length){
  char curr_time[100];
  time_t now = time(0);
  struct tm tm = *gmtime(&now);
  strftime(curr_time, sizeof curr_time, "%a, %d %b %Y %H:%M:%S %Z", &tm);
  char* fmt = "HTTP/1.1 200 OK \r\n"
              "Date: %s\r\n"
              "Content-Type: %s\r\n"
              "Content-Length: %lld\r\n"
              "\r\n";
  int res_len = sprintf(buf, fmt, curr_time, mimetype,content_length);
  return res_len;
}

