
/* -------------------------------------------------
* Copyright 2005 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 28:12:2005   12:44
*/


#ifndef HashedResourceManager_HEADER_INCLUDED // include guard
#define HashedResourceManager_HEADER_INCLUDED  // include guard

//for hashing data associated with filenames for fast lookup
class HashedResource;
class TileEditOperation;
class TilePic;
class CollisionData;
class World;

typedef std::map<unsigned int, HashedResource*> HashedResourceMap;

class HashedResourceManager
{
public:

	HashedResourceManager();
	virtual ~HashedResourceManager();

	CL_Surface * GetResourceByHashedID(unsigned int resourceID);
	CollisionData * GetCollisionDataByHashedIDAndRect(unsigned int m_resourceID, const CL_Rect &rectSource);
	bool Init();
	bool AddGraphic(string str); //cache and hash a new image, include the full path and extension to
	//the image, it will strip those when it creates the hash for it.

	void Kill(); //clear the cache entirely

	HashedResourceMap * GetHashedResourceMap() {return &m_hashedResourceMap;}
	void PutGraphicIntoTileBuffer(int resourceID, TileEditOperation &op, int gridSizeInPixels);
	void PutSubGraphicIntoTileBuffer(TilePic *pTile, TileEditOperation &op, CL_Rect srcRect);
	void PrintStatistics();
	void SaveUsedResources(World *pWorld, string path);
	
protected:

	void ShowResourceNotFoundError(unsigned int resourceID);

private:
	HashedResourceMap m_hashedResourceMap;
	
};

#endif                  // include guard
