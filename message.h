/*
 * message.h
 * Desc : message definition
 * Author: lxyfirst@163.com
 */

#pragma once

#include "template_packet.h"
#include<iostream>
#include <string.h>
//BODY
class message {
public:
	message() {
	}
	;
	virtual ~message() {
	}
	;
	virtual int DecodeBody(const char *body, int size) {
		return 0;
	}
	;
	virtual int EncodeBody(const char *body, int size) {
		return 0;
	}
	;
	virtual int ByteSize() {
	}
	;
};

class RegisterRequest: public message {
public:
	virtual int DecodeBody(const char *body, int size) {
		memcpy(buff, body, size);
		return size;
	}
	virtual int EncodeBody(char *body, int size) {
		memcpy(body, buff, size);
		return size;
	}
	void print() {
		std::cout << buff << std::endl;
	}
	virtual int ByteSize() {
		return strlen(buff);
	}
	char *GetBuff() {
		return buff;
	}
private:
	char buff[1024];

};
class RegisterResponse: public message {
public:
	virtual int DecodeBody(const char *body, int size) {
		memcpy(buff, body, size);
		return size;
	}
	virtual int EncodeBody(char *body, int size) {
		memcpy(body, buff, size);
		return size;
	}
	void print() {
		std::cout << buff << std::endl;
	}
	virtual int ByteSize() {
		return strlen(buff);
	}
	char *GetBuff() {
		return buff;
	}
private:
	char buff[1024];
};
enum SystemMessageType {
	SYSTEM_BASE = 256,
	REGISTER_REQUEST = 257,
	REGISTER_RESPONSE = 258,
	STATUS_REQUEST = 259,
	STATUS_RESPONSE = 260,
	CONFIG_DATA_REQUEST = 261,
	CONFIG_DATA_RESPONSE = 262,
	HTTP_REQUEST = 263,
	VOTE_REQUEST = 265,
	VOTE_RESPONSE = 266,
	VOTE_NOTIFY = 267,
	PUSH_QUEUE_REQUEST = 269,
	PUSH_QUEUE_RESPONSE = 270,
	POP_QUEUE_REQUEST = 271,
	POP_QUEUE_RESPONSE = 272,
	SYNC_QUEUE_REQUEST = 273,
	SYNC_QUEUE_RESPONSE = 274,
	FORWARD_REQUEST = 277,
	FORWARD_RESPONSE = 278,
	BROADCAST_NOTIFY = 500
};
// message definition: msgid + head + body
// msgid : module (1 byte) + type (1 byte)

//system message
typedef TemplatePacket<REGISTER_REQUEST,SSHead,RegisterRequest> SSRegisterRequest;
typedef TemplatePacket<REGISTER_RESPONSE,SSHead,RegisterResponse> SSRegisterResponse;

//typedef TemplatePacket<STATUS_REQUEST,SSHead,StatusRequest> SSStatusRequest;
//typedef TemplatePacket<STATUS_RESPONSE,SSHead,StatusResponse> SSStatusResponse;
//
//typedef TemplatePacket<CONFIG_DATA_REQUEST,SSHead,ConfigDataRequest> SSConfigDataRequest;
//typedef TemplatePacket<CONFIG_DATA_RESPONSE,SSHead,ConfigDataResponse> SSConfigDataResponse;
//
//typedef TemplatePacket<BROADCAST_NOTIFY,SSHead,BroadcastNotify> SSBroadcastNotify ;
//
//typedef TemplatePacket<VOTE_REQUEST,SSHead,VoteData> SSVoteRequest ;
//typedef TemplatePacket<VOTE_RESPONSE,SSHead,VoteResponse> SSVoteResponse ;
//
//typedef TemplatePacket<VOTE_NOTIFY,SSHead,VoteData> SSVoteNotify;
//
//
//typedef TemplatePacket<PUSH_QUEUE_REQUEST,SSHead,PushQueueRequest> SSPushQueueRequest ;
//typedef TemplatePacket<PUSH_QUEUE_RESPONSE,SSHead,CommonResponse> SSPushQueueResponse ;
//
//typedef TemplatePacket<POP_QUEUE_REQUEST,SSHead,PopQueueRequest> SSPopQueueRequest ;
//typedef TemplatePacket<POP_QUEUE_RESPONSE,SSHead,CommonResponse> SSPopQueueResponse ;
//
//typedef TemplatePacket<SYNC_QUEUE_REQUEST,SSHead,SyncQueueRequest> SSSyncQueueRequest ;
//typedef TemplatePacket<SYNC_QUEUE_RESPONSE,SSHead,SyncQueueData> SSSyncQueueResponse ;
//
//typedef TemplatePacket<FORWARD_REQUEST,SSHead,ForwardData> SSForwardRequest ;
//typedef TemplatePacket<FORWARD_RESPONSE,SSHead,ForwardData> SSForwardResponse ;
//
//工厂对象构造消息
