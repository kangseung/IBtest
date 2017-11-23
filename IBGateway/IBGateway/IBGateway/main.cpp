#include"StdAfx.h"
#include"IBGateway.h"
#include"ContractSamples.h"
#include <fstream>
#include"vld.h"
std::fstream f;
std::mutex mtx;
void onHistoryData(std::shared_ptr<Event>e)
{
	std::unique_lock<std::mutex>lck(mtx);
	std::shared_ptr<Event_Bar> eBar = std::static_pointer_cast<Event_Bar>(e);
	f << eBar->bar.symbol << "," << eBar->bar.date << "," << eBar->bar.time << "," << std::to_string(eBar->bar.open) << "," << std::to_string(eBar->bar.high) << "," << std::to_string(eBar->bar.low) << "," << std::to_string(eBar->bar.close) << "," << std::to_string(eBar->bar.volume) << "\n";
	std::cout << "symbol:" << eBar->bar.symbol << "date:" << eBar->bar.date << "time:" << eBar->bar.time << std::endl;
}

int main()
{
	f.open("./gcdata.csv", std::ios::app);
	f << "合约代码" << "," << "日期" << "," << "时间" << "," << "开" << "," << "高" << "," << "低" << "," << "收" << "," << "量\n";

	EventEngine eventengine;
	eventengine.regEvent(EVENT_BAR, std::bind(&onHistoryData,std::placeholders::_1));
	eventengine.startEngine();
	//创建gateway
	IBGateway ibgateway(&eventengine);

	ibgateway.connect();



	system("pause");

	ibgateway.realTimeBars(ContractSamples::EurGbpFx().symbol, ContractSamples::EurGbpFx(), 5);

	system("pause");
	ibgateway.close();
	f.close();
	return 0;
}