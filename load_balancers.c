#include "http_server.h"

char* round_robin(){
   pthread_mutex_lock(&lb_state_mutex);
   servers_container->last_served_index++;
   if (servers_container->last_served_index == 4) {
      servers_container->last_served_index=0;
   }
   pthread_mutex_unlock(&lb_state_mutex);
   return servers_container->servers[servers_container->last_served_index].ipaddress;
}

int init_server_container(AppServerContainer** container_ptr){
   *container_ptr = (AppServerContainer*)malloc(sizeof(AppServerContainer));
   (*container_ptr)->last_served_index = 3;
   init_server_struct(&((*container_ptr)->servers[0]), S2ELAB_IP);
   init_server_struct(&((*container_ptr)->servers[1]), S2ELABSTUDENT_IP);
   init_server_struct(&((*container_ptr)->servers[2]), S2ELABTEACHER_IP);
   init_server_struct(&((*container_ptr)->servers[3]), S2ELABTUTOR_IP);
   return 0;
}

int destroy_server_container(AppServerContainer** servers_ptr){
   free(*servers_ptr);
   return 0;
}

int init_server_struct(AppServer* server, char* ip){
   strcpy(server->ipaddress, ip);
   return 0;
}

char* choose_and_fetch_ip(char* lb_method){
   if (strcmp(lb_method, ROUND_ROBIN_ID)==0){
      return round_robin();
   }
}

char* pretty_print_method(char* lb_method_identifier){
   if (strcmp(lb_method_identifier, ROUND_ROBIN_ID)==0){
      return "round robin";
   }
}
