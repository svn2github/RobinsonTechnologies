#include "AppPrecomp.h"
#include "BrainTopPlayer.h"
#include "VisualProfile.h"
#include "physics/Contact.h"
#include "TextManager.h"
#include "MaterialManager.h"
#include "VisualProfileManager.h"

#define C_PLAYER_DESIRED_SPEED 3.3f
#define C_PLAYER_ACCEL_POWER 0.3f
#define C_PLAYER_GROUND_DAMPENING 0.4f //is removed from speed, not multiplied

BrainTopPlayer registryInstanceBrainTopPlayer(NULL); //self register ourselves in the brain registry

BrainTopPlayer::BrainTopPlayer(MovingEntity * pParent):Brain(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}

	ResetKeys();

	SetSort(100); //always run last
	m_SlotKeyUp = CL_Keyboard::sig_key_up().connect( this, &BrainTopPlayer::OnKeyUp);
	m_SlotKeyDown = CL_Keyboard::sig_key_down().connect( this, &BrainTopPlayer::OnKeyDown);

	m_walkSound.Init(g_pSoundManager, m_pParent->GetData()->Get("walk_sound"));
	m_attackTimer.SetInterval(200);
	GetGameLogic->SetGameMode(GameLogic::C_GAME_MODE_TOP_VIEW);
}

BrainTopPlayer::~BrainTopPlayer()
{
	RemoveActivePlayerIfNeeded(m_pParent);
}

void BrainTopPlayer::OnAdd()
{
	m_pParent->GetBrainManager()->SetBrainBase(this);
	ResetForNextFrame();
	m_pParent->GetBrainManager()->SetStateByName("TopPlayerIdle");
}

void BrainTopPlayer::ResetKeys()
{
	m_Keys = 0; 
}

void BrainTopPlayer::OnKeyDown(const CL_InputEvent &key)
{

	switch(key.id)
	{
	case CL_KEY_CONTROL:

		m_Keys |= C_KEY_ATTACK;
		break;

	case CL_KEY_LEFT:
		m_Keys |= C_KEY_LEFT;
		break;

	case CL_KEY_RIGHT:
		m_Keys |= C_KEY_RIGHT;
		break;

	case CL_KEY_SPACE:
		m_Keys |= C_KEY_SELECT;
		break;

	case CL_KEY_UP:
		m_Keys |= C_KEY_UP;
		break;

	case CL_KEY_DOWN:
		m_Keys |= C_KEY_DOWN;
		break;
	}

}

void BrainTopPlayer::OnKeyUp(const CL_InputEvent &key)
{
	switch(key.id)
	{

	case CL_KEY_CONTROL:
		m_Keys &= ~C_KEY_ATTACK;
		break;

	case CL_KEY_LEFT:
		m_Keys &= ~C_KEY_LEFT;
		break;

	case CL_KEY_RIGHT:
		m_Keys &= ~C_KEY_RIGHT;
		break;

	case CL_KEY_SPACE:
		m_Keys &= ~C_KEY_SELECT;
		break;

	case CL_KEY_UP:
		m_Keys &= ~C_KEY_UP;
		break;

	case CL_KEY_DOWN:
		m_Keys &= ~C_KEY_DOWN;
		break;
	}
}

void BrainTopPlayer::CheckForAttack()
{
	if (m_Keys & C_KEY_ATTACK)
	{
		if (m_attackTimer.IntervalReached())
		{
			try {luabind::call_function<bool>(m_pParent->GetScriptObject()->GetState(), 
				"OnAttack", m_pParent);
			} LUABIND_ENT_BRAIN_CATCH("Error while calling OnAttack with playerbrain");
			m_Keys &= ~C_KEY_ATTACK; //don't let them hold down the key
		}
	}
}


void BrainTopPlayer::CalculateForce(float step)
{
	float accelPower = C_PLAYER_ACCEL_POWER;

	CL_Vector2 curForce = m_pParent->GetLinearVelocity()/step; //figure out what needs to change to get our desired total force
	m_force = m_force-curForce;

	Clamp(m_force.x, -accelPower, accelPower); //limit force to accel power
	Clamp(m_force.y, -accelPower, accelPower); //limit force to accel power
}

void BrainTopPlayer::CheckForWarp()
{
	
	Zone *pZone = m_pParent->GetNearbyZoneByCollisionRectAndType(CMaterial::C_MATERIAL_TYPE_WARP);
	
		if (pZone)
		{
			//don't let them keep warping by holding down the key
			//Let's warp dude!
			MovingEntity *pEnt = (MovingEntity*) EntityMgr->GetEntityFromID(pZone->m_entityID);
			if (pEnt)
			{	
				try {luabind::call_function<bool>(pEnt->GetScriptObject()->GetState(), 
					"OnWarp", m_pParent);
				} LUABIND_ENT_BRAIN_CATCH("Error while calling OnWarp(Entity)");
			} else
			{
				g_textManager.Add("Warp Script missing OnWarp function.", m_pParent);
			}

		} 
}

void BrainTopPlayer::UpdateMovement(float step)
{
	//VisualProfile *pProfile = m_pParent->GetVisualProfile();
	
	if (m_pParent->GetBrainManager()->InState("TopPlayerIdle"))
	{
		int visualFacing;

		if (ConvertKeysToDirection(m_pParent->GetBrainManager()->GetBrainBase()->GetKeys(), visualFacing))
		{
			//they aren't idle anymore!
			m_pParent->GetBrainManager()->SetStateByName("TopPlayerWalk");
		}
	}
	
	CalculateForce(step);

	//LogMsg("Cur Linear: %.2f, %.2f  Impulse: %.2f, %.2f", GetBody()->GetLinVelocity().x, GetBody()->GetLinVelocity().y, force.x, force.y);

	m_pParent->AddForce(m_force);

	if (m_pParent->GetBody()->GetLinVelocity().Length() > 1)
	{
		m_walkSound.Play(true);
	} else
	{
		m_walkSound.Play(false);
	}

	CheckForWarp();
	CheckForAttack();

	m_pParent->SetSpriteByVisualStateAndFacing();

}

void BrainTopPlayer::ResetForNextFrame()
{
	m_maxForce = 4;
	m_force = CL_Vector2::ZERO;
}

void BrainTopPlayer::AddWeightedForce(const CL_Vector2 & force)
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

void BrainTopPlayer::Update(float step)
{
	AssignPlayerToCameraIfNeeded(m_pParent);

	if (!m_pParent->GetCollisionData())
	{
		LogMsg("Error, top player brain requires collision data to function.");
	} else
	{
		
		m_pParent->RotateTowardsVectorDirection(m_pParent->GetVectorFacingTarget(), 0.24f *step);
		
		UpdateMovement(step);
		m_pParent->SetSpriteByVisualStateAndFacing();
	}
}

void BrainTopPlayer::PostUpdate(float step)
{
	m_pParent->GetBody()->GetAngVelocity() = 0;

	if (m_Keys & C_KEY_SELECT)
	{
		m_Keys &= ~C_KEY_SELECT; //turn it off
	}

	ResetForNextFrame();
}