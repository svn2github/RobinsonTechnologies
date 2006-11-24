
/* -------------------------------------------------
* Copyright 2006 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
*/

#pragma once

#include "MyEntityManager.h"
#include "EntityManager.h"
#include "World.h"
#include "WorldManager.h"
#include "AppUtils.h"
#include "ScriptManager.h"
#include "Camera.h"
#include "HashedResourceManager.h"
#include "TagManager.h"
#include "MessageManager.h"
#include "DataManager.h"
#include "GUIStyleBitmap/stylemanager_Bitmap.h"

class EntWorldCache;
class EntEditor;
class MessageManager;
class CL_VirtualFileManager;

#define C_WORLD_INFO_EXTENSION "novashell"

class GameLogic
{

public:

    GameLogic();
    ~GameLogic();

      bool Init();
      
	  //note, I use GetMyPlayer instead of GetPlayer because otherwise VS gets confused with its
	  //intellisense parsing of the GetPlayer() #define below that I use to access it, same goes
	  //for other things I have macros for

      void Update(float step);
      void Render();
      void RebuildBuffers();
      MyEntityManager * GetMyEntityManager() {return &m_myEntityManager;}
	  void SaveGlobals();
	  void LoadGlobals();

      void Kill();
	  void SetMyPlayer(MovingEntity * pNew);
	  MovingEntity * GetMyPlayer() {return m_pPlayer;}
	  const string & GetBaseMapPath() {return m_strBaseMapPath;}
	  WorldManager * GetMyWorldManager() {return &m_worldManager;}
	  void ClearScreen();
	  void SetShowEntityCollisionData(bool bNew) {m_bShowEntityCollisionData = bNew;}
	  bool GetShowEntityCollisionData() {return m_bShowEntityCollisionData;}
	  bool GetGamePaused() {return m_GamePaused != 0;}
	  int SetGamePaused(bool bNew);
	  void SetEditorActive(bool bNew) {m_bEditorActive = bNew;}
	  bool GetEditorActive() {return m_bEditorActive;}
	  void SetParallaxActive(bool bNew) {m_bParallaxActive = bNew;}
	  bool GetParallaxActive() {return m_bParallaxActive;}
	  const string & GetScriptRootDir() {return m_strScriptRootDir;}
	  bool ToggleEditMode(); //returns true if it was turned on, false if it was turned off
	  void SetMakingThumbnail(bool bNew) {m_bMakingThumbnail = bNew;}
	  bool GetMakingThumbnail() {return m_bMakingThumbnail;}
	  bool SetUserProfileName(const string &name); //returns false if it's an invalid or illegal profile name
	  void ResetUserProfile(string name);
	  const string & GetUserProfilePathWithName() {return m_strUserProfilePathWithName;}
	  const string & GetUserProfileName() {return m_strUserProfileName;}
	  bool UserProfileActive() {return !m_strUserProfileName.empty();}
	  bool UserProfileExists(const string &name);
	  void HandleMessageString(const string &msg);
	  void ClearAllMapsFromMemory();
	  DataManager * Data() {return &m_data;}
	  void SetShowMessageActive(bool bNew) {m_bShowingMessageWindow = bNew;}
	  bool GetShowMessageActive() {return m_bShowingMessageWindow;}
	  void SetShowFPS(bool bNew) {m_bShowFPS = bNew;}
	  bool GetShowFPS() {return m_bShowFPS;}
	  void ToggleShowFPS() {m_bShowFPS = !m_bShowFPS;}
	  void SetRestartEngineFlag(bool bNew) { 			m_bRestartEngineFlag = bNew;}
	  void Quit() {GetApp()->OnWindowClose();}
	  void OnPlayerDeleted(int id);
	  bool GetShowPathfinding() {return m_bShowPathfinding;}
	  void SetShowPathfinding(bool bNew) {m_bShowPathfinding = bNew;}
	  bool GetShowAI() {return m_bShowAI;}
	  void SetShowAI(bool bNew) {m_bShowAI = bNew;}
	  void ClearModPaths() {m_modPaths.clear(); GetApp()->SetWindowTitle(GetApp()->GetDefaultTitle());}
	  void AddModPath(string s);
	  const string & GetWorldsDirPath() {return m_strWorldsDirPath;}
	  const string & GetActiveWorldName();
	  void InitGameGUI(string xmlFile); //don't make this const, we modify it in place
		void OneTimeModSetup();
	  CL_StyleManager_Bitmap * GetGUIStyle() {return m_pGUIStyle;}
	  CL_GUIManager * GetGameGUI() {return m_pGUIManager;}
	  void RequestRebuildCacheData();
	  
	  //setting the game mode right helps the game guess more accurately how gravity, physics and dynamic shadows should work.
	  //But its reliance on this var should be as minimum as possible.
	  
	  int GetGameMode() {return m_gameMode;}
	  void SetGameMode(int gameMode);

	  enum
	  {
		  C_GAME_MODE_SIDE_VIEW,
		  C_GAME_MODE_TOP_VIEW,

		  C_GAME_MODE_COUNT
	  };

private:

	void OnMouseUp(const CL_InputEvent &key);
	void OnMouseDown(const CL_InputEvent &key);

	void Zoom(bool zoomCloser);
	void OnRender();
    void OnKeyDown(const CL_InputEvent &key);
	void OnKeyUp(const CL_InputEvent &key);
	void RenderGameGUI(bool bDrawMainGUIToo);   
	void DeleteAllCacheFiles();

	MyEntityManager m_myEntityManager;
    CL_SlotContainer m_slots;

    int m_editorID; //0 if none
	MovingEntity *m_pPlayer;
	string m_strBaseUserProfilePath; //the path without the profile at the end
	string m_strBaseMapPath; //contains something like "maps/" (includes trailing backslash)
	string m_strUserProfileName; //blank if none (empty if none)
	string m_strUserProfilePathWithName; //the path plus the name (empty if none)
	WorldManager m_worldManager; //holds all our world data 
	bool m_bShowEntityCollisionData;
	int m_GamePaused; //0 if not paused, 3 if paused three layers deep
	bool m_bEditorActive;
	bool m_bParallaxActive;
	bool m_bMakingThumbnail; //if true we're in the middle of making a thumbnail
	string m_strScriptRootDir;
	string m_leftMouseButtonCallback; //name of the lua function to call
	DataManager m_data; //to store global variables controlled by Lua
	bool m_bShowingMessageWindow;
	bool m_bShowFPS;
	bool m_bRestartEngineFlag;
	int m_gameMode;
	CL_Slot m_playerDestroyedSlot;
	bool m_bShowPathfinding;
	bool m_bShowAI;
	vector<string> m_modPaths; //mount order is important
	string m_activeWorldName;
	string m_strWorldsDirPath;
	bool m_bRebuildCacheData;
	string m_rebuildUserName;

	CL_ResourceManager * m_pGUIResources;
	CL_StyleManager_Bitmap * m_pGUIStyle;
	CL_GUIManager *m_pGUIManager;
};

void MovePlayerToCamera();
void MoveCameraToPlayer();
void SetCameraToTrackPlayer();
void ShowMessage(string title, string msg, bool bForceClassicStyle = true);

extern TagManager g_TagManager;
extern CL_VirtualFileManager g_VFManager;

#define GetWorld GetApp()->GetMyGameLogic()->GetMyWorldManager()->GetActiveWorld()
#define GetWorldCache GetApp()->GetMyGameLogic()->GetMyWorldManager()->GetActiveWorldCache()
#define GetPlayer GetApp()->GetMyGameLogic()->GetMyPlayer()
#define GetWorldManager GetApp()->GetMyGameLogic()->GetMyWorldManager()
#define GetTagManager (&g_TagManager)

extern MessageManager g_MessageManager; //I should make the class a singleton. uh.. 

void Schedule(unsigned int deliveryMS, unsigned int targetID, const char * pMsg);
void ScheduleSystem(unsigned int deliveryMS, unsigned int targetID, const char * pMsg);
bool RunGlobalScriptFromTopMountedDir(const char *pName);
