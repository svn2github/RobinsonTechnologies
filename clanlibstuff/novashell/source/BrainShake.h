//  ***************************************************************
//  BrainShake - Creation date: 06/07/2006
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2006 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef BrainShake_h__
#define BrainShake_h__

#include "Brain.h"

class BrainShake: public Brain
{
public:
	BrainShake(MovingEntity * pParent);
	virtual ~BrainShake();

	virtual void Update(float step);
	virtual void PostUpdate(float step);
	virtual const char * GetName(){return "Shake";};
	virtual Brain * CreateInstance(MovingEntity *pParent) {return new BrainShake(pParent);}

protected:

private:

	CL_Vector2 m_lastDisplacement;
	bool m_bFirstTime;

};

#endif // BrainShake_h__