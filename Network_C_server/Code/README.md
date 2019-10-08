
# Pseudocode for a request response process.

1. `while(true)`:
    1. Accept connection. `read()` in http request to buffer.
    2. Parse Http request to some struct that identifies the endpoint and method
        ```c
            // defined in http_format.h
           struct http_request {
               char* endpoint_resource; // filename of resource to get. Parsed from http request.
               char* method;
           };
           bool parse_http_request(char* request_str, struct http_request* r);
           // return true if parsing successful, false if failed.
        ```
        - if method is not GET, then indicate flag to respond with 405 (method not allowed).
        - if parsing failed, respond immediately with 400 (Bad Request).
    3. open the requested resource (ignore caps), return fd if successful.
        ```c
            // defined in file_operations.h
            int open_file(const char* filename); // NOTE: `filename` here could be caps, or no caps. You need to normalize matching filenames by lowercasing them. Returns fildes of the file.
            off_t get_filelength(const int fildes); // stat the file. This should just be a one liner of fstat() syscall.
            bool get_mimetype(char* mimetype, const char* filename); //get mimetype, e.g. application/json 
            // returns true if supported mimetype, false otherwise.
        ```
        - if fail, respond immediately with 403 (permission denied) OR 404 (not found)
        1. stat the requested resource for file size. This is for `Content-Length` response header.
        2. Parse the mime-type of the resource requested for `Content-Type` response header.
    4. generate headers for http response. `write()` the headers.
        ```c
            // defined in http_format.h
            void get_error_response_header(char* buf, int http_response_code); // put into buf
            void get_200_OK_response_header(char* buf, char* mimetype, off-t content_length); // put into buf
            // we would then write() 'buf' to the connfd.
        ```
    5. `write()` the resource's binary data itself, if step 3 successful.
    6. close resource fd and fd connection to client.

