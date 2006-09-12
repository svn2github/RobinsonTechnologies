#include "AppPrecomp.h"
#include "StateTopPlayerIdle.h"
#include "MovingEntity.h"

StateTopPlayerIdle registryInstanceTopPlayerIdle(NULL); //self register ourselves i nthe brain registry

StateTopPlayerIdle::StateTopPlayerIdle(MovingEntity * pParent):State(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}
}

StateTopPlayerIdle::~StateTopPlayerIdle()
{
}

void StateTopPlayerIdle::OnAdd()
{
	m_pParent->SetVisualState(VisualProfile::VISUAL_STATE_IDLE);
}

void StateTopPlayerIdle::OnRemove()
{
}

void StateTopPlayerIdle::Update(float step)
{
}

void StateTopPlayerIdle::PostUpdate(float step)
{

}
