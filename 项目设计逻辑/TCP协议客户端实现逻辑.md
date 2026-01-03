# TCP协议客户端实现逻辑

## 一、TCP协议概述

TCP（传输控制协议）是互联网协议套件中的核心协议之一，位于传输层。它提供了一种可靠的、面向连接的、基于字节流的数据传输服务。TCP 的主要特点是确保数据在传输过程中不丢失、不重复，并且按顺序到达。

### TCP的特点

- **面向连接**：在数据传输前需要建立连接，传输结束后需要关闭连接。
- **可靠性**：通过确认机制和重传机制，确保数据不丢失、不重复。通过校验和检查数据完整性。
- **流量控制**：通过滑动窗口机制，动态调整发送速率。
- **拥塞控制**：通过慢启动、拥塞避免等算法，避免网络拥塞。
- **有序性**：数据按发送顺序到达接收方。

### 总结

总结来说，TCP 是一种可靠的、面向连接的协议，通过三次握手建立连接、四次挥手关闭连接，并在数据传输过程中使用确认、重传、流量控制和拥塞控制等机制来保证数据的可靠传输。

## 二、Qt TCP接口

Qt为我们提供了封装完成的TCP接口来方便我们实现TCP的传输，核心类是 `QTcpServer`（服务器）和 `QTcpSocket`（套接字）。

- **QTcpServer**：用于创建TCP服务器，监听客户端连接请求。
- **QTcpSocket**：用于TCP通信的套接字，服务器端和客户端都使用。

## 三、TCP客户端实现

### 1. 成员变量定义

```cpp
QList<int> tcpClientSocketDescriptorList;  // 客户端套接字描述符列表
QTcpSocket *tcpClientSocket;               // 客户端套接字指针
QHostAddress *rmtServerIP;                 // 远程服务器IP地址指针（用于连接）
QHostAddress rmtAddr;                      // 远程服务器地址
int rmtPort;                               // 远程服务器端口
```

### 2. 网络连接实现逻辑

- **清理旧连接**：检查旧客户端套接字资源是否已释放，如果未释放则先释放旧资源：
  - 调用 `tcpClientSocket->disconnect()` 断开所有信号槽连接
  - 调用 `tcpClientSocket->disconnectFromHost()` 断开网络连接
  - 检查套接字状态，如果未完全断开（`state() != QAbstractSocket::UnconnectedState`），则调用 `waitForDisconnected(1000)` 等待最多1秒
  - 使用 `delete tcpClientSocket` 删除对象，将指针置为 `nullptr`
- **读取并验证连接参数**：
  - 从界面控件 `ui->left_1_2` 读取IP地址，从 `ui->left_1_3` 读取端口号
  - 使用 `rmtServerIP->setAddress(ip)` 验证IP地址格式
  - 验证端口号范围（1-65535）
  - 验证失败则显示错误信息并返回
- **创建套接字对象**：使用 `new QTcpSocket(this)` 实例化TCP客户端套接字对象
- **连接信号槽**：连接 `readyRead()` 信号到 `tcpClientDataReceived()` 槽函数，用于接收数据
- **连接服务器**：调用 `tcpClientSocket->connectToHost(*rmtServerIP, port)` 连接服务器
- **连接等待与验证**：
  - 使用 `waitForConnected(2000)` 等待最多2秒
  - 如果连接失败则删除套接字对象，显示详细错误信息（包括错误字符串和服务器地址），重置界面状态并返回
- **更新界面**：连接成功后更新UI显示

### 3. 发送数据实现逻辑

- **空指针检查**：检查 `tcpClientSocket != nullptr`
- **发送数据**：使用 `tcpClientSocket->write(datagram.data(), datagram.size())` 方法将 `QByteArray` 类型的数据缓冲区写入套接字缓冲区

### 4. 发送文件实现逻辑

- **空指针检查**：检查 `tcpClientSocket != nullptr`
- **循环读取文件**：使用 `while(!curfile->atEnd())` 循环读取文件数据
- **读取文件块**：每次读取1024字节到缓冲区数组 `buf`
- **写入套接字**：将缓冲区数据写入客户端套接字缓冲区：`tcpClientSocket->write(buf, rdlength)`
- **保持界面响应**：每次写入后调用 `QCoreApplication::processEvents(QEventLoop::AllEvents, 1)` 处理事件，保持界面响应

### 5. 断开连接实现逻辑

- **空指针检查**：检查 `tcpClientSocket != nullptr`
- **断开连接**：
  - 调用 `tcpClientSocket->disconnect()` 断开所有信号槽连接
  - 调用 `tcpClientSocket->disconnectFromHost()` 断开网络连接
  - 检查套接字状态，如果未完全断开（`state() != QAbstractSocket::UnconnectedState`），则调用 `waitForDisconnected(1000)` 等待最多1秒
- **释放资源**：使用 `delete tcpClientSocket` 删除对象，将指针置为 `nullptr`
- **更新界面**：断开连接后更新UI显示

### 6. 接收数据实现逻辑

- **空指针检查**：检查 `tcpClientSocket != nullptr`，如果为空则直接返回
- **循环读取数据**：使用 `while (tcpClientSocket->bytesAvailable() > 0)` 循环读取所有可用数据，每次循环迭代执行以下步骤：
  - **创建并初始化数据容器**：创建 `QByteArray datagram` 对象，并根据当前可用数据大小调整其大小：`datagram.resize(tcpClientSocket->bytesAvailable())`
  - **读取数据**：使用 `tcpClientSocket->read(datagram.data(), datagram.size())` 从套接字缓冲区读取数据到 `QByteArray`
  - **数据处理**：根据接收数据复选框选择情况对数据进行处理（保存到文件或显示在界面，具体复选框功能说明见相关文档）