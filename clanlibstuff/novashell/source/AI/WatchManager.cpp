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
	m_postUpdateList.clear();
	m_visibilityNotificationList.clear();
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

void WatchManager::Remove(MovingEntity *pEnt)
{
	watch_list::iterator itor;
	for (itor=m_watchList.begin(); itor != m_watchList.end(); ++itor)
	{
		if (itor->m_entID == pEnt->ID())
		{
			itor = m_watchList.erase(itor);
			return;
		}
	}
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

		if (itor->m_watchTimer < gameTick)
		{
			//LogMsg("Removing %s from watch list", pEnt->GetName().c_str());
			itor = m_watchList.erase(itor);

			//now, it's just going to instantly run the "OnLoseVisible" again if we don't do something..
			pEnt->OnWatchListTimeout(pEnt->GetDrawID() == drawID);
			continue;
		}

		
		//it's there, let's do our logic on it if it needs it

		if (pEnt->GetDrawID() != drawID)
		{
			//it's currently off the screen
			pEnt->Update(step);
			m_postUpdateList.push_back(pEnt);
		} else
		{
			//it's on the screen and was already processed the normal way
			

		}

		itor++;
	}

	//at this point all entities who are going to run logic, have.  We can check our list to allow entities
	//to see if they were are no longer being updated or not

	for (unsigned int i=0; i < m_visibilityNotificationList.size(); i++)
	{
		if (m_visibilityNotificationList.at(i))
		m_visibilityNotificationList.at(i)->CheckVisibilityNotifications(m_visibilityID);
	}

	if (!GetGameLogic->GetGamePaused())
	{
		m_visibilityNotificationList.clear();
		m_visibilityID = GetApp()->GetUniqueNumber();
	}

	//post update will now be run on all ents, starting in the world cache, then calling our postupdate

}

void WatchManager::PostUpdate(float step)
{
	EntWorldCache *pWorldCache = GetWorldCache;
	assert(pWorldCache);
	
	for (unsigned int i=0; i < m_postUpdateList.size(); i++)
	{
		m_postUpdateList.at(i)->PostUpdate(step);
	}
}


void WatchManager::ApplyPhysics(float step)
{
	EntWorldCache *pWorldCache = GetWorldCache;
	assert(pWorldCache);

	for (unsigned int i=0; i < m_postUpdateList.size(); i++)
	{
		m_postUpdateList.at(i)->ApplyPhysics(step);
	}
}

void WatchManager::AddEntityToVisibilityList(MovingEntity *pEnt)
{
	m_visibilityNotificationList.push_back(pEnt);
}



void WatchManager::RemoveFromVisibilityList(MovingEntity *pEnt)
{
	for (unsigned int i=0; i < m_visibilityNotificationList.size(); i++)
	{
		//uh, is it possible this pointer could be bad?  Not sure
		if (pEnt == m_visibilityNotificationList.at(i))
		{
			m_visibilityNotificationList.at(i) = NULL;
		}
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

	if (pEnt->GetVisibilityNotifications())
	{
		RemoveFromVisibilityList(pEnt);
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
