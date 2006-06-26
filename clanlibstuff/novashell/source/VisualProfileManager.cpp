#include "AppPrecomp.h"
#include "VisualProfileManager.h"
#include "GameLogic.h"



VisualProfileManager::VisualProfileManager()
{
}

VisualProfileManager::~VisualProfileManager()
{
	Kill();
}

void VisualProfileManager::Kill()
{

	VisualResourceMap::iterator ent = m_hashedResourceMap.begin();
	for (; ent != m_hashedResourceMap.end(); ++ent)
	{
		delete (*ent).second;
	}

	m_hashedResourceMap.clear();
}
VisualResource * VisualProfileManager::GetVisualResourceByHashedID(unsigned int resourceID)
{
	VisualResourceMap::iterator itor = m_hashedResourceMap.find(resourceID);

	if (itor == m_hashedResourceMap.end()) 
	{
		return NULL;
	}

	return (*itor).second;
}

VisualResource * VisualProfileManager::GetVisualResource(const string &fileName)
{
	unsigned int resourceID = FileNameToID(fileName.c_str());

	VisualResource *pRes = GetVisualResourceByHashedID(resourceID);

	if (pRes) return pRes;

	//if we got here, it means this resource hasn't been loaded yet and we need to load and init it
	
	pRes = new VisualResource;
	if (!pRes || !pRes->Init(fileName))
	{
		SAFE_DELETE(pRes);
		LogMsg("Unable to load visual profile %s", fileName.c_str());
		return NULL;
	}

	//add it to our map
	VisualResourceMap::iterator itor = m_hashedResourceMap.find(resourceID);

	if (itor == m_hashedResourceMap.end()) 
	{
		m_hashedResourceMap.insert(std::make_pair(resourceID, pRes));
	} else
	{
		throw CL_Error("Hash conflict with VisualResource " + fileName);
	}

	return pRes; //error
}




//utils

CL_Vector2 FacingToVector(int facing)
{
	switch (facing)	
	{
	case VisualProfile::FACING_LEFT:
		return CL_Vector2(-1,0);

	case VisualProfile::FACING_RIGHT:
		return CL_Vector2(1,0);

	default:

		throw CL_Error("Unknown facing");
		break;
	}

	return CL_Vector2(0,0);
}
