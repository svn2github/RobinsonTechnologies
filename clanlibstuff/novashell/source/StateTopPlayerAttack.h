//  ***************************************************************
//  StateTopPlayerAttack - Creation date: 07/24/2006
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2006 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//this is a state file.  By including it in the project,
//it will automatically register itself and be available
//from lua script as a state behavior.

#ifndef StateTopPlayerAttack_h__
#define StateTopPlayerAttack_h__

#include "State.h"

class StateTopPlayerAttack: public State
{
public:
	StateTopPlayerAttack(MovingEntity *pParent);
	virtual ~StateTopPlayerAttack();
	virtual void Update(float step);
	virtual void PostUpdate(float step);
	virtual const char * GetName() {return "TopPlayerAttack";};
	virtual State * CreateInstance(MovingEntity *pParent) {return new StateTopPlayerAttack(pParent);}
	virtual void OnAdd();
	virtual void OnRemove();

protected:
	

private:
};

#endif // StateTopPlayerAttack_h__