#include "http_server.h"

int main(int argc, char* argv[]){

   char selected_algorithm[16];
   init_server_container(&servers_container);

   printf("first_ip: %s\n", servers_container->servers[0].ipaddress);
   if (argc == 1){  //no method given
      strcpy(selected_algorithm, "round_robin");
   }
   else {
      strcpy(selected_algorithm, argv[1]);
   }

   pthread_mutex_init(&lb_state_mutex, NULL);
   curl_global_init(CURL_GLOBAL_DEFAULT);
   operate_server(selected_algorithm);

   
   curl_global_cleanup();
   destroy_server_container(&servers_container);
   pthread_mutex_destroy(&lb_state_mutex);
}

