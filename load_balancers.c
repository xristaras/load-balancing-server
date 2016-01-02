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
   int least_loaded_value=servers_container->normalized_load[0];
   for (i=1; i<=3; i++){
      if (servers_container->normalized_load[i] < least_loaded_value){
         least_loaded_index = i;
         least_loaded_value = servers_container->normalized_load[i];
      }
   }
//   printf("NOW SERVING DISTRIBUTION: %d %d %d %d\n", servers_container->now_serving[0], servers_container->now_serving[1], servers_container->now_serving[2], servers_container->now_serving[3]);
   servers_container->now_serving[least_loaded_index]++;
   *served_by_idx=least_loaded_index;
   servers_container->normalized_load[least_loaded_index]=(servers_container->delay[least_loaded_index]*servers_container->now_serving[least_loaded_index]);
   pthread_mutex_unlock(&lb_state_mutex);
   return servers_container->servers[least_loaded_index].ipaddress;
}

int init_server_container(AppServerContainer** container_ptr){
   *container_ptr = (AppServerContainer*)malloc(sizeof(AppServerContainer));
   (*container_ptr)->delay[0]=0;
   (*container_ptr)->delay[1]=0;
   (*container_ptr)->delay[2]=0;
   (*container_ptr)->delay[3]=0;
   (*container_ptr)->now_serving[0]=0;
   (*container_ptr)->now_serving[1]=0;
   (*container_ptr)->now_serving[2]=0;
   (*container_ptr)->now_serving[3]=0;
   (*container_ptr)->normalized_load[0]=0;
   (*container_ptr)->normalized_load[1]=0;
   (*container_ptr)->normalized_load[2]=0;
   (*container_ptr)->normalized_load[3]=0;
   init_server_struct(&((*container_ptr)->servers[0]), S2ELAB_IP);
   init_server_struct(&((*container_ptr)->servers[1]), S2ELABSTUDENT_IP);
   init_server_struct(&((*container_ptr)->servers[2]), S2ELABTEACHER_IP);
   init_server_struct(&((*container_ptr)->servers[3]), S2ELABTUTOR_IP);
   return 0;
}

size_t ignore_response(char* ptr, size_t size, size_t nmemb, void* userdata){
   // override default libcurl behavior and do not print output to stdout
   return size*nmemb;
}

void perform_dummy_request(char* ipaddr){
   CURL *curl;
   CURLcode res;
//   int served_by_idx;
   char response_str[5000];
   curl = curl_easy_init();

   printf("Performing dummy request on %s\n", ipaddr);
   if (curl) {
      curl_easy_setopt(curl, CURLOPT_URL, ipaddr);
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ignore_response);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)response_str);
      res = curl_easy_perform(curl);
      if (res != CURLE_OK) {
         fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
      }
      curl_easy_cleanup(curl);
   }
}

void weight_calculator(){
   int i;
   clock_t t;
   while(1){
      sleep(SEC_INTERVAL);
      pthread_mutex_lock(&lb_state_mutex);
      for(i=0; i<4; i++){
         t = clock();
	 perform_dummy_request(servers_container->servers[i].ipaddress);
         t = clock() - t;
         printf("time passed: %d\n", (int)t);
         servers_container->delay[i]=t;
         servers_container->normalized_load[i]=(servers_container->now_serving[i])*(int)t;
         //update server weight and normalized weight here
      }
      pthread_mutex_unlock(&lb_state_mutex);
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
