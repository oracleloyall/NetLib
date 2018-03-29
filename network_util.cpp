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
 * inet_pton(AF_INET, ip, &foo.sin_addr);   //  ���� foo.sin_addr.addr=inet_addr(ip);

 char str[INET_ADDRSTRLEN];
 char *ptr = inet_ntop(AF_INET,&foo.sin_addr, str, sizeof(str));      // ���� ptr = inet_ntoa(foo.sin_addr)
 */
const char* addr2str(char* dst,int dst_size,const sa_in_t* addr)
{
    return inet_ntop(AF_INET,(const void*)&(addr->sin_addr),dst,dst_size) ;
}
//���÷�������һ��Ҫ�Ȼ�ȡ��������
int set_nonblock(int fd)
{
    int flag = fcntl(fd,F_GETFL) ;
    if ( flag == -1)
    {
        return -1 ;
    }

    return fcntl(fd,F_SETFL,flag | O_NONBLOCK) ;
}
//����TCP send/recv buffһ�㲻��Ҫ��������
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
 SO_REUSEADDR��������һ��������������������������֪�˿ڣ���ʹ��ǰ�����Ľ��˶˿��������ǵı��ض˿ڵ������Դ��ڡ���ͨ������������������ʱ���֣��������ô�ѡ���bindʱ������

 SO_REUSEADDR������ͬһ�˿�������ͬһ�������Ķ��ʵ����ֻҪÿ��ʵ������һ����ͬ�ı���IP��ַ���ɡ�����TCP�����Ǹ�������������������ͬIP��ַ����ͬ�˿ںŵĶ����������

 SO_REUSEADDR��������������ͬһ�˿ڵ�����׽ӿ��ϣ�ֻҪÿ������ָ����ͬ�ı���IP��ַ���ɡ���һ�㲻����TCP��������

 SO_REUSEADDR������ȫ�ظ������󣺵�һ��IP��ַ�Ͷ˿ڰ󶨵�ĳ���׽ӿ���ʱ���������IP��ַ�Ͷ˿�������һ���׽ӿ��ϡ�һ����˵��������Խ���֧�ֶಥ��ϵͳ�ϲ��У�����ֻ��UDP�׽ӿڶ��ԣ�TCP��֧�ֶಥ����

 SO_REUSEPORTѡ�����������壺

 ��ѡ��������ȫ�ظ����󣬵�������������ͬIP��ַ�Ͷ˿ڵ��׽ӿڶ�ָ���˴��׽ӿ�ѡ����С�

 ����������IP��ַ��һ���ಥ��ַ����SO_REUSEADDR��SO_REUSEPORT��Ч��

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
 �򵥵�˵�����ѡ������þ��ǽ��� Nagle��s Algorithm
 */
int set_socket_nodelay(int fd)
{
    int one = 1 ;
    return setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *) &one, sizeof(one)) ;
}
/*
 * ������ɱ�֣��ڴ濽�����ڴ���䣬�����л���ϵͳ���á�
 TCP_DEFER_ACCEPT �����ܵĹ��ף������� ����ϵͳ�����ˡ�
 ����server���ں˺���client����ACK����ֱ�ӵȴ����ݣ������յ�֮���ٻ���serve��accept���أ���server������Ϳ���ֱ�ӵõ����ݲ����������TCP_DEFER_ACCEPT�����á�
 ��֤�������ݲŷ��أ�����Ԥ�������ӹ�����ȫ���ӹ�����
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
 * Tcp ���ӱ�������
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

