#include "AppPrecomp.h"
#include "HashedResourceManager.h"
#include "GameLogic.h"
#include "TileEditOperation.h"
#include "CollisionData.h"
#include "HashedResource.h"

HashedResourceManager::HashedResourceManager()
{
	
}

HashedResourceManager::~HashedResourceManager()
{
	Kill();
}

bool HashedResourceManager::Init()
{

	Kill();

	//scan map directory for tile maps we can possibly load
	CL_DirectoryScanner scanner;
	
	scanner.scan(GetGameLogic->GetBaseMapPath(), "*");
	while (scanner.next())
	{
		std::string file = scanner.get_name();
			if (scanner.is_directory() && scanner.get_name() != "." && scanner.get_name() != "..")

		{
			//this is purportedly a tilemap for us to load for global use
			CL_DirectoryScanner scanPic;
			scanPic.scan(GetGameLogic->GetBaseMapPath()+file);
			while (scanPic.next())
			{
				string fileExtension = CL_String::get_extension(scanPic.get_name());

				if (!scanPic.is_directory())
				{
				if (fileExtension == "dat") continue; //not an image, skip it
				if (fileExtension == "chunk") continue; //not an image, skip it
				if (fileExtension == "txt") continue; //not an image, skip it
				//note, I build this whole path because .get_filename includes a backslash in the middle instead of
				//a forward slash which screws up the addgraphic function
				if (!AddGraphic(GetGameLogic->GetBaseMapPath()+file+string("/")+scanPic.get_name()))
					{
						return false;
					}
				}
			}
			
		}
	}
	
	return true; //success
}

bool HashedResourceManager::AddGraphic(string str)
{
	//LogMsg("Adding %s to tile graphics", str.c_str());
	
	HashedResource *pResource = new HashedResource;
	pResource->m_strFilename = str;

	unsigned int resourceID = FileNameToID(str.c_str());
	
	HashedResourceMap::iterator itor = m_hashedResourceMap.find(resourceID);
	
	if (itor == m_hashedResourceMap.end()) 
	{
		m_hashedResourceMap.insert(std::make_pair(resourceID, pResource));
	} else
	{
		throw CL_Error("Hash conflict with graphic resource " + str + ".  Rename it!");
		return false;
	}
	return true;
}

void HashedResourceManager::PrintStatistics()
{
	LogMsg("\n **Hashed Resource Manager");
	LogMsg("  %d items hashed.", m_hashedResourceMap.size());
}

void HashedResourceManager::Kill()
{
	
	HashedResourceMap::iterator ent = m_hashedResourceMap.begin();
	for (ent; ent != m_hashedResourceMap.end(); ++ent)
	{
		delete (*ent).second;
	}

	m_hashedResourceMap.clear();
}

CL_Surface * HashedResourceManager::GetResourceByHashedID(unsigned int resourceID)
{
	HashedResourceMap::iterator itor = m_hashedResourceMap.find(resourceID);

	if (itor == m_hashedResourceMap.end()) 
	{
		LogMsg("Unable to find graphic resourceID %u", resourceID);
		//throw ("Error finding hashed resource graphic");
		return NULL;
	}
	
	return (*itor).second->GetImage();
}


void HashedResourceManager::PutSubGraphicIntoTileBuffer(TilePic *pTile, TileEditOperation &op, CL_Rect srcRect)
{
	op.ClearSelection();
	CL_Surface *pPic = GetResourceByHashedID(pTile->m_resourceID);

	if (!pPic) 
	{
		throw CL_Error("Unable to get hashed resource" + CL_String::from_int(pTile->m_resourceID));
	}
	CL_Rect imageRect = CL_Rect(0,0, pPic->get_width(),pPic->get_height());
	
	//make sure we're within bounds
	srcRect.apply_alignment(origin_top_left, -pTile->m_rectSrc.left, -pTile->m_rectSrc.top);
	srcRect.left = max(srcRect.left, imageRect.left);
	srcRect.right = min(srcRect.right, imageRect.right);
	srcRect.top = max(srcRect.top, imageRect.top);
	srcRect.bottom = min(srcRect.bottom, imageRect.bottom);
	TilePic *pTilePic = (TilePic*)pTile->CreateClone();
	pTilePic->m_rectSrc = srcRect;
	op.AddTileToSelection(TileEditOperation::C_OPERATION_ADD, false, pTilePic);
}

CollisionData * HashedResourceManager::GetCollisionDataByHashedIDAndRect(unsigned int resourceID, const CL_Rect &rectSource)
{
	//first get the correct resource
	
	HashedResourceMap::iterator itor = m_hashedResourceMap.find(resourceID);

	if (itor == m_hashedResourceMap.end()) 
	{
		LogMsg("Unable to find resourceID %u", resourceID);
		//throw ("Error finding hashed resource " + CL_String::from_int(resourceID));
		return NULL;
	}
  
  HashedResource *pResource = itor->second;
	//convert the rect info into a single number to use as a hash

  return pResource->GetCollisionDataByRect(rectSource);


}

//here we manually break up a vanilla image into tiles and put it in the buffer given for easy copy and
//pasting into a real map
void HashedResourceManager::PutGraphicIntoTileBuffer(int resourceID, TileEditOperation &op, int gridSizeInPixels)
{
	CL_Size sizeGrid(gridSizeInPixels, gridSizeInPixels);
	op.ClearSelection();
	CL_Surface *pPic = GetResourceByHashedID(resourceID);
	
	if (!pPic) 
	{
		throw CL_Error("Unable to get hashed resource" + CL_String::from_int(resourceID));
	}
	CL_Size sz = CL_Size(pPic->get_width(),pPic->get_height());
	TilePic *pTilePic;

	if (gridSizeInPixels == 0)
	{
		//just one giant tile is fine
		sizeGrid = sz;
	}

	for (int x = 0; x < sz.width; x += sizeGrid.width)
	{
		for (int y = 0; y < sz.height; y += sizeGrid.height)
		{
			pTilePic = new TilePic();
			pTilePic->m_rectSrc = CL_Rect(x,y,x+sizeGrid.width,y+sizeGrid.height);
			pTilePic->m_resourceID = resourceID;
			pTilePic->SetLayer(C_LAYER_DETAIL1);
			pTilePic->SetPos(CL_Vector2(pTilePic->m_rectSrc.left,pTilePic->m_rectSrc.bottom));
			op.AddTileToSelection(TileEditOperation::C_OPERATION_ADD, false, pTilePic);
			
		}
	}
}
