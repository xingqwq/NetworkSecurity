#include <stdio.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024

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

void printWelcomInfo()
{
    for (int i = 0; i < 80; i++)
    {
        printf("*");
    }
    printf("\n");
    printf("已正确连接至服务器，请选择你要进行的功能\n");
    printf("功能序号 1\t 向服务器发送文件\n");
    printf("功能序号 2\t 从服务器接收文件\n");
    for (int i = 0; i < 80; i++)
    {
        printf("*");
    }
    printf("\n");
}

void sendFileToServer(int socketFd)
{
    char sendBuffer[BUFFER_SIZE], recvBuffer[BUFFER_SIZE];
    int sendLen, recvLen, readLen;

    // 发送功能序号
    memset(&sendBuffer, 0, BUFFER_SIZE);
    memset(&recvBuffer, 0, BUFFER_SIZE);
    strcpy(sendBuffer, "1");
    sendLen = write(socketFd, sendBuffer, BUFFER_SIZE);

    recvLen = read(socketFd, recvBuffer, BUFFER_SIZE);
    if (strcmp(recvBuffer, "Recv.") != 0)
    {
        printErrorInfo("传送文件时候出现错误");
        exit(-1);
    }

    // 选择需要发送的文件
    char fileName[BUFFER_SIZE];
    memset(&fileName, 0, BUFFER_SIZE);
    printf("请输入需要发送的文件名:");
    scanf("%s", fileName);

    // 读取文件并发送
    FILE *file = fopen(fileName, "rb");
    if (file == NULL)
    {
        printErrorInfo("读取文件失败");
        exit(-1);
    }

    // 构建响应帧
    union responseFrame2Char frame;
    memset(&frame.data, 0, BUFFER_SIZE);
    memcpy(frame.dataFrame.fileName, fileName, strlen(fileName));
    frame.dataFrame.size = getSize(file);
    printf("待发送文件的总字节数为:%d\n", frame.dataFrame.size);

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

void recvFileFromServer(int socketFd)
{
    char recvFileName[1024];
    char sendBuffer[BUFFER_SIZE], recvBuffer[BUFFER_SIZE];
    int sendLen, recvLen, readLen;

    // 发送功能序号
    memset(&sendBuffer, 0, BUFFER_SIZE);
    memset(&recvBuffer, 0, BUFFER_SIZE);
    strcpy(sendBuffer, "2");
    sendLen = write(socketFd, sendBuffer, BUFFER_SIZE);
    recvLen = read(socketFd, recvBuffer, BUFFER_SIZE);
    printf("这是服务器上的文件列表\n");
    printf("%s\n", recvBuffer);

    memset(&recvFileName, 0, BUFFER_SIZE);
    printf("请选择你要下载的文件名：");
    scanf("%s", recvFileName);

    // 发送接收的文件名
    memset(&sendBuffer, 0, BUFFER_SIZE);
    strncpy(sendBuffer, recvFileName, strlen(recvFileName));
    sendLen = write(socketFd, sendBuffer, BUFFER_SIZE);

    // 接收文件名称
    memset(&recvBuffer, 0, BUFFER_SIZE);
    recvLen = read(socketFd, recvBuffer, BUFFER_SIZE);
    union responseFrame2Char frame;
    memset(&frame.data, 0, BUFFER_SIZE);
    memcpy(frame.data, recvBuffer, BUFFER_SIZE);
    strcpy(recvFileName, frame.dataFrame.fileName);
    printf("待接收文件的总字节数为:%d\n", frame.dataFrame.size);

    memset(&sendBuffer, 0, BUFFER_SIZE);
    strcpy(sendBuffer, "Recv.");
    sendLen = write(socketFd, sendBuffer, BUFFER_SIZE);
    usleep(1e6);

    // 接收文件
    char *tmpPrefix = "tmp_";
    char *name = (char *)malloc(strlen(tmpPrefix) + strlen(recvFileName));
    strcat(name, tmpPrefix);
    strcat(name, recvFileName);
    printf("%s\n", name);
    FILE *file = fopen(name, "wb");
    int totalRecvLen = 0;
    while (1)
    {
        memset(&recvBuffer, 0, BUFFER_SIZE);
        recvLen = read(socketFd, recvBuffer, BUFFER_SIZE);
        fwrite(recvBuffer, sizeof(char), recvLen, file);
        totalRecvLen += recvLen;
        if (totalRecvLen >= frame.dataFrame.size)
        {
            break;
        }
    }
    fclose(file);
    printf("已完成文件下载\n");
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printErrorInfo("请正确输入服务器参数，包括IP地址和端口号");
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

    // 连接服务器
    struct sockaddr_in serverInfo;
    memset(&serverInfo, 0, sizeof(serverInfo));
    serverInfo.sin_family = AF_INET;
    serverInfo.sin_addr.s_addr = inet_addr(addr);
    serverInfo.sin_port = htons(port);
    int status = connect(socketFd, (struct sockaddr *)&serverInfo, sizeof(serverInfo));
    if (status == -1)
    {
        printErrorInfo("连接服务器失败，请检查IP地址或端口号是否正确");
        exit(-1);
    }
    printWelcomInfo();

    // 选择功能
    int funcID;

    printf("请键入你需要的功能：");
    scanf("%d", &funcID);

    if (funcID == 1)
    {
        sendFileToServer(socketFd);
    }
    else if (funcID == 2)
    {
        recvFileFromServer(socketFd);
    }

    usleep(5000000);
    close(socketFd);

    return 0;
}