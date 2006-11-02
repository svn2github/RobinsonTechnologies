
#include "AppPrecomp.h"
#include "MovingEntity.h"
#include "DataManager.h"

#include "VisualProfile.h"
#include "MaterialManager.h"
#include "MessageManager.h"
#include "TextManager.h"
#include "TagManager.h"
#include "ScriptKeyManager.h"
#include "VisualProfileManager.h"
#include "AI/Goal_Think.h"
#include "AI/WatchManager.h"
#include "EntCreationUtils.h"

#ifndef WIN32
//windows already has this in the precompiled header for speed, I couldn't get that to work on mac..
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#endif


using namespace luabind;


CL_Vector2 ScreenToWorld(CL_Vector2 v)
{
	if (GetWorld)
	{
		return GetWorldCache->ScreenToWorld(v);
	}	

	LogMsg("ScreenToWorld error: No world active");
	return v;

}

CL_Vector2 WorldToScreen(CL_Vector2 v)
{
	if (GetWorld)
	{
		return GetWorldCache->WorldToScreen(v);
	} 

	LogMsg("WorldToScreen error: No world active");
	return v;
}

MovingEntity * GetEntityByID(int ID)
{
	return (MovingEntity*)EntityMgr->GetEntityFromID(ID);
}


MovingEntity * GetEntityByName(const string &name)
{
	TagObject *t = GetTagManager->GetFromString(name);
	if (!t) return NULL; //can't find it?
	if (t->m_entID == 0)
	{
		//let's load it right now
		GetWorldManager->LoadWorld(t->m_pWorld->GetDirPath(), false);
		t->m_pWorld->PreloadMap();

		if (t->m_entID == 0)
		{
			LogMsg("GetEntityByName: Entity %s couldn't be located.  Outdated tag? Removing it.", name.c_str());
			GetTagManager->RemoveByHashID(t->m_hashID, 0);
			return NULL; //not loaded yet
		}
	}
	
	return (MovingEntity*)EntityMgr->GetEntityFromID(t->m_entID);
}

BaseGameEntity * GetSpecialEntityByName(const string &name)
{
	return EntityMgr->GetEntityByName(name);
}

int GetEntityIDByName(const string &name)
{
	MovingEntity *pEnt = GetEntityByName(name);

	if (pEnt) return pEnt->ID();
	return 0;
}

MovingEntity * GetEntityByWorldPos(CL_Vector2 v, MovingEntity *pEntToIgnore)
{
	if (!GetWorld)
	{

		LogMsg("GetEntityByWorldPos: Error, no world is active right now.");
		return NULL;
	}

	Tile *pTile = NULL;

	CL_Rect recArea(v.x, v.y, v.x + 0.1f, v.y + 0.1f);

	//returns a list of tile pointers, we shouldn't free them!
	tile_list tileList;

	GetWorldCache->AddTilesByRect(recArea, &tileList, GetWorld->GetLayerManager().GetDrawList());

	//now we need to sort them
	g_pLayerManager = &GetWorld->GetLayerManager();
	tileList.sort(compareTileBySortLevelOptimized);

	tile_list::reverse_iterator itor = tileList.rbegin();
	MovingEntity *pEnt;
	
	for( ;itor != tileList.rend(); itor++)
	{
		pTile = (*itor);
		if (pTile->GetType() == C_TILE_TYPE_ENTITY)
		{
		   pEnt = ((TileEntity*)pTile)->GetEntity();

		   if (pEnt->GetMap()->GetLayerManager().GetLayerInfo(pTile->GetLayer()).GetShowInEditorOnly())
		   {
				continue;
		   }
			if (pEnt == pEntToIgnore)
			{
				continue;
			}
			//looks good to me!
			return pEnt;
		}
	}

	//couldn't find anything
	return NULL;
}

void LogErrorLUA(const char *pMessage)
{
	LogError(pMessage);
}

bool RunScript(const string &scriptName)
{
	
	string fileName = scriptName;

	if (!g_VFManager.LocateFile(fileName))
	{
		LogError("Unable to locate script file %s.", fileName.c_str());
		return false;
	}

	GetScriptManager->LoadMainScript(fileName.c_str());
	return true;
}

//more class definitions
void luabindMisc(lua_State *pState)
{
	module(pState)
		[

		class_<CL_Color>("Color")
		.def(constructor<>())
		.def(constructor<unsigned int, unsigned int, unsigned int, unsigned int>())
		.def("GetRed", &CL_Color::get_red)
		.def("GetGreen", &CL_Color::get_green)
		.def("GetBlue", &CL_Color::get_blue)
		.def("Getalpha", &CL_Color::get_alpha)

		.def("SetRed", &CL_Color::set_red)
		.def("SetGreen", &CL_Color::set_green)
		.def("SetBlue", &CL_Color::set_blue)
		.def("Setalpha", &CL_Color::set_alpha)

		.def("Set", &CL_Color::set_color)

		,class_<MaterialManager>("MaterialManager")
		.def("AddMaterial", &MaterialManager::AddMaterial)
		.def("GetMaterial", &MaterialManager::GetMaterial)


		,class_<CMaterial>("Material")
		.def("SetType", &CMaterial::SetType)
		.def("GetType", &CMaterial::GetType)
		.def("SetSpecial", &CMaterial::SetSpecial)
		.def("GetSpecial", &CMaterial::GetSpecial)

		,class_<GameLogic>("GameLogic")
		.def("SetLeftMouseButtonCallback", &GameLogic::SetLeftMouseButtonCallback)
		.def("ToggleEditMode", &GameLogic::ToggleEditMode)
		.def("SetUserProfileName", &GameLogic::SetUserProfileName)
		.def("GetUserProfileName", &GameLogic::GetUserProfileName)
		.def("ResetUserProfile", &GameLogic::ResetUserProfile)
		.def("ClearAllMapsFromMemory", &GameLogic::ClearAllMapsFromMemory)
		.def("UserProfileExists", &GameLogic::UserProfileExists)
		.def("SetRestartEngineFlag", &GameLogic::SetRestartEngineFlag)
		.def("UserProfileActive", &GameLogic::UserProfileActive)
		.def("SetPlayer", &GameLogic::SetMyPlayer)
		.def("Quit", &GameLogic::Quit)
		.def("ClearModPaths", &GameLogic::ClearModPaths)
		.def("AddModPath", &GameLogic::AddModPath)
		.def("InitGameGUI", &GameLogic::InitGameGUI)

		,class_<App>("App")
		.def("SetGameLogicSpeed", &App::SetGameLogicSpeed)
		.def("SetGameSpeed", &App::SetGameSpeed)
		.def("GetGameTick", &App::GetGameTick)
		.def("GetTick", &App::GetTick)
		.def("ParmExists", &App::ParmExists)
		.def("SetSimulationSpeedMod", &App::SetSimulationSpeedMod)
		.def("GetSimulationSpeedMod", &App::GetSimulationSpeedMod)
		.def("SetScreenSize", &App::SetScreenSize)
		.def("GetEngineVersion", &App::GetEngineVersion)
		.def("GetEngineVersionAsString", &App::GetEngineVersionAsString)

		,class_<ISoundManager>("SoundManager")
		.def("PlayMusic", &ISoundManager::PlayMusic)
		.def("Play", &ISoundManager::Play)
		.def("PlayMixed", &ISoundManager::PlayMixed)
		.def("PlayLooping", &ISoundManager::PlayLooping)
		.def("MuteAll", &ISoundManager::MuteAll)
		.def("KillMusic", &ISoundManager::KillMusic)
		.def("KillChannel", &ISoundManager::KillChannel)
		.def("AddEffect", &ISoundManager::AddEffect)
		.def("SetVolume", &ISoundManager::SetVolume)
		.def("RemoveAllEffects", &ISoundManager::RemoveAllEffects)
		.def("SetSpeedFactor", &ISoundManager::SetSpeedFactor)
		.def("SetVolume", &ISoundManager::SetVolume)

		,class_<TextManager>("TextManager")
		.def("Add", &TextManager::Add)
		.def("AddCustom", &TextManager::AddCustom)
		.def("AddCustomScreen", &TextManager::AddCustomScreen)


		,class_<LayerManager>("LayerManager")
		.def("GetLayerIDByName", &LayerManager::GetLayerIDByName)
		
		,class_<World>("Map")
		.def("SetPersistent", &World::SetPersistent)
		.def("GetPersistent", &World::GetPersistent)
		.def("SetAutoSave", &World::SetAutoSave)
		.def("GetAutoSave", &World::GetAutoSave)
		.def("GetName", &World::GetName)
		.def("GetLayerManager", &World::GetLayerManager)
		.def("BuildLocalNavGraph", &World::BuildNavGraph)


		,class_<WorldManager>("MapManager")
		.def("SetActiveMapByName", &WorldManager::SetActiveWorldByName)
		.def("GetActiveMap", &WorldManager::GetActiveWorld)
		.def("UnloadMapByName", &WorldManager::UnloadWorldByName)
		
		,class_<TagManager>("TagManager")
		.def("GetFromString", &TagManager::GetFromString)
		.def("GetFromHash", &TagManager::GetFromHash)
		.def("GetPosFromName", &TagManager::GetPosFromName)
		.def("RegisterAsWarp", &TagManager::RegisterAsWarp)

		,class_<Camera>("Camera")
		.def("SetPos", &Camera::SetPos)
		.def("GetPos", &Camera::GetPos)
		.def("GetPosCentered", &Camera::GetPosCentered)
		.def("SetPosCentered", &Camera::SetPosCentered)
		.def("SetTargetPos", &Camera::SetTargetPos)
		.def("SetTargetPosCentered", &Camera::SetTargetPosCentered)
		.def("SetEntityTrackingByID", &Camera::SetEntTracking)
		.def("InstantUpdate", &Camera::InstantUpdate)
		.def("SetScale", &Camera::SetScale)
		.def("SetScaleTarget", &Camera::SetScaleTarget)
		.def("SetMoveLerp", &Camera::SetMoveLerp)
		.def("SetScaleLerp", &Camera::SetScaleLerp)
		.def("Reset", &Camera::Reset)
		.def("GetScale", &Camera::GetScale)
		.def("GetCameraSettings", &Camera::GetCameraSettings)
		.def("SetCameraSettings", &Camera::SetCameraSettings)

		,class_<TagObject>("TagObject")
		.def("GetMapName", &TagObject::GetMapName)
		.def("GetID", &TagObject::GetID)
		.def("GetPos", &TagObject::GetPos)

		,class_<CameraSetting>("CameraSettings")

		,class_<ScriptKeyManager>("KeyManager")
		.def("AssignKey", &ScriptKeyManager::AssignKey)

		,class_<Goal_Think>("GoalManager")
		.def("PushMoveToPosition", &Goal_Think::PushMoveToPosition)
		.def("AddMoveToPosition", &Goal_Think::AddMoveToPosition)
		.def("RemoveAllSubgoals", &Goal_Think::RemoveAllSubgoals)
		.def("PushDelay", &Goal_Think::PushDelay)
		.def("AddDelay", &Goal_Think::AddDelay)
		.def("AddApproach", &Goal_Think::AddApproach)
		.def("AddApproachAndSay", &Goal_Think::AddApproachAndSay)
		.def("PushApproach", &Goal_Think::PushApproach)
		.def("PushApproachAndSay", &Goal_Think::PushApproachAndSay)

		.def("AddSay", &Goal_Think::AddSay)
		.def("AddSayByID", &Goal_Think::AddSayByID)
		.def("PushSay", &Goal_Think::PushSay)
		.def("PushSayByID", &Goal_Think::PushSayByID)

		.def("PushRunScriptString", &Goal_Think::PushRunScriptString)
		.def("AddRunScriptString", &Goal_Think::AddRunScriptString)

		.def("PushMoveToTag", &Goal_Think::PushMoveToTag)
		.def("AddMoveToTag", &Goal_Think::AddMoveToTag)

		.def("PushNewGoal", &Goal_Think::PushNewGoal)
		.def("AddNewGoal", &Goal_Think::AddNewGoal)

		.def("IsGoalActiveByName", &Goal_Think::IsGoalActiveByName)

		.def("GetGoalCountByName", &Goal_Think::GetGoalCountByName)

		,class_<WatchManager>("WatchManager")
		.def("Add", &WatchManager::Add)
		.def("Remove", &WatchManager::Remove)
		.def("GetWatchCount", &WatchManager::GetWatchCount)
		
		,
		//stand alone functions

		def("CreateEntity", &CreateEntity),
		def("Schedule", &Schedule),
		def("ScheduleSystem", &ScheduleSystem),
		def("ScreenToWorld", &ScreenToWorld),
		def("WorldToScreen", &WorldToScreen),
		def("GetEntityByWorldPos", &GetEntityByWorldPos),
		def("GetEntityByID", &GetEntityByID),
		def("GetEntityByName", &GetEntityByName),
		def("GetSpecialEntityByName", &GetSpecialEntityByName),
		def("GetEntityIDByName", &GetEntityIDByName),
		def("ShowMessage", &ShowMessage),
		def("FacingToVector", &FacingToVector),
		def("VectorToFacing", &VectorToFacing),
		def("LogError", &LogErrorLUA),
		def("RunScript", &RunScript),
		def("CreateEntitySpecial", &CreateEntitySpecial),
		def("ColorToString", &ColorToString),
		def("StringToColor", &StringToColor),
		def("VectorToString", &VectorToString),
		def("StringToVector", &StringToVector),
		def("RectToString", &RectToString),
		def("StringToRect", &StringToRect)
		];
}
