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
    m_slots.connect(CL_Keyboard::sig_key_down(), this, &GameLogic::OnKeyDown);
	m_slots.connect(CL_Keyboard::sig_key_up(), this, &GameLogic::OnKeyUp);
	m_slots.connect(CL_Mouse::sig_key_up(), this, &GameLogic::OnMouseUp);
    m_editorID = 0;
	m_bRestartEngineFlag = false;
	m_bShowEntityCollisionData = false;
	m_bGamePaused = false;
	m_bEditorActive = false;
	m_bParallaxActive = true;
	m_bMakingThumbnail = false;
	SetShowMessageActive(false);
	m_strBaseMapPath = C_BASE_MAP_PATH+ string("/");
	m_strScriptRootDir = "media/script";
	CL_Directory::create(m_strBaseUserProfilePath);
	SetShowFPS(false);
}


GameLogic::~GameLogic()
{
	Kill();
}

void GameLogic::SaveGlobals()
{
	if (!GetUserProfileName().empty())
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
	
	if (!GetUserProfileName().empty())
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

void GameLogic::SetLeftMouseButtonCallback(const string &luaFunctionName)
{
	m_leftMouseButtonCallback = luaFunctionName;
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

	string stPath = m_strBaseUserProfilePath + "/" + StripDangerousFromString(name);
	
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

    string stPath = m_strBaseUserProfilePath + "/" + name + string("/") + string(C_BASE_MAP_PATH);

	CL_DirectoryScanner scanner;
	//also delete all

	LogMsg("Resetting!");

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

	RemoveFile(stPath+"/"+string(C_PROFILE_DAT_FILENAME));
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
	m_strUserProfilePathWithName = m_strBaseUserProfilePath + "/" + m_strUserProfileName;

	//make all dirs that will be needed
	CL_Directory::create(m_strUserProfilePathWithName);
	CL_Directory::create(m_strUserProfilePathWithName + "/media");
	CL_Directory::create(m_strUserProfilePathWithName + "/media/maps");

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

bool GameLogic::Init()
{

	LogMsg("Initializing GameLogic");
	m_worldManager.Kill();
	g_materialManager.Init();
	GetVisualProfileManager->Kill();

	g_VFManager.Reset();
	g_VFManager.MountDirectoryPath(CL_Directory::get_current());
	m_worldManager.ScanWorlds(m_strBaseMapPath);
	m_pPlayer = NULL;
	//calculate our user profile base path, later, this could be in the windows user dir or whatever is correct
	m_strBaseUserProfilePath = CL_Directory::get_current() + "/profiles";
	CL_Display::clear(CL_Color(0,0,255));
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

	//run a global script to init anything that needs doing
	GetScriptManager->LoadMainScript( (GetGameLogic->GetScriptRootDir()+"/system/startup.lua").c_str());
	return true;
}

bool GameLogic::ToggleEditMode() //returns NULL if it was toggled off, or the address if on
{
	EntEditor * pEnt = (EntEditor *)EntityMgr->GetEntityByName("editor");
	
	if (!pEnt)
	{
		if (!GetWorld)
		{
			ShowMessage("Oops", "Can't open world editor, no world is loaded.");
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

void GameLogic::SetMyPlayer(MovingEntity *pNew)
{
	m_pPlayer = pNew;
	//also let's tell the script about it
	if (m_pPlayer)
	{
		luabind::globals(GetScriptManager->GetMainState())["GetPlayer"] = m_pPlayer;
		luabind::globals(GetScriptManager->GetMainState())["g_PlayerID"] = m_pPlayer->ID();

	} else
	{
		luabind::globals(GetScriptManager->GetMainState())["GetPlayer"] = NULL;
		luabind::globals(GetScriptManager->GetMainState())["g_PlayerID"] = 0;

	}

}

void GameLogic::OnKeyUp(const CL_InputEvent &key)
{
	if (!GetGamePaused())
	{
		if (g_keyManager.HandleEvent(key, false) )
		{
			//they handled it, let's ignore it
			return;
		}

	}
}

void GameLogic::OnKeyDown(const CL_InputEvent &key)
{
	if (!GetGamePaused())
	{
		if (g_keyManager.HandleEvent(key, true))
		{
			//they handled it, let's ignore it
			return;
		}

	}
	
	switch (key.id)
	{

	case CL_KEY_F:
		if (CL_Keyboard::get_keycode(CL_KEY_CONTROL))
		{
			ToggleShowFPS();
		}
		break;

	case CL_KEY_R:
		if (CL_Keyboard::get_keycode(CL_KEY_CONTROL))
				if (CL_Keyboard::get_keycode(CL_KEY_SHIFT))
		{
			SetRestartEngineFlag(true);
			return;

			
		}
		break;


	case  CL_KEY_F1:
		{
			ToggleEditMode();
			break;
		}
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

		case CL_KEY_E:
			if (CL_Keyboard::get_keycode(CL_KEY_CONTROL))
			{
				SetShowEntityCollisionData(!GetShowEntityCollisionData());
			}
			break;

		case CL_KEY_H:
			if (CL_Keyboard::get_keycode(CL_KEY_CONTROL))
			{
				if (GetWorld)
				{
					GetWorldCache->SetDrawCollision(!GetWorldCache->GetDrawCollision());
				}
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

void GameLogic::OnMouseUp(const CL_InputEvent &key)
{
	
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
		GetCamera->GetCameraSettings(*GetWorld->GetCameraSetting());
	}
	
	g_EntEditModeCopyBuffer.ClearSelection();
	m_worldManager.Kill();
	m_myEntityManager.Kill();
}

void GameLogic::Update(float step)
{
	
	if (m_bRestartEngineFlag)
	{
		m_bRestartEngineFlag = false;
		LogMsg("Restarting engine...");
		Kill();
		Init();
	}
	
	g_pSoundManager->UpdateSounds();
	g_MessageManager.Update();
	GetCamera->Update(step);
	m_myEntityManager.Update(step);
	m_worldManager.Update(step);
	g_textManager.Update(step);
}

void GameLogic::Render()
{
	ClearScreen();
	m_worldManager.Render();
	m_myEntityManager.Render(); 
	g_textManager.Render();
	g_Console.Render();
}

void GameLogic::HandleMessageString(const string &msg)
{
  LogMsg("Gamelogic got %s", msg.c_str());
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


void ShowMessage(string title, string msg)
{
	CL_MessageBox message(title, msg, "Ok", "", "", GetApp()->GetGUI());
	
	GetGameLogic->SetShowMessageActive(true); //so it knows not to send mouse clicks to the engine while
	//we're showing this
	message.run();
	GetGameLogic->SetShowMessageActive(false);
}


void Schedule(unsigned int deliveryMS, unsigned int targetID, const char * pMsg)
{
	g_MessageManager.Schedule(deliveryMS, targetID, pMsg);
}

void ScheduleSystem(unsigned int deliveryMS, const char * pMsg)
{
	g_MessageManager.Schedule(deliveryMS, 0, pMsg);
}