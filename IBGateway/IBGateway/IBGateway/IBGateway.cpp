#include"StdAfx.h"
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
	m_ibapispi->connect("127.0.0.1", 7497, 0);
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
	m_ibapispi->disconnect();
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

void IBGateway::reqHistoryData(const std::string &symbol,const Contract& contract, const std::string& durationStr, const std::string&  barSizeSetting)
{
	m_ibapispi->historicalDataRequests(symbol,contract, durationStr, barSizeSetting);
}

void IBGateway::realTimeBars(const std::string &symbol, const Contract&contract, int barSize)
{
	m_ibapispi->realTimeBars(symbol, contract, barSize);
}

void IBGateway::onHistoricalData(std::shared_ptr<Event_Bar>e)
{
	m_eventengine->put(e);
}

void IBGateway::onTimeBarsData(std::shared_ptr<Event_Bar>e)
{
	m_eventengine->put(e);
}