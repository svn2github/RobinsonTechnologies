#include "AppPrecomp.h"
#include "StateSideSimpleIdle.h"

StateSideSimpleIdle registryInstance(NULL); //self register ourselves in the brain registry

StateSideSimpleIdle::StateSideSimpleIdle(MovingEntity * pParent):State(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}
	
}
StateSideSimpleIdle::~StateSideSimpleIdle()
{
}

void StateSideSimpleIdle::OnAdd()
{
	m_thinkTimer.SetInterval(random_range(2000,4000));
	m_pParent->SetVisualState(VisualProfile::VISUAL_STATE_IDLE);
}

void StateSideSimpleIdle::PostUpdate(float step)
{

	if (AnimIsLooping())
	{
		if (m_thinkTimer.IntervalReached())
		{
			//go back to walking around
			m_pParent->GetBrainManager()->SetStateByName("SideSimpleWalk");
		}
	}
}