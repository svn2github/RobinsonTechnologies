#include "AppPrecomp.h"
#include "BrainShake.h"
#include "MovingEntity.h"

BrainShake registryInstance(NULL); //self register ourselves in the brain registry

BrainShake::BrainShake(MovingEntity * pParent):Brain(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}

	m_lastDisplacement = CL_Vector2(0,0);
}

BrainShake::~BrainShake()
{
}

void BrainShake::Update(float step)
{
	CL_Vector2 pos = m_pParent->GetPos();

	pos -= m_lastDisplacement; //undo the last displacement
	m_lastDisplacement = CL_Vector2(random_range(-10,11),random_range(-10,11));
	pos += m_lastDisplacement;

	//move it somewhere crazy
	m_pParent->SetPos(pos);
}

void BrainShake::PostUpdate(float step)
{
}
