#include "AppPrecomp.h"
#include "StateTopPlayerWalk.h"
#include "MovingEntity.h"
#include "BrainPlayer.h"

StateTopPlayerWalk registryInstanceStateTopPlayerWalk(NULL); //self register ourselves i nthe brain registry

StateTopPlayerWalk::StateTopPlayerWalk(MovingEntity * pParent):State(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}
}

StateTopPlayerWalk::~StateTopPlayerWalk()
{
}

void StateTopPlayerWalk::OnAdd()
{
	m_pParent->SetVisualState(VisualProfile::VISUAL_STATE_WALK);
}

void StateTopPlayerWalk::OnRemove()
{
}

void StateTopPlayerWalk::Update(float step)
{
	float weight = 2.4f;
	int visualFacing;

	if (ConvertKeysToDirection(m_pParent->GetBrainManager()->GetBrainBase()->GetKeys(), visualFacing))
	{
		//they changed direction
		m_pParent->SetFacing(visualFacing);
	}

	m_pParent->GetBrainManager()->GetBrainBase()->AddWeightedForce(FacingToVector(m_pParent->GetFacing()) * weight);
}

void StateTopPlayerWalk::PostUpdate(float step)
{
	if (m_pParent->GetBrainManager()->GetBrainBase()->GetKeys() == 0)
	{
		//not pressing a direction anymore
		m_pParent->GetBrainManager()->SetStateByName("TopPlayerIdle");
	}
}