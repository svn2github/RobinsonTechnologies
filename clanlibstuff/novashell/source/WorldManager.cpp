#include "AppPrecomp.h"
#include "world.h"
#include "EntWorldCache.h"
#include "WorldManager.h"
#include "GameLogic.h"

WorldManager::WorldManager()
{
	m_pActiveWorld = NULL;
	m_pActiveWorldCache = NULL;
	
}

WorldManager::~WorldManager()
{
	Kill();
}


void WorldManager::ScanWorlds(const string &stPath)
{
	
	Kill();

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
				AddWorld(stPath+scanner.get_name()+"/");
			}

		}
	}
}

void WorldManager::Kill()
{
	//clear each
	world_info_list::iterator itor =m_worldInfoList.begin();
	while (itor != m_worldInfoList.end())
	{
		delete *itor;
		itor++;
	}

	m_worldInfoList.clear();
	m_pActiveWorld = NULL;
	m_pActiveWorldCache = NULL;

	GetTagManager->Kill();
}


bool WorldManager::AddWorld(string stPath)
{
	if (GetWorldInfoByPath(stPath)) return false; //already existed
	
	WorldInfo *pWorldInfo = new WorldInfo;
	pWorldInfo->m_world.SetDirPath(stPath);
	//precache its tagged entity info
	GetTagManager->Load(&pWorldInfo->m_world);

	m_worldInfoList.push_back(pWorldInfo);

	return true;
}

bool WorldManager::LoadWorld(string stPath)
{
	WorldInfo *pWorldInfo = GetWorldInfoByPath(stPath);

	if (!pWorldInfo) 
	{
		throw CL_Error("Couldn't load world " + stPath);
		return false;
	}

	pWorldInfo->m_world.Load(stPath);
	pWorldInfo->m_worldCache.SetWorld(&pWorldInfo->m_world);

	if (!m_pActiveWorld)
	{
		SetActiveWorldByPath(stPath);
	}
 	return true;
}

WorldInfo * WorldManager::GetWorldInfoByPath(const string &stPath)
{
	world_info_list::iterator itor =m_worldInfoList.begin();
	while (itor != m_worldInfoList.end())
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


WorldInfo * WorldManager::GetWorldInfoByName(const string &stName)
{
	world_info_list::iterator itor =m_worldInfoList.begin();
	while (itor != m_worldInfoList.end())
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

bool WorldManager::SetActiveWorldByName(const string &stName)
{
	WorldInfo *pWorldInfo = GetWorldInfoByName(stName);
	if (!pWorldInfo) 
	{
		LogMsg("World %s not found", stName.c_str());
		return false;
	}

	SetActiveWorldByPath(pWorldInfo->m_world.GetDirPath());
	return true;
}

//if pCameraSettings isn't null, we just use it and don't allow maps to 'remember'.  If null,
//it's probably the editor and we DO want to remember with that

bool WorldManager::SetActiveWorldByPath(const string &stPath, CameraSetting *pCameraSetting) 
{

WorldInfo *pWorldInfo = GetWorldInfoByPath(stPath);
	
	if (m_pActiveWorld)
	{
		
		if (&pWorldInfo->m_world == m_pActiveWorld)
		{
			//we're switching to the same world?
			LogMsg("Swithing world focus to ourself?");
			return true;
		}
		if (!pCameraSetting)
		{
		//let the old focus remember what its camera settings are
		GetCamera->GetCameraSettings(*m_pActiveWorld->GetCameraSetting());
		}
	}

	if (pWorldInfo)
	{
		//found it
		m_pActiveWorld = &pWorldInfo->m_world;
		m_pActiveWorldCache = &pWorldInfo->m_worldCache;

		if (!m_pActiveWorld->IsInitted())
		{
			LoadWorld(stPath);
			GetWorld->PreloadMap(); //later we might want to do this...

		}
		if (!pCameraSetting)
		{
			GetCamera->SetCameraSettings(*m_pActiveWorld->GetCameraSetting());
		} else
		{
			GetCamera->SetInstantUpdateOnNextFrame(true);
			GetCamera->Update(0);
		}
		
	    sig_map_changed(); //broadcast this to anybody who is interested
	}

	return false;
}

void WorldManager::Update(float step)
{
	if (m_pActiveWorldCache)
	m_pActiveWorldCache->Update(step);
}

void WorldManager::Render()
{
    if (m_pActiveWorldCache) m_pActiveWorldCache->Render(GetApp()->GetMainWindow()->get_gc());
}

EntWorldCache * WorldManager::GetActiveWorldCache()
{
	assert(m_pActiveWorldCache && "Uh oh");
	return m_pActiveWorldCache;
}
World* WorldManager::GetActiveWorld()
{
	//assert(m_pActiveWorld && "Uh oh");
  return m_pActiveWorld;
}
