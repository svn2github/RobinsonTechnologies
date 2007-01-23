#ifndef MOVING_ENTITY
#define MOVING_ENTITY

#include "BaseGameEntity.h"
#include "physics/Body.h"
#include "CollisionData.h"
#include "Screen.h"
#include "GameLogic.h"
#include "TileEntity.h"
#include "DataManager.h"
#include "BrainManager.h"
#include "Trigger.h"

class Goal_Think;
class PathPlanner;
//#include "AI/Goal_Think.h"

#define C_MAX_FALLING_DOWN_SPEED 20 //gravity won't be applied to objects going faster than this
#define C_ENT_TILE_PIC_ID_KEY "_TilePicID" //custom data to signal we use a simple single tilepic for our image
#define C_ENT_TILE_RECT_KEY "_TileRect" //custom data that goes along with the tile ID
#define C_ENT_DENSITY_KEY "_Density" //custom data to override density


#define C_ENT_ROTATE_KEY "_CanRotate" //custom data to override density

#ifndef WIN32
//windows already has this in the precompiled header for speed, I couldn't get that to work on mac..
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#endif


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
  void SetNameEx(const std::string &name, bool bRemoveOldTag);

  const CL_Vector2 & GetPos() {return *(CL_Vector2*)&m_body.GetPosition();}
  void  SetPos(const CL_Vector2 &new_pos);
  void SetPosAndMap(const CL_Vector2 &new_pos, const string &worldName);
  int GetSizeX(){ return int(m_pSprite->get_width() * m_pTile->GetScale().x); };
  int GetSizeY(){ return int(m_pSprite->get_height() * m_pTile->GetScale().y); };
  CL_Vector2 GetScale() {return m_pTile->GetScale();}
  void SetScale(const CL_Vector2 &vScale);
  CL_Vector2 GetCollisionScale();
  CL_Rectf GetCollisionRect();
  float GetBoundingCollisionRadius();
  bool IsValidPosition(World *pMap, const CL_Vector2 &pos, bool bIgnoreLivingCreatures);
  void Stop();
  void SetCollisionScale(const CL_Vector2 &vScale);
  void ApplyGenericMovement(float step);
  tile_list & GetNearbyTileList(); //note, this may not be valid if you're deleting tiles!  For debug only
  CL_Rectf & GetLastScanArea(){return m_scanArea;}
  bool IsAtRest() {return m_bIsAtRest;}
  bool IsOnGround() {return m_groundTimer > GetApp()->GetGameTick();}
  luabind::object RunFunction(const string &func);
  luabind::object RunFunction(const string &func, luabind::object obj1);
  luabind::object RunFunction(const string &func, luabind::object obj1, luabind::object obj2);
  luabind::object RunFunction(const string &func, luabind::object obj1, luabind::object obj2, luabind::object obj3);
  virtual CL_Rectf GetWorldRect();
  const CL_Rect & GetBoundsRect();
  void GetAlignment(CL_Origin &origin, int &x, int &y);
  void SetAlignment(int origin, CL_Vector2 v);
  void UpdateTilePosition();
  CollisionData * GetCollisionData() {return m_pCollisionData;}
  bool SetVisualProfile(const string &resourceFileName, const string &profileName);
  virtual void  Update(float step);
  virtual void  Render(void *pTarget);
  void RenderCollisionLists(CL_GraphicContext *pGC);
  void RenderShadow(void *pTarget);
  BaseGameEntity * CreateClone(TileEntity *pTile);
  void SetIsOnGround(bool bOnGround);
  CBody * GetBody(){return &m_body;}
  void SetSpriteData(CL_Sprite *pSprite);
 // void SetImageClipRect(const CL_Rect &r);
  CL_Rect GetImageClipRect();
  unsigned int GetImageID(); //returns 0 if not applicable
  bool SetImageByID(unsigned int picID, CL_Rect *pSrcRect);
  void SetImage(const string &fileName, CL_Rect *pSrcRect);

  bool LoadScript(const char *pFileName);
  void SetMainScriptFileName(const string &fileName);
  const string & GetMainScriptFileName() {return m_mainScript;}
  int GetNavNodeType(){return m_navNodeType;}
  void SetNavNodeType(int n);
  void SetHasPathNode(bool bHasNode);

  VisualProfile * GetVisualProfile() {return m_pVisualProfile;}
  void Serialize(CL_FileHelper &helper);
  void LoadCollisionInfo(const string &fileName);
  void SetCollisionInfoFromPic(unsigned int picID, const CL_Rect &recPic);
  void SetAnimByName(const string &name);
  void EnableRotation(bool bRotate);
  bool GetEnableRotation();
  unsigned int GetDrawID() {return m_drawID;}
  float GetMass() {return m_body.GetMass();}
  void SetMass(float mass){m_body.SetMass(mass);}
  void SetDensity(float fDensity);
  void PostUpdate(float step);
  void SetListenCollision(int eListen);
  int GetListenCollision() {return m_listenCollision;}
  void SetAnimFrame(int frame);
  int GetAnimFrame();
  void SetListenCollisionStatic(int eListen) {m_listenCollisionStatic = eListen;}
  int GetListenCollisionStatic() {return m_listenCollisionStatic;}
  void AddForce(CL_Vector2 force);
  void AddForceAndTorque(CL_Vector2 force, CL_Vector2 torque);
  CL_Vector2 GetForce() {return *(CL_Vector2*)&m_body.GetNetForce();};
  CL_Vector2 GetLinearVelocity() {return *(CL_Vector2*)&m_body.GetLinVelocity();}
  void SetPersistent(bool bOn){assert(m_pTile); m_pTile->SetBit(Tile::e_notPersistent, !bOn);}
  bool GetPersistent() {assert(m_pTile); return !m_pTile->GetBit(Tile::e_notPersistent);}
  float GetDistanceFromEntityByID(int id);
  float GetDistanceFromPosition(const CL_Vector2 &pos);
  MovingEntity * CreateEntity(CL_Vector2 pos, const string &script);

  int GetLayerID();
  void SetLayerID(int id);
  void SetLayerByName(const string &name);

  
  ScriptObject * GetScriptObject() {return m_pScriptObject;}
  Zone * GetZoneWeAreOnByMaterialType(int matType);
  Zone * GetNearbyZoneByPointAndType(const CL_Vector2 &vPos, int matType);
  Zone * GetNearbyZoneByCollisionRectAndType(int matType);

  bool InZoneByMaterialType(int matType) {return GetZoneWeAreOnByMaterialType(matType) != NULL;}
  bool GetOnLadder() {return m_bOnLadder;}
  void SetOnLadder(bool bOnLadder) {m_bOnLadder = bOnLadder;}
  CL_Sprite * GetSprite(){return m_pSprite;}
  int GetAnimID() {return m_animID;}
  void SetAnimPause(bool bPause) { m_bAnimPaused = bPause;}
  bool GetAnimPause() {return m_bAnimPaused;}
  
  virtual void HandleMessageString(const string &msg);
  virtual bool HandleMessage(const Message &msg);

  void SetDefaultTextColor(CL_Color color) {m_defaultTextColor = color;}
  CL_Color GetDefaultTextColor() {return m_defaultTextColor;}
  DataManager * GetData() {return &m_dataManager;}
  unsigned int GetNameHash() {return m_hashedName;}
  bool SetPosAndMapByTagName(const string &name);
  void SetImageFromTilePic(TilePic *pTilePic);
  bool GetUsingImageFromTilePic() {return m_bUsingSimpleSprite;}
  bool UsingCustomCollisionData() {return m_bUsingCustomCollisionData;}
  int GetFloorMaterialID() {return m_floorMaterialID;}
  BrainManager * GetBrainManager() {return &m_brainManager;}
  string ProcessPath(const string &st); //replaces ~ with current script path
  string ProcessPathNoScript(const string &st); //doesn't add the "script" part

  void OnDamage(const CL_Vector2 &normal, float depth, MovingEntity * enemy, int damage, int uservar, MovingEntity *pProjectile);
  void SetFacing(int facing);
  void SetFacingTarget(int facing);
  int GetFacing(){return m_facing;}
  void SetVectorFacing(const CL_Vector2 &v);
  void SetVectorFacingTarget(const CL_Vector2 &v);
  int GetFacingTarget();

  CL_Vector2 GetVectorFacing();
  CL_Vector2 GetVectorFacingTarget();
 
  unsigned int CalculateTimeToReachPosition(const CL_Vector2 &pos);


  int GetVisualState() {return m_visualState;}
  void SetVisualState(int visualState);
  void SetSpriteByVisualStateAndFacing();
  void LastCollisionWasInvalidated();

  void AddColorModRed(short mod) {m_colorModRed += mod;}
  void AddColorModGreen(short mod) {m_colorModGreen += mod;}
  void AddColorModBlue(short mod) {m_colorModBlue += mod;}
  void AddColorModAlpha(short mod) {m_colorModAlpha += mod;}

  void SetBaseColor(const CL_Color &col){m_pTile->SetColor(col);}
  CL_Color GetBaseColor() {return m_pTile->GetColor();}
  void SetTrigger(int triggerType, int typeVar, int triggerBehavior, int behaviorVar);
  void ClearColorMods();
  void DumpScriptInfo();
  CL_Vector2 GetVisualOffset();
  void RotateTowardsVectorDirection(const CL_Vector2 &vecTargetfloat, float maxTurn);
  void SetCollisionMode(int mode);
  void UpdateTriggers(float step);
  Goal_Think * GetGoalManager();
  bool IsGoalManagerActive() {return m_pGoalManager != NULL;}
  //normally you'd never use this, but during load I need this to disable the update...
  void SetMovedFlag(bool bNeedToUpdatePosition) {m_bMovedFlag = bNeedToUpdatePosition;}
  void ForceSpriteUpdate() {m_pSpriteLastUsed = NULL;}
  float GetDesiredSpeed() {return m_desiredSpeed;}
  void SetDesiredSpeed(float s) {m_desiredSpeed = s;}

  float GetMaxWalkSpeed() {return m_maxWalkSpeed;}
  void SetMaxWalkSpeed(float s) {m_maxWalkSpeed = s;}
  bool IsPlaced() {return m_pTile->GetParentScreen() != 0;}
  bool IsInitted() {return m_bHasRunOnInit;}
  bool IsFacingTarget(float tolerance); 

  bool IsCloseToEntityByID(int entID, int dist);
  bool IsCloseToEntity(MovingEntity *pEnt, int dist);

  bool CanWalkTo(CL_Vector2 &pos, bool ignoreLivingCreatures);
  bool CanWalkBetween(World *pMap, CL_Vector2 from, CL_Vector2 to, bool ignoreLivingCreatures);

  CL_Vector2 GetVectorToEntityByID(int entID);
  CL_Vector2 GetVectorToEntity(MovingEntity *pEnt);
  CL_Vector2 GetVectorToPosition(const CL_Vector2 &pos);

  bool IsOnSameMapAsEntityByID(int entID);

  PathPlanner * GetPathPlanner();
  void SetRunStringASAP(const string &command);
  void ProcessPendingMoveAndDeletionOperations();
  World * GetMap();
  void RunPostInitIfNeeded();
  void RunOnMapInsertIfNeeded();
  void CheckVisibilityNotifications(unsigned int notificationID);
  bool GetVisibilityNotifications() {return m_bRequestsVisibilityNotifications;}
  void SetVisibilityNotifications(bool bNew) {m_bRequestsVisibilityNotifications = bNew;}
  void OnWatchListTimeout(bool bIsOnScreen);
  int PlaySoundPositioned(const string &fName);
  int PlaySound(const string &fName);
  void SetText(const string &text);
  string GetText() {return m_text;}
  CL_Vector2 GetTextBounds();
 
  void SetTextColor(CL_Color color);
  CL_Color GetTextColor();
  void SetTextAlignment(int alignment, CL_Vector2 v);

  void SetTextScale(const CL_Vector2 &vecScale);
  CL_Vector2 GetTextScale();
  void SetTextRect(const CL_Rect &r);
  void SetAttach(int entityID, CL_Vector2 vOffset);
  int GetAttachEntityID() {return m_attachEntID;}
  CL_Vector2 GetAttachOffset() {return m_attachOffset;}
  void SetAttachOffset(const CL_Vector2 &pos) {m_attachOffset = pos;}
  MovingEntity * Clone(World *pMap, CL_Vector2 vecPos);
  void OnAttachedEntity(int entID);
  void SetRunUpdateEveryFrame(bool bNew) {m_bRunUpdateEveryFrame = bNew;}
  bool GetRunUpdateEveryFrame() {return m_bRunUpdateEveryFrame;}
  void SetDampening(float f) {m_customDampening = f;}
  bool FunctionExists(const char * pFunctionName);
  void SetTurnSpeed(float fNew) {m_turnSpeed = fNew;}
  float GetTurnSpeed() {return m_turnSpeed;}
  enum ListenCollision
{
	//I know I don't have to specify the #'s but it helps me visually keep
	//them syncronized with the lua defines that have to match. So don't
	//be changin' the order ya hear?
	
	LISTEN_COLLISION_NONE = 0,
	LISTEN_COLLISION_PLAYER_ONLY = 1,
	LISTEN_COLLISION_ALL_ENTITIES = 2,

	//add more above this
	LISTEN_COLLISION_COUNT
};

enum ListenStaticCollision
{
	//I know I don't have to specify the #'s but it helps me visually keep
	//them syncronized with the lua defines that have to match. So don't
	//be changin' the order ya hear?

	LISTEN_COLLISION_STATIC_NONE = 0,
	LISTEN_COLLISION_STATIC_ALL = 1,

	//add more above this
	LISTEN_COLLISION_STATIC_COUNT
};

enum  CollisionMode
{

	COLLISION_MODE_ALL = 0,
	COLLISION_MODE_NONE,
	COLLISION_MODE_STATIC_ONLY,

	COLLISION_MODE_COUNT

};

protected:

	void SetDefaults();

	void ProcessCollisionTileList(tile_list &tList, float step);
	void ProcessCollisionTile(Tile *pTile, float step);
	void OnCollision(const Vector & N, float &t, CBody *pOtherBody, bool *pBoolAllowCollide); 
	void RotateTowardsFacingTarget(float step);
	void SetIsOnScreen(bool bNew);

	CL_Rectf m_scanArea;
	tile_list m_nearbyTileList;
    CL_Slot m_collisionSlot;
	CBody m_body; //physics object
	bool m_bIsAtRest;
	bool m_bMovedFlag; //if true, we need to update our tile position
	unsigned int m_groundTimer; //relax when we say they are on the  ground or not to take care of tiny bounces
	CollisionData * m_pCollisionData; //null if we don't collide
	ScriptObject *m_pScriptObject;
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
	bool m_bOldTouchedAGroundThisFrame;
	int m_oldFloorMaterialID;
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
	BrainManager m_brainManager;
	int m_facing;  //may or may not be used
	int m_visualState; //may or may not be used
	CL_Vector2 m_moveToAtEndOfFrame;
	
	short m_colorModRed;
	short m_colorModGreen;
	short m_colorModBlue;
	short m_colorModAlpha;

	CL_Vector2 m_vecFacing;
	CL_Vector2 m_vecFacingTarget;

	Trigger m_trigger;
	bool m_bRestartAnim;
	int m_collisionMode;
	int m_animID;

	Goal_Think *m_pGoalManager;
	float m_desiredSpeed;
	float m_maxWalkSpeed;
	PathPlanner * m_pPathPlanner;
	string m_runScriptASAP; //empty if not being used, only used for the RunScript Goal
	int m_navNodeType; //only used if this entity has a pathnode, allows special markings for the graph, like doors or warps

	bool m_bHasRunOnInit; //run before inserted in map
	bool m_bHasRunPostInit; //run during map insertion, for handling warps mostly
	bool m_bHasRunOnMapInsert; //run in the entities think phase, never during a large map load

	unsigned int m_lastVisibilityNotificationID;
	bool m_bRequestsVisibilityNotifications;
	bool m_bOnScreen;
	bool m_bCanRotate;
	int m_requestNewLayerID; //-1 if none requested

	//stuff that may or may not be used for text in this entity
	string m_text; //simple way to show text in  the game
	CL_Font * m_pFont;
	CL_Rect m_textRect;

	int m_attachEntID;
	CL_Vector2 m_attachOffset;
	bool m_bLockedScale; //if true, we don't scale with the camera
	list<int> m_attachedEntities; 
	bool m_bRunUpdateEveryFrame;
	float m_customDampening;
	float m_turnSpeed;

};

MovingEntity * CreateEntity(CL_Vector2 vecPos, string scriptFileName); //creates an entity and puts it in the world

void AddShadowToParam1(CL_Surface_DrawParams1 &params1, Tile *pTile);


#endif
