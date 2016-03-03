#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <poll.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#define SERVER_IP_ADDR localhost
#define PORT_NUM       5000

int main(int argc, char *argv[])
{
int sockfd = 0;
char sendBuff[12];
struct sockaddr_in serv_addr;
int i;

if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
{
    printf("\n Error : Could not create socket \n");
    return 1;
}

memset(&serv_addr, '0', sizeof(serv_addr));

serv_addr.sin_family = AF_INET;
serv_addr.sin_port = htons(PORT_NUM);

if(inet_pton(AF_INET, SERVER_IP_ADDR, &serv_addr.sin_addr)<=0)
{
    printf("\n inet_pton error occured\n");
    return 1;
}

if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
{
   printf("\n Error : Connect Failed \n");
   return 1;
}

while(1){
    char curl_cmd[64];
    sprintf(curl_cmd, "curl --silent -o /dev/null %s -w %%{time_total}\\n", servers_container->servers[i].ipaddress);
    FILE *curl = popen(curl_cmd, "r");
    char res[8];
    fgets(res, sizeof(res), curl);
//         t[i] = (1000 * atof(res) + servers_container->time[i]) / 2;
//         t[i] = 1000 * atof(res);
//         if (t[i]<100){
//            t[i] = 100;
//         }
//         else if (t[i]>10000){
//            t[i] = 10000;
//         }
    pclose(ping);
    memset(sendBuff, '\0', sizeof(sendBuff));
    sprintf(sendBuff, "%s", res);
    send(sockfd, sendBuff, strlen(sendBuff), 0);
    sleep(10);
}

return 0;
}
