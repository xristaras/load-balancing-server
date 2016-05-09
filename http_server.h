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
#include <pthread.h>
#include <math.h>

//#define ROUND_ROBIN
//#define LEAST_CONN
#define LEAST_TOTAL_TIME
//#define LEAST_LATENCY

#define ROUND_ROBIN_ID       "rr"
#define LEAST_CONN_ID        "lc"
#define LEAST_TOTAL_TIME_ID  "lt"
#define LEAST_LATENCY_ID     "ll"

#define PORT          80
#define PORT_NUM      5000   //for other servers ports
#define MAXMSG        1024
#define SEC_INTERVAL  10

#define S2ELAB_IP         "83.212.112.122"
#define S2ELABSTUDENT_IP  "83.212.85.236"
#define S2ELABTEACHER_IP  "83.212.87.48"
#define S2ELABTUTOR_IP    "83.212.112.124"

#ifdef ROUND_ROBIN
typedef struct {
   char ipaddress[16];
} AppServer;

typedef struct {
   AppServer servers[4];
   int last_served_index;
} AppServerContainer;
#endif

#ifdef LEAST_CONN
typedef struct {
   char ipaddress[16];
} AppServer;

typedef struct {
   AppServer servers[4];
   int now_serving[4];  //num of active connections for each server
} AppServerContainer;
#endif

#ifdef LEAST_TOTAL_TIME
typedef struct {
   char ipaddress[16];
} AppServer;

typedef struct {
   AppServer servers[4];
   int ms_served[4];  //total num of milliseconds served
} AppServerContainer;
#endif


#ifdef LEAST_LATENCY
typedef struct {
   char ipaddress[16];
} AppServer;

typedef struct {
   AppServer servers[4];
   int weight[4];
} AppServerContainer;
#endif

typedef struct {
  int filedes;
  char lb_method[16];
} RequestHandlerArgs;

int num_clients_connected;
AppServerContainer* servers_container;
pthread_mutex_t lb_state_mutex, clients_counter_mutex;

// server.c
void operate_server(char* lb_method);
size_t append_headers(char* ptr, size_t size, size_t nitems, void* userdata);
size_t append_html(char* ptr, size_t size, size_t nmemb, void* userdata);
#ifdef LEAST_TOTAL_TIME
int serve_request(int client_socket, char* lb_method, int *ms_passed);
#else
int serve_request(int client_socket, char* lb_method);
#endif
int initialize_socket(uint16_t port);
int handle_request(RequestHandlerArgs* args);
void increment_clients_counter();
void decrement_clients_counter();

// load_balancers.c
char* round_robin();
char* least_conn(int* served_by_idx);
char* least_latency(int* served_by_idx);
int init_server_container(AppServerContainer** container_ptr);
int destroy_server_container(AppServerContainer** container_ptr);
int init_server_struct(AppServer* server, char* ip);
char* choose_and_fetch_ip(int* served_by_idx);
void weight_calculator();
char* pretty_print_method(char* lb_method_identifier);
