#ifndef JSSOCKETCONVERT
#define JSSOCKETCONVERT
#include"json11/json11.h"
#include "structs.hpp"
class JSSocketConvert
{
public:
	//request in client
	static std::string reqHistoryLog()
	{
		json11::Json jsonobj = json11::Json::object{
			{ "msg_head", "reqHistoryLog" },
	};
		std::string json = jsonobj.dump();
		return json;
	}

	static std::string reqHistoryChart(const std::string &strategyname)
	{
		json11::Json jsonobj = json11::Json::object{
			{ "msg_head", "reqHistoryChartData" },
			{ "strategyname", strategyname }
	};
		std::string json = jsonobj.dump();
		return json;
	}

	static std::string connect(const std::string &gatewayname)
	{
		json11::Json jsonobj = json11::Json::object{
			{ "msg_head", "connect_" + gatewayname },
	};
		std::string json = jsonobj.dump();
		return json;
	}

	static std::string close(const std::string &gatewayname)
	{
		json11::Json jsonobj = json11::Json::object{
			{ "msg_head", "close_" + gatewayname },
	};
		std::string json = jsonobj.dump();
		return json;
	}

	static std::string subscribeSymbol(const std::string &gatewayname, const std::string &symbol)
	{
		json11::Json jsonobj = json11::Json::object{
			{ "msg_head", "subscribe_" + gatewayname },
			{ "symbol", symbol },
	};
		std::string json = jsonobj.dump();
		return json;
	}

	static std::string sendOrder(const jsstructs::OrderReq &req, const std::string &gatewayname)
	{

		json11::Json jsonobj = json11::Json::object{
			{ "msg_head", "sendorder_" + gatewayname },
			{ "symbol", req.symbol },
			{ "currency", req.currency },
			{ "direction", req.direction },
			{ "exchange", req.exchange },
			{ "expiry", req.expiry },
			{ "offset", req.offset },
			{ "optionType", req.optionType },
			{ "price", req.price },
			{ "priceType", req.priceType },
			{ "productClass", req.productClass },
			{ "strikePrice", req.strikePrice },
			{ "volume", req.volume },
	};
		std::string json = jsonobj.dump();
		return json;
	}

	static std::string cancelOrder(const jsstructs::CancelOrderReq &req, const std::string &gatewayname)
	{
		json11::Json jsonobj = json11::Json::object{
			{ "msg_head", "cancelorder_" + gatewayname },
			{ "symbol", req.symbol },
			{ "orderID", req.orderID },
	};
		std::string json = jsonobj.dump();
		return json;
	}

	static std::string loadStrategy()
	{
		json11::Json jsonobj = json11::Json::object{
			{ "msg_head", "loadstrategy" },
	};
		std::string json = jsonobj.dump();
		return json;
	}

	static std::string initStrategy(const std::string &strategyname)
	{
		json11::Json jsonobj = json11::Json::object{
			{ "msg_head", "initstrategy" },
			{ "strategyname", strategyname }
	};
		std::string json = jsonobj.dump();
		return json;
	}

	static std::string startStrategy(const std::string &strategyname)
	{
		json11::Json jsonobj = json11::Json::object{
			{ "msg_head", "startstrategy" },
			{ "strategyname", strategyname },
	};
		std::string json = jsonobj.dump();
		return json;
	}

	static std::string stopStrategy(const std::string &strategyname)
	{
		json11::Json jsonobj = json11::Json::object{
			{ "msg_head", "stopstrategy" },
			{ "strategyname", strategyname },
	};
		std::string json = jsonobj.dump();
		return json;
	}

	static std::string modifyVar(double value, const std::string &varname, const std::string &strategyname)
	{
		json11::Json jsonobj = json11::Json::object{
			{ "msg_head", "modifyvar" },
			{ "varname", varname },
			{ "strategyname", strategyname },
			{ "value", value }
	};
		std::string json = jsonobj.dump();
		return json;
	}

	//callback for server

	static std::string historyCallbackMSG(const std::string &msg, int number, int maxnumber)
	{
		json11::Json jsonobj = json11::Json::object{
			{ "msg_head", "reply_log" },
			{ "number", number + 1 },
			{ "maxnumber", maxnumber },
			{ "msg_data", msg },
	};
		std::string json = jsonobj.dump();
		return json;
	}

	static std::string historyOrderCallback(const jsstructs::OrderCallback& data, int number, int maxnumber)
	{
		json11::Json jsonobj = json11::Json::object{
			{ "msg_head", "ordercallback" },
			{ "number", number + 1 },
			{ "maxnumber", maxnumber },
			{ "gatewayname", data.gatewayname },
			{ "orderID", data.orderID },
			{ "direction", data.direction },
			{ "nodealvolume", data.nodealvolume },
			{ "offset", data.offset },
			{ "price", data.price },
			{ "status", data.status },
			{ "symbol", data.symbol },
			{ "time", data.time },
			{ "volume", data.volume },
	};
		std::string json = jsonobj.dump();
		return json;
	}

	static std::string historyPlotData(const jsstructs::PlotDataStruct &plotdata, int number, int maxnumber)
	{
		json11::Json jsonobj = json11::Json::object{
			{ "msg_head", "plotdata" },
			{ "number", number + 1 },
			{ "maxnumber", maxnumber },
			{ "inited", plotdata.inited },
			{ "strategyname", plotdata.data.strategyname },
			{ "indicatormap", plotdata.data.indicatorMap },
			{ "mainchartmap", plotdata.data.mainchartMap },
			{ "symbol", plotdata.bar.symbol },
			{ "exchange", plotdata.bar.exchange },
			{ "open", plotdata.bar.open },
			{ "high", plotdata.bar.high },
			{ "low", plotdata.bar.low },
			{ "close", plotdata.bar.close },
			{ "openInterest", plotdata.bar.openInterest },
			{ "volume", plotdata.bar.volume },
			{ "date", plotdata.bar.date },
			{ "time", plotdata.bar.time },
			{ "openPrice", plotdata.bar.openPrice },
			{ "preClosePrice", plotdata.bar.preClosePrice },
			{ "highPrice", plotdata.bar.highPrice },
			{ "lowPrice", plotdata.bar.lowPrice },
			{ "upperLimit", plotdata.bar.upperLimit },
			{ "lowerLimit", plotdata.bar.lowerLimit }
	};
		std::string json = jsonobj.dump();
		return json;
	}

	static std::string historyTradeCallback(const jsstructs::TradeCallback& data, int number, int maxnumber)
	{
		json11::Json jsonobj = json11::Json::object{
			{ "msg_head", "tradecallback" },
			{ "number", number + 1 },
			{ "maxnumber", maxnumber },
			{ "gatewayname", data.gatewayname },
			{ "detailedstatus", data.detailedstatus },
			{ "direction", data.direction },
			{ "offset", data.offset },
			{ "price", data.price },
			{ "symbol", data.symbol },
			{ "time", data.time },
			{ "volume", data.volume },
	};
		std::string json = jsonobj.dump();
		return json;
	}

	static std::string historyStrategyCallback(const jsstructs::StrategyCallback& data, int number, int maxnumber)
	{
		json11::Json jsonobj = json11::Json::object{
			{ "msg_head", "strategycallback" },
			{ "number", number + 1 },
			{ "maxnumber", maxnumber },
			{ "strategyname", data.strategyname },
			{ "parammap", data.parammap },
			{ "varmap", data.varmap }
	};
		std::string json = jsonobj.dump();
		return json;
	}

	static std::string historyUpdateStrategyCallback(const jsstructs::StrategyCallback& data, int number, int maxnumber)
	{
		json11::Json jsonobj = json11::Json::object{
			{ "msg_head", "updatestrategy" },
			{ "number", number + 1 },
			{ "maxnumber", maxnumber },
			{ "strategyname", data.strategyname },
			{ "parammap", data.parammap },
			{ "varmap", data.varmap }
	};
		std::string json = jsonobj.dump();
		return json;
	}

	static std::string historyUpdatePortfolio(const jsstructs::PortfolioData& data, int number, int maxnumber)
	{
		json11::Json jsonobj = json11::Json::object{
			{ "msg_head", "updateportfolio" },
			{ "number", number + 1 },
			{ "maxnumber", maxnumber },
			{ "strategyname", data.strategyname },
			{ "datetime", data.datetime },
			{ "symbol", data.symbol },
			{ "delta", data.delta },
			{ "drawdown", data.drawdown },
			{ "holdingposition", data.holdingposition },
			{ "holdingprice", data.holdingprice },
			{ "holdingwinning", data.holdingwinning },
			{ "holding_and_totalclosewinning", data.holding_and_totalclosewinning },
			{ "losing", data.losing },
			{ "maxcapital", data.maxcapital },
			{ "totalclosewinning", data.totalclosewinning },
			{ "winning", data.winning }
	};
		std::string json = jsonobj.dump();
		return json;
	}

	static std::string PriceTable(const jsstructs::PriceTableData &data)
	{
		json11::Json jsonobj = json11::Json::object{
			{ "msg_head", "quotes_lastprice" },
			{ "symbol", data.symbol },
			{ "lastPrice", data.lastPrice },
			{ "bid", data.bid },
			{ "ask", data.ask },
			{ "openInterest", data.openInterest },
			{ "upperLimit", data.upperLimit },
			{ "lowerLimit", data.lowerLimit },
			{ "datetime", data.datetime },
	};
		std::string json = jsonobj.dump();
		return json;
	}

	static std::string PlotData(const jsstructs::BacktestGodData &data, const jsstructs::BarData &bar, bool inited)
	{
		json11::Json jsonobj = json11::Json::object{
			{ "msg_head", "plotdata" },
			{ "inited", inited },
			{ "strategyname", data.strategyname },
			{ "indicatormap", data.indicatorMap },
			{ "mainchartmap", data.mainchartMap },

			{ "symbol", bar.symbol },
			{ "exchange", bar.exchange },
			{ "open", bar.open },
			{ "high", bar.high },
			{ "low", bar.low },
			{ "close", bar.close },
			{ "openInterest", bar.openInterest },
			{ "volume", bar.volume },
			{ "date", bar.date },
			{ "time", bar.time },
			{ "openPrice", bar.openPrice },
			{ "preClosePrice", bar.preClosePrice },
			{ "highPrice", bar.highPrice },
			{ "lowPrice", bar.lowPrice },
			{ "upperLimit", bar.upperLimit },
			{ "lowerLimit", bar.lowerLimit }
	};
		std::string json = jsonobj.dump();
		return json;
	}

	static std::string AccountData(const jsstructs::AccountData& data)
	{
		json11::Json jsonobj = json11::Json::object{
			{ "msg_head", "account" },
			{ "gatewayname", data.gatewayname },
			{ "accountid", data.accountid },
			{ "available", data.available },
			{ "balance", data.balance },
			{ "closeProfit", data.closeProfit },
			{ "commission", data.commission },
			{ "gatewayname", data.gatewayname },
			{ "margin", data.margin },
			{ "positionProfit", data.positionProfit },
			{ "preBalance", data.preBalance },
	};
		std::string json = jsonobj.dump();
		return json;
	}

	static std::string PositionData(const jsstructs::PositionData& data)
	{
		json11::Json jsonobj = json11::Json::object{
			{ "msg_head", "position" },
			{ "gatewayname", data.gatewayname },
			{ "symbol", data.symbol },
			{ "direction", data.direction },
			{ "frozen", data.frozen },
			{ "gatewayname", data.gatewayname },
			{ "position", data.position },
			{ "price", data.price },
			{ "todayPosition", data.todayPosition },
			{ "todayPositionCost", data.todayPositionCost },
			{ "ydPosition", data.ydPosition },
			{ "ydPositionCost", data.ydPositionCost },
	};
		std::string json = jsonobj.dump();
		return json;
	}

	static std::string ordercallback(const jsstructs::OrderCallback& data)
	{
		json11::Json jsonobj = json11::Json::object{
			{ "msg_head", "ordercallback" },
			{ "gatewayname", data.gatewayname },
			{ "orderID", data.orderID },
			{ "direction", data.direction },
			{ "nodealvolume", data.nodealvolume },
			{ "offset", data.offset },
			{ "price", data.price },
			{ "status", data.status },
			{ "symbol", data.symbol },
			{ "time", data.time },
			{ "volume", data.volume },
	};
		std::string json = jsonobj.dump();
		return json;
	}

	static std::string cancelordercallback(const jsstructs::OrderCallback& data)
	{
		json11::Json jsonobj = json11::Json::object{
			{ "msg_head", "cancelordercallback" },
			{ "gatewayname", data.gatewayname },
			{ "orderID", data.orderID },
			{ "direction", data.direction },
			{ "nodealvolume", data.nodealvolume },
			{ "offset", data.offset },
			{ "price", data.price },
			{ "status", data.status },
			{ "symbol", data.symbol },
			{ "time", data.time },
			{ "volume", data.volume },
	};
		std::string json = jsonobj.dump();
		return json;
	}

	static std::string tradecallback(const jsstructs::TradeCallback& data)
	{
		json11::Json jsonobj = json11::Json::object{
			{ "msg_head", "tradecallback" },
			{ "gatewayname", data.gatewayname },
			{ "detailedstatus", data.detailedstatus },
			{ "direction", data.direction },
			{ "offset", data.offset },
			{ "price", data.price },
			{ "symbol", data.symbol },
			{ "time", data.time },
			{ "volume", data.volume },
	};
		std::string json = jsonobj.dump();
		return json;
	}

	static std::string strategycallback(const jsstructs::StrategyCallback& data)
	{
		json11::Json jsonobj = json11::Json::object{
			{ "msg_head", "strategycallback" },
			{ "strategyname", data.strategyname },
			{ "parammap", data.parammap },
			{ "varmap", data.varmap }
	};
		std::string json = jsonobj.dump();
		return json;
	}

	static std::string updatestrategycallback(const jsstructs::StrategyCallback& data)
	{
		json11::Json jsonobj = json11::Json::object{
			{ "msg_head", "updatestrategy" },
			{ "strategyname", data.strategyname },
			{ "parammap", data.parammap },
			{ "varmap", data.varmap }
	};
		std::string json = jsonobj.dump();
		return json;
	}

	static std::string updatePortfolio(const jsstructs::PortfolioData& data)
	{
		json11::Json jsonobj = json11::Json::object{
			{ "msg_head", "updateportfolio" },
			{ "strategyname", data.strategyname },
			{ "datetime", data.datetime },
			{ "symbol", data.symbol },
			{ "delta", data.delta },
			{ "drawdown", data.drawdown },
			{ "holdingposition", data.holdingposition },
			{ "holdingprice", data.holdingprice },
			{ "holdingwinning", data.holdingwinning },
			{ "holding_and_totalclosewinning", data.holding_and_totalclosewinning },
			{ "losing", data.losing },
			{ "maxcapital", data.maxcapital },
			{ "totalclosewinning", data.totalclosewinning },
			{ "winning", data.winning }
	};
		std::string json = jsonobj.dump();
		return json;
	}

	//callback for client  return structs for signal paramater	
	static std::string callbackMSG(const std::string &msg)
	{
		json11::Json jsonobj = json11::Json::object{
			{ "msg_head", "reply_log" },
			{ "msg_data", msg },
	};
		std::string json = jsonobj.dump();
		return json;
	}

	static jsstructs::StrategyCallback getStrategy(const std::string &json)
	{
		jsstructs::StrategyCallback data;
		std::string err;
		const auto document = json11::Json::parse(json, err);
		data.strategyname = document["strategyname"].string_value();
		json11::Json::object paramobj = document["parammap"].object_items();
		for (json11::Json::object::const_iterator it = paramobj.cbegin(); it != paramobj.cend(); ++it)
		{
			data.parammap.insert(std::pair<std::string, std::string>(it->first, it->second.string_value()));
		}
		json11::Json::object varobj = document["varmap"].object_items();
		for (json11::Json::object::const_iterator it = varobj.cbegin(); it != varobj.cend(); ++it)
		{
			data.varmap.insert(std::pair<std::string, std::string>(it->first, it->second.string_value()));
		}
		return data;
	}

	static std::string getInitStrategy(const std::string &json)
	{
		std::string err;
		const auto document = json11::Json::parse(json, err);
		std::string strategyname = document["strategyname"].string_value();
		return strategyname;
	}

	static std::string getStartStrategy(const std::string &json)
	{
		std::string err;
		const auto document = json11::Json::parse(json, err);
		std::string strategyname = document["strategyname"].string_value();
		return strategyname;
	}

	static std::string getStopStrategy(const std::string &json)
	{
		std::string err;
		const auto document = json11::Json::parse(json, err);
		std::string strategyname = document["strategyname"].string_value();
		return strategyname;
	}

	static jsstructs::PriceTableData getPriceTable(const std::string &json)
	{
		jsstructs::PriceTableData data;
		std::string err;
		const auto document = json11::Json::parse(json, err);
		data.gatewayname = document["gatewayname"].string_value();
		data.symbol = document["symbol"].string_value();
		data.lastPrice = document["lastPrice"].string_value();
		data.ask = document["ask"].string_value();
		data.bid = document["bid"].string_value();
		data.upperLimit = document["upperLimit"].string_value();
		data.lowerLimit = document["lowerLimit"].string_value();
		data.openInterest = document["openInterest"].string_value();
		data.datetime = document["datetime"].string_value();
		return data;
	}

	static jsstructs::PlotDataStruct getPlotData(const std::string &json)
	{
		jsstructs::PlotDataStruct data;
		std::string err;
		const auto document = json11::Json::parse(json, err);
		data.inited = document["inited"].bool_value();
		std::string strategyname = document["strategyname"].string_value();
		data.data.strategyname = strategyname;
		json11::Json::object indicatorobj = document["indicatormap"].object_items();
		for (json11::Json::object::const_iterator it = indicatorobj.cbegin(); it != indicatorobj.cend(); ++it)
		{
			data.data.indicatorMap[it->first] = it->second.number_value();
		}
		json11::Json::object mainchartobj = document["mainchartmap"].object_items();
		for (json11::Json::object::const_iterator it = mainchartobj.cbegin(); it != mainchartobj.cend(); ++it)
		{
			data.data.mainchartMap[it->first] = it->second.number_value();
		}

		data.bar.symbol = document["symbol"].string_value();
		data.bar.exchange = document["exchange"].string_value();
		data.bar.open = document["open"].number_value();
		data.bar.high = document["high"].number_value();
		data.bar.low = document["low"].number_value();
		data.bar.close = document["close"].number_value();
		data.bar.openInterest = document["openInterest"].number_value();
		data.bar.volume = document["volume"].number_value();
		data.bar.date = document["date"].string_value();
		data.bar.time = document["time"].string_value();
		data.bar.openPrice = document["openPrice"].number_value();
		data.bar.preClosePrice = document["preClosePrice"].number_value();
		data.bar.highPrice = document["highPrice"].number_value();
		data.bar.lowPrice = document["lowPrice"].number_value();
		data.bar.upperLimit = document["upperLimit"].number_value();
		data.bar.lowerLimit = document["lowerLimit"].number_value();
		data.bar.setUnixDatetime();
		return data;
	}

	static jsstructs::AccountData getAccountData(const std::string &json)
	{
		jsstructs::AccountData data;
		std::string err;
		const auto document = json11::Json::parse(json, err);
		data.gatewayname = document["gatewayname"].string_value();
		data.accountid = document["accountid"].string_value();
		data.available = document["available"].string_value();
		data.balance = document["balance"].string_value();
		data.closeProfit = document["closeProfit"].string_value();
		data.commission = document["commission"].string_value();
		data.gatewayname = document["gatewayname"].string_value();
		data.margin = document["margin"].string_value();
		data.positionProfit = document["positionProfit"].string_value();
		data.preBalance = document["preBalance"].string_value();
		return data;
	}

	static jsstructs::PositionData getPositionData(const std::string &json)
	{
		jsstructs::PositionData data;
		std::string err;
		const auto document = json11::Json::parse(json, err);
		data.gatewayname = document["gatewayname"].string_value();
		data.symbol = document["symbol"].string_value();
		data.direction = document["direction"].string_value();
		data.position = document["position"].string_value();
		data.frozen = document["frozen"].string_value();
		data.price = document["price"].string_value();
		data.todayPosition = document["todayPosition"].string_value();
		data.todayPositionCost = document["todayPositionCost"].string_value();
		data.ydPosition = document["ydPosition"].string_value();
		data.ydPositionCost = document["ydPositionCost"].string_value();
		return data;
	}

	static jsstructs::OrderReq getSendOrder(const std::string &json)
	{
		jsstructs::OrderReq req;
		std::string err;
		const auto document = json11::Json::parse(json, err);
		req.currency = document["currency"].string_value();
		req.direction = document["direction"].string_value();
		req.exchange = document["exchange"].string_value();
		req.expiry = document["expiry"].string_value();
		req.offset = document["offset"].string_value();
		req.optionType = document["optionType"].string_value();
		req.price = document["price"].number_value();
		req.priceType = document["priceType"].string_value();
		req.productClass = document["productClass"].string_value();
		req.strikePrice = document["strikePrice"].number_value();
		req.symbol = document["symbol"].string_value();
		req.volume = document["volume"].number_value();
		return req;
	}

	static jsstructs::OrderCallback getOrdercallback(const std::string &json)
	{
		jsstructs::OrderCallback data;
		std::string err;
		const auto document = json11::Json::parse(json, err);
		data.gatewayname = document["gatewayname"].string_value();
		data.orderID = document["orderID"].string_value();
		data.direction = document["direction"].string_value();
		data.nodealvolume = document["nodealvolume"].string_value();
		data.offset = document["offset"].string_value();
		data.price = document["price"].string_value();
		data.status = document["status"].string_value();
		data.symbol = document["symbol"].string_value();
		data.time = document["time"].string_value();
		data.volume = document["volume"].string_value();
		return data;
	}

	static jsstructs::OrderCallback getCancelordercallback(const std::string &json)
	{
		jsstructs::OrderCallback data;
		std::string err;
		const auto document = json11::Json::parse(json, err);
		data.gatewayname = document["gatewayname"].string_value();
		data.orderID = document["orderID"].string_value();
		data.direction = document["direction"].string_value();
		data.nodealvolume = document["nodealvolume"].string_value();
		data.offset = document["offset"].string_value();
		data.price = document["price"].string_value();
		data.status = document["status"].string_value();
		data.symbol = document["symbol"].string_value();
		data.time = document["time"].string_value();
		data.volume = document["volume"].string_value();
		return data;
	}

	static jsstructs::TradeCallback getTradecallback(const std::string &json)
	{
		jsstructs::TradeCallback data;
		std::string err;
		const auto document = json11::Json::parse(json, err);
		data.gatewayname = document["gatewayname"].string_value();
		data.detailedstatus = document["detailedstatus"].string_value();
		data.direction = document["direction"].string_value();
		data.offset = document["offset"].string_value();
		data.price = document["price"].string_value();
		data.symbol = document["symbol"].string_value();
		data.time = document["time"].string_value();
		data.volume = document["volume"].string_value();
		return data;
	}

	static jsstructs::CancelOrderReq getCancelOrder(const std::string &json)
	{
		jsstructs::CancelOrderReq data;
		std::string err;
		const auto document = json11::Json::parse(json, err);
		data.symbol = document["symbol"].string_value();
		data.orderID = document["orderID"].string_value();
		return data;
	}

	static jsstructs::PortfolioData getPortfoliodata(const std::string &json)
	{
		jsstructs::PortfolioData data;
		std::string err;
		const auto document = json11::Json::parse(json, err);
                data.strategyname = document["strategyname"].string_value();
                data.datetime = document["datetime"].string_value();
                data.symbol = document["symbol"].string_value();
		data.delta = document["delta"].number_value();
		data.drawdown = document["drawdown"].number_value();
		data.holdingposition = document["holdingposition"].number_value();
		data.holdingprice = document["holdingprice"].number_value();
		data.holdingwinning = document["holdingwinning"].number_value();
		data.holding_and_totalclosewinning = document["holding_and_totalclosewinning"].number_value();
		data.losing = document["losing"].number_value();
		data.maxcapital = document["maxcapital"].number_value();
		data.totalclosewinning = document["totalclosewinning"].number_value();
		data.winning = document["winning"].number_value();
		return data;
	}
};
#endif
