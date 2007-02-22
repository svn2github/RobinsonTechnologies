
/* -------------------------------------------------
* Copyright 2006 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 12:1:2006   15:21
*/


#ifndef WorldManager_HEADER_INCLUDED // include guard
#define WorldManager_HEADER_INCLUDED  // include guard

#include "AppPrecomp.h"
#include "Map.h"
#include "EntMapCache.h"

//things needed for each world map

class MapInfo
{
public:
	
	~MapInfo()
	{
		m_worldCache.ClearCache();
		m_world.SaveAndKill();
	}
	Map m_world;
	EntMapCache m_worldCache;
};

typedef std::list<MapInfo*> map_info_list;

class MapManager
{
public:

	MapManager();
	virtual ~MapManager();

	void Kill();
	bool AddMap(string stPath); //add it to our list without actually loading anything
	bool LoadMap(string stPath, bool bSetActiveIfNoneIs = true); //load main data file, with thumbnails
	void UnloadMapByName(const string &stName);

	Map * GetActiveWorld();
	EntMapCache * GetActiveMapCache();
	bool SetActiveMapByPath(const string &stPath, CameraSetting *pCameraSetting = NULL); //returns false if failed
	void Update(float step);
	void Render();
	MapInfo * GetMapInfoByPath(const string &stPath);
	MapInfo * GetMapInfoByName(const string &stName);
	map_info_list * GetWorldInfoList() {return &m_mapInfoList;}
	void ScanMaps(const string &stPath);
	bool SetActiveMapByName(const string &stName);
	void PreloadAllMaps();
	void SaveAllMaps();
	bool LoadMapByName(const string &stName);
	int GetTotalMapsAvailable() {return m_mapInfoList.size();}

	CL_Signal_v0 sig_map_changed;

protected:

	bool IsMapScanned(const string &stName);
	void ScanDirToAddMaps(const string &stPath, const string &stLocalPath);

	map_info_list m_mapInfoList;
	Map *m_pActiveMap;
	EntMapCache *m_pActiveMapCache;
};

bool ExistsInModPath(const string fName);

extern MapManager *g_pMapManager; //for speedy access

#endif                  // include guard
