#include "AppPrecomp.h"
#include "BrainTopPlayer.h"
#include "VisualProfile.h"
#include "physics/Contact.h"
#include "TextManager.h"
#include "MaterialManager.h"
#include "VisualProfileManager.h"

#define C_PLAYER_DESIRED_SPEED 3.3f
#define C_PLAYER_ACCEL_POWER 0.7f
#define C_PLAYER_GROUND_DAMPENING 0.4f //is removed from speed, not multiplied

BrainTopPlayer registryInstance(NULL); //self register ourselves in the brain registry

BrainTopPlayer::BrainTopPlayer(MovingEntity * pParent):Brain(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}

	ResetKeys();

	m_SlotKeyUp = CL_Keyboard::sig_key_up().connect( this, &BrainTopPlayer::OnKeyUp);
	m_SlotKeyDown = CL_Keyboard::sig_key_down().connect( this, &BrainTopPlayer::OnKeyDown);

	m_walkSound.Init(g_pSoundManager, m_pParent->GetData()->Get("walk_sound"));
	m_attackTimer.SetInterval(200);
}

BrainTopPlayer::~BrainTopPlayer()
{
	RemoveActivePlayerIfNeeded(m_pParent);
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


void BrainTopPlayer::CheckForMovement()
{
	bool bIdle = true;

	if (m_Keys & C_KEY_LEFT)
	{
		bIdle = false;
		m_pParent->SetFacing(VisualProfile::FACING_LEFT); 
	}

	if (m_Keys & C_KEY_RIGHT)
	{
		m_pParent->SetFacing(VisualProfile::FACING_RIGHT);
		bIdle = false;
	}

	if (m_Keys & C_KEY_UP)
	{
		m_pParent->SetFacing(VisualProfile::FACING_UP);
		bIdle = false;
	}

	if (m_Keys & C_KEY_DOWN)
	{
		m_pParent->SetFacing(VisualProfile::FACING_DOWN);
		bIdle = false;
	}

	if (m_Keys & C_KEY_DOWN && m_Keys & C_KEY_LEFT)
	{
		m_pParent->SetFacing(VisualProfile::FACING_DOWN_LEFT);
		bIdle = false;
	}

	if (m_Keys & C_KEY_UP && m_Keys & C_KEY_LEFT)
	{
		m_pParent->SetFacing(VisualProfile::FACING_UP_LEFT);
		bIdle = false;
	}

	if (m_Keys & C_KEY_DOWN && m_Keys & C_KEY_RIGHT)
	{
		m_pParent->SetFacing(VisualProfile::FACING_DOWN_RIGHT);
		bIdle = false;
	}

	if (m_Keys & C_KEY_UP && m_Keys & C_KEY_RIGHT)
	{
		m_pParent->SetFacing(VisualProfile::FACING_UP_RIGHT);
		bIdle = false;
	}

	if (!bIdle)
	{
		//we're moving or trying to move
		m_moveAngle = FacingToVector(m_pParent->GetFacing());
		m_pParent->SetVisualState(VisualProfile::VISUAL_STATE_WALK);

	}
}

void BrainTopPlayer::CalculateForce(CL_Vector2 &force, float step)
{
	float desiredSpeed = C_PLAYER_DESIRED_SPEED;
	float accelPower = C_PLAYER_ACCEL_POWER;

	if (m_moveAngle != CL_Vector2::ZERO)
	{
		force = CL_Vector2(m_moveAngle)*desiredSpeed;
		CL_Vector2 curForce = m_pParent->GetLinearVelocity()/step; //figure out what needs to change to get our desired total force
		force = force-curForce;

		Clamp(force.x, -accelPower, accelPower); //limit force to accel power
	}
	
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
	VisualProfile *pProfile = m_pParent->GetVisualProfile();

	m_moveAngle = CL_Vector2(0,0);

	CheckForMovement();
	
	//if not moving, force idle state
	if (m_moveAngle == CL_Vector2::ZERO)
	{
		m_pParent->SetVisualState(VisualProfile::VISUAL_STATE_IDLE);
	}
	
	CL_Vector2 force = CL_Vector2(0,0);

	CalculateForce(force, step);

	//LogMsg("Cur Linear: %.2f, %.2f  Impulse: %.2f, %.2f", GetBody()->GetLinVelocity().x, GetBody()->GetLinVelocity().y, force.x, force.y);

	m_pParent->AddForce(force);

	if (m_pParent->GetBody()->GetLinVelocity().Length() > 1)
	{
		m_walkSound.Play(true);
	} else
	{
		m_walkSound.Play(false);
	}

	CheckForWarp();
}

void BrainTopPlayer::Update(float step)
{
	AssignPlayerToCameraIfNeeded(m_pParent);

	if (!m_pParent->GetCollisionData())
	{
		LogMsg("Error, top player brain requires collision data to function.");
	} else
	{
		//Calculate the steering force and update the bot's velocity and position
		UpdateMovement(step);
		m_pParent->SetSpriteByVisualStateAndFacing();
	}
}

void BrainTopPlayer::PostUpdate(float step)
{
	m_pParent->GetBody()->GetAngVelocity() = 0;

	if (m_moveAngle == CL_Vector2::ZERO)
	{
		//slow down
		set_float_with_target(&m_pParent->GetBody()->GetLinVelocity().x, 0, C_PLAYER_GROUND_DAMPENING);
		set_float_with_target(&m_pParent->GetBody()->GetLinVelocity().y, 0, C_PLAYER_GROUND_DAMPENING);
	}

	if (m_Keys & C_KEY_SELECT)
	{
		m_Keys &= ~C_KEY_SELECT; //turn it off
//		OnAction();
	}
}