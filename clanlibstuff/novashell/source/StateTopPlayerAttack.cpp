#include "AppPrecomp.h"
#include "StateTopPlayerAttack.h"
#include "MovingEntity.h"

StateTopPlayerAttack registryInstanceStateTopPlayerAttack(NULL); //self register ourselves i nthe brain registry

StateTopPlayerAttack::StateTopPlayerAttack(MovingEntity * pParent):State(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}

}

StateTopPlayerAttack::~StateTopPlayerAttack()
{
}

void StateTopPlayerAttack::OnAdd()
{
	m_pParent->SetVisualState(VisualProfile::VISUAL_STATE_ATTACK1);
}

void StateTopPlayerAttack::OnRemove()
{
}

void StateTopPlayerAttack::Update(float step)
{
	if (AnimIsLooping())
	{
		//done playing anim
		m_pParent->GetBrainManager()->SetStateByName("TopPlayerIdle");
	}
}

void StateTopPlayerAttack::PostUpdate(float step)
{

}