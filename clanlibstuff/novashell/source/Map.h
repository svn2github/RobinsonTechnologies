
/* -------------------------------------------------
* Copyright 2006 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
*/

#pragma once

/*
This world class keeps track of the individual chunks that make up the world.  
*/

#define C_DEFAULT_SCREEN_ID 0
extern const CL_Vector2 g_mapDefaultCenterPos;
#define C_MAP_DAT_FILENAME "map.dat"

#include "main.h"
#include "Screen.h"
#include "Tile.h"
#include "MapChunk.h"
#include "CameraSetting.h"
#include "LayerManager.h"

class EntMapCache;
class NavGraphManager;

typedef std::map<ScreenID, MapChunk*> map_chunk_map;

typedef std::list<unsigned int> tag_hash_list;
#define C_MAP_FILE_VERSION 1

enum
{
	C_DATA_UNKNOWN = 0,
	C_DATA_MAP_CHUNK,
	C_DATA_MAP_END_OF_CHUNKS
};

class Map
{
public:

    Map();
    ~Map();

    void Kill();
    void Init(const CL_Rect worldRect = CL_Rect(C_DEFAULT_SCREEN_ID,C_DEFAULT_SCREEN_ID,C_DEFAULT_SCREEN_ID+1,C_DEFAULT_SCREEN_ID+1)); //you can send in a requested worldrect if you want,
	//other it calculates the middle and puts a screen there to start you off

    const CL_Rect *GetWorldRect(); //rect in screenchunks
	CL_Rect GetWorldRectInPixels(); //full world bounds in rect coordinates
    int GetWorldX(){return m_mapRect.get_width();}
    int GetWorldY(){return m_mapRect.get_height();}
    int GetXFromScreenID(ScreenID screenID);
    int GetYFromScreenID(ScreenID screenID);
    MapChunk * DoesWorldChunkExist(ScreenID screenID);
    ScreenID GetScreenID(short x, short y);
	MapChunk * GetMapChunk(ScreenID screenID); //map chunk id
	MapChunk * GetMapChunk(CL_Vector2 vecWorld); //in world coordinates
    Screen * GetScreen(ScreenID screenID); //inits it if needed
    Screen * GetScreen(int x, int y); //like above but automatically converts x/y into a "screenID" for you
	Screen * GetScreen(const CL_Vector2 &vecPos); //like above but from world coords
	CL_Vector2 ScreenIDToWorldPos(ScreenID screenID);
    ScreenID GetScreenIDFromWorld(CL_Vector2 vecPos);
	CL_Vector2 SnapWorldCoords(CL_Vector2 vecWorld, CL_Vector2 vecSnap);
	const string &GetDirPath(){return m_strDirPath;};//it includes the trailing backslash
	void SetDirPath(const string &str);
	bool Load(string dirPath=""); //attempts to load, returns false if nothing was loaded. It also
									//calls SetDirPath on what is sent in
	bool Save(bool bSaveTagCacheAlso);
	map_chunk_map * GetChunkMap() {return &m_chunkMap;}
	int GetWorldChunkPixelSize() {return m_worldChunkPixelSize;}
	int GetDefaultTileSizeX() {return m_defaultTileSizeX;}
	int GetDefaultTileSizeY() {return m_uintArray[e_uintDefaultTileSizeY];}
	void DeleteExistingMap();
	bool IsInitted() {return m_defaultTileSizeX != 0;}
	void SetDefaultTileSizeX(int size);
	void SetDefaultTileSizeY(int size);
	void SetWorldChunkPixelSize(int widthAndHeight);
	CameraSetting * GetCameraSetting() {return &m_cameraSetting;}
	CL_Color GetBGColor() {CL_Color color; color.color = m_uintArray[e_uintBGColor]; return color;}
	void SetBGColor(CL_Color col){m_uintArray[e_uintBGColor] = col.color;}
	void SetGravity(float grav){m_floatArray[e_floatGravity] = grav; m_bDataChanged = true;}
	float GetGravity() {return m_floatArray[e_floatGravity];}
	void SetThumbnailWidth(int width) {m_intArray[e_intThumbnailWidth] = width; m_bDataChanged = true;}
	void SetThumbnailHeight(int height) {m_intArray[e_intThumbnailHeight] = height; m_bDataChanged = true;}
	int GetThumbnailWidth() {return m_intArray[e_intThumbnailWidth];}
	int GetThumbnailHeight() {return m_intArray[e_intThumbnailHeight];}
	float GetCacheSensitivity() {return m_floatArray[e_floatCacheSensitivity];}
	void SetCacheSensitivity(float sensitivity) {m_floatArray[e_floatCacheSensitivity] = sensitivity; m_bDataChanged = true;}
	void InvalidateAllThumbnails();
	void AddTile(Tile *pTile); //adds a tile automatically based on its pos/layer info
	void GetAllWorldChunksWithinThisRect(std::vector<MapChunk*> &wcVector, CL_Rect rec, bool bIncludeBlanks);
	void PreloadMap();
	LayerManager & GetLayerManager() {return m_layerManager;}
	bool IsValidCoordinate(CL_Vector2 vec);
	cl_uint8 GetVersion(){return m_version;}
	const string & GetName(){return m_strMapName;}
	bool GetSnapEnabled() {return m_byteArray[e_byteSnapOn] != 0;}
	void SetSnapEnabled(bool bNew) {m_byteArray[e_byteSnapOn] = bNew; m_bDataChanged = true;}
	void SetMyWorldCache(EntMapCache *pWorldCache);
	bool GetPersistent() {return m_byteArray[e_byteNotPersistent] == 0;}
	void SetPersistent(bool bNew) {m_byteArray[e_byteNotPersistent] = !bNew; m_bDataChanged = true;}
	bool GetAutoSave() {return m_byteArray[e_byteAutoSave] == 0;}
	void SetAutoSave(bool bNew) {m_byteArray[e_byteAutoSave] = !bNew; m_bDataChanged = true;}
	bool SaveRequested();
	void ForceSaveNow();
	void RemoveUnusedFileChunks();

	EntMapCache * GetMyMapCache();
	void ReInitEntities(); //reinits all cached entities in this world, useful after 
	//changing a script
	void ReInitCollisionOnTilePics(); 
	NavGraphManager * GetNavGraph();
	bool NavGraphDataExists() {return m_pNavGraphManager != 0;}

	void AddWarpTagHashID(unsigned int hashID);
	void RemoveWarpTagHashID(unsigned int hashID);
	tag_hash_list & GetWarpTagHashList() {return m_warpTagHashIDList;}
	void SaveAndKill();
	bool IsWorldCacheInitted() {return m_pWorldCache != NULL;}
	void SetDataChanged(bool bNew);
	void BuildNavGraph();
	bool GetModified(); //this checks the world and all screesn to see if anything needs to be saved
	void SetModified(bool bModified);

private:

	bool TestCoordPacker(int x, int y);
	void MarkAllMapPiecesAsNeedingToSave();

	//you can add new variables by adding enums, the datafile will stay compatible automatically
	enum
	{
		e_intThumbnailWidth = 0,
		e_intThumbnailHeight,
	
		//add more above here
		e_intCount
	};
	enum
	{
		e_byteSnapOn = 0,
		e_byteNotPersistent,
		e_byteAutoSave,
		
		//add more above here
		e_byteCount
	};

	enum
	{
		e_floatCacheSensitivity = 0,
		e_floatViewSensitivity,
		e_floatGravity,

		//add more above here
		e_floatCount
	};

	enum
	{
		e_uintBGColor = 0, //what color the background clear should be
		e_uintDefaultTileSizeY,
		//add more above here
		e_uintCount
	};

	cl_uint8 m_byteArray[e_byteCount];
	cl_int32 m_intArray[e_intCount];
	cl_uint32 m_uintArray[e_uintCount];
	float m_floatArray[e_floatCount];

	//stuff we actually save/load, don't change the save/load order, ok? Just add more
	//at the end.
	cl_uint8 m_version;
	CL_Rect m_mapRect;
	int m_worldChunkPixelSize;
	int m_defaultTileSizeX; //default is 64
	//stuff we don't
	map_chunk_map m_chunkMap;

	string m_strDirPath; //where we're located, not saved, set upon loading, I mean, we'd know if we know where to load it, right?
	string m_strMapName; //we get this by cutting off the last part of the dir name
	CameraSetting m_cameraSetting; //to remember out last camera position
	LayerManager m_layerManager;
	EntMapCache *m_pWorldCache; //cached here for speed

	bool m_bDataChanged; //only applicable to what is in this file
	NavGraphManager *m_pNavGraphManager;
	int m_masterNavMapID; //a central node that connects to all warps on this map
	tag_hash_list m_warpTagHashIDList; //keep track of the hash's of named tagobjects WARPS that exist and belong to this map (note, only warps!)

	bool m_bKillingMap; //true if we're killing everything
};

void RemoveWorldFiles(const string &path); //util for deleting stuff

