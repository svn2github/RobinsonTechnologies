#include "AppPrecomp.h"
#include "World.h"
#include "AppUtils.h"
#include "GameLogic.h"
#include "AI/NavGraphManager.h"
#include "AI/WorldNavManager.h"

#define C_DEFAULT_THUMBNAIL_WIDTH 8
#define C_DEFAULT_THUMBNAIL_HEIGHT 8


const CL_Vector2 g_worldDefaultCenterPos(C_DEFAULT_SCREEN_ID*512, C_DEFAULT_SCREEN_ID*512);

World::World()
{
	m_pNavGraphManager = NULL;
	m_bDataChanged = true;
	m_pWorldCache = NULL;
	m_defaultTileSize = 0;
	m_masterNavMapID = invalid_node_index;
	for (int i=0; i < e_byteCount; i++) m_byteArray[i] = 0;
	for (int i=0; i < e_intCount; i++) m_intArray[i] = 0;
	for (int i=0; i < e_uintCount; i++) m_uintArray[i] = 0;
	for (int i=0; i < e_floatCount; i++) m_floatArray[i] = 0;
	m_uintArray[e_uintBGColor] = CL_Color::aliceblue.color;

#ifdef _DEBUG
 
	m_worldChunkPixelSize = 512;
	//small self test
	assert(TestCoordPacker(0, 0));
	assert(TestCoordPacker(1, 0));
	assert(TestCoordPacker(100, 100));
	
	assert(TestCoordPacker(-5, 0));

	assert(TestCoordPacker(0, -5));
	assert(TestCoordPacker(-23, -3));
	assert(TestCoordPacker(-50, -34));


	CL_Vector2 v(0, 2500);
	CL_Vector2 v2;
    int id;
	
	
		id = GetScreenIDFromWorld(v);
		v2 = ScreenIDToWorldPos(id);

		//LogMsg("Id %d made from %.2f, %.2f, but this screen starts at %f, %f.",
		//	id, v.x, v.y, v2.x, v2.y);


		v = CL_Vector2(-512, 2500);
		id = GetScreenIDFromWorld(v);
		v2 = ScreenIDToWorldPos(id);

		//LogMsg("Id %d made from %.2f, %.2f, but this screen starts at %f, %f.",
		//	id, v.x, v.y, v2.x, v2.y);

		v = CL_Vector2(-1024, 2500);
		id = GetScreenIDFromWorld(v);
		v2 = ScreenIDToWorldPos(id);

		//LogMsg("Id %d made from %.2f, %.2f, but this screen starts at %f, %f.",
		//	id, v.x, v.y, v2.x, v2.y);
	
		m_worldChunkPixelSize = 0;

#endif

}

int World::GetMasterNavMapID()
{
	
	assert(!"We don't use this anymore");
	//we'll have each node link to one central "map" node.
	
	/*
	if (m_masterNavMapID == invalid_node_index)
	{
		g_worldNavManager.GetGraph()->AddNode(GraphNode(
			m_masterNavMapID = g_worldNavManager.GetGraph().GetNextFreeNodeIndex()));
	}
*/
	return m_masterNavMapID;
}

bool World::TestCoordPacker(int x, int y)
{
	CL_Point pt(x,y);
	ScreenID id = GetScreenID(pt.x, pt.y);
	CL_Point rt (GetXFromScreenID(id), GetYFromScreenID(id));

	if (rt != pt)
	{
		LogMsg("Error, %d,%d converted to screenID %d then back to %d,%d.",
			pt.x, pt.y, id, rt.x, rt.y);
		return false;
	}
	return true;
}

void World::SaveAndKill()
{
	if (SaveRequested())
	{
		Save(true);
	}
	m_pWorldCache = NULL;
	Kill();
}

World::~World()
{
	//assert(!m_worldMap.empty() && "You need to call SaveAndKill manually!");
	Kill();
}

void World::Kill()
{
  
	m_pWorldCache = NULL; 
	WorldMap::iterator ent = m_worldMap.begin();
    for (; ent != m_worldMap.end(); ++ent)
    {
        {
              delete (*ent).second;
         }
    }
    m_worldMap.clear();
	SAFE_DELETE(m_pNavGraphManager);
 }

EntWorldCache * World::GetMyWorldCache()
{
	
	if (!m_pWorldCache)
	{
		LogMsg("Choosing to load world because someone wanted access to its world cache");
		GetWorldManager->LoadWorld(GetDirPath(), false);
		PreloadMap();
	}
	

	return m_pWorldCache;
}

NavGraphManager * World::GetNavGraph()
{
	if (m_pNavGraphManager) return m_pNavGraphManager;

	m_pNavGraphManager = new NavGraphManager(this);
	return m_pNavGraphManager;
}

void World::SetDirPath(const string &str)
{
	m_strDirPath = str; //include trailing backslash
	//also compute the map name from the dir path
	m_strMapName = ExtractFinalDirName(m_strDirPath);
}

void World::SetDefaultTileSize(int size)
{
  m_defaultTileSize = size;
  m_bDataChanged = true;
}

const CL_Rect *World::GetWorldRect()
{
    return &m_worldRect;
}

CL_Rect World::GetWorldRectInPixels()
{
	CL_Rect rec = *GetWorldRect();
	rec.left *= GetWorldChunkPixelSize();
	rec.right *= GetWorldChunkPixelSize();
	rec.top *= GetWorldChunkPixelSize();
	rec.bottom *= GetWorldChunkPixelSize();
	return rec;
}

bool World::IsValidCoordinate(CL_Vector2 vec)
{
	if (
		(fabs(vec.x) > 200000000)
		||

		(fabs(vec.y) > 200000000)
		)
	{
		LogMsg("Bad coord?");
	}

	
	return true;
}

ScreenID World::GetScreenID(short x, short y)
{
 return CL_MAKELONG( *(unsigned short*) &x, *(unsigned short*) &y);
// return MAKELPARAM( *(unsigned short*) &x, *(unsigned short*) &y);
}


int World::GetXFromScreenID(ScreenID screenID)
{
    static unsigned short val;
	val =CL_LOWORD(screenID);
	return *(short*)&val;
}

int World::GetYFromScreenID(ScreenID screenID)
{
	static unsigned short val;
	val =CL_HIWORD(screenID);
	return *(short*)&val;
}


void World::DeleteExistingMap()
{
	
	//run through and tell everybody not to save, cheap but it works
	WorldMap::const_iterator itor = m_worldMap.begin();
	while (itor != m_worldMap.end())
	{
		itor->second->SetDataChanged(false);
		itor->second->SetNeedsThumbnailRefresh(false);
		itor++;
	}
	
	RemoveWorldFiles(GetDirPath());

	Init();
}

void World::Init(CL_Rect worldRect)
{
    Kill();
	m_version = C_WORLD_FILE_VERSION;

	if (m_defaultTileSize == 0)
	{
		//set some defaults
		m_worldChunkPixelSize = 512;
		m_defaultTileSize = 64;
		SetThumbnailWidth(C_DEFAULT_THUMBNAIL_WIDTH);
		SetThumbnailHeight(C_DEFAULT_THUMBNAIL_HEIGHT);
		SetBGColor(CL_Color(0,0,100));
		SetGravity(0.08f);
	}

	m_worldRect = worldRect;
}


Screen * World::GetScreen(int x, int y) //screen #
{
    return GetScreen(GetScreenID(x,y));
}

Screen * World::GetScreen(const CL_Vector2 &vecWorld) //world coords
{
	return GetScreen(GetScreenIDFromWorld(vecWorld));
}


//get a stored worldchunk or dynamically create it if needed.  The screen member is left blank, as its possible
//to say "this is part of the map, but it isn't loaded yet" for the streaming operations
//If you absolutely need the screen to be initted, use GetScreen instead
WorldChunk * World::GetWorldChunk(ScreenID screenID)
{
	WorldMap::iterator screen = m_worldMap.find(screenID);

	if (screen == m_worldMap.end())
	{
		//it didn't exist ,create it
		//LogMsg("Creating screen %d.", screenID);
		WorldChunk *pChunk = new WorldChunk(this);
		pChunk->SetScreenID(screenID);
		m_worldMap.insert(std::make_pair(screenID, pChunk));

		//modify our bounds to include it if required
		int x = GetXFromScreenID(screenID);
		int y = GetYFromScreenID(screenID);

			if (m_worldMap.size() == 1)
			{
				//our only screen, set the world map to this
				m_worldRect = CL_Rect(x,y,x+1,y+1);
			}

		if (x < m_worldRect.left) m_worldRect.left = x;
		if (y < m_worldRect.top) m_worldRect.top = y;

		if (x > m_worldRect.right) m_worldRect.right = x;
		if (y > m_worldRect.bottom) m_worldRect.bottom = y;

		return pChunk;
	} 

	return screen->second;
}

WorldChunk * World::GetWorldChunk(CL_Vector2 vecWorld)
{
	return GetWorldChunk(GetScreenIDFromWorld(vecWorld));
}

Screen * World::GetScreen(ScreenID screenID)
{
	return (GetWorldChunk(screenID)->GetScreen());
}

//returns NULL for false
WorldChunk * World::DoesWorldChunkExist(ScreenID screenID)
{
	WorldMap::const_iterator itor = m_worldMap.find(screenID);
	if (itor == m_worldMap.end()) return NULL;
	return itor->second;
}

CL_Vector2 World::ScreenIDToWorldPos(ScreenID screenID)
{
	
	CL_Vector2 vecPos = CL_Vector2(GetXFromScreenID(screenID), GetYFromScreenID(screenID));
	vecPos *= GetWorldChunkPixelSize();
    return vecPos;
}

ScreenID World::GetScreenIDFromWorld(CL_Vector2 vecPos)
{
	if (vecPos.x < 0)
	{
		vecPos.x -= GetWorldChunkPixelSize()-1;
	}

	if (vecPos.y < 0)
	{
		vecPos.y -= GetWorldChunkPixelSize()-1;
	}

	vecPos /= GetWorldChunkPixelSize();
	
	return GetScreenID(vecPos.x, vecPos.y);
}

void World::SetWorldChunkPixelSize(int widthAndHeight)
{
	m_worldChunkPixelSize = widthAndHeight;
}

CL_Vector2 World::SnapWorldCoords(CL_Vector2 vecWorld, int snapSize)
{
  vecWorld.x -= altfmod(vecWorld.x, snapSize);
  vecWorld.y -= altfmod(vecWorld.y, snapSize);
  return vecWorld;
}

void World::InvalidateAllThumbnails()
{
	WorldMap::const_iterator itor = m_worldMap.begin();

	while (itor != m_worldMap.end())
	{
		itor->second->SetThumbNail(NULL);
		itor->second->SetNeedsThumbnailRefresh(true);
		itor++;
	}	
}

//sent a pointer, it handles deleting it!!
void World::AddTile(Tile *pTile)
{
	Screen *pScreen = GetScreen(pTile->GetPos());
	pScreen->AddTile(pTile);
}


bool World::Load(string dirPath) 
{
	Kill(); //this will save anything we have pending to save
	
	if (!dirPath.empty()) SetDirPath(dirPath);
	
	assert(m_strDirPath[m_strDirPath.length()-1] == '/' && "Load needs a path with an ending backslash on it");

	//init defaults, if loading an older file format they may be important
	
	m_defaultTileSize = 0;
	Init();

	m_layerManager.Load(m_strDirPath+C_LAYER_FILENAME);
	
	CL_InputSource *pFile = g_VFManager.GetFile(m_strDirPath+C_WORLD_DAT_FILENAME);
	if (!pFile)
	{
		SAFE_DELETE(pFile);
		return false;
	}

	try
	{
		LogMsg("Loading world map...");
	
		CL_FileHelper helper(pFile); //will autodetect if we're loading or saving

		helper.process(m_version);
	
		CL_Rect rectTemp;
		helper.process(rectTemp);
		//we could copy that rect to m_rectWorld but it might be better to let it 'auto figure' out
		//where the bounds are in case we'd deleted a lot

		LogMsg("Loaded rect is %d, %d, %d, %d", m_worldRect.top, m_worldRect.left, m_worldRect.right, m_worldRect.bottom);
		//now cycle through and load all the WorldChunks so we know where stuff is and what the thumbnail
		//looks like, but without actually loading the screens

		m_cameraSetting.Serialize(helper);
		helper.process(m_worldChunkPixelSize);
		helper.process(m_defaultTileSize);

		helper.process_smart_array(m_byteArray, e_byteCount);
		helper.process_smart_array(m_intArray, e_intCount);
		helper.process_smart_array(m_uintArray, e_uintCount);
		helper.process_smart_array(m_floatArray, e_floatCount);

		unsigned int chunkType;
		WorldChunk *pWorldChunk = NULL;
		ScreenID screenID;

		helper.process(chunkType);
		LogMsg("Loading worldchunks...");
		while (chunkType != C_DATA_WORLD_END_OF_CHUNKS)
		{
			switch (chunkType)
			{
			case C_DATA_WORLD_CHUNK:
				//LogMsg("Loading chunk..");
				//It's a world chunk definition, let's load it		
				helper.process(screenID); //get its screen id
				//LogMsg("%d", screenID);
				GetWorldChunk(screenID)->Serialize(pFile); //GetWorldChunk will init it for us, serialize will
				//load it from the file
				break;
			default:
				assert(!"Unknown chunk type");
				break;
			}

			helper.process(chunkType);
		}


	} catch(CL_Error error)
	{
		LogMsg(error.message.c_str());
		  ShowMessage(error.message.c_str(), "Error loading map.  Corrupted?");
		SAFE_DELETE(pFile);
		return false;
	}

	LogMsg("Loaded map.  %d non-empty chunks, size is %d by %d.", m_worldMap.size(), GetWorldX(), GetWorldY());
	SAFE_DELETE(pFile);

	m_bDataChanged = false;
	return true; //actually loaded something
}

bool World::SaveRequested()
{
	if (!GetAutoSave()) return false; //we're told not save 
	if (GetGameLogic->UserProfileActive())
	{
		//do we really want to save this?  We aren't in the official editor...
		if (!GetPersistent()) return false;
	}

	return true;
}


void World::ForceSaveNow()
{
	m_bDataChanged = true;
	Save(true);

	WorldMap::const_iterator itor = m_worldMap.begin();
	while (itor != m_worldMap.end())
	{
		if (!itor->second->IsEmpty() && itor->second->IsScreenLoaded())
		{
			itor->second->GetScreen()->Save(); //just accessing it causes it to precache
		}
		itor++;
	}	
}

bool World::Save(bool bSaveTagCacheAlso)
{
	if (m_defaultTileSize == 0) return false; //don't actually save.. nothing was loaded

	assert(m_strDirPath[m_strDirPath.length()-1] == '/' && "Save needs a path with an ending backslash on it");

	g_VFManager.CreateDir(m_strDirPath); //creates the dir if needed in the last mounted file tree
	
	if (bSaveTagCacheAlso)
	{
		//first save our cached tag data
		GetTagManager->Save(this);

		if (!GetApp()->GetMyGameLogic()->UserProfileActive())
		{
			//also a good time to save out used textures, used for missing texture errors
			GetHashedResourceManager->SaveUsedResources(this, m_strDirPath);
		}
	}

	m_layerManager.Save(m_strDirPath+C_LAYER_FILENAME);
	
	//do we absolutely have to save?
	bool bRequireSave = m_bDataChanged;

	WorldMap::const_iterator itor;

	if (bRequireSave == false)
	{
		//if a worldchunk changed (like the thumbnail, or a worldchunk was added), we need to save everything anyway.  this is kind of bad design..

		itor = m_worldMap.begin();

		while (itor != m_worldMap.end())
		{
			if (!itor->second->IsEmpty())
			{
				if (itor->second->GetChunkDataChanged())
				{
					bRequireSave = true;
					break;
				}
			}
			itor++;
		}
	}

	if (!bRequireSave) return true; //abort the save, not needed


	LogMsg("Saving world map header %s - (%d world chunks to look at, map size %d by %d)", GetName().c_str(), m_worldMap.size(),
		GetWorldX(), GetWorldY());

	//first save our world.dat file
	CL_OutputSource *pFile = g_VFManager.PutFile(m_strDirPath+C_WORLD_DAT_FILENAME);
	
	CL_FileHelper helper(pFile); //will autodetect if we're loading or saving
	
	m_version = C_WORLD_FILE_VERSION;
	helper.process(m_version);
	helper.process(m_worldRect);
	m_cameraSetting.Serialize(helper);
	helper.process(m_worldChunkPixelSize);
	helper.process(m_defaultTileSize);

	helper.process_smart_array(m_byteArray, e_byteCount);
	helper.process_smart_array(m_intArray, e_intCount);
	helper.process_smart_array(m_uintArray, e_uintCount);
	helper.process_smart_array(m_floatArray, e_floatCount);

	//run through every screen that is currently open and ask it to save itself if it hasn't been
	
	itor = m_worldMap.begin();
	while (itor != m_worldMap.end())
	{
		if (!itor->second->IsEmpty())
		{
			assert(sizeof(C_DATA_WORLD_CHUNK) == 4 && "This should be an unsigned int.. uh.. ");
			helper.process_const(C_DATA_WORLD_CHUNK);
			helper.process_const(itor->second->GetScreenID());

			itor->second->Serialize(pFile);
		}
		itor++;
	}

	helper.process_const(C_DATA_WORLD_END_OF_CHUNKS); //signal the end
	SAFE_DELETE(pFile);
	return true;
}

void World::PreloadMap()
{
	WorldMap::const_iterator itor = m_worldMap.begin();
	while (itor != m_worldMap.end())
	{
		if (!itor->second->IsEmpty())
		{
			itor->second->GetScreen(); //just accessing it causes it to precache
		}
		itor++;
	}	
}

void World::GetAllWorldChunksWithinThisRect(std::vector<WorldChunk*> &wcVector, CL_Rect rec, bool bIncludeBlanks)
{
	assert(wcVector.size() == 0 && "Shouldn't this be cleared before you send it?");

	int startingX = rec.left;

	CL_Rect scanRec;
	WorldChunk *pWorldChunk;
	CL_Rect screenRec;

	bool bScanMoreOnTheRight, bScanMoreOnTheBottom;

	while (1)
	{
		scanRec = rec;

		//get the screen the upper left is on
		pWorldChunk = GetWorldChunk(CL_Vector2(scanRec.left, scanRec.top));
		if (!bIncludeBlanks && pWorldChunk->IsEmpty())
		{
			//don't add this one
		} else
		{
			wcVector.push_back(pWorldChunk);
		}

		screenRec = pWorldChunk->GetRect();
		//truncate to the screen size if we need to

		if (scanRec.right > screenRec.right)
		{
			//we have more on our right to scan after this, truncate it for now
			scanRec.right = screenRec.right;
			bScanMoreOnTheRight = true;
		} else
		{
			bScanMoreOnTheRight = false;
		}

		if (scanRec.bottom > screenRec.bottom)
		{
			//we have more on our bottom to scan after this, truncate it for now
			scanRec.bottom = screenRec.bottom;
			bScanMoreOnTheBottom = true;
		} else
		{
			bScanMoreOnTheBottom = false;
		}

		//do we have more to scan on our right after this?

		if (bScanMoreOnTheRight)
		{
			//TEMP HACK
			//if (rec.left == scanRec.right) return;
			//***********

			rec.left = scanRec.right;
		} else if (bScanMoreOnTheBottom)
		{
			rec.top = scanRec.bottom;
			rec.left = startingX;
		} else
		{
			//all done
			break;
		}

	}
}

void World::ReInitEntities()
{
	tile_list tileList;
	tile_list::iterator tileItor;
	MovingEntity *pEnt;
	int tileType;

	if (!IsInitted()) return;

	WorldMap::const_iterator itor = m_worldMap.begin();
	while (itor != m_worldMap.end())
	{
		if (!itor->second->IsEmpty() && itor->second->IsScreenLoaded())
		{
			tileList.clear();

			itor->second->GetScreen()->GetPtrToAllTiles(&tileList);

			//now we need to reinit them or something
			for (tileItor = tileList.begin(); tileItor != tileList.end(); tileItor++)
			{
				tileType = (*tileItor)->GetType();
				switch (tileType)
				{

				case C_TILE_TYPE_ENTITY:
					pEnt = ((TileEntity*)*tileItor)->GetEntity();
					if (pEnt) 
					{
						pEnt->Kill();
						pEnt->Init();
					}

					break;
				}

			}
		}
		itor++;
	}	
}

void World::RemoveUnusedFileChunks()
{
	CL_DirectoryScanner scanner;
	scanner.scan(m_strDirPath, "*.chunk");
	while (scanner.next())
	{
		std::string file = scanner.get_name();
		if (!scanner.is_directory())
		{
			int worldchunk = CL_String::to_int(scanner.get_name());
			if (!DoesWorldChunkExist(worldchunk))
			{
				LogMsg("Deleting unused worldchunk file %s. (screenID %d)", scanner.get_name().c_str(),
					worldchunk);
				RemoveFile(m_strDirPath+scanner.get_name());
			}
		}
	}
}

void World::ReInitCollisionOnTilePics()
{
	tile_list tileList;
	tile_list::iterator tileItor;
	TilePic *pTilePic;
	int tileType;

	if (!IsInitted()) return;

	WorldMap::const_iterator itor = m_worldMap.begin();
	while (itor != m_worldMap.end())
	{
		if (!itor->second->IsEmpty() && itor->second->IsScreenLoaded())
		{
			tileList.clear();

			itor->second->GetScreen()->GetPtrToAllTiles(&tileList);

			//now we need to reinit them or something
			for (tileItor = tileList.begin(); tileItor != tileList.end(); tileItor++)
			{
				tileType = (*tileItor)->GetType();
				switch (tileType)
				{

				case C_TILE_TYPE_PIC:
					pTilePic = ((TilePic*)*tileItor);
					pTilePic->ReinitCollision();

					break;
				}

			}
		}
		itor++;
	}	
}

void World::AddWarpTagHashID(unsigned int hashID)
{
	m_warpTagHashIDList.push_back(hashID);
}

void World::RemoveWarpTagHashID(unsigned int hashID)
{
	list<unsigned int>::iterator itor = find(m_warpTagHashIDList.begin(), m_warpTagHashIDList.end(), hashID);

	if (itor != m_warpTagHashIDList.end())
	{
		m_warpTagHashIDList.erase(itor);
	}
}

void RemoveWorldFiles(const string &path)
{
	CL_DirectoryScanner scanner;
	scanner.scan(path, "*.chunk");
	while (scanner.next())
	{
		std::string file = scanner.get_name();
		if (!scanner.is_directory())
		{
			RemoveFile(path+scanner.get_name());
		}
	}

	RemoveFile(path+C_TAGCACHE_FILENAME);
	RemoveFile(path+C_WORLD_DAT_FILENAME);
	RemoveFile(path+C_LAYER_FILENAME);
}

void World::SetMyWorldCache(EntWorldCache *pWorldCache)
{
	 m_pWorldCache = pWorldCache;
}