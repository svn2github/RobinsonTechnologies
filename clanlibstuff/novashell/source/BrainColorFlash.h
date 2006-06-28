//  ***************************************************************
//  BrainColorFlash - Creation date: 06/28/2006
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2006 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//this is a brain file.  By including it in the project,
//it will automatically register itself and be available
//from lua script as a brain behavior.

#ifndef BrainColorFlash_h__
#define BrainColorFlash_h__

#include "Brain.h"
#include "AppUtils.h"

class BrainColorFlash: public Brain
{
public:
	BrainColorFlash(MovingEntity *pParent);
	virtual ~BrainColorFlash();
	virtual void Update(float step);
	virtual const char * GetName() {return "ColorFlash";};
	virtual Brain * CreateInstance(MovingEntity *pParent) {return new BrainColorFlash(pParent);}

protected:
	

private:

	GameTimer m_flashTimer;
	bool m_bOn;
	CL_Color m_color;
};

#endif // BrainColorFlash_h__