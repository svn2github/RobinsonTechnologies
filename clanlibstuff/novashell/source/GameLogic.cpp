#include "AppPrecomp.h"
#include "GameLogic.h"
#include "main.h"
#include "EntEditor.h"
#include "EntWorldCache.h"
#include "EntChooseScreenMode.h"
#include "AppUtils.h"
#include "TileEntity.h"
#include "EntEditMode.h"
#include "luabindBindings.h"
#include "MovingEntity.h"
#include "TextManager.h"
#include "Console.h"
#include "MaterialManager.h"
#include "VisualProfileManager.h"
#include "AI/WatchManager.h"
#include "AI/WorldNavManager.h"
#include "AI/WatchManager.h"
#include "GUIStyleBitmap/NS_MessageBox.h"
#include "EntWorldDialog.h"

#ifndef WIN32
//windows already has this in the precompiled header for speed, I couldn't get that to work on mac..
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#endif

#define C_PROFILE_DAT_FILENAME "profile.dat"


const unsigned int C_PROFILE_DAT_VERSION = 0;

CL_VirtualFileManager g_VFManager;

MessageManager g_MessageManager; 

TagManager g_TagManager;

GameLogic::GameLogic()
{
	m_pGUIResources = NULL;
	m_pGUIStyle = NULL;
	m_pPlayer = NULL;   
	m_pGUIManager= NULL;

	m_slots.connect(CL_Keyboard::sig_key_down(), this, &GameLogic::OnKeyDown);
	m_slots.connect(CL_Keyboard::sig_key_up(), this, &GameLogic::OnKeyUp);
	m_slots.connect(CL_Mouse::sig_key_up(), this, &GameLogic::OnMouseUp);
	m_slots.connect(CL_Mouse::sig_key_down(), this, &GameLogic::OnMouseDown);
    m_editorID = 0;
	m_bRestartEngineFlag = false;
	m_bShowEntityCollisionData = false;
	m_GamePaused = 0;
	m_bEditorActive = false;
	m_bParallaxActive = true;
	m_bMakingThumbnail = false;
	SetShowMessageActive(false);
	m_strBaseMapPath = C_BASE_MAP_PATH+ string("/");
	m_strScriptRootDir = "script";

	m_bRebuildCacheData = false;
	m_strWorldsDirPath = "worlds";

	CL_Directory::create(m_strBaseUserProfilePath);
	SetShowFPS(false);
	SetShowPathfinding(false);
	SetShowAI(false);
	SetGameMode(C_GAME_MODE_TOP_VIEW); //default.  Also is automatically changed when player brains are initted.
	m_activeWorldName = "base";
}

void GameLogic::OneTimeModSetup()
{
	if (GetApp()->GetStartupParms().size() > 0)
	{
		LogMsg("Command line parms received:");
	}

	for (unsigned int i=0; i < GetApp()->GetStartupParms().size(); i++)
	{

		LogMsg("	%s", GetApp()->GetStartupParms().at(i).c_str());
		string p1 = GetApp()->GetStartupParms().at(i);
		string p2;
		if (i+1 < GetApp()->GetStartupParms().size())
		{
			//this second part might be needed		
			p2 = GetApp()->GetStartupParms().at(i+1);
		}

		if (CL_String::compare_nocase(p1, "-worldpath")  )
		{
			if (p2.empty())
			{
				LogError("Ignored -worldpath in parms, no second part was specified");
			} else
			{
				m_strWorldsDirPath = p2;
			}
		}

		if (CL_String::get_extension(p1) == C_WORLD_INFO_EXTENSION)
		{
			m_strWorldsDirPath = CL_String::get_path(p1);
			SetupModPathsFromWorldInfo(p1);
		}

	}

}

void GameLogic::RequestRebuildCacheData()
{
	SetRestartEngineFlag(true);
	m_bRebuildCacheData = true;
	m_rebuildUserName = GetUserProfileName();
}

GameLogic::~GameLogic()
{
	Kill();
}

int GameLogic::SetGamePaused(bool bNew)
{
	if (bNew)
	{
		m_GamePaused++;
	} else
	{
		m_GamePaused--;
	}

	m_GamePaused = max(0, m_GamePaused);
	
	return m_GamePaused;
}

void GameLogic::SaveGlobals()
{
	if (UserProfileActive())
	{
		m_data.Set("gameTick", CL_String::from_int(GetApp()->GetGameTick()));

		//save out our globals
		CL_OutputSource *pSource = g_VFManager.PutFile(C_PROFILE_DAT_FILENAME);
		CL_FileHelper helper(pSource);

		helper.process_const(C_PROFILE_DAT_VERSION);
		m_data.Serialize(helper);
		SAFE_DELETE(pSource);
	}
}

void GameLogic::LoadGlobals()
{

	m_data.Clear();
	
	if (UserProfileActive())
	{
	
		CL_InputSource *pSource = g_VFManager.GetFile(C_PROFILE_DAT_FILENAME);
		if (pSource)
		{
			CL_FileHelper helper(pSource);

			unsigned int version;
			helper.process(version);
			m_data.Serialize(helper);
			SAFE_DELETE(pSource);
			GetApp()->SetGameTick(m_data.GetNum("gameTick"));
			return;
		}
	}

	//if we got here, let's init the defaults
	//init defaults if we couldn't load them
	
	m_data.Set("gameTick", "0");
	GetApp()->SetGameTick(0);

}

void GameLogic::SetGameMode(int gameMode)
{
	if (gameMode < 0 || gameMode > C_GAME_MODE_COUNT)
	{
		LogError("Illegal game mode");
		return;
	}

	m_gameMode = gameMode;
}

void GameLogic::ClearScreen()
{
	if (GetWorld)
	{
		CL_Display::clear(GetWorld->GetBGColor());
	} else
	{
		CL_Display::clear(CL_Color(0,0,0,255));
	}
}


string StripDangerousFromString(const string &name)
{
	string safe;

	for (unsigned int i=0; i < name.size(); i++)
	{
		if (name[i] != '\\'
			&& name[i] != '/'
			&& name[i] != '.'
			)
		{
			safe.push_back(name[i]);
		}
	}
	
	return safe;
}

bool GameLogic::UserProfileExists(const string &name)
{

	string stPath = m_strBaseUserProfilePath + "/" + StripDangerousFromString(name) + "/"+ m_activeWorldName;
	
	//TODO:  Replace with real DoesFileExist function, this one is slow and dumb

	CL_DirectoryScanner scanner;

	if (!scanner.scan(stPath, "*")) return false;
	while (scanner.next())
	{
		if (scanner.get_name() == ".") return true;
	}

	return false;
}

void GameLogic::ResetUserProfile(string name)
{
	name = StripDangerousFromString(name);
	
	//we have to go to the directory and delete everything.  Uh.  this could be potentially very dangerous, so let's
	//walk with care here.

    string stPath = m_strBaseUserProfilePath + "/" + name +"/" +m_activeWorldName+ string("/") + string(C_BASE_MAP_PATH);

	//also delete all

	LogMsg("Resetting!");
	g_textManager.Reset();

	{
		//we want the scanner to deconstruct otherwise we can't remove the path it was working with
		CL_DirectoryScanner scanner;
		if (scanner.scan(stPath, "*"))
		{
			while (scanner.next())
			{
				std::string file = scanner.get_name();
				if (scanner.is_directory() && scanner.get_name() != "." && scanner.get_name() != "..")
				{
					//no underscore at the start, let's show it
					RemoveWorldFiles(stPath+"/"+scanner.get_name()+"/");
					//now remove the directory
					CL_Directory::remove(stPath+"/"+scanner.get_name(), false, false);
				}
			}
		}

	}
	stPath = m_strBaseUserProfilePath + "/" + name + string("/") + m_activeWorldName +"/";

	RemoveFile(stPath+string(C_WORLD_NAV_FILENAME));
	RemoveFile(stPath+string(C_PROFILE_DAT_FILENAME));

	CL_Directory::remove(m_strBaseUserProfilePath + "/" + name + string("/") + m_activeWorldName+"/"+ string(C_BASE_MAP_PATH));
	CL_Directory::remove(m_strBaseUserProfilePath + "/" + name + string("/") + m_activeWorldName);
	m_data.Clear();
}

bool GameLogic::SetUserProfileName(const string &name)
{

	ClearAllMapsFromMemory();

	//first check security stuff
	if ( (name.find_last_of("/\\")) != -1)
	{
		LogMsg("Can't use %s as a profile name",name.c_str());
		return false;
	}

	//first close down any open map files

	m_strUserProfileName = name;
	CL_Directory::create( m_strBaseUserProfilePath); //just in case it wasn't created yet

	CL_Directory::create(m_strBaseUserProfilePath + "/" + m_strUserProfileName);

	m_strUserProfilePathWithName = m_strBaseUserProfilePath + "/" + m_strUserProfileName+"/" + m_activeWorldName;

	//make all dirs that will be needed
	CL_Directory::create(m_strUserProfilePathWithName);
	
	CL_Directory::create(m_strUserProfilePathWithName + "/maps");

	LogMsg("User profile path now %s", m_strUserProfilePathWithName.c_str());
	
	//set as new save/load path, with fallback still on the default path
	g_VFManager.MountDirectoryPath(m_strUserProfilePathWithName);
	m_worldManager.ScanWorlds(m_strBaseMapPath);

	//load globals
	LoadGlobals();	
	return true;
}

bool IsOnReadOnlyDisk()
{
	const char *pfName = "readonlychk.tmp";
	FILE *fp = fopen(pfName, "wb");
	if (!fp)
	{
		return true;
	}

	//successfully opened it, disk is not read only
	fclose(fp);
	RemoveFile(pfName);
	
	return false;
}

void GameLogic::AddModPath(string s)
{
//	m_modPaths.push_back(m_strWorldsDirPath+ "/"+ StripDangerousFromString(s));
	
	if (CL_String::get_extension(s) == C_WORLD_INFO_EXTENSION)
	{
		//convert this into the directory path
		s = s.substr(0, s.size()- (strlen(C_WORLD_INFO_EXTENSION)+1));
	}

	m_modPaths.push_back(s);
}

bool GameLogic::Init()
{

	assert(!GetWorld);
	g_pSoundManager->Init();

	g_VFManager.Reset();

	g_VFManager.MountDirectoryPath(CL_Directory::get_current()+"/base");
	
	m_activeWorldName = "base";
	ClearScreen();
	CL_Display::flip(2); //show it now
	string modInfoFile;
	//now mount any mod paths
	for (unsigned int i=0; i < m_modPaths.size(); i++)
	{
		if (LocateWorldPath(m_modPaths[i], modInfoFile))
		{
			//m_strWorldsDirPath = CL_String::get_path(modInfoFile);
			m_modPaths[i] = modInfoFile;
			LogMsg("Mounting world path %s.", m_modPaths[i].c_str());
			g_VFManager.MountDirectoryPath(m_modPaths[i]);
			m_activeWorldName = CL_String::get_filename(m_modPaths[i]);
		}

	}
	
	m_worldManager.ScanWorlds(m_strBaseMapPath);
	m_pPlayer = NULL;
	//calculate our user profile base path, later, this could be in the windows user dir or whatever is correct
	m_strBaseUserProfilePath = CL_Directory::get_current() + "/profiles";
	 BlitMessage("... loading ...");


	if (IsOnReadOnlyDisk())
	{
		ShowMessage("Hold on!", "You must drag this out of the DMG before running it.\n\n(all prefs and saves are contained inside for easy deletion for now)");
		return false;
	}

	//GetMyEntityManager()->Init();

	g_textManager.Reset();

	if (!GetHashedResourceManager->Init()) throw CL_Error("Error initting hashed resource manager");


	GetScriptManager->Init();
	//bind app specific functions at the global level
	RegisterLuabindBindings(GetScriptManager->GetMainState());
	
	if (! RunGlobalScriptFromTopMountedDir("system/startup.lua"))
	{
		LogError("Unable to locate script file system/startup.lua");
		return false;
	}
	

	if (m_bRebuildCacheData)
	{
		m_bRebuildCacheData = false;

		if (!m_rebuildUserName.empty())
		{
			SetUserProfileName(m_rebuildUserName);
		}
		//load all maps
		CL_Display::clear(CL_Color(0,0,0));
		BlitMessage("... loading all maps, please wait ...", (GetScreenY/2) -30);

		m_worldManager.PreloadAllMaps();

		CL_Display::clear(CL_Color(0,0,0));
		BlitMessage("... linking all navigation graphs ...", (GetScreenY/2) -30);

		//then rebuild node data
		g_worldNavManager.LinkEverything();
		CL_Display::clear(CL_Color(0,0,0));
		BlitMessage("... saving out navgraph and tagcashe data ...", (GetScreenY/2) -30);

		//now save everything, nothing will have changes except tagcache data
		m_worldManager.SaveAllMaps();

	
		SetRestartEngineFlag(true);
		return true;
	}

	
	if (! RunGlobalScriptFromTopMountedDir("game_start.lua"))
	{
		LogError("Unable to locate script file game_start.lua");
		return false;
	}

	return true;
}

bool GameLogic::ToggleEditMode() //returns NULL if it was toggled off, or the address if on
{
	EntEditor * pEnt = (EntEditor *)EntityMgr->GetEntityByName("editor");
	
	if (!pEnt)
	{
		if (m_modPaths.empty())
		{
			//assume they want to edit something in the base
			if (!GetWorld)
			{
				BaseGameEntity *pEnt = EntityMgr->GetEntityByName("ChooseWorldDialog");
				if (pEnt) pEnt->SetDeleteFlag(true);
				GetMyWorldManager()->SetActiveWorldByName("Generic Palette");

			} else
			{
				BaseGameEntity *pEnt = EntityMgr->GetEntityByName("ChooseWorldDialog");
				if (pEnt) pEnt->SetDeleteFlag(true);
			}

		}
		
		if (!GetWorld)
		{
			ShowMessage("Oops", "Can't open world editor, no map is loaded.");
			return NULL;
		} 

		//take the focus off the player
		GetCamera->SetEntTracking(0);
		pEnt = (EntEditor *) m_myEntityManager.Add( new EntEditor);
		return true;
	} 
	
	//guess we need to turn it off
	pEnt->SetDeleteFlag(true);
	return false;
}

void GameLogic::OnPlayerDeleted(int id)
{
	if (m_pPlayer)
	{
		if (m_pPlayer->ID() == id)
		{
			LogMsg("Player was removed?  Be careful.  Use GetGameLogic:SetPlayer(NIL);");
			m_pPlayer = 0;
		}
	}
	
}

void GameLogic::SetMyPlayer(MovingEntity *pNew)
{
	
	if (pNew && !pNew->GetTile()->GetParentScreen())
	{
		LogError("Error, this that doesn't exist on map is trying to say it's a player?");
		return;
	}
	if (m_pPlayer && pNew != m_pPlayer)
	{
		//tell the entity that currently holds the players focus he just lost it
		int newID = 0;
		if (pNew != 0) newID = pNew->ID();
		if (m_pPlayer->GetBrainManager()->GetBrainBase())
		{
			m_pPlayer->GetBrainManager()->SendToBrainBase("lost_player_focus="+CL_String::from_int(newID));
		}
	}
	
	//also let's tell the script about it
	if (pNew)
	{
		m_pPlayer = pNew;
		luabind::globals(GetScriptManager->GetMainState())["g_PlayerID"] = m_pPlayer->ID();

		if (m_pPlayer->GetBrainManager()->GetBrainBase())
		{
			m_pPlayer->GetBrainManager()->SendToBrainBase("got_player_focus");
		}

		//also setup ourselves to be notified when it is destroyed
		m_playerDestroyedSlot = pNew->sig_delete.connect(this, &GameLogic::OnPlayerDeleted);
		
	} else
	{

		if (m_pPlayer)
		{
			if (GetCamera->GetEntTracking() == m_pPlayer->ID())
			{
				//shouldn't track this anymore		
				GetCamera->SetEntTracking(0);
			}
		}
		luabind::globals(GetScriptManager->GetMainState())["g_PlayerID"] = 0;
		m_pPlayer = pNew;
	}

	luabind::globals(GetScriptManager->GetMainState())["GetPlayer"] = m_pPlayer;

}

void GameLogic::OnKeyUp(const CL_InputEvent &key)
{
		if (g_keyManager.HandleEvent(key, false) )
		{
			//they handled it, let's ignore it
			return;
		}
}

void GameLogic::OnKeyDown(const CL_InputEvent &key)
{
		if (g_keyManager.HandleEvent(key, true))
		{
			//they handled it, let's ignore it
			return;
		}
	
	switch (key.id)
	{

#ifdef __APPLE__
	case CL_KEY_NUMPAD_SUBTRACT:
#endif
	case CL_KEY_SUBTRACT:
		Zoom(false);
		break;

#ifdef __APPLE__
	case CL_KEY_NUMPAD_ADD:
#endif
	case CL_KEY_ADD:

		Zoom(true);
		break;
	}

	if (!GetEditorActive())
	{

		switch (key.id)
		{

		case CL_KEY_S:
			if (CL_Keyboard::get_keycode(CL_KEY_CONTROL))
			{
				SetShowEntityCollisionData(!GetShowEntityCollisionData());
			}
			break;

		case CL_KEY_H:
			if (CL_Keyboard::get_keycode(CL_KEY_CONTROL))
			{
				if (CL_Keyboard::get_keycode(CL_KEY_SHIFT))
				{
					SetShowPathfinding(!GetShowPathfinding());
				} else
				{
					if (GetWorld)
					{
						GetWorldCache->SetDrawCollision(!GetWorldCache->GetDrawCollision());
					}

				}
				
			}
			break;
	
		case CL_KEY_F:
			if (CL_Keyboard::get_keycode(CL_KEY_CONTROL))
			{
				ToggleShowFPS();
			}
			break;
		case CL_KEY_J:
			if (CL_Keyboard::get_keycode(CL_KEY_CONTROL))
			{
				SetShowAI(!GetShowAI());
			}
			break;


		}

	}
}

void GameLogic::Zoom(bool zoomCloser)
{
	CL_Vector2 vecScale = GetCamera->GetScale();
	float scaleSpeed = 0.85f;

	if (zoomCloser)
	{
		vecScale *= scaleSpeed;
	} else
	{
		vecScale /= scaleSpeed;
	}

	GetCamera->SetScale(vecScale);
	GetCamera->InstantUpdate();
}

void GameLogic::OnMouseDown(const CL_InputEvent &key)
{
	if (!GetGamePaused())
	{
		if (g_keyManager.HandleEvent(key, true))
		{
			return;
		}
	}
}

void GameLogic::OnMouseUp(const CL_InputEvent &key)
{

	if (!GetGamePaused())
	{
		if (g_keyManager.HandleEvent(key, false))
		{
			return;
		}
	}

	switch(key.id)
	{
	case CL_MOUSE_LEFT:
		if (!GetEditorActive())
		{

			if (!GetShowMessageActive())
			if (!m_leftMouseButtonCallback.empty())
			{

				try {luabind::call_function<void>(GetScriptManager->GetMainState(), 
					m_leftMouseButtonCallback.c_str(), CL_Vector2(key.mouse_pos.x,key.mouse_pos.y));
				}  LUABIND_CATCH(m_leftMouseButtonCallback);
				
			}
		}

		break;

	
	case CL_MOUSE_WHEEL_UP:
		Zoom(true);
		break;

	case CL_MOUSE_WHEEL_DOWN:
		Zoom(false);
		break;
	}

}

void GameLogic::RebuildBuffers()
{
    //note, currently, this can get called before Init() is called!
    GetApp()->GetBackgroundCanvas()->get_gc()->clear(CL_Color(0,255,255));
}

void GameLogic::Kill()
{
	BlitMessage("Saving data...");

	SaveGlobals();

	if (GetWorld)
	{
		//the main world is active, let's save out the current player position I guess
		*GetWorld->GetCameraSetting() = GetCamera->GetCameraSettings();
	}
	
	if (EntityMgr->GetEntityByName("coleditor"))
	{
		//we were editting collision data, let's kill it without saving
		EntityMgr->GetEntityByName("coleditor")->SetDeleteFlag(true);
	}

	g_EntEditModeCopyBuffer.ClearSelection();
	
	//save out warp cache
	g_worldNavManager.Save();
	m_worldManager.Kill();
	
	m_myEntityManager.Kill();

	SAFE_DELETE(m_pGUIStyle);
	SAFE_DELETE(m_pGUIResources);
	SAFE_DELETE(m_pGUIManager);

	LogMsg("Initializing GameLogic");

	g_pSoundManager->Kill();
	g_MessageManager.Reset();
	m_leftMouseButtonCallback.clear();

	m_strUserProfileName.clear();
	g_materialManager.Init();
	g_watchManager.Clear();
	GetVisualProfileManager->Kill();

	
}

void GameLogic::InitGameGUI(string xmlFile)
{
	if (m_pGUIResources)
	{
		LogError("Can't init game GUI twice");
		return;
	}

	if (g_VFManager.LocateFile(xmlFile))
	{
		m_pGUIResources = new CL_ResourceManager(xmlFile);
		m_pGUIStyle = new CL_StyleManager_Bitmap(m_pGUIResources);
		m_pGUIManager = new CL_GUIManager(m_pGUIStyle);

		// Everytime the GUI draws, let's draw the game under it
		m_slots.connect(m_pGUIManager->sig_paint(), this, &GameLogic::OnRender);

		GetApp()->SetFont(C_FONT_GRAY,  new CL_Font("font_gray", m_pGUIResources));
		GetApp()->SetFont(C_FONT_NORMAL, new CL_Font("font_dialog", m_pGUIResources));



	} else
	{
		LogError("Unable to find GUI xml %s in base or mounted paths.", xmlFile.c_str());
	}


}

void ClearCacheDataFromWorldPath(string worldPath)
{
	string fileToDelete = worldPath+"/"+string(C_WORLD_NAV_FILENAME);

	//first kill it's main navgraph file
	RemoveFile(fileToDelete);
	
	//next scan each map path and remove cached data

	//scan map directory for available maps
	CL_DirectoryScanner scanner;

	scanner.scan(worldPath+"/"+string(C_BASE_MAP_PATH), "*");
	
	while (scanner.next())
	{
		std::string file = scanner.get_name();
		if (scanner.is_directory())
		{
			if (scanner.get_name()[0] != '_')
				if (scanner.get_name()[0] != '.')
				{
					//no underscore at the start, let's show it
					fileToDelete = worldPath+"/"+string(C_BASE_MAP_PATH)+"/"+scanner.get_name()+"/"+string(C_TAGCACHE_FILENAME);
					RemoveFile(fileToDelete);
				}

		}
	}
}

void GameLogic::DeleteAllCacheFiles()
{
		LogMsg("Deleting all tagcache files..");
		ClearCacheDataFromWorldPath(g_VFManager.GetLastMountedDirectory());
}

void GameLogic::Update(float step)
{
	if (m_bRestartEngineFlag)
	{
		m_bRestartEngineFlag = false;
		LogMsg("Restarting engine...");
		g_Console.SetOnScreen(false);
		Kill();
		
		if (m_bRebuildCacheData)
		{
			DeleteAllCacheFiles();
		}
		
		Init();
	}
	
	if (g_pSoundManager)
	g_pSoundManager->UpdateSounds();
	g_MessageManager.Update();
	GetCamera->Update(step);
	m_myEntityManager.Update(step);
	m_worldManager.Update(step);
	g_textManager.Update(step);
}


void GameLogic::RenderGameGUI(bool bDrawMainGUIToo)
{
	if (GetApp()->GetRenderedGameGUI() == false)
	{
		GetApp()->SetRenderedGameGUI(true);
		ClearScreen();

		m_worldManager.Render();
		m_myEntityManager.Render(); 
		g_textManager.Render();

		g_Console.Render();

		if (bDrawMainGUIToo)
		{
			GetApp()->GetGUI()->show(); //this will trigger its OnRender() which happens right before
		}

		if (GetShowFPS()) 
		{
			ResetFont(GetApp()->GetFont(C_FONT_NORMAL));

			int tiles = 0;
			if (GetWorldCache)
			{
				tiles = GetWorldCache->GetTilesRenderedLastFrameCount();
			}
			GetApp()->GetFont(C_FONT_NORMAL)->draw(GetScreenX-220,0, "FPS:" + CL_String::from_int(GetApp()->GetFPS())
				+" T:"+CL_String::from_int(tiles) + " W:"+CL_String::from_int(g_watchManager.GetWatchCount()));
		}
	}

}

void GameLogic::OnRender()
{
	//when the game GUI is in a modal dialog, this is the only thing causes everything else to get
	//rendered
	RenderGameGUI(true);
}


//when the main GUI is in a modal dialog box, this is the only thing causes the game gui and game screens
//to get rendered underneath

void GameLogic::Render()
{

	RenderGameGUI(false);
	GetApp()->SetRenderedGameGUI(false);

	if (GetApp()->GetGUI()->get_modal_component())
	{
		//need to jump start so it renders every frame
		GetApp()->GetGUI()->update();

		if (GetApp()->GetRequestedQuit())
		{
			GetApp()->GetGUI()->get_modal_component()->quit();
		}
	}
}

//generally, this means a script sent us a text message through the message scheduler. 
//Which we'll let the main script instance run..

void GameLogic::HandleMessageString(const string &msg)
{
  //LogMsg("Gamelogic got %s", msg.c_str());
  GetScriptManager->RunString(msg.c_str());
}

void SetCameraToTrackPlayer()
{
	MovingEntity * pEnt = (MovingEntity *)GetGameLogic->GetMyPlayer();

	if (pEnt)
	{
		if (pEnt->GetTile()->GetParentScreen()->GetParentWorldChunk()->GetParentWorld() == GetWorld)
		{
			GetCamera->SetEntTracking(pEnt->ID());
			GetCamera->InstantUpdate();
		}  else
		{
			//he's not from our world
		}
	}
}

void MovePlayerToCamera()
{
	MovingEntity * pEnt = (MovingEntity *)GetGameLogic->GetMyPlayer();

	if (pEnt)
	{
		CL_Vector2 playerPos = GetWorldCache->ScreenToWorld(CL_Vector2(GetScreenX/2, GetScreenY/2));
		pEnt->SetPos(playerPos);
		
		GetCamera->SetEntTracking(pEnt->ID());
		GetCamera->InstantUpdate();
		assert(pEnt->GetPos() == playerPos);
	} else
	{
		//assert(!"Can't find the player");
	}
}

void GameLogic::ClearAllMapsFromMemory()
{
   LogMsg("Clearing all maps");
   g_textManager.Reset();

   m_worldManager.Kill();
   
}

void MoveCameraToPlayer()
{
	MovingEntity * pEnt = (MovingEntity *)GetGameLogic->GetMyPlayer();

	if (pEnt)
	{
		GetCamera->SetEntTracking(0);
		GetCamera->SetTargetPos(CL_Vector2(GetPlayer->GetPos().x, GetPlayer->GetPos().y));
		GetCamera->InstantUpdate();
	}
}


void ShowMessage(string title, string msg, bool bForceClassicStyle)
{
	CL_GUIManager *pStyle = GetApp()->GetGUI();

	GetGameLogic->SetShowMessageActive(true); //so it knows not to send mouse clicks to the engine while

	if (!bForceClassicStyle && GetGameLogic->GetGameGUI())
	{
			NS_MessageBox m(GetGameLogic->GetGameGUI(), title, msg, "Ok", "", "");
			m.run();
			GetGameLogic->SetShowMessageActive(false);
			return;
	}
	
	CL_MessageBox message(title, msg, "Ok", "", "", pStyle);
	
	//we're showing this
	message.run();
	GetGameLogic->SetShowMessageActive(false);
}


void Schedule(unsigned int deliveryMS, unsigned int targetID, const char * pMsg)
{
	g_MessageManager.Schedule(deliveryMS, targetID, pMsg);
}

void ScheduleSystem(unsigned int deliveryMS, unsigned int targetID,const char * pMsg)
{
	g_MessageManager.ScheduleSystem(deliveryMS, targetID, pMsg);
}

bool RunGlobalScriptFromTopMountedDir(const char *pName)
{
	string fileName = GetGameLogic->GetScriptRootDir()+"/";
	fileName += pName;

	if (!g_VFManager.LocateFile(fileName))
	{
		//couldn't find it
		return false;
	}

	//run a global script to init anything that needs doing
	GetScriptManager->LoadMainScript( fileName.c_str());
	return true;
}