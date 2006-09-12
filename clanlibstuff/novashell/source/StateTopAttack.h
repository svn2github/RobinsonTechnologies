//  ***************************************************************
//  StateTopAttack - Creation date: 08/01/2006
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2006 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//this is a state file.  By including it in the project,
//it will automatically register itself and be available
//from lua script as a state behavior.

#ifndef StateTopAttack_h__
#define StateTopAttack_h__

#include "State.h"

class StateTopAttack: public State
{
public:
	StateTopAttack(MovingEntity *pParent);
	virtual ~StateTopAttack();
	virtual void Update(float step);
	virtual void PostUpdate(float step);
	virtual const char * GetName() {return "TopAttack";};
	virtual State * CreateInstance(MovingEntity *pParent) {return new StateTopAttack(pParent);}
	virtual void OnAdd();
	virtual void OnRemove();

protected:
	

private:
};

#endif // StateTopAttack_h__
