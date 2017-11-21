#ifndef JSSOCKET_H
#define JSSOCKET_H
#include <string>
#include "nanomsg/nn.hpp"
#include "nanomsg/reqrep.h"
#include "nanomsg/pubsub.h"
#include<mutex>
class JSSocket
{
public:
	JSSocket(int domain, int protocol);
	~JSSocket();
	void bind(std::string address);
	void connect(std::string address);
	void send(std::string msg);
	void sub();
	void unsub();
	void setTimeout(int time);
	std::string recv();
private:
	mutable std::mutex msgmtx;
	int rc;
	nn::socket socket;

};
#endif