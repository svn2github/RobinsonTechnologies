//  ***************************************************************
//  BrainManager - Creation date: 06/07/2006
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2006 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef BrainManager_h__
#define BrainManager_h__

class Brain;
class MovingEntity;

typedef list<Brain*> brain_vector;


class BrainManager
{
public:
	BrainManager();
	virtual ~BrainManager();
	void Add(int brainType);
	void SetParent(MovingEntity *pEnt) {m_pParent = pEnt;}

	void Kill();

	void Update(float step);
	void PostUpdate(float step);

protected:

private:

	MovingEntity *m_pParent;
	brain_vector m_brainVec;
};

#endif // BrainManager_h__