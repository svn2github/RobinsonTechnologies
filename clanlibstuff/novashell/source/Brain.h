
/* -------------------------------------------------
* Copyright 2006 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 25:2:2006   15:05
*/


#ifndef Brain_HEADER_INCLUDED // include guard
#define Brain_HEADER_INCLUDED  // include guard

#include "MovingEntity.h"

class Brain
{
public:

	Brain(MovingEntity * pParent);
	virtual ~Brain();
	virtual void Update(float step) = NULL;
	virtual void PostUpdate(float step) = NULL;

	enum eBrainType
	{
	 PLAYER = 0
	};

protected:

	MovingEntity *m_pParent;
};

#endif                  // include guard
