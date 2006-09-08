#include "AppPrecomp.h"
#include "TagManager.h"
#include "WorldManager.h"
#include "MovingEntity.h"
#include "AI/WorldNavManager.h"

const string &TagObject::GetMapName()
{
	return m_pWorld->GetName();
}

TagManager::TagManager()
{
}

TagManager::~TagManager()
{
}

void TagManager::Kill()
{
	m_tagMap.clear();
}

void TagManager::RegisterAsWarp(MovingEntity *pEnt, const string &targetName)
{

	
	
	TagObject * pTag = GetFromHash(pEnt->GetNameHash());
	if (!pTag)
	{
		LogError("Unable to locate entity %s to register as warp object to %s.  Note:  It must be a NAMED entity.",
			pEnt->GetName().c_str(), targetName.c_str());
		return;
	}


	if (targetName.empty())
	{
		LogMsg("Warp %s has a blank target. Ignoring.  Just so you know.", targetName.c_str());
		return;
	}

	if (pTag->m_graphNodeID != invalid_node_index) return;

	pTag->m_warpTarget = targetName;
	g_worldNavManager.AddNode(pTag);

	//in this case, we'd also want to connect it now
	g_worldNavManager.LinkNode(pTag);
}

void TagManager::Update(World *pWorld, MovingEntity *pEnt)
{
	if (pEnt->GetName().empty()) return; //don't hash this

	unsigned int hashID = pEnt->GetNameHash();
	TagObject *pObject = NULL;

	assert(hashID != 0);

	TagResourceMap::iterator itor = m_tagMap.find(hashID);

	if (itor == m_tagMap.end()) 
	{
		//couldn't find it, add it
		list<TagObject> *pList = &m_tagMap[hashID];
		TagObject o;
		pList->push_back(o);
		pObject = &(*pList->begin());
		//LogMsg("Creating hash for id %d", pEnt->ID());
		pObject->m_tagName = pEnt->GetName();

	} else
	{
		//an entry exists.  but is it the right one?
		
		
		
		list<TagObject> *pTagList = &m_tagMap[hashID];
		list<TagObject>::iterator itorO = pTagList->begin();

		//go through each item and see if it's supposed to be us or not
		while (itorO != pTagList->end())
		{
			
			if (itorO->m_entID == pEnt->ID() 
				||
				(
				    (itorO->m_pos == pEnt->GetPos())
				&&  (itorO->m_pWorld == pWorld)
				)
				)
			{
				//note, if itorO->m_entID == 0, it means we're overwriting temp cache data that we loaded
				//as a way to know about entities that don't really exist yet

				//this is it
				pObject = &(*itorO);

				//not only that, but let's move it up on  the list
				//LogMsg("updating existing (id %d)", pEnt->ID());
				break;
			}
			
			//we weren't the first item.   Bad.

			/*
			if (m_pEnt->GetNavNodeType() != C_NODE_TYPE_NORMAL)
			{
				//don't allow dupes of this type

			}
			*/

			//let's just not allow dupes at all
			
			LogMsg("Conflict, tagname %s already in use.  Making unique.", pEnt->GetName().c_str());
			pEnt->GetTile()->GetParentScreen()->GetParentWorldChunk()->SetDataChanged(true);
			
			pEnt->SetNameEx(pEnt->GetName() + 'A', false);
			
			return;

			itorO++;
		}

	   if (!pObject)
	   {
			//this must be a new entry, add it
		   TagObject o;
		   pTagList->push_front(o);
		   pObject = &(*pTagList->begin());
		   pObject->m_tagName = pEnt->GetName();
	  // LogMsg("Added new (%d)",pEnt->ID());
	   }
		
	}


	pObject->m_pos = pEnt->GetPos();
	pObject->m_pWorld = pWorld;
	pObject->m_entID = pEnt->ID();
	pObject->m_hashID = hashID;
	
}

TagObject * TagManager::GetFromHash(unsigned int hashID)
{
	TagResourceMap::iterator itor = m_tagMap.find(hashID);
	
	if (itor == m_tagMap.end()) 
	{
		return NULL;
	}

	//there may be more than one, for now return the "top one"

	return & (*itor->second.rbegin());
}

TagObject * TagManager::GetFromString(const string &name)
{
	return GetFromHash(HashString(name.c_str()) );
}

CL_Vector2 TagManager::GetPosFromName(const string &name)
{
    TagObject *pTag = GetFromString(name);
	if (!pTag)
	{
		LogError("TagManager::GetPosFromName: Can't locate TAG %s, sending back 0.0", name.c_str());
		return CL_Vector2::ZERO;
	}
	return pTag->m_pos;
}

void TagManager::Remove(MovingEntity *pEnt)
{
	if (!pEnt || pEnt->GetName().empty()) return;
	
	int hashID = pEnt->GetNameHash();
	
	//remove it, but make sure it's the right one

	TagResourceMap::iterator itor = m_tagMap.find(hashID);

	if (itor == m_tagMap.end()) 
	{
		//it's not in here
		LogError("Failed to remove hash %d (ID %d)", hashID, pEnt->ID());
		return;
	} else
	{
		list<TagObject> *pTagList = &m_tagMap[hashID];
		list<TagObject>::iterator itorO = pTagList->begin();

		if (itorO->m_graphNodeID != invalid_node_index)
			g_worldNavManager.RemoveNode(&(*itorO));

		m_tagMap.erase(itor);


		return;
		
		/*
		//an entry exists.  but is it the right one?
		list<TagObject> *pTagList = &m_tagMap[hashID];
		list<TagObject>::iterator itorO = pTagList->begin();

			//we used to allow dupes, not anymore.  The stuff below isn't used

		//go through each item and see if it's supposed to be us or not
		while (itorO != pTagList->end())
		{
			if (itorO->m_entID == pEnt->ID())
			{
			
				if (itorO->m_graphNodeID != invalid_node_index)
				g_worldNavManager.RemoveNode(&(*itorO));

				//this is it, remove it
				if (pTagList->size() == 1)
				{
					//remove the whole entry
					m_tagMap.erase(itor);
					//LogMsg("Removed %d", pEnt->ID());
					return;
				} else
				{
					//only remove this one from the list
					pTagList->erase(itorO);
					//LogMsg("Removed %d", pEnt->ID());
					return;
				}
			}
			itorO++;
		}
		*/
	}

		LogError("Failed to remove hash %d (ID %d) in list search", hashID, pEnt->ID());

}

void TagManager::PrintStatistics()
{
	LogMsg("\n ** TagManager Statistics **");

	//count instances

	string name;
	TagResourceMap::iterator itor = m_tagMap.begin();
	while (itor != m_tagMap.end()) 
	{
		//an entry exists.  but is it the right one?
		list<TagObject> *pTagList = &itor->second;
		list<TagObject>::iterator itorO = pTagList->begin();
		while (itorO != pTagList->end())
		{
			MovingEntity *pEnt = NULL;
			string extra;

			if (itorO->m_entID != 0)
			{
				pEnt = (MovingEntity*)EntityMgr->GetEntityFromID(itorO->m_entID);
				if (pEnt)
				{
					extra = "GraphID: "+ CL_String::from_int(itorO->m_graphNodeID) + " WarpTarget: " + itorO->m_warpTarget;
				
					
					name = pEnt->GetName();
				} else
				{
					name = "Unable to locate! ";
				}
			} else
			{
				name = itorO->m_tagName;
			}


if (pEnt)
{
	assert(pEnt->GetName() == itorO->m_tagName && "Name mismatch! Out of date?");
}
			LogMsg("    Entity %s (%d) located at %s (in %s) Hash:%u %s", itorO->m_tagName.c_str(), itorO->m_entID, PrintVector(itorO->m_pos).c_str(), itorO->m_pWorld->GetName().c_str(), 
				itorO->m_hashID, extra.c_str());

			itorO++;
		}

  	itor++;	
	}

	LogMsg("  %d names hashed.", m_tagMap.size());
}

void TagManager::Save(World *pWorld)
{
  //cycle through and save all tag data applicable
	
	CL_OutputSource *pFile = g_VFManager.PutFile(pWorld->GetDirPath()+C_TAGCACHE_FILENAME);
	
	
	CL_FileHelper helper(pFile); //will autodetect if we're loading or saving

    helper.process_const(C_TAG_DATA_VERSION);

	int tag = E_TAG_RECORD;

	TagResourceMap::iterator itor = m_tagMap.begin();
	while (itor != m_tagMap.end()) 
	{
		//an entry exists.
		list<TagObject> *pTagList = &itor->second;
		list<TagObject>::iterator itorO = pTagList->begin();
		while (itorO != pTagList->end())
		{
			   //save out our entries if this is really from our world
				if (itorO->m_pWorld == pWorld)
				{
					helper.process(tag);
					helper.process_const( itor->first);
					helper.process(itorO->m_pos);
					helper.process(itorO->m_tagName);
					helper.process(itorO->m_warpTarget);

				}
				itorO++;
		}

		itor++;	
	}

	tag = E_TAG_DONE;
	helper.process(tag);

	SAFE_DELETE(pFile);
}

void TagManager::AddCachedNameTag(unsigned int hashID, const TagObject &o)
{
	
	TagResourceMap::iterator itor = m_tagMap.find(hashID);


	list<TagObject> *pList = &m_tagMap[hashID];

	if (pList->empty()) 
	{
		//couldn't find it, add it
		pList->push_back(o);
	} else
	{
		//add it to the end of whatever is here
		pList->push_front(o);
	}

	TagObject *pTag =  &(*pList->begin());
	
	pTag->m_hashID = hashID;
	
   if (!pTag->m_warpTarget.empty())
   {
	   g_worldNavManager.AddNode(&(*pList->begin()));
   }

}

void TagManager::Load(World *pWorld)
{
	//LogMsg("Loading tag data..");
	CL_InputSource *pFile = g_VFManager.GetFile(pWorld->GetDirPath()+C_TAGCACHE_FILENAME);

	if (!pFile) return;
	
	CL_FileHelper helper(pFile); //will autodetect if we're loading or saving

	//load everything we can find

	int version;
	helper.process(version);
		
	CL_Vector2 pos;
	int tag;
	TagObject o;
	o.m_entID = 0;
	o.m_pWorld = pWorld;

	
	unsigned int hashID;

	try
	{

	while(1)
	{
		helper.process(tag);

		switch (tag)
		{
		case E_TAG_DONE:
			SAFE_DELETE(pFile);
			return;

		case E_TAG_RECORD:

			helper.process(hashID);
			
			helper.process(o.m_pos);
			helper.process(o.m_tagName);
			helper.process(o.m_warpTarget);

			AddCachedNameTag(hashID, o);
			break;

		default:
			throw CL_Error("Error reading tagdata");
		}
	}
	} catch(CL_Error error)
	{
		LogMsg(error.message.c_str());
		ShowMessage(error.message.c_str(), "Error loading tagcache data.  Corrupted?");
		SAFE_DELETE(pFile);
		return;
	}
	

}