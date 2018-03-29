/*
 * template_packet.h
 *
 *  Created on: 2012-4-1
 *      Author: lxyfirst@163.com
 */

#pragma once

#include <sstream>
#include <exception>

#include "packet.h"
//1手动对齐;2#pragam pack(n) 对齐数据结构;3取消对齐__attribute__
struct PacketHead
{
    uint16_t length ;
    uint16_t msg_type ;
} __attribute__((packed)) ;

union MsgType
{
    uint16_t value ;
    struct
    {
        uint16_t module_type:8 ;
        uint16_t action_type:8 ;

    } __attribute__((packed)) ;
} __attribute__((packed)) ;

//解码
inline int16_t decode_packet_size(const char* data)
{
    return ntoh_int16(((PacketHead*)data)->length) ;
}

inline int16_t decode_packet_msg_type(const char* data)
{
    return ntoh_int16(((PacketHead*)data)->msg_type) ;
}

inline void decode_packet_head(PacketHead* head,const char* data)
{
    head->length = ntoh_int16( ((PacketHead*)data)->length ) ;
    head->msg_type = ntoh_int16( ((PacketHead*)data)->msg_type ) ;
}
//编码
inline void encode_packet_head(char* data,const PacketHead* head)
{
    ((PacketHead*)data)->length = hton_int16(head->length) ;
    ((PacketHead*)data)->msg_type = hton_int16(head->msg_type) ;
}


//
struct SSHead
{
    uint32_t seq ;
    uint32_t dst_key ;
    uint32_t src_key ;
    uint32_t channel ;
    SSHead():seq(0),dst_key(0),src_key(0),channel(0) { } ;
	SSHead(uint32_t seq1, uint32_t dst_key1, uint32_t src_key1,
			uint32_t channel1) :
			seq(seq1), dst_key(dst_key1), src_key(src_key1), channel(channel1) {
	}
	;
	//拷贝赋值运算符
	SSHead& operator =(const SSHead & s) {
		seq = s.seq;
		dst_key = s.dst_key;
		src_key = s.src_key;
		channel = s.channel;
		return *this;
	}
	;

//编码  数据编码到data中去
    int encode(char* data,int size)
    {
        //if(size < sizeof(this) ) return -1 ;
        SSHead* head = (SSHead*)data ;
        head->seq = hton_int32(this->seq) ;
        head->dst_key= hton_int32(this->dst_key) ;
        head->src_key= hton_int32(this->src_key) ;
        head->channel= hton_int32(this->channel) ;
        return  sizeof(this) ;
    }

    int decode(const char* data,int size)
    {
        //if(size < sizeof(this) ) return -1 ;
        SSHead* head = (SSHead*)data ;

        this->seq = ntoh_int32(head->seq) ;
        this->dst_key = ntoh_int32(head->dst_key) ;
        this->src_key = ntoh_int32(head->src_key) ;
        this->channel = ntoh_int32(head->channel) ;
        return sizeof(this) ;

    }
} __attribute__((packed)) ;

struct CSHead
{
    uint32_t seq ;
    uint32_t dst_key;
    CSHead():seq(0),dst_key(0) { } ;
	CSHead(uint32_t seq1, uint32_t dst_key1) :
			seq(seq1), dst_key(dst_key1) {
	}
	;
	CSHead& operator =(const CSHead & s) {
		seq = s.seq;
		dst_key = s.dst_key;
		return *this;
	}
	;
    int encode(char* data,int size)
    {
        //if(size < sizeof(this) ) return -1 ;
        CSHead* head = (CSHead*)data ;
        head->seq = hton_int32(this->seq) ;
        head->dst_key = hton_int32(this->dst_key) ;
        return  sizeof(this) ;
    }

    int decode(const char* data,int size)
    {
        //if(size < sizeof(this) ) return -1 ;
        CSHead* head = (CSHead*)data ;
        this->seq = ntoh_int32(head->seq) ;
        this->dst_key = ntoh_int32(head->dst_key) ;
        return sizeof(this) ;
    }

} __attribute__((packed)) ;


/*
 * @brief packet template , raw packet = PacketHead + (CSHead | SSHead) + body
 * TemplatePacket = packet_type + head + body
 * PT: Packet type;
 * HT: Head
 * BT: Body
*/

template<int PT,typename HT,typename BT>
class TemplatePacket : public framework::packet
{
public:
    enum{ packet_type = PT ,} ;
    typedef BT body_type ;
    typedef HT head_type ;
public:
    virtual int get_type() {return packet_type ; } ;

    virtual int encode(char* data,int size)
    {
        int total_head_size = sizeof(PacketHead) + sizeof(head) ;
        if(size < total_head_size ) return -1 ;

        head.encode(data +sizeof(PacketHead),sizeof(head)) ;

        PacketHead* head = (PacketHead*)data ;
        head->length = hton_int16(size) ;
        head->msg_type = hton_int16(PT) ;

        return size ;

    }
//解码
    virtual int decode(const char* data,int size)
    {
        int total_head_size = sizeof(PacketHead) + sizeof(head) ;
        if(size < total_head_size ) return -1 ;

		if (size < (int) sizeof(PacketHead))
			return -2;

        PacketHead* phead = (PacketHead*)data ;
		if (ntoh_int16(phead->msg_type) != PT)
			return -3;
        size = ntoh_int16(phead->length) ;

        head.decode(data +sizeof(PacketHead),sizeof(head)) ;

        return size ;

    }

    virtual int encode_size()
    {
		int need_size = (int) sizeof(PacketHead) + sizeof(head)
				+ body.ByteSize();
		if (need_size >= 65535)
			return -1;
		return need_size;
    }

    virtual int decode_size(const char* data,int size)
    {

		return decode_packet_size(data);

    }

public:
    head_type head ;
    body_type body ;
};



class NullBody
{
public:
    static int ByteSize() { return 0 ; } ;
    static int encode(char* data,int size) { return 0 ;} ;
    static int decode(const char* data,int size) { return 0 ; } ;
    static int encode_size() {return 0 ; } ;
}  ;


