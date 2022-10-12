# capture
## 依赖
* libpcap 网络嗅探需要使用，BPF原理
* flex libpcap依赖flex
* bison libpcap依赖bison
* gtest 本项目的单元测试
* spdlog 本项目的日志库

## 说明
本项目是对程序运行状态的监控，包括`network`,`cpu percent`, `memory percent`, `fd cnt`, `io`; 能够帮助开发和运维了解程序的运行状态，本项目导出为c接口的基础库，功能类似top命令+tcpdump命令。能作为资源采集的组成，上报系统中进程的运行情况。（TODO：之后会写一个项目能够上报状态，然后可视化）

## 采集指标
### network
`network`指标是基于`libpcap`。
libpcap（packet capture library），即数据包捕获函数库，是Unix/Linux平台下的网络数据包捕获函数库。它是一个独立于系统的用户层的包捕获API接口，为底层网络监测提供了一个可移植的框架。

libpcap主要由两部分组成：网络分接头（network tap）和数据过滤器（packet filter）。网络分接头从网络设备驱动程序中收集数据进行拷贝，过滤器决定是否接收该数据包。libpcap利用BSD packet filter（BPF）算法对网卡接收到的链路层数据包进行过滤。BPF算法的基本思想是在有BPF监听的网络中，网卡驱动将接收到的数据包复制一份交给BPF过滤器，过滤器根据用户定义的规则决定是否接收此数据包以及需要拷贝该数据包的哪些内容，然后将过滤后的数据交给与过滤器关联的上层应用程序。

libpcap的包捕获机制就是在数据链路层加一个旁路处理。当一个数据包到达网络接口时，libpcap首先利用已经创建的套接字从链路层驱动程序中获得该数据包的拷贝，再通过Tap函数将数据包发给BPF过滤器。BPF过滤器根据用户已经定义好的过滤规则对数据包进行逐一匹配，匹配成功则放入内核缓冲区，并传递给用户缓冲区，匹配失败则直接丢弃。如果没有设置过滤规则，所有数据包都将放入内核缓冲区，并传递给用户层缓冲区

对于网络数据包，获取`原地址:原端口-目的地址:目的端口`到`inode`映射，以及`inode`到`pid`的映射，从而监控进程的收发流量数据。

### cpu percent
分别解析`/proc/stat`和`/proc/pid/stat`。
```
cpupercent = (pid_now_utime + pid_now _stime - pid_last_utime - pid_last_stime) * 100/ ((now_utime + now_ntime + now_stime + now_itime + now_iowtime + now_irqtime + now_sirqtime) - (last_utime + last_ntime + now_stime + last_itime + last_iowtime + last_irqtime + last_sirqtime))
```

### memory percent
分别解析`/proc/meminfo`和`/proc/pid/status`获取总内存大小和程序内存大小。
```
memorypercent = pid_mem * 100 / system_mem
```

### fd cnt
解析`/proc/pid/fd`下有多少fd的数量

### io
解析 `/proc/pid/io`中的`wchar`和`rchar`

## build
对于只使用来说，只执行`./build_debug.sh -all`就行，具体可看脚本。
对于开发来说，执行`./build_debug.sh -all`（该选项会帮助下载第三方依赖），之后执行`./build_debug.sh -build`
都会出一个包在当前目录的`output`目录，可将该目录直接拷贝到其他机器，即可运行

## 使用
```
#include "libcapture.h"

static void Process(CaptureAction action, const Process_t* processes,
                    uint32_t* size) {
  uint32_t tmp = *size;
  if (action == ACTION_REMOVE) {
    std::cout << "remove pid:" << processes[0].pid << std::endl;
  } else {
    for (uint32_t idx = 0; idx < tmp; idx++) {
      std::cout << "name:" << processes[idx].name
                << " cmdline:" << processes[idx].cmdline
                << " user:" << processes[idx].user
                << " group:" << processes[idx].group
                << " pid:" << processes[idx].pid
                << " uid:" << processes[idx].uid
                << " gid:" << processes[idx].gid
                << " fdCnt:" << processes[idx].fdCnt
                << " memory:" << processes[idx].memory
                << " cpuPercent:" << processes[idx].cpuPercent
                << " memPercent:" << processes[idx].memPercent
                << " ioRead:" << processes[idx].ioRead
                << " ioWrite:" << processes[idx].ioWrite
                << " recv:" << processes[idx].recv
                << " send:" << processes[idx].send;
      std::cout << std::endl;
    }
  }
}

static void Packet(const Packet_t* packets, uint32_t* size) {
  uint32_t tmp = *size;
  // 打印包内容
  for (uint32_t idx = 0; idx < tmp; idx++) {
     std::cout << "hash:" << packets[idx].hash
               << " family:" << packets[idx].family
               << " packetSize:" << packets[idx].packetSize;
     std::cout << std::endl;
  }
}

int main() {
  CaptureInitt config;
  memset(&config, 0, sizeof(CaptureInitt));
  config.logfile = "./logs/exa_capture.log";
  config.ms = 100;
  config.snaplen = 100;

  int32_t ret = CaptureInit(&config, &Process, &Packet);
  std::cout << "ret:" << ret << std::endl;
  while (true) {
    /* code */
    CaptureUpdate(-1);
    sleep(3);
  }

  return 0;
}
```

## 之后的计划
* 使用ebpf再写一套（但更可能是用ebpf写profiling工具，目前方案是主动采集），提升性能
* 英文文档
* 其他平台支持，比如windows
* 性能优化，使用对象池，内存池之类进行优化，减少内存碎片出现
* 代码优化，及其机构优化
* 更多协议的支持 比如arp之类协议
* 写一个资源采集，及其可视化系统（进行中)
* 完善cmake框架，引入valgrind以及perf，gperf之类的工具，方便开发定位问题