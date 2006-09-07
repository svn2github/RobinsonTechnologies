// ***************************************************************
//  TagManager - date created: 04/22/2006
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com) 
//  Copyright (C) 2006 Robinson Technologies - All Rights Reserved

#ifndef TagManager_HEADER_INCLUDED // include guard
#define TagManager_HEADER_INCLUDED  // include guard

class World;
class MovingEntity;

const int C_TAG_DATA_VERSION  = 1;
#define C_TAGCACHE_FILENAME "tagcache.dat"

class TagObject
{
public:

	CL_Vector2 & GetPos() {return m_pos;}
	const string &GetMapName();
	int GetID() {return m_entID;} //will be 0 unless already loaded, this is normal

	CL_Vector2 m_pos;
	World *m_pWorld;
	int m_entID; //so we can keep track of the owner of a tag, even with multiple tags of the same name
	string m_tagName; //purely for debug info, not really needed
};

typedef std::map<unsigned int, std::list<TagObject> > TagResourceMap;

class TagManager
{
public:

    TagManager();
    ~TagManager();

	void Kill();
	void Update(World *pWorld, MovingEntity *pEnt);
	void Remove(MovingEntity *pEntity);
	TagObject * GetFromString(const string &name);
	TagObject * GetFromHash(unsigned int hashID);
	void Save(World *pWorld);
	void Load(World *pWorld);
	void PrintStatistics();
	CL_Vector2 GetPosFromName(const string &name);

protected:

private:

	enum
	{
		E_TAG_RECORD,
		E_TAG_DONE
	};

	void AddCachedNameTag(unsigned int hashID, TagObject &o);

	TagResourceMap m_tagMap;
};


#endif                  // include guard