#include <stdio.h>
#include <string.h>
#include <libnet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int main(int argc, char** argv){
    if (argc < 5)
    {
        printf("*********************************************\n");
        printf("请输入正确的源、目的IP地址和端口号\n");
        printf("*********************************************\n");
    }
    // 初始化
    libnet_t* libNetHandle = libnet_init(LIBNET_LINK_ADV, "wlp3s0", NULL);
    libnet_ptag_t pack = 0;

    unsigned char srcMac[6] = {0x70, 0x66, 0x55, 0x09, 0x4e, 0x8d}; 
    unsigned char dstMac[6] = {0x70, 0x66, 0x55, 0x09, 0x4e, 0x8d};

    char *srcIpStr = argv[1];
    int srcPort = atoi(argv[2]);                             
    char *dstIpStr = argv[3];
    int dstPort = atoi(argv[4]);
    printf("源IP：%s 目的IP：%s\n",srcIpStr, dstIpStr);
    printf("源端口：%d 目的端口:%d\n", srcPort, dstPort);            
    unsigned long srcIp = libnet_name2addr4(libNetHandle, srcIpStr, LIBNET_RESOLVE);
    unsigned long dstIp = libnet_name2addr4(libNetHandle, dstIpStr, LIBNET_RESOLVE);       

    // 装载发送数据
    char sendData[BUFFER_SIZE];
    memset(&sendData, 0, BUFFER_SIZE);
    int dataLen = sprintf(sendData,"Data From B.");

    // 构建UDP包
    pack = libnet_build_udp(
        srcPort,
        dstPort,
        8 + dataLen,
        0,
        sendData,
        dataLen,
        libNetHandle,
        0
    );

    // 构建IP数据报
    pack = libnet_build_ipv4(
        20 + 8 + dataLen,   // 包长度
        0,                  // 服务类型
        500,                // ip标识
        0,                  // 片偏移
        10,                 // 生存时间
        17,                 // UDP协议号
        0,                  // 校验和
        srcIp,          
        dstIp,
        NULL,               // 负载，上面生成了UDP包
        0,                  // 长度
        libNetHandle,   
        0
    );

    // 构建以太网帧
    pack = libnet_build_ethernet(
        (uint8_t *)dstMac,
        (uint8_t *)srcMac,
        ETHERTYPE_IP,
        NULL,
        0,
        libNetHandle,
        0
    );

    // 发送
    int res = libnet_write(libNetHandle);
    if(res == -1){
        printf("*********************************************\n");
        printf("发送失败\n");
        printf("*********************************************\n");
        exit(-1);
    }
    libnet_destroy(libNetHandle);
    printf("*********************************************\n");
    printf("已完成发送\n");
    printf("*********************************************\n");
}