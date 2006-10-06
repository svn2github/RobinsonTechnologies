
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
  void AddRunScriptString(const string &scriptString);

  void AddApproachAndSay(const string &msg, int entToFaceID, int distanceRequired);
  void PushApproachAndSay(const string &msg, int entToFaceID, int distanceRequired);

  void PushDelay(int timeMS);
  void AddDelay(int timeMS);
  void PushApproach(int entToFaceID, int distanceRequired);
  void AddApproach(int entToFaceID, int distanceRequired);
  Goal_Think * PushNewGoal(const string &goalName);
  Goal_Think * AddNewGoal(const string &goalName);

  //this adds the MoveToPosition goal to the *back* of the subgoal list.
  void AddMoveToPosition(CL_Vector2 pos);
  void PushMoveToTag(TagObject *pTag);
  void AddMoveToTag(TagObject *pTag);

  void AddSay(const string &msg, int entToFaceID);
  void AddSayByID(const string &msg, int entID, int entToFaceID);

  //the entToFaceID can be an entity ID or a direction (FACING_LEFT, etc) or FACING_NONE to not look anywhere
  void PushSay(const string &msg, int entToFaceID);
  void PushSayByID(const string &msg, int entID, int entToFaceID);

  //this renders the evaluations (goal scores) at the specified location
  void  RenderEvaluations(int left, int top)const;
  void  Render();
  void Terminate();
};


#endif
