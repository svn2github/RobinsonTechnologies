#include "AppPrecomp.h"
#include "Main.h"
#include "MessageManager.h"


MessageManager::MessageManager()
{
}

MessageManager::~MessageManager()
{
}

void MessageManager::Schedule(unsigned int deliveryMS, unsigned int targetID, const char * pMsg)
{
	//LogMsg("scheduling message to %u", targetID);

	Message m;
	m.m_deliveryTime = GetApp()->GetGameTick()+deliveryMS;
	m.m_text = pMsg;
	
	m_messageList.push_back(m);
	m_messageList.rbegin()->SetTarget(targetID);

}

void MessageManager::Update()
{
	message_list::iterator itor = m_messageList.begin();

	unsigned int timeNow = GetApp()->GetGameTick();
	while (itor != m_messageList.end())
	{
		if ( itor->m_deliveryTime < timeNow)
		{
			
			itor->Deliver();
			//delete it
			itor = m_messageList.erase(itor);
			continue;
		}

		itor++;
	}

}

