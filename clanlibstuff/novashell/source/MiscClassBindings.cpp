
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


//more class definitions
void luabindMisc(lua_State *pState)
{
	module(pState)
		[
/*
Section: Misc Objects
A hodge podge of objects to be sorted into separate areas later I guess.
*/


/*
Object: Color
The color object stores R G B A color information.

About:
Internally the color is stored as a 32 bit integer.


The range for the colors and alpha is between 0 and 255.

Usage:

(code)
myColor = Color(); //instantiate it

myColor:Set(255,255,255,255); //we just set it to white.

//Or we can save time by doing:

myColor = Color(255,255,255,255);

(end)

*/
		class_<CL_Color>("Color")
	
		//Group: Initialization
		
		/*
		func: Color()
		(code)
		Color(number red, number green, number blue, number alpha)
		(end)
		Creates the object with default values. (everything will be 0)
		*/

		.def(constructor<>())

		//func: Color(r,g,b,a)
		
		.def(constructor<unsigned int, unsigned int, unsigned int, unsigned int>())

		//Group: Member Functions

		//func: GetRed
		//(code)
		//number GetRed()
		//(end)
		.def("GetRed", &CL_Color::get_red)

		//func: GetGreen
		//(code)
		//number GetGreen()
		//(end)
		.def("GetGreen", &CL_Color::get_green)

		//Function: GetBlue
		//(code)
		//number GetBlue()
		//(end)
		.def("GetBlue", &CL_Color::get_blue)

		/*
		Function: GetAlpha
		(code)
		number GetAlpha()
		(end)
		
		Returns:
		the alpha component. (0 is transparent, 255 is opaque/fully visible)
		*/

		.def("GetAlpha", &CL_Color::get_alpha)

		//func: SetRed
		//(code)
		//nil SetRed(number red)
		//(end)
		.def("SetRed", &CL_Color::set_red)
		//func: SetGreen
		//(code)
		//nil SetGreen(number green)
		//(end)
		.def("SetGreen", &CL_Color::set_green)
		//func: SetBlue
		//(code)
		//nil SetBlue(number blue)
		//(end)
		.def("SetBlue", &CL_Color::set_blue)
		//func: SetAlpha
		//(code)
		//nil SetAlpha
		//(end)
		.def("SetAlpha", &CL_Color::set_alpha)

		/*
		func: Set
		(code)
		nil Set(number red, number green, number blue, number alpha)
		(end)
		Sets all the values at once.
		*/
		
		.def("Set", &CL_Color::set_color)
		//Group: -=-=-=-=-=-
	
		,class_<MaterialManager>("MaterialManager")
		
		.def("AddMaterial", &MaterialManager::AddMaterial)
		.def("GetMaterial", &MaterialManager::GetMaterial)


		,class_<CMaterial>("Material")
		.def("SetType", &CMaterial::SetType)
		.def("GetType", &CMaterial::GetType)
		.def("SetSpecial", &CMaterial::SetSpecial)
		.def("GetSpecial", &CMaterial::GetSpecial)

		,class_<GameLogic>("GameLogic")
		.def("ToggleEditMode", &GameLogic::ToggleEditMode)
		.def("SetUserProfileName", &GameLogic::SetUserProfileName)
		.def("GetUserProfileName", &GameLogic::GetUserProfileName)
		.def("ResetUserProfile", &GameLogic::ResetUserProfile)
		.def("UserProfileExists", &GameLogic::UserProfileExists)
		.def("SetRestartEngineFlag", &GameLogic::SetRestartEngineFlag)
		.def("UserProfileActive", &GameLogic::UserProfileActive)
		.def("SetPlayer", &GameLogic::SetMyPlayer)
		.def("Quit", &GameLogic::Quit)
		.def("ClearModPaths", &GameLogic::ClearModPaths)
		.def("AddModPath", &GameLogic::AddModPath)
		.def("InitGameGUI", &GameLogic::InitGameGUI)

		,class_<App>("App")
//		.def("SetGameLogicSpeed", &App::SetGameLogicSpeed)
	//	.def("SetGameSpeed", &App::SetGameSpeed)
		.def("GetGameTick", &App::GetGameTick)
		.def("GetTick", &App::GetTick)
		.def("ParmExists", &App::ParmExists)
		.def("SetSimulationSpeedMod", &App::SetSimulationSpeedMod)
		.def("GetSimulationSpeedMod", &App::GetSimulationSpeedMod)
		.def("SetScreenSize", &App::SetScreenSize)
		.def("GetEngineVersion", &App::GetEngineVersion)
		.def("GetEngineVersionAsString", &App::GetEngineVersionAsString)
		.def("SetWindowTitle", &App::SetWindowTitle)
		.def("SetCursorVisible", &App::SetCursorVisible)
		.def("GetCursorVisible", &App::GetCursorVisible)
		.def("GetPlatform", &App::GetPlatform)

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
		.def("LoadMapByName", &WorldManager::LoadWorldByName)
		
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

		,class_<ScriptKeyManager>("InputManager")
		.def("AddBinding", &ScriptKeyManager::AddBinding)
		.def("RemoveBinding", &ScriptKeyManager::RemoveBinding)
		.def("GetMousePos", &ScriptKeyManager::GetMousePos)
		.def("SetMousePos", &ScriptKeyManager::SetMousePos)

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
		
		
		];
}
