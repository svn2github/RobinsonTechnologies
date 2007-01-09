
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
	  void ShowLoadingMessage();
	  void ResetLastUpdateTimer() { m_lastUpdateTime = CL_System::get_time();} //resets how many MS we've been stuck in a function
	  unsigned int GetTimeSinceLastUpdateMS() {return CL_System::get_time()-m_lastUpdateTime;}  //how many MS we've been stuck in a function
	  bool IsEditorDialogOpen();

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
	unsigned int m_lastUpdateTime;

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

//lua doc info for natural docs processor.  Be careful how you move it, it will break..
/*

Object: GameLogic

Contains a lot of general game-related functions.

About:
This is automatically created by the engine and always available.  You can access it this way:

(code)
GetGameLogic:RunSomeCoolFunctionInIt()
(end)

Group: Member Functions

*/


/*
func: ToggleEditMode

(code)
boolean ToggleEditMode()
(end)

Returns:
True if the it just turned ON the editor, false if it was just turned off
*/


/*
func: SetUserProfileName
(code)
boolean SetUserProfileName(string name)
(end)

Re-initializes everything and attempts to load the profile sent in.  Think of this as loading a saved game.  If no profile exists, it is created.

Dangerous and illegal characters are stripped automatically, so the profile name may be slightly different than the one you sent in.

Parameters:

name - the name of the profile.  If you say, "Player", it will create/load/save profiles/Player/(active world name).

Returns:
False on error (for instance, the name passed in is not a valid path, or tries to access somewhere illegal), true on success
*/

/*
func: GetUserProfileName
(code)
string GetUserProfileName()
(end)

Returns:
The active user profile name. Will return a blank string if empty. (ie, "", not nil)
*/

/*
func: ResetUserProfile
(code)
nil ResetUserProfile(string name)
(end)

This completely deletes a profile, if it existed.

Input is stripped of dangerous/illegal characters.

Parameters:

name - The profile name you want to delete.

Note:

Novashell never uses a dangerous deltree type way to delete things, it carefully verifies and scans the directory and only deletes only valid novashell data.

*/

/*
func: UserProfileExists
(code)
boolean UserProfileExists(string name)
(end)

Parameters:

name - The profile name you want to check for.  Is stripped for dangerous/illegal characters.

Returns:

True if the profile exists.
*/

/*
func: UserProfileActive
(code)
boolean UserProfileActive()
(end)

Returns:

True if a user profile is currently loaded/active.
*/


/*
func: SetRestartEngineFlag
(code)
nil SetRestartEngineFlage(boolean bRestart)
(end)

Set this to true and the engine will save all modified data and restart as soon as cybernetic ally possible.
*/

/*
func: ClearModPaths
(code)
nil ClearModPaths()
(end)

Removes all overlaid paths.  This should be done right before doing a SetRestartEngineFlag() call.  Allows you to re-mount directories or go back to the world selection dialog. (that's default if no mods are mounted)
*/

/*
func: AddModPath
(code)
nil AddModPath(string pathName)
(end)

This lets you mount a world path.  Scripts, images, and resources are loaded in reverse order of mounting, ie, the last thing you mounted gets checked first.

Note:
"Base" is always mounted first automatically.

Worlds are also automatically mounted if specified in its .novashell configuration file.

Parameters:

pathName - A name of a world to mount.  Example: "Test World".  
*/

/*
func: InitGameGUI
(code)
nil InitGameGUI(string xmlFileName)
(end)

This xml file contains data about fonts, sizes, and colors that should be used for game dialogs and menus.

By editing this, you can use your own custom bitmap fonts.

Check the default one in base/game_gui/standard.xml for an example.

Note:

A world can only initialize this once.

Parameters:

xmlFileName - A path and filename to an xml file.
*/

/*
func: Quit
(code)
nil Quit()
(end)

Immediately closes the application.  Changed game data is auto-saved.
*/
