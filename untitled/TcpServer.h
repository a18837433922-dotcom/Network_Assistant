#ifndef TCPSERVER_H
#define TCPSERVER_H
#include <QTcpServer>
class TcpServer: public QTcpServer{
    Q_OBJECT
public:
     TcpServer(QObject *parent = 0);
     ~TcpServer();
     QList<QTcpSocket*> tcpClientSocketList;//套接字列表，里面存放的是客户端的套接字

    signals:
     void signal_removeClientLink(QString, int);//移除客户端信号
     void updateTcpServer(char*, int, int);//发送客户端数据信号，并给服务器槽函数传递数据
      void addClientLink(QString, int);//添加客户端连接信号
    public slots:
    void disconnectclient();//客户端断开
    void acceptnewclient();//接收新的客户端
    void clientDataReceived();//服务器接收客户端数据
    void sendDataToClient(char *msg, int length, int socketDescriptor, int socketDescriptorEx); //向客户端发送数据
protected:

private:
};


#endif // TCPSERVER_H
