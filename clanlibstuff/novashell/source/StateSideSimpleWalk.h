//  ***************************************************************
//  StateSideSimpleWalk - Creation date: 06/24/2006
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2006 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef StateSideSimpleWalk_h__
#define StateSideSimpleWalk_h__

#include "State.h"

class StateSideSimpleWalk: public State
{
public:
	StateSideSimpleWalk(MovingEntity * pParent);
	virtual ~StateSideSimpleWalk();

	virtual void Update(float step);
	virtual void PostUpdate(float step);	
	virtual const char * GetName(){return "SideSimpleWalk";};
	virtual State * CreateInstance(MovingEntity *pParent) {return new StateSideSimpleWalk(pParent);}
	virtual void OnAdd();

protected:
	void TurnAround();
	void ResetSlowThinkTimer();

private:

	void LookAround();

	CL_Vector2 m_force;
	float m_weight;
	GameTimer m_thinkTimer, m_slowThinkTimer;

};
#endif // StateSideSimpleWalk_h__
