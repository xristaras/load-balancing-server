#include "http_server.h"

char* round_robin(){
   //lock last_served_index
   return servers_container->servers[(servers_container->last_served_index++)%4].ipaddress;
}

int init_server_container(AppServerContainer* container_ptr){
   container_ptr = (AppServerContainer*)malloc(sizeof(AppServerContainer));
   container_ptr->last_served_index = 3;
   init_server_struct(&(container_ptr->servers[0]), "S2ELAB_IP");
   init_server_struct(&(container_ptr->servers[1]), "S2ELABSTUDENT_IP");
   init_server_struct(&(container_ptr->servers[2]), "S2ELABTEACHER_IP");
   init_server_struct(&(container_ptr->servers[3]), "S2ELABTUTOR_IP");
   return 0;
}

int destroy_server_container(AppServerContainer* servers_ptr){
   free(servers_ptr);
   return 0;
}

int init_server_struct(AppServer* server, char* ip){
   strcpy(server->ipaddress, ip);
   return 0;
}

char* choose_and_fetch_ip(){
   return "83.212.116.210";
}
