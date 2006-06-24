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
#include "BrainSideBase.h"

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

private:

	void WalkLeft();
	void WalkRight();
	void LookAround();

	BrainSideBase *m_pSideBase;
	CL_Vector2 m_force;
	float m_weight;
	unsigned int m_thinkTimer;

};
#endif // StateSideSimpleWalk_h__