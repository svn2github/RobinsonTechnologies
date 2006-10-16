#include "AppPrecomp.h"
#include "MovingEntity.h"
#include "DataManager.h"
#include "BrainManager.h"
#include "Brain.h"
#include "State.h"
#include "AI/Goal_Think.h"

#ifndef WIN32
//windows already has this in the precompiled header for speed, I couldn't get that to work on mac..
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#endif

#include <luabind/object.hpp>

using namespace luabind;

string EntityToString(BaseGameEntity * pEnt)
{
	return "Entity " + CL_String::from_int(pEnt->ID()) + " (" + pEnt->GetName()+")";
}

void luabindEntity(lua_State *pState)
{
	module(pState)
		[

			class_<DataManager>("DataManager")
			.def("Delete", &DataManager::Delete)
			.def("Exists", &DataManager::Exists)
			.def("Get", &DataManager::Get)
			.def("Set", &DataManager::Set)
			.def("SetNum", &DataManager::SetNum)
			.def("GetNum", &DataManager::GetNum)
			.def("ModNum", &DataManager::ModNum)
			.def("Clear", &DataManager::Clear)
			.def("SetIfNull", &DataManager::SetIfNull)

			,class_<State>("State")
			.def("GetName", &State::GetName)

			,class_<BrainManager>("BrainManager")
			.def("Add", &BrainManager::Add)
			.def("SendToBrainByName", &BrainManager::SendToBrainByName)
			.def("AskBrainByName", &BrainManager::AskBrainByName)
			.def("SetStateByName", &BrainManager::SetStateByName)
			.def("GetStateByName", &BrainManager::GetStateByName)
			.def("LastStateWas", &BrainManager::LastStateWas)
			.def("InState", &BrainManager::InState)
			.def("SendToBrainBase", &BrainManager::SendToBrainBase)
			

			,class_<Brain>("Brain")
			.def("GetName", &State::GetName)

			,class_<BaseGameEntity>("BaseEntity")
			.def("ID", &BaseGameEntity::ID)
			.def("GetName", &BaseGameEntity::GetName)
			.def("SetName", &BaseGameEntity::SetName)
			.def("__tostring", &EntityToString)
			.def("SetDeleteFlag", &BaseGameEntity::SetDeleteFlag)
			.def("Send", &BaseGameEntity::HandleMessageString)

			,class_<MovingEntity, BaseGameEntity>("Entity")
			.def(constructor<>())
			.def("GetPos", &MovingEntity::GetPos)
			.def("SetPos", &MovingEntity::SetPos)
			.def("SetPosAndMap", &MovingEntity::SetPosAndMap)
			.def("SetVisualProfile", &MovingEntity::SetVisualProfile)
			.def("GetBrainManager", &MovingEntity::GetBrainManager)
			.def("__tostring", &EntityToString)
			.def("InitCollisionDataBySize", &MovingEntity::InitCollisionDataBySize)
			.def("LoadCollisionInfo", &MovingEntity::LoadCollisionInfo)
			.def("SetCanRotate", &MovingEntity::EnableRotation)
			.def("GetCanRotate", &MovingEntity::GetEnableRotation)
			.def("SetDensity", &MovingEntity::SetDensity)
			.def("SetListenCollision", &MovingEntity::SetListenCollision)
			.def("GetListenCollision", &MovingEntity::GetListenCollision)
			.def("SetListenCollisionStatic", &MovingEntity::SetListenCollisionStatic)
			.def("GetListenCollisionStatic", &MovingEntity::GetListenCollisionStatic)
			.def("SetMass", &MovingEntity::SetMass)
			.def("GetMass", &MovingEntity::GetMass)
			.def("AddForce", &MovingEntity::AddForce)
			.def("AddForceAndTorque", &MovingEntity::AddForceAndTorque)
			.def("GetLinearVelocity", &MovingEntity::GetLinearVelocity)
			.def("SetPersistent", &MovingEntity::SetPersistent)
			.def("GetPersistent", &MovingEntity::GetPersistent)
			.def("InZoneByMaterialType", &MovingEntity::InZoneByMaterialType)
			.def("GetOnLadder", &MovingEntity::GetOnLadder)
			.def("SetDefaultTextColor", &MovingEntity::SetDefaultTextColor)
			.def("Data", &MovingEntity::GetData)
			.def("SetPosAndMapByTagName", &MovingEntity::SetPosAndMapByTagName)
			.def("OnDamage", &MovingEntity::OnDamage)
			.def("SetAnimByName", &MovingEntity::SetAnimByName)
			.def("SetSpriteByVisualStateAndFacing", &MovingEntity::SetSpriteByVisualStateAndFacing)
			.def("GetFacing", &MovingEntity::GetFacing)
			.def("SetFacing", &MovingEntity::SetFacing)
			.def("SetFacingTarget", &MovingEntity::SetFacingTarget)
			.def("GetVectorFacing", &MovingEntity::GetVectorFacing)
			.def("GetVectorFacingTarget", &MovingEntity::GetVectorFacingTarget)
			.def("SetVectorFacing", &MovingEntity::SetVectorFacing)
			.def("SetVectorFacingTarget", &MovingEntity::SetVectorFacingTarget)
			.def("SetVisualState", &MovingEntity::SetVisualState)
			.def("GetVisualState", &MovingEntity::GetVisualState)
			.def("GetLayerID", &MovingEntity::GetLayerID)
			.def("SetLayerID", &MovingEntity::SetLayerID)
			.def("RunFunction", (luabind::object(MovingEntity::*) (const string&)) &MovingEntity::RunFunction)
			.def("RunFunction", (luabind::object(MovingEntity::*) (const string&, luabind::object)) &MovingEntity::RunFunction)
			.def("RunFunction", (luabind::object(MovingEntity::*) (const string&, luabind::object, luabind::object)) &MovingEntity::RunFunction)
			.def("RunFunction", (luabind::object(MovingEntity::*) (const string&, luabind::object, luabind::object, luabind::object)) &MovingEntity::RunFunction)
			.def("GetScale", &MovingEntity::GetScale)
			.def("SetScale", &MovingEntity::SetScale)
			.def("SetCollisionScale", &MovingEntity::SetCollisionScale)
			.def("GetCollisionScale", &MovingEntity::GetCollisionScale)
			.def("SetTrigger", &MovingEntity::SetTrigger)
			.def("DumpScriptInfo", &MovingEntity::DumpScriptInfo)
			.def("SetCollisionMode", &MovingEntity::SetCollisionMode)
	
			.def("GetDistanceFromEntityByID", &MovingEntity::GetDistanceFromEntityByID)
			.def("GetDistanceFromPosition", &MovingEntity::GetDistanceFromPosition)
			
			.def("SetBaseColor", &MovingEntity::SetBaseColor)
			.def("GetBaseColor", &MovingEntity::GetBaseColor)
			.def("GetGoalManager", &MovingEntity::GetGoalManager)
			.def("SetDesiredSpeed", &MovingEntity::SetDesiredSpeed)
			.def("SetMaxWalkSpeed", &MovingEntity::SetMaxWalkSpeed)
			.def("IsPlaced", &MovingEntity::IsPlaced)
			.def("SetNavNodeType", &MovingEntity::SetNavNodeType)
			.def("SetHasPathNode", &MovingEntity::SetHasPathNode)
			.def("SetVisibilityNotifications", &MovingEntity::SetVisibilityNotifications)

			.def("HasLineOfSightToPosition", &MovingEntity::CanWalkTo)
			.def("GetVectorToEntity", &MovingEntity::GetVectorToEntity)
			.def("GetVectorToEntityByID", &MovingEntity::GetVectorToEntityByID)
			.def("GetVectorToPosition", &MovingEntity::GetVectorToPosition)
			.def("IsOnSameMapAsEntityByID", &MovingEntity::IsOnSameMapAsEntityByID)
			.def("IsCloseToEntity", &MovingEntity::IsCloseToEntity)
			.def("IsCloseToEntityByID", &MovingEntity::IsCloseToEntityByID)
			.def("GetMap", &MovingEntity::GetMap)
			.def("PlaySound", &MovingEntity::PlaySound)
			.def("PlaySoundPositioned", &MovingEntity::PlaySoundPositioned)
	];
}


		
