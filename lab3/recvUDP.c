#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define BUFFER_SIZE 1024

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("*********************************************\n");
        printf("请输入正确的IP地址和端口号\n");
        printf("*********************************************\n");
    }

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2]));
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    int ret = bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));

    struct sockaddr_in client;
    socklen_t len = sizeof(client);

    while (1)
    {
        // UDP接收数据
        char buffer[BUFFER_SIZE], strData[BUFFER_SIZE];
        memset(buffer, 0, BUFFER_SIZE);
        memset(strData,0, BUFFER_SIZE);
        recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&client, &len);
        sprintf(strData,"Data From B.");
        printf("buff = %s\n", buffer);
        if(strcmp(buffer, strData) == 0){
            printf("正确接收了B发来的信息\n");
        }else{
            printf("未正确接收了B发来的信息\n");
        }
    }

    close(sockfd);
    return 0;
}