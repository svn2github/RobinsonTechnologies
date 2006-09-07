#include "AppPrecomp.h"
#include "Goal_Think.h"
#include "ObjectEnumerations.h"
#include "Goal_MoveToPosition.h"
#include "Goal_Types.h"
//#include "Goal_AttackTarget.h"
//#include "AttackTargetGoal_Evaluator.h"
#include "Goal_Delay.h"
#include "Goal_RunScript.h"

Goal_Think::Goal_Think(MovingEntity* pBot, const string &goalName):Goal_Composite<MovingEntity>(pBot, goal_think)
{

	SetName(goalName);
	//create the evaluator objects
//  m_Evaluators.push_back(new AttackTargetGoal_Evaluator(AttackBias));
}

//----------------------------- dtor ------------------------------------------
//-----------------------------------------------------------------------------
Goal_Think::~Goal_Think()
{
  GoalEvaluators::iterator curDes = m_Evaluators.begin();
  for (curDes; curDes != m_Evaluators.end(); ++curDes)
  {
    delete *curDes;
  }
}

//------------------------------- Activate ------------------------------------
//-----------------------------------------------------------------------------
void Goal_Think::Activate()
{
	  m_iStatus = active;
}

//------------------------------ Process --------------------------------------
//
//  processes the subgoals
//-----------------------------------------------------------------------------
int Goal_Think::Process()
{
  ActivateIfInactive();
  
  int SubgoalStatus = ProcessSubgoals();

 
  if (SubgoalStatus == completed || SubgoalStatus == failed)
  {

	if (m_SubGoals.size() == 1)	 
	{
		//this is the last thing on the last and it's done
		if (m_goalName == "Base")
		{
			//nothing else to do right now.  Kill the thing too
			//RemoveAllSubgoals();
		    m_iStatus = inactive;
		} else
		{
			m_iStatus = SubgoalStatus;
		}
		
	}
  }

  return m_iStatus;
}

//----------------------------- Update ----------------------------------------
// 
//  this method iterates through each goal option to determine which one has
//  the highest desirability.
//-----------------------------------------------------------------------------


//---------------------------- notPresent --------------------------------------
//
//  returns true if the goal type passed as a parameter is the same as this
//  goal or any of its subgoals
//-----------------------------------------------------------------------------
bool Goal_Think::notPresent(unsigned int GoalType)const
{
  if (!m_SubGoals.empty())
  {
    return m_SubGoals.front()->GetType() != GoalType;
  }

  return true;
}

void Goal_Think::PushMoveToPosition(CL_Vector2 pos)
{
	AddSubgoal( new Goal_MoveToPosition(m_pOwner, pos));
}


void Goal_Think::PushAttackTarget()
{
  if (notPresent(goal_attack_target))
  {
    RemoveAllSubgoals();
   // AddSubgoal( new Goal_AttackTarget(m_pOwner));
  }
}

void Goal_Think::Terminate()
{


}


void Goal_Think::PushRunScriptString(const string &scriptString)
{
	AddSubgoal( new Goal_RunScript(m_pOwner, scriptString));
}

void Goal_Think::PushDelay(int timeMS)
{
	AddSubgoal( new Goal_Delay(m_pOwner,  timeMS));

}

Goal_Think * Goal_Think::PushNewGoal(const string &goalName)
{
	Goal_Think *pGoal = new Goal_Think(m_pOwner, goalName);
	AddSubgoal( pGoal);
	return pGoal;
}
//-------------------------- Queue Goals --------------------------------------
//-----------------------------------------------------------------------------
void Goal_Think::QueueMoveToPosition(CL_Vector2 pos)
{
  m_SubGoals.push_back(new Goal_MoveToPosition(m_pOwner, pos));
}



//----------------------- RenderEvaluations -----------------------------------
//-----------------------------------------------------------------------------
void Goal_Think::RenderEvaluations(int left, int top)const
{
  /*
	gdi->TextColor(Cgdi::black);
  
  std::vector<Goal_Evaluator*>::const_iterator curDes = m_Evaluators.begin();
  for (curDes; curDes != m_Evaluators.end(); ++curDes)
  {
    (*curDes)->RenderInfo(CL_Vector2(left, top), m_pOwner);

    left += 75;
  }
  */
}

void Goal_Think::Render()
{
  std::list<Goal<MovingEntity>*>::iterator curG;
  for (curG=m_SubGoals.begin(); curG != m_SubGoals.end(); ++curG)
  {
    (*curG)->Render();
  }
}


   
