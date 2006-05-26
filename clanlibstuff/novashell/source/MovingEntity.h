#ifndef MOVING_ENTITY
#define MOVING_ENTITY

#include "BaseGameEntity.h"
#include "physics/Body.h"
#include "CollisionData.h"
#include "Screen.h"
#include "GameLogic.h"
#include "TileEntity.h"
#include "DataManager.h"

#define C_MAX_FALLING_DOWN_SPEED 20 //gravity won't be applied to objects going faster than this

class Brain;
class VisualProfile;

class Zone
{
public:
	
	CL_Rectf m_boundingRect;
	CL_Vector2 m_vPos; //add this to the bounding rect to get the world coordinates
	int m_materialID;
	int m_entityID;

};

class MovingEntity : public BaseGameEntity
{
public:

  void InitCollisionDataBySize(float x, float y);
  MovingEntity();
  virtual ~MovingEntity();

  void Kill();
  bool Init();
  virtual void SetName(const std::string &name);

  const CL_Vector2 & GetPos() {return *(CL_Vector2*)&m_body.GetPosition();}
  void  SetPos(const CL_Vector2 &new_pos);
  void SetPosAndMap(const CL_Vector2 &new_pos, const string &worldName);
  int GetSizeX(){ return m_pSprite->get_width(); };
  int GetSizeY(){ return m_pSprite->get_height(); };
  void AddBrain(int brainType);
  void ApplyGenericMovement(float step);
  tile_list & GetNearbyTileList(); //note, this may not be valid if you're deleting tiles!  For debug only
  CL_Rectf & GetLastScanArea(){return m_scanArea;}
  bool IsAtRest() {return m_bIsAtRest;}
  bool IsOnGround() {return m_groundTimer > GetApp()->GetGameTick();}
  virtual CL_Rectf GetWorldRect();
  void GetAlignment(CL_Origin &origin, int &x, int &y);
  void UpdateTilePosition();
  CollisionData * GetCollisionData() {return m_pCollisionData;}
  bool SetVisualProfile(const string &resourceFileName, const string &profileName);
  virtual void  Update(float step);
  virtual void  Render(void *pTarget);
  BaseGameEntity * CreateClone();
  void SetIsOnGround(bool bOnGround);
  CBody * GetBody(){return &m_body;}
  void SetSpriteData(CL_Sprite *pSprite);
  bool LoadScript(const char *pFileName);
  void SetMainScriptFileName(const string &fileName);
  const string & GetMainScriptFileName() {return m_mainScript;}
  
  VisualProfile * GetVisualProfile() {return m_pVisualProfile;}
  void Serialize(CL_FileHelper &helper);
  void LoadCollisionInfo(const string &fileName);
  void SetCollisionInfoFromPic(unsigned int picID, const CL_Rect &recPic);

  void EnableRotation(bool bRotate);
  bool GetEnableRotation();
  unsigned int GetDrawID() {return m_drawID;}
  float GetMass() {return m_body.GetMass();}
  void SetMass(float mass){m_body.SetMass(0);}
  
  void SetDensity(float fDensity);
  void PostUpdate(float step);
  void SetListenCollision(int eListen) {m_listenCollision = eListen;}
  int GetListenCollision() {return m_listenCollision;}

  void SetListenCollisionStatic(int eListen) {m_listenCollisionStatic = eListen;}
  int GetListenCollisionStatic() {return m_listenCollisionStatic;}
  void AddForce(CL_Vector2 force) {m_body.AddForce( (*(Vector*)&force)*m_body.GetMass());}
  void AddForceAndTorque(CL_Vector2 force, CL_Vector2 torque) {m_body.AddForce( (*(Vector*)&force)*m_body.GetMass(),
	  (*(Vector*)&torque)*m_body.GetMass());}
  CL_Vector2 GetForce() {return *(CL_Vector2*)&m_body.GetNetForce();};
  CL_Vector2 GetLinearVelocity() {return *(CL_Vector2*)&m_body.GetLinVelocity();}
  void SetPersistant(bool bOn){assert(m_pTile); m_pTile->SetBit(Tile::e_notPersistant, !bOn);}
  bool GetPersistant() {assert(m_pTile); return !m_pTile->GetBit(Tile::e_notPersistant);}
  ScriptObject * GetScriptObject() {return m_pScriptObject;}
  Zone * GetZoneWeAreOnByMaterialType(int matType);
  bool InZoneByMaterialType(int matType) {return GetZoneWeAreOnByMaterialType(matType) != NULL;}
  bool GetOnLadder() {return m_bOnLadder;}
  void SetOnLadder(bool bOnLadder) {m_bOnLadder = bOnLadder;}
  CL_Sprite * GetSprite(){return m_pSprite;}
  void SetAnimPause(bool bPause) { m_bAnimPaused = bPause;}
  bool GetAnimPause() {return m_bAnimPaused;}
  virtual void HandleMessageString(const string &msg);
  void SetDefaultTextColor(CL_Color color) {m_defaultTextColor = color;}
  CL_Color GetDefaultTextColor() {return m_defaultTextColor;}
  DataManager * GetData() {return &m_dataManager;}
  unsigned int GetNameHash() {return m_hashedName;}
  bool SetPosAndMapByName(const string &name);
  void SetImageFromTilePic(TilePic *pTilePic);
  bool GetUsingImageFromTilePic() {return m_bUsingSimpleSprite;}
  bool UsingCustomCollisionData() {return m_bUsingCustomCollisionData;}
  int GetFloorMaterialID() {return m_floorMaterialID;}


enum ListenCollision
{
	//I know I don't have to specify the #'s but it helps me visually keep
	//them syncronized with the lua defines that have to match. So don't
	//be changin' the order ya hear?
	
	LISTEN_COLLISION_NONE = 0,
	LISTEN_COLLISION_PLAYER_ONLY = 1,
	LISTEN_COLLISION_ALL_ENTITIES = 2,
};

enum ListenStaticCollision
{
	//I know I don't have to specify the #'s but it helps me visually keep
	//them syncronized with the lua defines that have to match. So don't
	//be changin' the order ya hear?

	LISTEN_COLLISION_STATIC_NONE = 0,
	LISTEN_COLLISION_STATIC_ALL = 1,
};

protected:

	void SetDefaults();

	void ProcessCollisionTileList(tile_list &tList, float step);
	void ProcessCollisionTile(Tile *pTile, float step);
	void OnCollision(const Vector & N, float &t, CBody *pOtherBody, bool *pBoolAllowCollide); 
	
	CL_Rectf m_scanArea;
	tile_list m_nearbyTileList;
    CL_Slot m_collisionSlot;
	CBody m_body; //physics object
	bool m_bIsAtRest;
	bool m_bMovedFlag; //if true, we need to update our tile position
	unsigned int m_groundTimer; //relax when we say they are on the  ground or not to take care of tiny bounces
	CollisionData * m_pCollisionData; //null if we don't collide
	ScriptObject *m_pScriptObject;
	Brain * m_pBrain;
	CL_Sprite *m_pSprite, *m_pSpriteLastUsed;
	VisualProfile *m_pVisualProfile;
	string m_mainScript;
	unsigned int m_timeLastActive;
	float m_fDensity;
	int m_ticksOnGround; 
	bool m_bTouchedAGroundThisFrame;
	unsigned int m_drawID; //if this matches GetWorldCache->GetUniqueDrawID() it means they have
	//already "thought" this cycle
	vector<Zone> m_zoneVec;

	int m_listenCollision, m_listenCollisionStatic;
	bool m_bOnLadder;
	bool m_bAnimPaused;
	CL_Color m_defaultTextColor;
	string m_strWorldToMoveTo; //if empty, we don't have a world move scheduled
	DataManager m_dataManager;
	unsigned int m_hashedName;
	bool m_bUsingSimpleSprite; //no animation, just a still taken from a tile pic
	bool m_bUsingCustomCollisionData; //if true, we need to delete it
	int m_floorMaterialID;

};

MovingEntity * CreateEntity(CL_Vector2 vecPos, string scriptFileName); //creates an entity and puts it in the world



#endif