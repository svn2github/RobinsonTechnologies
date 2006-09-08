#include "AppPrecomp.h"
#include "Goal_MoveToMapAndPos.h"
#include "Goal_SeekToPosition.h"
#include "PathPlanner.h"
#include "Goal_FollowPath.h"
#include "World.h"
#include "Goal_MoveToPosition.h"

//------------------------------- Activate ------------------------------------
//-----------------------------------------------------------------------------
void Goal_MoveToMapAndPos::Activate()
{
	m_bRequestNextChunk = false;
	
	m_iStatus = active;
	//make sure the subgoal list is clear.
	RemoveAllSubgoals();



	if (!m_bTriedSimpleWay && m_pDestMap == m_pOwner->GetMap())
	{
		m_bTriedSimpleWay = true;
	
		//we don't really need this fancy system?
		AddSubgoal(new Goal_MoveToPosition(m_pOwner, m_vDestination));
		return;
		//however, if this fails, we'll need it.
	}


	//the destination is on a different map or requires a complicated intra-map route.  We need to sketch out a path to get to that map.
	
	m_macroPath = g_worldNavManager.FindPathToMapAndPos(m_pOwner, m_pDestMap, m_vDestination);
	
	if (!m_macroPath.IsValid())
	{
		m_iStatus = failed;
		LogMsg("Failed to find valid path between maps");
		return;
	}
	
	
	//we have data on which way to go

	ProcessNextMapChunk();
	

	/*
	//actually do something

	if (bUsePath)
	{
		AddSubgoal(new Goal_FollowPath(m_pOwner,
			m_pOwner->GetPathPlanner()->GetPath()));
	} else
	{
		//dumb seek method
		AddSubgoal(new Goal_SeekToPosition(m_pOwner, m_vDestination));
	}
	*/
}


void Goal_MoveToMapAndPos::ProcessNextMapChunk()
{
	m_bRequestNextChunk = false;

	if (!m_macroPath.IsValid())
	{
		//no more macro path, just move to the right spot now and we're done
		assert(m_pDestMap == m_pOwner->GetMap() && "How this happen?!");
		AddSubgoal(new Goal_MoveToPosition(m_pOwner, m_vDestination));
		return;
	}
	
	int nextNodeID = m_macroPath.m_path.front();
	
	LogMsg("Going to node %d", nextNodeID);

	MovingEntity *pNodeEnt = g_worldNavManager.ConvertWorldNodeToOwnerEntity(nextNodeID);
	
	if (!pNodeEnt)
	{
		LogError("Node no longer exists?");
		m_iStatus = inactive; //make it restart
	} else
	{
		AddSubgoal(new Goal_MoveToPosition(m_pOwner, pNodeEnt->GetPos()));
	}

}

//------------------------------ Process --------------------------------------
//-----------------------------------------------------------------------------
int Goal_MoveToMapAndPos::Process()
{
	//if status is inactive, call Activate()
	ActivateIfInactive();

	if (m_bRequestNextChunk)
	{
		ProcessNextMapChunk();
	}
	
	//process the subgoals
	m_iStatus = ProcessSubgoals();

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
				MovingEntity *pNodeEnt = g_worldNavManager.ConvertWorldNodeToOwnerEntity(nextNodeID);

				if (!pNodeEnt)
				{
					LogMsg("Warp node was deleted?");
					m_iStatus = inactive; //force a restart

				} else
				{
					m_pOwner->SetPosAndMap(pNodeEnt->GetPos(), pNodeEnt->GetMap()->GetName());
					m_macroPath.m_path.pop_front();
					m_iStatus = active;
					m_bRequestNextChunk = true;
				
				}
			}
		}
	}
	//if any of the subgoals have failed then this goal re-plans

	ReactivateIfFailed();

	return m_iStatus;
}

//---------------------------- HandleMessage ----------------------------------
//-----------------------------------------------------------------------------
bool Goal_MoveToMapAndPos::HandleMessage(const Message& msg)
{

	//first, pass the message down the goal hierarchy
	bool bHandled = ForwardMessageToFrontMostSubgoal(msg);

	//if the msg was not handled, test to see if this goal can handle it
	if (bHandled == false)
	{

		switch(msg.GetMsgType())
		{

			/*
			case Msg_PathReady:

			//clear any existing goals
			RemoveAllSubgoals();

			LogMsg("Adding subgoal: Followpath");
			/*    
			AddSubgoal(new Goal_FollowPath(m_pOwner,
			m_pOwner->GetPathPlanner()->GetPath()));

			return true; //msg handled

			case Msg_NoPathAvailable:
			m_iStatus = failed;
			return true; //msg handled
			}
			*/
		default: return false;
		}

	}

	//handled by subgoals
	return true;
}

//-------------------------------- Render -------------------------------------
//-----------------------------------------------------------------------------
void Goal_MoveToMapAndPos::Render()
{

	//forward the request to the subgoals
	Goal_Composite<MovingEntity>::Render();

	/*
	//draw a bullseye
	gdi->BlackPen();
	gdi->BlueBrush();
	gdi->Circle(m_vDestination, 6);
	gdi->RedBrush();
	gdi->RedPen();
	gdi->Circle(m_vDestination, 4);
	gdi->YellowBrush();
	gdi->YellowPen();
	gdi->Circle(m_vDestination, 2);
	*/
}

