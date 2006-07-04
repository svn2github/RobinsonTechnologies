#include "AppPrecomp.h"
#include "MovingEntity.h"
#include "GameLogic.h"
#include "TileEntity.h"
#include "EntWorldCache.h"
#include "EntCollisionEditor.h"
#include "ScriptManager.h"
#include "BrainPlayer.h"
#include "BrainShake.h"
#include "VisualProfileManager.h"
#include "physics/Contact.h"
#include "MaterialManager.h"

#ifndef WIN32
//windows already has this in the precompiled header for speed, I couldn't get that to work on mac..
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#endif

#define C_GROUND_RELAX_TIME_MS 150
#define C_DEFAULT_SCRIPT_PATH "media/script/"
#define C_DEFAULT_SCRIPT "system/ent_default.lua"
//there are issues with sliding along walls thinking it's a ground when they are built out of small blocks, this
//provides a work around
#define C_TICKS_ON_GROUND_NEEDED_TO_TRIGGER_GROUND 1

#define C_ENT_TILE_PIC_ID_KEY "_TilePicID" //custom data to signal we use a simple single tilepic for our image
#define C_ENT_TILE_RECT_KEY "_TileRect" //custom data that goes along with the tile ID
#define C_ENT_DENSITY_KEY "_Density" //custom data to override density

#define C_DEFAULT_DENSITY 0.4f

MovingEntity::MovingEntity(): BaseGameEntity(BaseGameEntity::GetNextValidID())
{
	m_mainScript = C_DEFAULT_SCRIPT;
	m_iType = C_ENTITY_TYPE_MOVING;
	m_defaultTextColor = CL_Color(255,100,200,255);
	m_hashedName = 0;
	m_bUsingCustomCollisionData = false;
	m_brainManager.SetParent(this);
	SetDefaults();
}

MovingEntity::~MovingEntity()
{
	Kill();
}

void MovingEntity::SetCollisionScale(const CL_Vector2 &vScale)
{
	if (!m_pCollisionData)
	{
		LogError("Error, can't ScaleCollisionInfo, there is no collision data here");
		return;
	}

	if (m_bUsingCustomCollisionData)
	m_pCollisionData->SetScale(vScale);
}

CL_Vector2 MovingEntity::GetCollisionScale()
{
	if (!m_pCollisionData)
	{
		LogError("Error, can't GetCollisionScale, there is no collision data here");
		return CL_Vector2(0,0);
	}

	return m_pCollisionData->GetScale();
}

void MovingEntity::ClearColorMods()
{
	m_colorModRed = 0;
	m_colorModGreen = 0;
	m_colorModBlue = 0;
	m_colorModAlpha = 0;
}

void MovingEntity::Kill()
{

	m_brainManager.Kill(); //reset it

	m_pVisualProfile = NULL;
	
	if (m_bUsingCustomCollisionData)
	{
		SAFE_DELETE(m_pCollisionData);
		m_bUsingCustomCollisionData = false;
	}
	
	SAFE_DELETE(m_pSprite);
	SAFE_DELETE(m_pScriptObject);
	SetDefaults();
}

void MovingEntity::HandleMessageString(const string &msg)
{
	if (GetScriptObject())
	{
		GetScriptObject()->RunString(msg.c_str());
	} else
	{
		LogMsg("Can't deliver message %s, script not active in ent %d",
			msg.c_str(), ID());
	}
}

void MovingEntity::SetDefaults()
{
	ClearColorMods();
	m_nearbyTileList.clear();
	m_floorMaterialID = -1;
	m_bUsingSimpleSprite = false;
	m_listenCollision = LISTEN_COLLISION_NONE;
	m_listenCollisionStatic = LISTEN_COLLISION_STATIC_NONE;
	m_fDensity = C_DEFAULT_DENSITY; //default
	m_bIsAtRest = false;
	m_groundTimer = 0;
	m_bMovedFlag = true;
	m_pScriptObject = NULL;
	m_timeLastActive = INT_MAX;
	m_pSprite = NULL;
	m_pVisualProfile = NULL; //note, we don't init or delete this ourself, visual manager handles it
	m_pSpriteLastUsed = NULL;
	m_pCollisionData = NULL;
	m_ticksOnGround = 0;
	m_bOnLadder = false;
	m_bAnimPaused = false;
	
	m_visualState = VisualProfile::VISUAL_STATE_IDLE;
	m_facing = VisualProfile::FACING_LEFT;

}

void MovingEntity::SetName(const std::string &name)
{
	//setting a name has a special meaning when done in a moving entity.  It
	//means this entity can be tracked by its name from anywhere in the game, it's
	//stored in a special database that is loaded even when the entities world
	//isn't.  So don't set this unless an entity specifically needs a trackable
	//identity

	if (m_pTile && name.empty())
	{
		if  (m_hashedName != 0)
		{
			//need to remove ourself
			GetTagManager->Remove(this);
		}
		m_hashedName = 0;
		return;
	}

	World *pWorld = NULL;
	if (m_pTile)
	{
		if (name != GetName())
		{
			if (m_pTile->GetParentScreen())
			{
				//by setting the world, we signal that we want to make a change to the tag DB now
				pWorld = m_pTile->GetParentScreen()->GetParentWorldChunk()->GetParentWorld();
			}
		}
	}
	
	if (pWorld)
	GetTagManager->Remove(this);

	BaseGameEntity::SetName(name);
	if (name.empty())
	{
		m_hashedName = 0;
	} else
	{
		m_hashedName = HashString(name.c_str());
	}
	
	if (pWorld)
	GetTagManager->Update(pWorld, this);
}

void MovingEntity::Serialize(CL_FileHelper &helper)
{
	//load/save needed data
	cl_uint8 ver = m_pTile->GetParentScreen()->GetVersion();

    helper.process(m_mainScript);

	string temp;

	if (helper.IsWriting())
	{
		temp = GetName();
		helper.process(temp);
	} else
	{
		helper.process(temp);
		SetName(temp);
	}

	//LogMsg("Loading version %d", ver);
    GetData()->Serialize(helper);

	if (!helper.IsWriting())
	{
		//we need to init this too
		Init();
	}
}

void MovingEntity::SetMainScriptFileName(const string &fileName)
{
	m_mainScript = fileName;
}

BaseGameEntity * MovingEntity::CreateClone(TileEntity *pTile)
{
	MovingEntity *pNew = new MovingEntity;
	pTile->SetEntity(pNew);

	pNew->SetMainScriptFileName(m_mainScript);
	pNew->SetName(GetName());
	pNew->SetPos(GetPos());
	*pNew->GetData() = m_dataManager; //copy our entity specific vars too
	pNew->Init();

	return pNew;
}

CL_Rectf MovingEntity::GetWorldRect()
{
	static CL_Rectf r;
	r.left = 0;
	r.top = 0;
	r.right = GetSizeX();
	r.bottom = GetSizeY();
	static CL_Origin origin;
	static int x,y;
	m_pSprite->get_alignment(origin, x, y);
	static CL_Pointf offset;
	offset = calc_origin(origin, r.get_size());
	r -= offset;
	r += *(const CL_Pointf*)(&GetPos());
	return r;
}

const CL_Rect & MovingEntity::GetBoundsRect()
{
	static CL_Rect r;
	r.left = 0;
	r.top = 0;
	r.right = GetSizeX();
	r.bottom = GetSizeY();
	static CL_Origin origin;
	static int x,y;
	m_pSprite->get_alignment(origin, x, y);
	static CL_Point offset;
	offset = calc_origin(origin, r.get_size());
	r -= offset;
	return r;
}

string MovingEntity::ProcessPath(const string &st)
{
	if (st[0] == '~')
	{
		return C_DEFAULT_SCRIPT_PATH + CL_String::get_path(m_mainScript) + (st.c_str()+1);
	}

	return st;
}

bool MovingEntity::SetVisualProfile(const string &resourceFileName, const string &profileName)
{
	VisualResource *pResource = GetVisualProfileManager->GetVisualResource(ProcessPath(resourceFileName));
	if (!pResource) return false; //failed that big time

	//now we need to get the actual profile
	m_pVisualProfile = pResource->GetProfile(profileName);
	return true; //success
}

void MovingEntity::SetPos(const CL_Vector2 &new_pos)
{
	m_body.GetPosition().x = new_pos.x;
	m_body.GetPosition().y = new_pos.y;
	m_bMovedFlag = true;
}

bool MovingEntity::SetPosAndMapByTagName(const string &name)
{
	TagObject *pO = GetTagManager->GetFromString(name);

	if (!pO)
	{
		LogMsg("SetPosAndMapByName: Unable to locate entity by name: %s", name.c_str());
		return false;
	} 
	float offsetY = GetWorldRect().bottom - GetPos().y;
	//we have to deduct half the height because this is written to move our FEET to the point
	//to warp to. 
	
	if (GetCollisionData() && GetCollisionData()->GetLineList()->size() > 0 )
	{
		PointList *m_pActiveLine = &(*GetCollisionData()->GetLineList()->begin());
		offsetY = m_pActiveLine->GetRect().bottom + m_pActiveLine->GetOffset().y;
	}

	SetPosAndMap(pO->m_pos - CL_Vector2(0, offsetY), pO->m_pWorld->GetName());
	return true; 
}

void MovingEntity::SetPosAndMap(const CL_Vector2 &new_pos, const string &worldName)
{

	if (worldName != m_pTile->GetParentScreen()->GetParentWorldChunk()->GetParentWorld()->GetName())
	{
//		LogMsg("Changing world to %s", worldName.c_str());
		//we also need to move it to a new world
		m_strWorldToMoveTo = worldName; //can't do it now, but we'll do it asap
		//we can't just move now, because we still need to finish drawing this frame
		m_moveToAtEndOfFrame = new_pos;
	} else
	{
		m_body.GetPosition().x = new_pos.x;
		m_body.GetPosition().y = new_pos.y;
	}
	
	m_bMovedFlag = true;
}


Zone * MovingEntity::GetZoneWeAreOnByMaterialType(int matType)
{
	for (unsigned int i=0; i < m_zoneVec.size(); i++)
	{
		if (g_materialManager.GetMaterial(m_zoneVec[i].m_materialID)->GetType() == matType)
		{
			return &m_zoneVec[i]; //valid for the rest of this frame
		}
	}

 return NULL; //nothing found
}

Zone * MovingEntity::GetNearbyZoneByPointAndType(const CL_Vector2 &vPos, int matType)
{
	static CL_Vector2 v;
	for (unsigned int i=0; i < m_zoneVec.size(); i++)
	{
		if (g_materialManager.GetMaterial(m_zoneVec[i].m_materialID)->GetType() == matType)
		{
			v = vPos;
			v -= m_zoneVec[i].m_vPos;
			if (m_zoneVec[i].m_boundingRect.is_inside(*(CL_Pointf*)&(v)))
			{
				return &m_zoneVec[i]; //valid for the rest of this frame
			}
		}
	}

	return NULL; //nothing found
}

void MovingEntity::GetAlignment(CL_Origin &origin, int &x, int &y)
{
	m_pSprite->get_alignment(origin, x, y);
}


//Note, this is NOT safe to use except in the post update functions.  An entity might have
//been deleted otherwise.

tile_list & MovingEntity::GetNearbyTileList()
{
	if (GetGameLogic->GetGamePaused())
	{
		m_nearbyTileList.clear();
	}
	return m_nearbyTileList;
}

bool MovingEntity::Init()
{
	m_body.SetParentEntity(this);
	assert(!m_pSprite);

	unsigned int picID = CL_String::to_int(m_dataManager.Get(C_ENT_TILE_PIC_ID_KEY));
	
	if (picID != 0)
	{
		
		CL_Surface *pSurf = GetHashedResourceManager->GetResourceByHashedID(picID);
		if (!pSurf)
		{
			LogError("Error, entity %d (%s) is unable to find image with hash %d to use.", ID(), GetName().c_str(), picID);
		} else
		{
			CL_Rect picSrcRect = StringToRect(m_dataManager.Get(C_ENT_TILE_RECT_KEY));
		
			m_pSprite = new CL_Sprite();
			m_pSprite->add_frame(*pSurf, picSrcRect);
	
			m_pSprite->set_alignment(origin_top_left,0,0);

			assert(!m_pCollisionData);
			SetCollisionInfoFromPic(picID, picSrcRect);
			m_bUsingSimpleSprite = true;
		}

	}

	if (!m_pSprite)
	m_pSprite = new CL_Sprite(); //visual data will be set on it later


	string stEmergencyMessage;

	if (!m_mainScript.empty())
	if (!LoadScript(m_mainScript.c_str()))
	{
		LoadScript(C_DEFAULT_SCRIPT);
		stEmergencyMessage = "Can't find lua script " + m_mainScript +".  Loading default.lua instead.";
	}

	if (m_pSprite->is_null())
	{
		//avoid a crash
		if (!m_pVisualProfile)
		{
			
			if (m_mainScript.empty())
			{
				LogError("(giving entity %d (%s) the default script so you can see it to delete it)", ID(), GetName().c_str());
			} else
			{
				LogError("No visual profile was set in script %s's init!  Maybe it had a syntax error, go check the log. (trying to load default)", m_mainScript.c_str());
			}
			SetVisualProfile(GetGameLogic->GetScriptRootDir()+"/system/system.xml", "ent_default");
		}

		if (!m_pVisualProfile->GetSprite(VisualProfile::VISUAL_STATE_IDLE, VisualProfile::FACING_LEFT))
		{
			LogMsg("Unable to set default anim of idle_left in profile %s!", m_pVisualProfile->GetName().c_str());
		}
		SetSpriteData(m_pVisualProfile->GetSprite(VisualProfile::VISUAL_STATE_IDLE, VisualProfile::FACING_LEFT));
	}

	if (!stEmergencyMessage.empty())
	{
		//message box causes crashes!
		//CL_MessageBox::info(stEmergencyMessage, GetApp()->GetGUI());
		LogMsg("ERROR - %s", stEmergencyMessage.c_str());
	}

	//override settings with certain things found
	if (m_dataManager.Exists(C_ENT_DENSITY_KEY))
	{
		
		if (GetCollisionData() && GetCollisionData()->HasData())
		{
			SetDensity(CL_String::to_float(m_dataManager.Get(C_ENT_DENSITY_KEY)));
		}
	}

	return true;
}

bool MovingEntity::LoadScript(const char *pFileName)
{
	SAFE_DELETE(m_pScriptObject);
	m_pScriptObject = new ScriptObject();
	string s = C_DEFAULT_SCRIPT_PATH;
	s += pFileName;
	
	if (!m_pScriptObject->Load(s.c_str()))
	{
		return false;
	}

	//set this script objects "this" variable to point to an instance of this class
	luabind::globals(m_pScriptObject->GetState())["this"] = this;
	m_pScriptObject->RunFunction("Init");
	return true;
}

luabind::object MovingEntity::RunFunction(const string &func)
{
	luabind::object ret;
	
	if (!m_pScriptObject)
	{
		LogError("Can't RunFunction %s on entity %d (%s), no script is attached.",
			func.c_str(), ID(), GetName().c_str());
		return ret;
	}

	try {ret = luabind::call_function<luabind::object>(m_pScriptObject->GetState(), func.c_str());
	} LUABIND_ENT_CATCH( ("Error while calling function " + func).c_str());
	return ret;
}


luabind::object MovingEntity::RunFunction(const string &func, luabind::object obj1)
{
	luabind::object ret;

	if (!m_pScriptObject)
	{
		LogError("Can't RunFunction %s on entity %d (%s), no script is attached.",
			func.c_str(), ID(), GetName().c_str());
		return ret;
	}

	try {ret = luabind::call_function<luabind::object>(m_pScriptObject->GetState(), func.c_str(), obj1);
	} LUABIND_ENT_CATCH( ("Error while calling function " + func).c_str());
	return ret;
}

luabind::object MovingEntity::RunFunction(const string &func, luabind::object obj1, luabind::object obj2)
{

	luabind::object ret;

	if (!m_pScriptObject)
	{
		LogError("Can't RunFunction %s on entity %d (%s), no script is attached.",
			func.c_str(), ID(), GetName().c_str());
		return ret;
	}

	try {ret = luabind::call_function<luabind::object>(m_pScriptObject->GetState(), func.c_str(), obj1, obj2);
	} LUABIND_ENT_CATCH( ("Error while calling function " + func).c_str());
	return ret;
}

luabind::object MovingEntity::RunFunction(const string &func, luabind::object obj1, luabind::object obj2, luabind::object obj3)
{
	luabind::object ret;

	if (!m_pScriptObject)
	{
		LogError("Can't RunFunction %s on entity %d (%s), no script is attached.",
			func.c_str(), ID(), GetName().c_str());
		return ret;
	}

	try {ret = luabind::call_function<luabind::object>(m_pScriptObject->GetState(), func.c_str(), obj1, obj2, obj3);
	} LUABIND_ENT_CATCH( ("Error while calling function " + func).c_str());
	return ret;
}

void MovingEntity::UpdateTilePosition()
{
		assert(m_pTile);
		assert(m_pTile->GetParentScreen());

	if (m_bMovedFlag)
	{
		World *pWorldToMoveTo = m_pTile->GetParentScreen()->GetParentWorldChunk()->GetParentWorld();
		
		if (!m_strWorldToMoveTo.empty())
		{
			 WorldInfo *pInfo = GetWorldManager->GetWorldInfoByName(m_strWorldToMoveTo);

			 if (pInfo)
			 {
				pWorldToMoveTo = &pInfo->m_world;
				m_body.GetPosition().x = m_moveToAtEndOfFrame.x;
				m_body.GetPosition().y = m_moveToAtEndOfFrame.y;

				if (this == GetPlayer)
				{
					 //the camera should follow the player
					CameraSetting cs;
					GetCamera->GetCameraSettings(cs);
					GetWorldManager->SetActiveWorldByPath(pInfo->m_world.GetDirPath(),&cs);
				}
			 } else
			 {

				 LogMsg("Error: Entity unable to move, world %s not found", m_strWorldToMoveTo.c_str());
				 m_bMovedFlag = false;
				 m_strWorldToMoveTo.clear();
				 return;
			 }

			 m_strWorldToMoveTo.clear();	 
		} 
		
		m_pTile->SetEntity(NULL); //so it won't delete us
		TileEntity *pTileEnt = (TileEntity*) m_pTile->CreateClone();
		m_pTile->GetParentScreen()->RemoveTileByPointer(m_pTile);

		//add it back
		pTileEnt->SetEntity(this); //this will set our m_pTile automatically
		pWorldToMoveTo->AddTile(m_pTile);
		m_bMovedFlag = false;
	} 

}

void MovingEntity::SetAnimByName(const string &name)
{
	if (GetVisualProfile())
	{
		SetSpriteData(GetVisualProfile()->GetSpriteByAnimID(GetVisualProfile()->TextToAnimID(name)));
	}
}

void MovingEntity::OnDamage(const CL_Vector2 &normal, float depth, MovingEntity * enemy, int damage, int uservar, MovingEntity * pProjectile)
{
	if (!GetScriptObject() || !GetScriptObject()->FunctionExists("OnDamage")) return;

	try {luabind::call_function<bool>(m_pScriptObject->GetState(), "OnDamage", normal, depth, enemy, damage, uservar, pProjectile);
	} LUABIND_ENT_CATCH("Error while calling OnDamage(Vector2 normal, float depth, enemy, int damage, int uservar, projectile)");
}

void MovingEntity::SetSpriteByVisualStateAndFacing()
{
	if (GetVisualProfile()) SetSpriteData(GetVisualProfile()->GetSprite(GetVisualState(), GetFacing()));
}

void MovingEntity::LastCollisionWasInvalidated()
{
	m_bTouchedAGroundThisFrame = m_bOldTouchedAGroundThisFrame;
	m_floorMaterialID = m_oldFloorMaterialID;

}

void MovingEntity::OnCollision(const Vector & N, float &t, CBody *pOtherBody, bool *pBoolAllowCollision)
{

	m_bOldTouchedAGroundThisFrame = m_bTouchedAGroundThisFrame;
	m_oldFloorMaterialID = m_floorMaterialID;


	if (N.y < -0.7f) //higher # here means allows a stronger angled surface to count as "ground"
	{
		m_bTouchedAGroundThisFrame = true;
		m_floorMaterialID = pOtherBody->GetMaterial()->GetID();

	}
	

	if (pOtherBody->GetParentEntity() != 0)
	{
		switch (GetListenCollision())
		{

		case LISTEN_COLLISION_NONE:
			break;

		case LISTEN_COLLISION_PLAYER_ONLY:
			
			if ( pOtherBody->GetParentEntity() == GetPlayer)
			{
				try {	*pBoolAllowCollision = luabind::call_function<bool>(m_pScriptObject->GetState(), "OnCollision", CL_Vector2(N.x, N.y), t,  pOtherBody->GetMaterial()->GetID(),  pOtherBody->GetParentEntity());
				} LUABIND_ENT_CATCH("Error while calling OnCollision(Vector2, float, materialID, Entity)");
			}
			break;

		case LISTEN_COLLISION_ALL_ENTITIES:
			try {	*pBoolAllowCollision = luabind::call_function<bool>(m_pScriptObject->GetState(), "OnCollision", CL_Vector2(N.x, N.y), t,  pOtherBody->GetMaterial()->GetID(), pOtherBody->GetParentEntity());
			} LUABIND_ENT_CATCH("Error while calling OnCollision(Vector2, float, materialID, Entity)");

			break;
		default:
			assert(!"Bad collision listen value");
		}
	} else
	{
		//it's a wall that touched them
		switch (m_listenCollisionStatic)
		{
		case LISTEN_COLLISION_STATIC_NONE:
			break;

		case LISTEN_COLLISION_STATIC_ALL:
			try {	*pBoolAllowCollision = luabind::call_function<bool>(m_pScriptObject->GetState(), "OnCollisionStatic", CL_Vector2(N.x, N.y), t, pOtherBody->GetMaterial()->GetID());
			} LUABIND_ENT_CATCH("Error while calling OnCollisionStatic(Vector2, float, materialID)");

			break;
		
		default:
			assert(!"Unknown collision static type");
		}
	}
}

void MovingEntity::SetIsOnGround(bool bOnGround)
{
	if (bOnGround)
	{
		m_groundTimer = GetApp()->GetGameTick()+C_GROUND_RELAX_TIME_MS;
	} else
	{
		m_groundTimer = 0;
		m_ticksOnGround = 0;
	}
}

void MovingEntity::SetDensity(float fDensity)
{
	m_fDensity = fDensity;
	GetBody()->SetDensity(m_fDensity);
}
void MovingEntity::EnableRotation(bool bRotate)
{
	if (bRotate)
	{
		GetBody()->SetDensity(m_fDensity);

	} else
	{
		GetBody()->SetInertia(0); //disable rotation

	}
}

bool MovingEntity::GetEnableRotation()
{
	return GetBody()->GetInertia() != 0;
}

void MovingEntity::SetScale(const CL_Vector2 &vScale)
{
	m_pTile->SetScale(vScale);
}

void MovingEntity::LoadCollisionInfo(const string &fileName)
{
	if (m_bUsingCustomCollisionData)
	{
  		SAFE_DELETE(m_pCollisionData);
	}
	m_pCollisionData = new CollisionData;
    m_bUsingCustomCollisionData = true;
	
	m_pCollisionData->Load(ProcessPath(fileName)); //it will init a default if the file is not found, and automatically handle

	PointList pl;
	PointList *pActiveLine;

	if (m_pCollisionData->GetLineList()->size() == 0)
	{
		//add default data
		m_pCollisionData->GetLineList()->push_back(pl);
		pActiveLine = &(*m_pCollisionData->GetLineList()->begin());
	
		//add our lines by hand
		pActiveLine->GetPointList()->resize(4);
		pActiveLine->GetPointList()->at(0) = CL_Vector2(-10,-10);
		pActiveLine->GetPointList()->at(1) = CL_Vector2(10,-10);
		pActiveLine->GetPointList()->at(2) = CL_Vector2(10,10);
		pActiveLine->GetPointList()->at(3) = CL_Vector2(-10,10);
		pActiveLine->CalculateOffsets();
	}

	pActiveLine = &(*m_pCollisionData->GetLineList()->begin());
	//get notified of collisions
	m_collisionSlot = m_body.sig_collision.connect(this, &MovingEntity::OnCollision);

	//link to data correctly
	pActiveLine->GetAsBody(CL_Vector2(0,0), &m_body);
	GetBody()->SetDensity(m_fDensity);
	
	m_pCollisionData->SetScale(GetScale());
}

void MovingEntity::SetCollisionInfoFromPic(unsigned picID, const CL_Rect &recPic)
{
	if (m_bUsingCustomCollisionData)
	{
		SAFE_DELETE(m_pCollisionData);
	}

	m_bUsingCustomCollisionData = false;
	
	//copy it
	m_pCollisionData = GetHashedResourceManager->GetCollisionDataByHashedIDAndRect(picID, recPic);

	PointList pl;
	PointList *pActiveLine = NULL;

	bool bHadNoData = false;
	if (m_pCollisionData->GetLineList()->size() != 0)
	{
		pActiveLine = &(*m_pCollisionData->GetLineList()->begin());
		if (!pActiveLine->HasData())
		{
			//we'll, we see a line but no points so this won't work well
			pActiveLine = NULL;
		}
	}

	m_pSprite->set_alignment(origin_center);

	//get notified of collisions
	m_collisionSlot = m_body.sig_collision.connect(this, &MovingEntity::OnCollision);

	//link to data correctly
	
	if (pActiveLine)
	{
		pActiveLine->GetAsBody(CL_Vector2(0,0), &m_body);
		GetBody()->SetDensity(m_fDensity);
	} else
	{
		SetMass(0); //immovable
	}
	
}

void MovingEntity::InitCollisionDataBySize(float x, float y)
{
	if (m_bUsingCustomCollisionData)
	{
		SAFE_DELETE(m_pCollisionData);
	}

	if (x == 0 && y == 0)
	{
		//let's guess here
		x = m_pTile->GetBoundsSize().x;
		y = m_pTile->GetBoundsSize().y;
	}
	
	m_pCollisionData = new CollisionData;
	
	PointList pl;
	m_pCollisionData->GetLineList()->push_back(pl);
	PointList *pActiveLine;
	pActiveLine = &(*m_pCollisionData->GetLineList()->begin());

	//add our lines by hand
    pActiveLine->GetPointList()->resize(4);
	pActiveLine->GetPointList()->at(0) = CL_Vector2(-x,-y);
	pActiveLine->GetPointList()->at(1) = CL_Vector2(x,-y);
	pActiveLine->GetPointList()->at(2) = CL_Vector2(x,y);
	pActiveLine->GetPointList()->at(3) = CL_Vector2(-x,y);
	pActiveLine->CalculateOffsets();
	//get notified of collisions
 	m_collisionSlot = m_body.sig_collision.connect(this, &MovingEntity::OnCollision);
	m_bUsingCustomCollisionData = true;

	GetBody()->SetDensity(1);
}

void MovingEntity::ProcessCollisionTile(Tile *pTile, float step)
{
	CollisionData *pCol = pTile->GetCollisionData();

	if (!pCol || pCol->GetLineList()->empty()) return;

	if (pTile == m_pTile) return; //don't process ourself, how much sense would that make?

	//if we are on  their list, we also don't want to process, because processing a pair once is all
	//that is needed

	if ( pTile->GetType() == C_TILE_TYPE_ENTITY)
	{
		static MovingEntity *pEnt;
		pEnt = ((TileEntity*)pTile)->GetEntity();

		if (pEnt->GetDrawID() == m_drawID)
		{
			//he's already been processed and we have to assume he would have process us too
			//note, it's possible this could be wrong as they may get their "neighbor list" slightly
			//differently, but I don't think it's possible for one to miss if they actually DO touch
			return;
		}

	}
	
	static CollisionData colCustomized;
	if (pTile->UsesTileProperties())
	{
		//we need a customized version
		CreateCollisionDataWithTileProperties(pTile, colCustomized);
		pCol = &colCustomized; //replacement version
	}

	line_list::iterator lineListItor = pCol->GetLineList()->begin();
	CBody *pWallBody;
	while (lineListItor != pCol->GetLineList()->end())
	{
		if (lineListItor->HasData() && g_materialManager.GetMaterial(lineListItor->GetType())->GetType()
			== CMaterial::C_MATERIAL_TYPE_NORMAL)
		{
			
			if (pTile->GetCustomBody())
			{
				//it's a real entity
				pWallBody = &lineListItor->GetAsBody(pTile->GetPos() /*-lineListItor->GetOffset()*/, pTile->GetCustomBody());
				//LogMsg("Colliding %d and %d at drawID %d, time: %d, delta: %.2f", ID(), ((TileEntity*)pTile)->GetEntity()->ID(), GetDrawID(),
				//	GetApp()->GetGameTick(), GetApp()->GetDelta());

			} else
			{
				//static wall thing
				pWallBody = &lineListItor->GetAsBody(pTile->GetPos(), NULL);

			}
			
			//examine each line list for suitability
 		  
			if (m_pTile->UsesTileProperties())
			{
				//OPTIMIZE slow hack to allow X/Y flipping for an entity, redo later?  Depends on if we'll use this much.
				static CollisionData colMyCustomized;
				CreateCollisionDataWithTileProperties(m_pTile, colMyCustomized);
				colMyCustomized.GetLineList()->begin()->GetAsBody(CL_Vector2(0,0), &m_body);
				m_body.Collide(*pWallBody, step);

				//fix it
				m_pCollisionData->GetLineList()->begin()->GetAsBody(CL_Vector2(0,0), &m_body);

			} else
			{
				m_body.Collide(*pWallBody, step);
			}
			
		} else
		{
			//might be a ladder zone or something else we should take note of
			if (lineListItor->HasData())
			{
				/*
				static CL_Rectf r;
				static CL_Rectf myRect;

				r = lineListItor->GetRect();
				r += *(CL_Pointf*)&(pTile->GetPos());

				myRect = m_pCollisionData->GetLineList()->begin()->GetRect();
				myRect += *(CL_Pointf*)&(GetPos());
				if (r.is_overlapped(myRect))
				{
*/				
				//LogMsg("Inside: Rect is %s, which overlaps %s", PrintRect(r).c_str(),
					//	PrintRect(myRect).c_str());

					//actually add the data
					Zone z;
					z.m_materialID = lineListItor->GetType();
					z.m_boundingRect = lineListItor->GetRect();
					z.m_vPos = pTile->GetPos();
					if (pTile->GetType() == C_TILE_TYPE_ENTITY)
					{
						//i send the ID because it's safer than the pointer
						z.m_entityID = ((TileEntity*)pTile)->GetEntity()->ID();
					} else
					{
						z.m_entityID = 0;
					}
					m_zoneVec.push_back(z);
					
				//}
			}
		}
		
		lineListItor++;
	}
}

void MovingEntity::ProcessCollisionTileList(tile_list &tList, float step)
{
	if (tList.empty()) return;

	tile_list::iterator listItor = tList.begin();

	while (listItor != tList.end())
	{
				
			ProcessCollisionTile(*listItor, step);
		
		listItor++;
	}
}

void MovingEntity::SetSpriteData(CL_Sprite *pSprite)
{
	   
	if (m_pSpriteLastUsed != pSprite)
	{
		m_pSprite->set_image_data(*pSprite);
		m_pSpriteLastUsed = pSprite;
		CL_Origin origin;
		int x,y;

		m_pSprite->set_angle(pSprite->get_angle());
		m_pSprite->set_angle_pitch(pSprite->get_angle_pitch());
		m_pSprite->set_angle_yaw(pSprite->get_angle_yaw());
		pSprite->get_alignment(origin, x,y);
		m_pSprite->set_alignment(origin, x, y);
		m_pSprite->set_show_on_finish(pSprite->get_show_on_finish());

		pSprite->get_rotation_hotspot(origin, x, y);
		m_pSprite->set_rotation_hotspot(origin, x, y);
	}

}

void MovingEntity::ApplyGenericMovement(float step)
{
	m_nearbyTileList.clear();
	m_zoneVec.clear();

	if (!m_pCollisionData->GetLineList()->size())
	{
		LogError("No collision data in entity %d. (%s)", ID(), GetName());
		return;
	}

	//apply world gravity?
	
	int padding = GetSizeX();
	padding = (max(padding, GetSizeY()));
	padding = (float(padding)*0.6f)+8;

	m_scanArea.left = GetPos().x - padding;
	m_scanArea.top = GetPos().y - padding;
	m_scanArea.right = GetPos().x + padding;
	m_scanArea.bottom = GetPos().y + padding;
	
	vector<unsigned int> layerIDVec;
	World *pWorld = m_pTile->GetParentScreen()->GetParentWorldChunk()->GetParentWorld();

	pWorld->GetLayerManager().PopulateIDVectorWithAllLayers(layerIDVec);

	//TODO: Optimize this to only scan the rects that hold collision data, main, details and entity probably
	pWorld->GetMyWorldCache()->AddTilesByRect(CL_Rect(m_scanArea), &m_nearbyTileList, layerIDVec);
	
	m_pCollisionData->GetLineList()->begin()->GetAsBody(CL_Vector2(0,0), &m_body);

	ProcessCollisionTileList(m_nearbyTileList, step);
	
	if (m_bTouchedAGroundThisFrame)
	{
		m_ticksOnGround++;

		if (m_ticksOnGround >= C_TICKS_ON_GROUND_NEEDED_TO_TRIGGER_GROUND)
		{
			SetIsOnGround(true);
		}
	} else
	{
		m_ticksOnGround = 0;
	}
	
	//schedule tile update if needed
	 m_bMovedFlag = true;
}

void MovingEntity::Render(void *pTarget)
{
	CL_GraphicContext *pGC = (CL_GraphicContext *)pTarget;
	static float yawHold, pitchHold;

	yawHold = m_pSprite->get_angle_yaw();
	pitchHold = m_pSprite->get_angle_pitch();

	bool bNeedsReversed = false;

	if (m_pTile->GetBit(Tile::e_flippedX))
	{
		m_pSprite->set_angle_yaw(m_pSprite->get_angle_yaw() -180);
	}

	if (m_pTile->GetBit(Tile::e_flippedY))
	{
		m_pSprite->set_angle_pitch(m_pSprite->get_angle_pitch() -180);
	}


	if (m_pSprite->get_angle_yaw() != 0)
	{
		bNeedsReversed = !bNeedsReversed;
	}

	if (m_pSprite->get_angle_pitch() != 0)
	{
		bNeedsReversed = !bNeedsReversed;
	}

	if (!bNeedsReversed)
	{
		m_pSprite->set_base_angle( RadiansToDegrees(m_body.GetOrientation()));
	} else
	{
		m_pSprite->set_base_angle( -RadiansToDegrees(m_body.GetOrientation()));
	}

	m_pSprite->set_scale(GetCamera->GetScale().x * m_pTile->GetScale().x, GetCamera->GetScale().y
		* m_pTile->GetScale().y);
	
	
	//construct the final color with tinting mods, insuring it is in range
	short r,g,b,a;

	r = m_pTile->GetColor().get_red() + m_colorModRed;
	g = m_pTile->GetColor().get_green() + m_colorModGreen;
	b = m_pTile->GetColor().get_blue() + m_colorModBlue;
	a = m_pTile->GetColor().get_alpha() + m_colorModAlpha;

	//force them to be within range
	r = min(r, 255); r = max(0, r);
	g = min(g, 255); g = max(0, g);
	b = min(b, 255); b = max(0, b);
	a = min(a, 255); a = max(0, a);

	m_pSprite->set_color(CL_Color(r,g,b,a));
	
	
	static bool bUseParallax;
	static Layer *pLayer;
	static World *pWorld;
	pWorld = m_pTile->GetParentScreen()->GetParentWorldChunk()->GetParentWorld();
	static EntWorldCache *pWorldCache;
    static CL_Vector2 vecPos;

	pWorldCache = pWorld->GetMyWorldCache();

	if (GetGameLogic->GetParallaxActive())
	{
		bUseParallax = true;
		pLayer = &pWorld->GetLayerManager().GetLayerInfo(m_pTile->GetLayer());
		
		if (GetGameLogic->GetMakingThumbnail())
		{
			if (!pLayer->GetUseParallaxInThumbnail())
			{
				bUseParallax = false;
			}
		}
	} else
	{
		bUseParallax = false;
	}

	if (bUseParallax)
	{
		vecPos = pWorldCache->WorldToScreen(pWorldCache->WorldToModifiedWorld(GetPos(), 
			pLayer->GetScrollMod()));
	} else
	{
 		vecPos = pWorldCache->WorldToScreen(GetPos());
	}

	//vecPos.x = RoundNearest(vecPos.x, 1.0f);
	//vecPos.y = RoundNearest(vecPos.y, 1.0f);

	m_pSprite->draw_subpixel( vecPos.x, vecPos.y, pGC);

	m_pSprite->set_angle_yaw(yawHold);
	m_pSprite->set_angle_pitch(pitchHold);
	/*
	//***** debug, draw rect
	CL_Rectf worldRect =GetWorldRect();
	CL_Vector2 vecStart(worldRect.left, worldRect.top);
	CL_Vector2 vecStop(worldRect.right, worldRect.bottom);
	DrawRectFromWorldCoordinates(vecStart, vecStop, CL_Color(255,255,255,180), pGC);
	//*********
	*/

	if (GetGameLogic->GetShowEntityCollisionData())
	{
		//show extra debug stuff

		/*
		RenderVertexListRotated(GetPos(), (CL_Vector2*)GetBody()->GetVertArray(), GetBody()->GetVertCount(), CL_Color(255,255,255,200), pGC,
			RadiansToDegrees(GetBody()->GetOrientation()));
*/

		if (GetLastScanArea().get_width() != 0)
		{
			DrawRectFromWorldCoordinates(CL_Vector2(GetLastScanArea().left,  GetLastScanArea().top), 
				CL_Vector2(GetLastScanArea().right, GetLastScanArea().bottom),	CL_Color(0,255,0,255),  pGC);
		}

		CL_Color col(255,255,0);
		RenderTileListCollisionData(GetNearbyTileList(), pGC, false, &col);
	}

}

void MovingEntity::Update(float step)
{

if (GetGameLogic->GetGamePaused()) return;

	ClearColorMods();

	m_brainManager.Update(step);
	
	if (!m_bAnimPaused)
	m_pSprite->update( float(GetApp()->GetGameLogicSpeed()) / 1000.0f );

	//apply gravity and things
	
	if (!GetBody()->IsUnmovable() && GetCollisionData())
	{
		//the draw ID let's other objects know that we've already processed
		m_drawID = GetWorldCache->GetUniqueDrawID();
		ApplyGenericMovement(step);
	}

}

void MovingEntity::PostUpdate(float step)
{

	if (GetGameLogic->GetGamePaused()) return;

	m_brainManager.PostUpdate(step);

	if (!GetBody()->IsUnmovable() &&  GetCollisionData())
	{
	if (m_body.GetLinVelocity().y < C_MAX_FALLING_DOWN_SPEED)
	{
		m_body.AddForce(Vector(0, m_pTile->GetParentScreen()->GetParentWorldChunk()->GetParentWorld()->GetGravity()* m_body.GetMass()));
	}

	const static float groundDampening = 1.6f;
	const static float angleDampening = 0.01f;
	
	//FIXME if (!m_pBrain && IsOnGround() && GetBody()->GetLinVelocity().Length() < 0.15f
	if (IsOnGround() && GetBody()->GetLinVelocity().Length() < 0.15f
		&& GetBody()->GetAngVelocity() < 0.01f)
	{
		//slow down
		set_float_with_target(&m_body.GetLinVelocity().x, 0, groundDampening);
		set_float_with_target(&m_body.GetLinVelocity().y, 0, groundDampening);
		set_float_with_target(&m_body.GetAngVelocity(), 0, angleDampening);
	}

		GetBody()->Update(step);

		//get ready for next frame
		m_bTouchedAGroundThisFrame = false;
	}

}

void MovingEntity::SetImageFromTilePic(TilePic *pTilePic)
{

	m_dataManager.Set(C_ENT_TILE_PIC_ID_KEY, CL_String::from_int(pTilePic->m_resourceID));
	m_dataManager.Set(C_ENT_TILE_RECT_KEY, RectToString(pTilePic->m_rectSrc));

/*
	m_pCollisionData = pTilePic->GetCollisionData();

	if (m_pCollisionData->GetLineList()->size() == 0)
	{
		//doesn't have any collision data, let's disable phyics.  They can always update the density to
		//something higher later.
		m_fDensity = 0;
	} 
*/
	m_dataManager.Set(C_ENT_DENSITY_KEY, CL_String::from_float(m_fDensity));
}

//************ global utils

MovingEntity * CreateEntity(CL_Vector2 vecPos, string scriptFileName)
{
	MovingEntity *pEnt = new MovingEntity;
	TileEntity *pTile = new TileEntity;
	pTile->SetEntity(pEnt);
	pEnt->SetMainScriptFileName(scriptFileName);
	pEnt->SetPos(vecPos);
	pEnt->Init();
	GetWorld->AddTile(pTile);

	return pEnt;
}
