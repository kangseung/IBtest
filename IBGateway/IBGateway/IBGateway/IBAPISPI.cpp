#include"StdAfx.h"

#include"IBAPISPI.h"


#ifdef WIN32
# include <Windows.h>
# define sleep( seconds) Sleep( seconds * 1000);
#else
# include <unistd.h>
#endif


#include"EClientSocket.h"
#include "EPosixClientSocketPlatform.h"

#include "Contract.h"
#include "Order.h"
#include "OrderState.h"
#include "Execution.h"
#include "CommissionReport.h"
#include "ContractSamples.h"
#include "OrderSamples.h"
#include "ScannerSubscription.h"
#include "ScannerSubscriptionSamples.h"
#include "executioncondition.h"
#include "PriceCondition.h"
#include "MarginCondition.h"
#include "PercentChangeCondition.h"
#include "TimeCondition.h"
#include "VolumeCondition.h"
#include "AvailableAlgoParams.h"
#include "FAMethodSamples.h"
#include "CommonDefs.h"
#include "AccountSummaryTags.h"

#include <stdio.h>
#include <iostream>
#include <thread>
#include <ctime>
#include <vector>
#include <string>
#include "utils.h"

const int PING_DEADLINE = 2; // seconds
const int SLEEP_BETWEEN_PINGS = 30; // seconds

IBAPISPI::IBAPISPI(IBGateway *ibgateway)
{
	this->m_ibgateway = ibgateway;
	m_osSignal = 2000;//2-seconds timeout;
	esocket = new ESocket;
	m_Client_API = new EClientSocket(this, esocket, &m_osSignal);
	m_extraAuth = false; //是否额外验证？ 估计大概是验证码之类的玩意儿
	m_sleepDeadline = 0;//可能是判断掉线用的？
	m_orderId = 0;//long 型 
}

IBAPISPI::~IBAPISPI()
{
	if (m_Client_API != NULL)
	{
		delete m_Client_API;
	}

	if (esocket != NULL)
	{
		delete esocket;
	}
}

bool IBAPISPI::connect(const char *host, unsigned int port, int clientId)
{
	// trying to connect
	printf("Connecting to %s:%d clientId:%d\n", !(host && *host) ? "127.0.0.1" : host, port, clientId);

	bool bRes = m_Client_API->eConnect(host, port, clientId, m_extraAuth);//是否额外验证？ 估计大概是验证码之类的玩意儿

	if (bRes) {
		printf("Connected to %s:%d clientId:%d\n", m_Client_API->host().c_str(), m_Client_API->port(), clientId);

		m_Reader = new EReader(m_Client_API, &m_osSignal);

		m_Reader->start();

		auto f = std::bind(&IBAPISPI::processMessages, this);
		m_queueWorker = new std::thread(f);
	}
	else
		printf("Cannot connect to %s:%d clientId:%d\n", m_Client_API->host().c_str(), m_Client_API->port(), clientId);

	return bRes;
}

void IBAPISPI::disconnect() const
{
	m_Client_API->eDisconnect();

	if (m_queueWorker != NULL)
	{
		m_queueWorker->join();
		delete m_queueWorker;
	}

	if (m_Reader != NULL)
	{
		delete m_Reader;
	}
	printf("Disconnected\n");
}

bool IBAPISPI::isConnected() const
{
	return m_Client_API->isConnected();
}

void IBAPISPI::setConnectOptions(const std::string& connectOptions)
{
	m_Client_API->setConnectOptions(connectOptions);
}

void IBAPISPI::processMessages()
{
	while (m_Client_API->isConnected())
	{
		m_Reader->checkClient();
		m_osSignal.waitForSignal();
		m_Reader->processMsgs();
	}
}

//////////////////////////////////////////////////////////////////
// methods
//! [connectack]
void IBAPISPI::connectAck() {
	if (!m_extraAuth && m_Client_API->asyncEConnect())
		m_Client_API->startApi();
}
//! [connectack]

void IBAPISPI::reqCurrentTime()
{
	printf("Requesting Current Time\n");

	// set ping deadline to "now + n seconds"
	m_sleepDeadline = time(NULL) + PING_DEADLINE;

	m_state = ST_PING_ACK;

	m_Client_API->reqCurrentTime();
}

void IBAPISPI::tickDataOperation()
{
	/*** Requesting real time market data ***/
	sleep(1);
	//! [reqmktdata]
	m_Client_API->reqMktData(1001, ContractSamples::StockComboContract(), "", false, TagValueListSPtr());
	m_Client_API->reqMktData(1002, ContractSamples::OptionWithLoacalSymbol(), "", false, TagValueListSPtr());
	//! [reqmktdata]
	//! [reqmktdata_snapshot]
	m_Client_API->reqMktData(1003, ContractSamples::FutureComboContract(), "", true, TagValueListSPtr());
	//! [reqmktdata_snapshot]

	//! [reqmktdata_genticks]
	//Requesting RTVolume (Time & Sales), shortable and Fundamental Ratios generic ticks
	m_Client_API->reqMktData(1004, ContractSamples::USStock(), "233,236,258", false, TagValueListSPtr());
	//! [reqmktdata_genticks]

	//! [reqmktdata_contractnews]
	m_Client_API->reqMktData(1005, ContractSamples::USStock(), "mdoff,292:BZ", false, TagValueListSPtr());
	m_Client_API->reqMktData(1006, ContractSamples::USStock(), "mdoff,292:BT", false, TagValueListSPtr());
	m_Client_API->reqMktData(1007, ContractSamples::USStock(), "mdoff,292:FLY", false, TagValueListSPtr());
	m_Client_API->reqMktData(1008, ContractSamples::USStock(), "mdoff,292:MT", false, TagValueListSPtr());
	//! [reqmktdata_contractnews]
	//! [reqmktdata_broadtapenews]
	m_Client_API->reqMktData(1009, ContractSamples::BTbroadtapeNewsFeed(), "mdoff,292", false, TagValueListSPtr());
	m_Client_API->reqMktData(1010, ContractSamples::BZbroadtapeNewsFeed(), "mdoff,292", false, TagValueListSPtr());
	m_Client_API->reqMktData(1011, ContractSamples::FLYbroadtapeNewsFeed(), "mdoff,292", false, TagValueListSPtr());
	m_Client_API->reqMktData(1012, ContractSamples::MTbroadtapeNewsFeed(), "mdoff,292", false, TagValueListSPtr());
	//! [reqmktdata_broadtapenews]

	//! [reqoptiondatagenticks]
	//Requesting data for an option contract will return the greek values
	m_Client_API->reqMktData(1005, ContractSamples::USOptionContract(), "", false, TagValueListSPtr());
	//! [reqoptiondatagenticks]

	sleep(1);
	/*** Canceling the market data subscription ***/
	//! [cancelmktdata]
	m_Client_API->cancelMktData(1001);
	m_Client_API->cancelMktData(1002);
	m_Client_API->cancelMktData(1003);
	//! [cancelmktdata]

	m_state = ST_TICKDATAOPERATION_ACK;
}

void IBAPISPI::marketDepthOperations()
{
	/*** Requesting the Deep Book ***/
	//! [reqmarketdepth]
	m_Client_API->reqMktDepth(2001, ContractSamples::EurGbpFx(), 5, TagValueListSPtr());
	//! [reqmarketdepth]
	sleep(2);
	/*** Canceling the Deep Book request ***/
	//! [cancelmktdepth]
	m_Client_API->cancelMktDepth(2001);
	//! [cancelmktdepth]

	m_state = ST_MARKETDEPTHOPERATION_ACK;
}

void IBAPISPI::realTimeBars(const std::string &symbol, const Contract&contract, int barSize)
{
	TickerId id = m_tickerId++;
	m_Client_API->reqRealTimeBars(id, contract, barSize, "TRADES", true, TagValueListSPtr());
	m_tickerID_mapping_symbol[id] = symbol;
}

void IBAPISPI::marketDataType()
{
	//! [reqmarketdatatype]
	/*** Switch to live (1) frozen (2) delayed (3) or delayed frozen (4)***/
	m_Client_API->reqMarketDataType(2);
	//! [reqmarketdatatype]

	m_state = ST_MARKETDATATYPE_ACK;
}

void IBAPISPI::historicalDataRequests(const std::string &symbol, const Contract& contract, const std::string& durationStr, const std::string&  barSizeSetting)
{
	/*** Requesting historical data ***/
	//! [reqhistoricaldata]
	std::time_t rawtime;
	std::tm* timeinfo;
	char queryTime[80];

	std::time(&rawtime);
	timeinfo = std::localtime(&rawtime);
	std::strftime(queryTime, 80, "%Y%m%d %H:%M:%S", timeinfo);

	TickerId id = m_tickerId++;

	m_Client_API->reqHistoricalData(id, contract, queryTime, durationStr, barSizeSetting, "TRADES", 1, 1, TagValueListSPtr());//(id, contract, queryTime, "1 M", "1 day", "MIDPOINT", 1, 1, TagValueListSPtr());
	m_tickerID_mapping_symbol[id] = symbol;

	/*** Canceling historical data requests ***/
	//m_Client_API->cancelHistoricalData(4001);
	//m_Client_API->cancelHistoricalData(4002);
}

void IBAPISPI::optionsOperations()
{
	//! [reqsecdefoptparams]
	m_Client_API->reqSecDefOptParams(0, "IBM", "", "STK", 8314);
	//! [reqsecdefoptparams]

	//! [calculateimpliedvolatility]
	m_Client_API->calculateImpliedVolatility(5001, ContractSamples::NormalOption(), 5, 85);
	//! [calculateimpliedvolatility]

	//** Canceling implied volatility ***
	m_Client_API->cancelCalculateImpliedVolatility(5001);

	//! [calculateoptionprice]
	m_Client_API->calculateOptionPrice(5002, ContractSamples::NormalOption(), 0.22, 85);
	//! [calculateoptionprice]

	//** Canceling option's price calculation ***
	m_Client_API->cancelCalculateOptionPrice(5002);

	//! [exercise_options]
	//** Exercising options ***
	m_Client_API->exerciseOptions(5003, ContractSamples::OptionWithTradingClass(), 1, 1, "", 1);
	//! [exercise_options]

	m_state = ST_OPTIONSOPERATIONS_ACK;
}

void IBAPISPI::contractOperations()
{
	m_Client_API->reqContractDetails(209, ContractSamples::EurGbpFx());
	sleep(2);
	//! [reqcontractdetails]
	m_Client_API->reqContractDetails(210, ContractSamples::OptionForQuery());
	//! [reqcontractdetails]

	//! [reqcontractdetailsnews]
	m_Client_API->reqContractDetails(211, ContractSamples::NewsFeedForQuery());
	//! [reqcontractdetailsnews]

	m_state = ST_CONTRACTOPERATION_ACK;
}

void IBAPISPI::marketScanners()
{
	/*** Requesting all available parameters which can be used to build a scanner request ***/
	//! [reqscannerparameters]
	m_Client_API->reqScannerParameters();
	//! [reqscannerparameters]
	sleep(2);

	/*** Triggering a scanner subscription ***/
	//! [reqscannersubscription]
	m_Client_API->reqScannerSubscription(7001, ScannerSubscriptionSamples::HotUSStkByVolume(), TagValueListSPtr());
	//! [reqscannersubscription]

	sleep(2);
	/*** Canceling the scanner subscription ***/
	//! [cancelscannersubscription]
	m_Client_API->cancelScannerSubscription(7001);
	//! [cancelscannersubscription]

	m_state = ST_MARKETSCANNERS_ACK;
}

void IBAPISPI::reutersFundamentals()
{
	/*** Requesting Fundamentals ***/
	//! [reqfundamentaldata]
	m_Client_API->reqFundamentalData(8001, ContractSamples::USStock(), "ReportsFinSummary");
	//! [reqfundamentaldata]
	sleep(2);

	/*** Canceling fundamentals request ***/
	//! [cancelfundamentaldata]
	m_Client_API->cancelFundamentalData(8001);
	//! [cancelfundamentaldata]

	m_state = ST_REUTERSFUNDAMENTALS_ACK;
}

void IBAPISPI::bulletins()
{
	/*** Requesting Interactive Broker's news bulletins */
	//! [reqnewsbulletins]
	m_Client_API->reqNewsBulletins(true);
	//! [reqnewsbulletins]
	sleep(2);
	/*** Canceling IB's news bulletins ***/
	//! [cancelnewsbulletins]
	m_Client_API->cancelNewsBulletins();
	//! [cancelnewsbulletins]

	m_state = ST_BULLETINS_ACK;
}

void IBAPISPI::accountOperations()
{
	/*** Requesting managed accounts***/
	//! [reqmanagedaccts]
	m_Client_API->reqManagedAccts();
	//! [reqmanagedaccts]
	sleep(2);
	/*** Requesting accounts' summary ***/
	//! [reqaaccountsummary]
	m_Client_API->reqAccountSummary(9001, "All", AccountSummaryTags::getAllTags());
	//! [reqaaccountsummary]
	sleep(2);
	//! [reqaaccountsummaryledger]
	m_Client_API->reqAccountSummary(9002, "All", "$LEDGER");
	//! [reqaaccountsummaryledger]
	sleep(2);
	//! [reqaaccountsummaryledgercurrency]
	m_Client_API->reqAccountSummary(9003, "All", "$LEDGER:EUR");
	//! [reqaaccountsummaryledgercurrency]
	sleep(2);
	//! [reqaaccountsummaryledgerall]
	m_Client_API->reqAccountSummary(9004, "All", "$LEDGER:ALL");
	//! [reqaaccountsummaryledgerall]
	sleep(2);
	//! [cancelaaccountsummary]
	m_Client_API->cancelAccountSummary(9001);
	m_Client_API->cancelAccountSummary(9002);
	m_Client_API->cancelAccountSummary(9003);
	m_Client_API->cancelAccountSummary(9004);
	//! [cancelaaccountsummary]
	sleep(2);
	/*** Subscribing to an account's information. Only one at a time! ***/
	//! [reqaaccountupdates]
	m_Client_API->reqAccountUpdates(true, "U150462");
	//! [reqaaccountupdates]
	sleep(2);
	//! [cancelaaccountupdates]
	m_Client_API->reqAccountUpdates(false, "U150462");
	//! [cancelaaccountupdates]
	sleep(2);

	//! [reqaaccountupdatesmulti]
	m_Client_API->reqAccountUpdatessMulti(9002, "U150462", "EUstocks", true);
	//! [reqaaccountupdatesmulti]
	sleep(2);

	/*** Requesting all accounts' positions. ***/
	//! [reqpositions]
	m_Client_API->reqPositions();
	//! [reqpositions]
	sleep(2);
	//! [cancelpositions]
	m_Client_API->cancelPositions();
	//! [cancelpositions]

	//! [reqpositionsmulti]
	m_Client_API->reqPositionsMulti(9003, "U150462", "EUstocks");
	//! [reqpositionsmulti]

	m_state = ST_ACCOUNTOPERATIONS_ACK;
}

void IBAPISPI::orderOperations()
{
	/*** Requesting the next valid id ***/
	//! [reqids]
	//The parameter is always ignored.
	m_Client_API->reqIds(-1);
	//! [reqids]
	//! [reqallopenorders]
	m_Client_API->reqAllOpenOrders();
	//! [reqallopenorders]
	//! [reqautoopenorders]
	m_Client_API->reqAutoOpenOrders(true);
	//! [reqautoopenorders]
	//! [reqopenorders]
	m_Client_API->reqOpenOrders();
	//! [reqopenorders]

	/*** Placing/modifying an order - remember to ALWAYS increment the nextValidId after placing an order so it can be used for the next one! ***/
	//! [order_submission]
	m_Client_API->placeOrder(m_orderId++, ContractSamples::USStock(), OrderSamples::LimitOrder("SELL", 1, 50));
	//! [order_submission]

	//m_Client_API->placeOrder(m_orderId++, ContractSamples::OptionAtBox(), OrderSamples::Block("BUY", 50, 20));
	//m_Client_API->placeOrder(m_orderId++, ContractSamples::OptionAtBox(), OrderSamples::BoxTop("SELL", 10));
	//m_Client_API->placeOrder(m_orderId++, ContractSamples::FutureComboContract(), OrderSamples::ComboLimitOrder("SELL", 1, 1, false));
	//m_Client_API->placeOrder(m_orderId++, ContractSamples::StockComboContract(), OrderSamples::ComboMarketOrder("BUY", 1, false));
	//m_Client_API->placeOrder(m_orderId++, ContractSamples::OptionComboContract(), OrderSamples::ComboMarketOrder("BUY", 1, true));
	//m_Client_API->placeOrder(m_orderId++, ContractSamples::StockComboContract(), OrderSamples::LimitOrderForComboWithLegPrices("BUY", 1, std::vector<double>(10, 5), true));
	//m_Client_API->placeOrder(m_orderId++, ContractSamples::USStock(), OrderSamples::Discretionary("SELL", 1, 45, 0.5));
	//m_Client_API->placeOrder(m_orderId++, ContractSamples::OptionAtBox(), OrderSamples::LimitIfTouched("BUY", 1, 30, 34));
	//m_Client_API->placeOrder(m_orderId++, ContractSamples::USStock(), OrderSamples::LimitOnClose("SELL", 1, 34));
	//m_Client_API->placeOrder(m_orderId++, ContractSamples::USStock(), OrderSamples::LimitOnOpen("BUY", 1, 35));
	//m_Client_API->placeOrder(m_orderId++, ContractSamples::USStock(), OrderSamples::MarketIfTouched("BUY", 1, 35));
	//m_Client_API->placeOrder(m_orderId++, ContractSamples::USStock(), OrderSamples::MarketOnClose("SELL", 1));
	//m_Client_API->placeOrder(m_orderId++, ContractSamples::USStock(), OrderSamples::MarketOnOpen("BUY", 1));
	//m_Client_API->placeOrder(m_orderId++, ContractSamples::USStock(), OrderSamples::MarketOrder("SELL", 1));
	//m_Client_API->placeOrder(m_orderId++, ContractSamples::USStock(), OrderSamples::MarketToLimit("BUY", 1));
	//m_Client_API->placeOrder(m_orderId++, ContractSamples::OptionAtIse(), OrderSamples::MidpointMatch("BUY", 1));
	//m_Client_API->placeOrder(m_orderId++, ContractSamples::USStock(), OrderSamples::Stop("SELL", 1, 34.4));
	//m_Client_API->placeOrder(m_orderId++, ContractSamples::USStock(), OrderSamples::StopLimit("BUY", 1, 35, 33));
	//m_Client_API->placeOrder(m_orderId++, ContractSamples::USStock(), OrderSamples::StopWithProtection("SELL", 1, 45));
	//m_Client_API->placeOrder(m_orderId++, ContractSamples::USStock(), OrderSamples::SweepToFill("BUY", 1, 35));
	//m_Client_API->placeOrder(m_orderId++, ContractSamples::USStock(), OrderSamples::TrailingStop("SELL", 1, 0.5, 30));
	//m_Client_API->placeOrder(m_orderId++, ContractSamples::USStock(), OrderSamples::TrailingStopLimit("BUY", 1, 50, 5, 30));

	/*** Cancel all orders for all accounts ***/
	m_Client_API->reqGlobalCancel();

	/*** Request the day's executions ***/
	//! [reqexecutions]
	m_Client_API->reqExecutions(10001, ExecutionFilter());
	//! [reqexecutions]

	m_state = ST_ORDEROPERATIONS_ACK;
}

void IBAPISPI::ocaSamples()
{
	//OCA ORDER
	//! [ocasubmit]
	std::vector<Order> ocaOrders;
	ocaOrders.push_back(OrderSamples::LimitOrder("BUY", 1, 10));
	ocaOrders.push_back(OrderSamples::LimitOrder("BUY", 1, 11));
	ocaOrders.push_back(OrderSamples::LimitOrder("BUY", 1, 12));
	for (unsigned int i = 0; i < ocaOrders.size(); i++){
		OrderSamples::OneCancelsAll("TestOca", ocaOrders[i], 2);
		m_Client_API->placeOrder(m_orderId++, ContractSamples::USStock(), ocaOrders[i]);
	}
	//! [ocasubmit]

	m_state = ST_OCASAMPLES_ACK;
}

void IBAPISPI::conditionSamples()
{
	//! [order_conditioning_activate]
	Order lmt = OrderSamples::LimitOrder("BUY", 100, 10);
	//Order will become active if conditioning criteria is met
	PriceCondition* priceCondition = dynamic_cast<PriceCondition *>(OrderSamples::Price_Condition(208813720, "SMART", 600, false, false));
	ExecutionCondition* execCondition = dynamic_cast<ExecutionCondition *>(OrderSamples::Execution_Condition("EUR.USD", "CASH", "IDEALPRO", true));
	MarginCondition* marginCondition = dynamic_cast<MarginCondition *>(OrderSamples::Margin_Condition(30, true, false));
	PercentChangeCondition* pctChangeCondition = dynamic_cast<PercentChangeCondition *>(OrderSamples::Percent_Change_Condition(15.0, 208813720, "SMART", true, true));
	TimeCondition* timeCondition = dynamic_cast<TimeCondition *>(OrderSamples::Time_Condition("20160118 23:59:59", true, false));
	VolumeCondition* volumeCondition = dynamic_cast<VolumeCondition *>(OrderSamples::Volume_Condition(208813720, "SMART", false, 100, true));

	lmt.conditions.push_back(ibapi::shared_ptr<PriceCondition>(priceCondition));
	lmt.conditions.push_back(ibapi::shared_ptr<ExecutionCondition>(execCondition));
	lmt.conditions.push_back(ibapi::shared_ptr<MarginCondition>(marginCondition));
	lmt.conditions.push_back(ibapi::shared_ptr<PercentChangeCondition>(pctChangeCondition));
	lmt.conditions.push_back(ibapi::shared_ptr<TimeCondition>(timeCondition));
	lmt.conditions.push_back(ibapi::shared_ptr<VolumeCondition>(volumeCondition));
	m_Client_API->placeOrder(m_orderId++, ContractSamples::USStock(), lmt);
	//! [order_conditioning_activate]

	//Conditions can make the order active or cancel it. Only LMT orders can be conditionally canceled.
	//! [order_conditioning_cancel]
	Order lmt2 = OrderSamples::LimitOrder("BUY", 100, 20);
	//The active order will be cancelled if conditioning criteria is met
	lmt2.conditionsCancelOrder = true;
	PriceCondition* priceCondition2 = dynamic_cast<PriceCondition *>(OrderSamples::Price_Condition(208813720, "SMART", 600, false, false));
	lmt2.conditions.push_back(ibapi::shared_ptr<PriceCondition>(priceCondition2));
	m_Client_API->placeOrder(m_orderId++, ContractSamples::EuropeanStock(), lmt2);
	//! [order_conditioning_cancel]

	m_state = ST_CONDITIONSAMPLES_ACK;
}

void IBAPISPI::bracketSample(){
	Order parent;
	Order takeProfit;
	Order stopLoss;
	//! [bracketsubmit]
	OrderSamples::BracketOrder(m_orderId++, parent, takeProfit, stopLoss, "BUY", 100, 30, 40, 20);
	m_Client_API->placeOrder(parent.orderId, ContractSamples::EuropeanStock(), parent);
	m_Client_API->placeOrder(takeProfit.orderId, ContractSamples::EuropeanStock(), takeProfit);
	m_Client_API->placeOrder(stopLoss.orderId, ContractSamples::EuropeanStock(), stopLoss);
	//! [bracketsubmit]

	m_state = ST_BRACKETSAMPLES_ACK;
}

void IBAPISPI::hedgeSample(){
	//F Hedge order
	//! [hedgesubmit]
	//Parent order on a contract which currency differs from your base currency
	Order parent = OrderSamples::LimitOrder("BUY", 100, 10);
	parent.orderId = m_orderId++;
	//Hedge on the currency conversion
	Order hedge = OrderSamples::MarketFHedge(parent.orderId, "BUY");
	//Place the parent first...
	m_Client_API->placeOrder(parent.orderId, ContractSamples::EuropeanStock(), parent);
	//Then the hedge order
	m_Client_API->placeOrder(m_orderId++, ContractSamples::EurGbpFx(), hedge);
	//! [hedgesubmit]

	m_state = ST_HEDGESAMPLES_ACK;
}

void IBAPISPI::testAlgoSamples(){
	//! [algo_base_order]
	Order baseOrder = OrderSamples::LimitOrder("BUY", 1000, 1);
	//! [algo_base_order]

	//! [arrivalpx]
	AvailableAlgoParams::FillArrivalPriceParams(baseOrder, 0.1, "Aggressive", "09:00:00 CET", "16:00:00 CET", true, true);
	m_Client_API->placeOrder(m_orderId++, ContractSamples::USStockAtSmart(), baseOrder);
	//! [arrivalpx]

	//! [darkice]
	AvailableAlgoParams::FillDarkIceParams(baseOrder, 10, "09:00:00 CET", "16:00:00 CET", true);
	m_Client_API->placeOrder(m_orderId++, ContractSamples::USStockAtSmart(), baseOrder);
	//! [darkice]

	//! [ad]
	AvailableAlgoParams::FillAccumulateDistributeParams(baseOrder, 10, 60, true, true, 1, true, true, "09:00:00 CET", "16:00:00 CET");
	m_Client_API->placeOrder(m_orderId++, ContractSamples::USStockAtSmart(), baseOrder);
	//! [ad]

	//! [twap]
	AvailableAlgoParams::FillTwapParams(baseOrder, "Marketable", "09:00:00 CET", "16:00:00 CET", true);
	m_Client_API->placeOrder(m_orderId++, ContractSamples::USStockAtSmart(), baseOrder);
	//! [twap]

	//! [vwap]
	AvailableAlgoParams::FillBalanceImpactRiskParams(baseOrder, 0.1, "Aggressive", true);
	m_Client_API->placeOrder(m_orderId++, ContractSamples::USStockAtSmart(), baseOrder);
	//! [vwap]

	//! [balanceimpactrisk]
	AvailableAlgoParams::FillBalanceImpactRiskParams(baseOrder, 0.1, "Aggressive", true);
	m_Client_API->placeOrder(m_orderId++, ContractSamples::USStockAtSmart(), baseOrder);
	//! [balanceimpactrisk]

	//! [minimpact]
	AvailableAlgoParams::FillMinImpactParams(baseOrder, 0.3);
	m_Client_API->placeOrder(m_orderId++, ContractSamples::USStockAtSmart(), baseOrder);
	//! [minimpact]

	//! [adaptive]
	AvailableAlgoParams::FillAdaptiveParams(baseOrder, "Normal");
	m_Client_API->placeOrder(m_orderId++, ContractSamples::USStockAtSmart(), baseOrder);
	//! [adaptive]

	m_state = ST_TESTALGOSAMPLES_ACK;
}

void IBAPISPI::financialAdvisorOrderSamples()
{
	//! [faorderoneaccount]
	Order faOrderOneAccount = OrderSamples::MarketOrder("BUY", 100);
	// Specify the Account Number directly
	faOrderOneAccount.account = "DU119915";
	m_Client_API->placeOrder(m_orderId++, ContractSamples::USStock(), faOrderOneAccount);
	//! [faorderoneaccount]
	sleep(1);

	//! [faordergroupequalquantity]
	Order faOrderGroupEQ = OrderSamples::LimitOrder("SELL", 200, 2000);
	faOrderGroupEQ.faGroup = "Group_Equal_Quantity";
	faOrderGroupEQ.faMethod = "EqualQuantity";
	m_Client_API->placeOrder(m_orderId++, ContractSamples::SimpleFuture(), faOrderGroupEQ);
	//! [faordergroupequalquantity]
	sleep(1);

	//! [faordergrouppctchange]
	Order faOrderGroupPC;
	faOrderGroupPC.action = "BUY";
	faOrderGroupPC.orderType = "MKT";
	// You should not specify any order quantity for PctChange allocation method
	faOrderGroupPC.faGroup = "Pct_Change";
	faOrderGroupPC.faMethod = "PctChange";
	faOrderGroupPC.faPercentage = "100";
	m_Client_API->placeOrder(m_orderId++, ContractSamples::EurGbpFx(), faOrderGroupPC);
	//! [faordergrouppctchange]
	sleep(1);

	//! [faorderprofile]
	Order faOrderProfile = OrderSamples::LimitOrder("BUY", 200, 100);
	faOrderProfile.faProfile = "Percent_60_40";
	m_Client_API->placeOrder(m_orderId++, ContractSamples::EuropeanStock(), faOrderProfile);
	//! [faorderprofile]

	m_state = ST_FAORDERSAMPLES_ACK;
}

void IBAPISPI::financialAdvisorOperations()
{
	/*** Requesting FA information ***/
	//! [requestfaaliases]
	m_Client_API->requestFA(faDataType::ALIASES);
	//! [requestfaaliases]

	//! [requestfagroups]
	m_Client_API->requestFA(faDataType::GROUPS);
	//! [requestfagroups]

	//! [requestfaprofiles]
	m_Client_API->requestFA(faDataType::PROFILES);
	//! [requestfaprofiles]

	/*** Replacing FA information - Fill in with the appropriate XML string. ***/
	//! [replacefaonegroup]
	m_Client_API->replaceFA(faDataType::GROUPS, FAMethodSamples::FAOneGroup());
	//! [replacefaonegroup]

	//! [replacefatwogroups]
	m_Client_API->replaceFA(faDataType::GROUPS, FAMethodSamples::FATwoGroups());
	//! [replacefatwogroups]

	//! [replacefaoneprofile]
	m_Client_API->replaceFA(faDataType::PROFILES, FAMethodSamples::FAOneProfile());
	//! [replacefaoneprofile]

	//! [replacefatwoprofiles]
	m_Client_API->replaceFA(faDataType::PROFILES, FAMethodSamples::FATwoProfiles());
	//! [replacefatwoprofiles]

	m_state = ST_FAOPERATIONS_ACK;
}

void IBAPISPI::testDisplayGroups(){
	//! [querydisplaygroups]
	m_Client_API->queryDisplayGroups(9001);
	//! [querydisplaygroups]

	sleep(1);

	//! [subscribetogroupevents]
	m_Client_API->subscribeToGroupEvents(9002, 1);
	//! [subscribetogroupevents]

	sleep(1);

	//! [updatedisplaygroup]
	m_Client_API->updateDisplayGroup(9002, "8314@SMART");
	//! [updatedisplaygroup]

	sleep(1);

	//! [subscribefromgroupevents]
	m_Client_API->unsubscribeFromGroupEvents(9002);
	//! [subscribefromgroupevents]

	m_state = ST_TICKDATAOPERATION_ACK;
}

void IBAPISPI::miscelaneous()
{
	/*** Request TWS' current time ***/
	m_Client_API->reqCurrentTime();
	/*** Setting TWS logging level  ***/
	m_Client_API->setServerLogLevel(5);

	m_state = ST_MISCELANEOUS_ACK;
}

//! [nextvalidid]
void IBAPISPI::nextValidId(OrderId orderId)
{
	printf("Next Valid Id: %ld\n", orderId);
	m_orderId = orderId;
	//! [nextvalidid]

	//m_state = ST_TICKDATAOPERATION;
	//m_state = ST_MARKETDEPTHOPERATION;
	//m_state = ST_REALTIMEBARS;
	//m_state = ST_MARKETDATATYPE;
	//m_state = ST_HISTORICALDATAREQUESTS;
	//m_state = ST_CONTRACTOPERATION;
	//m_state = ST_MARKETSCANNERS;
	//m_state = ST_REUTERSFUNDAMENTALS;
	//m_state = ST_BULLETINS;
	//m_state = ST_ACCOUNTOPERATIONS;
	//m_state = ST_ORDEROPERATIONS;
	//m_state = ST_OCASAMPLES;
	//m_state = ST_CONDITIONSAMPLES;
	//m_state = ST_BRACKETSAMPLES;
	//m_state = ST_HEDGESAMPLES;
	//m_state = ST_TESTALGOSAMPLES;
	//m_state = ST_FAORDERSAMPLES;
	//m_state = ST_FAOPERATIONS;
	//m_state = ST_DISPLAYGROUPS;
	//m_state = ST_MISCELANEOUS;
	//m_state = ST_PING;
}


void IBAPISPI::currentTime(long time)
{
	if (m_state == ST_PING_ACK) {
		time_t t = (time_t)time;
		struct tm * timeinfo = localtime(&t);
		printf("The current date/time is: %s", asctime(timeinfo));

		time_t now = ::time(NULL);
		m_sleepDeadline = now + SLEEP_BETWEEN_PINGS;

		m_state = ST_PING_ACK;
	}
}

//! [error]
void IBAPISPI::error(const int id, const int errorCode, const std::string errorString)
{
	printf("Error. Id: %d, Code: %d, Msg: %s\n", id, errorCode, errorString.c_str());
}
//! [error]

//! [tickprice]
void IBAPISPI::tickPrice(TickerId tickerId, TickType field, double price, int canAutoExecute) {
	printf("Tick Price. Ticker Id: %ld, Field: %d, Price: %g, CanAutoExecute: %d\n", tickerId, (int)field, price, canAutoExecute);
}
//! [tickprice]

//! [ticksize]
void IBAPISPI::tickSize(TickerId tickerId, TickType field, int size) {
	printf("Tick Size. Ticker Id: %ld, Field: %d, Size: %d\n", tickerId, (int)field, size);
}
//! [ticksize]

//! [tickoptioncomputation]
void IBAPISPI::tickOptionComputation(TickerId tickerId, TickType tickType, double impliedVol, double delta,
	double optPrice, double pvDividend,
	double gamma, double vega, double theta, double undPrice) {
	printf("TickOptionComputation. Ticker Id: %ld, Type: %d, ImpliedVolatility: %g, Delta: %g, OptionPrice: %g, pvDividend: %g, Gamma: %g, Vega: %g, Theta: %g, Underlying Price: %g\n", tickerId, (int)tickType, impliedVol, delta, optPrice, pvDividend, gamma, vega, theta, undPrice);
}
//! [tickoptioncomputation]

//! [tickgeneric]
void IBAPISPI::tickGeneric(TickerId tickerId, TickType tickType, double value) {
	printf("Tick Generic. Ticker Id: %ld, Type: %d, Value: %g\n", tickerId, (int)tickType, value);
}
//! [tickgeneric]

//! [tickstring]
void IBAPISPI::tickString(TickerId tickerId, TickType tickType, const std::string& value) {
	printf("Tick String. Ticker Id: %ld, Type: %d, Value: %s\n", tickerId, (int)tickType, value.c_str());
}
//! [tickstring]

void IBAPISPI::tickEFP(TickerId tickerId, TickType tickType, double basisPoints, const std::string& formattedBasisPoints,
	double totalDividends, int holdDays, const std::string& futureLastTradeDate, double dividendImpact, double dividendsToLastTradeDate) {
	printf("TickEFP. %ld, Type: %d, BasisPoints: %g, FormattedBasisPoints: %s, Total Dividends: %g, HoldDays: %d, Future Last Trade Date: %s, Dividend Impact: %g, Dividends To Last Trade Date: %g\n", tickerId, (int)tickType, basisPoints, formattedBasisPoints.c_str(), totalDividends, holdDays, futureLastTradeDate.c_str(), dividendImpact, dividendsToLastTradeDate);
}

//! [orderstatus]
void IBAPISPI::orderStatus(OrderId orderId, const std::string& status, double filled,
	double remaining, double avgFillPrice, int permId, int parentId,
	double lastFillPrice, int clientId, const std::string& whyHeld){
	printf("OrderStatus. Id: %ld, Status: %s, Filled: %g, Remaining: %g, AvgFillPrice: %g, PermId: %d, LastFillPrice: %g, ClientId: %d, WhyHeld: %s\n", orderId, status.c_str(), filled, remaining, avgFillPrice, permId, lastFillPrice, clientId, whyHeld.c_str());
}
//! [orderstatus]

//! [openorder]
void IBAPISPI::openOrder(OrderId orderId, const Contract& contract, const Order& order, const OrderState& ostate) {
	printf("OpenOrder. ID: %ld, %s, %s @ %s: %s, %s, %g, %s\n", orderId, contract.symbol.c_str(), contract.secType.c_str(), contract.exchange.c_str(), order.action.c_str(), order.orderType.c_str(), order.totalQuantity, ostate.status.c_str());
}
//! [openorder]

//! [openorderend]
void IBAPISPI::openOrderEnd() {
	printf("OpenOrderEnd\n");
}
//! [openorderend]

void IBAPISPI::winError(const std::string& str, int lastError) {}
void IBAPISPI::connectionClosed() {
	printf("Connection Closed\n");
}

//! [updateaccountvalue]
void IBAPISPI::updateAccountValue(const std::string& key, const std::string& val,
	const std::string& currency, const std::string& accountName) {
	printf("UpdateAccountValue. Key: %s, Value: %s, Currency: %s, Account Name: %s\n", key.c_str(), val.c_str(), currency.c_str(), accountName.c_str());
}
//! [updateaccountvalue]

//! [updateportfolio]
void IBAPISPI::updatePortfolio(const Contract& contract, double position,
	double marketPrice, double marketValue, double averageCost,
	double unrealizedPNL, double realizedPNL, const std::string& accountName){
	printf("UpdatePortfolio. %s, %s @ %s: Position: %g, MarketPrice: %g, MarketValue: %g, AverageCost: %g, UnrealisedPNL: %g, RealisedPNL: %g, AccountName: %s\n", (contract.symbol).c_str(), (contract.secType).c_str(), (contract.primaryExchange).c_str(), position, marketPrice, marketValue, averageCost, unrealizedPNL, realizedPNL, accountName.c_str());
}
//! [updateportfolio]

//! [updateaccounttime]
void IBAPISPI::updateAccountTime(const std::string& timeStamp) {
	printf("UpdateAccountTime. Time: %s\n", timeStamp.c_str());
}
//! [updateaccounttime]

//! [accountdownloadend]
void IBAPISPI::accountDownloadEnd(const std::string& accountName) {
	printf("Account download finished: %s\n", accountName.c_str());
}
//! [accountdownloadend]

//! [contractdetails]
void IBAPISPI::contractDetails(int reqId, const ContractDetails& contractDetails) {
	printf("ContractDetails. ReqId: %d - %s, %s, ConId: %ld @ %s, Trading Hours: %s, Liquidation Hours: %s\n", reqId, contractDetails.summary.symbol.c_str(), contractDetails.summary.secType.c_str(), contractDetails.summary.conId, contractDetails.summary.exchange.c_str(), contractDetails.tradingHours.c_str(), contractDetails.liquidHours.c_str());
}
//! [contractdetails]

void IBAPISPI::bondContractDetails(int reqId, const ContractDetails& contractDetails) {
	printf("Bond. ReqId: %d, Symbol: %s, Security Type: %s, Currency: %s, Trading Hours: %s, Liquidation Hours: %s\n", reqId, contractDetails.summary.symbol.c_str(), contractDetails.summary.secType.c_str(), contractDetails.summary.currency.c_str(), contractDetails.tradingHours.c_str(), contractDetails.liquidHours.c_str());
}

//! [contractdetailsend]
void IBAPISPI::contractDetailsEnd(int reqId) {
	printf("ContractDetailsEnd. %d\n", reqId);
}
//! [contractdetailsend]

//! [execdetails]
void IBAPISPI::execDetails(int reqId, const Contract& contract, const Execution& execution) {
	printf("ExecDetails. ReqId: %d - %s, %s, %s - %s, %ld, %g\n", reqId, contract.symbol.c_str(), contract.secType.c_str(), contract.currency.c_str(), execution.execId.c_str(), execution.orderId, execution.shares);
}
//! [execdetails]

//! [execdetailsend]
void IBAPISPI::execDetailsEnd(int reqId) {
	printf("ExecDetailsEnd. %d\n", reqId);
}
//! [execdetailsend]

//! [updatemktdepth]
void IBAPISPI::updateMktDepth(TickerId id, int position, int operation, int side,
	double price, int size) {
	printf("UpdateMarketDepth. %ld - Position: %d, Operation: %d, Side: %d, Price: %g, Size: %d\n", id, position, operation, side, price, size);
}
//! [updatemktdepth]

void IBAPISPI::updateMktDepthL2(TickerId id, int position, std::string marketMaker, int operation,
	int side, double price, int size) {
	printf("UpdateMarketDepthL2. %ld - Position: %d, Operation: %d, Side: %d, Price: %g, Size: %d\n", id, position, operation, side, price, size);
}

//! [updatenewsbulletin]
void IBAPISPI::updateNewsBulletin(int msgId, int msgType, const std::string& newsMessage, const std::string& originExch) {
	printf("News Bulletins. %d - Type: %d, Message: %s, Exchange of Origin: %s\n", msgId, msgType, newsMessage.c_str(), originExch.c_str());
}
//! [updatenewsbulletin]

//! [managedaccounts]
void IBAPISPI::managedAccounts(const std::string& accountsList) {
	printf("Account List: %s\n", accountsList.c_str());
}
//! [managedaccounts]

//! [receivefa]
void IBAPISPI::receiveFA(faDataType pFaDataType, const std::string& cxml) {
	std::cout << "Receiving FA: " << (int)pFaDataType << std::endl << cxml << std::endl;
}
//! [receivefa]

//! [historicaldata]
void IBAPISPI::historicalData(TickerId reqId, const std::string& date, double open, double high,
	double low, double close, int volume, int barCount, double WAP, int hasGaps) {
	std::shared_ptr<Event_Bar>e = std::make_shared<Event_Bar>();
	jsstructs::BarData bar;
	bar.symbol = m_tickerID_mapping_symbol[reqId];
	bar.open = open;
	bar.high = high;
	bar.low = low;
	bar.close = close;
	bar.volume = volume;
	if (date.find("  ") != std::string::npos)
	{
		std::vector<std::string>datetime = Utils::split(date, "  ");
		bar.date = datetime[0];
		bar.time = datetime[1];
	}
	e->bar = bar;
	m_ibgateway->onHistoricalData(e);
}
//! [historicaldata]

//! [scannerparameters]
void IBAPISPI::scannerParameters(const std::string& xml) {
	printf("ScannerParameters. %s\n", xml.c_str());
}
//! [scannerparameters]

//! [scannerdata]
void IBAPISPI::scannerData(int reqId, int rank, const ContractDetails& contractDetails,
	const std::string& distance, const std::string& benchmark, const std::string& projection,
	const std::string& legsStr) {
	printf("ScannerData. %d - Rank: %d, Symbol: %s, SecType: %s, Currency: %s, Distance: %s, Benchmark: %s, Projection: %s, Legs String: %s\n", reqId, rank, contractDetails.summary.symbol.c_str(), contractDetails.summary.secType.c_str(), contractDetails.summary.currency.c_str(), distance.c_str(), benchmark.c_str(), projection.c_str(), legsStr.c_str());
}
//! [scannerdata]

//! [scannerdataend]
void IBAPISPI::scannerDataEnd(int reqId) {
	printf("ScannerDataEnd. %d\n", reqId);
}
//! [scannerdataend]

//! [realtimebar]
void IBAPISPI::realtimeBar(TickerId reqId, long time, double open, double high, double low, double close,
	long volume, double wap, int count) {

	std::shared_ptr<Event_Bar>e = std::make_shared<Event_Bar>();
	jsstructs::BarData bar;
	bar.symbol = m_tickerID_mapping_symbol[reqId];
	bar.open = open;
	bar.high = high;
	bar.low = low;
	bar.close = close;
	bar.volume = volume;
	/*if (date.find("  ") != std::string::npos)
	{
		std::vector<std::string>datetime = Utils::split(date, "  ");
		bar.date = datetime[0];
		bar.time = datetime[1];
	}*/
	e->bar = bar;
	m_ibgateway->onHistoricalData(e);

}
//! [realtimebar]

//! [fundamentaldata]
void IBAPISPI::fundamentalData(TickerId reqId, const std::string& data) {
	printf("FundamentalData. ReqId: %ld, %s\n", reqId, data.c_str());
}
//! [fundamentaldata]

void IBAPISPI::deltaNeutralValidation(int reqId, const UnderComp& underComp) {
	printf("DeltaNeutralValidation. %d, ConId: %ld, Delta: %g, Price: %g\n", reqId, underComp.conId, underComp.delta, underComp.price);
}

//! [ticksnapshotend]
void IBAPISPI::tickSnapshotEnd(int reqId) {
	printf("TickSnapshotEnd: %d\n", reqId);
}
//! [ticksnapshotend]

//! [marketdatatype]
void IBAPISPI::marketDataType(TickerId reqId, int marketDataType) {
	printf("MarketDataType. ReqId: %ld, Type: %d\n", reqId, marketDataType);
}
//! [marketdatatype]

//! [commissionreport]
void IBAPISPI::commissionReport(const CommissionReport& commissionReport) {
	printf("CommissionReport. %s - %g %s RPNL %g\n", commissionReport.execId.c_str(), commissionReport.commission, commissionReport.currency.c_str(), commissionReport.realizedPNL);
}
//! [commissionreport]

//! [position]
void IBAPISPI::position(const std::string& account, const Contract& contract, double position, double avgCost) {
	printf("Position. %s - Symbol: %s, SecType: %s, Currency: %s, Position: %g, Avg Cost: %g\n", account.c_str(), contract.symbol.c_str(), contract.secType.c_str(), contract.currency.c_str(), position, avgCost);
}
//! [position]

//! [positionend]
void IBAPISPI::positionEnd() {
	printf("PositionEnd\n");
}
//! [positionend]

//! [accountsummary]
void IBAPISPI::accountSummary(int reqId, const std::string& account, const std::string& tag, const std::string& value, const std::string& currency) {
	printf("Acct Summary. ReqId: %d, Account: %s, Tag: %s, Value: %s, Currency: %s\n", reqId, account.c_str(), tag.c_str(), value.c_str(), currency.c_str());
}
//! [accountsummary]

//! [accountsummaryend]
void IBAPISPI::accountSummaryEnd(int reqId) {
	printf("AccountSummaryEnd. Req Id: %d\n", reqId);
}
//! [accountsummaryend]

void IBAPISPI::verifyMessageAPI(const std::string& apiData) {
	printf("verifyMessageAPI: %s\b", apiData.c_str());
}

void IBAPISPI::verifyCompleted(bool isSuccessful, const std::string& errorText) {
	printf("verifyCompleted. IsSuccessfule: %d - Error: %s\n", isSuccessful, errorText.c_str());
}

void IBAPISPI::verifyAndAuthMessageAPI(const std::string& apiDatai, const std::string& xyzChallenge) {
	printf("verifyAndAuthMessageAPI: %s %s\n", apiDatai.c_str(), xyzChallenge.c_str());
}

void IBAPISPI::verifyAndAuthCompleted(bool isSuccessful, const std::string& errorText) {
	printf("verifyAndAuthCompleted. IsSuccessful: %d - Error: %s\n", isSuccessful, errorText.c_str());
	if (isSuccessful)
		m_Client_API->startApi();
}

//! [displaygrouplist]
void IBAPISPI::displayGroupList(int reqId, const std::string& groups) {
	printf("Display Group List. ReqId: %d, Groups: %s\n", reqId, groups.c_str());
}
//! [displaygrouplist]

//! [displaygroupupdated]
void IBAPISPI::displayGroupUpdated(int reqId, const std::string& contractInfo) {
	std::cout << "Display Group Updated. ReqId: " << reqId << ", Contract Info: " << contractInfo << std::endl;
}
//! [displaygroupupdated]

//! [positionmulti]
void IBAPISPI::positionMulti(int reqId, const std::string& account, const std::string& modelCode, const Contract& contract, double pos, double avgCost) {
	printf("Position Multi. Request: %d, Account: %s, ModelCode: %s, Symbol: %s, SecType: %s, Currency: %s, Position: %g, Avg Cost: %g\n", reqId, account.c_str(), modelCode.c_str(), contract.symbol.c_str(), contract.secType.c_str(), contract.currency.c_str(), pos, avgCost);
}
//! [positionmulti]

//! [positionmultiend]
void IBAPISPI::positionMultiEnd(int reqId) {
	printf("Position Multi End. Request: %d\n", reqId);
}
//! [positionmultiend]

//! [accountupdatemulti]
void IBAPISPI::accountUpdateMulti(int reqId, const std::string& account, const std::string& modelCode, const std::string& key, const std::string& value, const std::string& currency) {
	printf("AccountUpdate Multi. Request: %d, Account: %s, ModelCode: %s, Key, %s, Value: %s, Currency: %s\n", reqId, account.c_str(), modelCode.c_str(), key.c_str(), value.c_str(), currency.c_str());
}
//! [accountupdatemulti]

//! [accountupdatemultiend]
void IBAPISPI::accountUpdateMultiEnd(int reqId) {
	printf("Account Update Multi End. Request: %d\n", reqId);
}
//! [accountupdatemultiend]

//! [securityDefinitionOptionParameter]
void IBAPISPI::securityDefinitionOptionalParameter(int reqId, const std::string& exchange, int underlyingConId, const std::string& tradingClass, const std::string& multiplier, std::set<std::string> expirations, std::set<double> strikes) {
	printf("Security Definition Optional Parameter. Request: %d, Trading Class: %s, Multiplier: %s\n", reqId, tradingClass.c_str(), multiplier.c_str());
}
//! [securityDefinitionOptionParameter]

//! [securityDefinitionOptionParameterEnd]
void IBAPISPI::securityDefinitionOptionalParameterEnd(int reqId) {
	printf("Security Definition Optional Parameter End. Request: %d\n", reqId);
}
//! [securityDefinitionOptionParameterEnd]

//! [softDollarTiers]
void IBAPISPI::softDollarTiers(int reqId, const std::vector<SoftDollarTier> &tiers) {
	printf("Soft dollar tiers (%d):", tiers.size());

	for (int i = 0; i < tiers.size(); i++) {
		printf("%s\n", tiers[0].displayName());
	}
}