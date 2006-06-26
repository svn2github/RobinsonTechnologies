//  ***************************************************************
//  StateSideSimpleIdle - Creation date: 06/25/2006
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2006 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef StateSideSimpleIdle_h__
#define StateSideSimpleIdle_h__

#include "State.h"

class StateSideSimpleIdle: public State
{
public:
	StateSideSimpleIdle(MovingEntity * pParent);
	virtual ~StateSideSimpleIdle();
	virtual void OnAdd();
	virtual void PostUpdate(float step);	
	virtual const char * GetName(){return "SideSimpleIdle";};
	virtual State * CreateInstance(MovingEntity *pParent) {return new StateSideSimpleIdle(pParent);}

protected:
	
	GameTimer m_thinkTimer;

private:
};

#endif // StateSideSimpleIdle_h__