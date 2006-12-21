
/* -------------------------------------------------
* Copyright 2006 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 12:1:2006   15:21
*/


#ifndef WorldManager_HEADER_INCLUDED // include guard
#define WorldManager_HEADER_INCLUDED  // include guard

#include "AppPrecomp.h"
#include "World.h"
#include "EntWorldCache.h"

//things needed for each world map

class WorldInfo
{
public:
	
	~WorldInfo()
	{
		m_worldCache.ClearCache();
		m_world.SaveAndKill();
	}
	World m_world;
	EntWorldCache m_worldCache;
};

typedef std::list<WorldInfo*> world_info_list;

class WorldManager
{
public:

	WorldManager();
	virtual ~WorldManager();

	void Kill();
	bool AddWorld(string stPath); //add it to our list without actually loading anything
	bool LoadWorld(string stPath, bool bSetActiveIfNoneIs = true); //load main data file, with thumbnails
	void UnloadWorldByName(const string &stName);

	World * GetActiveWorld();
	EntWorldCache * GetActiveWorldCache();
	bool SetActiveWorldByPath(const string &stPath, CameraSetting *pCameraSetting = NULL); //returns false if failed
	void Update(float step);
	void Render();
	WorldInfo * GetWorldInfoByPath(const string &stPath);
	WorldInfo * GetWorldInfoByName(const string &stName);
	world_info_list * GetWorldInfoList() {return &m_worldInfoList;}
	void ScanWorlds(const string &stPath);
	bool SetActiveWorldByName(const string &stName);
	void PreloadAllMaps();
	void SaveAllMaps();
	bool LoadWorldByName(const string &stName);

	CL_Signal_v0 sig_map_changed;

protected:

	bool IsWorldScanned(const string &stName);
	void ScanDirToAddWorlds(const string &stPath, const string &stLocalPath);

	world_info_list m_worldInfoList;
	World *m_pActiveWorld;
	EntWorldCache *m_pActiveWorldCache;
};

bool ExistsInModPath(const string fName);

#endif                  // include guard
