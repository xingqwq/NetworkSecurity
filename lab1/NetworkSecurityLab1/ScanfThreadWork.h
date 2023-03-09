#ifndef SCANFTHREADWORK_H
#define SCANFTHREADWORK_H

#include <iostream>
#include <vector>
#include <QMainWindow>
#include <QDebug>
#include <QString>
#include <QTextBrowser>
#include <QThread>
#include <thread>
#include <mutex>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

using boost::asio::ip::tcp;
using boost::asio::io_service;
using boost::asio::deadline_timer;
using namespace boost::placeholders;

class ScanfThreadWork:public QThread
{
    Q_OBJECT
public:
    ScanfThreadWork(std::vector<std::pair<long,int>> job);
    void run() override;
    bool aysnc_connect(io_service & ioC, tcp::socket & socketC, tcp::endpoint &ep, int millionSeconds);
    void timeOut(boost::system::error_code ec, bool &timeout);
    void connectSucc(boost::system::error_code ec, bool &connectStatus);

signals:
    void sendResult(QString data, QString ipAddr, int jobPort, int status);

public slots:
    void apply();
    void recvStopSignal();

private:
    std::vector<std::pair<long,int>> m_job;
    int m_isStop = 1;
};

#endif // SCANFTHREADWORK_H
