#include "AppPrecomp.h"
#include "MovingEntity.h"
#include "DataManager.h"
#include "BrainManager.h"
#include "State.h"

#ifndef WIN32
//windows already has this in the precompiled header for speed, I couldn't get that to work on mac..
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#endif

#include <luabind/object.hpp>

using namespace luabind;

string MovingEntityToString(MovingEntity * pEnt)
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
			

			,class_<MovingEntity>("Entity")
			.def(constructor<>())
			.def("ID", &MovingEntity::ID)
			.def("GetPos", &MovingEntity::GetPos)
			.def("SetPos", &MovingEntity::SetPos)
			.def("GetName", &MovingEntity::GetName)
			.def("SetName", &MovingEntity::SetName)
			.def("SetPosAndMap", &MovingEntity::SetPosAndMap)
			.def("SetVisualProfile", &MovingEntity::SetVisualProfile)
			.def("GetBrainManager", &MovingEntity::GetBrainManager)
			.def("__tostring", &MovingEntityToString)
			.def("InitCollisionDataBySize", &MovingEntity::InitCollisionDataBySize)
			.def("LoadCollisionInfo", &MovingEntity::LoadCollisionInfo)
			.def("EnableRotation", &MovingEntity::EnableRotation)
			.def("SetDensity", &MovingEntity::SetDensity)
			.def("SetListenCollision", &MovingEntity::SetListenCollision)
			.def("GetListenCollision", &MovingEntity::GetListenCollision)
			.def("SetListenCollisionStatic", &MovingEntity::SetListenCollisionStatic)
			.def("GetListenCollisionStatic", &MovingEntity::GetListenCollisionStatic)
			.def("SetDeleteFlag", &MovingEntity::SetDeleteFlag)
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
	
			.def("SetBaseColor", &MovingEntity::SetBaseColor)
			.def("GetBaseColor", &MovingEntity::GetBaseColor)
		];
}


		
