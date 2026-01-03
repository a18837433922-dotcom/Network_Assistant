# TCP协议服务器实现逻辑

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

## 三、项目TCP服务器模块（TcpServer.cpp/TcpServer.h）实现逻辑

### 1. TcpServer类概述

TcpServer类封装了TCP服务器监听套接字和通信套接字的方法，用于实现TCP服务器的便捷管理。

**类变量：**

```cpp
QList<QTcpSocket*> tcpClientSocketList;  // 存放已连接客户端的服务器端通信套接字列表
```

**信号：**

```cpp
signal_removeClientLink(QString, int);  // 移除客户端信号
addClientLink(QString, int);             // 添加客户端连接信号
updateTcpServer(char*, int, int);        // 客户端数据到达信号
```

### 2. 核心方法实现

#### disconnectclient() - 客户端断开槽函数

**功能：** 处理客户端断开连接，维护套接字列表

**实现逻辑：**
1. 遍历 `tcpClientSocketList`，查找状态为 `UnconnectedState` 的服务器通信套接字
2. 获取该套接字连接的客户端端口和地址
3. 发送 `signal_removeClientLink` 信号通知 MainWindow 移除客户端
4. 从 `tcpClientSocketList` 中移除该套接字

#### acceptnewclient() - 接收新客户端槽函数

**功能：** 接受新客户端连接，初始化通信套接字

**实现逻辑：**
1. 调用 `nextPendingConnection()` 生成服务器通信套接字
2. 记录该套接字的底层文件描述符（`socketDescriptor()`）
3. 初始化信号槽连接：
   - `disconnected()` → `disconnectclient()` 槽（处理断开）
   - `disconnected()` → `deleteLater()` 槽（自动释放内存）
   - `readyRead()` → `clientDataReceived()` 槽（接收数据）
4. 将服务器通信套接字添加到 `tcpClientSocketList` 列表
5. 发送 `addClientLink` 信号通知 MainWindow 添加客户端连接

#### clientDataReceived() - 服务器接收客户端数据槽函数

**功能：** 接收客户端发送的数据并转发给 MainWindow

**实现逻辑：**
1. 遍历 `tcpClientSocketList`，查找有数据可读的服务器通信套接字
2. 创建缓冲区数组（`QByteArray`）
3. 从服务器通信套接字的内核缓冲区读取数据到自定义缓冲区
4. 发送 `updateTcpServer` 信号，将数据传递给 MainWindow 的 `tcpServerDataReceived()` 槽函数

#### sendDataToClient() - 服务器发送数据到客户端

**功能：** 向指定客户端或所有客户端发送数据

**实现逻辑：**
1. 遍历 `tcpClientSocketList` 列表
2. 根据 `socketDescriptor` 参数判断发送模式：
   - `socketDescriptor == 0`：广播模式，发送给所有客户端（可排除黑名单客户端）
   - `socketDescriptor != 0`：单播模式，只发送给指定客户端
3. 将自定义缓冲区中的数据写入目标服务器通信套接字的内核发送缓冲区

## 四、MainWindow类中TCP服务器实现

MainWindow类通过封装TcpServer对象来实现TCP服务器的创建、管理和资源释放。TcpServer继承自`QTcpServer`类，通过继承获得`QTcpServer`封装的监听套接字功能。TcpServer类内部维护`tcpClientSocketList`成员变量，用于管理所有已连接的客户端通信套接字对象。


### 1. 成员变量定义

在MainWindow成员类的Public区定义`TcpServer`指针成员变量（自定义的TcpServer类，继承自`QTcpServer`），并在构造函数处将指针初始化为nullptr。

**相关成员变量：**

```cpp
TcpServer *mtcpServer;                    // TCP服务器对象指针
QList<int> tcpClientSocketDescriptorList; // 客户端套接字描述符列表
int TcpClientLinkCnt;                     // 客户端连接计数
QHostAddress lhAddr;                      // 本地地址
int lhPort;                               // 本地端口
```

### 2. 核心方法实现

#### slotTryCreateTcpServer() - 创建TCP服务器

**功能：** 创建并初始化TCP服务器实例

**实现逻辑：**
1. 检查`mtcpServer`是否为空，不为空则先调用`slotDeleteTcpServer()`释放旧资源
2. 使用`new TcpServer(this)`创建服务器对象，将MainWindow作为父对象
3. 调用`mtcpServer->listen(lhAddr, lhPort)`开始监听指定地址和端口
4. 检查`listen()`返回值，监听失败时删除对象并返回`false`
5. 监听成功后，连接TcpServer与MainWindow之间的自定义信号槽：
   - `updateTcpServer` → `tcpServerDataReceived`（接收客户端数据）
   - `sendDataToClient` → `sendDataToClient`（向客户端发送数据）
   - `addClientLink` → `addClientLink`（添加客户端连接）
   - `signal_removeClientLink` → `removeClientLink`（移除客户端连接）

#### slotDeleteTcpServer() - 删除TCP服务器

**功能：** 释放TCP服务器资源

**实现逻辑：**
1. 判断`mtcpServer`是否为`nullptr`，为空则直接返回
2. 调用`mtcpServer->disconnect()`断开TcpServer与MainWindow之间的自定义信号槽连接
3. 调用`mtcpServer->close()`停止监听，关闭底层监听套接字（会自动关闭所有客户端连接）
4. 客户端套接字会通过`disconnected()`信号自动调用`deleteLater()`释放内存
5. 使用`delete mtcpServer`删除TcpServer对象，释放C++对象内存
6. 将`mtcpServer`设为`nullptr`，防止悬空指针

#### 连接按钮槽函数中的服务器判断分支

**功能：** 在连接按钮槽函数中添加TCP服务器模式的处理逻辑

**实现逻辑：**
1. 检查用户输入的IP地址格式是否有效
2. 从UI控件获取并设置`lhAddr`（本地地址）和`lhPort`（本地端口）
3. 调用`slotTryCreateTcpServer()`创建TcpServer对象并启动监听
4. 通过`slotTryCreateTcpServer()`返回值判断服务器是否创建成功
5. 监听失败时，恢复UI控件初始状态，提示用户错误信息并返回
6. 创建成功后，更新UI控件状态（如按钮文本、连接状态显示等）

### 3. 文件发送功能

在`toSendFile()`（发送文件的核心函数）中添加TCP服务器判断分支实现TCP模式下的文件发送。

**核心逻辑：**
- 以二进制模式打开文件（`QFile::ReadOnly`）
- 利用`curfile`当前文件指针
- 获取连接对象下拉框索引（`ui->r_lineEdit_1->currentIndex()`）
- **发送所有客户端（索引为0）：**
  - 在循环中利用`read()`函数读取数据到缓冲区`buf`中（单次读取1024字节）
  - 调用`emit sendDataToClient(buf, rdlength, 0, 0)`发送给所有客户端
  - 调用`QCoreApplication::processEvents()`处理事件，保持界面响应
- **发送给指定客户端（索引>0）：**
  - 在循环中利用`read()`函数读取数据到缓冲区`buf`中
  - 从`tcpClientSocketDescriptorList`获取目标客户端套接字描述符
  - 调用`emit sendDataToClient(buf, rdlength, socketDescriptor, 0)`发送给指定客户端
  - 调用`QCoreApplication::processEvents()`处理事件
- **校验返回值**：在`sendDataToClient()`函数中检查`write()`返回值是否等于发送的数据长度，判断发送是否成功

实际上实现的操作就是：读数据 → 选择目标客户端 → 发送数据 → 校验返回值。

### 4. 文本数据发送功能

在`toSendData()`（文本数据发送功能）中添加TCP服务器判断分支实现TCP模式下的文本数据发送。

**核心逻辑：**
- 处理数据格式（16进制或字符串）
- 获取连接对象下拉框索引
- **发送所有客户端（索引为0）：**
  - 调用`emit sendDataToClient((char *)datagram.data(), datagram.size(), 0, 0)`发送给所有客户端
- **发送给指定客户端（索引>0）：**
  - 从`tcpClientSocketDescriptorList`获取目标客户端套接字描述符
  - 调用`emit sendDataToClient((char *)datagram.data(), datagram.size(), socketDescriptor, 0)`发送给指定客户端
- **校验返回值**：在`sendDataToClient()`函数中检查`write()`返回值，判断发送是否成功

实际上实现的操作就是：处理数据 → 选择目标客户端 → 发送数据 → 校验返回值。

### 5. TCP服务器数据接收功能

MainWindow类的`tcpServerDataReceived()`槽函数实现。当TcpServer接收到客户端数据时，会发送`updateTcpServer`信号，触发MainWindow的该槽函数处理数据。

**核心逻辑：**
1. **接收数据**：通过信号槽机制接收TcpServer发送的`updateTcpServer`信号，获取客户端数据和套接字描述符
2. **数据处理**：在`tcpServerDataReceived()`中对接收的复选框进行判断，针对不同的复选框对接收到的数据进行不同的操作（保存到文件、显示到接收区、群聊转发等）



### 6. 群聊功能

在`tcpServerDataReceived()`中实现群聊服务模式：

**核心逻辑：**
- 检查是否勾选了群聊服务模式（`r_check_5`）
- **回显模式（`r_check_6`勾选）：**
  - 调用`emit sendDataToClient(msg, length, 0, 0)`转发给所有客户端（包括发送者）
- **非回显模式（`r_check_6`未勾选）：**
  - 调用`emit sendDataToClient(msg, length, 0, socketDescriptorEx)`转发给所有客户端（排除发送者）

### 7. 资源释放

在按钮的资源释放部分插入TCP服务器判断：

**实现逻辑：**
- 调用`mtcpServer->disconnect()`断开所有信号槽连接
- 调用`mtcpServer->close()`停止监听，关闭服务器（释放底层socket资源）
- 使用`delete mtcpServer`删除`TcpServer`对象，释放C++对象内存（防止内存泄漏）
- 将指针设为`nullptr`，防止悬空指针
- 清空客户端连接列表（`tcpClientSocketList`、`tcpClientSocketDescriptorList`）

**注意：** 客户端套接字会在断开时自动调用`deleteLater()`释放内存，无需手动管理。
