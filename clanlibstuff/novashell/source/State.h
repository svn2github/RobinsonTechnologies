//  ***************************************************************
//  State - Creation date: 06/23/2006
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2006 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef State_h__
#define State_h__

class MovingEntity;

#include "AppUtils.h"
#include "VisualProfileManager.h"
#include "MovingEntity.h"
#include "BrainManager.h"
#include "Brain.h"

class State
{
public:

	State(MovingEntity * pParent);
	virtual ~State();
	virtual void Update(float step){};
	virtual void PostUpdate(float step){};
	virtual const char * GetName()=0;
	virtual State * CreateInstance(MovingEntity *pParent)=0;
	virtual void OnAdd(){}; //called once when State is added
	virtual void OnRemove(){}; //called once when State is removed
	virtual string HandleMsg(const string &msg) {return "";}

protected:

	bool AnimIsLooping();
	void RegisterClass();

	MovingEntity *m_pParent;
};

#endif // State_h__