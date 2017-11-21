#include"IBGateway.h"
IBGateway::IBGateway(EventEngine *eventengine) :JSGateway(eventengine)
{
	m_eventengine = eventengine;
	m_ibapispi = new IBAPISPI(this);
}

IBGateway::~IBGateway()
{
	if (m_ibapispi != nullptr)
	{
		delete m_ibapispi;
	}
}

void IBGateway::connect()
{

}

void IBGateway::subscribe(const jsstructs::SubscribeReq& subscribeReq)
{

}

std::string IBGateway::sendOrder(const jsstructs::OrderReq & req)
{
	return "";
}

void IBGateway::cancelOrder(const jsstructs::CancelOrderReq & req)
{

}
void IBGateway::qryAccount()
{

}

void IBGateway::qryPosition()
{

}

void IBGateway::close()
{

}

void IBGateway::writeLog(const std::string &msg)
{

}

void IBGateway::writeError(const std::string& msg, const int &errorID)
{

}

void IBGateway::removeOrder(const std::string &orderID)
{

}

void IBGateway::updateOrder(const std::string &orderID, const jsstructs::CancelOrderReq &req)
{

}

void IBGateway::initQuery()
{

}

void IBGateway::query()
{

}

jsstructs::CancelOrderReq IBGateway::getOrder(const std::string &orderID)
{
	jsstructs::CancelOrderReq req;
	return req;
}