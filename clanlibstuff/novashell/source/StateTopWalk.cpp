#include "AppPrecomp.h"
#include "StateTopWalk.h"
#include "MovingEntity.h"

StateTopWalk registryInstanceStateTopWalk(NULL); //self register ourselves i nthe brain registry

StateTopWalk::StateTopWalk(MovingEntity * pParent):State(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}

}

StateTopWalk::~StateTopWalk()
{
}

void StateTopWalk::OnAdd()
{
	if (m_pParent->GetVisualProfile()->IsActive(VisualProfile::WALK_LEFT))
	{
		m_pParent->SetVisualState(VisualProfile::VISUAL_STATE_WALK);
	} else
	{
		m_pParent->SetVisualState(VisualProfile::VISUAL_STATE_RUN);
	}

}

void StateTopWalk::OnRemove()
{
}

void StateTopWalk::Update(float step)
{
	float weight = 2.2f;
	m_pParent->GetBrainManager()->GetBrainBase()->AddWeightedForce( m_pParent->GetVectorFacing() * weight );

}

void StateTopWalk::PostUpdate(float step)
{
	if (AnimIsLooping())
	{
		m_pParent->RunFunction("OnWalkLoop");
	}
}