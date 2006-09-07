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
	m_iStatus = active;
	//make sure the subgoal list is clear.
	RemoveAllSubgoals();



	if (!m_bTriedSimpleWay && m_pDestMap == m_pOwner->GetMap())
	{
		m_bTriedSimpleWay = true;

		LogMsg("Trying simple");
		//we don't reall need this fancy system?
		AddSubgoal(new Goal_MoveToPosition(m_pOwner, m_vDestination));
		return;
		//however, if this fails, we'll need it.
	}


	//the destination is on a different map.  We need to sketch out a path to get to that map.
	LogMsg("Planning the super!");


	m_macroPath = g_worldNavManager.FindPathToMapAndPos(m_pOwner, m_pDestMap, m_vDestination);
	
	if (!m_macroPath.IsValid())
	{
		m_iStatus = failed;
		LogMsg("Failed to find valid path between maps");
		return;
	}
	/*
	bool bUsePath = m_pOwner->GetPathPlanner()->IsPathNeededToGetToTarget(m_vDestination);

	if (bUsePath)
	{

		//compute path
		if (m_pOwner->GetPathPlanner()->RequestPathToPosition(m_vDestination))
		{
			//we can do it?

			int result;

			//this is setup to do searches over time, but for now, let's just do the whole search now
			//and not mess with getting messages later about it
			do 
			{
				result = m_pOwner->GetPathPlanner()->CycleOnce();
			} while( result == search_incomplete );

			//examine results

			switch (result)
			{

			case target_not_found:
				LogMsg("Couldn't locate target.  Doing blind seek instead.");
				bUsePath = false;
				break;

			case target_found:
				//			  LogMsg("Found it");
				break;

			default:
				assert(!"Unknown search result");
			}

		} else
		{
			LogMsg("Error computing path?");
			bUsePath = false;
		}
	}


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

//------------------------------ Process --------------------------------------
//-----------------------------------------------------------------------------
int Goal_MoveToMapAndPos::Process()
{
	//if status is inactive, call Activate()
	ActivateIfInactive();

	//process the subgoals
	m_iStatus = ProcessSubgoals();

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

