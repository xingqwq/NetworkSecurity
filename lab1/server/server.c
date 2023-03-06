#include <stdio.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>

#define BUFFER_SIZE 1024

struct client
{
    int clientFd;
    struct sockaddr_in *clientAddr;
};

struct responseFrame
{
    int size;
    char fileName[1020];
};

union responseFrame2Char
{
    struct responseFrame dataFrame;
    char data[1024];
};

int getSize(FILE *file)
{
    fseek(file, 0, SEEK_END);
    int file_size = ftell(file);
    rewind(file);
    return file_size;
}

void getFileList(char *fileList)
{
    DIR *dirPtr;
    struct dirent *ptr;
    dirPtr = opendir("./");
    int totalLen = 0;
    while ((ptr = readdir(dirPtr)) != NULL)
    {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)
        {
            continue;
        }
        if (totalLen + strlen(ptr->d_name) > BUFFER_SIZE)
        {
            break;
        }
        else
        {
            strcat(fileList, ptr->d_name);
            strcat(fileList, "\n");
            totalLen += strlen(ptr->d_name);
        }
    }
    closedir(dirPtr);
}

void printErrorInfo(char *info)
{
    for (int i = 0; i < 80; i++)
    {
        printf("*");
    }
    printf("\n");
    printf("错误信息：%s\n", info);
    perror("Detail");
    for (int i = 0; i < 80; i++)
    {
        printf("*");
    }
    printf("\n");
}

void recvFileFromClient(int socketFd)
{
    char sendBuffer[BUFFER_SIZE], recvBuffer[BUFFER_SIZE];
    char fileName[BUFFER_SIZE];
    int sendLen, recvLen, readLen;

    // 发送回复
    memset(&sendBuffer, 0, BUFFER_SIZE);
    strcpy(sendBuffer, "Recv.");
    sendLen = write(socketFd, sendBuffer, BUFFER_SIZE);

    // 接收文件名称
    memset(&recvBuffer, 0, BUFFER_SIZE);
    recvLen = read(socketFd, recvBuffer, BUFFER_SIZE);
    union responseFrame2Char frame;
    memset(&frame.data, 0, BUFFER_SIZE);
    memcpy(frame.data, recvBuffer, BUFFER_SIZE);
    strcpy(fileName, frame.dataFrame.fileName);
    printf("待接收文件的总字节数为:%d\n",frame.dataFrame.size);

    memset(&sendBuffer, 0, BUFFER_SIZE);
    strcpy(sendBuffer, "Recv.");
    sendLen = write(socketFd, sendBuffer, BUFFER_SIZE);
    usleep(1e6);

    // 接收文件
    char *tmpPrefix = "tmp_";
    char *name = (char *)malloc(strlen(tmpPrefix) + strlen(fileName));
    strcat(name, tmpPrefix);
    strcat(name, fileName);
    printf("%s\n", name);
    FILE *file = fopen(name, "wb");
    int totalRecvLen = 0;
    while (1)
    {
        memset(&recvBuffer, 0, BUFFER_SIZE);
        recvLen = read(socketFd, recvBuffer, BUFFER_SIZE);
        fwrite(recvBuffer, sizeof(char), recvLen, file);
        totalRecvLen += recvLen;
        if(totalRecvLen >= frame.dataFrame.size){
            break;
        }
    }
    fclose(file);
    printf("已完成文件下载\n");
}

void sendFileToClient(int socketFd)
{
    char sendBuffer[BUFFER_SIZE], recvBuffer[BUFFER_SIZE];
    char fileName[BUFFER_SIZE];
    int sendLen, recvLen, readLen;

    // 发送文件列表
    char fileList[BUFFER_SIZE];
    memset(&fileList,0,BUFFER_SIZE);
    getFileList(fileList);
    memset(&sendBuffer, 0, BUFFER_SIZE);
    memcpy(sendBuffer, fileList, BUFFER_SIZE);
    sendLen = write(socketFd, sendBuffer, BUFFER_SIZE);

    // 接收待发送的文件名
    memset(&recvBuffer, 0, BUFFER_SIZE);
    recvLen = read(socketFd, recvBuffer, BUFFER_SIZE);
    memset(fileName,0,BUFFER_SIZE);
    memcpy(fileName, recvBuffer, BUFFER_SIZE);

    // 读取文件并发送
    FILE *file = fopen(fileName, "rb");
    if (file == NULL)
    {
        printErrorInfo("读取文件失败");
        exit(-1);
    }
    
    // 构建响应帧
    union responseFrame2Char frame;
    memset(&frame.data,0,BUFFER_SIZE);
    memcpy(frame.dataFrame.fileName,fileName,strlen(fileName));
    frame.dataFrame.size = getSize(file);
    printf("待发送文件的总字节数为:%d\n",frame.dataFrame.size);

    memset(&sendBuffer, 0, BUFFER_SIZE);
    memset(&recvBuffer, 0, BUFFER_SIZE);
    memcpy(sendBuffer, frame.data, BUFFER_SIZE);
    sendLen = write(socketFd, sendBuffer, BUFFER_SIZE);
    recvLen = read(socketFd, recvBuffer, BUFFER_SIZE);
    if (strcmp(recvBuffer, "Recv.") != 0)
    {
        printErrorInfo("传送文件时候出现错误");
        exit(-1);
    }

    int totalSendLen = 0;
    while (!feof(file))
    {
        memset(&sendBuffer, 0, BUFFER_SIZE);
        readLen = fread(sendBuffer, sizeof(char), BUFFER_SIZE, file);
        sendLen = write(socketFd, sendBuffer, readLen);
        totalSendLen += sendLen;
        printf("已发送字节数：%d\n", totalSendLen);
    }

    // 报告发送完毕
    usleep(1e6);
    fclose(file);
    printf("已发送完毕并已关闭文件句柄.\n");
}

void *process(void *arg)
{
    // 获取参数
    struct client *tmp_client = (struct client *)arg;
    struct sockaddr_in *clientAddr = tmp_client->clientAddr;
    int socketFd = tmp_client->clientFd;
    char recvBuffer[BUFFER_SIZE];
    int recvLen;

    // 获得客户端需求
    memset(&recvBuffer, 0, BUFFER_SIZE);
    recvLen = read(socketFd, recvBuffer, BUFFER_SIZE);
    int funcID = atoi(recvBuffer);
    if (funcID == 1)
    {
        recvFileFromClient(socketFd);
    }
    else if (funcID == 2)
    {
        sendFileToClient(socketFd);
    }
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printErrorInfo("请正确输入参数，包括IP地址和端口号");
        exit(-1);
    }
    char *addr = argv[1];
    int port = atoi(argv[2]);

    // 创建套接字
    int socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFd == -1)
    {
        printErrorInfo("创建套接字失败，请重试");
        exit(-1);
    }
    int opt = 1;
    setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));

    // 创建服务器
    struct sockaddr_in serverInfo;
    memset(&serverInfo, 0, sizeof(serverInfo));
    serverInfo.sin_family = AF_INET;
    serverInfo.sin_addr.s_addr = inet_addr(addr);
    serverInfo.sin_port = htons(port);
    int status = bind(socketFd, (struct sockaddr *)&serverInfo, sizeof(serverInfo));
    if (status == -1)
    {
        printErrorInfo("启动服务器失败，请检查IP地址或端口号是否正确");
        exit(-1);
    }
    status = listen(socketFd, 8);
    if (status == -1)
    {
        printErrorInfo("监听失败");
        exit(-1);
    }

    // 启动信息
    for (int i = 0; i < 80; i++)
    {
        printf("*");
    }
    printf("\n");
    printf("服务器已启动\n");
    for (int i = 0; i < 80; i++)
    {
        printf("*");
    }
    printf("\n");

    // 监听客户端
    while (1)
    {
        struct sockaddr_in clientAddr;
        int len = sizeof(clientAddr);
        int clientFd = accept(socketFd, (struct sockaddr *)&clientAddr, &len);
        if (clientFd == -1)
        {
            printErrorInfo("与客户端连接失败");
            continue;
        }
        printf("收到一个新的客户端连接\n");
        struct client tmp_client;
        tmp_client.clientFd = clientFd;
        tmp_client.clientAddr = &clientAddr;
        pthread_t id;
        // 创建子线程
        int ret = pthread_create(&id, NULL, (void *)process, (void *)&tmp_client);
        if (ret != 0)
        {
            printErrorInfo("与客户端连接失败");
        }
    }

    close(socketFd);
    return 0;
}