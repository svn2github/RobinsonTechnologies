#include "AppPrecomp.h"
#include "StatePain.h"
#include "MovingEntity.h"

StatePain registryInstanceStatePain(NULL); //self register ourselves i nthe brain registry

StatePain::StatePain(MovingEntity * pParent):State(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}

}

StatePain::~StatePain()
{
}

void StatePain::OnAdd()
{
	m_pParent->SetVisualState(VisualProfile::VISUAL_STATE_PAIN);
	
}

void StatePain::OnRemove()
{
}

void StatePain::Update(float step)
{

}

void StatePain::PostUpdate(float step)
{

}