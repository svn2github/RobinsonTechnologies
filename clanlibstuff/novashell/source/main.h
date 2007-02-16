
/* -------------------------------------------------
* Copyright 2006 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
*/


#ifndef main_HEADER_INCLUDED // include guard
#define main_HEADER_INCLUDED  // include guard

//stuff for the lua docs

/*
Section: Overview

Novashell  Game Creation System Scripting API Reference:

Wow, that's a long title.  

Lua and you:

Novashell uses Lua 5.1 under the hood.  You can use google to find good tutorials on how it works.

Novashell specific Lua additions/changes:

* // C++ style comments are supported
* != (inequality) operator is supported
* dofile, load, and other things deemed "dangerous" are missing. dofile was replaced with "RunScript".
* Many new object types, as detailed in these docs

How this API is organized:

System Objects - These are global single-instance objects that always exist that you can access at any time.
Game Objects - A game can have an unlimited number of these types of things. (maps, entities, etc)
Simple Objects - Smaller data structures that can be used anywhere.
Global Functions - These are functions not associated with an object that can be used from anywhere.


(*Click the section name on the sidebar to open/close the section*)


Links:

Seth A. Robinson (seth@rtsoft.com)

Novashell home <http://www.rtsoft.com/novashell>

Novashell Getting Starting tutorials <http://www.rtsoft.com/novashell/docs>

*/



#define C_BASE_MAP_PATH "maps"

enum
{
  C_FONT_GRAY, //it's tiny
  C_FONT_NORMAL, //bigger
      C_FONT_COUNT
};

#include <iostream>
#include <fstream>

#include "misc/ISoundManager.h"

extern ISoundManager *g_pSoundManager;

enum
{
	C_PLATFORM_WINDOWS = 0,
	C_PLATFORM_OSX,
	C_PLATFORM_LINUX
};

//this stream_redirector taken from a snippet taken from Gabriel Fleseriu's post on codeguru

#define C_LOGGING_BUFFER_SIZE (4096*3)


class stream_redirector
{
public:
	stream_redirector(char *log, char *error):
	  logFile(log), errFile(error)
	  {
		  outbuf = std::cout.rdbuf(logFile.rdbuf());
		  errbuf = std::cerr.rdbuf(errFile.rdbuf());
	  }
	 
	
    void DisableRedirection()
	{
		std::cout.rdbuf(outbuf);
		std::cerr.rdbuf(errbuf);
	 	
	}
	
	~stream_redirector()
	  {
		DisableRedirection();
	  }

private:
	std::ofstream logFile;
	std::ofstream errFile;
	std::streambuf *outbuf;
	std::streambuf *errbuf;
};

class GameLogic;
class HashedResourceManager;
class Camera;
class Map;
class ScriptManager;
class VisualProfileManager;

class App : public CL_ClanApplication
{
public:
    App();
    virtual ~App();
    
    virtual int main(int argc, char **argv);
    CL_ResourceManager * GetResourceManager(){return m_pResourceManager;}
    CL_ResourceManager * GetGUIResourceManager(){return m_pGUIResourceManager;}
    CL_DisplayWindow * GetMainWindow(){return m_pWindow;}
	HashedResourceManager * GetMyHashedResourceManager(){return m_pHashedResourceManager;}
	VisualProfileManager * GetMyVisualProfileManager() {return m_pVisualProfileManager;}
	ScriptManager * GetMyScriptManager() {return m_pScriptManager;}
    CL_Font * GetFont(int font_id){return m_pFonts[font_id];}
    void SetupBackground(int x, int y);
    void OnWindowResize(int x, int y);
    CL_Surface * GetBackground() {return m_pBackground;}
    CL_Canvas * GetBackgroundCanvas() { return m_pBackgroundCanvas;}
    
#ifdef WIN32
	HWND GetHWND() {return m_Hwnd;}
#endif
    
	
	void OnWindowClose();
    float GetDelta(){return m_delta;}
    unsigned int GetGameTick() {return m_gameTick;} 
    void ClearTimingAfterLongPause(); //so our delta timer etc doesn't get screwed up
    CL_GUIManager * GetGUI(){return m_pGui;}
    GameLogic * GetMyGameLogic(){return m_pGameLogic;}
	Camera * GetMyCamera() {return m_pCamera;}
	void RequestAppExit();
	void RequestToggleFullscreen();
	unsigned int GetUniqueNumber(); //it will start at 1
	float GetGameSpeed() {return m_baseGameSpeed;}
	float GetGameLogicSpeed() {return m_baseLogicMhz;}
	void SetGameLogicSpeed(float fNew);
	void SetGameSpeed(float fNew);
	void SetGameTick(unsigned int num);
	unsigned int GetTick(){return m_lastFrameTime;}
	bool ParmExists(const string &parm);
	void SetSimulationSpeedMod(float fNew) {m_simulationSpeedMod = fNew;}
	float GetSimulationSpeedMod() {return m_simulationSpeedMod;}
	bool GetJustRenderedFrame() {return m_bJustRenderedFrame;} //true if a frame was JUST rendered.  Reset after the next Update() cycle
	vector<string> & GetStartupParms() {return m_startupParms;}
	float GetEngineVersion() {return m_engineVersion;}
	string GetEngineVersionAsString() {return m_engineVersionString;}
	void SetWindowTitle(const string &title);

	enum eVideoRefresh
	{
		FPS_AT_REFRESH = 1,
		FPS_UNLIMITED = 0
	};
	void SetRefreshType(eVideoRefresh eRefresh) {m_videoflipStyle = eRefresh;}
	eVideoRefresh GetRefreshType() {return m_videoflipStyle;}
	bool ActivateVideoRefresh(bool bFullscreen);
	bool SetScreenSize(int x, int y);
	int GetFPS() {if (m_pFrameRate) return m_pFrameRate->get_fps(); else return 0;};

	bool GetRenderedGameGUI() {return m_bRenderedGameGUI;}
	void SetRenderedGameGUI(bool bNew) {m_bRenderedGameGUI = bNew;}

	void SetFont(int fontID, CL_Font *pFont) {m_pFonts[fontID] = pFont;} //we'll delete it ourselves later
	bool GetRequestedQuit() {return m_bQuit;}
	string GetDefaultTitle();
	void SetCursorVisible(bool bNew);
	bool GetCursorVisible();
	int GetPlatform();

private:
    
    void OneTimeDeinit();
    void OneTimeInit();
    void ToggleWindowedMode();
	void OnRender();
   void UpdateLogic();
    void OnLoseFocus();
    void OnGotFocus();
    void SetupMouseClipping();
    void OnKeyUp(const CL_InputEvent &key);
	void Update();
	void ComputeSpeed();
 
	CL_ResourceManager * m_pResourceManager;
    CL_ResourceManager * m_pGUIResourceManager;
    CL_DisplayWindow * m_pWindow;
    CL_Font *m_pFonts[C_FONT_COUNT];
    CL_Surface *m_pBackground;
    CL_Canvas *m_pBackgroundCanvas; //attached to the background, gives us an interface to its graphical context
	bool m_bWindowResizeRequest; 
	bool m_bQuit;
	bool m_HaveFocus;
	bool m_bClipCursorWhenFullscreen;
#ifdef WIN32
	HWND m_Hwnd; //I can't figure out how to get it from ClanLib so I'll keep my own copy
#endif

	CL_DisplayWindowDescription m_WindowDescription;
	GameLogic *m_pGameLogic;
    float m_delta; //averaged delta
    float m_deltaTarget; //what the actual last delta was
    unsigned int m_lastFrameTime;
    CL_StyleManager_Silver * m_pStyle;
    CL_GUIManager * m_pGui;
	HashedResourceManager * m_pHashedResourceManager;
	VisualProfileManager * m_pVisualProfileManager; 
	ScriptManager *m_pScriptManager;
	Camera *m_pCamera;
	bool m_bRequestToggleFullscreen;
	unsigned int m_uniqueNum;
	unsigned int m_gameTick;
	float m_thinkTicksToUse;
	eVideoRefresh m_videoflipStyle; //-1 is limited to refresh, 0 is not.  -2 causes problems
	float m_baseGameSpeed; //faster and we move faster, computed against baselogicmhz
	float m_baseLogicMhz; //MS between thinks
	vector<string> m_startupParms;
	float m_simulationSpeedMod;
	bool m_bJustRenderedFrame;
	bool m_bRequestVideoInit;
	float m_engineVersion;
	string m_engineVersionString;
	bool m_bRenderedGameGUI; //used for some complicated GUI trickery to work with model dialog boxes
	CL_FramerateCounter *m_pFrameRate;
	bool m_bCursorVisible;
	
};

extern App MyApp;

App * GetApp();
void LogMsg(const char *lpFormat, ...);

#define GetScreenX (GetApp()->GetMainWindow()->get_width())
#define GetScreenY (GetApp()->GetMainWindow()->get_height())
#define GetCamera GetApp()->GetMyCamera()
#define GetGameLogic GetApp()->GetMyGameLogic()
#define GetHashedResourceManager GetApp()->GetMyHashedResourceManager()
#define GetScriptManager GetApp()->GetMyScriptManager()
#define GetVisualProfileManager GetApp()->GetMyVisualProfileManager()

#endif                  // include guard
