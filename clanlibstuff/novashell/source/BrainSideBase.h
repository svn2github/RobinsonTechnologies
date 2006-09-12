//  ***************************************************************
//  BrainSideBase - Creation date: 06/21/2006
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2006 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef BrainSideBase_h__
#define BrainSideBase_h__

#include "Brain.h"

class BrainSideBase: public Brain
{
public:
	BrainSideBase(MovingEntity * pParent);
	virtual ~BrainSideBase();

	virtual void Update(float step);
	virtual void PostUpdate(float step);
	virtual const char * GetName(){return "SideBase";};
	virtual Brain * CreateInstance(MovingEntity *pParent) {return new BrainSideBase(pParent);}
	virtual void OnAdd();

	//for use by other brains directly
	virtual void AddWeightedForce(const CL_Vector2 & force);

protected:

	void ResetForNextFrame();

private:

	CL_Vector2 m_force;
	float m_maxForce;

};

#endif // BrainSideBase_h__
