#include "AppPrecomp.h"
#include "Goal_Approach.h"
#include "Goal_SeekToPosition.h"
#include "PathPlanner.h"
#include "Goal_FollowPath.h"
#include "Map.h"
#include "Goal_MoveToPosition.h"
#include "message_types.h"
#include "MovingEntity.h"
#include "VisualProfileManager.h"

#define C_MAX_UPDATE_SPEED_MS 200 //how fast we'll update our postion from the real entity at max

//------------------------------- Activate ------------------------------------
//-----------------------------------------------------------------------------

void Goal_Approach::ReinitializeCompletely()
{
	//reset everything
	m_bTriedSimpleWay = false;
	m_bTriedComplexWay = false;
	m_targetNotFoundCount = 0;
}

void Goal_Approach::Activate()
{
	m_bRequestNextChunk = false;
	m_bWaitingForTurn = false;
	m_macroPath.m_path.clear();
	m_iStatus = active;
	m_locationUpdateTimer = 0;
	//make sure the subgoal list is clear.
	RemoveAllSubgoals();

	if (!UpdatePositionFromEntityID()) return; //there was an error, it will set the failed flag for us

	if (!m_bTriedSimpleWay && m_pDestMap == m_pOwner->GetMap())
	{
		m_bTriedSimpleWay = true;

		//we don't really need this fancy system?
		AddSubgoal(new Goal_MoveToPosition(m_pOwner, m_vDestination));
		return;
		//however, if this fails, we'll need it.
	}

	m_bTriedComplexWay = true;
	
#ifdef C_SHOW_PATHFINDING_DEBUG_INFO
	LogMsg("Ent %d (%s) is calculating an Approach MACRO path", m_pOwner->ID(), m_pOwner->GetName().c_str());
#endif

	//the destination is on a different map or requires a complicated intra-map route.  We need to sketch out a path to get to that map.
	m_macroPath = g_worldNavManager.FindPathToMapAndPos(m_pOwner, m_pDestMap, m_vDestination);

	if (!m_macroPath.IsValid())
	{
		LogMsg("Ent %d (%s) failed to find valid path", m_pOwner->ID(), m_pOwner->GetName().c_str());
		
		//try anyway, even though it's probably a waste of time
		if (m_pDestMap == m_pOwner->GetMap())
		{
			//dumb seek method
			AddSubgoal(new Goal_SeekToPosition(m_pOwner, m_vDestination));
			return;
		}
		m_iStatus = failed;

		return;
	}

	if (m_macroPath.m_path.size() == 1)
	{
		//if this happens, it means we CAN actually go directly there without warping.  Possible the 
		//m_bTriedSimpleWay thing failed.
		assert(m_pDestMap == m_pOwner->GetMap());
		m_macroPath.m_path.clear();
		AddSubgoal(new Goal_MoveToPosition(m_pOwner, m_vDestination));
		m_bTriedSimpleWay = false;
		m_bTriedComplexWay = false;
		return;

	}

	//we have data on which way to go
	ProcessNextMapChunk();
}


void Goal_Approach::ProcessNextMapChunk()
{
	m_bRequestNextChunk = false;

	if (!m_macroPath.IsValid())
	{
		//no more macro path, just move to the right spot now and we're done
		assert(m_pDestMap == m_pOwner->GetMap() && "How this happen?!");
		AddSubgoal(new Goal_MoveToPosition(m_pOwner, m_vDestination));
		m_bTriedSimpleWay = false; //if we have to recompute, we can try this way again
		return;
	}

	int nextNodeID = m_macroPath.m_path.front();

#ifdef C_SHOW_PATHFINDING_DEBUG_INFO

	LogMsg("Approach: %s is going to node %d", m_pOwner->GetName().c_str(), nextNodeID);
#endif

	MovingEntity *pNodeEnt = g_worldNavManager.ConvertWorldNodeToOwnerEntity(nextNodeID, true);

	if (!pNodeEnt)
	{
		LogError("Node no longer exists?");
		m_iStatus = inactive; //make it restart
		m_bTriedComplexWay = false;
	} else
	{
		assert(pNodeEnt->GetMap() == m_pOwner->GetMap() && "How this happen?!");

		AddSubgoal(new Goal_MoveToPosition(m_pOwner, pNodeEnt->GetPos()));
	}

}

CL_Vector2 GetPointOutsideOfRectFromInside(const CL_Rectf &r, const CL_Vector2 v, float padding)
{
	//i'm sure there is a much better mathy way of doing this, but this will do.  we need to throw a ray
	//from the center of this rect until we hit the edge
	CL_Vector2 pos = CL_Vector2::ZERO;
	
	while (r.is_inside( *(CL_Pointf*)&pos))
	{
      pos += v*2;
	}

	return pos + v * padding;
}


bool Goal_Approach::UpdatePositionFromEntityID()
{
	
	MovingEntity *pEnt = (MovingEntity*)EntityMgr->GetEntityFromID(m_targetID);

	if (!pEnt)
	{
		LogMsg("Approach failed, entity %d no longer exists.", m_targetID);
		m_iStatus = failed;
		return false;
	}

	//don't change this to >= because gametick could be 0 in some cases
	if (m_locationUpdateTimer > GetApp()->GetGameTick()) return true; //don't update again so fast, it's too costly

	m_pDestMap = pEnt->GetMap();

	
	//bad idea to sit directly on top of  the entity, let's move off it
	CL_Rectf c;
	
	m_locationUpdateTimer = GetApp()->GetGameTick()+C_MAX_UPDATE_SPEED_MS;

	if (pEnt->GetCollisionData() && m_distanceRequired >= 0) //a -1 means they want to be "inside"
	{
		c = pEnt->GetTile()->GetWorldColRect()- *(CL_Pointf*)&pEnt->GetPos();
	} else
	{
		//well, let's get the direction we want another way
		CL_Vector2 pos = pEnt->GetPos();
		
		if (m_distanceRequired <= 0)
		{
			//they want to stand directly on top, we don't need to calculate anything fancy
			m_vDestination = pos;
			return true;
		}

		c = CL_Rectf(pos.x - 5, pos.y - 5, pos.x+5, pos.y + 5);
	}

	CL_Vector2 vecAngleFromTarget;
	
	if (m_pDestMap == m_pOwner->GetMap())
	{
	   vecAngleFromTarget = pEnt->GetVectorToEntity(m_pOwner);
	} else
	{
		//we don't care which way you approach it, no preference
		vecAngleFromTarget = CL_Vector2(0,-1); //up
	}

	CL_Rectf ourRect = m_pOwner->GetTile()->GetWorldColRect();

	float padding = max(ourRect.get_height(), ourRect.get_width()) + 1;

	m_vLookPosition = pEnt->GetPos(); //where we want to look and where we want to stand are two different things
	//is this a valid point?
	int tries = 9;
	while(tries--)
	{

		m_vDestination = GetPointOutsideOfRectFromInside(c, vecAngleFromTarget, padding) + pEnt->GetPos();
	
#ifdef C_SHOW_PATHFINDING_DEBUG_INFO

		LogMsg("Approach of %s: Trying angle %s at pos %s", m_pOwner->GetName().c_str(), PrintVector(vecAngleFromTarget).c_str(), 
			PrintVector(m_vDestination).c_str());
#endif
	
		//found a potential, let's do further checking for more accuracy?
#ifdef _DEBUG
//		DrawBullsEyeWorld(m_vDestination, CL_Color::red, 20, CL_Display::get_current_window()->get_gc());
#endif
	
		if (m_pOwner->IsValidPosition(m_pDestMap, m_vDestination, true))
		{
			//cool
			break;
		}

		if (!tries)
		{
			//give up
			LogMsg("Approach: Ent %d (%s) Failed to find acceptable position close to entity %d (%s), everything blocked?  Let's just skip this.",
				m_pOwner->ID(), m_pOwner->GetName().c_str(), pEnt->ID(), pEnt->GetName().c_str());
			m_iStatus = failed;
			return false;
		}

		//well, we failed.  Perhaps there is a wall or something here blocking us.  Let's try from another angle.

		float angle = atan2(vecAngleFromTarget.x, vecAngleFromTarget.y)- (PI/4);
		angle = fmod(angle+PI, PI*2)-PI;
		vecAngleFromTarget.x = sin(angle);
		vecAngleFromTarget.y = cos(angle);
		vecAngleFromTarget.unitize();
		
	}

	return true; //everything seems ok
}

bool Goal_Approach::CloseEnoughAndFacingTheRightWay()
{
	
	float padding = 10;
	
	//if (m_distanceRequired < 0) padding = 1;

	if (m_pDestMap != m_pOwner->GetMap()) return false;

	if (!m_bWaitingForTurn && m_pOwner->GetDistanceFromPosition(m_vDestination) > m_distanceRequired+padding)
	{
		//not close enough yet
		return false;
	} 

	//now that we're close enough, get more accurate
	//see if our cached position is wrong
	if (!UpdatePositionFromEntityID()) return false; //there was an error, it will set the failed flag for us

	if (m_pOwner->GetDistanceFromPosition(m_vDestination) > m_distanceRequired + padding)
	{
		//uh oh, we need to reroute, this will trigger it
		m_bTriedSimpleWay = false;
		m_bTriedComplexWay = false;
		m_iStatus = inactive;
		return false;
	} 

	RemoveAllSubgoals(); //no longer need to move, we're close enough, go into idle

	m_bWaitingForTurn = true;
	m_pOwner->SetFacingTarget( VectorToFacing(m_pOwner->GetVectorToPosition(m_vLookPosition)));
	if (!m_pOwner->IsFacingTarget(0.5)) 
	{
		return false;
	}

	return true;
}

//------------------------------ Process --------------------------------------
//-----------------------------------------------------------------------------
int Goal_Approach::Process()
{
	
	//we should probably check if we're close enough and facing the right way
	if (m_iStatus == active)
	{
		if (CloseEnoughAndFacingTheRightWay())
		{
			//we're done
			m_iStatus = completed;
			return m_iStatus;
		}
			
	}

	if (m_iStatus == failed)
		return m_iStatus;

	//if status is inactive, call Activate()
	ActivateIfInactive();

	if (m_iStatus == failed)
		return m_iStatus;

	if (m_bWaitingForTurn) 
	{
		return m_iStatus;
	}

	if (m_bRequestNextChunk)
	{
		ProcessNextMapChunk();
		return m_iStatus;
	}

	//process the subgoals
	m_iStatus = ProcessSubgoals();

	if (m_iStatus == failed)
	{
		if (m_bTriedSimpleWay && m_bTriedComplexWay)
		{
			LogMsg("Entity %d (%s) cannot find a path to approach entity %d, aborting",
				m_pOwner->ID(), m_pOwner->GetName().c_str(), m_targetID);
			return m_iStatus;
		} else
		{
			m_iStatus = inactive;
			//this will cause it to reactivate on  the next frame using the macro search which
			//is more powerful
			return m_iStatus;

		}
	}

	if (m_iStatus == completed)
	{
		if (m_macroPath.IsValid())
		{
			//go to the next leg of the journey perhaps
			m_macroPath.m_path.pop_front();

			if (!m_macroPath.IsValid())
			{
				LogError("Path cut off early?");
				m_iStatus = inactive;
			} else
			{
				int nextNodeID = m_macroPath.m_path.front();
				MovingEntity *pNodeEnt = g_worldNavManager.ConvertWorldNodeToOwnerEntity(nextNodeID, true);

				if (!pNodeEnt)
				{
					LogMsg("Warp node was deleted?");
					//uh oh, we absolutely need to load this now
					m_iStatus = inactive; //force a restart

				} else
				{
#ifdef C_SHOW_PATHFINDING_DEBUG_INFO

					LogMsg("Approach: Warping %s to %s from %s", m_pOwner->GetName().c_str(), 
							m_pOwner->GetMap()->GetName().c_str(), pNodeEnt->GetMap()->GetName().c_str());
#endif

					if (pNodeEnt->GetScriptObject())
					{
						if (pNodeEnt->GetScriptObject()->FunctionExists("OnGoalPreWarp"))
						{
							try {	luabind::call_function<void>(pNodeEnt->GetScriptObject()->GetState(), "OnGoalPreWarp", m_pOwner);
							} catch (luabind::error &e) 
							{
								ShowLUAMessagesIfNeeded(e.state(), 1); 
								LogError("Error occurred in Warp entity ID %d while entity %d (%s) was warping through it with goal AI",
									pNodeEnt->ID(), m_pOwner->ID(), m_pOwner->GetName().c_str());
							}

						}
					}

					m_pOwner->SetPosAndMap(pNodeEnt->GetPos(), pNodeEnt->GetMap()->GetName());
					m_macroPath.m_path.pop_front();
					m_iStatus = active;
					m_bRequestNextChunk = true;

					if (pNodeEnt->GetScriptObject())
					{
						if (pNodeEnt->GetScriptObject()->FunctionExists("OnGoalPostWarp"))
						{
							try {	luabind::call_function<bool>(pNodeEnt->GetScriptObject()->GetState(), "OnGoalPostWarp", m_pOwner);
							} catch (luabind::error &e) 
							{
								ShowLUAMessagesIfNeeded(e.state(), 1); 
								LogError("Error occurred in Warp entity ID %d while entity %d (%s) was warping through it with goal AI",
									pNodeEnt->ID(), m_pOwner->ID(), m_pOwner->GetName().c_str());
							}

						}
					}
				}
			}
		} else
		{
			//looks like we're done as far as moving there
			m_bWaitingForTurn = true;
			m_iStatus = active; //short circuit this, otherwise it won't wait for the final head turn
		}
	}
	//if any of the subgoals have failed then this goal re-plans

	ReactivateIfFailed();

	return m_iStatus;
}

//---------------------------- HandleMessage ----------------------------------
//-----------------------------------------------------------------------------
bool Goal_Approach::HandleMessage(const Message& msg)
{

	//first, pass the message down the goal hierarchy
	bool bHandled = ForwardMessageToFrontMostSubgoal(msg);

	//if the msg was not handled, test to see if this goal can handle it
	if (bHandled == false)
	{
		switch(msg.GetMsgType())
		{

		case C_MSG_TARGET_NOT_FOUND:

#ifdef C_SHOW_PATHFINDING_DEBUG_INFO
			LogMsg("Target wasn't found, restarting");
#endif
			
			//if this is the first time the target wasn't found, let's let it keep going, because who knows, it might
			//find it by luck, just by walking in that direction.
			m_targetNotFoundCount++;

			
			//note, this causes problems when the goal is on the same map but not directly connected with path nodes
			/*
			if (m_targetNotFoundCount <= 2 && m_pOwner->GetMap() == m_pDestMap) //if it's on a different map, this will never work
			{
				return false; //let them know we want them to make do, and still walk around
			}
			*/

			return true; //signal that we handled it, this will cause us to just give up.
			break;

		case C_MSG_GOT_STUCK:
			m_bTriedSimpleWay = false;
	#ifdef C_SHOW_PATHFINDING_DEBUG_INFO
				LogMsg("Got C_MSG_GOT_STUCK");
	#endif
			return true; //signal that we handled it
			break;

		case C_MSG_PATH_NODES_MISSING:
			m_bTriedSimpleWay = false;
		#ifdef C_SHOW_PATHFINDING_DEBUG_INFO
					LogMsg("Got C_MSG_PATH_NODES_MISSING");
		#endif
			return true; //signal that we handled it
			break;

		default: return false;
		}
	}

	//handled by subgoals
	return false;
}

void Goal_Approach::Terminate()
{
	//we were terminated
#ifdef C_SHOW_PATHFINDING_DEBUG_INFO
	LogMsg("Approach of %s to %s (id %d): terminated", m_pOwner->GetName().c_str(),
		PrintVector(m_vDestination).c_str(), m_targetID);
#endif
}

void Goal_Approach::LostFocus()
{
	m_iStatus = inactive;
#ifdef C_SHOW_PATHFINDING_DEBUG_INFO
	LogMsg("Approach of %s: LostFocus..", m_pOwner->GetName().c_str());
#endif
	ReinitializeCompletely();
	RemoveAllSubgoals();
}

//-------------------------------- Render -------------------------------------
//-----------------------------------------------------------------------------
void Goal_Approach::Render()
{
	//forward the request to the subgoals
	Goal_Composite<MovingEntity>::Render();
}

