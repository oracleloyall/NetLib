/*
 * network_util.h
 *
 *      Author: zhaoxi(oracleloyal@gmail.com)
 */

#define _XOPEN_SOURCE

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "network_util.h"

namespace framework
{


void init_sa_in(sa_in_t* addr,const char* ip,int port)
{
    if ( addr == NULL || ip == NULL || port < 1 ) return  ;
    memset(addr,0,sizeof(sa_in_t)) ;
    addr->sin_family = AF_INET ;
    addr->sin_port = htons(port) ;
    addr->sin_addr.s_addr = inet_addr(ip) ;
}

bool equal_sa_in(const sa_in_t* addr1,const sa_in_t* addr2)
{
    if (addr1->sin_port == addr2->sin_port &&
        addr1->sin_addr.s_addr == addr2->sin_addr.s_addr )
    {
        return true ;
    }


    return false ;
}


void init_sa_un(sa_un_t* addr,const char* file)
{
    if ( addr == NULL || file == NULL ) return  ;
    memset(addr,0,sizeof(sa_un_t)) ;
    addr->sun_family = AF_LOCAL ;
    strncpy(addr->sun_path,file,sizeof(addr->sun_path)-1) ;
}
/*
 * inet_pton(AF_INET, ip, &foo.sin_addr);   //  代替 foo.sin_addr.addr=inet_addr(ip);

 char str[INET_ADDRSTRLEN];
 char *ptr = inet_ntop(AF_INET,&foo.sin_addr, str, sizeof(str));      // 代替 ptr = inet_ntoa(foo.sin_addr)
 */
const char* addr2str(char* dst,int dst_size,const sa_in_t* addr)
{
    return inet_ntop(AF_INET,(const void*)&(addr->sin_addr),dst,dst_size) ;
}
//设置非阻塞，一定要先获取，再设置
int set_nonblock(int fd)
{
    int flag = fcntl(fd,F_GETFL) ;
    if ( flag == -1)
    {
        return -1 ;
    }

    return fcntl(fd,F_SETFL,flag | O_NONBLOCK) ;
}
//设置TCP send/recv buff一般不需要额外设置
void set_socket_buffer(int fd,int val)
{
    setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &val, sizeof(val)) ;
    setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &val, sizeof(val)) ;
}

int set_addr_reuse(int fd)
{
    int reuse = 1 ;
    return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) ;

}
/*
 * SO_REUSEADDR
 SO_REUSEADDR允许启动一个监听服务器并捆绑其众所周知端口，即使以前建立的将此端口用做他们的本地端口的连接仍存在。这通常是重启监听服务器时出现，若不设置此选项，则bind时将出错。

 SO_REUSEADDR允许在同一端口上启动同一服务器的多个实例，只要每个实例捆绑一个不同的本地IP地址即可。对于TCP，我们根本不可能启动捆绑相同IP地址和相同端口号的多个服务器。

 SO_REUSEADDR允许单个进程捆绑同一端口到多个套接口上，只要每个捆绑指定不同的本地IP地址即可。这一般不用于TCP服务器。

 SO_REUSEADDR允许完全重复的捆绑：当一个IP地址和端口绑定到某个套接口上时，还允许此IP地址和端口捆绑到另一个套接口上。一般来说，这个特性仅在支持多播的系统上才有，而且只对UDP套接口而言（TCP不支持多播）。

 SO_REUSEPORT选项有如下语义：

 此选项允许完全重复捆绑，但仅在想捆绑相同IP地址和端口的套接口都指定了此套接口选项才行。

 如果被捆绑的IP地址是一个多播地址，则SO_REUSEADDR和SO_REUSEPORT等效。

 */
int set_socket_reuse(int fd)
{
    int reuse = 1 ;
    if( setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse))!=0) return -1 ;
    if( setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse))!=0) return -1 ;
    return 0 ;

}

/*
 * TCP_NODELAY
 简单地说，这个选项的作用就是禁用 Nagle’s Algorithm
 */
int set_socket_nodelay(int fd)
{
    int one = 1 ;
    return setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *) &one, sizeof(one)) ;
}
/*
 * 性能四杀手：内存拷贝，内存分配，进程切换，系统调用。
 TCP_DEFER_ACCEPT 对性能的贡献，就在于 减少系统调用了。
 假如server端内核忽略client发的ACK，而直接等待数据，数据收到之后再唤醒serve（accept返回），server醒来后就可以直接得到数据并处理。这就是TCP_DEFER_ACCEPT的作用。
 保证办有数据才返回，可以预防半连接攻击和全连接攻击。
 */

int set_defer_accept(int fd,int seconds)
{
    return setsockopt(fd, IPPROTO_TCP, TCP_DEFER_ACCEPT, (char *) &seconds, sizeof(seconds)) ;
}



int get_socket_error(int fd)
{
    int error_code = 0 ;
    socklen_t len = (socklen_t)sizeof(error_code);
    getsockopt(fd, SOL_SOCKET, SO_ERROR, &error_code, &len);

    return error_code ;
}

int get_socket_option(int fd,int option_name)
{
    int option_value = 0 ;
    socklen_t len = (socklen_t)sizeof(option_value);
    if ( getsockopt(fd, SOL_SOCKET, option_name , &option_value, &len)!=0 ) return -1;

    return option_value ;
}

int set_socket_option(int fd,int option_name,int option_value)
{
    return setsockopt(fd, SOL_SOCKET, option_name , (char*)&option_value,sizeof(option_value) ) ;
}
/*
 * Tcp 连接保活心跳
 */
int set_tcp_keepalive(int fd,int idle,int count,int interval)
{
    int keepalive = idle >0 ? 1 : 0 ;
    int ret = setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE , (char*)&keepalive,sizeof(keepalive) ) ;
    if ( keepalive >0 && ret == 0 )
    {
        setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, (char*)&idle,sizeof(idle) ) ;
        setsockopt(fd, SOL_TCP, TCP_KEEPCNT, (char*)&count,sizeof(count) ) ;
        setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, (char*)&interval,sizeof(interval) ) ;
    }

    return ret ;

}





int create_tcp_service(sa_in_t* addr)
{
    if ( addr == NULL  ) return -1 ;
    int sockfd = socket(AF_INET,SOCK_STREAM,0) ;
    if ( sockfd < 0 ) return -1 ;
    set_nonblock(sockfd) ;
    set_socket_reuse(sockfd) ;
    if( bind(sockfd,(sa_t*)addr,sizeof(sa_in_t)) !=0 )
    {
        close(sockfd) ;
        return -2 ;
    }
    if ( listen(sockfd,1024) != 0 )
    {
        close(sockfd) ;
        return -2 ;
    }

    return sockfd ;

}

int create_udp_service(sa_in_t* addr)
{
    if ( addr == NULL ) return -1 ;
    int sockfd = socket(AF_INET,SOCK_DGRAM,0) ;
    if ( sockfd < 0 ) return -1 ;
    set_nonblock(sockfd) ;
    set_socket_reuse(sockfd) ;
    if( bind(sockfd,(sa_t*)addr,sizeof(sa_in_t)) !=0 )
    {
        close(sockfd) ;
        return -2 ;
    }


    return sockfd ;

}

int check_socket_event(int fd,int seconds,bool read_event,bool write_event)
{
    struct pollfd pfd = {0} ;
    pfd.fd = fd;
    pfd.events = 0 ;
    if(read_event) pfd.events |= POLLIN ;
    if(write_event) pfd.events |= POLLOUT ;

    if( poll(&pfd,1,seconds*1000) <1)
    {
        errno = ETIMEDOUT ;
        return -1 ;
    }


    errno = get_socket_error(fd) ;
    if ( errno!=0 )
    {
        return -1 ;
    }

    return 0 ;

}

int create_tcp_client(sa_in_t* addr,int second)
{
    if ( addr == NULL ) return -1 ;

    int sockfd = socket(AF_INET,SOCK_STREAM,0) ;
    if ( sockfd < 0 ) return -1 ;
    set_nonblock(sockfd) ;

    if ( connect(sockfd,(sa_t*)addr,sizeof(sa_in_t)) != 0 && errno != EINPROGRESS )
    {
        close(sockfd) ;
        return -errno ;

    }

    if(second == 0) return sockfd ;

    if(check_socket_event(sockfd,second)!=0)
    {
        close(sockfd) ;
        return -errno ;
    }

    return sockfd ;

}


int create_unix_client(sa_un_t* addr,int second)
{
    if ( addr == NULL ) return -1 ;


    int sockfd = socket(AF_LOCAL,SOCK_STREAM,0) ;
    if ( sockfd < 0 ) return -1 ;
    set_nonblock(sockfd) ;
    if ( connect(sockfd,(sa_t*)addr,sizeof(sa_un_t)) != 0 && errno != EINPROGRESS )
    {
        close(sockfd) ;
        return -errno ;
    }

    if(second == 0) return sockfd ;

    if(check_socket_event(sockfd,second)!=0)
    {
        close(sockfd) ;
        return -errno ;
    }

    return sockfd ;
}

}

