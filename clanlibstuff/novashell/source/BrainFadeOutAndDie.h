//  ***************************************************************
//  BrainFadeOutAndDie - Creation date: 06/27/2006
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2006 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef BrainFadeOutAndDie_h__
#define BrainFadeOutAndDie_h__

#include "Brain.h"

class BrainFadeOutAndDie: public Brain
{
public:
	BrainFadeOutAndDie(MovingEntity *pParent);
	virtual ~BrainFadeOutAndDie();
	virtual void Update(float step);
	virtual const char * GetName() {return "FadeOutAndDelete";};
	virtual Brain * CreateInstance(MovingEntity *pParent) {return new BrainFadeOutAndDie(pParent);}

protected:
	

private:

  int m_fadeOutTimeMS;
  int m_timeCreated;
};

#endif // BrainFadeOutAndDie_h__