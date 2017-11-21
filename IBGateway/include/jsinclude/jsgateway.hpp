#ifndef JSGATEWAY_H
#define JSGATEWAY_H
//抽象类，给所有接口类继承
#include<string>
#include"structs.hpp"
#include"eventengine.h"
class JSGateway
{
public:
    explicit JSGateway(EventEngine *eventengine)
    {
        m_eventengine = eventengine;
    }
    void onTick(std::shared_ptr<Event_Tick>e)
    {
        m_eventengine->put(e);
    }
    void onTrade(std::shared_ptr<Event_Trade>e)
    {
        m_eventengine->put(e);
    }
    void onOrder(std::shared_ptr<Event_Order>e)
    {
        m_eventengine->put(e);
    }
    void onPosition(std::shared_ptr<Event_Position>e)
    {
        m_eventengine->put(e);
    }
    void onAccount(std::shared_ptr<Event_Account>e)
    {
        m_eventengine->put(e);
    }
    void onError(std::shared_ptr<Event_Error>e)
    {
        m_eventengine->put(e);
    }
    void onLog(std::shared_ptr<Event_Log>e)
    {
        m_eventengine->put(e);
    }
    void onContract(std::shared_ptr<Event_Contract>e)
    {
        m_eventengine->put(e);
    }
    virtual void connect() = 0;																					//连接
	virtual void subscribe(const jsstructs::SubscribeReq& subscribeReq) = 0;									//订阅
	virtual std::string sendOrder(const jsstructs::OrderReq & req) = 0;											//发单
	virtual void cancelOrder(const jsstructs::CancelOrderReq & req) = 0;										//撤单
    virtual void qryAccount() = 0;																				//查询账户资金
    virtual void qryPosition() = 0;																				//查询持仓
    virtual void close() = 0;																					//断开API
protected:
    EventEngine *m_eventengine = nullptr;
};
#endif
