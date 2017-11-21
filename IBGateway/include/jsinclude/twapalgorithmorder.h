#ifndef TWAPALGORITHMORDER_H
#define TWAPALGORITHMORDER_H
#include <cstdlib>
#include"strategyTemplate.h"
class TWAPAlgorithmOrder
{
public:
    TWAPAlgorithmOrder(StrategyTemplate *strategy);
    void setTargetPos(const std::string &symbol, double volume,double beginTime_t,double endTime_t,std::string type);
    void setCallback(const std::string & orderid);
    void checkPosTick(const std::shared_ptr<Event_Tick> &tick);
    void checkPosBar(const jsstructs::BarData &bar);

private:
    void buy(const std::string &symbol,double nowtime,double price);
    void sell(const std::string &symbol,double nowtime,double price);
    void sellshort(const std::string &symbol,double nowtime,double price);
    void buycover(const std::string &symbol,double nowtime,double price);


    void cancelAllorders();
    std::string direction;

    StrategyTemplate *strategy;
    bool is_callback=true;
    double m_beginTime_t=0;
    double m_endTime_t=0;

    double m_totalSupposedPos = 0;
    double m_real_old=0;//在平仓时候记录一下有多少持仓

    std::mutex mapsMtx;

    std::map<std::string,double>m_orderID_mapping_orderprice;
    std::map<std::string,double>m_orderID_mapping_ordervolume;
    std::map<std::string,bool>m_orderID_mapping_callback;
};

#endif // TWAPALGORITHMORDER_H
