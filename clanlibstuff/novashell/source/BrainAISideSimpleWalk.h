//  ***************************************************************
//  BrainAISideSimpleWalk - Creation date: 06/19/2006
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2006 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef BrainAISideSimpleWalk_h__
#define BrainAISideSimpleWalk_h__

#include "Brain.h"

class BrainSideBase;

class BrainAISideSimpleWalk: public Brain
{
public:
	BrainAISideSimpleWalk(MovingEntity * pParent);
	virtual ~BrainAISideSimpleWalk();

	virtual void Update(float step);
	virtual void PostUpdate(float step);	
	virtual const char * GetName(){return "SideSimpleWalk";};
	virtual Brain * CreateInstance(MovingEntity *pParent) {return new BrainAISideSimpleWalk(pParent);}
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

#endif // BrainAISideSimpleWalk_h__