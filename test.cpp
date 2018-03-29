#include<iostream>
#include <unistd.h>
#include<stdio.h>
#include <string.h>
#include"message.h"
#include"packet.h"
#include"template_packet.h"
#include"buffer.h"
using namespace std;
using namespace framework;
enum {
	MAX_BUF_SIZE = 20480
};
//#define TESTNETWORK_UTILS_
#ifdef TESTNETWORK_UTILS_

int main(int argc, char **argv) {
	//create udp server
	sa_in_t service_addr;
	init_sa_in(&service_addr, "127.0.0.1", 9000);
	int sfd = create_udp_service(&service_addr);
	if (sfd < 0) {
		cout << "creat udp socket err\n";
		return -1;
	}
	//for set option
	set_socket_option(sfd, SO_RCVBUF, MAX_BUF_SIZE * 1024);
	set_socket_option(sfd, SO_SNDBUF, MAX_BUF_SIZE * 1024);

	//create tcp server
	sa_in_t service_addr1;
	init_sa_in(&service_addr1, "127.0.0.1", 8000);
	sfd = create_tcp_service(&service_addr1);
	if (sfd < 0) {
		cout << "create tcp socket error\n";
		return -2;
	}

	char IPdotdec[20]; //存放点分十进制IP地址
	struct in_addr s; // IPv4地址结构体
	// 输入IP地址
	strcpy(IPdotdec, "192.168.10.2");
	// 转换
	inet_pton(AF_INET, IPdotdec, (void *) &s);
	printf("inet_pton: 0x%x\n", s.s_addr); // 注意得到的字节序
	// 反转换
	inet_ntop(AF_INET, (void *) &s, IPdotdec, 16);
	printf("inet_ntop: %s\n", IPdotdec);

	sleep(20);
	//sudo netstat -aupn |grep 9000   --udp
	//sudo netstat -atpn|grep 8000    --tcp
	return 0;
}
#endif
#define PACKET_H_
#ifdef PACKET_H_
void test() {
//	SSRegisterRequest request;
//	this->send(&request, 0);
//
//	packet_info* pi;
//	SSRegisterRequest request;
//	if (request.decode(pi->data, pi->size) != pi->size)
//		return -1;
//	int node_type = request.body.node_type();
//	int node_id = request.body.node_id();
}

/*
 *
 SSVoteRequest request ;
 if(request.decode(pi->data,pi->size)!=pi->size) return -1 ;
 info_log_format(m_logger,"recv vote vote_id:%d node_id:%d",request.body.vote_id(),request.body.node_id()) ;

 SSVoteResponse response ;
 response.head = request.head ;

 if(is_leader() || get_leader() )
 {
 response.body.set_error_code(EC_VOTE_LEADER_EXIST) ;
 response.body.mutable_data()->CopyFrom(m_leader_vote_info.active()) ;
 }
 else if( vote_data_gt(request.body,m_self_vote_info)  &&
 vote_data_gt(request.body,m_leader_vote_info.active() )  )
 {
 response.body.set_error_code(EC_SUCCESS) ;
 m_leader_vote_info.backup().CopyFrom(request.body) ;
 m_leader_vote_info.switch_object();

 }
 else
 {
 response.body.set_error_code(EC_VOTE_FAILED) ;
 response.body.mutable_data()->CopyFrom(m_leader_vote_info.active() ) ;
 }


 return handler->send(&response,0) ;
 */
enum {
	MIN_WRITE_SIZE = 0x1 << 10, MAX_WRITE_SIZE = 0x1 << 24,
};

int main(int argc, char **argv) {
//packet info
	struct packet_info packe;
//send packet type:packethead:head+body
	SSRegisterRequest request;	//包头封装+包体封装
	SSHead shead(1000, 2000, 3000, 4000);
	struct PacketHead phead;
	char headbuff[100] = { "\0" };
	request.head = shead;	//decode 的时候必须构造好了数据
//body
	strncpy(request.body.GetBuff(), "zhaoxi send helloworld\n", 24);
	request.body.print();
//copy head to headbuff
	phead.msg_type = 257;	//type

//	strncpy(headbuff, (char *) &phead, sizeof(struct PacketHead));
//	strncpy(headbuff + sizeof(struct PacketHead), (char *) &shead,
//			sizeof(SSHead));
//第二个参数是包体的总长度
//	request.encode(headbuff,
//			sizeof(PacketHead) + sizeof(SSHead) + request.body.ByteSize());
	packet*p = &request;
	buffer m_sbuf;
	int size = sizeof(PacketHead) + sizeof(SSHead);	//整个报文大小
	if (size < 1)
		return -1;
	if (m_sbuf.space_size() < size
			&& m_sbuf.resize(m_sbuf.capacity() + size) != 0) {
		return -1;
	}
	//报文复制到buff中()这里只是把头复制进去还有报文体
	//cout << "space size:" << m_sbuf.space_size() << endl;
	size = p->encode(m_sbuf.space(),
			m_sbuf.space_size() + request.body.ByteSize());
	//	sizeof(PacketHead) + sizeof(SSHead) + request.body.ByteSize());

	if (size < 1)
		return -1;
	m_sbuf.push_data(size - request.body.ByteSize());
	cout << "buff size:" << m_sbuf.data_size() << endl;
	//报文体复制进去
	size = request.body.ByteSize();	//整个报文大小
	if (size < 1)
		return -1;
	if (m_sbuf.space_size() < size
			&& m_sbuf.resize(m_sbuf.capacity() + size) != 0) {
		return -1;
	}
	size = request.body.ByteSize();
	request.body.EncodeBody(m_sbuf.space(), m_sbuf.space_size());
	m_sbuf.push_data(size);


	cout << "buff size:" << m_sbuf.data_size() << endl;
	if (m_sbuf.data_size() > 0) {
		int to_send =
				m_sbuf.data_size() > MAX_WRITE_SIZE ?
						MAX_WRITE_SIZE : m_sbuf.data_size();
		//int send_size = send(1, m_sbuf.data(), to_send, 0);
		int send_size = 0;
		if (send_size > 0) {
			m_sbuf.pop_data(send_size);
			m_sbuf.adjust();

		} else if (send_size < 0) {

//	            if (errno != EAGAIN &&  errno != EINTR)
//	            {
//	                handle_error(ERROR_TYPE_SYSTEM) ;
//	                return ;
//	            }
		}
	}
//Recv packet
	SSRegisterRequest req;
	//解码 packethead
	PacketHead hed;
	decode_packet_head(&hed, m_sbuf.data());
	cout << "head type:" << hed.msg_type << " len:" << hed.length << endl;
	//解码报文头
	int length = req.decode(m_sbuf.data(), hed.length);
	if (0 > length) {
		cout << "packet body error:" << length << endl;
		return -1;
	}
	cout << "decode len:" << length << endl;
	req.body.DecodeBody(m_sbuf.data() + sizeof(PacketHead) + sizeof(SSHead),
			hed.length - sizeof(PacketHead) - sizeof(SSHead));
	req.body.print();


	return 0;
}

#endif
