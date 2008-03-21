#include "AppPrecomp.h"
#include "MovingEntity.h"
#include "GameLogic.h"
#include "TileEntity.h"
#include "EntMapCache.h"
#include "EntCollisionEditor.h"
#include "ScriptManager.h"
#include "BrainShake.h"
#include "VisualProfileManager.h"
#include "physics/Contact.h"
#include "MaterialManager.h"
#include "AI/Goal_Think.h"
#include "AI/PathPlanner.h"
#include "AI/WatchManager.h"

#define C_ANIM_FRAME_LAST -1
#define C_GROUND_RELAX_TIME_MS 150
#define C_DEFAULT_SCRIPT "system/ent_default.lua"
#define C_DISTANCE_NOT_ON_SAME_MAP 2000000000
//there are issues with sliding along walls thinking it's a ground when they are built out of small blocks, this
//provides a work around
#define C_TICKS_ON_GROUND_NEEDED_TO_TRIGGER_GROUND 1

#define C_DEFAULT_ENTITY_ACCELERATION 0.37f

#define C_DEFAULT_TURN_SPEED 0.1f

#define C_DEFAULT_DENSITY 0.4f

#define C_GRAVITY_OVERRIDE_DISABLED 9999999

Zone g_ZoneEmpty;

class ZoneEmptyInit
{
public:
	ZoneEmptyInit()
	{
		g_ZoneEmpty.m_entityID = C_ENTITY_NONE;
		g_ZoneEmpty.m_materialID = CMaterial::C_MATERIAL_TYPE_NONE;
		g_ZoneEmpty.m_vPos = CL_Vector2::ZERO;

	}
};

ZoneEmptyInit zoneEmpty;

MovingEntity::MovingEntity(): BaseGameEntity(BaseGameEntity::GetNextValidID())
{
	m_pPathPlanner = NULL;
	m_pGoalManager = NULL;
	m_mainScript = C_DEFAULT_SCRIPT;
	m_iType = C_ENTITY_TYPE_MOVING;
	m_pFont = NULL;
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
	
	return (Vec2DDistance(GetPos(), pos) *15) / m_desiredSpeed;
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

bool MovingEntity::FunctionExists(const char * pFunctionName)
{
	if (!m_pScriptObject) return false;
	return m_pScriptObject->FunctionExists(pFunctionName);
}

bool MovingEntity::VariableExists(const char * pVarName)
{
	if (!m_pScriptObject) return false;
	return m_pScriptObject->VariableExists(pVarName);
}



CL_Vector2 MovingEntity::GetVectorToEntity(MovingEntity *pEnt)
{
	CL_Vector2 v = pEnt->GetPos()-GetPos();
	if (v == CL_Vector2::ZERO)
	{
		//well, returning 0,0 can cause havok in the path planner, so
		//let's guess
		return m_vecFacing;
	}
	v.unitize();

	//check for NAN, it happens under gcc
	if (v.x != v.x || v.y != v.y)
		{
			//well, returning 0,0 can cause havok in the path planner, so
			//let's guess
			return m_vecFacing;
		}
			
	return v;
}

CL_Vector2 MovingEntity::GetVectorToPosition(const CL_Vector2 &pos)
{
	CL_Vector2 v = pos-GetPos();
	//avoid NAN on osx/gcc
	if (v.x != v.x || v.y != v.y)
	{
		//well, returning 0,0 can cause havok in the path planner, so
		//let's guess
		return m_vecFacing;
	}
	
	v.unitize();
	//check for NAN, it happens under gcc
	if (v.x != v.x || v.y != v.y)
	{
		//well, returning 0,0 can cause havok in the path planner, so
		//let's guess
		return m_vecFacing;
	}
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
		//LogError("Error, can't ScaleCollisionInfo, there is no collision data here");
		return;
	}

	if (m_bUsingTilePicCollision && vScale != CL_Vector2(1,1))
	{
		if (!m_bUsingCustomCollisionData)
		{
			//switch to custom collision from a copy of the shared tilepic stuff
			CollisionData *pNewCol = new CollisionData(*m_pCollisionData);
			m_pCollisionData = pNewCol;
			m_bUsingCustomCollisionData = true;
			m_pCollisionData->SetDataChanged(false);
		}
	}

	if (m_bUsingCustomCollisionData)
	{
		m_pCollisionData->SetScale(vScale);
	}
}

void MovingEntity::SetListenCollision(int eListen)
{
	m_listenCollision = eListen;
}

int MovingEntity::GetLayerID()
{
	 assert(m_pTile);
	 
	 if (m_requestNewLayerID != C_LAYER_NONE) return m_requestNewLayerID;
	 return m_pTile->GetLayer();

}

void MovingEntity::SetLayerByName(const string &name)
{
	if (!IsPlaced())
	{
		LogError("Entity %d (%s) cannot use SetLayerByName yet, he hasn't been placed on the map.  Use this in OnPostInit instead.",
			ID(), GetName().c_str());
		return;
	}
	SetLayerID(GetMap()->GetLayerManager().GetLayerIDByName(name));
}


void MovingEntity::SetLayerID(int id)
 {
	 assert(m_pTile);
	
	if (!m_pTile->GetParentScreen())
	{
		LogError("You can't use SetLayerID() in OnInit, it hasn't been placed on a map  yet.  Use OnPostInit.");
		return;
	}
	if (id != m_pTile->GetLayer())
	{
		m_requestNewLayerID = id;
		m_bMovedFlag = true;

	}

}

Map * MovingEntity::GetMap()
{
	return m_pTile->GetParentScreen()->GetParentMapChunk()->GetParentMap();
}

const string & MovingEntity::GetRequestedMapName()
{
	return m_strWorldToMoveTo;
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

	//tell any entities that were attached to us that we're dying and to also kill themselves
	if (!m_attachedEntities.empty())
	{
		list<int>::iterator itor;
		MovingEntity *pEnt;

		for (itor = m_attachedEntities.begin(); itor != m_attachedEntities.end();)
		{
			pEnt = (MovingEntity*) EntityMgr->GetEntityFromID(*itor);
			if (pEnt)
			{
				pEnt->SetDeleteFlag(true);
			} 
			itor++;
		}
	}

	if (m_pScriptObject && IsPlaced() && m_pScriptObject->FunctionExists("OnKill"))
	{
		m_pScriptObject->RunFunction("OnKill");
		g_inputManager.RemoveBindingsByEntity(this);
	}

	if (m_bRequestsVisibilityNotifications)
	{
			g_watchManager.RemoveFromVisibilityList(this);
	}

	SAFE_DELETE(m_pPathPlanner);
	SAFE_DELETE(m_pGoalManager);
	m_brainManager.Kill(); //reset it

	m_pVisualProfile = NULL;
	
	if (m_bUsingCustomCollisionData)
	{
		//there is a special case we need to check, if we're using a shared tilepic yet scaled it our own way..
		if (m_bUsingTilePicCollision)
		{
			if (m_pCollisionData && m_pCollisionData->GetDataChanged())
			{
				m_pCollisionData->SetDataChanged(false);
				//yes, we need to copy this to the real tilepics stuff..
				CollisionData *pColData = GetHashedResourceManager->GetCollisionDataByHashedIDAndRect(GetImageID(), GetImageClipRect());
				if (pColData)
				{
					*pColData = *m_pCollisionData; //copy it over
					pColData->SetScale(CL_Vector2(1,1));
					pColData->SetDataChanged(true);
					
					} else
				{
					assert(!"that was weird");
				}
			}
		}

		
		SAFE_DELETE(m_pCollisionData);
		m_bUsingCustomCollisionData = false;
		m_bUsingTilePicCollision = false;
	}
	
	SAFE_DELETE(m_pSprite);
	SAFE_DELETE(m_pScriptObject);
	SAFE_DELETE(m_pFont);

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
	m_bRunUpdateEveryFrame = false;
	m_accel = C_DEFAULT_ENTITY_ACCELERATION;
	m_bUsingTilePicCollision = false;
	m_bLockedScale = false;
	m_attachEntID = C_ENTITY_NONE;
	m_text.clear();
	m_turnSpeed = C_DEFAULT_TURN_SPEED;
	m_textRect = CL_Rect(0,0, 1024, 1024);
	ClearColorMods();
	m_requestNewLayerID = C_LAYER_NONE;
	m_bCanRotate = false;
	SetMaxWalkSpeed(6);
	SetDesiredSpeed(2.4f);
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
	m_bHasRunOnMapInsert = false;
	m_bRequestsVisibilityNotifications = false;
	m_lastVisibilityNotificationID = INT_MAX; //it starts at 0, so max_int is better to use than 0 as the default
	m_bOnScreen = false;
	m_customDampening = -1;
	m_gravityOverride = C_GRAVITY_OVERRIDE_DISABLED;
	m_blendMode = C_BLEND_MODE_NORMAL;
	SetIsCreature(false);
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

void MovingEntity::SetText(const string &text)
{
	m_text = text;
	StringReplace("\\n", "\n", m_text);

	if (m_text.empty())
	{
		SAFE_DELETE(m_pFont);
	} else
	{
		if (!m_pFont)
		{
			if (GetGameLogic()->GetGUIStyle())
			{
				m_pFont = new CL_Font("font_label", GetGameLogic()->GetGUIStyle()->get_resources());
			} else
			{
				m_pFont = new CL_Font(*GetApp()->GetFont(C_FONT_NORMAL));
			}
		}

		m_pFont->set_alignment(origin_center,0,0);
	}
}

CL_Vector2 MovingEntity::GetTextBounds()
{
	if (m_pFont)
	{
		float x,y;
		m_pFont->get_scale(x,y);
		//try to work around a clanlib bounds bug when dealing with small sizes
		if (x < 1)
		{
			m_pFont->set_scale(x* (1+( (1-x)*0.3   )), y);
		}

		CL_Size size = m_pFont->get_size(m_text);

		//put it back
		m_pFont->set_scale(x,y);

		return CL_Vector2(size.width, size.height);
	} else
	{
		LogError("Can't use GetTextBounds(), entity %d (%s) doesn't have any font assigned (Use SetText first)", ID(), GetName().c_str());
	}

	return CL_Vector2(0,0);
}

void MovingEntity::SetTextRect(const CL_Rect &r)
{
	if (!m_pFont)
	{
		LogError("Can't use SetTextRect, entity %d (%s) doesn't have any font assigned (Use SetText first)", ID(), GetName().c_str());
		return;
	}

	m_textRect = r;
	
}

void MovingEntity::SetTextAlignment(int alignment, CL_Vector2 v)
{
	if (!m_pFont)
	{
		LogError("Can't use SetTextAlignment, entity %d (%s) doesn't have any font assigned (Use SetText first)", ID(), GetName().c_str());
		return;
	}

	m_pFont->set_alignment(CL_Origin(alignment), v.x, v.y);
}

void MovingEntity::SetTextColor(CL_Color col)
{
	if (!m_pFont)
	{
		LogError("Can't use SetTextColor, entity %d (%s) doesn't have any font assigned (Use SetText first)", ID(), GetName().c_str());
		return;
	}

	m_pFont->set_color(col);
}

CL_Color MovingEntity::GetTextColor()
{
	if (!m_pFont)
	{
		LogError("Can't use GetTextColor, entity %d (%s) doesn't have any font assigned (Use SetText first)", ID(), GetName().c_str());
		return CL_Color(0,0,0);
	}

	return m_pFont->get_color();
}

void MovingEntity::SetTextScale(const CL_Vector2 &vecScale)
{
	if (!m_pFont)
	{
		LogError("Can't use SetTextScale, entity %d (%s) doesn't have any font assigned (Use SetText first)", ID(), GetName().c_str());
		return;
	}

	m_pFont->set_scale(vecScale.x, vecScale.y);
}

CL_Vector2 MovingEntity::GetTextScale()
{
	if (!m_pFont)
	{
		LogError("Can't use GetTextScale, entity %d (%s) doesn't have any font assigned (Use SetText first)", ID(), GetName().c_str());
		return CL_Vector2(0,0);
	}

	float x,y;
	m_pFont->get_scale(x,y);
	return CL_Vector2(x,y);
}



MovingEntity * MovingEntity::CreateEntity(Map *pMap, CL_Vector2 pos, const string &script)
{
	if (!pMap)
	{
		//use our own active map, none was specific
		pMap = GetMap();
	}

	return ::CreateEntity(pMap, pos, ProcessPathNoScript(script));
}


void MovingEntity::SetFacingTarget(int facing)
{
	SetVectorFacingTarget(FacingToVector(facing));
}

int MovingEntity::GetFacingTarget()
{
	return VectorToFacing(m_vecFacingTarget);
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
		g_TagManager.Remove(this);
		BaseGameEntity::SetName(name);

		if (name.empty())
		{
			m_hashedName = 0;
		} else
		{
			m_hashedName = HashString(name.c_str());
			g_TagManager.Update(GetMap(), this);
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

	float orientation = m_body.GetOrientation();

	if (helper.IsWriting() || ver > 0)
	{
		helper.process(m_vecFacing);
		helper.process(m_facing);
		helper.process(orientation);
	}


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
					LogError("%s doesn't match his tagcache's position", GetName().c_str());
				}
				if (pTag->m_pWorld != GetMap())
				{
					LogError("%s doesn't match his tagcache's world", GetName().c_str());
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
	SetVectorFacing(m_vecFacing);

	if (!helper.IsWriting())
	{
		//we need to init this too
		Init();
		m_body.SetOrientation(orientation);
		//SetSpriteByVisualStateAndFacing();
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
	
	pNew->SetVectorFacing(m_vecFacing);
	pNew->Init();

	pNew->GetBody()->SetOrientation(m_body.GetOrientation());

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
	x =  float(x) * m_pTile->GetScale().x;
	y =  float(y) * m_pTile->GetScale().y;

	if (origin == origin_center)
	{
		y = -y;
	}
	
	r -= CL_Pointf(-x,y);


	static CL_Pointf offset;

	offset = calc_origin(origin, r.get_size());

	r -= offset;
	r += *(const CL_Pointf*)&(GetPos());
	
		
	r += *(const CL_Pointf*)&(GetVisualOffset());
	
/*
	CL_Rect c(r);
	if (c.bottom < -1000 || c.bottom > 1000)
	{
		LogMsg("BEFORE:  rect is %s. Pos is %s.  Visual offset is %s",  PrintRect(r).c_str(), VectorToString(&GetPos()).c_str(),
			VectorToString(&GetVisualOffset()).c_str());

		LogMsg("AFTER:  rect is %s.  Size X is %d.  ScaleX is %.2f.  Offset is %.2f, %.2f", PrintRectInt(c).c_str(), GetSizeX(),
			m_pTile->GetScale().x, offset.x, offset.y);
	}
*/
	
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

string MovingEntity::ProcessPathNoScript(const string &st)
{
	if (st[0] == '~')
	{
		return CL_String::get_path(m_mainScript) + (st.c_str()+1);
	}

	return st;
}

int MovingEntity::PlaySoundPositioned(const string &fName)
{
	if (g_pMapManager->GetActiveMap() != GetMap())
	{
		//we're not in the same map
		return C_SOUND_NONE;
	}
	
	if (!g_pSoundManager) return C_SOUND_NONE;

	int hHandle = g_pSoundManager->Play(ProcessPath(fName).c_str());
	return hHandle;
}

int MovingEntity::PlaySound(const string &fName)
{
	if (!g_pSoundManager) return C_SOUND_NONE;

	int hHandle = g_pSoundManager->Play(ProcessPath(fName).c_str());
	return hHandle;
}

bool MovingEntity::SetVisualProfile(string resourceFileName, const string &profileName)
{
	resourceFileName = C_DEFAULT_SCRIPT_PATH + ProcessPathNoScript(resourceFileName);
	VisualResource *pResource = GetVisualProfileManager->GetVisualResource(resourceFileName);
	if (!pResource) return false; //failed that big time

	//now we need to get the actual profile
	m_pVisualProfile = pResource->GetProfile(profileName);
	
	SetIsCreature(true); //a guess here
	return true; //success
}

void MovingEntity::SetPos(const CL_Vector2 &new_pos)
{
	m_body.GetPosition().x = new_pos.x;
	m_body.GetPosition().y = new_pos.y;
	m_bMovedFlag = true;

	if (!m_strWorldToMoveTo.empty())
	{
		LogMsg("Warning: SetPos() command issued to Entity %d (%s) ignored, because a a map-move request was scheduled this frame",
			ID(), GetName().c_str());
	}
}

bool MovingEntity::SetPosAndMapByTagName(const string &name)
{
	TagObject *pO = g_TagManager.GetFromString(name);

	if (!pO)
	{
		LogMsg("SetPosAndMapByName: Unable to locate entity by name: %s", name.c_str());
		return false;
	} 


	SetPosAndMap(pO->m_pos, pO->m_pWorld->GetName());
	return true; 
}

void MovingEntity::SetPosAndMap(const CL_Vector2 &new_pos, const string &worldName)
{

	if (worldName != m_pTile->GetParentScreen()->GetParentMapChunk()->GetParentMap()->GetName())
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


Zone MovingEntity::GetZoneWeAreOnByMaterialType(int matType)
{
	
	for (unsigned int i=0; i < m_zoneVec.size(); i++)
	{
		if (g_materialManager.GetMaterial(m_zoneVec[i].m_materialID)->GetType() == matType)
		{
			return m_zoneVec[i]; //valid for the rest of this frame
		}
	}

	return g_ZoneEmpty;

}

Zone MovingEntity::GetNearbyZoneByCollisionRectAndType(int matType)
{

	if ( m_zoneVec.size() == 0) return g_ZoneEmpty;

	static CL_Rectf r;
	r = m_pTile->GetWorldColRect();

	for (unsigned int i=0; i < m_zoneVec.size(); i++)
	{
		if (g_materialManager.GetMaterial(m_zoneVec[i].m_materialID)->GetType() == matType)
		{
			if (r.is_overlapped(m_zoneVec[i].m_boundingRect + *(CL_Pointf*)&(m_zoneVec[i].m_vPos)) )
			{
				return m_zoneVec[i]; //valid for the rest of this frame
			}
			
		}
	}

	return g_ZoneEmpty;
}

Zone MovingEntity::GetNearbyZoneByPointAndType(const CL_Vector2 &vPos, int matType)
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
				return m_zoneVec[i]; //valid for the rest of this frame
			}
		}
	}

	return g_ZoneEmpty;

}

void MovingEntity::GetAlignment(CL_Origin &origin, int &x, int &y)
{
	m_pSprite->get_alignment(origin, x, y);
}

void MovingEntity::SetAlignment(int origin, CL_Vector2 v)
{

	if (m_pSprite && m_pSprite->get_frame_count() != 0)
	{
		m_pSprite->set_alignment(CL_Origin(origin), v.x, v.y);
	} else
	{
		LogError("Entity %d (%s) can't set alignment, it has no sprite visual yet.", ID(), GetName().c_str());
	}
}


//Note, this is NOT safe to use except in the post update functions.  An entity might have
//been deleted otherwise.

tile_list & MovingEntity::GetNearbyTileList()
{
	return m_nearbyTileList;
}

CL_Rect MovingEntity::GetImageClipRect()
{
	return StringToRect(m_dataManager.Get(C_ENT_TILE_RECT_KEY));
}

unsigned int MovingEntity::GetImageID()
{
	return CL_String::to_int(m_dataManager.Get(C_ENT_TILE_PIC_ID_KEY));
}

void MovingEntity::SetImage(const string &fileName, CL_Rect *pSrcRect)
{
	unsigned int imageID = FileNameToID(fileName.c_str());


	if (!GetHashedResourceManager->HashedIDExists(imageID))
	{
		//The image doesn't exist in our resource manager.  But let's try to add it...
	
		string fullFileName = ProcessPath(fileName);
		if (g_VFManager.LocateFile(fullFileName))
		{
			//LogMsg("Located file %s", fullFileName.c_str()); 
			GetHashedResourceManager->AddGraphic(fullFileName);
		}
	}

	SetImageByID(imageID, pSrcRect);

}


void MovingEntity::AddImageToMapCache(const string &fileName)
{
	unsigned int imageID = FileNameToID(fileName.c_str());


	if (!GetHashedResourceManager->HashedIDExists(imageID))
	{
		//The image doesn't exist in our resource manager.  But let's try to add it...

		string fullFileName = ProcessPath(fileName);
		if (g_VFManager.LocateFile(fullFileName))
		{
			//LogMsg("Located file %s", fullFileName.c_str()); 
			GetHashedResourceManager->AddGraphic(fullFileName);
		}
	}
}

/*
void MovingEntity::SetClipRect(CL_Rect *pSrcRect)
{
	if (!m_pSprite || m_pSprite->get_frame_count() == 0)
	{
		LogError("Entity:SetClipRect requires that an image be loaded before using.  Like with SetImage or something.");
		return;

	}
	
	if (!m_bUsingSimpleSprite)
	{
		LogError("Entity:SetClipRect can currently only be used with simple tilepic images, such as those set with SetImage.");
		return;
	}

	m_pSprite->get

}
*/

bool MovingEntity::SetImageByID(unsigned int picID, CL_Rect *pSrcRect)
{

	CL_Surface *pSurf = GetHashedResourceManager->GetResourceByHashedID(picID);
	if (!pSurf)
	{
		LogError("Error, entity %d (%s) is unable to find image with hash %u to use.", ID(), GetName().c_str(), picID);
		return false;
	} 

	int x = 0;
	int y = 0;
	CL_Origin align = origin_center; //default

	if (m_pSprite && m_pSprite->get_frame_count() != 0)
	{
		m_pSprite->get_alignment(align, x,y);
	}

	SAFE_DELETE(m_pSprite);

	m_pSprite = new CL_Sprite();
	static CL_Rect imageRect;
	if (pSrcRect == NULL )
	{
		//use default image size
		imageRect = CL_Rect(0,0, pSurf->get_width()-1, pSurf->get_height()-1);
		pSrcRect = &imageRect;
	}
	m_pSprite->add_frame(*pSurf, *pSrcRect);
	m_pSprite->set_alignment(align,x,y);

	if (m_bUsingCustomCollisionData)
	{
		SAFE_DELETE(m_pCollisionData);
		m_bUsingCustomCollisionData = false;
	}

	SetCollisionInfoFromPic(picID, *pSrcRect);

	m_bUsingSimpleSprite = true;
	m_dataManager.Set(C_ENT_TILE_PIC_ID_KEY, CL_String::from_int(picID));
	m_dataManager.Set(C_ENT_TILE_RECT_KEY, RectToString(*pSrcRect));

	
	return true;
}


bool MovingEntity::Init()
{
	m_body.SetParentEntity(this);
	assert(!m_pSprite);

	unsigned int picID = CL_String::to_int(m_dataManager.Get(C_ENT_TILE_PIC_ID_KEY));
	
	if (picID != 0)
	{
			CL_Rect picSrcRect = StringToRect(m_dataManager.Get(C_ENT_TILE_RECT_KEY));
		
			if (SetImageByID(picID, &picSrcRect))
			{
				m_pSprite->set_alignment(origin_center,0,0);
			}
	}

	if (!m_pSprite)
	m_pSprite = new CL_Sprite(); //visual data will be set on it later


	string stEmergencyMessage;

	if (!m_mainScript.empty())
	if (!LoadScript(m_mainScript.c_str()))
	{
		stEmergencyMessage = "Ent "+CL_String::from_int(ID()) + " ("+GetName()+") error while loading lua script " + m_mainScript +".";
		LogError(stEmergencyMessage.c_str());
		LoadScript(C_DEFAULT_SCRIPT);
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
			
			string sFile = "system/system.xml";
			g_VFManager.LocateFile(sFile);
			SetVisualProfile(sFile,"ent_default");
		}

		if (!m_pVisualProfile->GetSprite(VisualProfile::VISUAL_STATE_IDLE, VisualProfile::FACING_LEFT))
		{
			LogMsg("Unable to set default anim of idle_left in profile %s!", m_pVisualProfile->GetName().c_str());
		}
		SetSpriteData(m_pVisualProfile->GetSprite(VisualProfile::VISUAL_STATE_IDLE, VisualProfile::FACING_LEFT));
	}


	//override settings with certain things found
	if (m_dataManager.HasData())
	{

		
		if (m_dataManager.Exists(C_ENT_DENSITY_KEY))
		{
			if (GetCollisionData() && GetCollisionData()->HasData())
			{
				SetDensity(CL_String::to_float(m_dataManager.Get(C_ENT_DENSITY_KEY)));
			}
		}

		if (m_dataManager.Exists(C_ENT_ROTATE_KEY))
		{
			if (GetCollisionData() && GetCollisionData()->HasData())
			{
				EnableRotation(CL_String::to_bool(m_dataManager.Get(C_ENT_ROTATE_KEY)));
			}

		}
	}

	m_bHasRunOnInit = true;
	return true;
}

void MovingEntity::AddEffect(L_ParticleEffect *pEffect)
{
	pEffect->trigger();
	pEffect->initialize();
	m_effectManager.add(pEffect);
}

void MovingEntity::ResetEffects()
{
	if (m_effectManager.effect_list.size() > 0)
	{

		std::list<L_ParticleEffect*>::iterator effectItor = m_effectManager.effect_list.begin();

		for (; effectItor != m_effectManager.effect_list.end(); effectItor++)
		{
			(*effectItor)->clear();

		}

	}
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

void MovingEntity::RunOnMapInsertIfNeeded()
{
	if (!m_bHasRunOnMapInsert)
	{

		if (m_pScriptObject)
		{
			if (m_pScriptObject->FunctionExists("OnMapInsert"))
				m_pScriptObject->RunFunction("OnMapInsert");
		}

		m_bHasRunOnMapInsert = true;
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

bool MovingEntity::ScriptNotReady(const string &func, bool bShowError)
{

	if (!m_pScriptObject)
	{
		if (bShowError)
		{
			LogError("Can't RunFunction %s on entity %d (%s), no script is attached.",
				func.c_str(), ID(), GetName().c_str());
		}

		return true; //signal error condition
	}

	return false; //no error
}

luabind::object MovingEntity::RunFunction(const string &func)
{
	luabind::object ret;
	
	if (ScriptNotReady(func)) return ret; //also shows an error for us

	GetScriptManager->SetStrict(false);
	try {ret = luabind::call_function<luabind::object>(m_pScriptObject->GetState(), func.c_str());
	} LUABIND_ENT_CATCH( ("Error while calling function " + func).c_str());
	GetScriptManager->SetStrict(true);

	return ret;
}


luabind::object MovingEntity::RunFunction(const string &func, luabind::object obj1)
{
	luabind::object ret;
	if (ScriptNotReady(func)) return ret; //also shows an error for us
	GetScriptManager->SetStrict(false);

	try {ret = luabind::call_function<luabind::object>(m_pScriptObject->GetState(), func.c_str(), obj1);
	} LUABIND_ENT_CATCH( ("Error while calling function " + func).c_str());
	GetScriptManager->SetStrict(true);
return ret;
}

luabind::object MovingEntity::RunFunction(const string &func, luabind::object obj1, luabind::object obj2)
{

	luabind::object ret;

	if (ScriptNotReady(func)) return ret; //also shows an error for us
	GetScriptManager->SetStrict(false);

	try {ret = luabind::call_function<luabind::object>(m_pScriptObject->GetState(), func.c_str(), obj1, obj2);
	} LUABIND_ENT_CATCH( ("Error while calling function " + func).c_str());
	GetScriptManager->SetStrict(true);
return ret;
}

luabind::object MovingEntity::RunFunction(const string &func, luabind::object obj1, luabind::object obj2, luabind::object obj3)
{
	luabind::object ret;

	if (ScriptNotReady(func)) return ret; //also shows an error for us
	GetScriptManager->SetStrict(false);

	try {ret = luabind::call_function<luabind::object>(m_pScriptObject->GetState(), func.c_str(), obj1, obj2, obj3);
	} LUABIND_ENT_CATCH( ("Error while calling function " + func).c_str());
	GetScriptManager->SetStrict(true);
return ret;
}

luabind::object MovingEntity::RunFunction(const string &func, luabind::object obj1, luabind::object obj2, luabind::object obj3, luabind::object obj4)
{
	luabind::object ret;

	if (ScriptNotReady(func)) return ret; //also shows an error for us
	GetScriptManager->SetStrict(false);

	try {ret = luabind::call_function<luabind::object>(m_pScriptObject->GetState(), func.c_str(), obj1, obj2, obj3, obj4);
	} LUABIND_ENT_CATCH( ("Error while calling function " + func).c_str());
	GetScriptManager->SetStrict(true);
return ret;
}

luabind::object MovingEntity::RunFunction(const string &func, luabind::object obj1, luabind::object obj2, luabind::object obj3, luabind::object obj4, luabind::object obj5)
{
	luabind::object ret;

	if (ScriptNotReady(func)) return ret; //also shows an error for us
	GetScriptManager->SetStrict(false);

	try {ret = luabind::call_function<luabind::object>(m_pScriptObject->GetState(), func.c_str(), obj1, obj2, obj3, obj4, obj5);
	} LUABIND_ENT_CATCH( ("Error while calling function " + func).c_str());
	GetScriptManager->SetStrict(true);
return ret;
}


luabind::object MovingEntity::RunFunction(const string &func, luabind::object obj1, luabind::object obj2, luabind::object obj3, luabind::object obj4, luabind::object obj5, luabind::object obj6)
{
	luabind::object ret;

	if (ScriptNotReady(func)) return ret; //also shows an error for us
	GetScriptManager->SetStrict(false);

	try {ret = luabind::call_function<luabind::object>(m_pScriptObject->GetState(), func.c_str(), obj1, obj2, obj3, obj4, obj5, obj6);
	} LUABIND_ENT_CATCH( ("Error while calling function " + func).c_str());
	GetScriptManager->SetStrict(true);
return ret;
}

luabind::object MovingEntity::RunFunction(const string &func, luabind::object obj1, luabind::object obj2, luabind::object obj3, luabind::object obj4, luabind::object obj5, luabind::object obj6, luabind::object obj7)
{
	luabind::object ret;

	if (ScriptNotReady(func)) return ret; //also shows an error for us
	GetScriptManager->SetStrict(false);

	try {ret = luabind::call_function<luabind::object>(m_pScriptObject->GetState(), func.c_str(), obj1, obj2, obj3, obj4, obj5, obj6, obj7);
	} LUABIND_ENT_CATCH( ("Error while calling function " + func).c_str());
	GetScriptManager->SetStrict(true);
return ret;
}

luabind::object MovingEntity::RunFunction(const string &func, luabind::object obj1, luabind::object obj2, luabind::object obj3, luabind::object obj4, luabind::object obj5, luabind::object obj6, luabind::object obj7, luabind::object obj8)
{
	luabind::object ret;

	if (ScriptNotReady(func)) return ret; //also shows an error for us
	GetScriptManager->SetStrict(false);

	try {ret = luabind::call_function<luabind::object>(m_pScriptObject->GetState(), func.c_str(), obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8);
	} LUABIND_ENT_CATCH( ("Error while calling function " + func).c_str());
	GetScriptManager->SetStrict(true);
return ret;
}



luabind::object MovingEntity::RunFunctionIfExists(const string &func)
{
	luabind::object ret;

	if (ScriptNotReady(func, false)) return ret; //also shows an error for us
	if (!GetScriptObject()->FunctionExists(func.c_str())) return ret; //silent error
	try {ret = luabind::call_function<luabind::object>(m_pScriptObject->GetState(), func.c_str());
	} LUABIND_ENT_CATCH( ("Error while calling function " + func).c_str());
	return ret;
}


luabind::object MovingEntity::RunFunctionIfExists(const string &func, luabind::object obj1)
{
	luabind::object ret;
	if (ScriptNotReady(func, false)) return ret; //also shows an error for us
	if (!GetScriptObject()->FunctionExists(func.c_str())) return ret; //silent error

	try {ret = luabind::call_function<luabind::object>(m_pScriptObject->GetState(), func.c_str(), obj1);
	} LUABIND_ENT_CATCH( ("Error while calling function " + func).c_str());
	return ret;
}

luabind::object MovingEntity::RunFunctionIfExists(const string &func, luabind::object obj1, luabind::object obj2)
{

	luabind::object ret;

	if (ScriptNotReady(func, false)) return ret; //also shows an error for us
	if (!GetScriptObject()->FunctionExists(func.c_str())) return ret; //silent error

	try {ret = luabind::call_function<luabind::object>(m_pScriptObject->GetState(), func.c_str(), obj1, obj2);
	} LUABIND_ENT_CATCH( ("Error while calling function " + func).c_str());
	return ret;
}

luabind::object MovingEntity::RunFunctionIfExists(const string &func, luabind::object obj1, luabind::object obj2, luabind::object obj3)
{
	luabind::object ret;

	if (ScriptNotReady(func, false)) return ret; //also shows an error for us
	if (!GetScriptObject()->FunctionExists(func.c_str())) return ret; //silent error

	try {ret = luabind::call_function<luabind::object>(m_pScriptObject->GetState(), func.c_str(), obj1, obj2, obj3);
	} LUABIND_ENT_CATCH( ("Error while calling function " + func).c_str());
	return ret;
}

luabind::object MovingEntity::RunFunctionIfExists(const string &func, luabind::object obj1, luabind::object obj2, luabind::object obj3, luabind::object obj4)
{
	luabind::object ret;

	if (ScriptNotReady(func, false)) return ret; //also shows an error for us
	if (!GetScriptObject()->FunctionExists(func.c_str())) return ret; //silent error

	try {ret = luabind::call_function<luabind::object>(m_pScriptObject->GetState(), func.c_str(), obj1, obj2, obj3, obj4);
	} LUABIND_ENT_CATCH( ("Error while calling function " + func).c_str());
	return ret;
}

luabind::object MovingEntity::RunFunctionIfExists(const string &func, luabind::object obj1, luabind::object obj2, luabind::object obj3, luabind::object obj4, luabind::object obj5)
{
	luabind::object ret;

	if (ScriptNotReady(func, false)) return ret; //also shows an error for us
	if (!GetScriptObject()->FunctionExists(func.c_str())) return ret; //silent error

	try {ret = luabind::call_function<luabind::object>(m_pScriptObject->GetState(), func.c_str(), obj1, obj2, obj3, obj4, obj5);
	} LUABIND_ENT_CATCH( ("Error while calling function " + func).c_str());
	return ret;
}


luabind::object MovingEntity::RunFunctionIfExists(const string &func, luabind::object obj1, luabind::object obj2, luabind::object obj3, luabind::object obj4, luabind::object obj5, luabind::object obj6)
{
	luabind::object ret;
	
	if (ScriptNotReady(func, false)) return ret; //also shows an error for us
	if (!GetScriptObject()->FunctionExists(func.c_str())) return ret; //silent error

	try {ret = luabind::call_function<luabind::object>(m_pScriptObject->GetState(), func.c_str(), obj1, obj2, obj3, obj4, obj5, obj6);
	} LUABIND_ENT_CATCH( ("Error while calling function " + func).c_str());
	return ret;
}

luabind::object MovingEntity::RunFunctionIfExists(const string &func, luabind::object obj1, luabind::object obj2, luabind::object obj3, luabind::object obj4, luabind::object obj5, luabind::object obj6, luabind::object obj7)
{
	luabind::object ret;

	if (ScriptNotReady(func, false)) return ret; //also shows an error for us
	if (!GetScriptObject()->FunctionExists(func.c_str())) return ret; //silent error

	try {ret = luabind::call_function<luabind::object>(m_pScriptObject->GetState(), func.c_str(), obj1, obj2, obj3, obj4, obj5, obj6, obj7);
	} LUABIND_ENT_CATCH( ("Error while calling function " + func).c_str());
	return ret;
}

luabind::object MovingEntity::RunFunctionIfExists(const string &func, luabind::object obj1, luabind::object obj2, luabind::object obj3, luabind::object obj4, luabind::object obj5, luabind::object obj6, luabind::object obj7, luabind::object obj8)
{
	luabind::object ret;

	if (ScriptNotReady(func, false)) return ret; //also shows an error for us
	if (!GetScriptObject()->FunctionExists(func.c_str())) return ret; //silent error

	try {ret = luabind::call_function<luabind::object>(m_pScriptObject->GetState(), func.c_str(), obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8);
	} LUABIND_ENT_CATCH( ("Error while calling function " + func).c_str());
	return ret;
}

void MovingEntity::UpdateTilePosition()
{
		assert(m_pTile);
		assert(m_pTile->GetParentScreen());

	if (m_bMovedFlag)
	{
		Map *pWorldToMoveTo = m_pTile->GetParentScreen()->GetParentMapChunk()->GetParentMap();
		
		if (!m_strWorldToMoveTo.empty())
		{
			 MapInfo *pInfo = g_pMapManager->GetMapInfoByName(m_strWorldToMoveTo);

			 if (pInfo)
			 {
				
				//load it if needed
				 if (!pInfo->m_world.IsInitted())
				 {
					 g_pMapManager->LoadMap(pInfo->m_world.GetDirPath(), false);
					 pInfo->m_world.PreloadMap();

				 }
				 
				pWorldToMoveTo = &pInfo->m_world;
				m_body.GetPosition().x = m_moveToAtEndOfFrame.x;
				m_body.GetPosition().y = m_moveToAtEndOfFrame.y;

				SAFE_DELETE(m_pPathPlanner); //this would be invalid now

				if (GetCamera->GetEntTracking() == ID())
				{
					 //the camera should follow the entity
					CameraSetting cs = GetCamera->GetCameraSettings();
					g_pMapManager->SetActiveMapByPath(pInfo->m_world.GetDirPath(),&cs);
				}
				ResetEffects();

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
		
		if (m_requestNewLayerID != C_LAYER_NONE)
		{
			m_pTile->SetLayer(m_requestNewLayerID);
			m_requestNewLayerID = C_LAYER_NONE;
		}

		pWorldToMoveTo->AddTile(m_pTile);
		m_bMovedFlag = false;
	} 

}

void MovingEntity::SetAnimByName(const string &name)
{
	if (GetVisualProfile())
	{
		//LogMsg("Setting brain %s", name.c_str());
		m_animID = GetVisualProfile()->TextToAnimID(name);
		SetSpriteData(GetVisualProfile()->GetSpriteByAnimID(m_animID));
	}
}

void MovingEntity::SetAnimFrame(int frame)
{
	if (GetSprite() && GetSprite()->get_frame_count() > 0)
	{
		if (frame == C_ANIM_FRAME_LAST)
		{
			frame = GetSprite()->get_frame_count()-1;
		}
		GetSprite()->restart(); //clear finished flags, timing
		GetSprite()->set_frame(frame);	
		
	} else
	{
		LogMsg("Error, ent %d (%s) can't set anim frame, no visual seems to be assigned to it. (Use OnPostInit maybe?)", ID(), GetName().c_str());
	}

	m_bRestartAnim = false; //if this had been set, we probably don't want it activated now
}

int MovingEntity::GetAnimFrame()
{
	if (GetSprite())
	{
		return GetSprite()->get_current_frame();

	} else
	{
		LogMsg("Error, ent %d (%s) can't get anim frame, no visual seems to be assigned to it. (Use OnPostInit maybe?)", ID(), GetName().c_str());
	}

	return 0;
}

void MovingEntity::OnDamage(const CL_Vector2 &normal, float depth, MovingEntity * enemy, int damage, int uservar, MovingEntity * pProjectile)
{
	SetOnLadder(false);

	if (!GetScriptObject() || !GetScriptObject()->FunctionExists("OnDamage")) return;

	try {luabind::call_function<void>(m_pScriptObject->GetState(), "OnDamage", normal, depth, enemy, damage, uservar, pProjectile);
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

	switch (m_collisionMode)
	{

	case COLLISION_MODE_NONE:
		*pBoolAllowCollision = false;
		return;
		break;

	case COLLISION_MODE_STATIC_ONLY:
		if (pOtherBody->GetParentEntity() != 0)
		{
			*pBoolAllowCollision = false;
			return;
		}
		break;


	case COLLISION_MODE_ENTITIES_ONLY:
		if (pOtherBody->GetParentEntity() == 0)
		{
			//it's static, ignore this collision	
			*pBoolAllowCollision = false;
			return;
		}
		break;

	case COLLISION_MODE_PLAYER_ONLY:
		if ( !GetPlayer || pOtherBody->GetParentEntity() != GetPlayer)
		{
			//it's not the player...
			*pBoolAllowCollision = false;
			return;
		}
	}


	m_bOldTouchedAGroundThisFrame = m_bTouchedAGroundThisFrame;
	m_oldFloorMaterialID = m_floorMaterialID;

	if (N.y < -0.7f) //higher # here means allows a stronger angled surface to count as "ground"
	{
		//LogMsg("%s Touched ground of some sort", GetName().c_str());
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
				GetScriptManager->SetStrict(false);
				try {	*pBoolAllowCollision = luabind::call_function<bool>(m_pScriptObject->GetState(), "OnCollision", CL_Vector2(N.x, N.y), t,  pOtherBody->GetMaterial()->GetID(),  pOtherBody->GetParentEntity());
				} LUABIND_ENT_CATCH("Error while calling OnCollision(Vector2, float, materialID, Entity)");
			}
			GetScriptManager->SetStrict(true);

			break;

		case LISTEN_COLLISION_ALL_ENTITIES:
			GetScriptManager->SetStrict(false);
			try {	*pBoolAllowCollision = luabind::call_function<bool>(m_pScriptObject->GetState(), "OnCollision", CL_Vector2(N.x, N.y), t,  pOtherBody->GetMaterial()->GetID(), pOtherBody->GetParentEntity());
			} LUABIND_ENT_CATCH("Error while calling OnCollision(Vector2, float, materialID, Entity)");
			GetScriptManager->SetStrict(true);

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
			GetScriptManager->SetStrict(false);
			try {	*pBoolAllowCollision = luabind::call_function<bool>(m_pScriptObject->GetState(), "OnCollisionStatic", CL_Vector2(N.x, N.y), t, pOtherBody->GetMaterial()->GetID());
			} LUABIND_ENT_CATCH("Error while calling OnCollisionStatic(Vector2, float, materialID)");
			GetScriptManager->SetStrict(true);

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
		m_bTouchedAGroundThisFrame = true;
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
	if (!m_bCanRotate)
	{
		GetBody()->SetInertia(0);
	}

	if (fDensity == 0) SetMass(0);
}

void MovingEntity::EnableRotation(bool bRotate)
{
	m_bCanRotate = bRotate;
	SetDensity(m_fDensity);
}

bool MovingEntity::GetEnableRotation()
{
	return m_bCanRotate;
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
	
	m_bUsingTilePicCollision = false;

	if (fileName == "")	
	{
		//guess they really don't want collision
		m_bUsingCustomCollisionData = false;
		return;
	}
	
	m_pCollisionData = new CollisionData;
    m_bUsingCustomCollisionData = true;
	
	m_pCollisionData->Load(C_DEFAULT_SCRIPT_PATH + ProcessPathNoScript(fileName)); //it will init a default if the file is not found, and automatically handle

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
	SetDensity(m_fDensity);
	
	m_pCollisionData->SetScale(GetScale());
}

void MovingEntity::SetCollisionInfoFromPic(unsigned picID, const CL_Rect &recPic)
{
	if (m_bUsingCustomCollisionData)
	{
		SAFE_DELETE(m_pCollisionData);
	}

	m_bUsingTilePicCollision = true;
	m_bUsingCustomCollisionData = false;
	
	//copy it
	m_pCollisionData = GetHashedResourceManager->GetCollisionDataByHashedIDAndRect(picID, recPic);
	
	if (m_pTile->GetScale().x != 1 || m_pTile->GetScale().y != 1)
	{
		//we need to keep our own transformed version

		if (m_pCollisionData->GetLineList()->size() > 0)
		{
			//well, this has real data most likely, so let's create our own copy and apply scale to it
			CollisionData *pNewCol = new CollisionData(*m_pCollisionData);
			m_pCollisionData = pNewCol;
			m_pCollisionData->SetDataChanged(false);
			m_bUsingCustomCollisionData = true;
			m_pCollisionData->SetScale(GetScale());
		}
	}


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

	//m_pSprite->set_alignment(origin_center);

	//get notified of collisions
	m_collisionSlot = m_body.sig_collision.connect(this, &MovingEntity::OnCollision);

	//link to data correctly
	
	if (pActiveLine)
	{
		pActiveLine->GetAsBody(CL_Vector2(0,0), &m_body);
		SetDensity(m_fDensity);
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

	CL_Origin origin;
	int offsetX,offsetY;
	m_pSprite->get_alignment(origin, offsetX, offsetY);
	CL_Pointf vecOrigin = calc_origin(origin,CL_Size(x,y));


	CL_Rect colRect(-vecOrigin.x, -vecOrigin.y, x - vecOrigin.x, y - vecOrigin.y);

	m_pCollisionData = new CollisionData;
	
	PointList pl;
	m_pCollisionData->GetLineList()->push_back(pl);
	PointList *pActiveLine;
	pActiveLine = &(*m_pCollisionData->GetLineList()->begin());

	//add our lines by hand
    pActiveLine->GetPointList()->resize(4);
	pActiveLine->GetPointList()->at(0) = CL_Vector2(colRect.left,colRect.top);
	pActiveLine->GetPointList()->at(1) = CL_Vector2(colRect.right,colRect.top);
	pActiveLine->GetPointList()->at(2) = CL_Vector2(colRect.right, colRect.bottom);
	pActiveLine->GetPointList()->at(3) = CL_Vector2(colRect.left, colRect.bottom);
	pActiveLine->CalculateOffsets();
	//get notified of collisions
 	m_collisionSlot = m_body.sig_collision.connect(this, &MovingEntity::OnCollision);
	m_bUsingCustomCollisionData = true;
	//link to data correctly
	pActiveLine->GetAsBody(CL_Vector2(0,0), &m_body);

	SetDensity(1);

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

	int lineCount = pCol->GetLineList()->size();

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

				//the dangerous thing about Collide is it calls scripts which could replace or remove our collision info!
				if (m_pCollisionData)
				{
					//fix it back from reversed mode
					m_pCollisionData->GetLineList()->begin()->GetAsBody(CL_Vector2(0,0), &m_body);

				} else
				{
					return;
				}
				
				//we're assuming entities only have one collision line, we leave now to avoid a crash if their
				//collision info was removed or changed in a possible script callback
				if (pTile->GetType() == C_TILE_TYPE_ENTITY) return;

				
			} else
			{
				m_body.Collide(*pWallBody, step);

				//we're assuming entities only have one collision line, we leave now to avoid a crash if their
				//collision info was removed or changed in a possible script callback
				if (pTile->GetType() == C_TILE_TYPE_ENTITY) return;

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
			if (m_pSprite && !m_pSprite->is_null()) m_pSprite->restart();
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

	Map *pWorld = m_pTile->GetParentScreen()->GetParentMapChunk()->GetParentMap();

	pWorld->GetMyMapCache()->AddTilesByRect(CL_Rect(m_scanArea), &m_nearbyTileList, pWorld->GetLayerManager().GetCollisionList());
	
	m_pCollisionData->GetLineList()->begin()->GetAsBody(CL_Vector2(0,0), &m_body);

	ProcessCollisionTileList(m_nearbyTileList, step);
	
	if (m_bTouchedAGroundThisFrame)
	{
		m_ticksOnGround++;

		if (m_ticksOnGround >= C_TICKS_ON_GROUND_NEEDED_TO_TRIGGER_GROUND)
		{
			//LogMsg("%s turned on ground true", GetName().c_str());
			SetIsOnGround(true);
		}
	} else
	{
		m_ticksOnGround = 0;
	}
	
}


CL_Vector2 MovingEntity::GetVisualOffset()
{
	if (m_pCollisionData && m_pCollisionData->HasData()) 
	{
		return -m_pCollisionData->GetCombinedOffsets();
	}
	return CL_Vector2(0,0);
}

void MovingEntity::RenderShadow(void *pTarget)
{
	static CL_Vector2 vecPos;
	static Map *pWorld;
	static EntMapCache *pWorldCache;

	CL_GraphicContext *pGC = (CL_GraphicContext *)pTarget;
	CL_Vector2 vVisualPos = GetPos() + GetVisualOffset();

	pWorld = m_pTile->GetParentScreen()->GetParentMapChunk()->GetParentMap();
	pWorldCache = pWorld->GetMyMapCache();

	vecPos = pWorldCache->WorldToScreen(vVisualPos);

	if (m_bLockedScale)
	{
		m_pSprite->set_scale( m_pTile->GetScale().x, m_pTile->GetScale().y);
	} else
	{
		m_pSprite->set_scale(GetCamera->GetScale().x * m_pTile->GetScale().x, GetCamera->GetScale().y * m_pTile->GetScale().y);
	}

	short a = min(50, 255 + m_colorModAlpha);
	
	m_pSprite->set_color(CL_Color(0,0,0,a));

	static CL_Surface_DrawParams1 params1;
	m_pSprite->setup_draw_params(vecPos.x, vecPos.y, params1, true);
	AddShadowToParam1(params1, m_pTile);
	m_pSprite->draw(params1, pGC);

}

void MovingEntity::Stop()
{
	GetBody()->GetNetForce() = Vector(0,0);
	GetBody()->GetLinVelocity() = Vector(0,0);
	GetBody()->GetAngVelocity() = 0;
}

void MovingEntity::StopY()
{
	GetBody()->GetNetForce().y = 0;
	GetBody()->GetLinVelocity().y = 0;
	GetBody()->GetAngVelocity() = 0;
}

void MovingEntity::StopX()
{
	GetBody()->GetNetForce().x = 0;
	GetBody()->GetLinVelocity().x = 0;
	GetBody()->GetAngVelocity() = 0;
}


void MovingEntity::SetRotation(float angle)
{
	m_body.SetOrientation(angle);
}
float MovingEntity::GetRotation()
{
	return m_body.GetOrientation();
}


CL_Vector2 MovingEntity::GetRawScreenPosition(bool &bRootIsCam)
{

	bRootIsCam = false;
	//assert(m_bLockedScale && m_attachEntID != 0);

	
	if (m_attachEntID == C_ENTITY_CAMERA)
	{
		bRootIsCam = true;
		return m_attachOffset;
	}

	if (m_attachEntID == 0)
	{
		//not attached to anything...
		return GetPos()-GetCamera->GetPos();
	}
	MovingEntity *pEnt = (MovingEntity*) EntityMgr->GetEntityFromID(m_attachEntID);
	return pEnt->GetRawScreenPosition(bRootIsCam)+m_attachOffset;
	
}
void MovingEntity::Render(void *pTarget)
{
	CL_GraphicContext *pGC = (CL_GraphicContext *)pTarget;
	static float yawHold, pitchHold;

	
	CL_Vector2 vVisualPos = GetPos() + GetVisualOffset();
	CL_Vector2 vFinalScale = m_pTile->GetScale();

		
	if (!m_bLockedScale)
	{
		vFinalScale.x *= GetCamera->GetScale().x;
		vFinalScale.y *= GetCamera->GetScale().y;
	}

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
	
	static CL_Vector2 vecPos;

	if (a > 0)
	{
	
		static bool bUseParallax;
		static Layer *pLayer;
		static Map *pWorld;
		pWorld = m_pTile->GetParentScreen()->GetParentMapChunk()->GetParentMap();
		static EntMapCache *pWorldCache;
	
		pWorldCache = pWorld->GetMyMapCache();

		if (GetGameLogic()->GetParallaxActive())
		{
			bUseParallax = true;
			pLayer = &pWorld->GetLayerManager().GetLayerInfo(m_pTile->GetLayer());
			if (pLayer->RequiresParallax())
			{

			if (GetGameLogic()->GetMakingThumbnail())
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

		if (m_bLockedScale && m_attachEntID != 0)
		{
			
			
			bool bRootIsCam;
			
			CL_Vector2 vTemp = GetRawScreenPosition(bRootIsCam);
			if (bRootIsCam)
			{
				vecPos = vTemp;
			}
		}

		clTexParameteri(CL_TEXTURE_2D, CL_TEXTURE_MAG_FILTER, CL_NEAREST);
		clTexParameteri(CL_TEXTURE_2D, CL_TEXTURE_MIN_FILTER, CL_NEAREST);

		m_pSprite->set_scale(vFinalScale.x, vFinalScale.y);
		//do the real blit
		m_pSprite->set_color(CL_Color(r,g,b,a));

		if (m_blendMode != C_BLEND_MODE_NORMAL)
		{
			CL_BlendFunc src,dest;
			m_pSprite->get_blend_func(src, dest);
			switch (m_blendMode)
			{
			case C_BLEND_MODE_ADDITIVE:

				m_pSprite->set_blend_func(blend_src_alpha, blend_one); //screen/additive
				break;
			case C_BLEND_MODE_NEGATIVE:
				m_pSprite->set_blend_func(blend_src_alpha_saturate, blend_one_minus_src_alpha);  //Negative?
				break;
			default:
				LogError("Unknown blend mode: %d", m_blendMode);
			}

			m_pSprite->draw_subpixel( int(vecPos.x), int(vecPos.y), pGC);
			//m_pSprite->draw_subpixel( vecPos.x, vecPos.y, pGC);
			m_pSprite->set_blend_func(src, dest); //put it back how it was

		} else
		{
			m_pSprite->draw_subpixel( int(vecPos.x), int(vecPos.y), pGC);
			//m_pSprite->draw_subpixel( vecPos.x, vecPos.y, pGC);
		}

		//return things back to normal
		m_pSprite->set_angle_yaw(yawHold);
		m_pSprite->set_angle_pitch(pitchHold);
	
		if (m_effectManager.effect_list.size() > 0)
		{
			std::list<L_ParticleEffect*>::iterator effectItor = m_effectManager.effect_list.begin();

			for (; effectItor != m_effectManager.effect_list.end(); effectItor++)
			{
				(*effectItor)->draw(0,0,vFinalScale.x, vFinalScale.y);

			}
		}
	}

	/*
	//***** debug, draw rect
	CL_Rectf worldRect =GetWorldRect();
	CL_Vector2 vecStart(worldRect.left, worldRect.top);
	CL_Vector2 vecStop(worldRect.right, worldRect.bottom);
	DrawRectFromWorldCoordinates(vecStart, vecStop, CL_Color(255,255,255,180), pGC);
	//*********
	*/

	/*
	if (GetGameLogic()->GetMakingThumbnail())
	{
		return;
	}
	*/

	if (!m_text.empty())
	{
		//draw some text too
		vecPos = GetMap()->GetMyMapCache()->WorldToScreen(vVisualPos);
		
		float textScaleX, textScaleY;
		m_pFont->get_scale(textScaleX, textScaleY);

		CL_Origin originTemp; 
		int offsetX, offsetY;

		m_pFont->get_alignment(originTemp, offsetX, offsetY);

		if (m_bLockedScale)
		{
			m_pFont->set_scale(textScaleX , textScaleY);
			
		} else
		{
			m_pFont->set_scale(textScaleX * GetCamera->GetScale().x, textScaleY *GetCamera->GetScale().y);
			m_pFont->set_alignment(originTemp, offsetX * GetCamera->GetScale().x, offsetY * GetCamera->GetScale().y);
		}

		if (m_attachEntID != 0)
		{
			bool bRootIsCam;

			CL_Vector2 vTemp = GetRawScreenPosition(bRootIsCam);
			if (bRootIsCam)
			{
				vecPos = vTemp;
			}
		} else
		{
			//we're not attached to anything			
			//vecPos = m_attachOffset;
		}

		float oldAlpha = m_pFont->get_alpha();
		a = m_pFont->get_color().get_alpha() + m_colorModAlpha;
		a = cl_min(a, 255); a = cl_max(0, a);

		m_pFont->set_alpha(float(a)/255);
		m_pFont->draw( vecPos.x, vecPos.y, m_text, pGC);

		//put it back to how it was
		m_pFont->set_scale(textScaleX, textScaleY);
		m_pFont->set_alpha(oldAlpha);
		m_pFont->set_alignment(originTemp, offsetX, offsetY);
	}

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
		LogMsg("Script %s loaded in entity %d (%s).", m_mainScript.c_str(), ID(), GetName().c_str());
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
	
	m_nearbyTileList.clear();

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


	//while we're at it, if any entities are attached, make sure they "update".  Don't worry, it's smart enough
	//not to allow something to update twice if it's already done it

	if (!m_attachedEntities.empty())
	{
		list<int>::iterator itor;
		MovingEntity *pEnt;

		for (itor = m_attachedEntities.begin(); itor != m_attachedEntities.end();)
		{

			pEnt = (MovingEntity*) EntityMgr->GetEntityFromID(*itor);
			if (pEnt)
			{
				pEnt->ProcessPendingMoveAndDeletionOperations();
			}
			itor++;

		}
	}

}

void MovingEntity::CheckVisibilityNotifications(unsigned int notificationID)
{

	if (GetGameLogic()->GetGamePaused()) return;

	if (notificationID != m_lastVisibilityNotificationID)
	{
		SetIsOnScreen(false);
	} 
}

void MovingEntity::Update(float step)
{

if (GetGameLogic()->GetGamePaused()) return;

	if (g_watchManager.GetVisibilityID() == m_lastVisibilityNotificationID)
	{
		//we've already run our update
		return;
	}



	m_lastVisibilityNotificationID = g_watchManager.GetVisibilityID();
	m_bRanPostUpdate = false;
	m_bRanApplyPhysics = false;

	ClearColorMods();

	RunPostInitIfNeeded();


	if (m_bRunUpdateEveryFrame)
	{
		GetScriptManager->SetStrict(false);
		try {luabind::call_function<void>(m_pScriptObject->GetState(), "Update", step);
		} LUABIND_ENT_CATCH("Error while calling function Update");
		GetScriptManager->SetStrict(true);

	}
	
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
	{
		m_pSprite->update( float(GetApp()->GetGameLogicSpeed()) / 1000.0f );
	}

	if (m_effectManager.effect_list.size() > 0)
		{
			std::list<L_ParticleEffect*>::iterator effectItor = m_effectManager.effect_list.begin();

			for (; effectItor != m_effectManager.effect_list.end(); effectItor++)
			{
				(*effectItor)->set_position(GetPos().x, GetPos().y);
				(*effectItor)->set_velocity(L_Vector(GetLinearVelocity()/4));
			}
		}
	
	m_effectManager.run( (GetApp()->GetGameLogicSpeed()), false);

	if (!m_attachedEntities.empty())
	{
		list<int>::iterator itor;
		MovingEntity *pEnt;

		for (itor = m_attachedEntities.begin(); itor != m_attachedEntities.end();)
		{
			pEnt = (MovingEntity*) EntityMgr->GetEntityFromID(*itor);
			if (pEnt)
			{
				pEnt->Update(step);
			} else
			{
				itor = m_attachedEntities.erase(itor);
				continue;
			}
			itor++;
		}
	}

	//get ready for next frame
	m_bTouchedAGroundThisFrame = false;
}

void MovingEntity::ApplyPhysics(float step)
{
	
	
	
	if (m_attachEntID != C_ENTITY_NONE)
	{
		if (m_attachEntID != C_ENTITY_CAMERA)
		{
			//we are attached to another entity.  It makes sense to not update us until AFTER it updates
			MovingEntity *pEnt = (MovingEntity*)EntityMgr->GetEntityFromID(m_attachEntID);
			if (!pEnt->HasRunPhysics())
			{
				return; //don't update yet
			}
		}
	}

	
	//apply gravity and things
	m_zoneVec.clear();

	if (m_bRanApplyPhysics) return; else m_bRanApplyPhysics = true;

	if (!GetBody()->IsUnmovable() && GetCollisionData())
	{
		//the draw ID let's other objects know that we've already processed
		m_drawID = g_pMapManager->GetActiveMapCache()->GetUniqueDrawID();

		ApplyGenericMovement(step);
	}

	if (m_attachEntID != C_ENTITY_NONE)
	{
		UpdatePositionFromParent();
	} 

	//while we're at it, if any entities are attached *to* us, make sure they "update".  Don't worry, it's smart enough
	//not to allow something to update twice if it's already done it

	if (!m_attachedEntities.empty())
	{
		list<int>::iterator itor;
		MovingEntity *pEnt;

		for (itor = m_attachedEntities.begin(); itor != m_attachedEntities.end();)
		{
			pEnt = (MovingEntity*) EntityMgr->GetEntityFromID(*itor);
			if (pEnt)
			{
				pEnt->ApplyPhysics(step);
			} else
			{
				itor = m_attachedEntities.erase(itor);
				continue;
			}
			itor++;
		}
	}
}

void MovingEntity::AddForce(CL_Vector2 force)
{
	assert(GetApp()->GetDelta() != 0);
	m_body.AddForce( ((*(Vector*)&force)*m_body.GetMass())  );
}

void MovingEntity::AddForceBurst(CL_Vector2 force)
{
	assert(GetApp()->GetDelta() != 0);
	m_body.AddForce( ((*(Vector*)&force)*m_body.GetMass()) / GetApp()->GetDelta()  );
}

void MovingEntity::AddForceAndTorque(CL_Vector2 force, CL_Vector2 torque)
{
	m_body.AddForce( (*(Vector*)&force)*m_body.GetMass(), (*(Vector*)&torque));
}

void MovingEntity::AddForceAndTorqueBurst(CL_Vector2 force, CL_Vector2 torque)
{
	m_body.AddForce( ((*(Vector*)&force)*m_body.GetMass()) / GetApp()->GetDelta(),  ((*(Vector*)&torque)) / GetApp()->GetDelta());
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

		if (m_pScriptObject->FunctionExists("OnWatchTimeout"))
		{
			try {luabind::call_function<void>(GetScriptObject()->GetState(), "OnWatchTimeout", bIsOnScreen);
			} LUABIND_ENT_CATCH( "Error while calling function OnWatchTimeout(bool bIsOnScreen)");
		}
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
	
	if (GetGameLogic()->GetGamePaused()) return;
	if (m_bRanPostUpdate) return; else m_bRanPostUpdate = true;
	if (m_bRequestsVisibilityNotifications)
	{
		if (m_lastVisibilityNotificationID != g_watchManager.GetVisibilityID())
		{
			SetIsOnScreen(true);
		}
		g_watchManager.AddEntityToVisibilityList(this);
	}
	
	m_brainManager.PostUpdate(step);
	float groundDampening = 0.05f;
	if (m_customDampening != -1) groundDampening = m_customDampening;
	Map *pWorld = m_pTile->GetParentScreen()->GetParentMapChunk()->GetParentMap();

	if (!GetBody()->IsUnmovable() &&  GetCollisionData())
	{
		float gravity;

		if (m_gravityOverride != C_GRAVITY_OVERRIDE_DISABLED)
		{
			gravity = m_gravityOverride;
		} else
		{
			gravity = pWorld->GetGravity();
		}

		if (gravity != 0)
		{
			//change to allow gravity in any direction!
			if ( (m_body.GetLinVelocity().y) < C_MAX_FALLING_DOWN_SPEED)
			{
				AddForce(CL_Vector2(0, (gravity)) );
			
				if (!IsOnGroundAccurate())
				{
					//LogMsg(" %s In the air", GetName().c_str());
					//additional gravity
					AddForce(CL_Vector2(0, (gravity*3)) );
				}
			}

			if (IsOnGround() && m_body.GetLinVelocity().Length() < 0.4)
			{
				//apply dampening
				set_float_with_target(&m_body.GetLinVelocity().x, 0, (groundDampening) * step);
				set_float_with_target(&m_body.GetLinVelocity().y, 0, (groundDampening) * step);
				set_float_with_target(&m_body.GetAngVelocity(), 0, (groundDampening/3) *step);
			}

		} else
		{
			//no gravity active
			
				//LogMsg(" %s dampening", GetName().c_str());
				//apply dampening
				set_float_with_target(&m_body.GetLinVelocity().x, 0, (groundDampening) * step);
				set_float_with_target(&m_body.GetLinVelocity().y, 0, (groundDampening) * step);
				set_float_with_target(&m_body.GetAngVelocity(), 0, (groundDampening/3) *step);

		}
		GetBody()->Update(step);
	}

	if (!m_attachedEntities.empty())
	{
		list<int>::iterator itor;
		MovingEntity *pEnt;

		for (itor = m_attachedEntities.begin(); itor != m_attachedEntities.end();)
		{
			pEnt = (MovingEntity*) EntityMgr->GetEntityFromID(*itor);
			if (pEnt)
			{
				pEnt->PostUpdate(step);
			} else
			{
				itor = m_attachedEntities.erase(itor);
				continue;
			}
			itor++;
		}
	}

	m_nearbyTileList.clear();
	
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

CL_Rectf MovingEntity::GetWorldCollisionRect()
{
	//OPTIMIZE for gods sake, cache this
	if (!m_pTile->GetCollisionData()) return CL_Rectf(0,0,0,0);
	return m_pTile->GetWorldColRect();
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
	EntMapCache *pWC = m_pTile->GetParentScreen()->GetParentMapChunk()->GetParentMap()->GetMyMapCache();	
	return !pWC->IsPathObstructed(GetPos(), to, GetBoundingCollisionRadius(), m_pTile, ignoreLivingCreatures);
}

//similar to above. Returns true if the bot can move between the two
//given positions without bumping into any walls
//warning, only tests against edges
bool MovingEntity::CanWalkBetween(Map *pMap, CL_Vector2 from, CL_Vector2 to, bool ignoreLivingCreatures)
{
	return !pMap->GetMyMapCache()->IsPathObstructed(from, to, GetBoundingCollisionRadius(), m_pTile, ignoreLivingCreatures);
}

bool MovingEntity::IsValidPosition(Map *pMap, const CL_Vector2 &pos, bool bIgnoreLivingCreatures)
{

	if (!pMap) pMap = GetMap();
	return !pMap->GetMyMapCache()->IsAreaObstructed(pos, GetBoundingCollisionRadius(), bIgnoreLivingCreatures, this);
}


MovingEntity * MovingEntity::Clone(Map *pMap, CL_Vector2 vecPos)
{
	Tile *pNew = m_pTile->CreateClone();
	
	MovingEntity *pNewEnt = ((TileEntity*)pNew)->GetEntity();
	pNewEnt->SetPos(vecPos);
	if (!pMap)
	{
		//use our own active map, none was specific
		pMap = GetMap();
	}
	
	pMap->AddTile(pNew);
	return pNewEnt;
}

void MovingEntity::OnAttachedEntity(int entID)
{
	m_attachedEntities.push_back(entID);
}




void MovingEntity::UpdatePositionFromParent()
{
	CL_Vector2 vScaleMod(1,1);
	string mapName;
	CL_Vector2 vPos;

	if (m_bLockedScale)
	{
		vScaleMod = GetCamera->GetScale();
		//LogMsg("Ent %d got cam scale %s.  visualoffset is %s", ID(), PrintVector(vScaleMod).c_str(), PrintVector(GetVisualOffset()).c_str());
	}

	if (m_attachEntID == C_ENTITY_CAMERA)
	{

		vPos = GetCamera->GetPos() + m_attachOffset;

		if (GetMap() != g_pMapManager->GetActiveMap())
		{
			mapName = g_pMapManager->GetActiveMap()->GetName();
		}

	}else

	{
		//we're stuck to a common entity
		MovingEntity *pEnt = (MovingEntity*)EntityMgr->GetEntityFromID(m_attachEntID);
		if (!pEnt)
		{
			//the entity we were attached to no longer exists, so let's just kill ourself
			SetDeleteFlag(true);
		} else
		{

			vPos = pEnt->GetPos() + m_attachOffset;

			if (GetMap() != pEnt->GetMap() || !pEnt->GetRequestedMapName().empty())
			{
				if (!pEnt->GetRequestedMapName().empty())
				{
					mapName = pEnt->GetRequestedMapName();
				} else
				{
					mapName = pEnt->GetMap()->GetName();
				}
				vPos = pEnt->GetRequestPosition() + m_attachOffset;
			} 

		}
	}

	if (!mapName.empty())
	{
		SetPosAndMap(vPos, mapName);
		//LogMsg(" Ent %d changing to map %s at %u.", ID(),  mapName.c_str(), GetApp()->GetGameTick());

	} else
	{
		SetPos(vPos);

	}


}

void MovingEntity::OnMapChange( const string &mapName )
{
	
	if (m_attachEntID == C_ENTITY_NONE)
	{
		//don't care	
		return;
	}

	UpdatePositionFromParent();

	//force children to do the same
	if (!m_attachedEntities.empty())
	{
		list<int>::iterator itor;
		MovingEntity *pEnt;

		for (itor = m_attachedEntities.begin(); itor != m_attachedEntities.end();)
		{
			pEnt = (MovingEntity*) EntityMgr->GetEntityFromID(*itor);
			if (pEnt)
			{
				pEnt->UpdatePositionFromParent();
			} 
			itor++;
		}
	}

}

void MovingEntity::SetAttach(int entityID, CL_Vector2 vOffset)
{
	m_attachEntID = entityID;
	m_attachOffset = vOffset;

	if (entityID == C_ENTITY_CAMERA)
	{
		g_watchManager.Add(this, INT_MAX);
		m_bLockedScale = true;

	} else
	{
		//we're stuck to a common entity
		MovingEntity *pEnt = (MovingEntity*)EntityMgr->GetEntityFromID(m_attachEntID);
		if (!pEnt)
		{
			LogError("Warning: Ent %d (%s) can't attach to entity ID %d, doesn't exist", ID(), GetName().c_str(),
				m_attachEntID);
		} else
		{
			if (GetMap() != pEnt->GetMap())
			{
				SetPosAndMap(pEnt->GetPos()+m_attachOffset, pEnt->GetMap()->GetName());
			} else
			{
				SetPos(pEnt->GetPos()+m_attachOffset);
			}
			
			pEnt->OnAttachedEntity(ID());
		}
	}
}

