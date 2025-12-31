#include "TcpServer.h"

#include <QTcpSocket>
//构造函数
TcpServer::TcpServer(QObject *parent): QTcpServer(parent){
connect(this, SIGNAL(newConnection()), this, SLOT(acceptnewclient()));
}
//解构函数
 TcpServer::~TcpServer(){};
//槽函数
 //接收新客户端
 void TcpServer::acceptnewclient(){
     int socketDescriptor;
     QTcpSocket *tcpcilentsocket=nextPendingConnection();
     socketDescriptor=tcpcilentsocket->socketDescriptor();
     //配置客户端断开的信号，到客户端断开槽函数和socket内存释放槽函数
     connect(tcpcilentsocket,SIGNAL(disconnected()),this,SLOT(disconnectclient()));
     connect(tcpcilentsocket,SIGNAL(disconnected()),tcpcilentsocket,SLOT(deleteLater()));
     connect(tcpcilentsocket,SIGNAL(readyRead()/*客户端有数据*/),this,SLOT(clientDataReceived()));
     //把客户端添加到客户端列表
     tcpClientSocketList.append(tcpcilentsocket);
     //构造客户端地址端口字符串
     QString peerPortStr=QString::number(tcpcilentsocket->peerPort());
     QString rdClientAddress_Port=tcpcilentsocket->peerAddress().toString()+":"+peerPortStr;
     emit addClientLink(rdClientAddress_Port,socketDescriptor);

 }
 //客户端断开
 void TcpServer::disconnectclient(){
     for(int i=0;i<tcpClientSocketList.count();i++){
         QTcpSocket *item=tcpClientSocketList[i];
         if(item->state()==0){
             QString peerportstr=QString::number(item->peerPort());
             QString rdClientAddress_Port=item->peerAddress().toString()+":"+peerportstr;
             emit signal_removeClientLink(rdClientAddress_Port,item->socketDescriptor());
             tcpClientSocketList.removeAt(i);
             break;
            }
     }
 }
 //服务器接收客户端数据
 void TcpServer::clientDataReceived(){
     for(int i=0;i<tcpClientSocketList.count();i++){
         QTcpSocket *item=tcpClientSocketList[i];
         while(item->bytesAvailable()>0){
             //创建数组，并设置大小
              QByteArray datagram;
              datagram.resize(item->bytesAvailable());
             //给数组赋值
             item->read(datagram.data(),datagram.size());
             emit updateTcpServer((char*)datagram.data(),datagram.size(),item->socketDescriptor());
         }
        }
 }
 //客户端接收服务器数据
 void TcpServer::sendDataToClient(char *msg, int length, int socketDescriptor/*目标客户端套接字*/, int socketDescriptorEx/*黑名单客户端套接字*/){
   for(int i=0;i<tcpClientSocketList.count();i++){
       QTcpSocket *item=tcpClientSocketList[i];
     //判断套接字
     if(socketDescriptor==0){
         //排除广播模式中的黑名单客户端
         if(item->socketDescriptor()!=socketDescriptorEx){
            if(item->write(msg,length)!=length){
                continue;//单个客户端发送失败，尽心下个客户端
            }
         }
     }else{
         if(item->socketDescriptor()==socketDescriptor){
             if(item->write(msg,length)!=length){
                 break;
             }
         }
     }
}
 }
