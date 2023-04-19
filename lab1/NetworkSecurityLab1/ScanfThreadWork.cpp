#include "ScanfThreadWork.h"

ScanfThreadWork::ScanfThreadWork(std::vector<std::pair<long,int>> job){
    m_job = job;
}
void ScanfThreadWork::run(){
    apply();
}

void ScanfThreadWork::apply()
{
    for(auto i = m_job.begin(); i!=m_job.end() && m_isStop;i++){
        int jobIp = i->first;
        int jobPort = i->second;
        int ip1 = (jobIp & 0xff000000) >> 24;
        int ip2 = (jobIp & 0x00ff0000) >> 16;
        int ip3 = (jobIp & 0x0000ff00) >> 8;
        int ip4 = (jobIp & 0x000000ff);
        QString ipAddr = QString("%1.%2.%3.%4")
                .arg(QString::number(ip1))
                .arg(QString::number(ip2))
                .arg(QString::number(ip3))
                .arg(QString::number(ip4));
        io_service ios;
        tcp::socket s(ios);
        tcp::endpoint ep(boost::asio::ip::address_v4::from_string(ipAddr.toStdString()), jobPort);
        if (!aysnc_connect(ios, s, ep, 1000))
        {
            emit sendResult(QString("IP Addr:"+ipAddr+"\tPort:"+QString::number(jobPort)+"\tCLOSE"),ipAddr,jobPort,0);
        }else{
            emit sendResult(QString("IP Addr:"+ipAddr+"\tPort:"+QString::number(jobPort)+"\tOPEN"),ipAddr,jobPort,1);
        }
    }
}

void ScanfThreadWork::recvStopSignal(){
    m_isStop = 0;
}

bool ScanfThreadWork::aysnc_connect(io_service & ioC, tcp::socket & socketC, tcp::endpoint &ep, int millionSeconds)
{
    bool connectStatus = false;
    socketC.async_connect(ep, boost::bind(&ScanfThreadWork::connectSucc, this, _1, boost::ref(connectStatus)));
    boost::asio::deadline_timer timer(ioC);
    timer.expires_from_now(boost::posix_time::milliseconds(millionSeconds));
    bool timeout = false;
    timer.async_wait(boost::bind(&ScanfThreadWork::timeOut, this, _1, boost::ref(timeout)));
    do
    {
        ioC.run_one();
    } while (!timeout && !connectStatus);
    timer.cancel();
    return connectStatus;
}
void ScanfThreadWork::timeOut(boost::system::error_code ec, bool &timeout){
    if(!ec){
        timeout = true;
    }
}
void ScanfThreadWork::connectSucc(boost::system::error_code ec, bool &connectStatus){
    if(!ec){
        connectStatus = true;
    }
}
