#include "http_server.h"

#ifdef ROUND_ROBIN
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
#endif

#ifdef LEAST_CONN
char* least_conn(int* served_by_idx){
   pthread_mutex_lock(&lb_state_mutex);
   int i, least_conn_index=0;
   int least_conn_value=servers_container->now_serving[0];
   for (i=1; i<=3; i++){
      if (servers_container->now_serving[i] < least_conn_value){
         least_conn_index = i;
         least_conn_value = servers_container->now_serving[i];
      }
   }
   printf("NOW SERVING DISTRIBUTION: %d %d %d %d\n", servers_container->now_serving[0], servers_container->now_serving[1], servers_container->now_serving[2], servers_container->now_serving[3]);
   servers_container->now_serving[least_conn_index]++;
   *served_by_idx=least_conn_index;
   pthread_mutex_unlock(&lb_state_mutex);
   return servers_container->servers[least_conn_index].ipaddress;
}

int init_server_container(AppServerContainer** container_ptr){
   *container_ptr = (AppServerContainer*)malloc(sizeof(AppServerContainer));
   (*container_ptr)->now_serving[0]=0;
   (*container_ptr)->now_serving[1]=0;
   (*container_ptr)->now_serving[2]=0;
   (*container_ptr)->now_serving[3]=0;
   init_server_struct(&((*container_ptr)->servers[0]), S2ELAB_IP);
   init_server_struct(&((*container_ptr)->servers[1]), S2ELABSTUDENT_IP);
   init_server_struct(&((*container_ptr)->servers[2]), S2ELABTEACHER_IP);
   init_server_struct(&((*container_ptr)->servers[3]), S2ELABTUTOR_IP);
   return 0;
}
#endif

int destroy_server_container(AppServerContainer** servers_ptr){
   free(*servers_ptr);
   return 0;
}

int init_server_struct(AppServer* server, char* ip){
   strcpy(server->ipaddress, ip);
   return 0;
}

char* choose_and_fetch_ip(int* served_by_idx){
   #ifdef ROUND_ROBIN
   return round_robin();
   #endif
   #ifdef LEAST_CONN
   return least_conn(served_by_idx);
   #endif
}

char* pretty_print_method(char* lb_method_identifier){
   if (strcmp(lb_method_identifier, ROUND_ROBIN_ID)==0){
      return "round robin";
   }
   else if (strcmp(lb_method_identifier, LEAST_CONN_ID)==0){
      return "least_connections";
   }
}
