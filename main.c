#include "http_server.h"

int main(int argc, char* argv[]){

   char selected_algorithm[15];
   init_server_container(&servers_container);

   printf("first_ip: %s\n", servers_container->servers[0].ipaddress);
   if (argc == 1){  //no method given
      strcpy(selected_algorithm, "round_robin");
   }
   else {
      strcpy(selected_algorithm, argv[1]);
   }

   operate_server(selected_algorithm);

   destroy_server_container(&servers_container);
}

