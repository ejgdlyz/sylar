# sylar： C++ 高性能服务器框架
## 简介
根据 [sylar: C++ 高性能分布式服务器框架](https://github.com/sylar-yin/sylar) 教程内容并参考[sylar-from-scratch Public](https://github.com/zhongluqiang/sylar-from-scratch) 进行实现。

## 开发环境
    Ubuntu  11.4.0
    gcc     11.04 
    cmake   3.22.1

## 项目目录
- bin -------------- 可执行文件输出路径
- build ------------ 中间文件路径
- cmake ------------ cmake 函数文件夹
- example ---------- 回声服务器案例
- lib -------------- 库的输出路径
- samples----------- 简单的 http server 实现
- sylar ------------ 源代码路径
- template --------- 项目模板文件路径
- tests ------------ 测试代码路径, 提供每个模块的简单测试案例
- CMakeLists.txt --- cmake 定义文件 
- generate.sh ------ 创建新项目脚本
- Makefile

## 基础篇
### 日志系统 (log.h)
支持流式日志风格和格式化风格写日志，支持日志格式自定义、日志级别、多日志分离等功能。

流式日志使用：
```cpp
SYLAR_LOG_INFO(g_logger) << "hello sylar log!";
2024-03-05 15:53:18     9875    UNKNOW  0       [INFO]  [root]  tests/test_logger.cc:54 hello sylar log!
```

格式化日志使用：
```cpp
SYLAR_LOG_FMT_INFO(g_logger, "%s", "hello sylar log!"); 
2024-03-05 15:53:18     9875    UNKNOW  0       [DEBUG] [root]  tests/test_logger.cc:55 hello sylar log!
```
日志支持自由配置日期时间、累计运行的毫秒数、线程 id、线程名称、协程 id、日志级别、日志名称、文件名、行号。

日志相关的类：
- `LogEvent`: 日志事件。用于保存日志现场，如日志器名称、时间、线程/协程 id、文件名/行号，以及日志消息内容。
- `LogFormatter`: 日志格式器。其构造函数接收一个模板字符串，根据该字符串定义的模板项对一个日志事件进行格式化，提供 `format()` 方法对 `LogEvent` 对象进行格式化并返回对应的字符串或者流。
- `LogAppender`: 日志输出地。用于将日志输出到不同的目的地，如终端和文件。`LogAppender` 内部包含一个 `LogFormatter` 成员，提供 `log()` 方法将 `LogEvent` 对象输出到不同的目的地。该类为虚类，通过继承的方式可派生出不同的 `LogAppender`，目前默认支持 `StdoutLogAppender` 和 `FileLogAppender`，分别用于将日志事件输出到终端和文件。
- `Logger`: 日志器。用于写日志，包含日志名称、日志级别，`LogAppender` 集合等属性。一个日志事件的日志级别高于日志器本身的日志级别时才会启动 `LogAppender` 进行输出。日志器默认提供一个 `LogFormatter` 但是不提供默认的 `LogAppender`，需要用户手动添加。
- `LoggerManager`: 日志管理器，单例模式。该类包含全部的日志器集合，并且提供工厂方法，用于创建或获取日志器。`LogManager` 初始化时默认提供一个 root 日志器，以为日志模块提供一个初始可用的日志器。

### 配置系统 (config.h)
概述
- 配置原则 ： 约定优于配置
- 定义即可使用。
- 支持变更通知功能。
- 使用 YAML 文件作为配置内容，配置内容不区分大小写。
- 支持级别格式的数据类型，支持 STL 容器 (vector/list/set/map等)。
- 支持自定义类型（需要实现序列化和反序列化）

使用案例
```cpp
static sylar::ConfigVar<int>::ptr g_tcp_connect_timeout = 
   sylar::Config::Lookup("tcp.connect.timeout", 5000, "tcp connect timeout");
```
该案例定义了一个 TCP 连接超时参数，可以直接使用 `g_tcp_connect_timeout->getValue()` 获取参数的值。当配置文件被加载，该值自动更新（并触发对应的更新回调函数），对应的 YAML 文件如下：
```yaml
tcp:
    connect:
        timeout: 10000
```
配置系统相关的类：
- `ConfigVarBase`: 配置参数基类。
  
  定义了配置参数的基本属性和方法，如 名称、描述、纯虚函数 `fromString()` 和 `toString()` 等。配置参数的类型由继承类实现。
- `ConfigVar`: 配置参数类，继承 `ConfigVarBase`，模板类。
  
  该类包含三个模板参数，分别为类型 T、FromStr 和 ToStr。后两个参数是仿函数，分别用于将字符串转为类型 T 和 将类型 T 转为字符串。FromStr 和 ToStr 通过一个类型模板类和不同的偏特化类来实现不同类型的序列化与反序列化。

  `ConfigVar` 包含一个 T 类型的配置参数和一个变更回调函数组。其 `toString()` 和 `fromString()` 方法，分别使用模板参数 `ToStr` 和 `FromStr` 进行实现。
  `ConfigVar`  还提供了 `setValue()` 和 `getValue()` 方法用于设置或获取参数值，而 `setValue()` 方法会触发回调函数组。`addListener()` 和 `delListener()` 分别用于添加或删除回调函数。

- `Config`: `ConfigVar` 管理类，负责管理全部的 `ConfigVar` 对象。
  
  `Config` 提供 `Lookup()` 方法，根据参数名称查找配置参数。该方法支持为配置参数的提供默认值和描述信息，以在未找到目标配置时，可以根据传入的参数值创建新的配置项，从而保证配置模块定义即可用的特性。

  `Config` 提供 `loadFromYaml()` 和 `loadFromConfDir()` 方法，分别用于从 YAML 节点或从命令行 -c 选项执行的配置文件路径中加载配置。

  `Config` 的全部成员变量和方法都是 static 类型，以保证全局只有一个实例。

### 线程模块 (thread.h)
线程模块封装了 pthread 的一些常用功能，实现了 Thread、Semaphore、Mutex、RWMutex、SpinLock 等实例。

线程模块相关的类：
- Thread: 线程类。
   
   构造函数传入线程入口函数和线程名称。线程的入口函数类型为 void()，如果带参数，需要使用 std::bind() 进行绑定。

   线程类保证在构造完成之后线程函数一定处于运行状态，这是通过一个信号量来实现的。
- 线程同步类（见 mutex.h）：
  - `Semaphore`： 计数信号量，基于 sem_t 实现。
  - `Mutex`：互斥锁，基于 pthread_mutex_t 实现。
  - `RWMutex` ：读写锁，基于 pthread_rwlock_t 实现。
  - `Spinlock` ：自旋锁，基于 pthread_spinlock_t 实现。
  - `CASLock` ： 原子锁，基于 std::atomic_flag 实现。

### 协程和协程调度
#### 协程模块 (fiber.h)
基于 ucontext_t 实现非对称协程，即子协程只能和线程的主协程进行切换，而不能和另一个子协程进行切换。

协程模块相关的类和方法：
- `Fiber`: 协程类。
  
  定义了协程的基本属性，包括协程 id、协程栈大小、协程状态、协程上下文、协程入口函数等。此外，对于每个线程，还定义了两个线程局部变量 t_fiber 和 t_threadFiber 分别用来保存线程内的当前运行的协程和线程主协程的上下文信息。

- `Fiber` 构造函数：无参构造和有参构造。
  
  无参构造函数用于创建当前线程中的主协程对象，以及与 t_fiber 和 t_threadFiber 初始化。有参构造函数用于创建子协程，初始化子协程的 ucontext_t 上下文、协程入口函数以及协程栈大小。

- `Fiber` 原语实现：swapIn() 和 swapOut()。
  
  swapIn() 表示切换到目标协程执行，而 swapOut() 表示将当前协程 yield 并切换到线程主协程执行。
  

#### 协程调度模块 (scheduler.h)
协程调度器管理协程的调度，内部实现为一个线程池和一个协程任务队列。支持协程在多线程间切换，也可以指定协程在固定的线程中执行。协程调度模块是一个 N-M 调度模型，即 N 个线程运行 M 个协程。

协程调度相关的类和方法：
- `Scheduler`: 协程调度类。
  
  定义协程调度的基本属性，包括协程调度器名称、线程池、协程任务队列、是否使用 caller 线程参与调度等。

- 协程调度方法 `schedule()`: 支持协程的单个任务以及批量任务的添加。

- 协程调度的实现 `run()`: 内部一个 while(1) 循环，不停地从任务队列中去任务并执行。当任务队列为空时，每个线程将执行 idle 协程。

- 限制：当任务队列为空时，调度协程将不停地检测其是否有新任务产生，即忙等待。这使得调度协程和 idle 协程将不停地切换，导致 CPU 占用率过高。在后续的 IOManager 中，该问题得到了改善。

#### IO 协程调度 (iomanager.h)
概述
- IO 协程调度模块继承调度器 Scheduler，并封装了 epoll，支持为 socket fd 注册读写时间回调函数。
- IO 协程调度模块还解决了调度器在 idle 状态下忙等待导致 CPU 占用率高问题。

IO 协程调度相关的类和方法：
- `IOManager`: IO 协程调度器，继承 Scheduler。
  
  对于 IO 协程调度而言，每次调度都包含一个三元组信息，分别是 描述符——事件类型（可读或可写）——回调函数。调度器记录全部需要调度的三元组信息，其中描述符和事件类型用于 `epoll_wait`，回调函数用于协程调度。这个三元组信息在源码上通过 `FdContext` 结构体来描述，在执行 `epoll_wait` 时通过 `epoll_event` 的私有数据指针 `data.ptr` 来保存 `FdContext` 结构体信息。

- `idle()` 方法: idle 协程的入口函数。
  
  这是 IOManager 解决 Schduler 在 idle 状态下忙等待的关键实现。 IO 协程调度器使用一对管道 fd 来 tickle() 调度协程。当调度器空闲时，idle 协程通过 epoll_wait 阻塞在管道的读描述符上，等管道的可读事件。添加新任务时，tickle() 方法写管道，idle 协程检测到管道可读后退出，调度器进行调度。

- 事件操作方法：注册事件回调 `addEvent()`、删除事件回调 `delEvent()`、取消事件回调 `cancelEvent()`，以及取消全部事件 `cancelAll()`。

#### 定时器模块 (timer.h)
在 IO 协程调度的基础上在增加定时器调度功能，即在指定的超时时间结束后执行对应的回调函数。

定时器的实现机制是 idle 协程的 epoll_wait 超时参数。定时器的主要思想是基于时间堆，即创建定时器时指定超时时间（计算出绝对时间）和回调函数，按照定时器的绝对时间对其进行排序。在 idle() 中，取出堆顶的超时时间作为 idle 协程的 epoll_wait 超时参数，当 epoll_wait 超时返回检查所有已超时的定时器并执行它们的回调函数。

定时器的相关类：
- Timer: 定时器类
- TimerManager: 定时器管理类, IOManager 的基类之一。

### Hook 模块 (hook.h)
Hook 的实现机制基于动态库的全局符号介入功能，用自定义的接口替换掉同名的系统调用接口。HOOK 可以在不重新编写代码的情况下，将原始代码中的 socket IO 相关的API 都转为异步，以提升系统性能。Hook 与 I/O 协程调度密切相关，如果不使用 I/O 协程调度，那 hook 没有意义。

sylar 中的 hook 以线程为单位，可自由设置当前线程是否使用 hook。sylar 中对三类接口进行了 hook:
- sleep 等延时接口，包括 sleep/usleep/nanosleep。
  
  对这些接口进行 hook 后，当前协程只需在 IOManager 注册一个定时事件，之后就可以 yield 让出执行权，等定时事件触发后再继续恢复当前协程的执行。

- socket IO 系列接口，包括 括 read/write/recv/send 等，还包括 connect 和 accept。
  
  这类接口的 hook 首先需要判断操作的 fd 是否是 socket fd，以及用户是否显示地对该 fd 设置过非阻塞模式。如果不是 socket fd 或者 用户显示设置过非阻塞模式，那么就不需要 hook，直接调用操作系统的 IO 接口即可。如果需要 hook，那么首先在 IO 协程调度器上注册对应的读写事件，等事件发生后再继续执行当前协程。当前协程在注册完 IO 事件后即可 yield 让出执行权。

- socket/fcntl/ioctl/close 等接口。
  
  这类接口主要处理的是边缘情况，比如分配 fd 上下文，处理超时及用户显示设置非阻塞问题。

### Address 模块
提供网络地址相关的类，支持与网络地址相关的操作。包括以下几个类：
- `Address` ：所有网络地址基类，抽象类，对应 sockaddr 类型，表示通用的网络地址。只包含抽象方法，不包含具体成员。此外，Address 作为地址基类提供了地址解析与本机网卡地址查询功能，分别用来实现域名解析和获取本机网卡的 IP 地址。
- `IPAddress` ：IP 地址基类，抽象类。继承 Address，并增加了 IP 地址相关的端口、子网掩码、广播地址、网络地址等操作。无论是IPv4还是IPv6都支持这些操作，但这些方法都是抽象方法，需要由继承类来实现。
- `IPv4Address` ：IPv4 地址类，实体类，表示一个 IPv4 地址，对应 sockaddr_in 类型。包含一个 sockaddr_in 成员，可以操作该成员的网络地址和端口，以及获取子网掩码等操作。
- `IPv6Address`：IPv6 地址类，实体类，表示一个 IPv6 地址，对应 sockaddr_in6 类型，包含一个 sockaddr_in6 成员。
- `UnixAddreess`：Unix域套接字类，对应 sockaddr_un 类型，是一个实体类，可以用于实例化对象。UnixAddress类包含一个sockaddr_un对象以及一个路径字符串长度。
- `UnknownAddress`：表示一个未知类型的套接字地址，实体类，对应 sockaddr 类型，这个类型与 Address 类型的区别是它包含一个 sockaddr 成员，并且是一个实体类。

### Socket 模块
将 socket 相关的 API 统一封装为 Socket 模块。Socket 提供创建各种套接字的方法（TCP/UDP/Unix域 socket）。

Socket 相关的类：
- `Socket`: 套接字类，表示一个套接字对象。包括以下属性：文件描述符、地址类型、套接字类型、协议类型、是否连接、本地地址和对端地址。提供了  bind()/connect()/listen() 和 accept()，以及 send()/recv() 等方法。
- `SSLSocket`: 加密套接字类。提供加密的 Socket 通信。
- `FdCtx`: 文件句柄上下文类。对文件句柄进行封装。包括 文件句柄、句柄是否为 socket fd、是否 hook 阻塞、发送/接收超时事件等属性。
- `FdManager`: 文件句柄上下文管理类，文件句柄上下文集合。

### ByteArray 模块
字节数组容器，提供基础类型的序列化与反序列化功能。ByteArray 底层是以链表的形式组织的固定大小的内存块。ByteArray 在序列化不固定长度的有符号/无符号 32 位、64位整数时，使用 zigzag 算法；在序列化字符串时，使用 TLV 编码。

序列化与反序列化相关的类：`ByteArray`。

### Stream 模块
流结构，提供字节流读写接口。所有的流结构都继承自抽象类 Stream，该类规定了一个流必须具备的 read()/write() 和 readFixSize()/writeFixSize()。

Stream 模块相关类：
- `Stream`: 流结构抽象类，定义 read()/write() 等方法。
- `SocketStream`: 套接字流结构，继承 Stream，实现其虚方法。

### TcpServer 模块
TCP 服务器封装，支持同时绑定多个监听地址。TcpServer 还支持分别指定接收客户端和处理客户端的协程调度器。

TcpServer 模块相关类如下：
- `TcpServer`: TCP 服务器。采用模板设计模式，它的 handleClient() 方法由继承类来实现。使用 TcpServer 时，必须派生一个子类，并重新实现子类的 handleClient() 方法，见 [test_tcp_server](./tests/test_tcp_server.cc)。

### HTTP 模块
提供 HTTP 服务，基于 Ragel（有限状态机），实现了 HTTP/1.1 的简单协议实现和 uri 的解析。

HTTP 相关的类如下：
- `HttpMethod` 和 `HttpStatus`: HTTP 方法和状态，复用了  [nodejs/http-parser](https://github.com/nodejs/http-parser) 中定义的方法和状态。
- `HttpRequest` 和 `HttpResponse`: HTTP 请求和响应结构。
- `HttpRequestParser` 和 `HttpResponseParser`: HTTP 请求解析器和响应解析器。基于 [nodejs/http-parser](https://github.com/nodejs/http-parser) 实现，将套接字读到的 HTTP 消息传递给解析器，解析器解析消息并通过回调的方式通知调用方 HTTP 解析的内容。
- `HTTPSession`: HTTP 会话结构。继承自 SocketStream，实现了在套接字流上读取 HTTP 请求与发送 HTTP 响应。在读取 HTTP 请求时，借助 HttpRequestParser 解析请求。
- `HttpServlet`: 提供 HTTP 请求路径到处理类的映射，以规范化 HTTP 消息处理流程。
  - `Servlet`: 每个 Servlet 对象表示一种处理 HTTP 消息的方法。
  - `ServletDispatch`: 包含一个 HTTP 请求路径到 Servlet 对象的映射，以指定一个请求路径由哪个 Servlet 来处理，支持精准匹配和模糊匹配。
- `HttpConnection`: 用于发起 GET/POST 等请求并获取响应，支持超时设置、长连接(keep-alive)等，支持连接池。
  - `HttpConnectionPool`: 连接池实现，只在设置 keep-alive 起作用
- `HttpServer`: HTTP 服务器，继承 TcpServer，重写 handleClient() 方法。将 accept() 后的客户端套接字封装为 HttpSession 结构，以便于 发送/接收 消息。


## 性能测试
使用 AB 测试工具对 HTTP 服务器进行压力测试。

```shell
$ ./my_http_server
$ ab -n 1000000 -c 200 "http://192.168.233.128:80/sylar"
$ ab -n 1000000 -c 200 -k "http://192.168.233.128:80/sylar"
```

测试结果（均使用一个工作线程）：
|        | sylar   | libevent | nginx |
| ------ | ------- | -------  | ------- |
| 短链接 | 14396.57   | 14716.03 | 14126.98 |
| 长连接 | 41515.61   |          | 39582.10 |

## 系统篇
支持以守护进程的方式启动服务器，并实现双进程唤醒功能，当子进程异常退出时，父进程可以重新拉起子进程。

服务器系统相关的类：
- `Application`: 服务器实例类，创建一个新的服务器实例。与配置系统结合，支持以配置文件的方式创建相应的服务器实例。
- `Env`: 环境变量类。支持相对路径转为绝对路径，并解析服务器启动的参数 (argc/argv)。
  ```shell
  -s : start with the terminal
  -d : run as daemon
  -c : default conf path: ./conf
  -p : print help
  ```
- `daemon.h`: 守护进程相关函数。

## WebSocket 模块
WebSocket 协议的实现，以解决客户端和服务器的实时通信问题。

WebSocket 相关的类：
- `WSConnection`: Websocket 连接结构，继承 HttpConnection。提供创建 WebSocket, sendMessage()/recvMessage() 以及 ping()/pong() 等方法。
- `WSSession`: Websocket 会话结构。定义 WebSocket 的消息帧结构，提供 sendMessage()/recvMessage()、ping()/pong() 以及 handleShake() 握手实现。
- `WSServlet`: 提供 WS 请求到路径的映射。
  - `WSServlet`: 每个 WSServlet 对象表示一种处理 WS 消息的方法。
  - `WSServletDispatch`: 包含一个 WS 请求路径到 WSServlet 对象的映射，以指定一个请求路径由哪个 WSServlet 来处理，支持精准匹配和模糊匹配。
- `WSServer`: WebSocket 服务器，继承 TcpServer，重写 handleClient() 方法。将 WebSocket 握手后的客户端套接字封装为 WSSession 结构，以便于 发送/接收 WebSocket 消息。

## 创建项目脚本
提供创建新项目的脚本，将 sylar 作为内置框架。比如创建聊天室的新项目步骤为：
```shell
$ git clone https://github.com/ejgdlyz/sylar.git
# 参数分别为新的项目名称和新的命名空间
$ sh generate.sh chat_room chat       
$ mv sylar/chat_room/ .
$ git init
$ make
$ sh move.sh
```
