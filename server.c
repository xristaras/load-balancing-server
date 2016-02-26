#include "http_server.h"

size_t append_headers(char* ptr, size_t size, size_t nitems, void* userdata){
   if (strstr(ptr, "503") != NULL){
//      printf("503 found!\n");
   }
   strncat(userdata, ptr, size*nitems);
   return size*nitems;
}

size_t append_html(char* ptr, size_t size, size_t nmemb, void* userdata){
   strncat(userdata, ptr, nmemb);
   return size*nmemb;
}

int serve_request(int client_socket, char* lb_method){
   CURL *curl;
   CURLcode res;
   int served_by_idx;
   char response_str[5000];
   curl = curl_easy_init();

   char selected_ip[16];
   strcpy(selected_ip, choose_and_fetch_ip(&served_by_idx));
//   printf("Request being served by %s, currently serving %d clients\n", selected_ip, num_clients_connected);

   if (curl) {
      curl_easy_setopt(curl, CURLOPT_URL, selected_ip);
      curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, append_headers);
      curl_easy_setopt(curl, CURLOPT_HEADERDATA, (void*)response_str);
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, append_html);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)response_str);
      curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0 );  //because of a (encoding?) bug in latest version
#ifdef LEAST_LATENCY_ALT
      if(rand()%250==0){
         clock_t before, after, t;
	 time(&before);
         res = curl_easy_perform(curl);
	 time(&after);
         if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
         }
         else {
            pthread_mutex_lock(&lb_state_mutex);
	    printf("difftime result: %2.3f\n", difftime(after, before));
   	    weight_calculator(served_by_idx, 100*difftime(after, before));
            pthread_mutex_unlock(&lb_state_mutex);

            strcat(response_str, "\0");
            write(client_socket, response_str, strlen(response_str));
         }
      }
      else{
#endif
         res = curl_easy_perform(curl);
	 if (res != CURLE_OK) {
	    fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
	 }
         else{
            strcat(response_str, "\0");
            write(client_socket, response_str, strlen(response_str));
         }
#ifdef LEAST_LATENCY_ALT
      }
#endif
      curl_easy_cleanup(curl);
   }

   strcpy(response_str, "\0");
   return served_by_idx;

}

int initialize_socket(uint16_t port){
   int sock;
   struct sockaddr_in name;

   /* Create the socket. */
   sock = socket(PF_INET, SOCK_STREAM, 0);
   if (sock < 0) {
      perror("socket");
      exit(EXIT_FAILURE);
   }

   /* Give the socket a name. */
   name.sin_family = AF_INET;
   name.sin_port = htons(port);
   name.sin_addr.s_addr = htonl(INADDR_ANY);
   if (bind(sock, (struct sockaddr *) &name, sizeof (name)) < 0) {
      perror("bind");
      exit(EXIT_FAILURE);
   }
   return sock;
}

int handle_request(RequestHandlerArgs *args){
   char buffer[MAXMSG];
   int nbytes;
   increment_clients_counter();
   nbytes = read(args->filedes, buffer, MAXMSG);
   if (nbytes < 0) {
      /* Read error. */
      perror("read");
      exit(EXIT_FAILURE);
   }
   else if (nbytes == 0) {
      /* End-of-file. */
      return -1;
   }
   else {
      /* Data read. */
      int len = strlen(buffer);
      buffer[len-1] = 0;
      int served_by_idx=serve_request(args->filedes, args->lb_method);
      close(args->filedes);
      free(args);
      decrement_clients_counter();
      #ifdef LEAST_CONN
      pthread_mutex_lock(&lb_state_mutex);
      servers_container->now_serving[served_by_idx]--;
      pthread_mutex_unlock(&lb_state_mutex);
      #endif
      return 0;
   }
}

void operate_server(char* lb_method){
   extern int initialize_socket(uint16_t port);
   int i, sock, client_sock;
   struct sockaddr_in clientname;
   size_t size;

   printf("Server operation initiated, using %s algorithm\n", pretty_print_method(lb_method));
   srand(time(NULL));
   /* Create the socket and set it up to accept connections. */
   sock = initialize_socket(PORT);
   if (listen(sock, 1) < 0) {
       perror("listen");
       exit(EXIT_FAILURE);
   }

   size = sizeof(clientname);
   pthread_t thread_id;
   #ifdef LEAST_LATENCY
   if (pthread_create(&thread_id, NULL, (void *)weight_calculator, (void*)NULL) < 0){
      perror("calculator pthread_create");
      exit(EXIT_FAILURE);
   }
   pthread_detach(thread_id);
   #endif
   while ((client_sock = accept(sock, (struct sockaddr *)&clientname, (socklen_t* __restrict__)&size))){
//      printf("New connection request, socket %d will handle it\n", client_sock);
      RequestHandlerArgs *thread_args;
      thread_args=(RequestHandlerArgs*)malloc(sizeof(RequestHandlerArgs));
      thread_args->filedes=client_sock;
      strcpy(thread_args->lb_method, lb_method);
      if (pthread_create(&thread_id, NULL, (void *)handle_request, (void*)thread_args) < 0){
          perror("pthread_create");
          exit(EXIT_FAILURE);
      }
      pthread_detach(thread_id);
   }

}

void increment_clients_counter(){
   pthread_mutex_lock(&clients_counter_mutex);
   num_clients_connected++;
   pthread_mutex_unlock(&clients_counter_mutex);
}

void decrement_clients_counter(){
   pthread_mutex_lock(&clients_counter_mutex);
   num_clients_connected--;
   pthread_mutex_unlock(&clients_counter_mutex);
}
