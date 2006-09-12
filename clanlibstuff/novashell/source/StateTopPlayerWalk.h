//  ***************************************************************
//  StateTopPlayerWalk - Creation date: 07/24/2006
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2006 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//this is a state file.  By including it in the project,
//it will automatically register itself and be available
//from lua script as a state behavior.

#ifndef StateTopPlayerWalk_h__
#define StateTopPlayerWalk_h__

#include "State.h"

class StateTopPlayerWalk: public State
{
public:
	StateTopPlayerWalk(MovingEntity *pParent);
	virtual ~StateTopPlayerWalk();
	virtual void Update(float step);
	virtual void PostUpdate(float step);
	virtual const char * GetName() {return "TopPlayerWalk";};
	virtual State * CreateInstance(MovingEntity *pParent) {return new StateTopPlayerWalk(pParent);}
	virtual void OnAdd();
	virtual void OnRemove();

protected:
	

private:
};

#endif // StateTopPlayerWalk_h__
