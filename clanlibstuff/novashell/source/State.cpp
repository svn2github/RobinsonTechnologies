#include "AppPrecomp.h"
#include "State.h"
#include "BrainManager.h"


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
