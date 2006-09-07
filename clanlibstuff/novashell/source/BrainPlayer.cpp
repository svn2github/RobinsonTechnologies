#include "AppPrecomp.h"
#include "BrainPlayer.h"
#include "VisualProfile.h"
#include "physics/Contact.h"
#include "TextManager.h"
#include "MaterialManager.h"
#include "VisualProfileManager.h"

#ifndef WIN32
//windows already has this in the precompiled header for speed, I couldn't get that to work on mac..
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#endif

#define C_PLAYER_DESIRED_SPEED 3.3f
#define C_PLAYER_ACCEL_POWER 0.7f

//how many frames to apply the jump force
#define C_PLAYER_JUMP_SECOND_BURST_MS 0
#define C_PLAYER_JUMP_AGAIN_LENGTH_MS 250
//how much force to apply each frame
#define C_PLAYER_JUMP_INITIAL_FORCE 1
#define C_PLAYER_JUMP_SECOND_FORCE 1
#define C_PLAYER_JUMP_DRAG_DOWN 0.32f
#define C_PLAYER_JUMP_FORCE_WEAKEN_MULT 0.8f;
#define C_PLAYER_AIR_DAMPENING 0.4f
#define C_PLAYER_GROUND_DAMPENING 0.4f //is removed from speed, not multiplied

#define C_LADDER_SENSITIVITY 10 //how many pixels to look up and down to verify a ladder really is starting or
								//ending


BrainPlayer registryInstancePlayer(NULL); //self register ourselves in the brain registry


BrainPlayer::BrainPlayer(MovingEntity * pParent):Brain(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}

	ResetKeys();
	SetSort(100); //always run last
	m_SlotKeyUp = CL_Keyboard::sig_key_up().connect( this, &BrainPlayer::OnKeyUp);
	m_SlotKeyDown = CL_Keyboard::sig_key_down().connect( this, &BrainPlayer::OnKeyDown);
	m_jumpTimer= 0;
	m_bFrozen = false;
	m_jumpBurstTimer = 0;
	m_bRequestJump = false;
	//LogMsg("Making player %x (parent %x)", this, m_pParent);

	m_walkSound.Init(g_pSoundManager, m_pParent->GetData()->Get("walk_sound"));
	m_climbSound.Init(g_pSoundManager, m_pParent->GetData()->Get("climb_sound"));
	m_jumpSound = m_pParent->GetData()->Get("jump_sound");
	
	m_attackTimer.SetInterval(200);
}

void BrainPlayer::OnAdd()
{
	m_pParent->GetBrainManager()->SetBrainBase(this);
}

void AssignPlayerToCameraIfNeeded(MovingEntity *pEnt)
{
	//there can only be one OFFICIAL player, but I like letting the editor
	//cut and paste the editor and making dupes so I need to pay special care
	//like this
	if (!GetGameLogic->GetEditorActive() && !GetGameLogic->GetMyPlayer())  
	{
		GetGameLogic->SetMyPlayer(pEnt);
		SetCameraToTrackPlayer();
	}
}

void RemoveActivePlayerIfNeeded(BaseGameEntity *pEnt)
{
	if (GetGameLogic && GetGameLogic->GetMyPlayer() == pEnt)
	{
		GetGameLogic->SetMyPlayer(NULL);
		GetCamera->SetEntTracking(0);
	}
}

BrainPlayer::~BrainPlayer()
{
 	RemoveActivePlayerIfNeeded(m_pParent);
}

void BrainPlayer::ResetKeys()
{
	m_Keys = 0; 
}

void BrainPlayer::OnKeyDown(const CL_InputEvent &key)
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
		//if (! (m_Keys & C_KEY_UP) ) //make them let go of the key before jumping again
		{
			if (!m_pParent->GetOnLadder())
			m_bRequestJump = true;	
		}
		m_Keys |= C_KEY_UP;
		break;

	case CL_KEY_DOWN:
		m_Keys |= C_KEY_DOWN;
		break;
	}

}

void BrainPlayer::OnKeyUp(const CL_InputEvent &key)
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

void BrainPlayer::AttemptToJump()
{
	if (m_pParent->IsOnGround() && m_jumpTimer < GetApp()->GetGameTick())
	{
		//looks ok to jump now
		m_jumpTimer = GetApp()->GetGameTick()+C_PLAYER_JUMP_AGAIN_LENGTH_MS;
		m_jumpBurstTimer = GetApp()->GetGameTick()+C_PLAYER_JUMP_SECOND_BURST_MS;
		m_bAppliedSecondJumpForce = false;
		m_curJumpForce = C_PLAYER_JUMP_INITIAL_FORCE * m_pParent->GetScale().y;
		m_pParent->SetIsOnGround(false); //don't wait for the ground timer to run out
		g_pSoundManager->PlayMixed(m_jumpSound.c_str());
		m_pParent->GetBody()->GetLinVelocity().y = 0;
	} 
}

void BrainPlayer::CheckForDoor()
{
	if ( !m_pParent->GetOnLadder() && m_Keys & C_KEY_DOWN && m_pParent->IsOnGround())
	{
		CL_Vector2 vPos = m_pParent->GetPos(); 
		vPos.y += m_pParent->GetCollisionData()->GetLineList()->begin()->GetRect().bottom; //work from the position of our feet, instead of middle

		Zone *pZone = m_pParent->GetNearbyZoneByPointAndType(vPos, CMaterial::C_MATERIAL_TYPE_WARP);

		if (pZone)
		{
			//don't let them keep warping by holding down the key

			m_Keys &= ~C_KEY_DOWN;
			
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
}

void BrainPlayer::CheckForLadder()
{
	VisualProfile *pProfile = m_pParent->GetVisualProfile();

	CL_Vector2 vPos = m_pParent->GetPos(); 
	vPos.y += m_pParent->GetCollisionData()->GetLineList()->begin()->GetRect().bottom; //work from the position of our feet, instead of middle
	
	if (m_pParent->GetOnLadder())
	{
		//should we stop being in ladder mode?
		Zone *pZone = m_pParent->GetNearbyZoneByPointAndType(vPos, CMaterial::C_MATERIAL_TYPE_VERTICAL_LADDER);

		if (!pZone || 
			(
			(m_Keys & C_KEY_LEFT || m_Keys & C_KEY_RIGHT)
			&& (! (m_Keys & C_KEY_UP)) && (! (m_Keys & C_KEY_DOWN))
			) 
			)
		{
			//disengage from ladder
			m_pParent->SetOnLadder(false);
			m_pParent->SetAnimPause(false);

		} else
		{
			//still on the ladder

			if (m_Keys & C_KEY_DOWN || m_Keys & C_KEY_UP)
			{
				//um, maybe we should be using forces, not directly moving it..
				CL_Vector2 pos = m_pParent->GetPos();
				float centerOfLadderX = pZone->m_vPos.x + pZone->m_boundingRect.left + (pZone->m_boundingRect.get_width()/2);
				//center them a bit
				set_float_with_target(&pos.x, centerOfLadderX, 1.5);
				m_pParent->SetPos(pos);
			}
		}

	} else
	{

		//see if they are trying to mount the ladder	

		//we need some special checks here to see if this is the start or end of a ladder, otherwise
		//we can't smoothly jump up while standing on a ladder, because it will mount it if even for
		//an extremely short time

		bool bAllowed = false;

		if (m_Keys & C_KEY_UP)
		{
			//is there really a ladder going up here?
			if (m_pParent->GetNearbyZoneByPointAndType(vPos, CMaterial::C_MATERIAL_TYPE_VERTICAL_LADDER))
			if (m_pParent->GetNearbyZoneByPointAndType(vPos - CL_Vector2(0,C_LADDER_SENSITIVITY), CMaterial::C_MATERIAL_TYPE_VERTICAL_LADDER))
			{
				bAllowed = true;
			}
		}

		if (m_Keys & C_KEY_DOWN)
		{
			
			//LogMsg("Feet are at %s", PrintVector(vPos).c_str());
			//is there really a ladder going down here?
			if (m_pParent->GetNearbyZoneByPointAndType(vPos, CMaterial::C_MATERIAL_TYPE_VERTICAL_LADDER))
			if (m_pParent->GetNearbyZoneByPointAndType(vPos+ CL_Vector2(0,C_LADDER_SENSITIVITY), CMaterial::C_MATERIAL_TYPE_VERTICAL_LADDER))
			{
				bAllowed = true;
			}
		}

		if (bAllowed)
		{
			//able to enter ladder mode
			m_pParent->SetOnLadder(true);
			m_bRequestJump = false;
			m_pParent->SetAnimByName("climb");
			m_jumpTimer = 0;
		}

	}

	if (m_pParent->GetOnLadder())
	{
		m_pParent->SetIsOnGround(true); 

		if (m_Keys & C_KEY_UP)
		{
			//move up the ladder
			m_moveAngle = m_moveAngle +CL_Vector2(0,-1);
			m_moveAngle.unitize();
		}

		if (m_Keys & C_KEY_DOWN)
		{

			//move down the ladder
			m_moveAngle = m_moveAngle +CL_Vector2(0,1);
			m_moveAngle.unitize();
		}

	}
}

string BrainPlayer::HandleAskMsg(const string &msg)
{
	vector<string> messages = CL_String::tokenize(msg, ";",true);

	string stReturn;

	for (unsigned int i=0; i < messages.size(); i++)
	{
		vector<string> words = CL_String::tokenize(messages[i], "=",true);

		if (words[0] == "get_freeze")
		{
			stReturn += CL_String::from_bool(m_bFrozen);
		} else
			{
				LogMsg("Brain %s doesn't understand ask keyword %s", GetName(), words[0].c_str());
			}
	}

	return stReturn;
}


void BrainPlayer::HandleMsg(const string &msg)
{
	vector<string> messages = CL_String::tokenize(msg, ";",true);

		for (unsigned int i=0; i < messages.size(); i++)
	{
		vector<string> words = CL_String::tokenize(messages[i], "=",true);

		if (words[0] == "lost_player_focus")
		{
			ResetKeys();
			m_walkSound.Play(false);
			m_climbSound.Play(false);
		
		} else
			if (words[0] == "got_player_focus")
			{
				ResetKeys();
				GetGameLogic->SetGameMode(GameLogic::C_GAME_MODE_SIDE_VIEW);
		
			} else
				if (words[0] == "freeze")
		{
			SetFreeze(CL_String::to_bool(words[1]));
		} else
		{
			LogMsg("Brain %s doesn't understand keyword %s", GetName(), words[0].c_str());
		}
	}
}

void BrainPlayer::CheckForMovement()
{
	if (m_Keys & C_KEY_LEFT)
	{
		m_moveAngle = CL_Vector2(-1,0);
	
		m_pParent->SetFacing(VisualProfile::FACING_LEFT);

		if (m_pParent->IsOnGround())
		{
			m_pParent->SetVisualState(VisualProfile::VISUAL_STATE_RUN);
		}
	}

	if (m_Keys & C_KEY_RIGHT)
	{
		m_moveAngle = CL_Vector2(1,0);
		m_pParent->SetFacing(VisualProfile::FACING_RIGHT);

		if (m_pParent->IsOnGround())
		{
			m_pParent->SetVisualState(VisualProfile::VISUAL_STATE_RUN);
		}
	}
}

void BrainPlayer::CalculateForce(CL_Vector2 &force, float step)
{
	float desiredSpeed = C_PLAYER_DESIRED_SPEED;
	float accelPower = C_PLAYER_ACCEL_POWER;
	float airDampening = C_PLAYER_AIR_DAMPENING;

	if (m_moveAngle != CL_Vector2::ZERO)
	{
	//	m_moveAngle.y += 0.1f;
		//LogMsg("Move angle: %s", PrintVector(m_moveAngle).c_str());
		
		force = CL_Vector2(m_moveAngle)*desiredSpeed;
		CL_Vector2 curForce = m_pParent->GetLinearVelocity()/step; //figure out what needs to change to get our desired total force
		force = force-curForce;
	
		Clamp(force.x, -accelPower, accelPower); //limit force to accel power

		if (!m_pParent->IsOnGround() || m_jumpTimer > GetApp()->GetGameTick())
		{
			//air jumps should be weaker
			force.y = 0;
			force.x *= airDampening;
		} 

		//LogMsg("WIth mass, it'd be %.2f, %.2f", force.x*step, force.y*step);
	}

	if (m_jumpTimer > GetApp()->GetGameTick())
	{
		//we're jumping right now, apply steady force

		if ( m_jumpBurstTimer < GetApp()->GetGameTick()
			&& !(m_Keys & C_KEY_UP))
		{
			//m_bAppliedSecondJumpForce = true;
			//LogMsg("Applying second");
			force.y = C_PLAYER_JUMP_DRAG_DOWN;
			m_jumpTimer += 1; //so gravity will keep pushing down
		} else
		{
			if (!m_bAppliedSecondJumpForce)
			{
				force.y = -m_curJumpForce;
				m_curJumpForce = C_PLAYER_JUMP_SECOND_FORCE;
				m_bAppliedSecondJumpForce = true;
			} else
			{

				force.y = -m_curJumpForce;
				m_curJumpForce *= C_PLAYER_JUMP_FORCE_WEAKEN_MULT;
			}
		}

	} else
	{
		
		//push the player down if we are in the air and not doing an official jump

		if (!m_pParent->IsOnGround() || (m_pParent->GetFloorMaterialID() != -1 && g_materialManager.GetMaterial(m_pParent->GetFloorMaterialID())->GetSpecial() == CMaterial::C_MATERIAL_SPECIAL_FLOOR) )
		{
			//LogMsg("Player y is %f", m_pParent->GetBody()->GetLinVelocity().y);
			if (!m_pParent->GetOnLadder())
				if (m_pParent->GetBody()->GetLinVelocity().y < C_MAX_FALLING_DOWN_SPEED)
				{
					force.y = C_PLAYER_JUMP_DRAG_DOWN;
				}
		}
	}
}

void BrainPlayer::CheckForAttack()
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

void BrainPlayer::UpdateMovement(float step)
{
	VisualProfile *pProfile = m_pParent->GetVisualProfile();

	m_moveAngle = CL_Vector2(0,0);

	if (!m_bFrozen)
	{
		CheckForMovement();
		CheckForLadder();
		CheckForDoor();
		CheckForAttack();

		//check for jump
		if ( m_Keys & C_KEY_UP && !m_pParent->GetOnLadder())
		{
			m_bRequestJump = false;
			AttemptToJump();
		}

		//if not moving, force idle state
		if (m_moveAngle == CL_Vector2::ZERO)
		{
			if (m_pParent->IsOnGround() && !m_pParent->GetOnLadder())
			{
				m_pParent->SetVisualState(VisualProfile::VISUAL_STATE_IDLE);
			}
		}
	}
	
	CL_Vector2 force = CL_Vector2(0,0);
	
	CalculateForce(force, step);

	//LogMsg("Cur Linear: %.2f, %.2f  Impulse: %.2f, %.2f", GetBody()->GetLinVelocity().x, GetBody()->GetLinVelocity().y, force.x, force.y);
	
	m_pParent->AddForce(force);
	
	if (m_pParent->GetOnLadder())
	{
		if (m_Keys & C_KEY_UP)
		{
			m_pParent->SetAnimPause(false);
		} else
			if (m_Keys & C_KEY_DOWN)
			{
				m_pParent->SetAnimPause(false);
			} else
			{
				m_pParent->SetAnimPause(true);
			}
	} else
	{
		m_pParent->SetSpriteByVisualStateAndFacing();
	}
	
	//play walk sound?

	

	if (m_pParent->IsOnGround() && !m_pParent->GetOnLadder() && m_pParent->GetBody()->GetLinVelocity().Length() > 1)
	{
		if (m_Keys) //only play a sound if a key is pressed
		m_walkSound.Play(true);
	} else
	{
		m_walkSound.Play(false);
	}

	//handle the sounds for the ladder here

	if (m_pParent->GetOnLadder() && m_moveAngle != CL_Vector2::ZERO)
	{
		if (m_Keys) //only play a sound if a key is pressed
		m_climbSound.Play(true);
	} else
	{
		m_climbSound.Play(false);
	}
}


void BrainPlayer::OnAction()
{
	CL_Vector2 vStartPos = m_pParent->GetPos();
	//first, look for any entity directly behind us
	if (TryToDoActionAtPoint(vStartPos, m_pParent)) return;

	CL_Vector2 vFacing = FacingToVector(m_pParent->GetFacing());

	const int actionRange = 60;
	TileEntity *pTileEnt;
	CL_Vector2 vEndPos;
	vEndPos = vStartPos + (vFacing * actionRange);

	//shoot it at an angle starting from our knees
	vStartPos.y += (m_pParent->GetSizeY()/3);

//	LogMsg("Starting ground check from %.2f, %.2f, to %.2f, %.2f", 
//		vStartPos.x, vStartPos.y, vEndPos.x, vEndPos.y);

	CL_Vector2 vColPos;
	Tile *pTile = NULL;

	if (GetTileLineIntersection(vStartPos, vEndPos, m_pParent->GetNearbyTileList(), &vColPos, pTile, m_pParent->GetTile() ))
	{
		 //LogMsg("Found tile at %.2f, %.2f", vColPos.x, vColPos.y);
		 switch ((pTile)->GetType())
		 {

		 case C_TILE_TYPE_PIC:
			 //g_textManager.Add("A wall.", m_pParent);
			 break;

		 case C_TILE_TYPE_ENTITY:

			 pTileEnt = static_cast<TileEntity*>(pTile);
			 if (pTileEnt->GetEntity()->GetScriptObject() && pTileEnt->GetEntity()->GetScriptObject()->FunctionExists("OnAction"))
			 {
					try {luabind::call_function<bool>(pTileEnt->GetEntity()->GetScriptObject()->GetState(), 
						"OnAction", m_pParent);
					} LUABIND_ENT_BRAIN_CATCH("Error while calling OnAction(Entity)");
			 } 
				return;
			 break;
		 }
	}

//on last ditch effort
	//higher
	if (TryToDoActionAtPoint(vStartPos, m_pParent)) return;

	//lower
	vStartPos.y -= (m_pParent->GetSizeY()/1.6f);
	if (TryToDoActionAtPoint(vStartPos, m_pParent)) return;

	//towards where we are facing
	vStartPos += vFacing*10;
	if (TryToDoActionAtPoint(vStartPos, m_pParent)) return;
	g_textManager.Add("Hmm?", m_pParent);
}

void BrainPlayer::Update(float step)
{
	AssignPlayerToCameraIfNeeded(m_pParent);

	if (!m_pParent->GetCollisionData())
	{
		LogMsg("Error, player brain requires collision data to function.");
	} else
	{
		//Calculate the steering force and update the bot's velocity and position
		UpdateMovement(step);
	}
}

void BrainPlayer::PostUpdate(float step)
{
	m_pParent->GetBody()->GetAngVelocity() = 0;
	
	if (m_pParent->IsOnGround() && (m_moveAngle == CL_Vector2::ZERO) )
		{
			//slow down
			set_float_with_target(&m_pParent->GetBody()->GetLinVelocity().x, 0, C_PLAYER_GROUND_DAMPENING);

		if (m_pParent->GetOnLadder())
		{
			set_float_with_target(&m_pParent->GetBody()->GetLinVelocity().y, 0, C_PLAYER_GROUND_DAMPENING);
		}
	}

	if (m_bFrozen) return;

	if (m_Keys & C_KEY_SELECT)
	{
		m_Keys &= ~C_KEY_SELECT; //turn it off
		OnAction();
	}
}


bool TryToDoActionAtPoint(const CL_Vector2 &vecPos, MovingEntity *pEnt)
{
	TileEntity *pTileEnt;
	int ttype;
	tile_list::iterator itor = pEnt->GetNearbyTileList().begin();
	for(;itor != pEnt->GetNearbyTileList().end(); itor++)
	{

		ttype = (*itor)->GetType();
		if (ttype == C_TILE_TYPE_ENTITY)
		{
			pTileEnt = (TileEntity*)(*itor);

			if (pTileEnt->GetWorldRect().is_inside(CL_Pointf(vecPos.x, vecPos.y)))
				if (pTileEnt->GetEntity()->GetScriptObject() && pTileEnt->GetEntity()->GetScriptObject()->FunctionExists("OnAction"))
				{
					
					MovingEntity *m_pParent = pEnt; //hack so the macro works right
					try {luabind::call_function<bool>(pTileEnt->GetEntity()->GetScriptObject()->GetState(), 
						"OnAction", pEnt);
					} LUABIND_ENT_BRAIN_CATCH("Error while calling OnAction(Entity)");

					return true;
				} 
		}
	}

	return false; //didn't find anything
}


bool ConvertKeysToDirection(unsigned int keys, int &visualFacingOut)
{
	bool bIdle = true;

	if (keys & C_KEY_LEFT)
	{
		bIdle = false;
		visualFacingOut = (VisualProfile::FACING_LEFT); 
	}

	if (keys & C_KEY_RIGHT)
	{
		visualFacingOut = (VisualProfile::FACING_RIGHT);
		bIdle = false;
	}

	if (keys & C_KEY_UP)
	{
		visualFacingOut = (VisualProfile::FACING_UP);
		bIdle = false;
	}

	if (keys & C_KEY_DOWN)
	{
		visualFacingOut = (VisualProfile::FACING_DOWN);
		bIdle = false;
	}

	if (keys & C_KEY_DOWN && keys & C_KEY_LEFT)
	{
		visualFacingOut = (VisualProfile::FACING_DOWN_LEFT);
		bIdle = false;
	}

	if (keys & C_KEY_UP && keys & C_KEY_LEFT)
	{
		visualFacingOut = (VisualProfile::FACING_UP_LEFT);
		bIdle = false;
	}

	if (keys & C_KEY_DOWN && keys & C_KEY_RIGHT)
	{
		visualFacingOut = (VisualProfile::FACING_DOWN_RIGHT);
		bIdle = false;
	}

	if (keys & C_KEY_UP && keys & C_KEY_RIGHT)
	{
		visualFacingOut = (VisualProfile::FACING_UP_RIGHT);
		bIdle = false;
	}

	return !bIdle;
}