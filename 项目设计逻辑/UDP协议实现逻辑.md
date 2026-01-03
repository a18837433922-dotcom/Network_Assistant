# UDP协议实现逻辑

## 一、UDP协议概述

UDP是一种简单的、无连接的传输层协议，用于在网络中传输数据。UDP 协议本身不提供可靠性、顺序性和流量控制，但它具有低延迟和高效的特点，适合对实时性要求较高的应用。UDP协议本身不提供确认和应答机制，数据包直接发送，无需等待接收方准备就绪。

### UDP的特点

- **无连接**：不需要建立连接，直接发送数据。
- **不可靠性**：不保证数据包的到达、顺序和完整性。
- **低延迟**：由于不需要建立连接和确认，传输延迟较低。
- **高效性**：头部开销小，适合传输小数据包。
- **单包大小限制**：受MTU（最大传输单元）限制，IPv4环境下单个UDP数据包有效载荷约1472字节（MTU 1500 - IP头20 - UDP头8），这是文件发送的关键约束。
- **支持广播和多播**：可以向多个接收方发送数据包。

### 总结

UDP 是一种简单的、无连接的传输层协议，通过低延迟和高效的数据传输适合对实时性要求高的应用。它广泛应用于实时应用、广播和多播等场景，但需要注意其不可靠性和无连接管理的缺点。

## 二、Qt UDP接口

Qt为我们提供了封装完成的UDP接口来方便我们实现UDP的传输，核心是 `QUdpSocket` 类。

## 三、项目UDP通信实现逻辑

### 1. 成员变量定义

在MainWindow成员类的Public区定义`QUdpSocket`指针成员变量，并在构造函数处将指针初始化为nullptr。

```cpp
QUdpSocket *udpSocket;  // UDP套接字指针
QHostAddress rmtAddr;   // 远程地址
int rmtPort;            // 远程端口
```

### 2. Socket初始化

在承载网络功能的按钮控件中，添加UDP判断分支：
- 初始化UDP所用的本地IP、端口
- 初始化目标地址和端口
- 连接`readyRead()`信号到槽函数`udpDataReceived()`
- 调用`udpSocket->bind()`绑定端口：绑定`QHostAddress::Any`（对应0.0.0.0）表示监听所有网络接口，或绑定指定IP地址

### 3. 文件发送功能

在`toSendFile()`（发送文件的核心函数）中添加UDP判断分支实现UDP模式下的文件发送。

**核心逻辑：**
- 以二进制模式打开文件（`QFile::ReadOnly`）
- 利用`curfile`当前文件指针
- 在循环中利用`read()`函数读取数据到缓冲区`buf`中（单次读取建议≤1472字节，受MTU限制）
- 调用`udpSocket->writeDatagram(buf, rdlength, rmtAddr, rmtPort)`（核心代码）发送到目标地址和端口
- **校验返回值**：检查`writeDatagram()`返回值是否等于发送的数据长度，判断发送是否成功

实际上实现的操作就是：读数据 → 发送数据 → 校验返回值。

### 4. 文本数据发送功能

在`toSendData()`（文本数据发送功能）中添加UDP判断分支实现UDP模式下的文本数据发送。

**核心逻辑：**
- 利用`udpSocket->writeDatagram(data, len, rmtAddr, rmtPort)`（核心代码）发送到目标地址和端口
- **校验返回值**：检查`writeDatagram()`返回值是否等于发送的数据长度，判断发送是否成功

实际上实现的操作就是：发送数据 → 校验返回值。

### 5. UDP数据接收功能

`udpSocket`的`readyRead()`信号的槽函数实现。当UDP Socket发送`readyRead()`信号时，调用该方法读数据。

**核心逻辑：**
- **循环读取所有待处理数据包**：使用`while(udpSocket->hasPendingDatagrams())`循环，确保读取所有到达的数据包，避免漏读
- **调整缓冲区大小**：先调用`udpSocket->pendingDatagramSize()`获取数据包大小，然后调整`datagram`缓冲区大小
- **读取数据包**：调用`udpSocket->readDatagram(datagram.data(), datagram.size(), &address, &port)`把UDP数据包里的数据、端口、IP地址信息读取出来并储存在变量中
- **校验返回值**：检查`readDatagram()`返回值是否大于0，判断读取是否成功
- **数据处理**：对接收的复选框进行判断，针对不同的复选框对接收到的数据进行不同的操作

### 6. 资源释放

在按钮的资源释放部分插入UDP判断：
- 调用`QUdpSocket`类的`close()`方法释放底层socket资源（解除端口绑定、释放缓冲区、释放内核数据结构）
- 使用`delete udpSocket`删除`QUdpSocket`对象，释放C++对象内存（防止内存泄漏）
- 将指针设为`nullptr`，防止悬空指针


