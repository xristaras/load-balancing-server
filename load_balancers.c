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

#ifdef LEAST_LATENCY
char* least_latency(int* served_by_idx){
   pthread_mutex_lock(&lb_state_mutex);
   int i, least_loaded_index=0;
   int least_loaded_value=servers_container->normalized_weight[0];
   for (i=1; i<=3; i++){
      if (servers_container->normalized_weight[i] < least_loaded_value){
         least_loaded_index = i;
         least_loaded_value = servers_container->normalized_weight[i];
      }
   }
//   printf("NOW SERVING DISTRIBUTION: %d %d %d %d\n", servers_container->now_serving[0], servers_container->now_serving[1], servers_container->now_serving[2], servers_container->now_serving[3]);
   servers_container->now_serving[least_loaded_index]++;
   *served_by_idx=least_loaded_index;
   servers_container->normalized_weight[least_loaded_index]=(float)(servers_container->weight[least_loaded_index]/servers_container->now_serving[least_loaded_index]);
   pthread_mutex_unlock(&lb_state_mutex);
   return servers_container->servers[least_loaded_index].ipaddress;
}

int init_server_container(AppServerContainer** container_ptr){
   *container_ptr = (AppServerContainer*)malloc(sizeof(AppServerContainer));
   (*container_ptr)->weight[0]=0;
   (*container_ptr)->weight[1]=0;
   (*container_ptr)->weight[2]=0;
   (*container_ptr)->weight[3]=0;
   (*container_ptr)->now_serving[0]=0;
   (*container_ptr)->now_serving[1]=0;
   (*container_ptr)->now_serving[2]=0;
   (*container_ptr)->now_serving[3]=0;
   (*container_ptr)->normalized_weight[0]=0;
   (*container_ptr)->normalized_weight[1]=0;
   (*container_ptr)->normalized_weight[2]=0;
   (*container_ptr)->normalized_weight[3]=0;
   init_server_struct(&((*container_ptr)->servers[0]), S2ELAB_IP);
   init_server_struct(&((*container_ptr)->servers[1]), S2ELABSTUDENT_IP);
   init_server_struct(&((*container_ptr)->servers[2]), S2ELABTEACHER_IP);
   init_server_struct(&((*container_ptr)->servers[3]), S2ELABTUTOR_IP);
   return 0;
}

void perform_dummy_request(char* ipaddr){
   CURL *curl;
   CURLcode res;
//   int served_by_idx;
//   char response_str[5000];
   curl = curl_easy_init();

   printf("Performing dummy request on %s\n", ipaddr);

   if (curl) {
      curl_easy_setopt(curl, CURLOPT_URL, ipaddr);
//      curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, append_headers);
//      curl_easy_setopt(curl, CURLOPT_HEADERDATA, (void*)response_str);
//      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, append_html);
//      curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)response_str);
//      curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0 );  //because of a (encoding?) bug in latest version
      res = curl_easy_perform(curl);
      if (res != CURLE_OK) {
         fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
      }
//      else {
//         strcat(response_str, "\0");
//         write(client_socket, response_str, strlen(response_str));
//      }
      curl_easy_cleanup(curl);
   }

//   strcpy(response_str, "\0");
//   return served_by_idx;

}

void weight_calculator(){
   int i;
   clock_t t;
   while(1){
      sleep(SEC_INTERVAL);
      for(i=0; i<4; i++){
         t = clock();
	 perform_dummy_request(servers_container->servers[i].ipaddress);
         t = clock() - t;
         printf("time passed: %d\n", t);
         //update server weight and normalized weight here
      }
   }
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
   #ifdef LEAST_LATENCY
   return least_latency(served_by_idx);
   #endif
}

char* pretty_print_method(char* lb_method_identifier){
   if (strcmp(lb_method_identifier, ROUND_ROBIN_ID)==0){
      return "round robin";
   }
   else if (strcmp(lb_method_identifier, LEAST_CONN_ID)==0){
      return "least_connections";
   }
   else if (strcmp(lb_method_identifier, LEAST_LATENCY_ID)==0){
      return "least latency";
   }

}
