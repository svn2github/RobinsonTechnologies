#include "AppPrecomp.h"
#include "CollisionData.h"
#include "HashedResource.h"
#include "AppUtils.h"
#include "GameLogic.h"

HashedResource::HashedResource()
{
	m_pImage = NULL;
}

CollisionData * HashedResource::GetCollisionDataByRect(const CL_Rect &rectSource)
{
	if (!m_pImage)
	{
		//need to load stuff first
		Init();
	}
	
	unsigned int hash = CL_MAKELONG(rectSource.left, rectSource.top);
	CollisionDataMap::iterator colItor = m_collisionMap.find(hash);
	if (colItor == m_collisionMap.end()) 
	{

		//LogMsg("Adding new collision resource");
		std::vector<CollisionData*> colVec;
		CollisionData *pColData = new CollisionData(rectSource);
		colVec.push_back(pColData);
		m_collisionMap.insert(std::make_pair(hash, colVec));
		return pColData;
	}

	//found it, but now we need to locate the exact one if there are more than one

	std::vector<CollisionData*> *pColVec = &colItor->second;
	for (unsigned int i =0; i < pColVec->size(); i++)
	{
		if (rectSource == pColVec->at(i)->GetRect())
		{
			//this is it
			return pColVec->at(i);
		}
	}

	//if we got here, it means we don't have data on this tile/subtile piece.  Create an empty one.
	CollisionData *pColData = new CollisionData(rectSource);
	pColVec->push_back(pColData);
	return pColData;
}

bool HashedResource::HasCollisionData()
{
	CollisionDataMap::iterator ent = m_collisionMap.begin();
	std::vector<CollisionData*> *pColVec;
	unsigned int i;
	for (ent; ent != m_collisionMap.end(); ++ent)
	{
		pColVec = &ent->second;	
		for (i=0; i < pColVec->size(); i++)
		{
			if (pColVec->at(i)->HasData())	
			{
				//has data to save
				return true;
			}
		}
	}

	return false;		
}

string HashedResource::GetCollisionDataFileName()
{
	return m_strFilename.substr(0,m_strFilename.rfind('.')) + ".dat";
}

bool HashedResource::LoadDefaults()
{
	CL_InputSource_File *pFile = NULL;

	try
	{
		pFile = new CL_InputSource_File(GetCollisionDataFileName());
	} catch(CL_Error error)
	{
		SAFE_DELETE(pFile);
		return false;
	}

	CL_FileHelper helper(pFile); //will autodetect if we're loading or saving

	unsigned int chunkType;

	try
	{
		helper.process(chunkType);
		while (chunkType != C_HASHED_RESOURCE_END_OF_CHUNKS)
		{
			switch (chunkType)
			{
			case C_HASHED_RESOURCE_COLLISION_CHUNK:
				//load it from the file
				{
					CollisionData col;
					col.Serialize(helper);
					*GetCollisionDataByRect(col.GetRect()) = col;
				}
				break;
				
			default:
				assert(!"Unknown chunk type");
				break;
			}

			helper.process(chunkType);
		}

	}
	catch(CL_Error error)
	{
		LogMsg(error.message.c_str());
		ShowMessage(error.message, "Error loading collision data for graphic.  Corrupted?");
		SAFE_DELETE(pFile);
		return true;
	}

	SAFE_DELETE(pFile);
	return true; //success
}

void HashedResource::SaveDefaults()
{
	if (!m_pImage) return;
	string fName = GetCollisionDataFileName();

	if (!HasCollisionData())
	{
		//instead of saving, we should destroy ourselves..
		RemoveFile(fName);
		return;
	}

	//we know there is data, so let's save it out

	CL_OutputSource_File file(fName);
	CL_FileHelper helper(&file); //will autodetect if we're loading or saving

	CollisionDataMap::iterator ent = m_collisionMap.begin();

	std::vector<CollisionData*> *pColVec;
	unsigned int i;
	for (ent; ent != m_collisionMap.end(); ++ent)
	{
		pColVec = &ent->second;	
		for (i=0; i < pColVec->size(); i++)
		{
			if (pColVec->at(i)->HasData())		
			{
				//has data to save
				//save header
				assert(sizeof(C_HASHED_RESOURCE_COLLISION_CHUNK) == 4 && "This should be an unsigned int.. uh.. ");

				helper.process_const(C_HASHED_RESOURCE_COLLISION_CHUNK);
				pColVec->at(i)->Serialize(helper);
			}
		}
	}
	//save end of data header
	helper.process_const(C_HASHED_RESOURCE_END_OF_CHUNKS);


}

HashedResource::~HashedResource()
{
	SaveDefaults();
	Kill();
}

void HashedResource::Kill()
{
	CollisionDataMap::iterator ent = m_collisionMap.begin();
	unsigned int i;
	for (ent; ent != m_collisionMap.end(); ++ent)
	{
		//run through the vector and delete its stuff, note that this could cause
		//hanging pointers if anyone is still caching it

		for (i=0; i < (*ent).second.size(); i++)
		{
			delete (*ent).second.at(i);
		}
	}
	m_collisionMap.clear();
	SAFE_DELETE(m_pImage);
}

CL_Surface * HashedResource::GetImage()
{
	if (!m_pImage) 
	{
		BlitMessage("(loading art)");
		Init();
	}
	return m_pImage;
}
bool HashedResource::Init()
{
	Kill();
	assert(m_strFilename.size() > 0 && "Set the filename first");

	SAFE_DELETE(m_pImage);
	m_pImage = new CL_Surface(m_strFilename);
	clTexParameteri(CL_TEXTURE_2D, CL_TEXTURE_MAG_FILTER, CL_NEAREST);
	clTexParameteri(CL_TEXTURE_2D, CL_TEXTURE_MIN_FILTER, CL_NEAREST);

	if (!m_pImage) return false;
	LoadDefaults(); //if applicable
	return true;
}
