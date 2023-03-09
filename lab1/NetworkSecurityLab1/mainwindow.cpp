#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_ui(new Ui::MainWindow)
{
    m_ui->setupUi(this);
    this->drawIpEditor();
    this->bindButtonFuc();

    // 连接槽
    connect(m_ui->lineEdit, SIGNAL(textChanged(QString)), this,SLOT(checkIPNum(QString)));
    connect(m_ui->lineEdit_2, SIGNAL(textChanged(QString)), this,SLOT(checkIPNum(QString)));
    connect(m_ui->lineEdit_3, SIGNAL(textChanged(QString)), this,SLOT(checkIPNum(QString)));
    connect(m_ui->lineEdit_4, SIGNAL(textChanged(QString)), this,SLOT(checkIPNum(QString)));
    connect(m_ui->lineEdit_5, SIGNAL(textChanged(QString)), this,SLOT(checkIPNum(QString)));
    connect(m_ui->lineEdit_6, SIGNAL(textChanged(QString)), this,SLOT(checkIPNum(QString)));
    connect(m_ui->lineEdit_7, SIGNAL(textChanged(QString)), this,SLOT(checkIPNum(QString)));
    connect(m_ui->lineEdit_8, SIGNAL(textChanged(QString)), this,SLOT(checkIPNum(QString)));
    connect(m_ui->lineEdit_9, SIGNAL(textChanged(QString)), this,SLOT(checkPortNum(QString)));
    connect(m_ui->lineEdit_10, SIGNAL(textChanged(QString)), this,SLOT(checkPortNum(QString)));
    connect(m_ui->lineEdit_11, SIGNAL(textChanged(QString)), this,SLOT(checkThreadNum(QString)));
    qDebug()<<"程序已经正常启动";
}

MainWindow::~MainWindow()
{
    delete m_ui;
}

void MainWindow::checkIPNum(QString data){
    int num = data.toInt();
    if(num>255){
        QMessageBox::critical(this, tr("错误"),  tr("最大值为255"));
    }
}

void MainWindow::checkPortNum(QString data){
    int num = data.toInt();
    if(data == ""){
        return ;
    }
    if(num>=65535){
        QMessageBox::critical(this, tr("错误"),  tr("最大值为65535"));
    }else if(num<0){
        QMessageBox::critical(this, tr("错误"),  tr("最小值为10"));
    }
}

void MainWindow::checkThreadNum(QString data){
    int num = data.toInt();
    if(data == ""){
        return ;
    }
    if(num>=1000){
        QMessageBox::critical(this, tr("错误"),  tr("最大值为1000"));
    }else if(num<0){
        QMessageBox::critical(this, tr("错误"),  tr("最小值为1"));
    }
}

bool MainWindow::checkInput(){
    if (this->m_ui->lineEdit->text() == "" ||this->m_ui->lineEdit_2->text() == "" ||this->m_ui->lineEdit_3->text() == "" ||
        this->m_ui->lineEdit_4->text() == "" ||this->m_ui->lineEdit_5->text() == "" ||this->m_ui->lineEdit_6->text() == "" ||
        this->m_ui->lineEdit_7->text() == "" ||this->m_ui->lineEdit_8->text() == "" ||this->m_ui->lineEdit_9->text() == "" ||
        this->m_ui->lineEdit_10->text() == "" ||this->m_ui->lineEdit_11->text() == ""){
        return false;
    }
    return true;
}

void MainWindow::startIPScan(){
    if(!checkInput()){
        QMessageBox::critical(this, tr("错误"),  tr("输入的参数不完整"));
        return ;
    }
    m_ui->startBtn->setText("停止扫描");
    m_ui->textBrowser->clear();
    m_isRunner = 1;
    m_startTime = Clock::now();
    std::vector<std::pair<QString,int>> ().swap(m_opendPort);
    long ipAddrS = 0,ipAddrE = 0,tmpIP1 = 0,tmpIP2 = 0,tmpIP3 = 0,tmpIP4 = 0;
    int portS,portE,threadNum;

    // ipAddrS
    tmpIP1 = this->m_ui->lineEdit->text().toLong();
    tmpIP2 = this->m_ui->lineEdit_2->text().toLong();
    tmpIP3 = this->m_ui->lineEdit_3->text().toLong();
    tmpIP4 = this->m_ui->lineEdit_4->text().toLong();
    ipAddrS = (tmpIP1<<24)+(tmpIP2<<16)+(tmpIP3<<8)+tmpIP4;

    //ipAddrE
    tmpIP1 = this->m_ui->lineEdit_5->text().toLong();
    tmpIP2 = this->m_ui->lineEdit_6->text().toLong();
    tmpIP3 = this->m_ui->lineEdit_7->text().toLong();
    tmpIP4 = this->m_ui->lineEdit_8->text().toLong();
    ipAddrE = (tmpIP1<<24)+(tmpIP2<<16)+(tmpIP3<<8)+tmpIP4;

    portS = this->m_ui->lineEdit_9->text().toInt();
    portE = this->m_ui->lineEdit_10->text().toInt();
    threadNum = this->m_ui->lineEdit_11->text().toInt();

    //IPAcanner Apply
    applyJob(ipAddrS, ipAddrE, portS, portE, threadNum, m_ui->textBrowser);
}

void MainWindow::startJob(){
    if(m_isRunner == 0){
        startIPScan();
    }else{
        QMessageBox::StandardButton qBox = QMessageBox::question(this,"提示", "确定要终止扫描程序吗?", QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);
        if(qBox == QMessageBox::Yes){
            emit sendStopSignal();
            m_ui->progressBar->setValue(0);
            m_ui->startBtn->setText("扫描");
            m_isRunner = 0;
            m_ui->textBrowser->append(QString("============================================="));
            m_ui->textBrowser->append(QString("STOP"));
            m_ui->textBrowser->append(QString("============================================="));
        }
    }
}

void MainWindow::bindButtonFuc(){
    m_ui->progressBar->setMinimum(0);
    m_ui->progressBar->setMaximum(100);
    m_ui->progressBar->setValue(0);
    connect(m_ui->startBtn,SIGNAL(clicked(bool)),this,SLOT(startJob()));
}

void MainWindow::applyJob(long ipv4S, long ipv4E, int portS, int portE, int threadNum, QTextBrowser* textUi)
{
    qDebug()<<"IPScanner Apply";
    int lastJobCnt = (ipv4E - ipv4S +1)*(portE-portS+1);
    totalJob = lastJobCnt;
    int lastThreadCnt = threadNum;
    int jobPerThread = lastJobCnt/lastThreadCnt;
    std::vector<std::pair<long,int>> jobList;

    int threadID = 0;
    for(long i = ipv4S; i<=ipv4E; i++){
        for(int j = portS; j<=portE; j++){
            jobList.push_back(std::pair<long,int>(i,j));
            lastJobCnt -= 1;
            if(jobList.size()>=jobPerThread){
                m_scanfThread[threadID] = new ScanfThreadWork(jobList);
                connect(m_scanfThread[threadID],SIGNAL(sendResult(QString,QString,int,int)),this,SLOT(printResult(QString,QString,int,int)),Qt::QueuedConnection);
                connect(this, &MainWindow::sendStopSignal,m_scanfThread[threadID],&ScanfThreadWork::recvStopSignal);
                m_scanfThread[threadID]->start();
                threadID += 1;
                lastThreadCnt -= 1;
                std::vector<std::pair<long,int>>().swap(jobList);
                if(lastJobCnt > 0){
                    jobPerThread = lastJobCnt/lastThreadCnt;
                }
            }
        }
    }
    if(jobList.size()!=0){
        m_scanfThread[threadID] = new ScanfThreadWork(jobList);
        connect(m_scanfThread[threadID],SIGNAL(sendResult(QString,QString,int,int)),this,SLOT(printResult(QString,QString,int,int)),Qt::QueuedConnection);
        connect(this, &MainWindow::sendStopSignal,m_scanfThread[threadID],&ScanfThreadWork::recvStopSignal);
        m_scanfThread[threadID]->start();
        std::vector<std::pair<long,int>>().swap(jobList);
    }
    qDebug()<<"Finished Job Sent To Thread.";
}

void MainWindow::printResult(QString data, QString ipAddr, int jobPort, int status){
    if(m_isRunner == 0){
        return;
    }
    m_lock.lock();
    if(status == 1){
        m_opendPort.push_back(std::pair<QString,int>(ipAddr,jobPort));
    }
    m_ui->progressBar->setValue((int)(((float)finishedJob/(float)totalJob)*100));
    m_ui->textBrowser->append(data);
    finishedJob += 1;
    m_lock.unlock();
    if(finishedJob == totalJob){
        m_ui->startBtn->setText("扫描");
        m_ui->progressBar->setValue(100);
        m_endTime = Clock::now();
        long long interval = std::chrono::duration_cast<std::chrono::seconds>(m_endTime - m_startTime).count();

        m_ui->textBrowser->append(QString("============================================="));
        m_ui->textBrowser->append(QString("扫描完成"));
        m_ui->textBrowser->append(QString("用时:"+QString::number(interval)+"s"));
        m_ui->textBrowser->append(QString("开启的端口:"));
        for(auto i:m_opendPort){
            qDebug()<<" 1";
            m_ui->textBrowser->append(QString(i.first+" "+QString::number(i.second)));
        }
        m_ui->textBrowser->append(QString("============================================="));
    }
}

void MainWindow::drawIpEditor(){
    //给每个IP Editor设置下一个文本框
    m_ui->lineEdit->setNext(m_ui->lineEdit_2);
    m_ui->lineEdit_2->setNext(m_ui->lineEdit_3);
    m_ui->lineEdit_3->setNext(m_ui->lineEdit_4);
    m_ui->lineEdit_4->setNext(m_ui->lineEdit_5);
    m_ui->lineEdit_5->setNext(m_ui->lineEdit_6);
    m_ui->lineEdit_6->setNext(m_ui->lineEdit_7);
    m_ui->lineEdit_7->setNext(m_ui->lineEdit_8);

    //输入限制
    m_ui->lineEdit->setValidCheck();
    m_ui->lineEdit_2->setValidCheck();
    m_ui->lineEdit_3->setValidCheck();
    m_ui->lineEdit_4->setValidCheck();
    m_ui->lineEdit_5->setValidCheck();
    m_ui->lineEdit_6->setValidCheck();
    m_ui->lineEdit_7->setValidCheck();
    m_ui->lineEdit_8->setValidCheck();

    m_ui->lineEdit_9->setMaxLength(5);
    m_ui->lineEdit_10->setMaxLength(5);
    m_ui->lineEdit_11->setMaxLength(5);
}
