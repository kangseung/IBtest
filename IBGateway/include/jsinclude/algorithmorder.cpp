#include"algorithmorder.h"
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif
AlgorithmOrder::AlgorithmOrder(StrategyTemplate*strategy)
{
    this->is_backtest = false;
    this->is_callback = true;
    this->unitVolume = 20;
    this->strategy = strategy;
}

AlgorithmOrder::~AlgorithmOrder()
{

}

void AlgorithmOrder::setUnitVolume(double unitVolume)//for strategy
{
    this->unitVolume = unitVolume;
}

void AlgorithmOrder::setTargetPos(const std::string &symbol, double volume)//for strategy
{
    std::unique_lock<std::mutex>lck(targetPosMtx);
    if (bannedSymbols.find(symbol) == bannedSymbols.end())
    {
        this->targetPosMap[symbol] = volume;
    }
    else
    {
        strategy->algotrading->writeAlgoTradingLog("setTargetPos禁止该合约:"+symbol);
    }
}

void AlgorithmOrder::setBacktestMode()//for backtestengine
{
    this->is_backtest = true;
}

void AlgorithmOrder::setCallback()
{
    this->is_callback = true;
}

void AlgorithmOrder::setBannedSymbol(const std::string &symbol)
{
    bannedSymbols.insert(symbol);
}

void AlgorithmOrder::checkPosTick(const std::shared_ptr<Event_Tick> &tick)
{
    if (is_backtest == true)
    {
        this->unitVolume = 9999999;
    }
    std::unique_lock<std::mutex>lck(targetPosMtx);
    if (this->targetPosMap[tick->symbol] == this->strategy->getPos(tick->symbol))
    {
        return;
    }

    if (tick->bidprice1 == tick->upperLimit || tick->askprice1 == tick->lowerLimit)
    {
        return;
    }

    std::unique_lock<std::mutex>lck2(symbol_mapping_orderIDlistmtx);
    if (this->symbol_mapping_orderIDlist.find(tick->symbol) != this->symbol_mapping_orderIDlist.end())
    {
        for (std::vector<std::string>::const_iterator it = this->symbol_mapping_orderIDlist[tick->symbol].cbegin(); it != this->symbol_mapping_orderIDlist[tick->symbol].cend(); ++it)
        {
            this->strategy->cancelOrder(*it);
        }
        this->symbol_mapping_orderIDlist[tick->symbol].clear();
    }
    else
    {
        std::vector<std::string>vector;
        this->symbol_mapping_orderIDlist.insert(std::pair<std::string, std::vector<std::string>>(tick->symbol, vector));
    }

    if (this->is_callback == true)
    {
        if (this->strategy->getPos(tick->symbol) == 0)//no position
        {
            if (this->targetPosMap[tick->symbol] > this->strategy->getPos(tick->symbol))//buy
            {
                std::vector<std::string>v;
                v = this->strategy->buy(tick->symbol, tick->askprice1, (std::min)((this->targetPosMap[tick->symbol] - this->strategy->getPos(tick->symbol)), unitVolume));
                if (this->strategy->trading == true)
                {
                    this->is_callback = false;
                    starttime = clock();
                }
                for (std::vector<std::string>::const_iterator it = v.cbegin(); it != v.cend(); ++it)
                {
                    this->symbol_mapping_orderIDlist[tick->symbol].push_back(*it);
                }
            }
            else if (this->targetPosMap[tick->symbol] < this->strategy->getPos(tick->symbol))//sell
            {
                std::vector<std::string>v;
                v = this->strategy->sellshort(tick->symbol, tick->bidprice1, (std::min)((this->strategy->getPos(tick->symbol) - this->targetPosMap[tick->symbol]), unitVolume));
                if (this->strategy->trading == true)
                {
                    this->is_callback = false;
                    starttime = clock();
                }
                for (std::vector<std::string>::const_iterator it = v.cbegin(); it != v.cend(); ++it)
                {
                    this->symbol_mapping_orderIDlist[tick->symbol].push_back(*it);
                }
            }
        }
        else if (this->strategy->getPos(tick->symbol)>0)// long position
        {
            if (this->targetPosMap[tick->symbol] > this->strategy->getPos(tick->symbol))//add position
            {
                std::vector<std::string>v;
                v = this->strategy->buy(tick->symbol, tick->askprice1, (std::min)((this->targetPosMap[tick->symbol] - this->strategy->getPos(tick->symbol)), unitVolume));
                if (this->strategy->trading == true)
                {
                    this->is_callback = false;
                    starttime = clock();
                }
                for (std::vector<std::string>::const_iterator it = v.cbegin(); it != v.cend(); ++it)
                {
                    this->symbol_mapping_orderIDlist[tick->symbol].push_back(*it);
                }
            }
            else if (this->targetPosMap[tick->symbol] < this->strategy->getPos(tick->symbol))
            {
                if (this->targetPosMap[tick->symbol] >= 0)//substract position
                {
                    std::vector<std::string>v;
                    v = this->strategy->sell(tick->symbol, tick->bidprice1, (std::min)((this->strategy->getPos(tick->symbol) - this->targetPosMap[tick->symbol]), unitVolume));
                    if (this->strategy->trading == true)
                    {
                        this->is_callback = false;
                        starttime = clock();
                    }
                    for (std::vector<std::string>::const_iterator it = v.cbegin(); it != v.cend(); ++it)
                    {
                        this->symbol_mapping_orderIDlist[tick->symbol].push_back(*it);
                    }
                }
                else//close all position
                {
                    std::vector<std::string>v;
                    v = this->strategy->sell(tick->symbol, tick->bidprice1, (std::min)((this->strategy->getPos(tick->symbol)), unitVolume));
                    if (this->strategy->trading == true)
                    {
                        this->is_callback = false;
                        starttime = clock();
                    }
                    for (std::vector<std::string>::const_iterator it = v.cbegin(); it != v.cend(); ++it)
                    {
                        this->symbol_mapping_orderIDlist[tick->symbol].push_back(*it);
                    }
                }
            }
        }
        else if (this->strategy->getPos(tick->symbol) < 0)// short position
        {
            if (this->targetPosMap[tick->symbol] < this->strategy->getPos(tick->symbol))//add  position
            {
                std::vector<std::string>v;
                v = this->strategy->sellshort(tick->symbol, tick->bidprice1, (std::min)(this->strategy->getPos(tick->symbol) - this->targetPosMap[tick->symbol], unitVolume));
                if (this->strategy->trading == true)
                {
                    this->is_callback = false;
                    starttime = clock();
                }
                for (std::vector<std::string>::const_iterator it = v.cbegin(); it != v.cend(); ++it)
                {
                    this->symbol_mapping_orderIDlist[tick->symbol].push_back(*it);
                }
            }
            else if (this->targetPosMap[tick->symbol] > this->strategy->getPos(tick->symbol))
            {
                if (this->targetPosMap[tick->symbol] <= 0)//substract position
                {
                    std::vector<std::string>v;
                    v = this->strategy->buycover(tick->symbol, tick->askprice1, (std::min)(this->targetPosMap[tick->symbol] - this->strategy->getPos(tick->symbol), unitVolume));
                    if (this->strategy->trading == true)
                    {
                        this->is_callback = false;
                        starttime = clock();
                    }
                    for (std::vector<std::string>::const_iterator it = v.cbegin(); it != v.cend(); ++it)
                    {
                        this->symbol_mapping_orderIDlist[tick->symbol].push_back(*it);
                    }
                }
                else//close all position
                {
                    std::vector<std::string>v;
                    v = this->strategy->buycover(tick->symbol, tick->askprice1, (std::min)(std::abs(this->strategy->getPos(tick->symbol)), unitVolume));
                    if (this->strategy->trading == true)
                    {
                        this->is_callback = false;
                        starttime = clock();
                    }
                    for (std::vector<std::string>::const_iterator it = v.cbegin(); it != v.cend(); ++it)
                    {
                        this->symbol_mapping_orderIDlist[tick->symbol].push_back(*it);
                    }
                }
            }
        }
    }
    else
    {
        clock_t end = clock();
        double passseconds = (double)(end - starttime) / CLOCKS_PER_SEC;
        if (passseconds >= 20)
        {
            this->is_callback = true;
        }
    }
}

void AlgorithmOrder::checkPosBar(const jsstructs::BarData &bar)
{
    if (is_backtest == true)
    {
        this->unitVolume = 9999999;

        std::unique_lock<std::mutex>lck(targetPosMtx);
        if (this->targetPosMap[bar.symbol] == this->strategy->getPos(bar.symbol))
        {
            return;
        }

        std::unique_lock<std::mutex>lck2(symbol_mapping_orderIDlistmtx);
        if (this->symbol_mapping_orderIDlist.find(bar.symbol) == this->symbol_mapping_orderIDlist.end())
        {
            std::vector<std::string>vector;
            this->symbol_mapping_orderIDlist.insert(std::pair<std::string, std::vector<std::string>>(bar.symbol, vector));
        }

        for (std::vector<std::string>::const_iterator it = this->symbol_mapping_orderIDlist[bar.symbol].cbegin(); it != this->symbol_mapping_orderIDlist[bar.symbol].cend(); ++it)
        {
            this->strategy->cancelOrder(*it);
        }
        this->symbol_mapping_orderIDlist[bar.symbol].clear();

        if (this->is_callback == true)
        {
            if (this->strategy->getPos(bar.symbol) == 0)//no position
            {
                if (this->targetPosMap[bar.symbol] > this->strategy->getPos(bar.symbol))//buy
                {
                    std::vector<std::string>v;
                    v = this->strategy->buy(bar.symbol, bar.close + 100, (std::min)((this->targetPosMap[bar.symbol] - this->strategy->getPos(bar.symbol)), unitVolume));
                    if (this->strategy->trading == true)
                    {
                        this->is_callback = false;
                    }
                    for (std::vector<std::string>::const_iterator it = v.cbegin(); it != v.cend(); ++it)
                    {
                        this->symbol_mapping_orderIDlist[bar.symbol].push_back(*it);
                    }
                }
                else if (this->targetPosMap[bar.symbol] < this->strategy->getPos(bar.symbol))//sell
                {
                    std::vector<std::string>v;
                    v = this->strategy->sellshort(bar.symbol, bar.close - 100, (std::min)((this->strategy->getPos(bar.symbol) - this->targetPosMap[bar.symbol]), unitVolume));
                    if (this->strategy->trading == true)
                    {
                        this->is_callback = false;
                    }
                    for (std::vector<std::string>::const_iterator it = v.cbegin(); it != v.cend(); ++it)
                    {
                        this->symbol_mapping_orderIDlist[bar.symbol].push_back(*it);
                    }
                }
            }
            else if (this->strategy->getPos(bar.symbol)>0)// long position
            {
                if (this->targetPosMap[bar.symbol] > this->strategy->getPos(bar.symbol))//add position
                {
                    std::vector<std::string>v;
                    v = this->strategy->buy(bar.symbol, bar.close + 100, (std::min)((this->targetPosMap[bar.symbol] - this->strategy->getPos(bar.symbol)), unitVolume));
                    if (this->strategy->trading == true)
                    {
                        this->is_callback = false;
                    }
                    for (std::vector<std::string>::const_iterator it = v.cbegin(); it != v.cend(); ++it)
                    {
                        this->symbol_mapping_orderIDlist[bar.symbol].push_back(*it);
                    }
                }
                else if (this->targetPosMap[bar.symbol] < this->strategy->getPos(bar.symbol))
                {
                    if (this->targetPosMap[bar.symbol] >= 0)//substract position
                    {
                        std::vector<std::string>v;
                        v = this->strategy->sell(bar.symbol, bar.close - 100, (std::min)((this->strategy->getPos(bar.symbol) - this->targetPosMap[bar.symbol]), unitVolume));
                        if (this->strategy->trading == true)
                        {
                            this->is_callback = false;
                        }
                        for (std::vector<std::string>::const_iterator it = v.cbegin(); it != v.cend(); ++it)
                        {
                            this->symbol_mapping_orderIDlist[bar.symbol].push_back(*it);
                        }
                    }
                    else//close all position
                    {
                        std::vector<std::string>v;
                        v = this->strategy->sell(bar.symbol, bar.close - 100,(std::min)((this->strategy->getPos(bar.symbol)), unitVolume));
                        if (this->strategy->trading == true)
                        {
                            this->is_callback = false;
                        }
                        for (std::vector<std::string>::const_iterator it = v.cbegin(); it != v.cend(); ++it)
                        {
                            this->symbol_mapping_orderIDlist[bar.symbol].push_back(*it);
                        }
                    }
                }
            }
            else if (this->strategy->getPos(bar.symbol) < 0)// short position
            {
                if (this->targetPosMap[bar.symbol] < this->strategy->getPos(bar.symbol))//add  position
                {
                    std::vector<std::string>v;
                    v = this->strategy->sellshort(bar.symbol, bar.close - 100, (std::min)(this->strategy->getPos(bar.symbol) - this->targetPosMap[bar.symbol], unitVolume));
                    if (this->strategy->trading == true)
                    {
                        this->is_callback = false;
                    }
                    for (std::vector<std::string>::const_iterator it = v.cbegin(); it != v.cend(); ++it)
                    {
                        this->symbol_mapping_orderIDlist[bar.symbol].push_back(*it);
                    }
                }
                else if (this->targetPosMap[bar.symbol] > this->strategy->getPos(bar.symbol))
                {
                    if (this->targetPosMap[bar.symbol] <= 0)//substract position
                    {
                        std::vector<std::string>v;
                        v = this->strategy->buycover(bar.symbol, bar.close + 100, (std::min)(this->targetPosMap[bar.symbol] - this->strategy->getPos(bar.symbol), unitVolume));
                        if (this->strategy->trading == true)
                        {
                            this->is_callback = false;
                        }
                        for (std::vector<std::string>::const_iterator it = v.cbegin(); it != v.cend(); ++it)
                        {
                            this->symbol_mapping_orderIDlist[bar.symbol].push_back(*it);
                        }
                    }
                    else//close all position
                    {
                        std::vector<std::string>v;
                        v = this->strategy->buycover(bar.symbol, bar.close + 100, (std::min)(std::abs(this->strategy->getPos(bar.symbol)), unitVolume));
                        if (this->strategy->trading == true)
                        {
                            this->is_callback = false;
                        }
                        for (std::vector<std::string>::const_iterator it = v.cbegin(); it != v.cend(); ++it)
                        {
                            this->symbol_mapping_orderIDlist[bar.symbol].push_back(*it);
                        }
                    }
                }
            }
        }
    }
}
