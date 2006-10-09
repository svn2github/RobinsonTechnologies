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
#include "AI/Goal_Think.h"
#include "AI/PathPlanner.h"
#include "AI/WatchManager.h"


#define C_GROUND_RELAX_TIME_MS 150
#define C_DEFAULT_SCRIPT_PATH "script/"
#define C_DEFAULT_SCRIPT "system/ent_default.lua"
#define C_DISTANCE_NOT_ON_SAME_MAP 2000000000
//there are issues with sliding along walls thinking it's a ground when they are built out of small blocks, this
//provides a work around
#define C_TICKS_ON_GROUND_NEEDED_TO_TRIGGER_GROUND 1


#define C_DEFAULT_DENSITY 0.4f

MovingEntity::MovingEntity(): BaseGameEntity(BaseGameEntity::GetNextValidID())
{
	m_pPathPlanner = NULL;
	m_pGoalManager = NULL;
	m_mainScript = C_DEFAULT_SCRIPT;
	m_iType = C_ENTITY_TYPE_MOVING;
	m_defaultTextColor = CL_Color(255,100,200,255);
	m_bUsingCustomCollisionData = false;
	m_brainManager.SetParent(this);
	m_hashedName = 0;
    m_drawID = 0;


	SetDefaults();
}

MovingEntity::~MovingEntity()
{
	Kill();
	
}

unsigned int MovingEntity::CalculateTimeToReachPosition(const CL_Vector2 &pos)
{
	return (Vec2DDistance(GetPos(), pos) *10) / m_desiredSpeed;
}


PathPlanner * MovingEntity::GetPathPlanner()
{
	if (!m_pPathPlanner)
	{
		m_pPathPlanner = new PathPlanner(this);
	}
	
	return m_pPathPlanner;
}


bool MovingEntity::IsFacingTarget(float tolerance)
{
	float angle = GetAngleBetweenVectorFacings(GetVectorFacing(), GetVectorFacingTarget());
	return fabs(angle) <= tolerance;
}

float MovingEntity::GetDistanceFromEntityByID(int id)
{
	MovingEntity *pEnt = (MovingEntity*)EntityMgr->GetEntityFromID(id);

	if (!pEnt)
	{
		LogMsg("GetDistanceFrom sent invalid ID: %d", id);
		return -1;
	}

	if (GetMap() != pEnt->GetMap()) return C_DISTANCE_NOT_ON_SAME_MAP;

	return (GetPos()-pEnt->GetPos()).length();
}

float MovingEntity::GetDistanceFromPosition(const CL_Vector2 &pos)
{
	return (GetPos()-pos).length();
}

CL_Vector2 MovingEntity::GetVectorToEntityByID(int entID)
{
	MovingEntity *pEnt = (MovingEntity*) EntityMgr->GetEntityFromID(entID);
	if (pEnt) return GetVectorToEntity(pEnt);
	
	LogMsg("Ent %d (%s) can't get angle to entity %d, doesn't exist", ID(), GetName().c_str(),
		entID);
	return CL_Vector2(0,0);
}

CL_Vector2 MovingEntity::GetVectorToEntity(MovingEntity *pEnt)
{
	CL_Vector2 v = pEnt->GetPos()-GetPos();
	v.unitize();
	return v;
}

CL_Vector2 MovingEntity::GetVectorToPosition(const CL_Vector2 &pos)
{
	CL_Vector2 v = pos-GetPos();
	v.unitize();
	return v;
}

bool MovingEntity::IsOnSameMapAsEntityByID(int entID)
{
	MovingEntity *pEnt = (MovingEntity*) EntityMgr->GetEntityFromID(entID);
	if (!pEnt) return false;
	return (GetMap() == pEnt->GetMap());
}

bool MovingEntity::IsCloseToEntity(MovingEntity *pEnt, int dist)
{
	if (GetMap() != pEnt->GetMap()) return false;
	return ( dist >   (GetPos()-pEnt->GetPos()).length()  );
}

bool MovingEntity::IsCloseToEntityByID(int entID, int dist)
{
	MovingEntity *pEnt = (MovingEntity*) EntityMgr->GetEntityFromID(entID);
	if (!pEnt) return false;

	return IsCloseToEntity(pEnt, dist);
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

void MovingEntity::SetListenCollision(int eListen)
{
	m_listenCollision = eListen;
}

World * MovingEntity::GetMap()
{
	return m_pTile->GetParentScreen()->GetParentWorldChunk()->GetParentWorld();
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

	if (m_pScriptObject && m_pScriptObject->FunctionExists("OnKill"))
	{
		m_pScriptObject->RunFunction("OnKill");
	}

	/*
	if (IsPlaced())
	{
		if (!GetName().empty())	
		{
			GetTagManager->Remove(this);

		}
	}
	*/

	SAFE_DELETE(m_pPathPlanner);
	SAFE_DELETE(m_pGoalManager);
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

bool MovingEntity::HandleMessage(const Message &msg)
{

	if (m_pGoalManager)
	{
		return m_pGoalManager->HandleMessage(msg);
	}
	return false; //didn't handle
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
	SetMaxWalkSpeed(2.4f);
	SetDesiredSpeed(GetMaxWalkSpeed());
	m_nearbyTileList.clear();
	m_floorMaterialID = -1;
	m_bUsingSimpleSprite = false;
	m_listenCollision = LISTEN_COLLISION_NONE;
	m_listenCollisionStatic = LISTEN_COLLISION_STATIC_NONE;
	m_fDensity = C_DEFAULT_DENSITY; //default
	m_bIsAtRest = false;
	m_groundTimer = 0;
	m_bMovedFlag = false;
	m_pScriptObject = NULL;
	m_timeLastActive = INT_MAX;
	m_pSprite = NULL;
	m_pVisualProfile = NULL; //note, we don't init or delete this ourself, visual manager handles it
	m_pSpriteLastUsed = NULL;
	m_pCollisionData = NULL;
	m_ticksOnGround = 0;
	m_bOnLadder = false;
	m_navNodeType = C_NODE_TYPE_NORMAL;
	m_bAnimPaused = false;
	SetCollisionMode(COLLISION_MODE_ALL);
	
	m_visualState = VisualProfile::VISUAL_STATE_IDLE;
	m_animID = 0;
	SetFacing(VisualProfile::FACING_LEFT);
	m_trigger.Reset();
	m_bHasRunOnInit = false;
	m_bHasRunPostInit = false;
	m_bRequestsVisibilityNotifications = false;
	m_lastVisibilityNotificationID = 0;
	m_bOnScreen = false;

}

void MovingEntity::SetNavNodeType(int n)
{
	if (IsInitted())
	{
		LogError("Cannot use SetNavNodeType except in the OnInit() function");
		return;
	}

	m_navNodeType = n;
}

void MovingEntity::SetHasPathNode(bool bHasNode)
{
	if (IsInitted())
	{
		LogError("Cannot use SetHasPathNode except in the OnInit() function");
		return;
	}

	m_pTile->SetBit(Tile::e_pathNode, bHasNode);
}


Goal_Think * MovingEntity::GetGoalManager()
{
	//create it if it doesn't exist
	if (m_pGoalManager) return m_pGoalManager;
	return (m_pGoalManager = new Goal_Think(this, "Base"));
}

void MovingEntity::SetFacingTarget(int facing)
{
	SetVectorFacingTarget(FacingToVector(facing));
}

void MovingEntity::SetVisualState(int visualState)
{
	if (m_visualState != visualState) 
	{
		m_bRestartAnim = true;
	}
	m_visualState = visualState;
}

void MovingEntity::SetFacing(int facing)
{
	m_facing = facing;
	m_vecFacing = m_vecFacingTarget = FacingToVector(m_facing);
}

void MovingEntity::SetVectorFacing(const CL_Vector2 &v)
{
	m_vecFacingTarget = m_vecFacing = v;
	m_facing = VectorToFacing(v);
}

void MovingEntity::SetVectorFacingTarget(const CL_Vector2 &v)
{
	m_vecFacingTarget = v;
}

CL_Vector2 MovingEntity::GetVectorFacing()
{
	return m_vecFacing;
}

CL_Vector2 MovingEntity::GetVectorFacingTarget()
{
	return m_vecFacingTarget;
}

void MovingEntity::SetName(const std::string &name)
{
	SetNameEx(name, true);

}
void MovingEntity::SetNameEx(const std::string &name, bool bRemoveOldTag)
{
	//setting a name has a special meaning when done in a moving entity.  It
	//means this entity can be tracked by its name from anywhere in the game, it's
	//stored in a special database that is loaded even when the entities world
	//isn't.  So don't set this unless an entity specifically needs a trackable
	//identity
	
	if (IsPlaced())
	{
		if (bRemoveOldTag)
		GetTagManager->Remove(this);
		BaseGameEntity::SetName(name);

		if (name.empty())
		{
			m_hashedName = 0;
		} else
		{
			m_hashedName = HashString(name.c_str());
			GetTagManager->Update(GetMap(), this);
		}
	} else
	{
		BaseGameEntity::SetName(name);
		m_hashedName = HashString(name.c_str());
	}
	
}

void MovingEntity::Serialize(CL_FileHelper &helper)
{
	//load/save needed data
	cl_uint8 ver = m_pTile->GetParentScreen()->GetVersion();

    helper.process(m_mainScript);

	string temp;

	if (helper.IsWriting())
	{

#ifdef _DEBUG
		if (!GetName().empty())
		{
			//named entity.  Let's make sure the tagcache data is right
			TagObject * pTag = g_TagManager.GetFromHash(m_hashedName);
			if (!pTag)
			{
				LogError("%s's Tagcache data missing", GetName().c_str());
			} else
			{

				if (pTag->GetPos() != GetPos())
				{
					LogError("%s's doesn't match his tagcache's position", GetName().c_str());
				}
				if (pTag->m_pWorld != GetMap())
				{
					LogError("%s's doesn't match his tagcache's world", GetName().c_str());
				}
			}
		}
#endif
		
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
	
	//OPTIMIZE:  This is called many times during a frame PER entity, we can probably get a big speed increase
	//by caching this info out with a changed flag or something
	
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
	r += *(const CL_Pointf*)(&GetVisualOffset());

	return r;
}

const CL_Rect & MovingEntity::GetBoundsRect()
{
	
	//OPTIMIZE:  This is called many times during a frame PER entity, we can probably get a big speed increase
	//by caching this info out with a changed flag or something

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
	
	float offsetY = 0;
	

	if (GetGameLogic->GetGameMode() == GameLogic::C_GAME_MODE_SIDE_VIEW)
	{
		//we have to deduct half the height because this is written to move our FEET to the point
		//to warp to. (only applicable to sideview mode)

		offsetY = m_pTile->GetWorldColRect().get_height()/2;
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

		if (GetCamera->GetEntTracking() == ID())
		{
			GetCamera->SetInstantUpdateOnNextFrame(true);
		}
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

Zone * MovingEntity::GetNearbyZoneByCollisionRectAndType(int matType)
{

	if ( m_zoneVec.size() == 0) return NULL;

	static CL_Rectf r;
	r = m_pTile->GetWorldColRect();

	for (unsigned int i=0; i < m_zoneVec.size(); i++)
	{
		if (g_materialManager.GetMaterial(m_zoneVec[i].m_materialID)->GetType() == matType)
		{
			if (r.is_overlapped(m_zoneVec[i].m_boundingRect + *(CL_Pointf*)&(m_zoneVec[i].m_vPos)) )
			{
				return &m_zoneVec[i]; //valid for the rest of this frame
			}
			
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
			
			string sFile = GetGameLogic->GetScriptRootDir()+"/system/system.xml";
			g_VFManager.LocateFile(sFile);
			SetVisualProfile(sFile,"ent_default");
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

	m_bHasRunOnInit = true;
	return true;
}

void MovingEntity::RunPostInitIfNeeded()
{
	if (!m_bHasRunPostInit)
	{

		if (m_pScriptObject)
		{
			if (m_pScriptObject->FunctionExists("OnPostInit"))
				m_pScriptObject->RunFunction("OnPostInit");
		}

		m_bHasRunPostInit = true;
	}
}

bool MovingEntity::LoadScript(const char *pFileName)
{
	SAFE_DELETE(m_pScriptObject);
	m_pScriptObject = new ScriptObject();
	string s = C_DEFAULT_SCRIPT_PATH;
	s += pFileName;
	
	g_VFManager.LocateFile(s);

	if (!m_pScriptObject->Load(s.c_str()))
	{
		return false;
	}

	//set this script objects "this" variable to point to an instance of this class
	luabind::globals(m_pScriptObject->GetState())["this"] = this;
	m_pScriptObject->RunFunction("OnInit");
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

				SAFE_DELETE(m_pPathPlanner); //this would be invalid now

				if (GetCamera->GetEntTracking() == ID())
				{
					 //the camera should follow the entity
					CameraSetting cs = GetCamera->GetCameraSettings();
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
		m_animID = GetVisualProfile()->TextToAnimID(name);
		SetSpriteData(GetVisualProfile()->GetSpriteByAnimID(m_animID));
	}
}

void MovingEntity::OnDamage(const CL_Vector2 &normal, float depth, MovingEntity * enemy, int damage, int uservar, MovingEntity * pProjectile)
{
	SetOnLadder(false);

	if (!GetScriptObject() || !GetScriptObject()->FunctionExists("OnDamage")) return;

	try {luabind::call_function<bool>(m_pScriptObject->GetState(), "OnDamage", normal, depth, enemy, damage, uservar, pProjectile);
	} LUABIND_ENT_CATCH("Error while calling OnDamage(Vector2 normal, float depth, enemy, int damage, int uservar, projectile)");
}

void MovingEntity::SetSpriteByVisualStateAndFacing()
{
	
	if (GetVisualProfile())
	{
		m_animID = GetVisualProfile()->GetAnimID(GetVisualState(), GetFacing());
		SetSpriteData(GetVisualProfile()->GetSpriteByAnimID(m_animID));
	}
}

void MovingEntity::LastCollisionWasInvalidated()
{
	m_bTouchedAGroundThisFrame = m_bOldTouchedAGroundThisFrame;
	m_floorMaterialID = m_oldFloorMaterialID;
}

void MovingEntity::OnCollision(const Vector & N, float &t, CBody *pOtherBody, bool *pBoolAllowCollision)
{

	if (m_collisionMode == COLLISION_MODE_NONE)
	{
		pBoolAllowCollision = false;
		return;

	}
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
		x = GetSizeX();
		y = GetSizeY();
	}
	
	x *= 0.5f;
	y *= 0.5f;
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
	//link to data correctly
	pActiveLine->GetAsBody(CL_Vector2(0,0), &m_body);

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
			
			//static objects will send Null for the custombody, which is ok
			pWallBody = &lineListItor->GetAsBody(pTile->GetPos(), pTile->GetCustomBody());
			
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
			
					//actually add the data
					static Zone z;
					z.m_materialID = lineListItor->GetType();
					z.m_boundingRect = lineListItor->GetRect();
					z.m_vPos = pTile->GetPos();
					if (pTile->GetType() == C_TILE_TYPE_ENTITY)
					{
						//i send the ID because it's safer than the pointer
						z.m_entityID = ((TileEntity*)pTile)->GetEntity()->ID();
						//z.m_vPos += lineListItor->GetOffset();
						z.m_boundingRect -=  *(CL_Pointf*)& (pCol->GetLineList()->begin()->GetOffset());

					} else
					{
						z.m_entityID = 0;
					}
					m_zoneVec.push_back(z);
				
			}
		}
		
		lineListItor++;
	}
}

void MovingEntity::SetCollisionMode(int mode)
{

	if (mode < 0 || mode > COLLISION_MODE_COUNT)
	{
		LogError("SetCollisionMode: Invalid mode sent: %d", mode);
		return;
	}

	m_collisionMode = mode;
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
		int frameTemp = 0;
		if (m_bRestartAnim)
		{
			m_bRestartAnim = false;
		} else
		{
			if (m_pSprite && m_pSprite->get_frame_count() > 0) frameTemp = m_pSprite->get_current_frame();
		}

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
		m_pSprite->set_frame(frameTemp);
		
	}
}

void MovingEntity::ApplyGenericMovement(float step)
{
	m_nearbyTileList.clear();
	m_zoneVec.clear();

	//schedule tile update if needed
	m_bMovedFlag = true;

	if (m_collisionMode == COLLISION_MODE_NONE) return;

	if (!m_pCollisionData->GetLineList()->size())
	{
		LogError("No collision data in entity %d. (%s)", ID(), GetName().c_str());
		return;
	}

	if (!m_pSprite || m_pSprite->get_frame_count() == 0)
	{
		throw CL_Error("Error, sprite not valid in entity " + CL_String::from_int(ID()) + " (trying to walk in a direction that doesn't exist maybe?");
		return;
	}
	
	//scan around us for entities

	m_scanArea = m_pTile->GetWorldColRect();
	float padding;
	padding = (max(m_scanArea.get_width(), m_scanArea.get_height()));
	padding = (float(padding)*0.2f)+8;

	m_scanArea.left -= padding;
	m_scanArea.top -= padding;
	m_scanArea.right += padding;
	m_scanArea.bottom += padding;

	World *pWorld = m_pTile->GetParentScreen()->GetParentWorldChunk()->GetParentWorld();

	pWorld->GetMyWorldCache()->AddTilesByRect(CL_Rect(m_scanArea), &m_nearbyTileList, pWorld->GetLayerManager().GetCollisionList());
	
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
	
}


CL_Vector2 MovingEntity::GetVisualOffset()
{
	if (GetCollisionData()) return -GetCollisionData()->GetCombinedOffsets();
	return CL_Vector2(0,0);
}

void MovingEntity::RenderShadow(void *pTarget)
{

	static CL_Vector2 vecPos;
	static World *pWorld;
	static EntWorldCache *pWorldCache;

	CL_GraphicContext *pGC = (CL_GraphicContext *)pTarget;
	CL_Vector2 vVisualPos = GetPos() + GetVisualOffset();

	pWorld = m_pTile->GetParentScreen()->GetParentWorldChunk()->GetParentWorld();
	pWorldCache = pWorld->GetMyWorldCache();

	vecPos = pWorldCache->WorldToScreen(vVisualPos);

	m_pSprite->set_scale(GetCamera->GetScale().x * m_pTile->GetScale().x, GetCamera->GetScale().y * m_pTile->GetScale().y);

	short a = min(50, 255 + m_colorModAlpha);
	
	m_pSprite->set_color(CL_Color(0,0,0,a));

	static CL_Surface_DrawParams1 params1;
	m_pSprite->setup_draw_params(vecPos.x, vecPos.y, params1, true);
	AddShadowToParam1(params1, m_pTile);
	m_pSprite->draw(params1, pGC);

}

void MovingEntity::Render(void *pTarget)
{
	
	CL_GraphicContext *pGC = (CL_GraphicContext *)pTarget;
	static float yawHold, pitchHold;

	CL_Vector2 vVisualPos = GetPos() + GetVisualOffset();

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
	
	//construct the final color with tinting mods, insuring it is in range
	short r,g,b,a;

	r = m_pTile->GetColor().get_red() + m_colorModRed;
	g = m_pTile->GetColor().get_green() + m_colorModGreen;
	b = m_pTile->GetColor().get_blue() + m_colorModBlue;
	a = m_pTile->GetColor().get_alpha() + m_colorModAlpha;

	//force them to be within range
	r = cl_min(r, 255); r = cl_max(0, r);
	g = cl_min(g, 255); g = cl_max(0, g);
	b = cl_min(b, 255); b = cl_max(0, b);
	a = cl_min(a, 255); a = cl_max(0, a);
		
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
		vecPos = pWorldCache->WorldToScreen(pWorldCache->WorldToModifiedWorld(vVisualPos, 
			pLayer->GetScrollMod()));
	} else
	{
 		vecPos = pWorldCache->WorldToScreen(vVisualPos);
	}

	clTexParameteri(CL_TEXTURE_2D, CL_TEXTURE_MAG_FILTER, CL_NEAREST);
	clTexParameteri(CL_TEXTURE_2D, CL_TEXTURE_MIN_FILTER, CL_NEAREST);

	m_pSprite->set_scale(GetCamera->GetScale().x * m_pTile->GetScale().x, GetCamera->GetScale().y
		* m_pTile->GetScale().y);
	//do the real blit
	m_pSprite->set_color(CL_Color(r,g,b,a));
	m_pSprite->draw_subpixel( vecPos.x, vecPos.y, pGC);

	//return things back to normal
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

}

void MovingEntity::RenderCollisionLists(CL_GraphicContext *pGC)
{
	if (GetLastScanArea().get_width() != 0)
	{
		DrawRectFromWorldCoordinates(CL_Vector2(GetLastScanArea().left,  GetLastScanArea().top), 
			CL_Vector2(GetLastScanArea().right, GetLastScanArea().bottom),	CL_Color(0,255,0,255),  pGC);
	}

	CL_Color col(255,255,0);
	RenderTileListCollisionData(GetNearbyTileList(), pGC, false, &col);
}

void MovingEntity::SetTrigger(int triggerType, int typeVar, int triggerBehavior, int behaviorVar)
{
	m_trigger.Set(this, triggerType, typeVar, triggerBehavior, behaviorVar);
}

void MovingEntity::DumpScriptInfo()
{
	if (m_pScriptObject)
	{
		LogMsg("Script %s loaded in entity %d (%s).", m_pScriptObject, ID(), GetName().c_str());
		DumpTable(m_pScriptObject->GetState());
	} else
	{
		LogMsg("No script is loaded in ent %d. (%s)", ID(), GetName().c_str());
	}
}

void MovingEntity::RotateTowardsVectorDirection(const CL_Vector2 &vecTargetfloat, float maxTurn)
{
	if (RotateVectorTowardsAnotherVector(m_vecFacing, vecTargetfloat, maxTurn))
	{
		//actual change was made, update
		m_facing = VectorToFacing(m_vecFacing);
	}

}

void MovingEntity::UpdateTriggers(float step)
{
	m_trigger.Update(step);
}

void MovingEntity::SetRunStringASAP(const string &command)
{
	if (m_runScriptASAP.empty())
	{
		m_runScriptASAP = command;
	} else
	{
		LogError("Can't queue up %s, %s is already set to run.", command.c_str(), m_runScriptASAP.c_str());
	}
}

void MovingEntity::ProcessPendingMoveAndDeletionOperations()
{
	if (GetDeleteFlag())
	{
		//remove tile completely
		assert(GetTile());
		GetTile()->GetParentScreen()->RemoveTileByPointer(GetTile());
		return;
	} else
	{
		UpdateTilePosition();
	}
}

void MovingEntity::CheckVisibilityNotifications(unsigned int notificationID)
{

	if (GetGameLogic->GetGamePaused()) return;

	if (notificationID != m_lastVisibilityNotificationID)
	{
		SetIsOnScreen(false);
	} 
}

void MovingEntity::Update(float step)
{

if (GetGameLogic->GetGamePaused()) return;


	m_lastVisibilityNotificationID = g_watchManager.GetVisibilityID();

	ClearColorMods();

	RunPostInitIfNeeded();

	UpdateTriggers(step);

	if (m_pGoalManager)
	{
		m_pGoalManager->Process();
		if (!m_runScriptASAP.empty())
		{
			m_pScriptObject->RunString(m_runScriptASAP.c_str());
			m_runScriptASAP.clear();
		}
	}

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

void MovingEntity::OnWatchListTimeout(bool bIsOnScreen)
{
	if (!bIsOnScreen)
	{
		
		if (GetVisibilityNotifications())
		{
			//hack so the onlosescreen won't run again
			g_watchManager.RemoveFromVisibilityList(this);
		}

		try {luabind::call_function<luabind::object>(GetScriptObject()->GetState(), "OnWatchTimeout", bIsOnScreen);
		} LUABIND_ENT_CATCH( "Error while calling function OnWatchTimeout(bool bIsOnScreen)");

	}
}


void MovingEntity::SetIsOnScreen(bool bNew)
{
	if (bNew != m_bOnScreen)
	{
		m_bOnScreen = bNew;

		if (m_bOnScreen)
		{
			//LogMsg("Ent %d (%s) has just entered the screen", ID(), GetName().c_str());
			RunFunction("OnGetVisible");
		} else
		{
			RunFunction("OnLoseVisible");
			//LogMsg("Ent %d (%s) has left the screen", ID(), GetName().c_str());
		}
	}
}

void MovingEntity::PostUpdate(float step)
{
	if (GetGameLogic->GetGamePaused()) return;

	if (m_bRequestsVisibilityNotifications)
	{

		if (m_lastVisibilityNotificationID != g_watchManager.GetVisibilityID())
		{
			SetIsOnScreen(true);
		}

		g_watchManager.AddEntityToVisibilityList(this);
	}
	
	m_brainManager.PostUpdate(step);

	const static float groundDampening = 1.6f;
	const static float angleDampening = 0.01f;

	World *pWorld = m_pTile->GetParentScreen()->GetParentWorldChunk()->GetParentWorld();

	if (!GetBody()->IsUnmovable() &&  GetCollisionData())
	{
		float gravity = pWorld->GetGravity();
		if (gravity != 0)
		{
			if ( (m_body.GetLinVelocity().y /step) < C_MAX_FALLING_DOWN_SPEED)
			{
				m_body.AddForce(Vector(0, pWorld->GetGravity() * m_body.GetMass()) * step);
			}
			
			if (GetGameLogic->GetGameMode() == GameLogic::C_GAME_MODE_SIDE_VIEW)
			{
	
			if (!m_brainManager.GetBrainBase() && IsOnGround() && GetBody()->GetLinVelocity().Length() < 0.15f
				&& GetBody()->GetAngVelocity() < 0.01f)
			{
				//slow down
				set_float_with_target(&m_body.GetLinVelocity().x, 0, groundDampening * step);
				set_float_with_target(&m_body.GetLinVelocity().y, 0, groundDampening* step);
				set_float_with_target(&m_body.GetAngVelocity(), 0, angleDampening* step);
			}
			}

		} else
		{
		
			if (GetGameLogic->GetGameMode() == GameLogic::C_GAME_MODE_TOP_VIEW && !m_brainManager.GetBrainBase())
			{
				//this is only run if things don't have real brains
			SetIsOnGround(true);

			const static float top_groundDampening = 0.06f;
			const static float top_angleDampening = 0.002f;

			set_float_with_target(&m_body.GetLinVelocity().x, 0, top_groundDampening * step);
			set_float_with_target(&m_body.GetLinVelocity().y, 0, top_groundDampening * step);
			set_float_with_target(&m_body.GetAngVelocity(), 0, top_angleDampening * step);
			}
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
		//doesn't have any collision data, let's disable physics.  They can always update the density to
		//something higher later.
		m_fDensity = 0;
	} 
*/
	m_dataManager.Set(C_ENT_DENSITY_KEY, CL_String::from_float(m_fDensity));
}

CL_Rectf MovingEntity::GetCollisionRect()
{
	//OPTIMIZE for gods sake, cache this
	if (!m_pTile->GetCollisionData()) return CL_Rectf(0,0,0,0);
	return m_pTile->GetCollisionData()->GetCombinedCollisionRect();
}

float MovingEntity::GetBoundingCollisionRadius()
{
	
	//OPTIMIZE for gods sake, cache this
	CL_Rectf c = GetCollisionRect();
	return max(c.get_width(), c.get_height());
}
//returns true if this bot can move directly to the given position
//without bumping into any walls
//warning, only tests against edges
bool MovingEntity::CanWalkTo(CL_Vector2 & to, bool ignoreLivingCreatures)
{
	EntWorldCache *pWC = m_pTile->GetParentScreen()->GetParentWorldChunk()->GetParentWorld()->GetMyWorldCache();	
	return !pWC->IsPathObstructed(GetPos(), to, GetBoundingCollisionRadius(), m_pTile, ignoreLivingCreatures);
}

//similar to above. Returns true if the bot can move between the two
//given positions without bumping into any walls
//warning, only tests against edges
bool MovingEntity::CanWalkBetween(World *pMap, CL_Vector2 from, CL_Vector2 to, bool ignoreLivingCreatures)
{
	return !pMap->GetMyWorldCache()->IsPathObstructed(from, to, GetBoundingCollisionRadius(), m_pTile, ignoreLivingCreatures);
}

bool MovingEntity::IsValidPosition(World *pMap, const CL_Vector2 &pos, bool bIgnoreLivingCreatures)
{
	return !pMap->GetMyWorldCache()->IsAreaObstructed(pos, GetBoundingCollisionRadius(), bIgnoreLivingCreatures, this);
}

