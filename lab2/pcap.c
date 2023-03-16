#include <pcap.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#define ETHERNET_ADDR_LEN 6
#define IP_ADDR_LEN 4

// 以太网数据帧头部结构体
struct ethernet
{
    u_char eth_dsthost[ETHERNET_ADDR_LEN]; // 以太网MAC目的地址
    u_char eth_srchost[ETHERNET_ADDR_LEN]; // 以太网MAC源地址
    u_short eth_type;                      // 协议类型
};

// IPv4报文头部结构体
struct ip
{
    u_char ip_hlv;              // 版本号+头部长度
    u_char ip_tos;              // 区分服务
    u_short ip_len;             // IP数据报长度
    u_short ip_id;              // 标识
    u_short ip_foff;            // 标志3位+片偏移13位
    u_char ip_ttl;              // 生存时间
    u_char ip_pro;              // 协议
    u_short ip_checksum;        // 首部校验和
    u_char ip_src[IP_ADDR_LEN]; // 源IP地址
    u_char ip_dst[IP_ADDR_LEN]; // 目的IP地址
};

// TCP报文头
struct tcp
{
    u_short tcp_srcport;  // 源端口号
    u_short tcp_dstport;  // 目的端口号
    u_int tcp_seq;        // 序列号
    u_int tcp_ack;        // 确认号
    u_char tcp_headlen;   // 4位头部长度+4位保留
    u_char tcp_flag;      // 2保留位+6位标志位
    u_short tcp_win;      // 窗口大小
    u_short tcp_checksum; // 校验和
    u_short tcp_urp;      // 紧急指针
};

// UDP报文头部结构体
struct udp
{
    u_short udp_srcport;  // 源端口号
    u_short udp_dstport;  // 目的端口号
    u_short udp_len;      // 总长度
    u_short udp_checksum; // 校验和
};

struct callbackMsg
{
    int *cnt;      
    FILE *file;
};

void parseUdp(struct udp *tmpUDP, const u_char *datagram)
{
    int udp_start = 14 + 4 * (datagram[14] & 0x0f);
    tmpUDP->udp_srcport = ntohs(*(u_short *)(datagram + udp_start));
    tmpUDP->udp_dstport = ntohs(*(u_short *)(datagram + udp_start + 2));
    tmpUDP->udp_len = ntohs(*(u_short *)(datagram + udp_start + 4));
    tmpUDP->udp_checksum = ntohs(*(u_short *)(datagram + udp_start + 6));
}

void parseTcp(struct tcp *tmpTCP, const u_char *datagram)
{
    int tcp_start = 14 + 4 * (datagram[14] & 0x0f);
    tmpTCP->tcp_srcport = ntohs(*(u_short *)(datagram + tcp_start));
    tmpTCP->tcp_dstport = ntohs(*(u_short *)(datagram + tcp_start + 2));
    tmpTCP->tcp_seq = ntohl(*(u_int *)(datagram + tcp_start + 4));
    tmpTCP->tcp_ack = ntohl(*(u_int *)(datagram + tcp_start + 8));
    tmpTCP->tcp_headlen = datagram[tcp_start + 12];
    tmpTCP->tcp_flag = datagram[tcp_start + 13];
    tmpTCP->tcp_win = ntohs(*(u_short *)(datagram + tcp_start + 14));
    tmpTCP->tcp_checksum = ntohs(*(u_short *)(datagram + tcp_start + 16));
    tmpTCP->tcp_urp = ntohs(*(u_short *)(datagram + tcp_start + 18));
}

void parseIp(struct ip *tmpIP, const u_char *datagram)
{
    tmpIP->ip_hlv = datagram[14];
    tmpIP->ip_tos = datagram[15];
    tmpIP->ip_len = ntohs(*(u_short *)(datagram + 16));
    tmpIP->ip_id = ntohs(*(u_short *)(datagram + 18));
    tmpIP->ip_foff = ntohs(*(u_short *)(datagram + 20));
    tmpIP->ip_ttl = datagram[22];
    tmpIP->ip_pro = datagram[23];
    tmpIP->ip_checksum = ntohs(*(u_short *)(datagram + 24));
    *(unsigned int *)tmpIP->ip_src = *(unsigned int *)(datagram + 26);
    *(unsigned int *)tmpIP->ip_dst = *(unsigned int *)(datagram + 30);
}

void parseEthernet(struct ethernet *tmpEthernet, const u_char *datagram)
{
    for (int i = 0; i < 6; i++)
    {
        tmpEthernet->eth_dsthost[i] = datagram[i];
    }
    for (int i = 6; i < 12; i++)
    {
        tmpEthernet->eth_srchost[i - 6] = datagram[i];
    }
    tmpEthernet->eth_type = ntohs(*(u_short *)(datagram + 12));
}

void callback(u_char *arg, const struct pcap_pkthdr *pkthdr, const u_char *datagram)
{
    // 打印序号信息
    struct callbackMsg* msg = (struct callbackMsg*)arg;
    int* cnt = msg->cnt;
    FILE* filePtr = msg->file;
    *cnt = (*cnt)+1;
    fprintf(filePtr, "cnt: %d\n", *cnt);
    fprintf(filePtr, "Recieved time: %s \n", ctime((const time_t *)&pkthdr->ts.tv_sec));

    struct ethernet tmpE;
    struct ip tmpIP;

    // 解析以太网帧头
    parseEthernet(&tmpE, datagram);
    fprintf(filePtr, "src_mac: ");
    for (int i = 0; i < 6; i++)
    {
        fprintf(filePtr, " %02x", tmpE.eth_srchost[i]);
    }
    fprintf(filePtr, "\n");
    fprintf(filePtr, "dst_mac: ");
    for (int i = 0; i < 6; i++)
    {
        fprintf(filePtr, " %02x", tmpE.eth_dsthost[i]);
    }
    fprintf(filePtr, "\n");
    // 解析IP帧头
    parseIp(&tmpIP, datagram);
    char ipv4[64];
    memset(&ipv4,0,64);
    /// 输出的是点分表示
    inet_ntop(AF_INET, &tmpIP.ip_src, ipv4, 64);
    fprintf(filePtr, "src_ip: %s \n", ipv4);

    memset(&ipv4,0,64);
    inet_ntop(AF_INET, &tmpIP.ip_dst, ipv4, 64);
    fprintf(filePtr, "dst_ip: %s \n", ipv4);

    // 根据协议选择分析报文
    if(tmpIP.ip_pro == 6){
        struct tcp tmpTCP;
        parseTcp(&tmpTCP,datagram);
        fprintf(filePtr, "src_port[TCP]: %u\n", tmpTCP.tcp_srcport);
        fprintf(filePtr, "dst_port[TCP]: %u\n", tmpTCP.tcp_dstport);
    }else if(tmpIP.ip_pro == 17){
        struct udp tmpUDP;
        parseUdp(&tmpUDP,datagram);
        fprintf(filePtr, "src_port[UDP]: %u\n", tmpUDP.udp_srcport);
        fprintf(filePtr, "dst_port[UDP]: %u\n", tmpUDP.udp_dstport);
    }

    fprintf(filePtr, "\n");    
}

char* getTime(){
    char *timeStr = (char*)malloc(20);
    bzero(timeStr, 20);
    time_t nowTime;
    struct tm *timeTm;
    nowTime = time(NULL);
    timeTm = localtime(&nowTime);
    strftime(timeStr, 20, "%Y_%m_%d_%H_%M_%S", timeTm);
    return timeStr;
}

int main(int argc, char *argv[])
{
    char rule[4];
    memset(&rule, 0, 4);
    // 要求输入过滤规则
    printf("***************************************************\n");
    printf("目前支持下列规则的过滤：\n");
    printf("[1] TCP\n");
    printf("[2] UDP\n");
    printf("[3] NULL\n");
    printf("***************************************************\n");
    printf("请输入你所需要的过滤规则：");
    scanf("%s",rule);

    // 查找网卡
    pcap_if_t *allDevs;
    pcap_t *device;
    char errorbuf[PCAP_ERRBUF_SIZE];
    if (pcap_findalldevs(&allDevs, errorbuf) == -1)
    {
        printf("查找网卡出现错误: %s\n", errorbuf);
        exit(1);
    }
    if(allDevs == NULL){
        printf("没有找到任何网卡");
        exit(1);
    }
    pcap_if_t *tmpDev = allDevs;
    int cnt = 1;
    printf("***************************************************\n");
    printf("下面是所查找到的网卡信息：\n");
    printf("***************************************************\n");
    while(tmpDev != NULL){
        printf("%d %s ",cnt,tmpDev->name);
        if(tmpDev->description){
            printf("%s\n",tmpDev->description);
        }else{
            printf("no detail Info\n");
        }
        tmpDev = tmpDev->next;
        cnt += 1;
    }
    printf("\n");
    // 选择所要打开的网卡
    int interfaceNum;
    printf("请选择你所要打开的网卡号，或者输入0退出程序:");
    scanf("%d",&interfaceNum);
    if(interfaceNum == 0){
        pcap_freealldevs(allDevs);
        return -1;
    }

    tmpDev = allDevs;
    for(int i = 0;i<interfaceNum - 1;i++){
        tmpDev = tmpDev->next;
    }
    if ((device = pcap_open_live(tmpDev->name,
                                 65536,
                                 0,     // 非混杂模式
                                 1000,  // 读超时为1秒
                                 errorbuf
                                 )) == NULL)
    {

        printf("打开网卡出现错误");
        pcap_freealldevs(allDevs);
        return -1;
    }
    pcap_freealldevs(allDevs);

    // 创建保存捕获数据的文件
    FILE *filePtr;
    char fileName[32];
    memset(&fileName, 0 ,32);

    sprintf(fileName, "capture_%s.txt", getTime());

    printf("捕获的数据将保存在文件 %s 中\n",fileName);
    if ((filePtr = fopen(fileName, "w")) == NULL)
    {
        printf("打开文件出现错误\n");
        exit(1);
    }
    setbuf(filePtr,NULL);

    // 创建过滤规则
    if(strcmp(rule,"NULL")!=0){
        struct bpf_program filter;
        if(pcap_compile(device, &filter, rule, 1, 0)<0){
            printf("设置规则出现错误");
        }
        pcap_setfilter(device, &filter);
    }
    fprintf(filePtr, "过滤规则: %s\n\n", rule);

    // 开始捕获
    cnt = 0;
    struct callbackMsg arg;
    arg.cnt = &cnt;
    arg.file = filePtr;
    pcap_loop(device, -1, callback, (u_char *)&arg);
    pcap_close(device);
    fclose(filePtr);
    return 0;
}