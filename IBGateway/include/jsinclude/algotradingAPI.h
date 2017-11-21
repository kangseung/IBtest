#ifndef ALGOTRADINGAPI_H
#define ALGOTRADINGAPI_H
#include<vector>
#include<string>
#include<memory>
#include"json11.h"
#include"structs.hpp"
#include"strategyTemplate.h"
#include"bson.h"
#include"mongoc.h"
class StrategyTemplate;
class AlgoTradingAPI
{
public:
	virtual std::vector<std::string> sendOrder(const std::string &symbol, const std::string &orderType, double price, double volume, StrategyTemplate* Strategy)=0;
	virtual void cancelOrder(const std::string &orderID, const std::string &gatewayName)=0;
	virtual void writeAlgoTradingLog(const std::string &msg)=0;
	virtual void writeTradingReason(const std::shared_ptr<Event_Tick> &tick, const std::string &msg, StrategyTemplate *strategy) = 0;
	virtual void writeTradingReason(const jsstructs::BarData &bar, const std::string &msg, StrategyTemplate *strategy) = 0;
	virtual void PutBarChart(const jsstructs::BarData &bar, const jsstructs::BacktestGodData&data, bool inited){};
	virtual void PutEvent(std::shared_ptr<Event>e) = 0;
	virtual std::vector<std::shared_ptr<Event_Tick>>loadTick(const std::string &tickDbName, const std::string &symbol, int days) = 0;
	virtual std::vector<jsstructs::BarData>loadBar(const std::string &BarDbName, const std::string &symbol, int days, time_t startDateTime) = 0;
        mongoc_client_pool_t *m_pool=NULL;
        mongoc_uri_t         *m_uri=NULL;
};
#endif
