#include "AppPrecomp.h"
#include "StateTopAttack.h"
#include "MovingEntity.h"

StateTopAttack registryInstanceStateTopAttack(NULL); //self register ourselves i nthe brain registry

StateTopAttack::StateTopAttack(MovingEntity * pParent):State(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}

}

StateTopAttack::~StateTopAttack()
{
}

void StateTopAttack::OnAdd()
{
		m_pParent->SetVisualState(VisualProfile::VISUAL_STATE_ATTACK1);
}

void StateTopAttack::OnRemove()
{
}

void StateTopAttack::Update(float step)
{
	
}

void StateTopAttack::PostUpdate(float step)
{
	if (AnimIsLooping())
	{
		m_pParent->RunFunction("OnAttackLoop");
	}
}