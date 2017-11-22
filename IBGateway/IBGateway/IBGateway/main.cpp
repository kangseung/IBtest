#include"StdAfx.h"
#include"IBGateway.h"
#include"ContractSamples.h"
#include"vld.h"

void onHistoryData(std::shared_ptr<Event>e)
{
	std::shared_ptr<Event_Bar> eBar = std::static_pointer_cast<Event_Bar>(e);




}

int main()
{
	EventEngine eventengine;
	eventengine.regEvent(EVENT_BAR, std::bind(&onHistoryData,std::placeholders::_1));
	eventengine.startEngine();
	//´´½¨gateway
	IBGateway ibgateway(&eventengine);

	ibgateway.connect();



	system("pause");

	ibgateway.reqHistoryData(ContractSamples::EuropeanStock(), "30 D", "1 min");

	system("pause");
	ibgateway.close();
	return 0;
}