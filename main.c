#include "http_server.h"

int main(int argc, char **argv){
   extern int initialize_socket(uint16_t port);
   int sock;
   fd_set active_fd_set, read_fd_set;
   int i;
   struct sockaddr_in clientname;
   size_t size;

   /* Create the socket and set it up to accept connections. */
   sock = initialize_socket(PORT);
   if (listen(sock, 1) < 0) {
       perror("listen");
       exit(EXIT_FAILURE);
   }

   /* Initialize the set of active sockets. */
   FD_ZERO(&active_fd_set);
   FD_SET(sock, &active_fd_set);

   while(1){
      /* Block until input arrives on one or more active sockets. */
      read_fd_set = active_fd_set;
      if (select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0) {
         perror("select");
         exit(EXIT_FAILURE);
      }

      /* Service all the sockets with input pending. */
      for (i = 0; i < FD_SETSIZE; ++i) {
         if (FD_ISSET (i, &read_fd_set)) {
            if (i == sock) {
               /* Connection request on original socket. */
               int new_conn;
               size = sizeof (clientname);
               new_conn = accept (sock, (struct sockaddr *)&clientname, (socklen_t * __restrict__)&size);
               printf("FD_SETSIZE is %d, new connection request, just accepted in %d\n", FD_SETSIZE, i);
               if (new_conn < 0) {
                  printf("accept failure\n");
                  perror("accept");
                  exit(EXIT_FAILURE);
               }
               FD_SET(new_conn, &active_fd_set);
            }
            else {
               /* Data arriving on an already-connected socket. */
               printf("WILL HANDLE %d\n", i);
               handle_request(i);
               close(i);
               FD_CLR(i, &active_fd_set);
            }
         }
      }
   }
}

