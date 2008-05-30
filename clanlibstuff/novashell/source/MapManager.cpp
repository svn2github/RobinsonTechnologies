#include "AppPrecomp.h"
#include "Map.h"
#include "EntMapCache.h"
#include "MapManager.h"
#include "GameLogic.h"
#include "AI/WorldNavManager.h"
#include "AI/WatchManager.h"

MapManager *g_pMapManager; //global pointer for speed

MapManager::MapManager()
{
	m_pActiveMap = NULL;
	m_pActiveMapCache = NULL;
	g_pMapManager = this;

}

MapManager::~MapManager()
{
	Kill();
	g_pMapManager = NULL;
}

void MapManager::ScanDirToAddMaps(const string &stPath, const string &stLocalPath)
{
	//scan map directory for available maps
	CL_DirectoryScanner scanner;

	scanner.scan(stPath, "*");
	while (scanner.next())
	{
		std::string file = scanner.get_name();
		if (scanner.is_directory())
		{
			if (scanner.get_name()[0] != '_')
				if (scanner.get_name()[0] != '.')
				{
					//no underscore at the start, let's show it

					if (!IsMapScanned(scanner.get_name()))
					{
						AddMap(stLocalPath+scanner.get_name());
					} else
					{
						//LogMsg("Ignoring additional %s", scanner.get_name().c_str());
					}
				}

		}
	}

}

bool MapManager::IsMapScanned(const string &stName)
{
	return (GetMapInfoByName(stName) != NULL);
}

void MapManager::ScanMaps(const string &stPath)
{
	
	Kill();
	g_worldNavManager.Init();

	vector<string> vecPaths;
	g_VFManager.GetMountedDirectories(&vecPaths);

	vector<string>::reverse_iterator itor = vecPaths.rbegin();

	for (;itor != vecPaths.rend(); itor++)
	{
		ScanDirToAddMaps( (*itor) +"/"+stPath, stPath);
	}

	g_worldNavManager.Load();

}

void MapManager::Kill()
{

	//clear each
	while (m_mapInfoList.begin() != m_mapInfoList.end())
	{
		delete *m_mapInfoList.begin();
		m_mapInfoList.pop_front();
	}
	
	m_mapInfoList.clear();
	g_watchManager.Clear();
	m_pActiveMap = NULL;
	m_pActiveMapCache = NULL;

	g_TagManager.Kill();
}


bool MapManager::AddMap(string stPath)
{

	//add the trailing backslash if required

	if (stPath[stPath.size()-1] != '/')
	{
		stPath += "/";
	}

	if (GetMapInfoByPath(stPath)) return false; //already existed
	
	MapInfo *pWorldInfo = new MapInfo;
	pWorldInfo->m_world.SetDirPath(stPath);
	
	//precache its tagged entity info, but only if it's in the dir where the world is
	
	if (ExistsInModPath(stPath+"/"+C_TAGCACHE_FILENAME)
		|| !ExistsInModPath(stPath+"/"+C_MAP_DAT_FILENAME)
		)
	{
		//basically, we only care what's in this data if:
		//1. It's in our world path (last mounted)
		//2. We don't have a local copy of that world.  If we did, we wouldn't
		//care what something we were modding has to say about it

		g_TagManager.Load(&pWorldInfo->m_world);
	}
	
	m_mapInfoList.push_back(pWorldInfo);

	return true;
}

bool MapManager::LoadMapByName(const string &stName)
{
	MapInfo *pWorldInfo = GetMapInfoByName(stName);

	if (pWorldInfo) return false;

	return LoadMap(pWorldInfo->m_world.GetDirPath(), false);
}

bool MapManager::LoadMap(string stPath, bool bSetActiveIfNoneIs)
{
	MapInfo *pWorldInfo = GetMapInfoByPath(stPath);

	if (!pWorldInfo) 
	{
		throw CL_Error("Couldn't load map \"" + stPath+"\"");
		return false;
	}

	if (pWorldInfo->m_world.IsInitted())
	{
		LogMsg("Why load %s, it's already initted!", m_pActiveMap->GetName().c_str());
		return false;
	}

	if (m_pActiveMap == &pWorldInfo->m_world)
	{
#ifdef _DEBUG
		LogMsg("Loading active map %s", stPath.c_str());
#endif	
		
		//Um, we are reloading the world that is currently on the screen.  We better
		//let people know so the cached data will be reset etc
		m_pActiveMap = NULL;
		m_pActiveMapCache = NULL;
		sig_map_changed(); //broadcast this to anybody who is interested
	} else
	{
#ifdef _DEBUG
		LogMsg("Loading map %s", stPath.c_str());
#endif	
	}

	pWorldInfo->m_world.Load(stPath);
	pWorldInfo->m_worldCache.SetWorld(&pWorldInfo->m_world);

	if (!m_pActiveMap && bSetActiveIfNoneIs)
	{
		SetActiveMapByPath(stPath);
	}
 	return true;
}

MapInfo * MapManager::GetMapInfoByPath(const string &stPath)
{
	map_info_list::iterator itor =m_mapInfoList.begin();
	while (itor != m_mapInfoList.end())
	{
		if ( (*itor)->m_world.GetDirPath() == stPath)
		{
			return *itor;
		}
		itor++;
	}

	//failed
	return NULL;
}

void MapManager::PreloadAllMaps()
{
	map_info_list::iterator itor =m_mapInfoList.begin();
	while (itor != m_mapInfoList.end())
	{
		SetActiveMapByPath((*itor)->m_world.GetDirPath());
		itor++;
	}
}

bool ExistsInModPath(const string fName)
{
	string fNameWithPath = fName;
	
	if (!g_VFManager.LocateFile(fNameWithPath))
	{
		return false;
	}
	
	if (fNameWithPath.compare(0, g_VFManager.GetLastMountedDirectory().size(), g_VFManager.GetLastMountedDirectory()) == 0)
	{
		return true;
	}

	return false;
}

void MapManager::SaveAllMaps()
{
	map_info_list::iterator itor =m_mapInfoList.begin();
	while (itor != m_mapInfoList.end())
	{
		if (ExistsInModPath((*itor)->m_world.GetDirPath() + C_MAP_DAT_FILENAME))
		{

			//LogMsg("Save %s?", (*itor)->m_world.GetDirPath().c_str());
			(*itor)->m_world.ForceSaveNow();

			//might as well do some housecleaning while we're at it?  
		//	(*itor)->m_world.RemoveUnusedFileChunks();

		}
		itor++;
	}
}

MapInfo * MapManager::GetMapInfoByName(const string &stName)
{
	map_info_list::iterator itor =m_mapInfoList.begin();
	while (itor != m_mapInfoList.end())
	{
		if ( (*itor)->m_world.GetName() == stName)
		{
			return *itor;
		}
		itor++;
	}

	//failed
	return NULL;
}
void MapManager::UnloadMapByName(const string &stName)
{


	
	map_info_list::iterator itor =m_mapInfoList.begin();
	bool bMapChanged = false;
	
	string worldPath;

	while (itor != m_mapInfoList.end())
	{
		if ( (*itor)->m_world.GetName() == stName)
		{

			worldPath = (*itor)->m_world.GetDirPath();
			if ( ! (*itor)->m_world.IsInitted())
			{
				LogMsg("Ignoring UnloadMapByName, %s isn't loaded yet.", stName.c_str());
				return;
			}
			
			if (GetActiveMap() == &(*itor)->m_world)
			{
				//we're about to delete something currently active, let the rest of the world know
#ifdef _DEBUG
				LogMsg("Unloading active map %s changed", stName.c_str());
#endif
				bMapChanged = true;
			} else
			{

#ifdef _DEBUG
				LogMsg("Unload map %s", stName.c_str());
#endif	
			}
			
			delete *itor;
			m_mapInfoList.erase(itor);
			break;

		}
		itor++;
	}
	
	if (!worldPath.empty())
	{
		//well, even though we deleted it, let's put it back without loading it so future things can find it by name
		AddMap(worldPath);
	}

	if (bMapChanged)
	{
		m_pActiveMap = NULL;
		m_pActiveMapCache = NULL;
		sig_map_changed(); //broadcast this to anybody who is interested
	}
	
	return;

}

bool MapManager::SetActiveMapByName(const string &stName)
{
	MapInfo *pWorldInfo = GetMapInfoByName(stName);
	if (!pWorldInfo) 
	{
		LogMsg("Map %s not found", stName.c_str());
		return false;
	}

	SetActiveMapByPath(pWorldInfo->m_world.GetDirPath());
	return true;
}


//if pCameraSettings isn't null, we just use it and don't allow maps to 'remember'.  If null,
//it's probably the editor and we DO want to remember with that

bool MapManager::SetActiveMapByPath(const string &stPath, CameraSetting *pCameraSetting) 
{

MapInfo *pWorldInfo = GetMapInfoByPath(stPath);
	
	if (m_pActiveMap)
	{
		
		if (&pWorldInfo->m_world == m_pActiveMap)
		{
			//we're switching to the same world?
			LogMsg("Switching world focus to ourself?");
			return true;
		}
		if (!pCameraSetting)
		{
		//let the old focus remember what its camera settings are
		*m_pActiveMap->GetCameraSetting() = GetCamera->GetCameraSettings();
		}
	}

	if (pWorldInfo)
	{
		//found it
		m_pActiveMap = &pWorldInfo->m_world;
		m_pActiveMapCache = &pWorldInfo->m_worldCache;

		if (!m_pActiveMap->IsInitted())
		{
			LoadMap(stPath);
			g_pMapManager->GetActiveMap()->PreloadMap(); //later we might not want to do this...
		}

		if (!pCameraSetting)
		{
			GetCamera->SetCameraSettings(*m_pActiveMap->GetCameraSetting());
		} else
		{
			GetCamera->SetCameraSettings(*pCameraSetting);
			GetCamera->SetInstantUpdateOnNextFrame(true);
			GetCamera->Update(1);
		}
		
	    sig_map_changed(); //broadcast this to anybody who is interested

		//LogMsg("Switched map at %u", GetApp()->GetGameTick());
		//also let all entities on the watch listen know
		g_watchManager.OnMapChange(m_pActiveMap->GetName());

	}

	return false;
}

void MapManager::Update(float step)
{
	if (m_pActiveMapCache)
	m_pActiveMapCache->Update(step);
}

void MapManager::Render()
{
    if (m_pActiveMapCache) m_pActiveMapCache->Render(GetApp()->GetMainWindow()->get_gc());
}

EntMapCache * MapManager::GetActiveMapCache()
{
	return m_pActiveMapCache;
}
Map* MapManager::GetActiveMap()
{
  return m_pActiveMap;
}

/*
Object: MapManager
Handles loading and unloading maps.

About:

This is a global object that can always be accessed.

Usage:

(code)
GetMapManager:SetActiveMapByName("Main");
(end)

Group: Member Functions

func: LoadMapByName
(code)

boolean LoadMapByName(string mapName)
(end)

Loading a <Map> causes the entire thing to be preloaded into memory.

In the future, you will be able to also enable map-streaming. (load as you go)

Note:

There is no "Save Map" command because <Map>'s are automatically saved when changes are made, unless the map has requested otherwise. See <Map::SetAutoSave>.

Parameters:

mapName - The name of the map directory you want to load.

Returns:

True if the map was found and loaded successfully.


func: UnloadMapByName
(code)
nil UnloadMapByName(string mapName)
(end)

Unload a <Map> causes all resources associated with it to be freed.

Parameters:

mapName - The name of the map directory you want to unload.


func: SetActiveMapByName
(code)
boolean SetActiveMapByName(string mapName)
(end)

Causes a <Map> to receive "focus" and be shown on the screen.

Parameters:

mapName - The name of the map you want the <Camera> to focus on.

Returns:

True if the <Map> was made active successfully.

func: GetActiveMap
(code)
Map GetActiveMap()
(end)

Returns:

A <Map> object interface of the <Map> currently being displayed.
*/