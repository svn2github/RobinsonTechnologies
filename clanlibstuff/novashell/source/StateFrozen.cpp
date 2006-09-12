#include "AppPrecomp.h"
#include "StateFrozen.h"
#include "MovingEntity.h"

StateFrozen registryInstanceFrozen(NULL); //self register ourselves in the brain registry

StateFrozen::StateFrozen(MovingEntity * pParent):State(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}

}

StateFrozen::~StateFrozen()
{
}

void StateFrozen::OnAdd()
{
}

void StateFrozen::OnRemove()
{
}

void StateFrozen::Update(float step)
{

}

void StateFrozen::PostUpdate(float step)
{
	if (AnimIsLooping())
	{
		//this resets the animation, just play forever
	}
}
