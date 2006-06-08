#include "AppPrecomp.h"
#include "BrainManager.h"
#include "Brain.h"
#include "BrainPlayer.h"
#include "BrainShake.h"

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

void BrainManager::Add(int brainType)
{

	Brain *pNewBrain = NULL;

	switch (brainType)
		{
		case Brain::PLAYER_SIDE_VIEW:
			pNewBrain = new BrainPlayer(m_pParent);
			break;

		case Brain::SHAKE:
			pNewBrain = new BrainShake(m_pParent);
			break;
		
		default:

			LogMsg("Unknown brain type: %d", brainType);
		}
	
     m_brainVec.push_back(pNewBrain);

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