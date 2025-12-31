#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTextBrowser>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QHostInfo>
#include <QLineEdit>
#include <QNetworkInterface>
#include <QDebug>
#include <QMainWindow>
#include <QtNetwork>
#include <QDataStream>
#include <QTextStream>
#include  "TcpServer.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    //成员变量
    QString curpath;//当前路径
    QFile *curfile;//当前文件指针
    QTimer *timer;//定时器指针
    int TcpClientLinkCnt;//客户端连接计数
    QList<int> tcpClientSocketDescriptorList;//客户端套接字描述符列表
    QString CurIPPort;//当前数据来源和IP端口
    TcpServer *mtcpServer;//TCP服务器对象指针
    QHostAddress lhAddr;//本地地址
    int lhPort;//本地端口
    QTcpSocket *tcpClientSocket;//客户端套接字指针
    QHostAddress rmtAddr;//远程地址
    int rmtPort;//远程端口
    QUdpSocket *udpSocket;//UDP套接字指针
    bool loopSending;//循环发送标志
    QHostAddress *rmtServerIP;//远程服务器IP地址指针
    QString m_ip;//本机IP地址
    const char toHex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};//16进制映射表
    //自定义调用函数
    void msDelay(unsigned int msec);//延时函数
    char ConvertHexChar(char ch);//Hex转数值
    char ConvertHexStr(QString hexSubStr);//Hex转str
    void slotDeleteTcpServer();//删除TCP服务器
    bool slotTryCreateTcpServer();//创建TCP服务器
    void insertDateTimeInRcvDisp();//在接收显示区插入日期时间
    void toSendFile();//发送文件核心函数
signals:
    void sendDataToClient(char *msg, int length, int socketDescriptor, int socketDescriptorEx);//发送数据到客户端信号
private slots:
    void on_left_1_1_currentIndexChanged(int index);

    void on_s_check_3_toggled(bool checked);

    void on_s_check_1_toggled(bool checked);

    void on_r_check_1_toggled(bool checked);

    void on_pushButton_5_clicked();

    void on_s_check_4_toggled(bool checked);

    void on_pushButton_clicked();

    void on_lineEdit_3_editingFinished();

    void on_pushButton_3_clicked();

    void on_r_lineEdit_2_textChanged(const QString &arg1);

    void on_r_lineEdit_3_textChanged(const QString &arg1);

    void on_pushButton_6_clicked();
    void on_pushButton_4_clicked();
    void on_pushButton_2_clicked(bool checked);
    //自定义槽函数
    void removeClientLink(QString clientAddrPort, int socketDescriptor);//移除客户端连接槽函数
    void addClientLink(QString clientAddrPort, int socketDescriptor);//添加客户端连接槽函数
    void tcpServerDataReceived(char *msg, int length, int socketDescriptorEx);//TCP服务器接收客户端数据槽函数
    void tcpClientDataReceived();//TCP客户端接收服务器数据槽函数，
    void udpDataReceived();//UDP数据接收槽函数,Socket的readyRead()信号槽函数
    void toSendData();//发送数据核心函数，与定时器信号连接
private:
      QList<QHostAddress> IPlist;
    Ui::MainWindow *ui;
    /* 获取本地的所有ip */
        void getLocalHostIP();
};
#endif // MAINWINDOW_H
