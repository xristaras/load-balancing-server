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
#define MAXMSG  512

#include <curl/curl.h>

size_t append_html(char *ptr, size_t size, size_t nmemb, void *userdata){
//  userdata = malloc(16 + 19 + 25 + size*nmemb + 7 + 1);
  strcpy(userdata, "HTTP/1.1 200 OK\n");  //16
  strcat(userdata, "Content-length: 5000\n");   //19
  strcat(userdata, "Content-Type: text/html\n\n");  //25
  strncat(userdata, ptr, nmemb);
  strcat(userdata, "\0");
  return size*nmemb;
}

int serve_request(int client_socket)
{
  CURL *curl;
  CURLcode res;
  char response_str[4096];

  curl_global_init(CURL_GLOBAL_DEFAULT);

  curl = curl_easy_init();


  printf("init ok\n");
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, "http://83.212.116.210/");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, append_html);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)response_str);
    printf("ok before curl perform\n");
    res = curl_easy_perform(curl);
    printf("ok after curl perform\n");
    if(res != CURLE_OK){
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }
    else {
      printf("SIZEOF response_str:%d\n", response_str);
      write(client_socket, response_str, strlen(response_str));
//      write(client_socket, response_str, sizeof(response_str));
    }
    curl_easy_cleanup(curl);
  }

  curl_global_cleanup();

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
  if (listen (sock, 1) < 0)
    {
      perror ("listen");
      exit (EXIT_FAILURE);
    }

  /* Initialize the set of active sockets. */
  FD_ZERO (&active_fd_set);
  FD_SET (sock, &active_fd_set);

  while (1)
    {
      /* Block until input arrives on one or more active sockets. */
      read_fd_set = active_fd_set;
      if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0)
        {
          perror ("select");
          exit (EXIT_FAILURE);
        }

      /* Service all the sockets with input pending. */
      for (i = 0; i < FD_SETSIZE; ++i){
        if (FD_ISSET (i, &read_fd_set))
          {
            if (i == sock)
              {
                /* Connection request on original socket. */
                int new_conn;
                size = sizeof (clientname);
                new_conn = accept (sock, (struct sockaddr *)&clientname, (socklen_t * __restrict__)&size);
                printf("new connection request, just accepted\n");
                if (new_conn < 0)
                  {
                    printf("accept failure\n");
                    perror ("accept");
                    exit (EXIT_FAILURE);
                  }
                FD_SET (new_conn, &active_fd_set);
              }
            else {
                /* Data arriving on an already-connected socket. */
                printf("Data arriving on an already-connected socket. Trying to read from client\n");
                read_request(i);
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
  if (sock < 0)
    {
      perror ("socket");
      exit (EXIT_FAILURE);
    }

  /* Give the socket a name. */
  name.sin_family = AF_INET;
  name.sin_port = htons (port);
  name.sin_addr.s_addr = htonl (INADDR_ANY);
  if (bind (sock, (struct sockaddr *) &name, sizeof (name)) < 0)
    {
      perror ("bind");
      exit (EXIT_FAILURE);
    }
  return sock;
}

int read_request(int filedes){
  char buffer[MAXMSG];
  int nbytes;

  nbytes = read (filedes, buffer, MAXMSG);
  if (nbytes < 0)
    {
      /* Read error. */
      perror ("read");
      exit (EXIT_FAILURE);
    }
  else if (nbytes == 0)
    /* End-of-file. */
    return -1;
  else
    {
      /* Data read. */
      int len = strlen(buffer);
      buffer[len-1] = 0;
      fprintf (stderr, "Server: got message: `%s'\n", buffer);
      if(strncmp(buffer, "GET / HTTP/1.1", 14) == 0){
        serve_request(filedes);
//      write (filedes, "Thanks\n", 7);
      }
      return 0;
    }
}
