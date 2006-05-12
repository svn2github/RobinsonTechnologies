
/* -------------------------------------------------
* Copyright 2006 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 24:1:2006   15:08
*/


#ifndef HashedResource_HEADER_INCLUDED // include guard
#define HashedResource_HEADER_INCLUDED  // include guard

//we want to cache the pointers so we use a pointer to CollisionData instead of CollisionData to
//avoid complications with random STL realocations that could screw us up
class CollisionData;
typedef std::map<unsigned int, std::vector<CollisionData*> > CollisionDataMap;

class HashedResource
{
public:

	HashedResource();
	~HashedResource();

	bool Init(); //load the image and whatnot
	void Kill();
	bool HasCollisionData(); //slow check
	void SaveDefaults();
	bool LoadDefaults();
	string GetCollisionDataFileName();
	CollisionData * GetCollisionDataByRect(const CL_Rect &rectSource);
	CL_Surface * GetImage();

	string m_strFilename; //good to remember the original filename

private:

	CollisionDataMap m_collisionMap;
	CL_Surface *m_pImage;

	enum
	{
		//chunk descriptions
		C_HASHED_RESOURCE_COLLISION_CHUNK = 0,
		C_HASHED_RESOURCE_END_OF_CHUNKS
	};
};


#endif                  // include guard
