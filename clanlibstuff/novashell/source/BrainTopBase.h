//  ***************************************************************
//  BrainTopBase - Creation date: 07/28/2006
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2006 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//this is a brain file.  By including it in the project,
//it will automatically register itself and be available
//from lua script as a brain behavior.

#ifndef BrainTopBase_h__
#define BrainTopBase_h__

#include "Brain.h"

class BrainTopBase: public Brain
{
public:
	BrainTopBase(MovingEntity *pParent);
	virtual ~BrainTopBase();
	virtual void Update(float step);
	virtual void PostUpdate(float step);

	virtual const char * GetName() {return "TopBase";};
	virtual Brain * CreateInstance(MovingEntity *pParent) {return new BrainTopBase(pParent);}
	virtual void OnAdd();

	//for use by other brains directly
	virtual void AddWeightedForce(const CL_Vector2 & force);

protected:

	void ResetForNextFrame();

private:

	CL_Vector2 m_force;
	float m_maxForce;
	float m_turnSpeed;
};

#endif // BrainTopBase_h__