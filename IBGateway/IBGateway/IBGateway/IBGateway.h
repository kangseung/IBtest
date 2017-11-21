#ifndef IBGATEWAY_H
#define IBGATEWAY_H
#include "StdAfx.h"
#include "jsgateway.hpp"
#include "eventengine.h"
#include "IBAPISPI.h"

class IBAPISPI;
class IBGateway:JSGateway //这里要继承jsgateway
{
public:
	IBGateway(EventEngine *eventengine);
	~IBGateway();

	void connect();																					//连接
	void subscribe(const jsstructs::SubscribeReq& subscribeReq);									//订阅
	std::string sendOrder(const jsstructs::OrderReq & req);											//发单
	void cancelOrder(const jsstructs::CancelOrderReq & req);										//撤单
	void qryAccount();																				//查询账户资金
	void qryPosition();																				//查询持仓
	void close();																					//断开API
	void writeLog(const std::string &msg);
	void writeError(const std::string& msg, const int &errorID);
	void removeOrder(const std::string &orderID);                                                   //移除order
	void updateOrder(const std::string &orderID, const jsstructs::CancelOrderReq &req);              //更新order
	void initQuery();                                                                               //初始化查询循环
	void query();                                                                                   //查询
	jsstructs::CancelOrderReq getOrder(const std::string &orderID);                                 //获取order
private:
	IBAPISPI *m_ibapispi=nullptr;

	EventEngine *m_eventengine = nullptr;
};
#endif