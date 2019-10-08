#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/errno.h>
#include <sys/uio.h>
#include <signal.h>
#include <arpa/inet.h>

#include "file_operations.h"
#include "http_format.h"
#include "utils.h"

#define REQ_BUFF_LEN 900
#define RES_BUFF_LEN 200
#define RES_DATA_BUFF_LEN 65336
#define DEFAULT_PORT 7900 

int backlog_max = 10;
/*
 The backlog parameter defines the maximum length for the queue of pending connections.
     If a connection request arrives with the queue full, the client may receive an error with
     an indication of ECONNREFUSED.
*/

// our buffers.
char req_buff[REQ_BUFF_LEN];
char res_buff[RES_BUFF_LEN];
char mimetype_str[30];
char res_data_buff[RES_DATA_BUFF_LEN];


// our connection fds.
int sockfd, connfd;

struct req_res_stat {
    int bytes_read;
    int bytes_readinto_res; // useful for freeing res_buff
    int bytes_readinto_res_data; // used for freeing res_data_buff
};

struct http_request current_req;

//responds with http code.
int send_http_response(int connfd, struct req_res_stat * rrs) 
{ 
    rrs->bytes_readinto_res_data = 0;
    // first, read in the http request.
    int bytes_read = read(connfd, req_buff, REQ_BUFF_LEN);
    fprintf(stdout, "%s", req_buff);
    rrs->bytes_read = bytes_read;
    if(bytes_read == -1) {
        fprintf(stderr, "[SERVER] error read syscall: %s.\n", strerror(errno));
        int res_len = get_error_response_header(res_buff, 500);
        rrs->bytes_read = 0;
        rrs->bytes_readinto_res = write(connfd, res_buff, res_len);
        return 500;
    }

    // Then, parse it.
    bool parse_success = parse_http_request(req_buff, &current_req);
    if (!parse_success) {
        int res_len = get_error_response_header(res_buff, 400);
        rrs->bytes_readinto_res = write(connfd, res_buff, res_len);
        return 400;
    }
    if(strncmp(current_req.method, "GET", 4) != 0) {
        // method not supported.
        int res_len = get_error_response_header(res_buff, 405);
        rrs->bytes_readinto_res = write(connfd, res_buff, res_len);
        return 405;
    }
    // endpoint is just '/': this is not valid!
    if(current_req.endpoint_resource == NULL) {
        int res_len = get_error_response_header(res_buff, 404);
        rrs->bytes_readinto_res = write(connfd, res_buff, res_len);
        return 404;
    }


    // then, open the resource.
    int fildes = open_file(current_req.endpoint_resource);
    if (fildes == -1) {
        int error_code = 400;
        if(errno == EACCES) {
            error_code = 403;
        } else if (errno == ENOENT) {
            error_code = 404;
        }
        fprintf(stderr, "[SERVER] open_file for %s: %s\n", current_req.endpoint_resource, strerror(errno));
        int res_len = get_error_response_header(res_buff, error_code);
        rrs->bytes_readinto_res = write(connfd, res_buff, res_len);
        return error_code;
    }

    // check if mimetype supported.
    bool mimetype_is_supported = get_mimetype(mimetype_str, current_req.endpoint_resource);
    if(!mimetype_is_supported) {
        fprintf(stderr, "[SERVER] mimetype not supported. Error. \n");
        int res_len = get_error_response_header(res_buff, 415); // 415 media not supported.
        rrs->bytes_readinto_res = write(connfd, res_buff, res_len);
        return 415;
    }

    // then, get some stats about the resource.
    off_t content_length = get_filelength(fildes);
    if (content_length == -1) {
        fprintf(stderr, "[SERVER] fstat error for file. %s\n", strerror(errno));
        int res_len = get_error_response_header(res_buff, 500);
        rrs->bytes_readinto_res = write(connfd, res_buff, res_len);
        close(fildes);
        return 500;
    }

    // then, craft the response message, and send the initial headers.
    rrs->bytes_readinto_res = get_200_OK_response_header(res_buff, mimetype_str, content_length);
    bool has_readinto_init_headers = false;
    // Finally, send the resource in binary form.
    int read_res;
    while((read_res = read(fildes, res_data_buff, RES_DATA_BUFF_LEN)) != 0) {
        if(read_res == -1) {
            fprintf(stderr, "[SERVER] read error for data. %s\n", strerror(errno));
            int res_len = get_error_response_header(res_buff, 500);
            rrs->bytes_readinto_res = write(connfd, res_buff, res_len);
            close(fildes);
            return 500;
        }
        rrs->bytes_readinto_res_data = read_res;
        if (!has_readinto_init_headers) {
            const struct iovec header_data [2] = {
                {res_buff, rrs->bytes_readinto_res}, 
                {res_data_buff, rrs->bytes_readinto_res_data}
            };
            int total_written = writev(connfd, header_data, 2);
            if (total_written == -1) {
                fprintf(stderr, "[SERVER] iov write error for data. %s\n", strerror(errno));
                int res_len = get_error_response_header(res_buff, 500);
                rrs->bytes_readinto_res = write(connfd, res_buff, res_len);
                close(fildes);
                return 500;
            } else {
                has_readinto_init_headers = true;
            }
        } else {
            int write_res = write(connfd, res_data_buff, read_res);
            if(write_res == -1) {
                fprintf(stderr, "[SERVER] write error for data. %s\n", strerror(errno));
                rrs->bytes_readinto_res = get_error_response_header(res_buff, 500);
                if(write(connfd, res_buff, rrs->bytes_readinto_res))
                    rrs->bytes_readinto_res = 0;
                close(fildes);
                return 500;
            }
        }
    }
    close(fildes);
    return 200;
}

// HANDLE SIGPIPE when, e.g. client cuts connection midway.
// e.g. curl localhost:7900/lalal | head -n 2
void initialize_sighandler(struct sigaction* sa, void (*handler)(int) ) {
    memset(sa, 0, sizeof (sigaction));
    sigemptyset(&(sa->sa_mask));
    sa->sa_handler = handler;
}


void sigpipe_handlerfunc(int signo) {
    //just close the connfd here.
    unbuf_printf(STDERR_FILENO,
            "[SERVER] SIGPIPE handler: signal %d received. Closing connfd.\n", signo);
    close(connfd);

}
void sigtermint_handlerfunc(int signo) {
    //close socket here.
    unbuf_printf(STDERR_FILENO,
            "[SERVER] SIGTERMINT handler: signal %d received. Closing sockfd and shutting down .\n", signo);
    close(connfd);
    _exit(1);

}
int main(int argc, char** argv)
{
    int PORT = DEFAULT_PORT;
    char c;
    // read port argument here.
    while ((c = getopt (argc, argv, "p:")) != -1) {
    switch (c) {
      case 'p':
        PORT = atoi(optarg);
        break;
      case '?':
        if (optopt == 'p')
          fprintf (stderr, "[SERVER] option -p requires a port argument.\n");
        return 1;
      }
    }

    struct sigaction sigpipe_handler, sigtermint_handler;
    initialize_sighandler(&sigpipe_handler, sigpipe_handlerfunc);
    initialize_sighandler(&sigtermint_handler, sigtermint_handlerfunc);
    struct sockaddr_in servaddr, cli;

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        fprintf(stderr, "[SERVER] socket creation failed...\n");
        exit(1);
    }
    int option = 1;
    // we reuse the address.
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    sigaction(SIGINT, &sigtermint_handler, NULL);
    sigaction(SIGTERM, &sigtermint_handler, NULL);
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) {
        fprintf(stderr, "[SERVER] socket bind failed: %s\n", strerror(errno));
        exit(0);
    }


    // zero the data buffers.
    bzero(req_buff, REQ_BUFF_LEN);
    bzero(res_buff, RES_BUFF_LEN);
    bzero(res_data_buff, RES_DATA_BUFF_LEN);

    // Now server is ready to listen and verification 
    if ((listen(sockfd, backlog_max)) != 0) {
        fprintf(stderr, "[SERVER] Listen failed: %s\n", strerror(errno));
        close(sockfd);
        exit(1);
    }
    else {
        fprintf(stdout, "[SERVER] listening on port %d\n", PORT);
    }

    socklen_t len = sizeof(cli);

    sigaction(SIGPIPE, &sigpipe_handler, NULL);
    while (true) {
        struct req_res_stat rrs;
        // Accept the data packet from client and verification
        connfd = accept(sockfd, (struct sockaddr*)&cli, &len);
        char* client_addr = inet_ntoa(cli.sin_addr);
        if (connfd < 0) {
            fprintf(stderr, "[SERVER] acccept failed: %s\n", strerror(errno));
            continue;
        }

        int response_code = send_http_response(connfd, &rrs);
        fprintf(stdout, "[SERVER] response: %s - %s /%s %d\n", client_addr, current_req.method, current_req.endpoint_resource, response_code);
        close(connfd);
        free(current_req.endpoint_resource);
        free(current_req.method);
        if(rrs.bytes_read > 0) {
            bzero(req_buff, rrs.bytes_read);
        }
        if(rrs.bytes_readinto_res > 0) {
            bzero(res_buff, rrs.bytes_readinto_res);
        }
        if(rrs.bytes_readinto_res_data > 0) {
            bzero(res_data_buff, rrs.bytes_readinto_res_data);
        }
    }
    close(sockfd);
    return 0;
}
