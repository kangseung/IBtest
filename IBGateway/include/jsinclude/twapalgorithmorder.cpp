#include "twapalgorithmorder.h"
TWAPAlgorithmOrder::TWAPAlgorithmOrder(StrategyTemplate *strategy)
{
    this->strategy=strategy;
}

void TWAPAlgorithmOrder::setTargetPos(const std::string &symbol, double volume,double beginTime_t,double endTime_t,std::string type)//for strategy
{
    m_real_old=0;

    direction=type;
    m_beginTime_t=beginTime_t;
    m_endTime_t=endTime_t;
    m_totalSupposedPos=volume;
    cancelAllorders();   //每次设置前撤掉，清空所有挂单
}

void TWAPAlgorithmOrder::setCallback(const std::string &orderid)
{
    std::unique_lock<std::mutex>lck(mapsMtx);
    m_orderID_mapping_callback[orderid]=true;
}

void TWAPAlgorithmOrder::checkPosTick(const std::shared_ptr<Event_Tick> &tick)
{

}

void TWAPAlgorithmOrder::checkPosBar(const jsstructs::BarData &bar)
{
    std::unique_lock<std::mutex>lck(mapsMtx);
    bool is_callback=true;
    for(std::map<std::string,bool>::const_iterator it=m_orderID_mapping_callback.cbegin();it!=m_orderID_mapping_callback.cend();++it)
    {
        if(it->second==false)
        {
            is_callback=false;
        }
    }

    if(is_callback==true)
    {
        if(m_real_old==0)
        {
            m_real_old=this->strategy->getPos(bar.symbol);
        }

        double nowtime=bar.getTime_t();//获取当前时间

        if(direction=="buy")
        {
            buy(bar.symbol,nowtime,bar.close);
        }
        else if(direction=="sellshort")
        {
            sellshort(bar.symbol,nowtime,bar.close);
        }
        else if(direction=="sell")
        {
            if(m_real_old!=0)
            {
                sell(bar.symbol,nowtime,bar.close);
            }
        }
        else
        {
            if(m_real_old!=0)
            {
                buycover(bar.symbol,nowtime,bar.close);
            }
        }
    }
}

void TWAPAlgorithmOrder::buy(const std::string &symbol, double nowtime, double price)
{
    std::set<std::string>cancelSets;//设置要撤单的orderID

    //开多
    bool orderStop=false;
    double real=this->strategy->getPos(symbol);

    if (real>m_totalSupposedPos)
    {
        this->strategy->algotrading->writeAlgoTradingLog("real>m_totalSupposedPos 出现错误停止报单");
        orderStop=true;
    }

    double order_real_pos=0;
    for(std::map<std::string,double>::const_iterator it=m_orderID_mapping_ordervolume.cbegin();it!=m_orderID_mapping_ordervolume.cend();++it)
    {
        order_real_pos+=it->second;
    }

    double twapPos=std::min(m_totalSupposedPos*((nowtime-m_beginTime_t)/(m_endTime_t-m_beginTime_t)),m_totalSupposedPos);


    double order_hypo1_pos=std::min(twapPos-real,m_totalSupposedPos-twapPos);
    double order_hypo1_price=price;


    double order_cancel_pos=0;

    for(std::map<std::string,double>::iterator it=m_orderID_mapping_orderprice.begin();it!=m_orderID_mapping_orderprice.end();)
    {
        if(it->second<price) //挂单价低于最新价
        {
            cancelSets.insert(it->first);
            order_cancel_pos+=m_orderID_mapping_ordervolume[it->first];
            m_orderID_mapping_ordervolume.erase(it->first);
            m_orderID_mapping_orderprice.erase(it++);
        }
        else
        {
            ++it;
        }

    }

    double pos_diff = order_hypo1_pos-order_real_pos+order_cancel_pos;
    double order_new_pos = std::max(pos_diff, 0.0);
    order_cancel_pos += std::max(-pos_diff, 0.0);

    if(pos_diff<0)
    {
        for(std::map<std::string,double>::iterator it=m_orderID_mapping_orderprice.begin();it!=m_orderID_mapping_orderprice.end();)
        {
            if(cancelSets.find(it->first)==cancelSets.end())
            {
                cancelSets.insert(it->first);
                m_orderID_mapping_ordervolume.erase(it->first);
                m_orderID_mapping_orderprice.erase(it++);
            }
            else
            {
                it++;
            }
        }
    }


    double order_hypo2_pos=std::min(twapPos - real -order_hypo1_pos,m_totalSupposedPos - real - order_cancel_pos - order_hypo1_pos);

    if(orderStop || real>twapPos)
    {
        order_new_pos=0;
        order_cancel_pos=order_real_pos;
        order_hypo1_pos=0;
        order_hypo2_pos=0;
        cancelSets.clear();
        for(std::map<std::string,double>::iterator it=m_orderID_mapping_orderprice.begin();it!=m_orderID_mapping_orderprice.end();)
        {
            cancelSets.insert(it->first);
        }
    }

    //撤单

    for(std::set<std::string>::const_iterator it=cancelSets.cbegin();it!=cancelSets.cend();++it)
    {
        this->strategy->cancelOrder(*it);
    }

    if(order_hypo1_pos!=0||order_hypo2_pos!=0)
    {
        if(order_hypo1_pos<0)
        {
            this->strategy->algotrading->writeAlgoTradingLog("order_hypo1_pos 仓位小于0出现了BUG");
            return ;
        }

        if(order_hypo2_pos<0)
        {
            this->strategy->algotrading->writeAlgoTradingLog("order_hypo2_pos 仓位小于0出现了BUG");
            return ;
        }
        //买入
        std::vector<std::string>v;
        if(order_hypo1_pos!=0)
        {
            v = this->strategy->buy(symbol, price,order_hypo1_pos);

            for (std::vector<std::string>::const_iterator it = v.cbegin(); it != v.cend(); ++it)
            {
                m_orderID_mapping_ordervolume[*it]=order_hypo1_pos;
                m_orderID_mapping_orderprice[*it]=price;
                m_orderID_mapping_callback[*it]=false;
            }
        }

        if(order_hypo2_pos!=0)
        {
            v = this->strategy->buy(symbol, price+100,order_hypo2_pos);

            for (std::vector<std::string>::const_iterator it = v.cbegin(); it != v.cend(); ++it)
            {
                m_orderID_mapping_ordervolume[*it]=order_hypo2_pos;
                m_orderID_mapping_orderprice[*it]=price+100;
                m_orderID_mapping_callback[*it]=false;
            }
        }
    }
}

void TWAPAlgorithmOrder::sellshort(const std::string &symbol, double nowtime, double price)
{
    std::set<std::string>cancelSets;//设置要撤单的orderID

    //开空
    bool orderStop=false;
    double real=std::abs(this->strategy->getPos(symbol));

    if (real>std::abs(m_totalSupposedPos))
    {
        this->strategy->algotrading->writeAlgoTradingLog("real>std::abs(m_totalSupposedPos) 出现错误停止报单");
        orderStop=true;
    }

    double order_real_pos=0;
    for(std::map<std::string,double>::const_iterator it=m_orderID_mapping_ordervolume.cbegin();it!=m_orderID_mapping_ordervolume.cend();++it)
    {
        order_real_pos+=it->second;
    }

    double twapPos=std::min(std::abs(m_totalSupposedPos)*((nowtime-m_beginTime_t)/(m_endTime_t-m_beginTime_t)),std::abs(m_totalSupposedPos));


    double order_hypo1_pos=std::min(twapPos-real,std::abs(m_totalSupposedPos)-twapPos);
    double order_hypo1_price=price;


    double order_cancel_pos=0;

    for(std::map<std::string,double>::iterator it=m_orderID_mapping_orderprice.begin();it!=m_orderID_mapping_orderprice.end();)
    {
        if(it->second>price) //挂单价高于最新价
        {
            cancelSets.insert(it->first);
            order_cancel_pos+=m_orderID_mapping_ordervolume[it->first];
            m_orderID_mapping_ordervolume.erase(it->first);
            m_orderID_mapping_orderprice.erase(it++);
        }
        else
        {
            ++it;
        }

    }

    double pos_diff = order_hypo1_pos-order_real_pos+order_cancel_pos;
    double order_new_pos = std::max(pos_diff, 0.0);
    order_cancel_pos += std::max(-pos_diff, 0.0);

    if(pos_diff<0)
    {
        for(std::map<std::string,double>::iterator it=m_orderID_mapping_orderprice.begin();it!=m_orderID_mapping_orderprice.end();)
        {
            if(cancelSets.find(it->first)==cancelSets.end())
            {
                cancelSets.insert(it->first);
                m_orderID_mapping_ordervolume.erase(it->first);
                m_orderID_mapping_orderprice.erase(it++);
            }
            else
            {
                it++;
            }
        }
    }


    double order_hypo2_pos=std::min(twapPos - real -order_hypo1_pos,std::abs(m_totalSupposedPos) - real - order_cancel_pos - order_hypo1_pos);

    if (orderStop || real>twapPos)
    {
        order_new_pos=0;
        order_cancel_pos=order_real_pos;
        order_hypo1_pos=0;
        order_hypo2_pos=0;
        cancelSets.clear();
        for(std::map<std::string,double>::iterator it=m_orderID_mapping_orderprice.begin();it!=m_orderID_mapping_orderprice.end();)
        {
            cancelSets.insert(it->first);
        }
    }

    //撤单

    for(std::set<std::string>::const_iterator it=cancelSets.cbegin();it!=cancelSets.cend();++it)
    {
        this->strategy->cancelOrder(*it);
    }

    if(order_hypo1_pos!=0||order_hypo2_pos!=0)
    {
        if(order_hypo1_pos<0)
        {
            this->strategy->algotrading->writeAlgoTradingLog("order_hypo1_pos 仓位小于0出现了BUG");
            return ;
        }

        if(order_hypo2_pos<0)
        {
            this->strategy->algotrading->writeAlgoTradingLog("order_hypo2_pos 仓位小于0出现了BUG");
            return ;
        }
        //卖出
        std::vector<std::string>v;
        if(order_hypo1_pos!=0)
        {
            v = this->strategy->sellshort(symbol, price,order_hypo1_pos);

            for (std::vector<std::string>::const_iterator it = v.cbegin(); it != v.cend(); ++it)
            {
                m_orderID_mapping_ordervolume[*it]=order_hypo1_pos;
                m_orderID_mapping_orderprice[*it]=price;
                m_orderID_mapping_callback[*it]=false;
            }
        }

        if(order_hypo2_pos!=0)
        {
            v = this->strategy->sellshort(symbol, price-100,order_hypo2_pos);

            for (std::vector<std::string>::const_iterator it = v.cbegin(); it != v.cend(); ++it)
            {
                m_orderID_mapping_ordervolume[*it]=order_hypo2_pos;
                m_orderID_mapping_orderprice[*it]=price-100;
                m_orderID_mapping_callback[*it]=false;
            }
        }
    }
}

void TWAPAlgorithmOrder::sell(const std::string &symbol, double nowtime, double price)
{
    std::set<std::string>cancelSets;//设置要撤单的orderID

    //平多
    bool orderStop=false;
    double real=std::abs(this->strategy->getPos(symbol));

    if (real>std::abs(m_totalSupposedPos))
    {
        this->strategy->algotrading->writeAlgoTradingLog("real>std::abs(m_totalSupposedPos) 出现错误停止报单");
        orderStop=true;
    }

    double order_real_pos=0;
    for(std::map<std::string,double>::const_iterator it=m_orderID_mapping_ordervolume.cbegin();it!=m_orderID_mapping_ordervolume.cend();++it)
    {
        order_real_pos+=it->second;
    }

    double twapPos=std::min(std::abs(m_totalSupposedPos)*((nowtime-m_beginTime_t)/(m_endTime_t-m_beginTime_t)),std::abs(m_totalSupposedPos));


    double order_hypo1_pos=std::min(twapPos-(m_real_old-real),m_real_old-twapPos);
    double order_hypo1_price=price;


    double order_cancel_pos=0;

    for(std::map<std::string,double>::iterator it=m_orderID_mapping_orderprice.begin();it!=m_orderID_mapping_orderprice.end();)
    {
        if(it->second>price) //挂单价高于最新价
        {
            cancelSets.insert(it->first);
            order_cancel_pos+=m_orderID_mapping_ordervolume[it->first];
            m_orderID_mapping_ordervolume.erase(it->first);
            m_orderID_mapping_orderprice.erase(it++);
        }
        else
        {
            ++it;
        }

    }

    double pos_diff = order_hypo1_pos-order_real_pos+order_cancel_pos;
    double order_new_pos = std::max(pos_diff, 0.0);
    order_cancel_pos += std::max(-pos_diff, 0.0);

    if(pos_diff<0)
    {
        for(std::map<std::string,double>::iterator it=m_orderID_mapping_orderprice.begin();it!=m_orderID_mapping_orderprice.end();)
        {
            if(cancelSets.find(it->first)==cancelSets.end())
            {
                cancelSets.insert(it->first);
                m_orderID_mapping_ordervolume.erase(it->first);
                m_orderID_mapping_orderprice.erase(it++);
            }
            else
            {
                it++;
            }
        }
    }


    double order_hypo2_pos=std::min(twapPos - real -order_hypo1_pos,m_real_old - real - order_cancel_pos - order_hypo1_pos);

    if (orderStop || real>twapPos)
    {
        order_new_pos=0;
        order_cancel_pos=order_real_pos;
        order_hypo1_pos=0;
        order_hypo2_pos=0;
        cancelSets.clear();
        for(std::map<std::string,double>::iterator it=m_orderID_mapping_orderprice.begin();it!=m_orderID_mapping_orderprice.end();)
        {
            cancelSets.insert(it->first);
        }
    }

    //撤单

    for(std::set<std::string>::const_iterator it=cancelSets.cbegin();it!=cancelSets.cend();++it)
    {
        this->strategy->cancelOrder(*it);
    }

    if(order_hypo1_pos!=0||order_hypo2_pos!=0)
    {
        if(order_hypo1_pos<0)
        {
            this->strategy->algotrading->writeAlgoTradingLog("order_hypo1_pos 仓位小于0出现了BUG");
            return ;
        }

        if(order_hypo2_pos<0)
        {
            this->strategy->algotrading->writeAlgoTradingLog("order_hypo2_pos 仓位小于0出现了BUG");
            return ;
        }
        //卖出
        std::vector<std::string>v;
        if(order_hypo1_pos!=0)
        {
            v = this->strategy->sellshort(symbol, price,order_hypo1_pos);

            for (std::vector<std::string>::const_iterator it = v.cbegin(); it != v.cend(); ++it)
            {
                m_orderID_mapping_ordervolume[*it]=order_hypo1_pos;
                m_orderID_mapping_orderprice[*it]=price;
                m_orderID_mapping_callback[*it]=false;
            }
        }

        if(order_hypo2_pos!=0)
        {
            v = this->strategy->sellshort(symbol, price-100,order_hypo2_pos);

            for (std::vector<std::string>::const_iterator it = v.cbegin(); it != v.cend(); ++it)
            {
                m_orderID_mapping_ordervolume[*it]=order_hypo2_pos;
                m_orderID_mapping_orderprice[*it]=price-100;
                m_orderID_mapping_callback[*it]=false;
            }
        }
    }
}

void TWAPAlgorithmOrder::buycover(const std::string &symbol, double nowtime,double price)
{
    std::set<std::string>cancelSets;//设置要撤单的orderID
    //平空
    bool orderStop=false;
    double real=std::abs(this->strategy->getPos(symbol));

    if (real<m_totalSupposedPos)
    {
        this->strategy->algotrading->writeAlgoTradingLog("real<m_totalSupposedPos 出现错误停止报单");
        orderStop=true;
    }

    double order_real_pos=0;
    for(std::map<std::string,double>::const_iterator it=m_orderID_mapping_ordervolume.cbegin();it!=m_orderID_mapping_ordervolume.cend();++it)
    {
        order_real_pos+=it->second;
    }

    double twapPos=std::min(m_real_old*((nowtime-m_beginTime_t)/(m_endTime_t-m_beginTime_t)),m_real_old);


    double order_hypo1_pos=std::min(twapPos-(m_real_old-real),m_real_old-twapPos);
    double order_hypo1_price=price;


    double order_cancel_pos=0;

    for(std::map<std::string,double>::iterator it=m_orderID_mapping_orderprice.begin();it!=m_orderID_mapping_orderprice.end();)
    {
        if(it->second<price) //挂单价低于最新价
        {
            cancelSets.insert(it->first);
            order_cancel_pos+=m_orderID_mapping_ordervolume[it->first];
            m_orderID_mapping_ordervolume.erase(it->first);
            m_orderID_mapping_orderprice.erase(it++);
        }
        else
        {
            ++it;
        }

    }

    double pos_diff = order_hypo1_pos-order_real_pos+order_cancel_pos;
    double order_new_pos = std::max(pos_diff, 0.0);
    order_cancel_pos += std::max(-pos_diff, 0.0);

    if(pos_diff<0)
    {
        for(std::map<std::string,double>::iterator it=m_orderID_mapping_orderprice.begin();it!=m_orderID_mapping_orderprice.end();)
        {
            if(cancelSets.find(it->first)==cancelSets.end())
            {
                cancelSets.insert(it->first);
                m_orderID_mapping_ordervolume.erase(it->first);
                m_orderID_mapping_orderprice.erase(it++);
            }
            else
            {
                it++;
            }
        }
    }


    double order_hypo2_pos=std::min(twapPos - real -order_hypo1_pos,m_real_old - real - order_cancel_pos - order_hypo1_pos);

    if (orderStop || real>twapPos)
    {
        order_new_pos=0;
        order_cancel_pos=order_real_pos;
        order_hypo1_pos=0;
        order_hypo2_pos=0;
        cancelSets.clear();
        for(std::map<std::string,double>::iterator it=m_orderID_mapping_orderprice.begin();it!=m_orderID_mapping_orderprice.end();)
        {
            cancelSets.insert(it->first);
        }
    }

    //撤单

    for(std::set<std::string>::const_iterator it=cancelSets.cbegin();it!=cancelSets.cend();++it)
    {
        this->strategy->cancelOrder(*it);
    }

    if(order_hypo1_pos!=0||order_hypo2_pos!=0)
    {
        if(order_hypo1_pos<0)
        {
            this->strategy->algotrading->writeAlgoTradingLog("order_hypo1_pos 仓位小于0出现了BUG");
            return ;
        }

        if(order_hypo2_pos<0)
        {
            this->strategy->algotrading->writeAlgoTradingLog("order_hypo2_pos 仓位小于0出现了BUG");
            return ;
        }
        //买入
        std::vector<std::string>v;
        if(order_hypo1_pos!=0)
        {
            v = this->strategy->buycover(symbol, price,order_hypo1_pos);

            for (std::vector<std::string>::const_iterator it = v.cbegin(); it != v.cend(); ++it)
            {
                m_orderID_mapping_ordervolume[*it]=order_hypo1_pos;
                m_orderID_mapping_orderprice[*it]=price;
                m_orderID_mapping_callback[*it]=false;
            }
        }

        if(order_hypo2_pos!=0)
        {
            v = this->strategy->buycover(symbol, price+100,order_hypo2_pos);

            for (std::vector<std::string>::const_iterator it = v.cbegin(); it != v.cend(); ++it)
            {
                m_orderID_mapping_ordervolume[*it]=order_hypo2_pos;
                m_orderID_mapping_orderprice[*it]=price+100;
                m_orderID_mapping_callback[*it]=false;
            }
        }
    }
}

void TWAPAlgorithmOrder::cancelAllorders()
{
    for(std::map<std::string,double>::iterator iter=m_orderID_mapping_orderprice.begin();iter!=m_orderID_mapping_orderprice.cend();)
    {
        this->strategy->cancelOrder(iter->first);
    }
    m_orderID_mapping_orderprice.clear();
    m_orderID_mapping_ordervolume.clear();
}


