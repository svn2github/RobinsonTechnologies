//  ***************************************************************
//  StateTopPlayerIdle - Creation date: 07/24/2006
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2006 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//this is a state file.  By including it in the project,
//it will automatically register itself and be available
//from lua script as a state behavior.

#ifndef StateTopPlayerIdle_h__
#define StateTopPlayerIdle_h__

#include "State.h"

class StateTopPlayerIdle: public State
{
public:
	StateTopPlayerIdle(MovingEntity *pParent);
	virtual ~StateTopPlayerIdle();
	virtual void Update(float step);
	virtual void PostUpdate(float step);
	virtual const char * GetName() {return "TopPlayerIdle";};
	virtual State * CreateInstance(MovingEntity *pParent) {return new StateTopPlayerIdle(pParent);}
	virtual void OnAdd();
	virtual void OnRemove();

protected:
	

private:
};

#endif // StateTopPlayerIdle_h__