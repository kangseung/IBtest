#include"IBGateway.h"
IBGateway::IBGateway()
{
	m_ibapispi = new IBAPISPI(this);
}

IBGateway::~IBGateway()
{
	if (m_ibapispi != nullptr)
	{
		delete m_ibapispi;
	}
}

