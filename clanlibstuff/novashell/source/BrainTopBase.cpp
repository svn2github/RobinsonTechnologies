#include "AppPrecomp.h"
#include "BrainTopBase.h"
#include "MovingEntity.h"

BrainTopBase registryInstanceBrainTopBase(NULL); //self register ourselves i nthe brain registry

BrainTopBase::BrainTopBase(MovingEntity * pParent):Brain(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}

	SetSort(100); //always run last


}

BrainTopBase::~BrainTopBase()
{
}

void BrainTopBase::ResetForNextFrame()
{
	m_maxForce = 5;
	m_force = CL_Vector2::ZERO;
}

void BrainTopBase::AddWeightedForce(const CL_Vector2 & force)
{
	float magnitudeSoFar = m_force.length();
	float magnitudeRemaining = m_maxForce - magnitudeSoFar;

	if (magnitudeRemaining <= 0) return;

	float magnitudeToAdd = force.length();

	if (magnitudeToAdd < magnitudeRemaining)
	{
		m_force += force;
	} else
	{
		//only add part
		CL_Vector2 unitForce = force;
		unitForce.unitize();
		m_force += (unitForce * magnitudeRemaining);
	}

}

void BrainTopBase::OnAdd()
{
	m_pParent->GetBrainManager()->SetBrainBase(this);
	m_turnSpeed = 0.1f;
	ResetForNextFrame();
}

void BrainTopBase::Update(float step)
{
	CL_Vector2 curForce = m_pParent->GetLinearVelocity()/step; //figure out what needs to change to get our desired total force
	m_force = m_force-curForce;

#define C_TOP_ACCEL_POWER 0.17f
	Clamp(m_force.x, -C_TOP_ACCEL_POWER, C_TOP_ACCEL_POWER); //limit force to accel power
	Clamp(m_force.y, -C_TOP_ACCEL_POWER, C_TOP_ACCEL_POWER); //limit force to accel power

		m_pParent->RotateTowardsVectorDirection(m_pParent->GetVectorFacingTarget(), m_turnSpeed *step);
		m_pParent->AddForce(m_force);

	m_pParent->SetSpriteByVisualStateAndFacing();

	//completely stop rotation
	m_pParent->GetBody()->GetAngVelocity() = 0;

}

void BrainTopBase::PostUpdate(float step)
{
	ResetForNextFrame();
}