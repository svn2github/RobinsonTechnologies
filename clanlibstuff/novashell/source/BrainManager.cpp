#include "AppPrecomp.h"
#include "BrainManager.h"
#include "Brain.h"

BrainRegistry::BrainRegistry()
{
	
}

void BrainRegistry::ListAllBrains()
{
	LogMsg("Showing all %d valid brain types.", m_brainMap.size());

	brainMap::iterator itor = m_brainMap.begin();
	while (itor != m_brainMap.end())
	{
		LogMsg(itor->second->GetName());
		itor++;
	}
}

Brain * BrainRegistry::CreateBrainByName(const string &brainName, MovingEntity *pParent)
{
	brainMap::iterator itor = m_brainMap.find(brainName);

	if (itor == m_brainMap.end())
	{
		return NULL; //failed, it will handle  the error message later
	}

	return itor->second->CreateInstance(pParent);
}

BrainRegistry * BrainRegistry::GetInstance()
{
	static BrainRegistry theInstance;
	return &theInstance;
}

void BrainRegistry::Add(Brain *pBrain)
{
	brainMap::iterator itor = m_brainMap.find(pBrain->GetName());

	if (itor != m_brainMap.end())
	{
		LogError("Brain %s was already registered.  Only do this once.", pBrain->GetName());
		return;
	}

	m_brainMap[pBrain->GetName()] = pBrain;

}

BrainManager::BrainManager()
{
	m_pParent = NULL;
}

BrainManager::~BrainManager()
{
}

void BrainManager::Kill()
{
	
	brain_vector::iterator itor = m_brainVec.begin();
	while (itor != m_brainVec.end())
	{

		delete *itor;
		itor++;
	}

	m_brainVec.clear();

}

bool compareBrainBySort(const Brain *pA, const Brain *pB) 
{
	return pA->GetSort() < pB->GetSort();
}

void BrainManager::Sort()
{
	m_brainVec.sort(compareBrainBySort);
}

void BrainManager::Add(const string &brainName)
{

	Brain *pNewBrain = BrainRegistry::GetInstance()->CreateBrainByName(brainName, m_pParent);

	if (!pNewBrain)
	{
		LogMsg("Brain name %s was not found in the brain registry.  It's case sensitive.  Valid brains are:", brainName.c_str());
		BrainRegistry::GetInstance()->ListAllBrains();
		return;
	}
	
     m_brainVec.push_back(pNewBrain);
	 Sort();

	 pNewBrain->OnAdd();
}

Brain * BrainManager::GetBrainByName(const string &brainName)
{
	brain_vector::iterator itor = m_brainVec.begin();
	while (itor != m_brainVec.end())
	{

		if ( (*itor)->GetName() == brainName)
		{
			//found it
			return (*itor);
		}
		itor++;
	}

	return NULL; //couldn't find this brain
}

void BrainManager::Update(float step)
{
	brain_vector::iterator itor = m_brainVec.begin();
	while (itor != m_brainVec.end())
	{

		(*itor)->Update(step);
		itor++;
	}

}

void BrainManager::PostUpdate(float step)
{
	brain_vector::iterator itor = m_brainVec.begin();
	while (itor != m_brainVec.end())
	{

		(*itor)->PostUpdate(step);
		itor++;
	}

}