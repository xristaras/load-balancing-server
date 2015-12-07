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

#define S2ELAB_IP         83.212.112.122
#define S2ELABSTUDENT_IP  83.212.85.236
#define S2ELABTEACHER_IP  83.212.87.48
#define S2ELABTUTOR_IP    83.212.112.124

typedef struct {
   char ipaddress[16];
} AppServer;

typedef struct {
   AppServer servers[4];
   int last_served_index;
} AppServerContainer;

AppServerContainer* servers_container;

// server.c
void operate_server(char* selected_algorithm);
size_t append_headers(char* ptr, size_t size, size_t nitems, void* userdata);
size_t append_html(char* ptr, size_t size, size_t nmemb, void* userdata);
int serve_request(int client_socket);
int initialize_socket(uint16_t port);
int handle_request(int filedes);

// load_balancers.c
int init_server_container(AppServerContainer* container_ptr);
int destroy_server_container(AppServerContainer* container_ptr);
int init_server_struct(AppServer* server, char* ip);
char* choose_and_fetch_ip();

