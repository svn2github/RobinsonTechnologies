#include "AppPrecomp.h"
#include "StateDie.h"
#include "MovingEntity.h"

StateDie registryInstance(NULL); //self register ourselves i nthe brain registry

StateDie::StateDie(MovingEntity * pParent):State(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}

}

StateDie::~StateDie()
{
}

void StateDie::OnAdd()
{
	m_pParent->SetVisualState(VisualProfile::VISUAL_STATE_DIE);
	
}

void StateDie::OnRemove()
{
}

void StateDie::Update(float step)
{

}

void StateDie::PostUpdate(float step)
{

}