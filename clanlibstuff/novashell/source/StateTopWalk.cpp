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
	m_bCallbackActive = m_pParent->GetScriptObject()->FunctionExists("OnWalkLoop");

}

void StateTopWalk::OnRemove()
{
}

void StateTopWalk::Update(float step)
{
	if (m_pParent->IsFacingTarget(1))
	{
		m_pParent->GetBrainManager()->GetBrainBase()->AddWeightedForce( m_pParent->GetVectorFacing() * m_pParent->GetDesiredSpeed() );
	}
}

void StateTopWalk::PostUpdate(float step)
{
	if (m_bCallbackActive && AnimIsLooping())
	{
		m_pParent->RunFunction("OnWalkLoop");
	}
}
