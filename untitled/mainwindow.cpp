



#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTranslator>
#include <QMessageBox>
#include <stdlib.h>
#include <QFileDialog>
#include <QFile>
#include <QString>
#include <QTimer>
#include <QInputDialog>
#include <QDateTime>
#include "TcpServer.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //左上角，协议类型
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    for(int i = 0; i < ipAddressesList.size(); ++i){
    if(ipAddressesList.at(i) != QHostAddress::LocalHost && ipAddressesList.at(i).toIPv4Address()){
         m_ip = ipAddressesList.at(i).toString();
         break;}
    }
    //本地端口
    ui->left_1_3->setRange(1000,99999);
    
    // 设置连接按钮为可切换按钮（重要！）
    ui->pushButton_2->setCheckable(true);
    ui->pushButton_2->setText(tr("连接网络"));
    
    //初始化全局变量
     rmtServerIP = new QHostAddress();
     TcpClientLinkCnt = 0;
     loopSending = false;
     CurIPPort = "";
     curpath = "";
     curfile = 0;
     mtcpServer = nullptr;
     tcpClientSocket = nullptr;
     udpSocket = nullptr;
     timer = nullptr;
}

MainWindow::~MainWindow()
{
    delete ui;
}

/************************************************************************************************************************************/
//槽函数
//left_1索引改变时触发槽函数
void MainWindow::on_left_1_1_currentIndexChanged(int index)
{
    if(index==0){//UDP模式索引为0
        //左
    //发送
    ui->s_check_1->setVisible(true);
    ui->s_check_2->setVisible(true);
    ui->s_check_3->setVisible(true);
    ui->s_check_4->setVisible(true);
    //接收
    ui->r_check_1->setVisible(true);
    ui->r_check_2->setVisible(true);
    ui->r_check_3->setVisible(true);
    ui->r_check_4->setVisible(true);
    ui->r_check_5->setVisible(false);
    ui->r_check_6->setVisible(false);
    //IP和Port
    ui->label_2->setText(tr("本地IP地址"));
    ui->label_3->setText(tr("本地端口"));
    ui->label_6->setText(tr("目标IP地址"));
    ui->label_5->setText(tr("目标端口"));
    ui->left_1_2->setText(m_ip);
    //右
    ui->label_4->setVisible(false);
    ui->label_5->setVisible(true);
    ui->label_6->setVisible(true);
    ui->r_lineEdit_1->setVisible(false);
    ui->r_lineEdit_2->setVisible(true);
    ui->r_lineEdit_3->setVisible(true);
    }
    if(index==1){//TCP服务器索引为1
        //发送
        ui->s_check_1->setVisible(true);
        ui->s_check_2->setVisible(true);
        ui->s_check_3->setVisible(true);
        ui->s_check_4->setVisible(true);
        //接收
        ui->r_check_1->setVisible(true);
        ui->r_check_2->setVisible(true);
        ui->r_check_3->setVisible(true);
        ui->r_check_4->setVisible(true);
        ui->r_check_5->setVisible(true);
        ui->r_check_6->setVisible(true);
        //IP和Port
        ui->label_2->setText(tr("本地端口"));
        ui->label_3->setText(tr("本地IP地址"));
        ui->left_1_2->setText(m_ip);
        //右
        ui->label_4->setVisible(true);
        ui->label_5->setVisible(false);
        ui->label_6->setVisible(false);
        ui->r_lineEdit_1->setVisible(true);
        ui->r_lineEdit_2->setVisible(false);
        ui->r_lineEdit_3->setVisible(false);

    }
    if(index==2){//TCP客户端索引为2
        //发送
        ui->s_check_1->setVisible(true);
        ui->s_check_2->setVisible(true);
        ui->s_check_3->setVisible(true);
        ui->s_check_4->setVisible(true);
        //接收
        ui->r_check_1->setVisible(true);
        ui->r_check_2->setVisible(true);
        ui->r_check_3->setVisible(true);
        ui->r_check_4->setVisible(true);
        ui->r_check_5->setVisible(true);
        ui->r_check_6->setVisible(true);
        //IP和Port
        ui->label_2->setText(tr("服务器IP地址"));
        ui->label_3->setText(tr("服务器端口"));
        ui->label_6->setText(tr("本地IP地址"));
        ui->label_5->setText(tr("本地端口"));
        ui->left_1_2->setText(tr("192.168.129.128"));//默认IP
        //右
        ui->label_4->setVisible(false);
        ui->label_5->setVisible(true);
        ui->label_6->setVisible(true);
        ui->r_lineEdit_1->setVisible(false);
        ui->r_lineEdit_2->setVisible(true);
        ui->r_lineEdit_3->setVisible(true);
    }
}
/************************************************************************************************************************************/
//自定义函数
//Hex转数值，用char来表示整数数
char MainWindow::ConvertHexChar(char ch){
    if((ch>='0')&&(ch<='9')){
        return ch-'0';
    }
    if((ch>='A')&&(ch<='F')){
        return ch-'A'+10;
    }
    if((ch>='a')&&(ch<='f')){
        return ch-'a'+10;
    }
    return -1;
}
//Hex转str
char MainWindow::ConvertHexStr(QString hexSubStr){
    char ch=0;
    if(hexSubStr.length()==2){
        ch=ConvertHexChar(hexSubStr.at(0).toLatin1())*16+ConvertHexChar(hexSubStr.at(1).toLatin1());
    }
    if(hexSubStr.length()==1){
        ch=ConvertHexChar(hexSubStr.at(0).toLatin1());
    }
    return ch;
}
//毫秒演示函数
void MainWindow::msDelay(unsigned int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while(QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 1);  // 处理事件，保持界面响应
}
//删除TCP服务器
void MainWindow::slotDeleteTcpServer(){
    if (mtcpServer != nullptr)
    {
        mtcpServer->disconnect();
        mtcpServer->close();
        delete mtcpServer;
        mtcpServer = nullptr;
    }
}
//创建TCP服务器
bool MainWindow::slotTryCreateTcpServer(){
    if (mtcpServer != nullptr)
    {
        slotDeleteTcpServer();
    }
    mtcpServer=new TcpServer(this);
    if(!mtcpServer->listen(lhAddr,lhPort)){
     QMessageBox::information(this, tr("错误"), tr("尝试建立服务器失败! 请确认网络状态和端口。"));
     delete mtcpServer;
     mtcpServer = nullptr;
     return false;
    }
    //连接服务器内置信号和槽
    connect(mtcpServer, SIGNAL(updateTcpServer(char*, int, int)), this, SLOT(tcpServerDataReceived(char*, int, int)));//信号槽中传函数不能有参数名，只需要传类型
    connect(this, SIGNAL(sendDataToClient(char*, int, int, int)), mtcpServer, SLOT(sendDataToClient(char*, int, int, int)));
    connect(mtcpServer, SIGNAL(addClientLink(QString, int)), this, SLOT(addClientLink(QString, int)));
    connect(mtcpServer, SIGNAL(signal_removeClientLink(QString, int)), this, SLOT(removeClientLink(QString, int)));
    return true;  // 创建成功
}
//在接收显示区插入日期时间
void MainWindow::insertDateTimeInRcvDisp(){
    int year, month, day;
    QDateTime::currentDateTime().date().getDate(&year, &month, &day);
     QString date = QString::number(year, 10) + "-" + QString::number(month, 10) + "-" + QString::number(day, 10);
    ui->textreceive->append(tr("【") + date + tr(" ") + QDateTime::currentDateTime().time().toString() + tr("】"));
}
//发送文件核心函数
void MainWindow::toSendFile(){
    //检查文件是否有效
    if(curfile==0)
        return ;
    char buf[1024];
    int rdlength=0;
    //UDP发送
    if(ui->left_1_1->currentIndex()==0){//UDP索引为0
        while(!curfile->atEnd()){
              rdlength= curfile->read(buf, 1024);
              udpSocket->writeDatagram(buf,rdlength,rmtAddr,rmtPort);
              msDelay(1);
        }
    }
    if(ui->left_1_1->currentIndex()==1){//TCP服务器索引为1
        int idx=ui->r_lineEdit_1->currentIndex();//取连接对象下拉框索引
        if(idx==0){//发送所有客户端
            while(!curfile->atEnd()){
            rdlength = curfile->read(buf, 1024);
            emit sendDataToClient(buf, rdlength, 0, 0);
            QCoreApplication::processEvents(QEventLoop::AllEvents, 1);  // 处理事件，保持界面响应
        }}else{//发送给指定客户端
            while(!curfile->atEnd()){
            rdlength = curfile->read(buf, 1024);
            emit sendDataToClient(buf, rdlength, tcpClientSocketDescriptorList.at(idx), 0);
             QCoreApplication::processEvents(QEventLoop::AllEvents, 1);
            }
        }
    }
    if(ui->left_1_1->currentIndex()==2){//以客户端模式发送
        if(tcpClientSocket != nullptr){
            while(!curfile->atEnd()){
            rdlength = curfile->read(buf, 1024);
            tcpClientSocket->write(buf, rdlength); //写入客户端套接字缓冲区
            QCoreApplication::processEvents(QEventLoop::AllEvents, 1);  // 处理事件，保持界面响应
            }
        }
    }
}
//发送数据核心函数
void MainWindow::toSendData(){
     QByteArray datagram;
     //检查数据处理选项
     if(ui->s_check_3->checkState()){//检查是否按照16进制发送
         QStringList hexStr=ui->textsend->toPlainText().split(" ", QString::SkipEmptyParts);
         int hexSize = hexStr.size();
        for(int i=0;i<hexSize;i++){
            QString hexSubStr = hexStr.at(i);
            datagram.append(ConvertHexStr(hexSubStr));
        }
        datagram.resize(hexSize);
         }else {//以字符串发送模式
        datagram=ui->textsend->toPlainText().toLocal8Bit();}
    //数据发送
     if(datagram.size()==0){
         return ;
     }
     if(ui->left_1_1->currentIndex()==0){//以UDP协议发送
         udpSocket->writeDatagram(datagram.data(),datagram.size(),rmtAddr,rmtPort);
     }
     if(ui->left_1_1->currentIndex()==1){//TCP服务器模式
        int idx = ui->r_lineEdit_1->currentIndex();
        if(idx == 0) {
         emit sendDataToClient((char *)datagram.data(), datagram.size(), 0, 0);
         } else {
          emit sendDataToClient((char *)datagram.data(), datagram.size(), tcpClientSocketDescriptorList.at(idx), 0);}
     }
     if(ui->left_1_1->currentIndex()==2){//TCP客户端模式
        if(tcpClientSocket != nullptr){
            tcpClientSocket->write(datagram.data(), datagram.size());
        }
     }}
/************************************************************************************************************************************/
//复选框
//复选框按16进制发送的槽函数
void MainWindow::on_s_check_3_toggled(bool checked)
{
     QByteArray datagram;//字节数组


    if(checked==true){//切换到16进制显示模式
        if(ui->textsend->toPlainText().length()!=0){//非空
        datagram=ui->textsend->toPlainText().toLocal8Bit();//储存文本框内容
        ui->textsend->clear();//情况文本框
        for(int i=0;i<datagram.size();i++){
            char ch=datagram.at(i);
             QString tmpStr=QString::number(ch,16);
             ui->textsend->insertPlainText(tmpStr+" ");

        } }}
    if(checked==false){//切换到文本显示模式
if(ui->textsend->toPlainText().length()!=0){//非空
QStringList hexstr=ui->textsend->toPlainText().split(" ",QString::SkipEmptyParts);
for(int i=0;i<hexstr.size();i++){
    QString hexubstr=hexstr.at(i);
    datagram.append(ConvertHexStr(hexubstr));
}
ui->textsend->clear();
ui->textsend->setPlainText(datagram.data());
}}}

//复选框启动文件数据源的槽函数
void MainWindow::on_s_check_1_toggled(bool checked)
{
if(checked==true){
    QFileDialog  *qfd=new QFileDialog(this);
    //设置文件对话框
    qfd->setViewMode(QFileDialog::List);
    qfd->setFileMode(QFileDialog::AnyFile);
    qfd->setWindowTitle(tr("选择发送文件"));
    qfd->setNameFilter(tr("所有文件(*.*)"));
    //文件对话框用户选择文件
if(qfd->exec()/*模态显示*/==QDialog::Accepted){
    QStringList list=qfd->selectedFiles();
    curpath=list.at(0);
    curfile=new QFile(curpath);
    if(curfile->open(QFile::ReadOnly)==false){
        ui->s_check_1->setChecked(false);
       return  ;
    }
        ui->textreceive->setPlainText(tr("正在发送文件")+curpath+tr("\n"));
}
        //文件对话框用户选择取消
        else{
        ui->s_check_1->setChecked(false);
        return  ;
    }
        }
        if(checked==false){
         ui->textreceive->clear();
        //释放资源
        if(curfile){
        curfile->close();
        delete curfile;
       curfile = nullptr;  // 建议：将指针设置为空，避免悬空指针
         }}}

//复选框接收转向文件（把接收的数据写入指定文件的基础设置）
void MainWindow::on_r_check_1_toggled(bool checked)
{
    if(checked==true){
    QFileDialog *qfd=new QFileDialog(this);
    qfd->setViewMode(QFileDialog::List);
    qfd->setFileMode(QFileDialog::AnyFile);
    qfd->setWindowTitle(tr("建立接收文件"));
    qfd->setNameFilter(tr("所有文件(*.*)"));
    if(qfd->exec()==QDialog::Accepted){
        QStringList list=qfd->selectedFiles();
        curpath=list.at(0);
        curfile=new QFile( curpath);
        if(curfile->open(QFile::WriteOnly)==false){
            ui->r_check_1->setChecked(false);
                return ;}
          ui->textreceive->setPlainText(tr("接收数据保存到文件：\n") + curpath + "\n");
    }else{
                ui->r_check_1->setChecked(false);
                    return ;      }
    }else{
         ui->textreceive->clear();
         if(curfile){
             curfile->close();
             delete curfile;
            curfile = nullptr;
         }}}

//复选框循环发送的槽函数
void MainWindow::on_s_check_4_toggled(bool checked)
{
    if(checked == true){//启动循环发送
        // 如果已经存在定时器，先清理
        if(timer != nullptr){
            timer->stop();
            delete timer;
            timer = nullptr;
        }
        
        timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(toSendData()));
        int data = ui->lineEdit_3->text().toInt();
        if(data > 0){
            timer->setInterval(data);
        }else{//间隔时间无效
            ui->s_check_4->setChecked(false);
            delete timer;
            timer = nullptr;
            QMessageBox::information(this, tr("错误"), tr("间隔时间无效! 请输入大于0的数值。"));
        }
    }else{
        if(timer != nullptr){
            timer->stop();
            delete timer;
            timer = nullptr;
        }
        loopSending = false;
        ui->pushButton_4->setText(tr("发送"));
    }
}
/************************************************************************************************************************************/
//按钮
//保存数据(接收区)按钮槽函数
void MainWindow::on_pushButton_5_clicked(){
 QString path = QFileDialog::getSaveFileName(this, tr("保存接收区内容到文本文件"), tr(""), tr("文本文件(*.txt)"));
 QFile savefile(path);//使用栈对象
 if(savefile.open(QFile::WriteOnly| QIODevice::Append)/*不清除旧数据防止丢失*/==true){
     QTextStream out(&savefile);
    QString str= ui->textreceive->toPlainText();
     out<<str;
 }
savefile.close();
}
//载入数据按钮槽函数
void MainWindow::on_pushButton_3_clicked()
{
    QString path=QFileDialog::getOpenFileName(this,tr("载入文本文件到发送区"));
    QFile file(path);//这里使用栈对象
    //创建缓冲区数组
    QByteArray str;
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)==false){
       QMessageBox::information(this,tr("error"),tr("open file error!!!"));
       return ;
    }else{
        while(!file.atEnd()){
            str=file.readLine();
            ui->textsend->insertPlainText(str);
        }
    }
    file.close();
}
//清空发送区按钮槽函数
void MainWindow::on_pushButton_clicked()
{
    ui->textsend->clear();
}
//清空接收区按钮槽函数
void MainWindow::on_pushButton_6_clicked()
{
    ui->textreceive->clear();
}
//间隔秒数据用户输入槽函数
void MainWindow::on_lineEdit_3_editingFinished()
{
    if(timer != nullptr){
    int time=ui->lineEdit_3->text().toInt();
    if(time>0){
    timer->setInterval(ui->lineEdit_3->text().toInt());
    }
}
}
//发送按钮槽函数
void MainWindow::on_pushButton_4_clicked()
{
    //文件发送
    if(ui->s_check_1->checkState()){//检查是否为文件发送模式
        ui->pushButton_4->setText(tr("正在发送"));
        ui->pushButton_4->setEnabled(false);
         insertDateTimeInRcvDisp();//插入时间戳
        ui->textreceive->append(tr("开始发送"));
         toSendFile();//执行文件发送
         insertDateTimeInRcvDisp();
         ui->textreceive->append(tr("发送完成！"));
         ui->pushButton_4->setText(tr("发送完成"));
         ui->pushButton_4->setEnabled(true);
         return;
    }
    //文本发送
    if(ui->textsend->toPlainText().size()==0){
      QMessageBox::information(this, tr("提示"), tr("发送区为空，请输入内容。"));
        return ;
    }
    if(ui->s_check_4->checkState()){//判断是否为循环发送
        if(timer == nullptr){
            QMessageBox::information(this, tr("提示"), tr("请先设置发送间隔时间并勾选循环发送选项。"));
            return;
        }
        
        if(!loopSending) {
            ui->pushButton_4->setText(tr("停止发送"));
            timer->start();
            loopSending = true;
        } else {
            timer->stop();
            ui->pushButton_4->setText(tr("发送"));
            loopSending = false; 
        }
    }else{//为单次发送
         toSendData();
       if(ui->s_check_2->checkState()){//判断是否需要发送完自动清空
           ui->textsend->clear();
       }
    }
}
//网络连接按钮点击槽函数
void MainWindow::on_pushButton_2_clicked(bool checked)
{
    if(checked){//连接
        if(ui->left_1_1->currentIndex()==0){//UDP连接
            udpSocket = new QUdpSocket(this);
            //初始化信号和槽
           connect(udpSocket, SIGNAL(readyRead()), this, SLOT(udpDataReceived()));
           //初始化本地IP，端口
            lhAddr.setAddress(ui->left_1_2->text());
            lhPort=ui->left_1_3->text().toInt();
           //初始化远程连接端口
            rmtAddr.setAddress(ui->r_lineEdit_2->text());
            rmtPort=ui->r_lineEdit_3->text().toInt();
            //开始绑定
            bool result = udpSocket->bind(lhPort);
            if(!result){//绑定失败
                ui->pushButton_2->setChecked(0);
               QMessageBox::information(this, tr("错误"), tr("UDP绑定端口失败!"));
               return; 
            }
            // UDP连接成功
            ui->pushButton_2->setText(tr("断开连接"));
            ui->pushButton_4->setEnabled(true);
        }
        else if(ui->left_1_1->currentIndex()==1){//TCP服务器连接
            // 检查IP地址是否有效
            if(!lhAddr.setAddress(ui->left_1_2->text())){
                ui->pushButton_2->setChecked(0);
                QMessageBox::information(this, tr("错误"), tr("IP地址格式错误!"));
                return;
            }
            lhPort=ui->left_1_3->text().toInt();
            if(! slotTryCreateTcpServer()){
                ui->pushButton_2->setChecked(0);
                QMessageBox::information(this, tr("错误"), tr("尝试建立服务器失败! 请确认网络状态和端口。"));
                return ;
            }
            // TCP服务器连接成功
            ui->pushButton_2->setText(tr("断开连接"));
            ui->pushButton_4->setEnabled(true);
        }
        else if(ui->left_1_1->currentIndex()==2){//TCP客户端连接
            // 先清理之前的连接（如果存在）
            if(tcpClientSocket != nullptr){
                tcpClientSocket->disconnect();
                tcpClientSocket->disconnectFromHost();
                if(tcpClientSocket->state() != QAbstractSocket::UnconnectedState){
                    tcpClientSocket->waitForDisconnected(1000);
                }
                delete tcpClientSocket;
                tcpClientSocket = nullptr;
            }
            
            QString ip = ui->left_1_2->text();
            if(!rmtServerIP->setAddress(ip)){
                ui->pushButton_2->setChecked(0);
                QMessageBox::information(this, tr("错误"), tr("TCP服务器IP设置失败!"));
                return ;
            }
            
            // 验证端口号
            int port = ui->left_1_3->text().toInt();
            if(port <= 0 || port > 65535){
                ui->pushButton_2->setChecked(0);
                QMessageBox::information(this, tr("错误"), tr("端口号无效! 请输入1-65535之间的端口号。"));
                return ;
            }
            
            tcpClientSocket = new QTcpSocket(this);
            //初始化信号和槽
            connect(tcpClientSocket, SIGNAL(readyRead()), this, SLOT(tcpClientDataReceived()));
            tcpClientSocket->connectToHost(*rmtServerIP, port); //连接服务器
            if(!tcpClientSocket->waitForConnected(2000)) {
                // 连接失败时删除socket并显示详细错误信息
                QString errorMsg = tr("尝试连接服务器失败!\n");
                errorMsg += tr("错误: ") + tcpClientSocket->errorString();
                errorMsg += tr("\n服务器: ") + ip + ":" + QString::number(port);
                
                QMessageBox::information(this, tr("错误"), errorMsg);
                
                delete tcpClientSocket;
                tcpClientSocket = nullptr;
                
                ui->r_lineEdit_2->setText(QString::number(0, 10));
                ui->pushButton_2->setChecked(0);
                return ;
            }
            ui->r_lineEdit_2->setText(QString::number(tcpClientSocket->localPort(), 10));
            // TCP客户端连接成功
            ui->pushButton_2->setText(tr("断开连接"));
            ui->pushButton_4->setEnabled(true);
        }
    }else {//断开连接状态
        if(ui->left_1_1->currentIndex()==0){
            if(udpSocket != nullptr){
                udpSocket->close();
                delete udpSocket;
                udpSocket = nullptr;
            }
        }
        if(ui->left_1_1->currentIndex()==1){
             slotDeleteTcpServer();
        }
        if(ui->left_1_1->currentIndex()==2){
            if(tcpClientSocket != nullptr){
                tcpClientSocket->disconnect();
                tcpClientSocket->disconnectFromHost();
                if(tcpClientSocket->state() != QAbstractSocket::UnconnectedState){
                    tcpClientSocket->waitForDisconnected(1000);
                }
                delete tcpClientSocket;
                tcpClientSocket = nullptr;
            }
        }
        ui->pushButton_2->setText(tr("连接网络"));
        ui->pushButton_4->setEnabled(false);
    }
}

/************************************************************************************************************************************/
//自定义信号槽函数
//客户端
//移除客户端连接槽函数， 移除客户端信号void signal_removeClientLink(QString, int)，对应的槽函数
void MainWindow::removeClientLink(QString clientAddrPort, int socketDescriptor){
    if(socketDescriptor==-1){
        return ;
    }
    if(TcpClientLinkCnt<=1){
        tcpClientSocketDescriptorList.clear();
        ui->r_lineEdit_1->clear();//连接对象框清除
    }else{
        TcpClientLinkCnt--;
        //在添加客户端槽函数时，一定要保证客户端套接字链表和r_lineEdit_1的插入顺序一致，才能公用索引
        int index=ui->r_lineEdit_1->findText(clientAddrPort);
        ui->r_lineEdit_1->removeItem(index);
        tcpClientSocketDescriptorList.removeAt(index);
    }
}
//添加客户端连接槽函数，添加客户端连接信号void addClientLink(QString, int)，对应的槽函数
void MainWindow::addClientLink(QString clientAddrPort, int socketDescriptor){
    if(TcpClientLinkCnt==0){
        tcpClientSocketDescriptorList.clear();
        //保证列表下拉框进行对应
        tcpClientSocketDescriptorList.append(0);
         ui->r_lineEdit_1->addItem(tr("全部连接"));
    }
    TcpClientLinkCnt++;
    //保证顺序
    tcpClientSocketDescriptorList.append(socketDescriptor);
     ui->r_lineEdit_1->addItem(clientAddrPort);
}
//TCP客户端接收服务器数据槽函数，
void MainWindow::tcpClientDataReceived(){
    if(tcpClientSocket == nullptr){
        return;
    }
    while (tcpClientSocket->bytesAvailable()>0) {
    QByteArray datagram;
    datagram.resize(tcpClientSocket->bytesAvailable());
    tcpClientSocket->read(datagram.data(), datagram.size());
    //检查是否需要保存数据到文件
    if(ui->r_check_1->checkState()){
        char*buf;
        buf=datagram.data();
        if(curfile!=0){
            curfile->write(buf,datagram.size());
        }
    }else{//普通接收模式
        if(!ui->r_check_4->checkState()){//判断是否勾选暂停接收数据显示
        QString rcvMsg = QString::fromUtf8(datagram, datagram.size());
        QString tmpIPPort=ui->r_lineEdit_2->text()+":"+ui->r_lineEdit_3->text();
        if(CurIPPort!=tmpIPPort){
            CurIPPort=tmpIPPort;
            //判断文本框是否有数据
        if(ui->textreceive->toPlainText().size() != 0) {
            ui->textreceive->insertPlainText("\n");}
          ui->textreceive->insertPlainText(tr("【数据来自")+CurIPPort+tr("】\n"));
        }
        //判断是否勾选了显示接收日期
        if(ui->r_check_2->checkState()){
             insertDateTimeInRcvDisp();//插入时间戳（自定义函数）
        }
         if(ui->r_check_3->checkState()){//判断是否勾选了16进制显示模式
             for(int i=0;i<datagram.size();i++){
                 char ch=datagram.at(i);
                 QString tmpStr = "";
                 tmpStr.append(toHex[(ch & 0xf0)/16]);
                 tmpStr.append(toHex[ch & 0x0f]);
                 tmpStr.append(" ");
                  ui->textreceive->insertPlainText(tmpStr);}
                }else{//文本显示
                 ui->textreceive->insertPlainText(rcvMsg);}
        }}
    }}
//服务器
//TCP服务器接收客户端数据槽函数，发送客户端数据信号，并给服务器槽函数传递数据 void updateTcpServer(char*, int, int)，对应的槽函数
void MainWindow::tcpServerDataReceived(char *msg, int length, int socketDescriptorEx){
    //检查是否需要保存数据到文件
    if(ui->r_check_1->checkState()){
        if(curfile!=0){
            curfile->write(msg,length);
        }
    }else{
        if(!ui->r_check_4->checkState()){//判断是否勾选暂停接收数据显示
            int idx= tcpClientSocketDescriptorList.indexOf( socketDescriptorEx);
            if(idx == -1) {
                return;
            }
            QString tmpIPPort=ui->r_lineEdit_1->itemText(idx);
            if(CurIPPort!=tmpIPPort){
                CurIPPort=tmpIPPort;
                //判断接收区是否已有内容
                if(ui->textreceive->toPlainText().size()!=0){
                    ui->textreceive->insertPlainText("\n");//插入换行符
                }
                ui->textreceive->insertPlainText(tr("【数据来自")+CurIPPort+tr("】\n"));
            }
            //判断是否勾选了显示接收日期
            if(ui->r_check_2->checkState()){
                 insertDateTimeInRcvDisp();//插入时间戳（自定义函数）
            }
            if(ui->r_check_3->checkState()){//判断是否勾选了16进制显示模式
                for(int i=0;i<length;i++){
                    char ch=*(msg+i);//通过地址解引用来取数据
                    QString tmpStr = "";
                    tmpStr.append(toHex[(ch & 0xf0)/16]);
                    tmpStr.append(toHex[ch & 0x0f]);
                    tmpStr.append(" ");
                     ui->textreceive->insertPlainText(tmpStr);
                }

            }else{//字符串显示
                ui->textreceive->insertPlainText(msg);}
        }
    }

    if(ui->r_check_5->checkState()){//判断是否勾选了群聊服务模式
        if(ui->r_check_6->checkState()){//判断是否勾选了群聊回显开关
        emit sendDataToClient(msg, length, 0, 0);//转发给所有客户端（包括自己）
        }else{
             emit sendDataToClient(msg, length, 0, socketDescriptorEx);//非回显模式不转发自己
        }
    }
}
//UDP
//UDP数据接收槽函数,Socket的readyRead()信号槽函数
void MainWindow::udpDataReceived(){
    QHostAddress address;
    quint16 port;
    QString tmpIPPort = "";

     while(udpSocket->hasPendingDatagrams()){
    QByteArray datagram;
    datagram.resize(udpSocket->pendingDatagramSize());
    udpSocket->readDatagram(datagram.data(), datagram.size(), &address, &port);
    if(ui->r_check_1->checkState()) {//判断是否勾选了发送数据到文件按
        char *buf;
        buf = datagram.data();
        if(curfile != 0) {
         curfile->write(buf, datagram.size());}
    }else{//发送数据到接收框
        if(!ui->r_check_4->checkState()){//判断是否勾选暂停接收数据显示
             tmpIPPort = address.toString() + ":" + QString::number(port, 10);
              QString rcvMsg = QString::fromUtf8(datagram, datagram.size());
              if(CurIPPort!=tmpIPPort){
                  CurIPPort=tmpIPPort;
                  //判断接收区是否已有内容
                  if(ui->textreceive->toPlainText().size()!=0){
                      ui->textreceive->insertPlainText("\n");//插入换行符
                  }
                  ui->textreceive->insertPlainText(tr("【数据来自")+CurIPPort+tr("】\n"));
              }
              //判断是否勾选了显示接收日期
              if(ui->r_check_2->checkState()){
                   insertDateTimeInRcvDisp();//插入时间戳（自定义函数）
              }
              if(ui->r_check_3->checkState()){//判断是否勾选了16进制显示模式
                  for(int i=0;i<datagram.size();i++){
                      char ch=datagram.at(i);//通过地址解引用来取数据
                      QString tmpStr = "";
                      tmpStr.append(toHex[(ch & 0xf0)/16]);
                      tmpStr.append(toHex[ch & 0x0f]);
                      tmpStr.append(" ");
                       ui->textreceive->insertPlainText(tmpStr);
                  }

              }else{//字符串显示
                  ui->textreceive->insertPlainText(rcvMsg);}}
      }}}

/************************************************************************************************************************************/
//文本输入框
//IP输入框
void MainWindow::on_r_lineEdit_2_textChanged(const QString &arg1)
{
    rmtAddr.setAddress(arg1);  // 设置远程IP地址
}
//端口输入框
void MainWindow::on_r_lineEdit_3_textChanged(const QString &arg1)
{
    rmtPort=arg1.toInt();
}



