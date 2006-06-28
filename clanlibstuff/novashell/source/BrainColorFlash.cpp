#include "AppPrecomp.h"
#include "BrainColorFlash.h"
#include "MovingEntity.h"

BrainColorFlash registryInstance(NULL); //self register ourselves i nthe brain registry

BrainColorFlash::BrainColorFlash(MovingEntity * pParent):Brain(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}

	m_flashTimer.SetInterval(300);
	m_bOn = true;
	m_color = CL_Color(100,0,0);
	
}

BrainColorFlash::~BrainColorFlash()
{
}

void BrainColorFlash::Update(float step)
{
	

}