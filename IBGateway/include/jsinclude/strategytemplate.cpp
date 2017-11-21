#include"strategyTemplate.h"
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

void StrategyData::insertParam(const std::string &key, const std::string &value)
{
    std::unique_lock<std::mutex>lck(mtx);
    this->paramMap[key] = value;
}

void StrategyData::insertVar(const std::string &key, const std::string &value)
{
    std::unique_lock<std::mutex>lck(mtx);
    this->varMap[key] = value;
}

std::string StrategyData::getParam(const std::string& key)
{
    std::unique_lock<std::mutex>lck(mtx);
    if (this->paramMap.find(key) != this->paramMap.end())
    {
        return this->paramMap[key];
    }
    else
    {
        return "Null";
    }
}

std::string StrategyData::getVar(const std::string &key)
{
    std::unique_lock<std::mutex>lck(mtx);
    if (this->varMap.find(key) != this->varMap.end())
    {
        return this->varMap[key];
    }
    else
    {
        return "Null";
    }
}

inline std::map<std::string, std::string>StrategyData::getParamMap() const
{
    std::unique_lock<std::mutex>lck(mtx);
    return this->paramMap;
}

inline std::map<std::string, std::string>StrategyData::getVarMap() const
{
    std::unique_lock<std::mutex>lck(mtx);
    return this->varMap;
}

StrategyTemplate::StrategyTemplate(AlgoTradingAPI *algotrading) :inited(false), trading(false)
{
    this->algotrading = algotrading;
    this->algorithmorder = new AlgorithmOrder(this);
    std::vector<std::string>ninetoeleven = { "bu", "rb", "hc", "ru" };
    this->ninetoeleven.insert(ninetoeleven.begin(), ninetoeleven.end()); //9点到11点的合约列表
    std::vector<std::string>ninetohalfeleven = { "p", "j", "m", "y", "a", "b", "jm", "i", "SR", "CF", "RM", "MA", "ZC", "FG", "OI", "CY" };
    this->ninetohalfeleven.insert(ninetohalfeleven.begin(), ninetohalfeleven.end()); //9点到11点半的合约
    std::vector<std::string>ninetoone = { "cu", "al", "zn", "pb", "sn", "ni" };
    this->ninetoone.insert(ninetoone.begin(), ninetoone.end()); //9点到1点的合约列表
    std::vector<std::string>ninetohalftwo = { "ag", "au" };
    this->ninetohalftwo.insert(ninetohalftwo.begin(), ninetohalftwo.end()); //9点到2点半的合约
    std::vector<std::string>treasury_futures = { "TF" };
    this->treasury_futures.insert(treasury_futures.begin(), treasury_futures.end()); //国债到下午三点十五分
}

StrategyTemplate::~StrategyTemplate()
{
    delete this->algorithmorder;
}

void  StrategyTemplate::onInit()
{
    if(this->inited==false)
    {
        this->algotrading->writeAlgoTradingLog("策略初始化" + this->getParam("name"));
        std::vector<std::string>symbollist = Utils::split(this->getParam("symbol"), ",");
        if (this->getParam("mode") == "bar")
        {
            std::vector<jsstructs::BarData>alldatalist;
            for (std::vector<std::string>::const_iterator it = symbollist.cbegin(); it != symbollist.cend(); ++it)
            {
                this->algotrading->writeAlgoTradingLog("策略" + this->getParam("name")+"初始化提取的合约为"+*it);
                std::vector<jsstructs::BarData>datalist = this->algotrading->loadBar(this->barDbName, *it, std::stoi(this->getParam("initdays")),0);
                for (std::vector<jsstructs::BarData>::const_iterator it = datalist.cbegin(); it != datalist.cend(); ++it)
                {
                    alldatalist.push_back(*it);
                }
            }

            std::sort(alldatalist.begin(), alldatalist.end(), BarGreater());

            for (std::vector<jsstructs::BarData>::const_iterator it = alldatalist.cbegin(); it != alldatalist.cend(); ++it)
            {
                if (it == alldatalist.cend() - 1)
                {
                    this->onBar_template(*it, true);
                }
                else
                {
                    this->onBar_template(*it, false);
                }
            }
        }
        else if (this->getParam("mode") == "tick")
        {
            std::vector<std::shared_ptr<Event_Tick>>alldatalist;
            for (std::vector<std::string>::const_iterator it = symbollist.cbegin(); it != symbollist.cend(); ++it)
            {
                this->algotrading->writeAlgoTradingLog("策略" + this->getParam("name")+"初始化提取的合约为"+*it);
                std::vector<std::shared_ptr<Event_Tick>>datalist = this->algotrading->loadTick(tickDbName, *it, std::stoi(this->getParam("initdays")));
                for (std::vector<std::shared_ptr<Event_Tick>>::const_iterator it = datalist.cbegin(); it != datalist.cend(); ++it)
                {
                    alldatalist.push_back(*it);
                }
            }

            std::sort(alldatalist.begin(), alldatalist.end(), TickGreater());

            for (std::vector<std::shared_ptr<Event_Tick>>::const_iterator it = alldatalist.cbegin(); it != alldatalist.cend(); ++it)
            {
                this->onTick_template(*it);
            }
        }
        this->loadMongoDB();  //读取持仓
        std::map<std::string, double>map = this->getPosMap();
        for (std::map<std::string, double>::iterator iter = map.begin(); iter != map.end(); iter++)
        {
            this->algorithmorder->setTargetPos(iter->first, iter->second);
            this->algorithmorder->setTargetPos(iter->first, iter->second);
        }
        this->inited = true;
        this->putEvent();
    }
    else
    {
        this->algotrading->writeAlgoTradingLog("策略" + this->getParam("name")+"已经初始化，无需再次初始化");
    }
}

void  StrategyTemplate::onStart()
{
    this->trading = true;
    this->algotrading->writeAlgoTradingLog("策略" + this->getParam("name") + "启动");
    this->putEvent();
}

void  StrategyTemplate::onStop()
{
    this->trading = false;
    this->algotrading->writeAlgoTradingLog("策略" + this->getParam("name") + "停止");
    this->putEvent();
}

void  StrategyTemplate::cancelOrder(const std::string &orderID) //不可以在onorder里头回调撤销list
{
    this->algotrading->cancelOrder(orderID, "ctp");
}

void StrategyTemplate::cancelOrder(std::vector<std::string> cancelList)
{
    for(std::vector<std::string>::const_iterator iter=cancelList.cbegin();iter!=cancelList.cend();++iter)
    {
        this->algotrading->cancelOrder(*iter,"ctp");
    }
}

void  StrategyTemplate::cancelOrders()
{
    std::unique_lock<std::mutex>lck(orderListmtx);
    for (std::vector<std::string>::iterator it = this->orderList.begin(); it != this->orderList.end(); ++it)
    {
        this->cancelOrder(*it);
    }
}

void  StrategyTemplate::onTick_template(const std::shared_ptr<Event_Tick>&tick)
{
    if (barHourmap.find(tick->symbol) == barHourmap.end())
    {
        barHourmap[tick->symbol] = 99;
    }
    if (barMinutemap.find(tick->symbol) == barMinutemap.end())
    {
        barMinutemap[tick->symbol] = 99;
    }
    if (barmap.find(tick->symbol) == barmap.end())
    {
        jsstructs::BarData bar;
        bar.symbol = tick->symbol;
        bar.exchange = tick->exchange;
        bar.open = 0;
        bar.high = 0;
        bar.low = 0;
        bar.close = 0;
        bar.openPrice = tick->openPrice;//今日开
        bar.highPrice = tick->highPrice;//今日高
        bar.lowPrice = tick->lowPrice;//今日低
        bar.preClosePrice = tick->preClosePrice;//昨收
        bar.upperLimit = tick->upperLimit;//涨停
        bar.lowerLimit = tick->lowerLimit;//跌停
        bar.volume = tick->volume;
        bar.openInterest = tick->openInterest;
        bar.date = tick->date;
        bar.time = tick->time;
        bar.setUnixDatetime();
        this->barmap[tick->symbol] = bar;
    }

    std::string symbolHead = Utils::regexSymbol(tick->symbol);
    if (tick->getMinute() != this->barMinutemap.at(tick->symbol) || tick->getHour() != this->barHourmap.at(tick->symbol)) {
        if (!((this->barHourmap.at(tick->symbol) == 11 && this->barMinutemap.at(tick->symbol) == 30) || (this->barHourmap.at(tick->symbol) == 10 && this->barMinutemap.at(tick->symbol) == 15)))
        { //剔除10点15分11点半下午3点的一根TICK合成出来的K线
            if (this->ninetoeleven.find(symbolHead) != this->ninetoeleven.end()) {
                if (this->barHourmap.at(tick->symbol) == 23) {
                    this->barHourmap.at(tick->symbol) = 99;
                    this->barmap.erase(tick->symbol);
                    return;
                }
            }
            else if (this->ninetohalfeleven.find(symbolHead) != this->ninetohalfeleven.end()) {
                if ((this->barHourmap.at(tick->symbol) == 23 && this->barMinutemap.at(tick->symbol) == 30) || (this->barHourmap.at(tick->symbol) == 15 && this->barMinutemap.at(tick->symbol) == 00)) {
                    this->barHourmap.at(tick->symbol) = 99;
                    this->barMinutemap.at(tick->symbol) = 99;
                    this->barmap.erase(tick->symbol);
                    return;
                }
            }
            else if (this->ninetoone.find(symbolHead) != this->ninetoone.end()) {
                if ((this->barHourmap.at(tick->symbol) == 1) || (this->barHourmap.at(tick->symbol) == 15 && this->barMinutemap.at(tick->symbol) == 00)) {
                    this->barHourmap.at(tick->symbol) = 99;
                    this->barmap.erase(tick->symbol);
                    return;
                }
            }
            else if (this->ninetohalftwo.find(symbolHead) != this->ninetohalftwo.end()) {
                if ((this->barHourmap.at(tick->symbol) == 2 && this->barMinutemap.at(tick->symbol) == 30) || (this->barHourmap.at(tick->symbol) == 15 && this->barMinutemap.at(tick->symbol) == 00)) {
                    this->barHourmap.at(tick->symbol) = 99;
                    this->barMinutemap.at(tick->symbol) = 99;
                    this->barmap.erase(tick->symbol);
                    return;
                }
            }
            else if (this->treasury_futures.find(symbolHead) != this->treasury_futures.end())
            {
                if (this->barHourmap.at(tick->symbol) == 15 && this->barMinutemap.at(tick->symbol) == 15) {
                    this->barHourmap.at(tick->symbol) = 99;
                    this->barMinutemap.at(tick->symbol) = 99;
                    this->barmap.erase(tick->symbol);
                    return;
                }
            }

            if (this->barHourmap.at(tick->symbol) == 15 && this->barMinutemap.at(tick->symbol) == 00)
            {
                if (this->treasury_futures.find(symbolHead) == this->treasury_futures.end())
                {
                    this->barHourmap.at(tick->symbol) = 99;
                    this->barMinutemap.at(tick->symbol) = 99;
                    this->barmap.erase(tick->symbol);
                    return;
                }
                else
                {
                    onBar_template(this->barmap.at(tick->symbol), true);
                }
            }
            else
            {
                onBar_template(this->barmap.at(tick->symbol), true);
            }
        }
        jsstructs::BarData bar;
        bar.symbol = tick->symbol;
        bar.exchange = tick->exchange;
        bar.open = tick->lastprice;
        bar.high = tick->lastprice;
        bar.low = tick->lastprice;
        bar.close = tick->lastprice;
        bar.openPrice = tick->openPrice;//今日开
        bar.highPrice = tick->highPrice;//今日高
        bar.lowPrice = tick->lowPrice;//今日低
        bar.preClosePrice = tick->preClosePrice;//昨收
        bar.upperLimit = tick->upperLimit;//涨停
        bar.lowerLimit = tick->lowerLimit;//跌停
        bar.volume = tick->volume;
        bar.openInterest = tick->openInterest;
        bar.date = tick->date;
        bar.time = tick->time;
        bar.setUnixDatetime();
        this->barmap[tick->symbol] = bar;
        this->barMinutemap[tick->symbol] = tick->getMinute();
        this->barHourmap[tick->symbol] = tick->getHour();
    }
    else
    {
        this->barmap[tick->symbol].high = (std::max)(this->barmap[tick->symbol].high, tick->lastprice);
        this->barmap[tick->symbol].low = (std::min)(this->barmap[tick->symbol].low, tick->lastprice);
        this->barmap[tick->symbol].close = tick->lastprice;
        this->barmap[tick->symbol].highPrice = tick->highPrice;
        this->barmap[tick->symbol].lowPrice = tick->lowPrice;
        this->barmap[tick->symbol].openInterest = tick->openInterest;
        this->barmap[tick->symbol].volume += tick->volume;
    }
    this->onTick(tick);
    this->putEvent();
}

void  StrategyTemplate::onTick(const std::shared_ptr<Event_Tick>&tick)
{

}

void  StrategyTemplate::onBar_template(const jsstructs::BarData &bar, bool inited)
{
    if (bar.close != 0)
    {
        onBar(bar);
        this->algotrading->PutBarChart(bar, this->backtestgoddata, inited);
    }

}

void  StrategyTemplate::onBar(const jsstructs::BarData &bar)
{

}

void  StrategyTemplate::onOrder_template(const std::shared_ptr<Event_Order>&order)
{
    if (order->status == STATUS_CANCELLED)
    {
        this->algorithmorder->setCallback();
    }
    onOrder(order);
}

void  StrategyTemplate::onTrade_template(const std::shared_ptr<Event_Trade>&trade)
{
    this->algorithmorder->setCallback();
    onTrade(trade);
    saveMongoDB();
}

void  StrategyTemplate::onOrder(const std::shared_ptr<Event_Order>&order)
{

}

void  StrategyTemplate::onTrade(const std::shared_ptr<Event_Trade>&trade)
{

}

void StrategyTemplate::loadStrategyEvent()
{
    std::shared_ptr<Event_LoadStrategy>e = std::make_shared<Event_LoadStrategy>();
    e->parammap = strategyData.getParamMap();
    e->varmap = strategyData.getVarMap();
    e->strategyname = strategyData.getParam("name");
    this->algotrading->PutEvent(e);
}

void StrategyTemplate::updateStrategyEvent()
{
    std::shared_ptr<Event_UpdateStrategy>e = std::make_shared<Event_UpdateStrategy>();
    e->parammap = this->strategyData.getParamMap();
    e->varmap = this->strategyData.getVarMap();
    e->strategyname = this->getParam("name");
    this->algotrading->PutEvent(e);
}

void  StrategyTemplate::addParam(const std::string &paramName, const std::string &paramValue)
{
    this->strategyData.insertParam(paramName, paramValue);
}

std::string  StrategyTemplate::getParam(const std::string &paramName)
{
    return this->strategyData.getParam(paramName);
}

void  StrategyTemplate::modifyPos(const std::string &symbol, double pos)
{
    std::unique_lock<std::mutex>lck(posMapmtx);
    this->posMap[symbol] = pos;
}

double  StrategyTemplate::getPos(const std::string &symbol)
{
    std::unique_lock<std::mutex>lck(posMapmtx);
    if (this->posMap.find(symbol) != this->posMap.end())
    {
        return this->posMap[symbol];
    }
    else
    {
        return 0;
    }
}

std::map<std::string, double> StrategyTemplate::getPosMap()
{
    std::unique_lock<std::mutex>lck(posMapmtx);
    return this->posMap;
}

//做多
std::vector<std::string>  StrategyTemplate::buy(const std::string &symbol, double price, double volume)
{
    return this->sendOrder(symbol, ALGOBUY, price, volume);
}
//平多
std::vector<std::string>  StrategyTemplate::sell(const std::string &symbol, double price, double volume)
{
    return this->sendOrder(symbol, ALGOSELL, price, volume);
}
//做空
std::vector<std::string>  StrategyTemplate::sellshort(const std::string &symbol, double price, double volume)
{
    return this->sendOrder(symbol, ALGOSHORT, price, volume);
}
//平空
std::vector<std::string>  StrategyTemplate::buycover(const std::string &symbol, double price, double volume)
{
    return this->sendOrder(symbol, ALGOCOVER, price, volume);
}

void StrategyTemplate::showStrategy()
{
    this->strategyData.insertVar("inited", Utils::Bool2String(inited));
    this->strategyData.insertVar("trading", Utils::Bool2String(trading));
    std::map<std::string, double>map = getPosMap();
    for (std::map<std::string, double>::iterator iter = map.begin(); iter != map.end(); iter++)
    {
        this->strategyData.insertVar(("pos_" + iter->first), std::to_string(iter->second));
    }

    std::shared_ptr<Event_LoadStrategy>e = std::make_shared<Event_LoadStrategy>();
    e->parammap = strategyData.getParamMap();
    e->varmap = strategyData.getVarMap();
    e->strategyname = strategyData.getParam("name");
    this->algotrading->PutEvent(e);
}

void  StrategyTemplate::putEvent()
{
    this->strategyData.insertVar("inited", Utils::Bool2String(inited));
    this->strategyData.insertVar("trading", Utils::Bool2String(trading));
    std::map<std::string, double>map = getPosMap();
    for (std::map<std::string, double>::iterator iter = map.begin(); iter != map.end(); iter++)
    {
        this->strategyData.insertVar(("pos_" + iter->first), std::to_string(iter->second));
    }
    //将参数和变量传递到界面上去
    std::shared_ptr<Event_UpdateStrategy>e = std::make_shared<Event_UpdateStrategy>();
    e->parammap = this->strategyData.getParamMap();
    e->varmap = this->strategyData.getVarMap();
    e->strategyname = this->getParam("name");
    this->algotrading->PutEvent(e);
}

std::vector<std::string>  StrategyTemplate::sendOrder(const std::string &symbol, const std::string &orderType, double price, double volume)
{
    std::vector<std::string>orderIDv;
    if (trading == true)
    {
        orderIDv = this->algotrading->sendOrder(symbol, orderType, price, volume, this);
        for (std::vector<std::string>::const_iterator it = orderIDv.cbegin(); it != orderIDv.cend(); ++it)
        {
            std::unique_lock<std::mutex>lck(orderListmtx);
            this->orderList.push_back(*it);
        }
        return orderIDv;
    }
    return orderIDv;
}

void  StrategyTemplate::saveMongoDB()
{
    if(isbacktestmode==false)
    {
        mongoc_client_t     *client = mongoc_client_pool_pop(this->algotrading->m_pool);//取出
        mongoc_collection_t *collection = mongoc_client_get_collection(client, "strategypos","pos");//连接
        bson_t *doc = bson_new();//doc
        bson_error_t error;

        std::map<std::string, double>map = this->getPosMap();
        BSON_APPEND_UTF8(doc,      "strategyname",this->strategyData.getParam("name").c_str());
        for (std::map<std::string, double>::iterator it = map.begin(); it != map.end(); it++)
        {
            BSON_APPEND_DOUBLE(doc,    it->first.c_str(),it->second);
        }

        bson_t query;
        bson_init(&query);
        BSON_APPEND_UTF8(&query,      "strategyname",this->strategyData.getParam("name").c_str());
        // 将bson文档插入到集合
        if (!mongoc_collection_update(collection, MONGOC_UPDATE_UPSERT, &query, doc, NULL, &error))
        {
            std::cout<<"savemongodb更新持仓出错"<<error.message<<std::endl;
        }
        bson_destroy(&query);
        bson_destroy(doc);//销毁doc
        mongoc_collection_destroy(collection);//销毁连接
        mongoc_client_pool_push(this->algotrading->m_pool, client);//放回
    }
}

void  StrategyTemplate::loadMongoDB()
{
    if(isbacktestmode==false)
    {
        bson_t query;
        bson_init(&query);
        BSON_APPEND_UTF8(&query,      "strategyname",this->strategyData.getParam("name").c_str());
        // 将bson文档插入到集合

        // 从客户端池中获取一个客户端
        mongoc_client_t    *client = mongoc_client_pool_pop(this->algotrading->m_pool);																				//取一个mongo连接
        mongoc_collection_t *collection  = mongoc_client_get_collection(client,"strategypos","pos");

        mongoc_cursor_t *cursor = mongoc_collection_find(collection, MONGOC_QUERY_NONE, 0, 0, 0, &query, NULL, NULL);

        bson_error_t error;
        const bson_t *doc;

        while (mongoc_cursor_next(cursor, &doc))
        {
            char *str=str = bson_as_json(doc, NULL);
            std::string s = str;
            std::string err;

            auto json = json11::Json::parse(s, err);
            if (!err.empty())
            {
                bson_destroy(&query);
                mongoc_cursor_destroy(cursor);
                mongoc_client_pool_push(this->algotrading->m_pool, client);																						//放回一个mongo连接
            }
            auto mongo_posmap = json.object_items();

            std::map<std::string, double>strategy_posmap = this->getPosMap();
            for (std::map<std::string, json11::Json>::iterator it = mongo_posmap.begin(); it != mongo_posmap.end(); it++)
            {
                if (strategy_posmap.find(it->first) != strategy_posmap.end())
                {
                    this->modifyPos(it->first, it->second.number_value());
                }
            }
            //		printf("%s\n", str);
            bson_free(str);
        }

        if (mongoc_cursor_error(cursor, &error)) {
            fprintf(stderr, "An error occurred: %s\n", error.message);
        }
        bson_destroy(&query);
        mongoc_cursor_destroy(cursor);
        mongoc_client_pool_push(this->algotrading->m_pool, client);																						//放回一个mongo连接
    }
}
