#include "AppPrecomp.h"
#include "BrainManager.h"
#include "Brain.h"
#include "State.h"

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



StateRegistry::StateRegistry()
{

}

void StateRegistry::ListAllStates()
{
	LogMsg("Showing all %d valid State types.", m_stateMap.size());

	stateMap::iterator itor = m_stateMap.begin();
	while (itor != m_stateMap.end())
	{
		LogMsg(itor->second->GetName());
		itor++;
	}
}

State * StateRegistry::CreateStateByName(const string &stateName, MovingEntity *pParent)
{
	stateMap::iterator itor = m_stateMap.find(stateName);

	if (itor == m_stateMap.end())
	{
		LogError("State %s is unknown.  It's case sensitive.", stateName.c_str());
		ListAllStates();
		return NULL; //failed, it will handle  the error message later
	}

	return itor->second->CreateInstance(pParent);
}

StateRegistry * StateRegistry::GetInstance()
{
	static StateRegistry theInstance;
	return &theInstance;
}

void StateRegistry::Add(State *pState)
{
	stateMap::iterator itor = m_stateMap.find(pState->GetName());

	if (itor != m_stateMap.end())
	{
		LogError("State %s was already registered.  Only do this once.", pState->GetName());
		return;
	}

	m_stateMap[pState->GetName()] = pState;

}


BrainManager::BrainManager()
{
	m_pParent = NULL;
	m_pActiveState = NULL;
}

BrainManager::~BrainManager()
{
	
	//hmm, do we want to run its OnRemove() here?  Maybe not.
	SAFE_DELETE(m_pActiveState);
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

string BrainManager::SendToBrainByName(const string &brainName, const string &msg)
{
	Brain *pBrain = GetBrainByName(brainName);
	if (!pBrain)
	{
		LogMsg("Can't locate brain %s to send %s to it.", brainName.c_str(), msg.c_str());
		return "";
	}

	return pBrain->HandleMsg(msg);

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

State * BrainManager::SetState(State *pState)
{
	if (m_pActiveState)
	{
		m_pActiveState->OnRemove();
	}

	SAFE_DELETE(m_pActiveState);

	m_pActiveState = pState;
	return m_pActiveState;
}

State * BrainManager::SetStateByName(const string &stateName)
{
	return SetState(StateRegistry::GetInstance()->CreateStateByName(stateName, m_pParent));
}

bool BrainManager::InState(const string &stateName)
{
	 return (m_pActiveState && stateName == m_pActiveState->GetName());
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