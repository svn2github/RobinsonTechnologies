//  ***************************************************************
//  StateDie - Creation date: 06/28/2006
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2006 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//this is a state file.  By including it in the project,
//it will automatically register itself and be available
//from lua script as a state behavior.

#ifndef StateDie_h__
#define StateDie_h__

#include "State.h"

class StateDie: public State
{
public:
	StateDie(MovingEntity *pParent);
	virtual ~StateDie();
	virtual void Update(float step);
	virtual void PostUpdate(float step);
	virtual const char * GetName() {return "Die";};
	virtual State * CreateInstance(MovingEntity *pParent) {return new StateDie(pParent);}
	virtual void OnAdd();
	virtual void OnRemove();

protected:
	

private:
};

#endif // StateDie_h__
