#include "AppPrecomp.h"
#include "BrainTopBase.h"
#include "MovingEntity.h"

BrainTopBase registryInstanceBrainTopBase(NULL); //self register ourselves in the brain registry

BrainTopBase::BrainTopBase(MovingEntity * pParent):Brain(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}

	SetSort(100); //always run last

	m_controlFilter = CL_Vector2(1,1); //full control
}

BrainTopBase::~BrainTopBase()
{
}

void BrainTopBase::ResetForNextFrame()
{
	m_maxForce = m_pParent->GetMaxWalkSpeed();
	m_force = CL_Vector2::ZERO;
	m_DontMessWithVisual = false;
}

int BrainTopBase::HandleSimpleMessage(int message, int user1, int user2)
{
	if (message == C_BRAIN_MESSAGE_DONT_SET_VISUAL)
	{
		m_DontMessWithVisual = true;
	}

	return 0;
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
	ResetForNextFrame();
}

void BrainTopBase::Update(float step)
{
	CL_Vector2 curForce = m_pParent->GetLinearVelocity(); //figure out what needs to change to get our desired total force
	m_force = m_force-curForce;

	
#define C_TOP_ACCEL_POWER 0.37f
	Clamp(m_force.x, -C_TOP_ACCEL_POWER, C_TOP_ACCEL_POWER); //limit force to accel power
	Clamp(m_force.y, -C_TOP_ACCEL_POWER, C_TOP_ACCEL_POWER); //limit force to accel power

	
	m_force.x *= m_controlFilter.x;
	m_force.y *= m_controlFilter.y;
	
	m_pParent->RotateTowardsVectorDirection(m_pParent->GetVectorFacingTarget(), m_pParent->GetTurnSpeed() *step);
	
	m_pParent->AddForce(m_force);

	if (!m_DontMessWithVisual)
	{
		m_pParent->SetSpriteByVisualStateAndFacing();		
	}

	//completely stop rotation
	if (!m_pParent->GetEnableRotation())
	m_pParent->GetBody()->GetAngVelocity() = 0;

}


void BrainTopBase::HandleMsg(const string &msg)
{
	vector<string> messages = CL_String::tokenize(msg, ";",true);

	for (unsigned int i=0; i < messages.size(); i++)
	{
		vector<string> words = CL_String::tokenize(messages[i], "=",true);

		if (words[0] == "control_filterx")
		{
			m_controlFilter.x = CL_String::to_float(words[1]);
		} 
		else if (words[0] == "control_filtery")
		{
			m_controlFilter.y = CL_String::to_float(words[1]);
		} 
				else
				{
					LogMsg("Brain %s doesn't understand keyword '%s'", GetName(), words[0].c_str());
				}
	}

}


void BrainTopBase::PostUpdate(float step)
{
	ResetForNextFrame();
}

/*
Object: StandardBase
A brain for use with the <BrainManager> that performs many generic functions such as movement and path-finding.

About:

This brain installs itself as a "base brain".  An <Entity> can only have one base brain active.

Parameters:

control_filterx - a number from 0 to 1 showing how much influence it should have for movement on this axis.  Default is 1, full influence.
control_filtery - a number from 0 to 1 showing how much influence it should have for movement on this axis.  Default is 1, full influence.

Usage:
(code)
this:GetBrainManager():Add("StandardBase", "");
(end)

