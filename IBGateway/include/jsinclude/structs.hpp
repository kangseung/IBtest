#ifndef JSTRADERSTRUCT_H
#define JSTRADERSTRUCT_H
#pragma once
#include<string>
#include<memory>
#include<iostream>
#include<vector>
#include<mutex>
#include<map>
#include"utils.h"
/*************************************/
#define PRICETYPE_LIMITPRICE "pricetypelimit"
#define PRICETYPE_MARKETPRICE "pricetypemarket"
#define PRICETYPE_FAK "FAK"
#define PRICETYPE_FOK "FOK"
#define DIRECTION_NONE "directionnone"
#define DIRECTION_LONG "directionlong"
#define DIRECTION_SHORT "directionshort"
#define DIRECTION_UNKNOWN "unknow"
#define DIRECTION_NET "directionnet"
#define OFFSET_NONE "offsetnone"
#define OFFSET_OPEN "offsetopen"
#define OFFSET_CLOSE "offsetclose"
#define OFFSET_CLOSETODAY "offsetclosetoday"
#define OFFSET_CLOSEYESTERDAY "offsetcloseyesterday"
#define OFFSET_UNKNOWN "offsetunknow"
#define EXCHANGE_SHFE "SHFE"
#define STATUS_ALLTRADED "orderalltraded"
#define STATUS_CANCELLED "ordercanceld"
#define STATUS_WAITING "orderwaiting"
#define ALGOBUY "algobuy"
#define ALGOSELL "algosell"
#define ALGOSHORT "algoshort"
#define ALGOCOVER "algocover"
namespace jsstructs
{
struct SubscribeReq
{
    std::string symbol;
    std::string exchange;
    std::string productClass;
    std::string currency;
    std::string expiry;
    double strikePrice;
    std::string optionType;
};

struct OrderReq
{
    std::string symbol;
    std::string exchange;
    double price;
    double volume;
    std::string priceType;
    std::string direction;
    std::string offset;
    std::string productClass;
    std::string currency;
    std::string expiry;
    double strikePrice;
    std::string optionType;
};

struct CancelOrderReq
{
    std::string symbol;
    std::string exchange;
    std::string orderID;
    std::string frontID;
    std::string sessionID;
};

struct CtpConnectData
{
    std::string	 userID;
    std::string  password;
    std::string  brokerID;
    std::string  md_address;
    std::string  td_address;
};

struct SessionData
{
    std::string brokerID;
    std::string investorID;
    int frontID;
    int sessionID;
};

struct PriceTableData
{
    std::string gatewayname;
    std::string symbol;
    std::string lastPrice;
    std::string bid;
    std::string ask;
    std::string openInterest;
    std::string upperLimit;
    std::string lowerLimit;
    std::string datetime;
};

struct AccountData
{
    std::string gatewayname;
    std::string accountid;
    std::string preBalance;//昨日账户结算净值
    std::string balance;//账户净值
    std::string available;//可用资金
    std::string commission;//今日手续费
    std::string margin;//保证金占用
    std::string closeProfit;//平仓盈亏
    std::string positionProfit;//持仓盈亏
};

struct PositionData
{
    std::string gatewayname;
    std::string symbol;
    std::string direction;
    std::string position;
    std::string todayPosition;
    std::string ydPosition;
    std::string todayPositionCost;
    std::string ydPositionCost;
    std::string price;
    std::string frozen;
};

struct OrderCallback
{
    std::string gatewayname;
    std::string orderID;
    std::string symbol;
    std::string direction;
    std::string offset;
    std::string volume;
    std::string nodealvolume;
    std::string price;
    std::string status;
    std::string time;
};

struct TradeCallback
{
    std::string gatewayname;
    std::string symbol;
    std::string direction;
    std::string offset;
    std::string volume;
    std::string price;
    std::string time;
    std::string detailedstatus;
};

struct StrategyCallback
{
    std::string strategyname;
    std::map<std::string, std::string>parammap;
    std::map<std::string, std::string>varmap;
};

struct BacktestGodData
{
    std::string strategyname;
    std::map<std::string, double>mainchartMap;
    std::map<std::string, double>indicatorMap;
};

class BarData
{
public:
    inline int getMinute() const
    {
        std::vector<std::string>timelist = Utils::split(this->time, ":");
        if (!timelist.empty())
        {
            return std::stoi(timelist.at(1));
        }
        else
        {
            std::vector<std::string>timelist = Utils::split(Utils::getCurrentTime(), ":");
            if (!timelist.empty())
            {
                return std::stoi(timelist.at(1));
            }
        }
    }

    inline int getHour() const
    {
        std::vector<std::string>timelist = Utils::split(this->time, ":");
        if (!timelist.empty()) {
            return std::stoi(timelist.at(0));
        }
        else {
            std::vector<std::string>timelist = Utils::split(Utils::getCurrentTime(), ":");
            if (!timelist.empty()) {
                return std::stoi(timelist.at(0));
            }
        }
    }

    inline time_t getTime_t() const
    {
        return unixdatetime;
    }

    void setUnixDatetime()
    {
        std::vector<std::string>datelist = Utils::split(date, "-");
        std::vector<std::string>timelist = Utils::split(time, ":");
        std::vector<std::string>seconds;
        if (timelist.at(2).find(".") != std::string::npos)
        {
            seconds = Utils::split(timelist.at(2), ".");
        }
        else
        {
            seconds.push_back(timelist.at(2));
        }

        if (datelist.size() == 3 && timelist.size() == 3) {
            tm l;
            l.tm_year = std::stoi(datelist.at(0)) - 1900;
            l.tm_mon = std::stoi(datelist.at(1)) - 1;
            l.tm_mday = std::stoi(datelist.at(2));
            l.tm_hour = std::stoi(timelist.at(0));
            l.tm_min = std::stoi(timelist.at(1));
            l.tm_sec = std::stoi(seconds.at(0));
            time_t ft = mktime(&l);
            this->unixdatetime = ft;
        }
    }
    std::string gatewayname;
    std::string symbol;
    std::string exchange;
    double open = 0;
    double high = 0;
    double low = 0;
    double close = 0;
    double openPrice = 0;
    double highPrice = 0;
    double lowPrice = 0;
    double preClosePrice = 0;
    double upperLimit = 0;
    double lowerLimit = 0;
    double volume = 0;
    double openInterest = 0;
    double weightFactor = 1;
    std::string date;
    std::string time;
    bool isLastBar=false;
private:
    time_t unixdatetime = 0;
};

struct PlotDataStruct
{
    bool inited;
    BarData bar;
    BacktestGodData data;
};

struct PortfolioData
{
    std::string strategyname;
    std::string datetime;
    std::string symbol;
    double winning = 0;
    double losing = 0;
    double totalclosewinning = 0;
    double holdingwinning = 0;
    double holding_and_totalclosewinning = 0;
    double holdingposition = 0;
    double holdingprice = 0;
    double drawdown = 0;
    double maxcapital = 0;
    double delta = 0;
};
}

#define EVENT_QUIT "equit"//退出事件
#define EVENT_TIMER "etimer"//循环1秒钟事件，用来不断的查持仓，防止CTP流控
#define EVENT_CLOSEMARKET "eclose" //收盘事件
#define EVENT_TICK "etick"
#define EVENT_TRADE "etrade"
#define EVENT_ORDER "eorder"
#define EVENT_POSITION "ePosition"
#define EVENT_ACCOUNT "eAccount"
#define EVENT_CONTRACT "eContract"
#define EVENT_ERROR "eError"
#define EVENT_LOG "elog"
#define EVENT_LOADSTRATEGY "eloadstrategy"
#define EVENT_UPDATESTRATEGY "eupdatestrategy"
#define EVENT_UPDATEPORTFOLIO "eupdateportfolio"
#define EVENT_BACKTEST_TICK "ebacktesttick"
#define EVENT_BACKTEST_BAR "ebacktestbar"
class   Event //结构中定义事件
{
public:
    Event(std::string type) :m_eventtype(type)
    {}
    std::string GetEventType() {
        return m_eventtype;
    }
private:
    std::string m_eventtype;
};

class   Event_Exit :public Event
{
public:
    Event_Exit() :Event(EVENT_QUIT)
    {}
};

class    Event_Timer :public Event
{
public:
    Event_Timer() :Event(EVENT_TIMER)
    {}
};

class    Event_Tick :public Event
{
public:
    Event_Tick() :Event(EVENT_TICK)
    {}
    inline int getMinute() const
    {
        std::vector<std::string>timelist = Utils::split(this->time, ":");
        if (!timelist.empty())
        {
            return std::stoi(timelist.at(1));
        }
        else {
            std::vector<std::string>timelist = Utils::split(Utils::getCurrentTime(), ":");
            if (!timelist.empty())
            {
                return std::stoi(timelist.at(1));
            }
        }
    }

    inline int getHour() const
    {
        std::vector<std::string>timelist = Utils::split(this->time, ":");
        if (!timelist.empty()) {
            return std::stoi(timelist.at(0));
        }
        else {
            std::vector<std::string>timelist = Utils::split(Utils::getCurrentTime(), ":");
            if (!timelist.empty()) {
                return std::stoi(timelist.at(0));
            }
        }
    }

    inline time_t getTime_t() const
    {
        return unixdatetime;
    }

    void setUnixDatetime()
    {
        std::vector<std::string>datelist = Utils::split(date, "-");
        std::vector<std::string>timelist = Utils::split(time, ":");
        std::vector<std::string>seconds;
        if (timelist.at(2).find(".") != std::string::npos)
        {
            seconds = Utils::split(timelist.at(2), ".");
        }
        else
        {
            seconds.push_back(timelist.at(2));
        }
        if (datelist.size() == 3 && timelist.size() == 3) {
            tm l;
            l.tm_year = std::stoi(datelist.at(0)) - 1900;
            l.tm_mon = std::stoi(datelist.at(1)) - 1;
            l.tm_mday = std::stoi(datelist.at(2));
            l.tm_hour = std::stoi(timelist.at(0));
            l.tm_min = std::stoi(timelist.at(1));
            l.tm_sec = std::stoi(seconds.at(0));
            time_t ft = mktime(&l);
            unixdatetime = ft;
        }
    }

    //代码
    std::string symbol;
    std::string exchange;
    std::string gatewayname;
    //成交数据
    double lastprice;//最新成交价
    double volume;//总成交量
    double openInterest;//持仓量
    std::string date;//日期
    std::string time;//时间
    //常规行情
    double openPrice;//今日开
    double highPrice;//今日高
    double lowPrice;//今日低
    double preClosePrice;//昨收
    double upperLimit;//涨停
    double lowerLimit;//跌停
    //五档行情
    double bidprice1;
    double bidprice2;
    double bidprice3;
    double bidprice4;
    double bidprice5;
    double askprice1;
    double askprice2;
    double askprice3;
    double askprice4;
    double askprice5;
    double bidvolume1;
    double bidvolume2;
    double bidvolume3;
    double bidvolume4;
    double bidvolume5;
    double askvolume1;
    double askvolume2;
    double askvolume3;
    double askvolume4;
    double askvolume5;
private:
    time_t unixdatetime;
};

class    Event_Trade :public Event
{
public:
    Event_Trade() :Event(EVENT_TRADE)
    {}
    //代码编号
    std::string symbol;
    std::string exchange;
    std::string tradeID;   //交易编号
    std::string orderID;  //订单编号
    std::string gatewayname;
    //成交相关
    std::string direction;//方向
    std::string offset; //成交开平仓
    double price;//成交价格
    double volume;//成交量
    std::string tradeTime;//成交时间
};

class    Event_Order :public Event
{
public:
    Event_Order() :Event(EVENT_ORDER)
    {}
    //编号相关
    std::string symbol;
    std::string exchange;
    std::string orderID;//订单编号
    std::string gatewayname;
    //报单相关
    std::string direction;//方向
    std::string offset;//开平方向
    double price; //报单价格
    double totalVolume;//报单总量
    double tradedVolume;//成交数量
    std::string status;//报单状态

    std::string orderTime;//发单时间
    std::string cancelTime;//撤单时间

    int frontID;//前置机编号
    int sessionID;//连接编号

};

class    Event_Contract :public Event
{
public:
    Event_Contract() :Event(EVENT_CONTRACT)
    {}
    std::string symbol;
    std::string exchange;
    std::string name;                   //合约中文名
    std::string gatewayname;
    std::string productClass;           //合约类型
    int size;                           //合约大小
    double priceTick;					//合约最小价格Tick

    double strikePrice;					//行权价
    std::string underlyingSymbol;       //标的物合约代码
    std::string optionType;				//期权类型
};

class    Event_Position :public Event
{
public:
    Event_Position() :Event(EVENT_POSITION) {
        position = 0;
        todayPosition = 0;
        ydPosition = 0;
        todayPositionCost = 0;
        ydPositionCost = 0;
        price = 0;
        frozen = 0;
    }
    std::string symbol;
    std::string direction;
    std::string gatewayname;
    double position;
    double todayPosition;
    double ydPosition;
    double todayPositionCost;
    double ydPositionCost;
    double price;
    double frozen;
};

class    Event_Account :public Event
{
public:
    Event_Account() :Event(EVENT_ACCOUNT)
    {}
    std::string gatewayname;
    std::string accountid;
    double preBalance;//昨日账户结算净值
    double balance;//账户净值
    double available;//可用资金
    double commission;//今日手续费
    double margin;//保证金占用
    double closeProfit;//平仓盈亏
    double positionProfit;//持仓盈亏
};

class    Event_Error :public Event
{
public:
    Event_Error() :Event(EVENT_ERROR)
    {}
    std::string errorID;//错误代码
    std::string errorMsg;//错误信息
    std::string additionalInfo;//附加信息
    std::string gatewayname;
    std::string errorTime = Utils::getCurrentDateTime();
};

class    Event_Log :public Event
{
public:
    Event_Log() :Event(EVENT_LOG)
    {}
    std::string msg;//log信息
    std::string gatewayname;
    std::string logTime = Utils::getCurrentDateTime();
};

class    Event_marketclose :public Event
{
public:
    Event_marketclose() :Event(EVENT_CLOSEMARKET)
    {}
    std::string msg;//log信息
    std::string gatewayname;
    std::string logTime = Utils::getCurrentDateTime();
};

class    Event_UpdateStrategy :public Event
{
public:
    Event_UpdateStrategy() :Event(EVENT_UPDATESTRATEGY)
    {}
    std::string strategyname;
    std::map<std::string, std::string>parammap;
    std::map<std::string, std::string>varmap;
};

class    Event_UpdatePortfolio :public Event
{
public:
    Event_UpdatePortfolio() :Event(EVENT_UPDATEPORTFOLIO)
    {}
    std::string strategyname;
    std::string symbol;
    std::string datetime;
    jsstructs::PortfolioData Portfoliodata;
};

class    Event_LoadStrategy :public Event
{
public:
    Event_LoadStrategy() :Event(EVENT_LOADSTRATEGY)
    {}
    std::string strategyname;
    std::map<std::string, std::string>parammap;
    std::map<std::string, std::string>varmap;
};

class    Event_Backtest_Tick :public Event
{
public:
    Event_Backtest_Tick() :Event(EVENT_BACKTEST_TICK)
    {}
    Event_Tick tick;
};

class    Event_Backtest_Bar :public Event
{
public:
    Event_Backtest_Bar() :Event(EVENT_BACKTEST_BAR)
    {}
    jsstructs::BarData bar;
};

class BarGreater
{
public:
    bool operator ()(const jsstructs::BarData& bar1, const jsstructs::BarData& bar2)
    {
        return bar1.getTime_t() < bar2.getTime_t();
    }
};

class TickGreater
{
public:
    bool operator ()(const std::shared_ptr<Event_Tick>& tick1, const std::shared_ptr<Event_Tick>& tick2)
    {
        return tick1->getTime_t() < tick2->getTime_t();
    }
};
#endif
