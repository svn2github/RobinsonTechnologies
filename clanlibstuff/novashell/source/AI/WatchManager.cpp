#include "AppPrecomp.h"
#include "WatchManager.h"
#include "../MovingEntity.h"

WatchManager g_watchManager;


WatchManager::WatchManager()
{
}

WatchManager::~WatchManager()
{
}

void WatchManager::Clear()
{
	m_watchList.clear();
}

void WatchManager::Add(MovingEntity *pEnt, int timeToWatchMS)
{
	
	//is it already on the list?

	watch_list::iterator itor;
	for (itor=m_watchList.begin(); itor != m_watchList.end(); ++itor)
	{
		if (itor->m_entID == pEnt->ID())
		{
			itor->m_watchTimer = GetApp()->GetGameTick()+timeToWatchMS;
			return;
		}
		
	}
	
	WatchObject w;
	w.m_entID = pEnt->ID();
	w.m_watchTimer = GetApp()->GetGameTick()+timeToWatchMS;
	m_watchList.push_back(w);
}

void WatchManager::Update(float step, unsigned int drawID)
{
	
	//if any of these entities have not been processed yet, let's do it

	m_postUpdateList.clear();

	MovingEntity *pEnt;

	unsigned int gameTick = GetApp()->GetGameTick();

	watch_list::iterator itor;
	for (itor=m_watchList.begin(); itor != m_watchList.end();)
	{
		
		pEnt = (MovingEntity*)EntityMgr->GetEntityFromID(itor->m_entID);

		if (!pEnt)
		{
			//entity doesn't exist anymore I guess
			itor = m_watchList.erase(itor);
			continue;
		}

		//it's there, let's do our logic on it if it needs it

		if (pEnt->GetDrawID() != drawID)
		{
			
			if (itor->m_watchTimer < gameTick)
			{
				itor = m_watchList.erase(itor);
				continue;
			}

			//pEnt->GetTile()->SetLastScanID(0); //invalidate whatever was there
			pEnt->Update(step);
			//pEnt->SetDrawID(0);
			m_postUpdateList.push_back(pEnt);
		}

		itor++;
	}

}

void WatchManager::PostUpdate(float step)
{
	for (unsigned int i=0; i < m_postUpdateList.size(); i++)
	{
		m_postUpdateList.at(i)->PostUpdate(step);
	}

}
void WatchManager::OnEntityDeleted(MovingEntity *pEnt)
{
	//allows us to remove someone from our list when he's deleted to avoid a crash
	for (unsigned int i=0; i < m_postUpdateList.size(); i++)
	{
		//uh, is it possible this pointer could be bad?  Not sure
		if (pEnt == m_postUpdateList.at(i))
		{
			m_postUpdateList.at(i) = NULL;
		}
	}

}

void WatchManager::ProcessPendingEntityMovementAndDeletions()
{
	for (unsigned int i=0; i < m_postUpdateList.size(); i++)
	{
		//uh, is it possible this pointer could be bad?  Not sure
		if (m_postUpdateList.at(i))
		{
			m_postUpdateList.at(i)-> ProcessPendingMoveAndDeletionOperations();
		}
	}
}