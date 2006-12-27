#include "AppPrecomp.h"
#include "BrainFadeIn.h"
#include "MovingEntity.h"

BrainFadeIn registryInstanceBrainFadeIn(NULL); //self register ourselves i nthe brain registry

BrainFadeIn::BrainFadeIn(MovingEntity * pParent):Brain(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}

	m_fadeOutTimeMS = 1000;
	m_timeCreated = GetApp()->GetGameTick();

}

BrainFadeIn::~BrainFadeIn()
{
}

void BrainFadeIn::HandleMsg(const string &msg)
{
	vector<string> messages = CL_String::tokenize(msg, ";",true);

	for (unsigned int i=0; i < messages.size(); i++)
	{
		vector<string> words = CL_String::tokenize(messages[i], "=",true);

		if (words[0] == "fade_speed_ms")
		{
			m_fadeOutTimeMS = CL_String::to_int(words[1]);
		} 
		else
		{
			LogMsg("Brain %s doesn't understand keyword %s", GetName(), words[0].c_str());
		}
	}



}


void BrainFadeIn::Update(float step)
{

	//set alpha for fading
	int timeLeft = (m_timeCreated+ m_fadeOutTimeMS) - GetApp()->GetGameTick();

	float alpha;

	if (timeLeft < m_fadeOutTimeMS)
	{
		alpha = ( float(timeLeft) / float(m_fadeOutTimeMS));
	} else
	{
		alpha = 1;
	}

	m_pParent->AddColorModAlpha( (alpha)*-255);

	if (alpha <= 0)
	{
#ifdef _DEBUG
		LogMsg("Removing brain %s!!", GetName());
#endif
		this->SetDeleteFlag(true);
	}

}