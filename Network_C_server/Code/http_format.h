#ifndef HTTP_FORMAT_H 
#define HTTP_FORMAT_H
#include <stdbool.h>
#include <sys/types.h>
struct http_request {
   char* endpoint_resource;
   char* method;
};
bool parse_http_request(char* request_str, struct http_request* r);
int get_error_response_header(char* buf, int http_response_code); // put into buf
int get_200_OK_response_header(char* buf, const char* mimetype, off_t content_length); // put into buf
/* e.g. buf could have:
HTTP/1.1 200 Ok
Content-Length:4

OK
*/
#endif
