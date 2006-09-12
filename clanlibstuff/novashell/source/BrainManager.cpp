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
	m_pLastState = NULL;
	m_pBrainBase = NULL;
}

BrainManager::~BrainManager()
{
	
	//hmm, do we want to run its OnRemove() here?  Maybe not.
	
	Kill();	
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

	SAFE_DELETE(m_pActiveState);
	SAFE_DELETE(m_pLastState);
	m_pBrainBase = NULL;

}


bool compareBrainBySort(const Brain *pA, const Brain *pB) 
{
	return pA->GetSort() < pB->GetSort();
}

void BrainManager::Sort()
{
	m_brainVec.sort(compareBrainBySort);
}

Brain * BrainManager::Add(const string &brainName, const string &initMsg)
{

	Brain *pNewBrain = BrainRegistry::GetInstance()->CreateBrainByName(brainName, m_pParent);

	if (!pNewBrain)
	{
		LogMsg("Brain name %s was not found in the brain registry.  It's case sensitive.  Valid brains are:", brainName.c_str());
		BrainRegistry::GetInstance()->ListAllBrains();
		return NULL;
	}
	
     m_brainVec.push_back(pNewBrain);
	 Sort();

	 pNewBrain->OnAdd();
	 if (initMsg.size() > 0)
	 {
		 pNewBrain->HandleMsg(initMsg);
	 }

	 return pNewBrain; //just in case we want to fool with it
}

void BrainManager::SendToBrainByName(const string &brainName, const string &msg)
{
	Brain *pBrain = GetBrainByName(brainName);
	if (!pBrain)
	{
		LogMsg("Can't locate brain %s to send %s to it.", brainName.c_str(), msg.c_str());
		return;
	}

	pBrain->HandleMsg(msg);
}

void BrainManager::SendToBrainBase(const string &msg)
{
	if (!GetBrainBase())
	{
		LogError("No base brain assigned yet.");
		return;
	}

	GetBrainBase()->HandleMsg(msg);
}

string BrainManager::AskBrainByName(const string &brainName, const string &msg)
{
	Brain *pBrain = GetBrainByName(brainName);
	if (!pBrain)
	{
		LogMsg("Can't locate brain %s to ask it %s.", brainName.c_str(), msg.c_str());
		return "";
	}

	return pBrain->HandleAskMsg(msg);
}

Brain * BrainManager::GetBrainByName(const string &brainName)
{
	brain_vector::iterator itor = m_brainVec.begin();
	while (itor != m_brainVec.end())
	{
		if ( !(*itor)->GetDeleteFlag())
		if ( (*itor)->GetName() == brainName)
		{
			//found it
			return (*itor);
		}
		itor++;
	}

	return NULL; //couldn't find this brain
}
void BrainManager::SetBrainBase(Brain *pBrain)
{
	if (m_pBrainBase)
	{
		LogError("Uh, shouldn't you add code to remove brain %s first?", m_pBrainBase->GetName());
	}
	m_pBrainBase = pBrain;

	//notify everywhere the base brain changed here?
}

State * BrainManager::SetState(State *pState)
{
	if (!m_pBrainBase)
	{
		LogError("Must add a base brain before state %s can be set.", pState->GetName());
		return NULL;
	}
	

	if (!pState)
	{
		return NULL;
	}
	if (m_pActiveState)
	{
		m_pActiveState->OnRemove();
	}

	SAFE_DELETE(m_pLastState);
	m_pLastState = m_pActiveState; //save this for later queries, might be useful
	m_pActiveState = pState;
	m_pActiveState->OnAdd();
	return m_pActiveState;
}

State * BrainManager::SetStateByName(const string &stateName)
{
	return SetState(StateRegistry::GetInstance()->CreateStateByName(stateName, m_pParent));
}

const char * BrainManager::GetStateByName()
{
	if (!m_pActiveState)
	{
		return "";
	}
	return m_pActiveState->GetName();
}


bool BrainManager::InState(const string &stateName)
{
	 return (m_pActiveState && stateName == m_pActiveState->GetName());
}

bool BrainManager::LastStateWas(const string &stateName)
{
	return (m_pLastState && stateName == m_pLastState->GetName());
}

void BrainManager::Update(float step)
{

	if (m_pActiveState) m_pActiveState->Update(step);

	brain_vector::iterator itor = m_brainVec.begin();
	while (itor != m_brainVec.end())
	{
		if ( !(*itor)->GetDeleteFlag())
		{
			(*itor)->Update(step);
		}

		itor++;
	}


}

void BrainManager::PostUpdate(float step)
{

	if (m_pActiveState) m_pActiveState->PostUpdate(step);

	brain_vector::iterator itor = m_brainVec.begin();
	while (itor != m_brainVec.end())
	{
		
		if ((*itor)->GetDeleteFlag())
		{
			(*itor)->OnRemove();
			delete *itor;

			itor = m_brainVec.erase(itor);
			continue;
		} else
		{
			(*itor)->PostUpdate(step);

		}

		itor++;
	}

}
