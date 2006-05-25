#include "AppPrecomp.h"
#include "Message.h"
#include "GameLogic.h"

Message::Message()
{
	m_pEnt = NULL;
}

Message::~Message()
{
}

void Message::EntDeleted(int ID)
{
  m_pEnt = NULL;
}

void Message::Deliver()
{
	
	if (m_targetID == 0)
	{
		//special system message
		GetGameLogic->HandleMessageString(m_text);
		return;
	}
	if (!m_pEnt) 
	{
		//the ent must have been deleted
		return;
	}

	m_pEnt->HandleMessageString(m_text);
}

void Message::SetTarget(unsigned int entID)
{
	assert(!m_pEnt && "messages can't be reassigned in mid transit!");
	
	if (entID == 0)
	{
		//special kind of message that we delivery directly to the system
		m_targetID = 0;
	}  else
	{
		m_targetID = entID;
		m_pEnt = EntityMgr->GetEntityFromID(m_targetID);

		if (!m_pEnt) 
		{
			LogMsg("Unable to schedule message, entID %d doesn't exist", entID);
			assert(m_pEnt);
			return;
		}

		m_slots.connect(m_pEnt->sig_delete, this, &Message::EntDeleted);
	}
	
	
}
