#include "AppPrecomp.h"
#include "State.h"

State::State(MovingEntity * pParent): m_pParent(pParent)
{
	
}

State::~State()
{
}

void State::RegisterClass()
{
	assert(!m_pParent);

	//no parent class given, assume we want to register this instance in the
	//State registry
	StateRegistry::GetInstance()->Add(this);
}

bool State::AnimIsLooping()
{
	if (m_pParent->GetSprite()->is_looping())
	{
		return true;
	}

	if (m_pParent->GetSprite()->is_finished())
	{
		//m_pParent->GetSprite()->restart();
		return true;
	}

	return false;
}
