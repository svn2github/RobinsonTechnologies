#include "AppPrecomp.h"
#include "Map.h"
#include "AppUtils.h"
#include "GameLogic.h"
#include "AI/NavGraphManager.h"
#include "AI/WorldNavManager.h"

#define C_DEFAULT_THUMBNAIL_WIDTH 8
#define C_DEFAULT_THUMBNAIL_HEIGHT 8


const CL_Vector2 g_mapDefaultCenterPos(C_DEFAULT_SCREEN_ID*512, C_DEFAULT_SCREEN_ID*512);

Map::Map()
{
	m_pNavGraphManager = NULL;
	m_bDataChanged = true;
	m_pWorldCache = NULL;
	SetDefaultTileSizeX(0);
	SetDefaultTileSizeY(64);

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

bool Map::TestCoordPacker(int x, int y)
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

void Map::SaveAndKill()
{
	if (SaveRequested())
	{
		Save(true);
	}
	m_pWorldCache = NULL;
	Kill();
}

Map::~Map()
{
	//assert(!m_worldMap.empty() && "You need to call SaveAndKill manually!");
	Kill();
}

void Map::Kill()
{
  
//	m_pWorldCache = NULL; 

	if (m_pWorldCache)
	{
		m_pWorldCache->ClearCache();
	}

	map_chunk_map::iterator ent = m_chunkMap.begin();
    for (; ent != m_chunkMap.end(); ++ent)
    {
        {
              delete (*ent).second;
         }
    }
    m_chunkMap.clear();
	SAFE_DELETE(m_pNavGraphManager);
 }

EntMapCache * Map::GetMyMapCache()
{
	
	if (!m_pWorldCache)
	{
		LogMsg("Choosing to load world because someone wanted access to its world cache");
		GetWorldManager->LoadMap(GetDirPath(), false);
		PreloadMap();
	}
	

	return m_pWorldCache;
}

NavGraphManager * Map::GetNavGraph()
{
	if (m_pNavGraphManager) return m_pNavGraphManager;

	m_pNavGraphManager = new NavGraphManager(this);
	return m_pNavGraphManager;
}

void Map::SetDirPath(const string &str)
{
	m_strDirPath = str; //include trailing backslash
	//also compute the map name from the dir path
	m_strMapName = ExtractFinalDirName(m_strDirPath);
}

void Map::SetDefaultTileSizeX(int size)
{
  m_defaultTileSizeX = size;
  SetModified(true) ;
}


void Map::SetDefaultTileSizeY(int size)
{
	m_uintArray[e_uintDefaultTileSizeY] = size;
	SetModified(true) ;
}

const CL_Rect *Map::GetWorldRect()
{
    return &m_mapRect;
}

CL_Rect Map::GetWorldRectInPixels()
{
	CL_Rect rec = *GetWorldRect();
	rec.left *= GetWorldChunkPixelSize();
	rec.right *= GetWorldChunkPixelSize();
	rec.top *= GetWorldChunkPixelSize();
	rec.bottom *= GetWorldChunkPixelSize();
	return rec;
}

bool Map::IsValidCoordinate(CL_Vector2 vec)
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

ScreenID Map::GetScreenID(short x, short y)
{
 return CL_MAKELONG( *(unsigned short*) &x, *(unsigned short*) &y);
// return MAKELPARAM( *(unsigned short*) &x, *(unsigned short*) &y);
}


int Map::GetXFromScreenID(ScreenID screenID)
{
    static unsigned short val;
	val =CL_LOWORD(screenID);
	return *(short*)&val;
}

int Map::GetYFromScreenID(ScreenID screenID)
{
	static unsigned short val;
	val =CL_HIWORD(screenID);
	return *(short*)&val;
}


void Map::DeleteExistingMap()
{
	
	//run through and tell everybody not to save, cheap but it works
	map_chunk_map::const_iterator itor = m_chunkMap.begin();
	while (itor != m_chunkMap.end())
	{
		itor->second->SetDataChanged(false);
		itor->second->SetNeedsThumbnailRefresh(false);
		itor++;
	}
	
	RemoveWorldFiles(GetDirPath());

	Init();
}

void Map::Init(CL_Rect worldRect)
{
    Kill();
	m_version = C_MAP_FILE_VERSION;

	if (m_defaultTileSizeX == 0)
	{
		//set some defaults
		m_worldChunkPixelSize = 512;
		SetDefaultTileSizeX(64);
		SetDefaultTileSizeY(64);
		SetThumbnailWidth(C_DEFAULT_THUMBNAIL_WIDTH);
		SetThumbnailHeight(C_DEFAULT_THUMBNAIL_HEIGHT);
		SetBGColor(CL_Color(0,0,100));
//		SetGravity(0.08f);
		SetGravity(0);
	}

	m_mapRect = worldRect;
}


Screen * Map::GetScreen(int x, int y) //screen #
{
    return GetScreen(GetScreenID(x,y));
}

Screen * Map::GetScreen(const CL_Vector2 &vecWorld) //world coords
{
	return GetScreen(GetScreenIDFromWorld(vecWorld));
}


//get a stored worldchunk or dynamically create it if needed.  The screen member is left blank, as its possible
//to say "this is part of the map, but it isn't loaded yet" for the streaming operations
//If you absolutely need the screen to be initted, use GetScreen instead
MapChunk * Map::GetMapChunk(ScreenID screenID)
{
	map_chunk_map::iterator screen = m_chunkMap.find(screenID);

	if (screen == m_chunkMap.end())
	{
		//it didn't exist ,create it
		//LogMsg("Creating screen %d.", screenID);
		MapChunk *pChunk = new MapChunk(this);
		pChunk->SetScreenID(screenID);
		m_chunkMap.insert(std::make_pair(screenID, pChunk));

		//modify our bounds to include it if required
		int x = GetXFromScreenID(screenID);
		int y = GetYFromScreenID(screenID);

			if (m_chunkMap.size() == 1)
			{
				//our only screen, set the world map to this
				m_mapRect = CL_Rect(x,y,x+1,y+1);
			}

		if (x < m_mapRect.left) m_mapRect.left = x;
		if (y < m_mapRect.top) m_mapRect.top = y;

		if (x > m_mapRect.right) m_mapRect.right = x;
		if (y > m_mapRect.bottom) m_mapRect.bottom = y;

		return pChunk;
	} 

	return screen->second;
}

MapChunk * Map::GetMapChunk(CL_Vector2 vecWorld)
{
	return GetMapChunk(GetScreenIDFromWorld(vecWorld));
}

Screen * Map::GetScreen(ScreenID screenID)
{
	return (GetMapChunk(screenID)->GetScreen());
}

//returns NULL for false
MapChunk * Map::DoesWorldChunkExist(ScreenID screenID)
{
	map_chunk_map::const_iterator itor = m_chunkMap.find(screenID);
	if (itor == m_chunkMap.end()) return NULL;
	return itor->second;
}

CL_Vector2 Map::ScreenIDToWorldPos(ScreenID screenID)
{
	
	CL_Vector2 vecPos = CL_Vector2(GetXFromScreenID(screenID), GetYFromScreenID(screenID));
	vecPos *= GetWorldChunkPixelSize();
    return vecPos;
}

ScreenID Map::GetScreenIDFromWorld(CL_Vector2 vecPos)
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

void Map::SetWorldChunkPixelSize(int widthAndHeight)
{
	m_worldChunkPixelSize = widthAndHeight;
	SetModified(true);
}

CL_Vector2 Map::SnapWorldCoords(CL_Vector2 vecWorld, CL_Vector2 vecSnap)
{
	
	vecWorld.x -= altfmod(vecWorld.x+vecSnap.x, vecSnap.x);
	vecWorld.y -= altfmod(vecWorld.y+vecSnap.y, vecSnap.y);

	return vecWorld;
}

void Map::InvalidateAllThumbnails()
{
	map_chunk_map::const_iterator itor = m_chunkMap.begin();

	while (itor != m_chunkMap.end())
	{
		itor->second->SetThumbNail(NULL);
		itor->second->SetNeedsThumbnailRefresh(true);
		itor++;
	}	
}

//sent a pointer, it handles deleting it!!
void Map::AddTile(Tile *pTile)
{
	Screen *pScreen = GetScreen(pTile->GetPos());
	pScreen->AddTile(pTile);
}


bool Map::Load(string dirPath) 
{
	Kill(); //this will save anything we have pending to save
	
	if (!dirPath.empty()) SetDirPath(dirPath);
	
	assert(m_strDirPath[m_strDirPath.length()-1] == '/' && "Load needs a path with an ending backslash on it");

	//init defaults, if loading an older file format they may be important
	
	SetDefaultTileSizeX(0);
	Init();


	m_layerManager.Load(m_strDirPath+C_LAYER_FILENAME);
	
	GetGameLogic->ShowLoadingMessage();

	CL_InputSource *pFile = g_VFManager.GetFile(m_strDirPath+C_MAP_DAT_FILENAME);
	if (!pFile)
	{
		SAFE_DELETE(pFile);
		return false;
	}

	try
	{
		//LogMsg("Loading world map...");




		CL_FileHelper helper(pFile); //will autodetect if we're loading or saving

		helper.process(m_version);
	
		CL_Rect rectTemp;
		helper.process(rectTemp);
		//we could copy that rect to m_rectWorld but it might be better to let it 'auto figure' out
		//where the bounds are in case we'd deleted a lot

		//LogMsg("Loaded rect is %d, %d, %d, %d", m_worldRect.top, m_worldRect.left, m_worldRect.right, m_worldRect.bottom);
		//now cycle through and load all the WorldChunks so we know where stuff is and what the thumbnail
		//looks like, but without actually loading the screens

		m_cameraSetting.Serialize(helper);
		helper.process(m_worldChunkPixelSize);
		helper.process(m_defaultTileSizeX);

		helper.process_smart_array(m_byteArray, e_byteCount);
		helper.process_smart_array(m_intArray, e_intCount);
		helper.process_smart_array(m_uintArray, e_uintCount);
		helper.process_smart_array(m_floatArray, e_floatCount);

		unsigned int chunkType;
		MapChunk *pWorldChunk = NULL;
		ScreenID screenID;

		helper.process(chunkType);
		//LogMsg("Loading worldchunks...");
		while (chunkType != C_DATA_MAP_END_OF_CHUNKS)
		{
			switch (chunkType)
			{
			case C_DATA_MAP_CHUNK:
				//LogMsg("Loading chunk..");
				//It's a world chunk definition, let's load it		
				helper.process(screenID); //get its screen id
				//LogMsg("%d", screenID);
				GetMapChunk(screenID)->Serialize(pFile); //GetWorldChunk will init it for us, serialize will
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

	LogMsg("Loaded map %s.  %d non-empty chunks, size is %d by %d.", GetName().c_str(), m_chunkMap.size(), GetWorldX(), GetWorldY());
	SAFE_DELETE(pFile);

	SetModified(false) ; //how could it be modified?  we just loaded it
	return true; //actually loaded something
}

bool Map::SaveRequested()
{
	if (!GetAutoSave()) return false; //we're told not save 
	if (GetGameLogic->UserProfileActive())
	{
		//do we really want to save this?  We aren't in the official editor...
		if (!GetPersistent()) return false;
	}

	return true;
}


void Map::MarkAllMapPiecesAsNeedingToSave()
{
	map_chunk_map::const_iterator itor = m_chunkMap.begin();
	while (itor != m_chunkMap.end())
	{
		if (!itor->second->IsEmpty())
		{
			itor->second->GetScreen(); //just accessing it causes it to precache
			itor->second->SetDataChanged(true);
		}
		itor++;
	}
}

void Map::ForceSaveNow()
{
	SetModified(true) ;
	MarkAllMapPiecesAsNeedingToSave();
	Save(true);

	map_chunk_map::const_iterator itor = m_chunkMap.begin();
	while (itor != m_chunkMap.end())
	{
		if (!itor->second->IsEmpty())
		{
			itor->second->GetScreen()->Save(); //just accessing it causes it to precache
		}
		itor++;
	}
}

void Map::SetDataChanged(bool bChanged)
{
	m_bDataChanged = bChanged;
}
void Map::SetModified(bool bModified)
{
	m_bDataChanged = bModified;
}

bool Map::GetModified()
{

	if (m_bDataChanged) return true;

	map_chunk_map::const_iterator itor;

		//if a worldchunk changed (like the thumbnail, or a worldchunk was added), we need to save everything anyway.  this is kind of bad design..

		itor = m_chunkMap.begin();

		while (itor != m_chunkMap.end())
		{
			if (itor->second->GetChunkDataChanged())
			{
				return true;
			}

			if (itor->second->GetDataChanged())
			{
				return true;
			}

			itor++;
		}

		return false;
}


bool Map::Save(bool bSaveTagCacheAlso)
{
	if (m_defaultTileSizeX == 0) return false; //don't actually save.. nothing was loaded

	assert(m_strDirPath[m_strDirPath.length()-1] == '/' && "Save needs a path with an ending backslash on it");

	g_VFManager.CreateDir(m_strDirPath); //creates the dir if needed in the last mounted file tree

	m_layerManager.Save(m_strDirPath+C_LAYER_FILENAME);
	bool bScreenDataWasModified = false;

	//do we absolutely have to save?
	bool bRequireSave = m_bDataChanged;

	map_chunk_map::const_iterator itor;

	if (bRequireSave == false)
	{
		//if a worldchunk changed (like the thumbnail, or a worldchunk was added), we need to save everything anyway.  this is kind of bad design..

		itor = m_chunkMap.begin();

		while (itor != m_chunkMap.end())
		{
				if (itor->second->GetChunkDataChanged())
				{
					bRequireSave = true;
					break;
				}

				if (itor->second->GetDataChanged())
				{
					bScreenDataWasModified = true;
					break;
				}
			
			itor++;
		}
	}

	if ( (bScreenDataWasModified  || bRequireSave) && !ExistsInModPath(m_strDirPath+C_MAP_DAT_FILENAME))
	{
		//oh oh, we've modified a map we're modding.  To avoid nasty tagcache issues we'll need to
		//copy the entire thing here.
		MarkAllMapPiecesAsNeedingToSave();
		bRequireSave = true;
	}
	if (bScreenDataWasModified || bRequireSave)
	{
		GetTagManager->Save(this);
	}

	if (!bRequireSave && !bScreenDataWasModified ) return true; //abort the save, not needed

		
	
		if (!GetApp()->GetMyGameLogic()->UserProfileActive())
		{
			//also a good time to save out used textures, used for missing texture errors
			GetHashedResourceManager->SaveUsedResources(this, m_strDirPath);
		}
	

	LogMsg("Saving map header %s - (%d chunks to look at, map size %d by %d)", GetName().c_str(), m_chunkMap.size(),
		GetWorldX(), GetWorldY());

	//first save our map.dat file
	CL_OutputSource *pFile = g_VFManager.PutFile(m_strDirPath+C_MAP_DAT_FILENAME);
	
	CL_FileHelper helper(pFile); //will autodetect if we're loading or saving
	
	m_version = C_MAP_FILE_VERSION;
	helper.process(m_version);
	helper.process(m_mapRect);
	m_cameraSetting.Serialize(helper);
	helper.process(m_worldChunkPixelSize);
	helper.process(m_defaultTileSizeX);

	helper.process_smart_array(m_byteArray, e_byteCount);
	helper.process_smart_array(m_intArray, e_intCount);
	helper.process_smart_array(m_uintArray, e_uintCount);
	helper.process_smart_array(m_floatArray, e_floatCount);

	//run through every screen that is currently open and ask it to save itself if it hasn't been
	
	itor = m_chunkMap.begin();
	while (itor != m_chunkMap.end())
	{
		if (!itor->second->IsEmpty())
		{
			assert(sizeof(C_DATA_MAP_CHUNK) == 4 && "This should be an unsigned int.. uh.. ");
			helper.process_const(C_DATA_MAP_CHUNK);
			helper.process_const(itor->second->GetScreenID());

			itor->second->Serialize(pFile);
		}
		itor++;
	}

	helper.process_const(C_DATA_MAP_END_OF_CHUNKS); //signal the end
	SAFE_DELETE(pFile);
	return true;
}

void Map::PreloadMap()
{
	map_chunk_map::const_iterator itor = m_chunkMap.begin();
	
	GetNavGraph()->SetPerformLinkOnAdd(false);

	while (itor != m_chunkMap.end())
	{
		if (!itor->second->IsEmpty())
		{
			itor->second->GetScreen(); //just accessing it causes it to precache
		}
		itor++;
	}	

	GetNavGraph()->SetPerformLinkOnAdd(true);
	BuildNavGraph();
}

void Map::BuildNavGraph()
{
	if (!IsInitted()) return;

	GetNavGraph()->Clear();

	tile_list tileList;
	tile_list::iterator tileItor;
	
	
	map_chunk_map::const_iterator itor = m_chunkMap.begin();
	while (itor != m_chunkMap.end())
	{
		if (!itor->second->IsEmpty() && itor->second->IsScreenLoaded())
		{
			itor->second->GetScreen()->LinkNavGraph();
		}
		itor++;
	}	
}

void Map::GetAllWorldChunksWithinThisRect(std::vector<MapChunk*> &wcVector, CL_Rect rec, bool bIncludeBlanks)
{
	assert(wcVector.size() == 0 && "Shouldn't this be cleared before you send it?");

	int startingX = rec.left;

	CL_Rect scanRec;
	MapChunk *pWorldChunk;
	CL_Rect screenRec;

	bool bScanMoreOnTheRight, bScanMoreOnTheBottom;

	while (1)
	{
		scanRec = rec;

		//get the screen the upper left is on
		pWorldChunk = GetMapChunk(CL_Vector2(scanRec.left, scanRec.top));
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

void Map::ReInitEntities()
{
	tile_list tileList;
	tile_list::iterator tileItor;
	MovingEntity *pEnt;
	int tileType;

	if (!IsInitted()) return;

	map_chunk_map::const_iterator itor = m_chunkMap.begin();
	while (itor != m_chunkMap.end())
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
					//we don't want to reinit things attached to the camera because generally those are GUI elements and get screwy
					if (pEnt && pEnt->GetAttachEntityID() != C_ENTITY_CAMERA) 
					{
						float orientation = pEnt->GetBody()->GetOrientation();
						CL_Vector2 facing = pEnt->GetVectorFacing();
						pEnt->Kill();
						pEnt->SetVectorFacing(facing);
						pEnt->Init();
						pEnt->GetBody()->SetOrientation(orientation);
						pEnt->RunOnMapInsertIfNeeded();
					}

					break;
				}

			}
		}
		itor++;
	}	
}

void Map::RemoveUnusedFileChunks()
{
	
	vector<string> paths;
	g_VFManager.GetMountedDirectories(&paths);

	
	CL_DirectoryScanner scanner;
	scanner.scan(paths.back() +"/" +m_strDirPath, "*.chunk");
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
				g_VFManager.RemoveFile(m_strDirPath+scanner.get_name());
			}
		}
	}
}

void Map::ReInitCollisionOnTilePics()
{
	tile_list tileList;
	tile_list::iterator tileItor;
	TilePic *pTilePic;
	int tileType;

	if (!IsInitted()) return;

	map_chunk_map::const_iterator itor = m_chunkMap.begin();
	while (itor != m_chunkMap.end())
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

void Map::AddWarpTagHashID(unsigned int hashID)
{
	m_warpTagHashIDList.push_back(hashID);
}

void Map::RemoveWarpTagHashID(unsigned int hashID)
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
	RemoveFile(path+C_MAP_DAT_FILENAME);
	RemoveFile(path+C_LAYER_FILENAME);
}

void Map::SetMyWorldCache(EntMapCache *pWorldCache)
{
	 m_pWorldCache = pWorldCache;
}



//natural docs stuff
/*
Object: Map
A Map is a single area that can be any size and contain any amount of tiles and entities.

Most of these map settings are also available in the editor under Map Properties.

If the map is saved (by defaults maps auto-save and are persistent), changes made are remembered.

See also:

<MapManager>

Group: Member Functions

func: SetPersistent
(code)
nil SetPersistent(boolean bPersistent)
(end)
By default, maps are persistent, meaning all changes are remembered and automatically saved in the player's profile. (the original is not changed, only the player's modified version of it)

This only has meaning when a player profile is active.

Parameters:

bPersistant - True if the map should be persistent and changes remembered, false if not

func: GetPersistent
(code)
boolean GetPersistent()
(end)

Returns:

True if the map is persistent and changes will be remembered on a per-player-profile basis.

func: SetAutoSave
(code)
nil SetAutoSave(boolean bAutoSave)
(end)

This only applies when a visual profile is not yet loaded.

For title screens, auto-save is usually turned off because no player profile is loaded, yet we don't want to save after we mess up the screen.

Parameters:

bAutoSave - True if the map should automatically be saved without needing to choose "Save map now" from the editor.

func: GetAutoSave
(code)
boolean GetAutoSave()
(end)

Returns:

True if auto-save is currently enabled.

func: GetName
(code)
string GetName()
(end)

Returns:

The map name.  (the name of its directory)

func: GetLayerManager
(code)
LayerManager GetLayerManager()
(end)

Returns:

This map's <LayerManager>, containing all information about its layers and layer settings.

func: BuildLocalNavGraph
(code)
nil BuildLocalNavGraph()
(end)

Rebuilds the navigational graph for this map.

In general, this isn't needed, as navigational graphs are grown/destroyed fluidly on the fly as the map changes.

func: GetCollisionByRay
(code)
Zone GetCollisionByRay(Vector2 vStartPos, Vector2 vDir, number rayRange, number raySpread, Entity entToIgnore, number mode, boolean bIgnoreCreatures)
(end)

Allows you to shoot a ray from any point in this map and see what it hits.

The returned <Zone> object's *vPos* member will contain the exact position of the hit, its other values will contain additional information about the collision.

Usage:
(code)
//let's shoot a ray in front of this entity and see if it detects any other entities)

local rayRange = 80;
local raySpread = 8; //causes 5 rays to be shot in a spread formation 8 units apart, easier to detect hits.  0 to use 1 ray only
local entToIgnore = this;
local hitZone = this:GetMap():GetCollisionByRay(this:GetPos(), this:GetVectorFacing(), rayRange, raySpread, entToIgnore, C_RAY_ENTITIES, false);

if (hitZone.entityID != C_ENTITY_NONE) then
	//we have an entity in front of us!
	
	local ent = GetEntityByID(hitZone.entityID);
	LogMsg("Entity " .. tostring(ent) .. " is sitting in front of us.");

end

(end)

Parameters:

vStartPos - A <Vector2> object containing the start position of the ray.
vDir - A unit vector containing the direction of the ray.
rayRange - How far the ray can reach.
raySpread - 0 for a single ray, otherwise will shoot five rays in a spread formation, this distance apart from each one.
entToIgnore - An <Entity> we should ignore during the check, otherwise nil
mode - One of the <C_RAY_CONSTANTS>.  Use <C_RAY_DEBUG_MODE> to visually see the rays being shot.
bIgnoreCreature - If true, creatures are ignored during the check.

Returns:

A <Zone> object containing information on what was hit.  The Zone's materialID will be <C_MATERIAL_TYPE_NONE> if no collision occured.


func: GetTilesByRect
(code)
TileList GetTilesByRect(Rect rect, LayerList layers, boolean bWithCollisionOnly)
(end)
This allows you to grab a list of all the entities and tiles in a rectangular area of the map.

You can later cycle through the list examining each one, or move or copy the list to a new place. (well, the moving/pasting as a group isn't accessible yet in script, coming soon?)

Usage:
(code)

//grab all tiles/entityes we're touching

local layerList = this:GetMap():GetLayerManager():GetVisibleLayers(); //what layers we'll scan
local tileList = this:GetMap():GetTilesByRect( Rect(this:GetWorldCollisionRect()), layerList, false); //grab them

LogMsg("Found " .. tileList:GetCount() .. " tiles after scanning the " .. layerList:GetCount() .. " layers.");

//ok, now we have our list and need to run through and look at each one

local tile; 

while true do

tile = tileList:GetNext();
	if (tile == nil) then break; end;

	LogMsg("Found tile type " .. tile:GetType());

	if (tile:GetType() == C_TILE_TYPE_ENTITY and tile:GetAsEntity():GetID() != this:GetID()) then
		LogMsg("We are standing near an entity that isn't us! It's ID is " ..  tile:GetAsEntity():GetID());
	end
end
(end)

Parameters:

rect - A <Rect> object containing the area we should grab tiles from.  Any tile/entity that overlays this area will be included.
layers - A <LayerList> object containing which layers should be included for the search.
bWidthCollisionOnly - If true, entities/tiles without collision information will be ignored.

Returns:

A <TileList> object containing handles to all the tiles/entities found. If <TileList::GetCount> is 0, nothing was found.

Section: Related Constants

Group: C_RAY_CONSTANTS
Used with <Map::GetCollisionByRay>.

constant: C_RAY_ENTITIES
The ray will hit only entities.

constant: C_RAY_TILE_PIC
The ray will hit only tile pics.

constant: C_RAY_EVERYTHING
The ray will hit entities as well as tile pics.

constant: C_RAY_DEBUG_MODE
The rays will be visually drawn on the screen, helps to figure out problems.  Slow, so don't leave this on. Hits everything.

*/


