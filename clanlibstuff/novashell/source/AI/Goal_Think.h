
#ifndef GOAL_THINK_H
#define GOAL_THINK_H

#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:   Goal_Think .h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   class to arbitrate between a collection of high level goals, and
//          to process those goals.
//-----------------------------------------------------------------------------
#include <vector>
#include <string>
#include "Goal_Composite.h"
#include "Goal_Evaluator.h"

class Goal_Think : public Goal_Composite<MovingEntity>
{
private:
  
  typedef std::vector<Goal_Evaluator*>   GoalEvaluators;

private:
  
  GoalEvaluators  m_Evaluators;

public:

  Goal_Think(MovingEntity* pBot,const string &goalName);
  ~Goal_Think();

  //this method iterates through each goal evaluator and selects the one
  //that has the highest score as the current goal

  //returns true if the given goal is not at the front of the subgoal list
  bool notPresent(unsigned int GoalType)const;

  //the usual suspects
  int  Process();
  void Activate();
  
  //top level goal types
  void PushMoveToPosition(CL_Vector2 pos);
  void PushAttackTarget();
  void PushRunScriptString(const string &scriptString);
  void PushDelay(int timeMS);
  Goal_Think * PushNewGoal(const string &goalName);

  //this adds the MoveToPosition goal to the *back* of the subgoal list.
  void QueueMoveToPosition(CL_Vector2 pos);

  //this renders the evaluations (goal scores) at the specified location
  void  RenderEvaluations(int left, int top)const;
  void  Render();
  void Terminate();
};


#endif