#include "http_server.h"

int main(int argc, char* argv[]){

   char lb_method[4];
   init_server_container(&servers_container);

   if (argc == 1){  //no method given
      strcpy(lb_method, ROUND_ROBIN_ID);
   }
   else {
      strcpy(lb_method, argv[1]);
   }

   pthread_mutex_init(&lb_state_mutex, NULL);
   pthread_mutex_init(&clients_counter_mutex, NULL);
   curl_global_init(CURL_GLOBAL_DEFAULT);

   operate_server(lb_method);

   curl_global_cleanup();
   destroy_server_container(&servers_container);
   pthread_mutex_destroy(&lb_state_mutex);
   pthread_mutex_destroy(&clients_counter_mutex);
}

