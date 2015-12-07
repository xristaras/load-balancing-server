#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <curl/curl.h>

#define PORT    5555
#define MAXMSG  1024

size_t append_headers(char* ptr, size_t size, size_t nitems, void* userdata);
size_t append_html(char* ptr, size_t size, size_t nmemb, void* userdata);
int serve_request(int client_socket);
int initialize_socket(uint16_t port);
int handle_request(int filedes);
