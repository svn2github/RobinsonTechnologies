#include "AppPrecomp.h"
#include "BrainForceVisual.h"
#include "MovingEntity.h"

BrainForceVisual registryInstanceBrainForceVisual(NULL); //self register ourselves i nthe brain registry

BrainForceVisual::BrainForceVisual(MovingEntity * pParent):Brain(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}

}


void BrainForceVisual::HandleMsg(const string &msg)
{
	vector<string> messages = CL_String::tokenize(msg, ";",true);

	for (unsigned int i=0; i < messages.size(); i++)
	{
		vector<string> words = CL_String::tokenize(messages[i], "=",true);

		if (words[0] == "force_anim")
		{
			m_forceAnim = words[1];
			m_pParent->SetAnimByName(m_forceAnim);

		} 
		else
		{
			LogMsg("Brain %s doesn't understand keyword '%s'", GetName(), words[0].c_str());
		}
	}

}


BrainForceVisual::~BrainForceVisual()
{
}

void BrainForceVisual::Update(float step)
{
		m_pParent->GetBrainManager()->SendToBrainBaseSimple(C_BRAIN_MESSAGE_DONT_SET_VISUAL,0,0);
	
}