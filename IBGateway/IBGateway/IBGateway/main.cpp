#include"StdAfx.h"
#include"IBGateway.h"
#include"ContractSamples.h"
#include <fstream>
#include"vld.h"
std::fstream f;
std::mutex mtx;
void onTick(std::shared_ptr<Event>e)
{
	std::unique_lock<std::mutex>lck(mtx);
	 
}

int main()
{
	EventEngine eventengine;
	eventengine.regEvent(EVENT_TICK, std::bind(&onTick, std::placeholders::_1));
	eventengine.startEngine();
	//´´½¨gateway
	IBGateway ibgateway(&eventengine);
	ibgateway.connect();
	system("pause");



	system("pause");
	ibgateway.close();
	f.close();
	return 0;
}