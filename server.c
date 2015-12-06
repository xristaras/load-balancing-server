#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define PORT    5555
#define MAXMSG  1024

#include <curl/curl.h>

size_t append_headers(char* ptr, size_t size, size_t nitems, void* userdata){
  strncat(userdata, ptr, size*nitems);
//  printf("\n\n\nPRINTED AFTER HEADER COPY\n\n\n%s\n\n\n", userdata);
  return size*nitems;
}

size_t append_html(char* ptr, size_t size, size_t nmemb, void* userdata){
//  userdata = malloc(16 + 19 + 25 + size*nmemb + 7 + 1);
//  strcpy(userdata, "HTTP/1.1 200 OK\n");  //16
//  strcat(userdata, "Content-length: 5000\n");   //19
//  strcat(userdata, "Content-Type: text/html\n\n");  //25
  strncat(userdata, ptr, nmemb);
//  printf("\n\n\nPRINTED AFTER HTML COPY\n\n\n\n%s\n\n\n", userdata);
//  printf("IN APPEND_HTML SIZEOF userdata: %d, STRLEN: %d\n\n", sizeof(userdata), strlen(userdata));
  return size*nmemb;
}

int serve_request(int client_socket){

  CURL *curl;
  CURLcode res;
  char response_str[5000];
  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();

  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, "http://83.212.116.210/");
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, append_headers);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, (void*)response_str);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, append_html);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)response_str);
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0 );  //because of a (encoding?) bug in latest version
    printf("ok before curl perform\n");
    res = curl_easy_perform(curl);
    printf("ok after curl perform\n");
    if(res != CURLE_OK){
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }
    else {
      strcat(response_str, "\0");
//      printf("SIZEOF response_str:%d\n\n\n\n\nRESPONSE_STR:%s\n\n\n\n\n", strlen(response_str), response_str);
      write(client_socket, response_str, strlen(response_str));
    }
    curl_easy_cleanup(curl);
  }

  curl_global_cleanup();
  strcpy(response_str, "\0");
  return 0;

}

int main (void){
  extern int initialize_socket(uint16_t port);
  int sock;
  fd_set active_fd_set, read_fd_set;
  int i;
  struct sockaddr_in clientname;
  size_t size;

  /* Create the socket and set it up to accept connections. */
  sock = initialize_socket(PORT);
  if (listen (sock, 1) < 0){
      perror ("listen");
      exit (EXIT_FAILURE);
  }

  /* Initialize the set of active sockets. */
  FD_ZERO (&active_fd_set);
  FD_SET (sock, &active_fd_set);

  while (1){
      /* Block until input arrives on one or more active sockets. */
      read_fd_set = active_fd_set;
      if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0){
          perror ("select");
          exit (EXIT_FAILURE);
      }

      /* Service all the sockets with input pending. */
      for (i = 0; i < FD_SETSIZE; ++i){
        if (FD_ISSET (i, &read_fd_set)){
            if (i == sock){
                /* Connection request on original socket. */
                int new_conn;
                size = sizeof (clientname);
                new_conn = accept (sock, (struct sockaddr *)&clientname, (socklen_t * __restrict__)&size);
                printf("FD_SETSIZE is %d, new connection request, just accepted in %d\n", FD_SETSIZE, i);
                if (new_conn < 0){
                    printf("accept failure\n");
                    perror ("accept");
                    exit (EXIT_FAILURE);
                }
                FD_SET (new_conn, &active_fd_set);
            }
            else {
                /* Data arriving on an already-connected socket. */
                printf("WILL HANDLE %d\n", i);
                handle_request(i);
                close (i);
                FD_CLR (i, &active_fd_set);
            }
        }
      }
  }
}

int initialize_socket (uint16_t port){
  int sock;
  struct sockaddr_in name;

  /* Create the socket. */
  sock = socket (PF_INET, SOCK_STREAM, 0);
  if (sock < 0){
      perror ("socket");
      exit (EXIT_FAILURE);
  }

  /* Give the socket a name. */
  name.sin_family = AF_INET;
  name.sin_port = htons (port);
  name.sin_addr.s_addr = htonl (INADDR_ANY);
  if (bind (sock, (struct sockaddr *) &name, sizeof (name)) < 0){
      perror ("bind");
      exit (EXIT_FAILURE);
  }
  return sock;
}

int handle_request(int filedes){
  char buffer[MAXMSG];
  int nbytes;
  nbytes = read (filedes, buffer, MAXMSG);
  if (nbytes < 0){
      /* Read error. */
      perror ("read");
      exit (EXIT_FAILURE);
  }
  else if (nbytes == 0){
    /* End-of-file. */
    return -1;
  |
  else {
      /* Data read. */
      int len = strlen(buffer);
      buffer[len-1] = 0;
//      printf("\n\nServer: got message of len %d:\n '%s'\n\n\n", strlen(buffer), buffer);
      serve_request(filedes);
      printf("RQUEST SERVED MUST NOT WAIT ANYMORE\n");
      return 0;
  }
}
