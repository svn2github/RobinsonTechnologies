#include "AppPrecomp.h"
#include "BrainShake.h"

BrainShake::BrainShake(MovingEntity * pParent):Brain(pParent)
{
	m_brainType = SHAKE;
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
	
	//if we moved it like this, it would have physics
	//m_pParent->AddForce(CL_Vector2( (frandom()-0.5)*0.1f, (frandom()-0.5)*0.1f));
}

void BrainShake::PostUpdate(float step)
{
	
}
