#include "AppPrecomp.h"
#include "StateTopIdle.h"
#include "MovingEntity.h"

StateTopIdle registryInstanceStateTopIdle(NULL); //self register ourselves i nthe brain registry

StateTopIdle::StateTopIdle(MovingEntity * pParent):State(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}

}

StateTopIdle::~StateTopIdle()
{
}

void StateTopIdle::OnAdd()
{
}

void StateTopIdle::OnRemove()
{
}

void StateTopIdle::Update(float step)
{

}

void StateTopIdle::PostUpdate(float step)
{

}