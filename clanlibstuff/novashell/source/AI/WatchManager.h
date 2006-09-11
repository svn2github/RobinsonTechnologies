//  ***************************************************************
//  WatchManager - Creation date: 09/06/2006
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2006 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//watch entities and run their logic no matter where they are in the world

#ifndef WatchManager_h__
#define WatchManager_h__

class MovingEntity;

class WatchObject
{
public:

	unsigned int m_watchTimer;
	unsigned int m_entID;
};

typedef std::list<WatchObject> watch_list;

class WatchManager
{
public:
	WatchManager();
	virtual ~WatchManager();

	void Add(MovingEntity *pEnt, int timeToWatchMS);
	void Update(float step, unsigned int drawID);
	void PostUpdate(float step);
	void ProcessPendingEntityMovementAndDeletions();
	void OnEntityDeleted(MovingEntity *pEnt);
	void AddEntityToVisibilityList(MovingEntity *pEnt);
	void Clear();
	int GetWatchCount() {return m_watchList.size();}
	void RemoveFromVisibilityList(MovingEntity *pEnt);
	void Remove(MovingEntity *pEnt);

	unsigned int GetVisibilityID() {return m_visibilityID;}

protected:

	watch_list m_watchList;
	vector<MovingEntity*> m_postUpdateList;
	vector<MovingEntity*> m_visibilityNotificationList; //entities we need to check to see if they weren't rendered the next frame
	unsigned int m_visibilityID; //changes every update cycle

private:
};

extern WatchManager g_watchManager;
#endif // WatchManager_h__