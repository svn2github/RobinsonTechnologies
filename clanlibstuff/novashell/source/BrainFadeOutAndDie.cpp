#include "AppPrecomp.h"
#include "BrainFadeOutAndDie.h"
#include "MovingEntity.h"

BrainFadeOutAndDie registryInstance(NULL); //self register ourselves i nthe brain registry

BrainFadeOutAndDie::BrainFadeOutAndDie(MovingEntity * pParent):Brain(pParent)
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

BrainFadeOutAndDie::~BrainFadeOutAndDie()
{
}

void BrainFadeOutAndDie::Update(float step)
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
	
	m_pParent->GetSprite()->set_alpha( min (alpha, m_pParent->GetSprite()->get_alpha()));

	if (alpha <= 0)
	{
		m_pParent->SetDeleteFlag(true);
	}
}