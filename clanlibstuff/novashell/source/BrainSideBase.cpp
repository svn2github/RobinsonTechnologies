#include "AppPrecomp.h"
#include "BrainSideBase.h"
#include "MovingEntity.h"
#include "VisualProfileManager.h"

BrainSideBase registryInstance(NULL); //self register ourselves in the brain registry

BrainSideBase::BrainSideBase(MovingEntity * pParent):Brain(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}

	SetSort(100); //always run last
}

BrainSideBase::~BrainSideBase()
{
}

void BrainSideBase::ResetForNextFrame()
{

	m_maxForce = 4;
	m_force = CL_Vector2::ZERO;

}

void BrainSideBase::AddWeightedForce(const CL_Vector2 & force)
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

void BrainSideBase::OnAdd()
{
	m_facing = VisualProfile::FACING_LEFT;
	ResetForNextFrame();

	m_pParent->GetBrainManager()->SetStateByName("SideSimpleWalk");
}


void BrainSideBase::Update(float step)
{
	CL_Vector2 pos = m_pParent->GetPos();
	CL_Vector2 curForce = m_pParent->GetLinearVelocity()/step; //figure out what needs to change to get our desired total force
	m_force = m_force-curForce;
	#define C_SIDE_ACCEL_POWER 0.17f
	Clamp(m_force.x, -C_SIDE_ACCEL_POWER, C_SIDE_ACCEL_POWER); //limit force to accel power
	
	if (m_pParent->IsOnGround())
	{
		m_pParent->AddForce(m_force);
	} else
	{
		//LogMsg("in air!");

	}

	VisualProfile *pProfile = m_pParent->GetVisualProfile();

	if (m_pParent->GetLinearVelocity().length() < 0.3f)
	{
		m_state = VisualProfile::VISUAL_STATE_IDLE;
	} else
	{
		m_state = VisualProfile::VISUAL_STATE_RUN;
	}

	if (pProfile) m_pParent->SetSpriteData(pProfile->GetSprite(m_state,m_facing));

}

void BrainSideBase::PostUpdate(float step)
{
	ResetForNextFrame();
}
