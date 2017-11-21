#include"jssocket.h"
#include<thread>
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif
JSSocket::JSSocket(int domain, int protocol) :socket(domain,protocol)
{
	//if (protocol == NN_PUB)
	//{
	//	int queue_length = 9000000;	
	//	this->socket.setsockopt(NN_SOL_SOCKET, NN_RCVBUF , &queue_length, sizeof(queue_length));
	//}
	//else if (protocol == NN_SUB)
	//{
	//	int queue_length = 9000000;
	//	this->socket.setsockopt(NN_SOL_SOCKET, NN_SNDBUF, &queue_length, sizeof(queue_length));
	//}
}

JSSocket::~JSSocket()
{
}

void JSSocket::bind(std::string address)
{
	rc=socket.bind(address.c_str());
}

void JSSocket::connect(std::string address)
{
	rc=socket.connect(address.c_str());
}

void JSSocket::send(std::string msg)
{
	std::unique_lock<std::mutex>lck(msgmtx);
	socket.send(msg.c_str(), msg.size(), 0);
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void JSSocket::sub()
{
	this->socket.setsockopt(NN_SUB, NN_SUB_SUBSCRIBE, "", 0);
}

void JSSocket::unsub()
{
	this->socket.setsockopt(NN_SUB, NN_SUB_UNSUBSCRIBE, "", 0);
}

void JSSocket::setTimeout(int time)
{
	this->socket.setsockopt(NN_SOL_SOCKET, NN_RCVTIMEO, &time,sizeof(time));
}


std::string JSSocket::recv()
{
	char * buffer=NULL;
	int bytes = socket.recv(&buffer, NN_MSG, 0);
	if (buffer != NULL)
	{
		std::string str(buffer);
		nn_freemsg(buffer);
		std::string string = str.substr(0, bytes);
		return string;
	}
	else
	{
		return "Exit";
	}
}