#ifndef GET_IP_H
#define GET_IP_H

#include <iostream>     ///< cout          输入输出操作库
#include <cstring>      ///< memset        字符串操作库
#include <errno.h>      ///< errno         错误处理库
#include <sys/socket.h> ///< socket        socket通信库 提供socket函数及数据结构
#include <netinet/in.h> ///< sockaddr_in   网络地址库 定义数据结构sockaddr_in
#include <arpa/inet.h>  ///< getsockname   提供IP地址转换函数
#include <unistd.h>     ///< close         由字面意思，unistd.h是unix std的意思，是POSIX标准定义的unix类系统定义符号常量的头文件

#include <iostream>

using namespace std;

string get_my_local_ip();


#endif