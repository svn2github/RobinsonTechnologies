
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

class EntWorldCache;
class EntEditor;
class MessageManager;
class CL_VirtualFileManager;

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
	  bool GetGamePaused() {return m_bGamePaused;}
	  void SetGamePaused(bool bNew) {m_bGamePaused = bNew;}
	  void SetEditorActive(bool bNew) {m_bEditorActive = bNew;}
	  bool GetEditorActive() {return m_bEditorActive;}
	  void SetParallaxActive(bool bNew) {m_bParallaxActive = bNew;}
	  bool GetParallaxActive() {return m_bParallaxActive;}
	  const string & GetScriptRootDir() {return m_strScriptRootDir;}
	  void SetLeftMouseButtonCallback(const string &luaFunctionName);
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

private:

	void OnMouseUp(const CL_InputEvent &key);
	void Zoom(bool zoomCloser);

    void OnKeyDown(const CL_InputEvent &key);
	void OnKeyUp(const CL_InputEvent &key);
    MyEntityManager m_myEntityManager;
    CL_SlotContainer m_slots;

    int m_editorID; //0 if none
	MovingEntity *m_pPlayer;
	string m_strBaseUserProfilePath; //the path without the profile at the end
	string m_strBaseMapPath; //contains something like "media/maps/" (includes trailing backslash)
	string m_strUserProfileName; //blank if none (empty if none)
	string m_strUserProfilePathWithName; //the path plus the name (empty if none)
	WorldManager m_worldManager; //holds all our world data 
	bool m_bShowEntityCollisionData;
	bool m_bGamePaused;
	bool m_bEditorActive;
	bool m_bParallaxActive;
	bool m_bMakingThumbnail; //if true we're in the middle of making a thumbnail
	string m_strScriptRootDir;
	string m_leftMouseButtonCallback; //name of the lua function to call
	DataManager m_data; //to store global variables controlled by Lua
	bool m_bShowingMessageWindow;
	bool m_bShowFPS;
	bool m_bRestartEngineFlag;
	
};

void MovePlayerToCamera();
void MoveCameraToPlayer();
void SetCameraToTrackPlayer();
void ShowMessage(string title, string msg);

extern TagManager g_TagManager;
extern CL_VirtualFileManager g_VFManager;

#define GetWorld GetApp()->GetMyGameLogic()->GetMyWorldManager()->GetActiveWorld()
#define GetWorldCache GetApp()->GetMyGameLogic()->GetMyWorldManager()->GetActiveWorldCache()
#define GetPlayer GetApp()->GetMyGameLogic()->GetMyPlayer()
#define GetWorldManager GetApp()->GetMyGameLogic()->GetMyWorldManager()
#define GetTagManager (&g_TagManager)

extern MessageManager g_MessageManager; //I should make the class a singleton. uh.. 

void Schedule(unsigned int deliveryMS, unsigned int targetID, const char * pMsg);
void ScheduleSystem(unsigned int deliveryMS, const char * pMsg);
