#include "AppPrecomp.h"
#include "MovingEntity.h"
#include "DataManager.h"
#include "BrainManager.h"
#include "Brain.h"
#include "State.h"
#include "AI/Goal_Think.h"
#include "ListBindings.h"

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


TileList GetNearbyTileListForScript(MovingEntity *pEnt)
{
	return TileList(&pEnt->GetNearbyTileList());
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
			.def("Remove", &BrainManager::Remove)
			.def("AskBrainByName", &BrainManager::AskBrainByName)
			.def("SetStateByName", &BrainManager::SetStateByName)
			.def("GetStateByName", &BrainManager::GetStateByName)
			.def("LastStateWas", &BrainManager::LastStateWas)
			.def("InState", &BrainManager::InState)
			.def("SendToBrainBase", &BrainManager::SendToBrainBase)
			

			,class_<Brain>("Brain")
			.def("GetName", &State::GetName)

			,class_<BaseGameEntity>("BaseEntity")
			.def("GetID", &BaseGameEntity::ID)
			.def("GetName", &BaseGameEntity::GetName)
			.def("SetName", &BaseGameEntity::SetName)
			.def("__tostring", &EntityToString)
			.def("SetDeleteFlag", &BaseGameEntity::SetDeleteFlag)
			.def("Send", &BaseGameEntity::HandleMessageString)


/*

Object: Entity
The Entity object.

About:

The heart of the Novashell system is the flexible <Entity> object.

An <Entity> can represent a patch of ground, an chair, a persistent database, a warp, a health bar, a text overlay or a drooling monster.

Advanced abilities such as:

 * An attached script
 * The BrainManager
 * The GoalManager
 * Collision information
 * Pathfinding system
 * Triggers
 
are initialized on demand, only wasting memory if they are actually used.

Relative paths:

After a script is loaded, the ~/ character sequence can be used to mean a relative path from the script that is loaded.
For example:

(code)
//this assumes duck_headless.xml and head.col are in the same directory as the script
//that is now running and attached to this entity
this:SetVisualProfile("~/duck_headless.xml", "duck_head");
this:LoadCollisionInfo("~/head.col");
(end)

Namespace:

An entity contains it's own lua namespace, with the global namespace used as a fallback if nothing local is found.

(code)
m_myOwnVar = 4; //this var will be unique for each entity using this script

//If a global function or variable exists, you can access and modify it the normal way:
g_dialogCanBeCanceled = true; //write to a global var from an entity's script

//However, to CREATE a global variable that didn't exist from an entity you must do this:
_G.g_myNewGlobal = 3;

The _G forces the global namespace to be used.
(end)

To get local variable data from an entity, use Entity::RunFunction() which gives you access to its local functions.

Group: General
*/

			,class_<MovingEntity, BaseGameEntity>("Entity")
			.def(constructor<>())
			
			/*
			func: GetPos
			(code)
			Vector2 GetPos()
			(end)

			Returns:

			A <Vector2> object containing the entity's position.
			*/
			
			.def("GetPos", &MovingEntity::GetPosSafe)

			/*
			func: SetPos
			(code)
			nil SetPos(Vector2 vPos)
			(end)

			Parameters:

			vPos - a <Vector2> object containing the position to move to.
			*/

			.def("SetPos", &MovingEntity::SetPos)
			.def("SetPosAndMap", &MovingEntity::SetPosAndMap)
		
			/*
			func: SetPosAndMap
			(code)
			nil SetPosAndMap(Vector2 vPos, Map map)
			(end)

			Parameters:

			vPos - a <Vector2> object containing the position to move to. Our own position information is ignored.
			map - The <Map> the entity should move to.  Note, the move actually happens at the end of the logic cycle, not instantly.
			
			*/
			.def("SetPosAndMapByTagName", &MovingEntity::SetPosAndMapByTagName)

			/*
			func: SetPosAndMapByTagName
			(code)
			boolean SetPosAndMapByTagName(string tagName)
			(end)

			This can be an easy way to move a player to a specific place.
			
			Parameters:

			tagName - Any entity that has been named can be used as a tagName

			Returns:
			True on success, false if the tag name wasn't found
			*/

			
			.def("GetMap", &MovingEntity::GetMap)
			/*
			func: GetMap
			(code)
			Map GetMap()
			(end)

			Usage:
			(code)
			LogMsg("Hi, I'm in the " .. this:GetMap():GetName() .. " map.);
			(end)
			Returns:

			The <Map> object that the entity is currently in.
			*/

			.def("SetAttach", &MovingEntity::SetAttach)
			/*
			func: SetAttach
			(code)
			nil SetAttach(number entityID, Vector2 vOffset)
			(end)

			Attaching means we will automatically move with the entity we are attached to.

			There are two special things you can use instead of an entity ID:

			* <C_ENTITY_NONE> - This unattaches us if we were previously attached to something.
			* <C_ENTITY_CAMERA> - This attaches us to the camera.  Use this on GUI items, and they will stay "on-screen" without moving.

			Parameters:

			entityID - The entity's ID that we're supposed to attached to.
			vOffset - a <Vector2> containing the offset to attach at.  Use Vector2(0,0) for no offset.

			*/
			
			.def("GetAttach", &MovingEntity::GetAttachEntityID)
			/*
			func: GetAttach
			(code)
			number GetAttach()
			(end)

			Returns:
			The entity ID of who we are attached to.  <C_ENTITY_NONE> if none.
			*/

			
			.def("SetAttachOffset", &MovingEntity::SetAttachOffset)
			/*
			func: SetAttachOffset
			(code)
			nil SetAttachOffset(Vector2 vOffset)
			(end)

			Change the offset position in reference to the entity you're attached to.
			Only useful if you've already used <SetAttach> to attach an entity.

			Parameters:

			vOffset - A <Vector2> object containing the new offset that should be used.
			*/

			.def("GetAttachOffset", &MovingEntity::GetAttachOffset)

			/*
			func: GetAttachOffset
			(code)
			Vector2 SetAttachOffset()
			(end)

			Returns:

			A <Vector2> object containing the current attach offset.
			*/

			.def("SetLayerID", &MovingEntity::SetLayerID)

			/*
			func: SetLayerID
			(code)
			nil SetLayerID(number layerID)
			(end)

			Allows you to change the layer of an entity.

			Note that the actual movement will not happen until the end of that game logic cycle.

			Usage:
			(code)
			this:SetLayerID(this:GetLayerID()+1); //move up one layer
			//Note, this may or may not affect our sorting level as expected, depends on the layer's settings.
			(end)
			*/



			.def("GetLayerID", &MovingEntity::GetLayerID)
		
			/*
			func: GetLayerID
			(code)
			number GetLayerID()
			(end)

			Usage:
			(code)
			local layerID = this:GetLayerID();
			LogMsg("We're on layer " .. layerID);			
			(end)

			Returns:

			The layer ID that we're on.
			*/
			.def("SetLayerByName", &MovingEntity::SetLayerByName)

			/*
			func: SetLayerIDByName
			(code)
			nil SetLayerIDByName(string layerName)
			(end)

			Allows you to change the layer of an entity by using it's name.

			Usage:
			(code)
			this:SetLayerIDByName("Overlay 1"); //move to the Overlay 1 layer.
			(end)
			*/

			.def("SetScale", &MovingEntity::SetScale)
		
			/*
			func: SetScale
			(code)
			nil SetScale(Vector2 vScale)
			(end)

			Allows you to scale up or down an <Entity> in size.  The collision information is automatically scaled with it.  If you'd like to scale the collision information only, use <SetCollisionScale>.

			Parameters:
			
			vScale - A <Vector2> object containing the new scale. A scale of Vector2(2,2) would mean twice the default size, Vector2(0.5,0.5) would mean half size.
			*/

			.def("GetScale", &MovingEntity::GetScale)

			/*
			func: GetScale
			(code)
			Vector2 GetScale()
			(end)

			Returns:

			The current entity scale.
			*/

		

			
			.def("GetWorldCollisionRect", &MovingEntity::GetWorldCollisionRect)
			
			/*
			func: GetWorldCollisionRect
			(code)
			Rectf GetWorldCollisionRect()
			(end)

			Usage:
			(code)
			LogMsg("My collision box in world coordinates is " .. tostring(this:GetWorldCollisionRect());
			(end)
			Returns:

			A <Rectf> object containing the region the entity's collision occupies in world coordinates.
			*/



			.def("GetCollisionRect", &MovingEntity::GetCollisionRect)

			/*
			func: GetCollisionRect
			(code)
			Rectf GetCollisionRect()
			(end)

			Returns:

			A <Rectf> object containing the entity's collision box in local coordinates.
			*/


//Group: Image related

.def("SetVisualProfile", &MovingEntity::SetVisualProfile)
/*
func: SetVisualProfile
(code)
boolean SetVisualProfile(string fileName, string profileName)
(end)

A visual profile is an .xml file that contains data about how the entity should look.  An .xml can contain more than one "profile", but only one can be assigned to an entity at a time.

To create a profile, it's recommended you start with an existing one from one of the examples and use it as a template.

A profile can contain data for many directions and many states, such as walking, attacking, idling and dieing.  As well as special animations that are used in a custom way by the script.

Brains automatically communicate with profiles to see what can be displayed and intelligently choose what to display if an animation is missing.
Parameters:

Technical Note:

VisualProfiles and images contained are cached out and shared between Entities.

Parameters:

fileName - Filename of the .xml to load.
profileName - The profile name within the .xml to use.

Returns:

True on success, false if the xml file or profile name wasn't found.
*/


.def("SetBaseColor", &MovingEntity::SetBaseColor)

/*
func: SetBaseColor
(code)
nil SetBaseColor(Color color);
(end)

Using this you can change the color of an entity.  Technically, you're just setting how much of red, green, blue can get through.

So if you start with white, you can make any color by removing the colors you don't want.

A <Color> object also contains an alpha value, this let's you set the translucency.  (This happens on top of whatever per-pixel alpha the image already has)

Usage:
(code)
this:SetBaseColor(Color(255,20,20,255)); //we should now look very red, since we're not
//letting much green or blue through.

//another example

Color c = Color(255,255,255,110); //full color, but only about half the alpha
this:SetBaseColor(c); //we're now about 50% invisible
(end)

Parameters:

color - A <Color> object.
*/


.def("GetBaseColor", &MovingEntity::GetBaseColor)

/*
func: GetBaseColor
(code)
Color GetBaseColor();
(end)

Returns:

A <Color> object containing the entity's base color.  Temporary color changes such as those created by a ColorFlash brain will not affect this value.
*/

.def("GetSizeX", &MovingEntity::GetSizeX)

/*
func: GetSizeX
(code)
number GetSizeX()
(end)

Returns:

The width of the image currently representing the entity.
*/


.def("GetSizeY", &MovingEntity::GetSizeY)
/*
func: GetSizeY
(code)
number GetSizeY()
(end)

Returns:

The height of the image currently representing the entity.
*/

.def("SetAnimByName", &MovingEntity::SetAnimByName)

/*
func: SetAnimByName
(code)
nil SetAnimByName(string animName)
(end)

This is a simple way to cause an animation to be played on an Entity.  Check the barrel.lua script in the Dink Example to see it in action.

Note:

This method won't work for objects that use the BrainManager extensively, as they will set change the state and animation themselves.  For those situations, use the ForceVisual brain instead.

Parameters:

animName - This the name of an animation that has been defined in its Visual Profile.
*/


.def("SetAnimFrame", &MovingEntity::SetAnimFrame)


/*
func: SetAnimFrame
(code)
nil SetAnimFrame(number frame)
(end)

A method to set the current frame of the currently playing animation.

Parameters:

frame - This is a frame number or <C_ANIM_FRAME_LAST> to specify the last frame of the animation.

Parameters:

animName - This the name of an animation that has been defined in its Visual Profile.
*/

.def("GetAnimFrame", &MovingEntity::GetAnimFrame)

/*
func: GetAnimFrame
(code)
frame GetAnimFrame(number frame)
(end)

Returns:

The frame of the animation currently being display.
*/


.def("SetBlendMode", &MovingEntity::SetBlendMode)

/*
func: SetBlendMode
(code)
nil SetBlendMode(number blendConstant)
(end)

Parameters:

blendConstant - Must be one of the <C_BLEND_MODE_CONSTANTS>.
*/


.def("GetBlendMode", &MovingEntity::GetBlendMode)

/*
func: GetBlendMode
(code)
number GetBlendMode()
(end)

Returns:

One of the <C_BLEND_MODE_CONSTANTS>.
*/



.def("SetAnimPause", &MovingEntity::SetAnimPause)

/*
func: SetAnimPause
(code)
nil SetAnimPause(boolean bPause)
(end)

Allows you to pause the currently playing animation.  Used in the TreeWorld example when stopping on ladders.

Parameters:

bPause - true to pause, false to unpause
*/

.def("GetAnimPause", &MovingEntity::GetAnimPause)

/*
func: GetAnimPause
(code)
boolean GetAnimPause()
(end)

Returns:

True if the active animation is currently paused.
*/

.def("SetAlignment", &MovingEntity::SetAlignment)


.def("SetSpriteByVisualStateAndFacing", &MovingEntity::SetSpriteByVisualStateAndFacing)


/*
func: SetSpriteByVisualStateAndFacing
(code)
nil SetSpriteByVisualStateAndFacing()
(end)

This sets the current animation based on the facing and visual state.  Normally brains do this for you.
*/

.def("SetVisualState", &MovingEntity::SetVisualState)
.def("GetVisualState", &MovingEntity::GetVisualState)

.def("SetImage", &MovingEntity::SetImage)


/*
func: SetImage
(code)
nil SetImage(string fileName, Rect clipRect)
(end)

Allows you to set a tilepic-style image.  When you click "Convert to entity" on a tilepic, internally, this is how it remembers what to show.

Because the change is saved in the entity's Data() system, it is persistent.  (You will see a _TilePic and _TileRect entry in the entities data, using the editor)

Note:

VisualProfiles will override this image.

Parameters:

fileName - A filename of an image in one of the Map directories.  No path should be included, it will find automatically.
clipRect - A <Rect> that contains what portion of the image to show.  Send nil to use the entire image.
*/


.def("SetImageByID", &MovingEntity::SetImageByID)

/*
func: SetImageByID
(code)
nil SetImageByID(number imageHashID)
(end)

Like <SetImage> but Sets the image by its hash directly.  As file names are not stored, you may only have a hash to work with at times.

Parameters:

imageHashID - A number that represents an image.
*/

.def("GetImageID", &MovingEntity::GetImageID)

/*
func: GetImageByID
(code)
number GetImageByID()
(end)

Returns the hashID of the active tilepic style image, if one has been set by script of by the editor when "Convert To Entity" is used.

Returns:

A number representing the image's hashID.
*/


.def("GetImageClipRect", &MovingEntity::GetImageClipRect)


/*
func: GetImageClipRect
(code)
Rect GetImageClipRect()
(end)

Returns:

A <Rect> containing the portion of the tilepic-style image being displayed.
*/

//Group: Physics/Collision Related

.def("SetCollisionMode", &MovingEntity::SetCollisionMode)
/*
func: SetCollisionMode
(code)
nil SetCollisionMode(number collisionMode)
(end)

Sets the overall collision mode of the Entity.

Parameters:

collisionMode - One of the <C_COLLISION_MODE_CONSTANTS>.

*/



.def("InitCollisionDataBySize", &MovingEntity::InitCollisionDataBySize)

/*
func: InitCollisionDataBySize
(code)
nil InitCollisionDataBySize(number x, number y)
(end)

Allows you to specify a basic rectangle collision shape.
If you enter 0's for the size, the current image size will be used.

Note:

Only objects with collision data of some kind can fall from gravity or move.  This is used in intro_menu.lua to allow clicked objects to fall.

Parameters:

x - The width of the new collision area
y - The height of the new collision area

*/

.def("LoadCollisionInfo", &MovingEntity::LoadCollisionInfo)


/*
func: LoadCollisionInfo
(code)
nil LoadCollisionInfo(string fileName)
(end)

Loads a .col (collision) file.  If it doesn't exist, it will be created, and any subsequent collision editing done in  the editor will automatically be saved to its collision file.

About Collision Files:

A collision file contains one or more vector shapes and information about which materials are assigned to them.

Note:

It's fine to share a single collision file between multiple kinds of entities.

Parameters:

fileName - The collision file to load.  Relative paths are supposed with ~/.

*/

.def("SetEnableRotationPhysics", &MovingEntity::EnableRotation)

/*
func: SetEnableRotationPhysics
(code)
nil SetEnableRotationPhysics(boolean bEnable)
(end)

Rotation in this sense means the physics of barrels and boxes that can spin on the Z axis.

* In TopView RPG test, nothing has rotation physics turned on, we don't want barrels and boxes twisting around when their corners are pushed
* In TreeWorld Side view test, barrels, boxes and apples have rotation physics on. The player doesn't, otherwise he will tip over and look drunk.

Parameters:
bEnable - True to activate rotation physics
*/

.def("GetEnableRotationPhysics", &MovingEntity::GetEnableRotation)

/*
func: GetEnableRotationPhysics
(code)
boolean GetEnableRotationPhysics()
(end)

Returns:

True if rotation physics are enabled for this entity.
*/



.def("SetListenCollision", &MovingEntity::SetListenCollision)

/*
func: SetListenCollision
(code)
nil SetListenCollision(number listenCollisionMode)
(end)

If not set to <C_LISTEN_COLLISION_NONE> the entity's script will be called with information about collision with entities.

The callback allows you to examine the collision angle, who it was with, the force, and reject it if desired by returning false instead of true.

If you enable this, your script must include a function with the following name and parameters:
(code)
function OnCollision(normal, depth, materialID, ent) //return true/false to allow/disallow the collision

	LogMsg("Touched Entity # " .. ent:GetID() .. " Depth: " .. tostring(depth) .. " normal: " .. tostring(normal));

	if (normal.y < -0.1 and depth > 0) then
		//hmm, this means the entity must have jumped up and hit us with its head.  
		Crumble(); //let this brick break
	end

	//if we return false, the collision is ignored by both entities and we can walk through eachother.

	return true; //enable the collision
end
(end)

Parameters:

listenCollisionMode - Must be one of the <C_LISTEN_COLLISION_CONSTANTS>.

*/


.def("GetListenCollision", &MovingEntity::GetListenCollision)


/*
func: GetListenCollision
(code)
collisionMode GetListenCollision()
(end)

Returns:

One of the <C_LISTEN_COLLISION_CONSTANTS> to indicate the current entity collision listen mode.
*/


.def("SetListenCollisionStatic", &MovingEntity::SetListenCollisionStatic)

/*
func: SetListenCollisionStatic
(code)
nil SetListenCollisionStatic(number listenCollisionStaticMode)
(end)

If not set to <C_LISTEN_COLLISION_STATIC_NONE> the entity's script will be called with information about collision with static tiles. (non entities)

The callback allows you to examine the collision angle, who it was with, the force, and reject it if desired by returning false instead of true.

If you enable this, your script must include a function with the following name and parameters:
(code)
function OnCollisionStatic(normal, depth, materialID)

	LogMsg("Hit Static: Depth: " .. tostring(depth) .. " normal: " .. tostring(normal));

	if (materialID == C_MATERIAL_VERTICAL_LADDER) then
			LogMsg("You just touched a tile that has a collision line set to "V Ladder" type!
		end

	//we'd need to return false here to ignore the collision, so we can walk through whatever it is.

	return true;
end
(end)

Parameters:

listenCollisionStaticMode - Must be one of the <C_LISTEN_COLLISION_STATIC_CONSTANTS>.

*/
.def("GetListenCollisionStatic", &MovingEntity::GetListenCollisionStatic)

/*
func: GetListenCollisionStatic
(code)
collisionStaticMode GetListenCollisionStatic()
(end)

Returns:

One of the <C_LISTEN_COLLISION_STATIC_CONSTANTS> to indicate the current static (meaning tiles that aren't entities) collision listen mode.
*/



.def("SetDensity", &MovingEntity::SetDensity)

/*
func: SetDensity
(code)
nil SetDensity(number density)
(end)

Setting density allows you to control how hard it is to push certain objects.  Note that if a _Density data field is found this is automatically set as the objects density.

A density is not the total weight/mass of an object, it's a more like the weight per square inch of collision size.  So a very large object with a small density could still push around a tiny object with a large density.

Note:

Setting density to 0 is a special case that means "Immovable" and saves processor time.

Parameters:

density - How dense this object is.  0.2 is like Styrofoam, 3 is like a block of steel.

*/

.def("SetMass", &MovingEntity::SetMass)

/*
func: SetMass
(code)
nil SetMass(number mass)
(end)

This sets the total mass of the current collision shape.  Normally you would use <SetDensity> instead, so it would scale to  the size of the object automatically.  (Big snakes should be able to push around small snakes, for example)

Parameters:

mass - The total mass of the entity when computed collisions.
*/


.def("GetMass", &MovingEntity::GetMass)

/*
func: GetMass
(code)
number GetMass()
(end)

Returns:

The total mass of this entity.
*/

.def("SetGravityOverride", &MovingEntity::SetGravityOverride)

/*
func: SetGravityOverride
(code)
nil SetGravityOverride(number gravity)
(end)

This allows you to override gravity settings on a per-entity basis.  So floating monsters can exist along-side ones that fall to the ground.

Set to <C_GRAVITY_OVERRIDE_DISABLED> to return to using the <Map>'s default gravity.

Parameters:

The gravity you'd like applied to this object, instead of the <Map>'s default gravity.
*/

.def("GetGravityOverride", &MovingEntity::GetGravityOverride)

/*
func: GetGravityOverride
(code)
number GetGravityOverride()
(end)

Returns:

The current gravity override, or <C_GRAVITY_OVERRIDE_DISABLED> if not being used.
*/


.def("Stop", &MovingEntity::Stop)

/*
func: Stop
(code)
nil Stop()
(end)

Instantly stops movement of this entity, including both linear and angular velocity. (Stops moving and stops rotating)
*/

.def("StopX", &MovingEntity::StopX)

/*
func: StopX
(code)
nil StopX()
(end)

Instantly stops movement of this entity on the X axis only.  (Stops moving side to side)
*/


.def("StopY", &MovingEntity::StopY)

/*
func: StopY
(code)
nil StopY()
(end)

Instantly stops movement of this entity on the Y axis only.  (Stops moving up and down)
*/

.def("AddForce", &MovingEntity::AddForceBurst)

/*
func: AddForce
(code)
nil AddForce(Vector2 vForce)
(end)

Adds the applied force to the entity.  Useful to make something jump or get "hit" in a certain direction.

Parameters:

vForce - A <Vector2> object containing how much force to add.  this:AddForce(Vector2(20,0)) would send someone to the right.
*/

.def("AddForceAndTorque", &MovingEntity::AddForceAndTorqueBurst)

/*
func: AddForceAndTorque
(code)
nil AddForceAndTorque(Vector2 force, Vector2 vTorque)
(end)

Adds a force as well as a rotational torque.  Normally you only need <AddForce> and don't care about torque.

Parameters:

force - A <Vector2> object containing how much force to add.  this:AddForce(Vector2(20,0)) would send someone to the right.
*/

.def("AddForceConstant", &MovingEntity::AddForce)
.def("AddForceAndTorqueConstant", &MovingEntity::AddForceAndTorque)

.def("GetLinearVelocity", &MovingEntity::GetLinearVelocity)
.def("GetOnGround", &MovingEntity::IsOnGround)
.def("SetOnGround", &MovingEntity::SetIsOnGround)
.def("GetOnGroundAccurate", &MovingEntity::IsOnGroundAccurate)
.def("SetCollisionScale", &MovingEntity::SetCollisionScale)
.def("GetCollisionScale", &MovingEntity::GetCollisionScale)
.def("SetDampening", &MovingEntity::SetDampening)

//Group: AI Related

.def("GetBrainManager", &MovingEntity::GetBrainManager)
.def("GetGoalManager", &MovingEntity::GetGoalManager)
.def("SetDesiredSpeed", &MovingEntity::SetDesiredSpeed)
.def("SetMaxMovementSpeed", &MovingEntity::SetMaxWalkSpeed)
.def("SetTurnSpeed", &MovingEntity::SetTurnSpeed)
.def("GetTurnSpeed", &MovingEntity::GetTurnSpeed)
.def("GetFacing", &MovingEntity::GetFacing)
.def("SetFacing", &MovingEntity::SetFacing)
.def("SetFacingTarget", &MovingEntity::SetFacingTarget)
.def("GetFacingTarget", &MovingEntity::GetFacingTarget)
.def("GetVectorFacing", &MovingEntity::GetVectorFacing)
.def("GetVectorFacingTarget", &MovingEntity::GetVectorFacingTarget)
.def("SetVectorFacing", &MovingEntity::SetVectorFacing)
.def("SetVectorFacingTarget", &MovingEntity::SetVectorFacingTarget)
.def("GetVectorToEntity", &MovingEntity::GetVectorToEntity)
.def("GetVectorToEntityByID", &MovingEntity::GetVectorToEntityByID)
.def("GetVectorToPosition", &MovingEntity::GetVectorToPosition)
.def("GetDistanceFromEntityByID", &MovingEntity::GetDistanceFromEntityByID)
.def("GetDistanceFromPosition", &MovingEntity::GetDistanceFromPosition)
.def("SetNavNodeType", &MovingEntity::SetNavNodeType)
.def("SetHasPathNode", &MovingEntity::SetHasPathNode)
.def("IsFacingTarget", &MovingEntity::IsFacingTarget)
.def("IsOnSameMapAsEntityByID", &MovingEntity::IsOnSameMapAsEntityByID)
.def("IsCloseToEntity", &MovingEntity::IsCloseToEntity)
.def("IsCloseToEntityByID", &MovingEntity::IsCloseToEntityByID)
.def("CanWalkTo", &MovingEntity::CanWalkTo)
.def("IsValidPosition", &MovingEntity::IsValidPosition)
.def("HasLineOfSightToPosition", &MovingEntity::CanWalkTo)


//Group: Scripting Related
.def("SetVisibilityNotifications", &MovingEntity::SetVisibilityNotifications)
.def("SetRunUpdateEveryFrame", &MovingEntity::SetRunUpdateEveryFrame)
.def("GetRunUpdateEveryFrame", &MovingEntity::GetRunUpdateEveryFrame)
.def("SetTrigger", &MovingEntity::SetTrigger)
.def("RunFunction", (luabind::object(MovingEntity::*) (const string&)) &MovingEntity::RunFunction)
.def("RunFunction", (luabind::object(MovingEntity::*) (const string&, luabind::object)) &MovingEntity::RunFunction)
.def("RunFunction", (luabind::object(MovingEntity::*) (const string&, luabind::object, luabind::object)) &MovingEntity::RunFunction)
.def("RunFunction", (luabind::object(MovingEntity::*) (const string&, luabind::object, luabind::object, luabind::object)) &MovingEntity::RunFunction)
.def("FunctionExists", &MovingEntity::FunctionExists)
.def("OnDamage", &MovingEntity::OnDamage)
.def("DumpScriptInfo", &MovingEntity::DumpScriptInfo)



//Group: Audio Related
.def("PlaySound", &MovingEntity::PlaySound)
.def("PlaySoundPositioned", &MovingEntity::PlaySoundPositioned)

//Group: Text Related

.def("SetText", &MovingEntity::SetText)
.def("GetText", &MovingEntity::GetText)
.def("GetTextBounds", &MovingEntity::GetTextBounds)
.def("SetTextAlignment", &MovingEntity::SetTextAlignment)
.def("SetTextColor", &MovingEntity::SetTextColor)
.def("SetTextScale", &MovingEntity::SetTextScale)
.def("GetTextScale", &MovingEntity::GetTextScale)
.def("SetTextRect", &MovingEntity::SetTextRect)
.def("SetDefaultTextColor", &MovingEntity::SetDefaultTextColor)

		
//Group: Data/Cloning Related
.def("Data", &MovingEntity::GetData)
.def("Clone", &MovingEntity::Clone)
.def("CreateEntity", &MovingEntity::CreateEntity)
.def("SetPersistent", &MovingEntity::SetPersistent)
.def("GetPersistent", &MovingEntity::GetPersistent)
.def("IsPlaced", &MovingEntity::IsPlaced)

			.def("__tostring", &EntityToString)
			
		//Group: Zone/Scanning Related
		
	.def("InZoneByMaterialType", &MovingEntity::InZoneByMaterialType)
	.def("InNearbyZoneByMaterialType", &MovingEntity::InNearbyZoneByMaterialType)
	.def("GetActiveZoneByMaterialType", &MovingEntity::GetNearbyZoneByCollisionRectAndType)
	.def("GetNearbyZoneByMaterialType", &MovingEntity::GetNearbyZoneByPointAndType)
	.def("GetNearbyTileList", &GetNearbyTileListForScript)
			
			
	];

/*

Section: Entity Related Constants

Group: C_BLEND_MODE_CONSTANTS

constants: C_BLEND_MODE_NORMAL
Default blending mode.

constants: C_BLEND_MODE_ADDITIVE
Colors overlay to get brighter.

constants: C_BLEND_MODE_NEGATIVE
Colors are removed to make things darker.


Group: C_ANIM_FRAME_CONSTANTS

constants: C_ANIM_FRAME_LAST
Means the last frame of the animation.


Group: C_ENTITY_CONSTANTS

constants: C_ENTITY_NONE
Means no entity is specified.

constants: C_ENTITY_CAMERA
Use <Entity::SetAttach> with this to make things move with the camera, good for GUI overlays like health bars.

Group: C_COLLISION_MODE_CONSTANTS
Used with <Entity::SetCollisionMode>.

constants: C_COLLISION_MODE_ALL
Can collide with entities and static tiles.

constants: C_COLLISION_MODE_NONE
Doesn't collide with anything.

constants: C_COLLISION_MODE_STATIC_ONLY
Only collides with static tiles, not other entities.


Group: C_LISTEN_COLLISION_CONSTANTS
Used with <Entity::SetListenCollision>.

constants: C_LISTEN_COLLISION_NONE
No script callbacks for entity collisions.

constants: C_LISTEN_COLLISION_PLAYER_ONLY
Callbacks only happen when colliding with an active player.

constants: C_LISTEN_COLLISION_ALL_ENTITIES
Script callbacks happen for all entity collisions.


Group: C_LISTEN_COLLISION_STATIC_CONSTANTS
Used with <Entity::SetListenCollisionStatic>.

constants: C_LISTEN_COLLISION_STATIC_NONE
No script callbacks for static tile collisions.

constants: C_LISTEN_COLLISION_STATIC_ALL
Script callbacks happen on every static tile collision.


Group: C_GRAVITY_CONSTANTS
Used with <Entity::SetGravityOverride>.

constants: C_GRAVITY_OVERRIDE_DISABLED
Use the default <Map>'s gravity on this entity instead of a user defined one.
*/


}


		
