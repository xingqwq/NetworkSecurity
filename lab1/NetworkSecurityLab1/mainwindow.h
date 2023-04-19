#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <iostream>
#include <QMainWindow>
#include <QLineEdit>
#include <QKeyEvent>
#include <QWidget>
#include <QDebug>
#include <QIntValidator>
#include <QString>
#include <QMessageBox>
#include <thread>
#include <chrono>

#include "LineEdit.h"
#include "ScanfThreadWork.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

using Clock = std::chrono::steady_clock;
using TimePoint = std::chrono::time_point<Clock>;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *m_ui;
    ScanfThreadWork* m_scanfThread[1000];
    std::mutex m_lock;
    int totalJob, finishedJob = 0;

    void drawIpEditor();
    void bindButtonFuc();
    bool checkInput();
    void applyJob(long ipv4S, long ipv4E, int portS, int portE, int threadNum, QTextBrowser* textUi);
    void startIPScan();

signals:
    void sendStopSignal();

private slots:
    void startJob();
    void checkIPNum(QString data);
    void checkPortNum(QString data);
    void checkThreadNum(QString data);

    void printResult(QString data, QString ipAddr, int jobPort, int status);

private:
    int m_isRunner = 0;
    TimePoint m_startTime,m_endTime;
    std::vector<std::pair<QString,int>> m_opendPort;
};

#endif // MAINWINDOW_H
