# 计算机网络课程设计项目 (2018级)

本项目为吉林大学软件学院计算机网络课程设计项目(2018级)课程项目的代码仓库。

本项目包含《实验三：使用 Raw Socket》与《实验四：实现文件传输服务器》

本项目服务端专为 Linux 设计。

## 特性

- 使用Epoll高性能并发处理
- 支持多客户端并发访问，采用多Reactor架构实现
- 基于零拷贝的高性能文件传输，结合协议优化，传输效率高于常见程序
- 支持多线程分块下载，采用优化的分块选择算法
- 支持断点续传

性能测试结果可以参考 benchmark.md 文件。

## 编译

### Windows

1. 安装CMake
2. 下载源代码并解压，cd至目录
3. 执行

```bash
mkdir build
cd build
cmake --build . --config Release
```

## 使用项目

[Unity](https://github.com/ThrowTheSwitch/Unity) - 单元测试框架
