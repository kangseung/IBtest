#pragma once
#ifndef STRATEGYTEMPLATE_H
#define STRATEGYTEMPALTE_H
#include<set>
#include"talib.h"
#include"algotradingAPI.h"
#include"algorithmorder.h"
class StrategyData
{
public:
    void insertParam(const std::string &key, const std::string &value);
    void insertVar(const std::string &key, const std::string &value);
    std::string getParam(const std::string& key);
    std::string getVar(const std::string &key);
    inline std::map<std::string, std::string>getParamMap() const;
    inline std::map<std::string, std::string>getVarMap() const;
private:
    mutable std::mutex mtx;					//变量锁
    std::map<std::string, std::string>paramMap;//参数列表
    std::map<std::string, std::string>varMap;//变量列表
};
class AlgoTradingAPI;
class AlgorithmOrder;
class StrategyTemplate
{
public:
    bool isbacktestmode=false;
    bool inited = false;
    bool trading = false;
    AlgorithmOrder *algorithmorder = nullptr;
    jsstructs::BacktestGodData backtestgoddata;

    explicit StrategyTemplate(AlgoTradingAPI *algotrading);
    ~StrategyTemplate();
    virtual void  showStrategy();
    void  onInit();
    void  onRollingInit(const std::string & symbol,time_t change_time);//换月重新初始化  只在回测时候才使用
    void  onStart();
    void  onStop();

    void  cancelOrder(const std::string &orderID);
    void  cancelOrder(std::vector<std::string>cancelList);
    void  cancelOrders();

    void  onTick_template(const std::shared_ptr<Event_Tick>&tick);
    void  onBar_template(const jsstructs::BarData &bar,bool inited);
    void  onOrder_template(const std::shared_ptr<Event_Order>&order);
    void  onTrade_template(const std::shared_ptr<Event_Trade>&trade);

    void  addParam(const std::string &paramName, const std::string &paramValue);
    std::string  getParam(const std::string &paramName);
    void  modifyPos(const std::string &symbol, double pos);
    double  getPos(const std::string &symbol);
    std::map<std::string, double>getPosMap();

    //做多
    std::vector<std::string>buy(const std::string &symbol, double price, double volume);
    //平多
    std::vector<std::string>sell(const std::string &symbol, double price, double volume);
    //做空
    std::vector<std::string>sellshort(const std::string &symbol, double price, double volume);
    //平空
    std::vector<std::string>buycover(const std::string &symbol, double price, double volume);

    AlgoTradingAPI *algotrading=nullptr;

protected:
    virtual void  onTick(const std::shared_ptr<Event_Tick>&tick);
    virtual void  onBar(const jsstructs::BarData &bar);
    virtual void  onOrder(const std::shared_ptr<Event_Order>&order);
    virtual void  onTrade(const std::shared_ptr<Event_Trade>&trade);

    void loadStrategyEvent();
    void updateStrategyEvent();

    virtual void  putEvent();

    std::vector<std::string>sendOrder(const std::string &symbol, const std::string &orderType, double price, double volume);

    std::set<std::string>ninetoeleven;
    std::set<std::string>ninetohalfeleven;
    std::set<std::string>ninetoone;
    std::set<std::string>ninetohalftwo;
    std::set<std::string>treasury_futures;
    StrategyData strategyData;
    const std::string tickDbName = "ctptickdb";
    const std::string barDbName = "ctpminutedb";
    std::map<std::string, double>posMap; std::mutex posMapmtx;

    std::map<std::string, double>name_mapping_var;
private:
    void  saveMongoDB();
    void  loadMongoDB();

    std::map<std::string, int>barMinutemap;
    std::map<std::string, int>barHourmap;
    std::map<std::string, jsstructs::BarData>barmap;
    std::vector<std::string>orderList; std::mutex orderListmtx;
};
#endif
