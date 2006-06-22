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

//This is setup so if a Brain .cpp file is compiled, it automatically can add itself
//to our brain registry without any changes in here.  This shared between all classes
//and never modified during the duration of the game.

typedef std::map<string, Brain*> brainMap;

class BrainRegistry
{
public:
	void Add(Brain *pBrain);
	void ListAllBrains();
	static BrainRegistry * GetInstance();
	Brain * CreateBrainByName(const string &brainName, MovingEntity * pParent);

private:

	BrainRegistry(); //private constructor, must be access though BrainRegistry::GetInstance, to
	//avoid problems with order of instantiation (singleton pattern)

	brainMap m_brainMap; //we store one of each brain to get info from it later
};

class BrainManager
{
public:
	BrainManager();
	virtual ~BrainManager();
	void Add(const string &brainName);
	void SetParent(MovingEntity *pEnt) {m_pParent = pEnt;}

	void Kill();

	void Update(float step);
	void PostUpdate(float step);

	Brain * GetBrainByName(const string &brainName);

protected:

	void Sort(); 

private:

	MovingEntity *m_pParent;
	brain_vector m_brainVec;
};

#endif // BrainManager_h__