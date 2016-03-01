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
///   printf("NOW SERVING DISTRIBUTION: %d %d %d %d\n", servers_container->now_serving[0], servers_container->now_serving[1], servers_container->now_serving[2], servers_container->now_serving[3]);
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
   int r = rand() % servers_container->weight[3];
   if (r < servers_container->weight[0]){
      return servers_container->servers[0].ipaddress;
   }
   else if (r < servers_container->weight[1]){
      return servers_container->servers[1].ipaddress;
   }
   else if (r < servers_container->weight[2]){
      return servers_container->servers[2].ipaddress;
   }
   if (r < servers_container->weight[3]){
      return servers_container->servers[3].ipaddress;
   }
}

int init_server_container(AppServerContainer** container_ptr){
   *container_ptr = (AppServerContainer*)malloc(sizeof(AppServerContainer));
   (*container_ptr)->weight[0]=0;
   (*container_ptr)->weight[1]=0;
   (*container_ptr)->weight[2]=0;
   (*container_ptr)->weight[3]=0;
   (*container_ptr)->time[0]=100;
   (*container_ptr)->time[1]=100;
   (*container_ptr)->time[2]=100;
   (*container_ptr)->time[3]=100;
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

int perform_dummy_request(char* ipaddr){
   CURL *curl;
   CURLcode res;
   char response_str[5000];
   curl = curl_easy_init();
   clock_t t;

   printf("Performing dummy request on %s\n", ipaddr);
   if (curl) {
      curl_easy_setopt(curl, CURLOPT_URL, ipaddr);
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ignore_response);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)response_str);
      t = clock();
      res = curl_easy_perform(curl);
      t = clock() - t;
      if (res != CURLE_OK) {
         fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
      }
      curl_easy_cleanup(curl);
      return t;
   }
}

void weight_calculator(){
   int i;
   float temp_weights[4], total_weight;
   clock_t tmp_t, t[4], total_t;
   while(1){
      total_t = 0;
      total_weight = 0.0;
      for(i=0; i<4; i++){
         char ping_cmd[64];
         sprintf(ping_cmd, "ping -c 3 %s | tail -1 | awk -F '/' '{print $5}'", servers_container->servers[i].ipaddress);
//         sprintf(ping_cmd, "curl --silent -o /dev/null %s -w %%{time_total}\\n", servers_container->servers[i].ipaddress);      
         FILE *ping = popen(ping_cmd, "r");
         char res[8];
         fgets(res, sizeof(res), ping);
//         t[i] = (1000 * atof(res) + servers_container->time[i]) / 2;
         t[i] = 1000 * atof(res);
         if (t[i]<100){
            t[i] = 100;
         }
         else if (t[i]>10000){
            t[i] = 10000;
         }
         servers_container->time[i] = t[i];
         pclose(ping);
         printf("time passed: %d\n", (int)t[i]);
         total_t += t[i];
      }
//      printf("total time: %d\n", (int)total_t);
      servers_container->weight[0] = round(100*((float)((float)total_t/(float)t[0])));
      for(i=1; i<4; i++){
         servers_container->weight[i] = round(100*((float)((float)total_t/(float)t[i]))) + servers_container->weight[i-1];
//         total_weight += temp_weights[i];
      }
//      printf("Servers tmp_weights: %f %f %f %f, total weight is %f\n", temp_weights[0], temp_weights[1], temp_weights[2], temp_weights[3], total_weight);
      printf("Servers weights: %d %d %d %d\n", servers_container->weight[0], servers_container->weight[1], servers_container->weight[2], servers_container->weight[3]);
//      pthread_mutex_unlock(&lb_state_mutex);
      sleep(SEC_INTERVAL);
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
