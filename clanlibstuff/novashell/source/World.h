
/* -------------------------------------------------
* Copyright 2006 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
*/

#pragma once

/*
This world class keeps track of the individual chunks that make up the world.  
*/

#define C_DEFAULT_WORLD_X 1 //how many screens we start with by default
#define C_DEFAULT_WORLD_Y 1
#define C_DEFAULT_SCREEN_ID 0
extern const CL_Vector2 g_worldDefaultCenterPos;
#define C_WORLD_DAT_FILENAME "world.dat"

#include "main.h"
#include "Screen.h"
#include "Tile.h"
#include "WorldChunk.h"
#include "CameraSetting.h"
#include "LayerManager.h"

class EntWorldCache;
class NavGraphManager;

typedef std::map<ScreenID, WorldChunk*> WorldMap;

typedef std::list<unsigned int> tag_hash_list;
#define C_WORLD_FILE_VERSION 0

enum
{
	C_DATA_UNKNOWN = 0,
	C_DATA_WORLD_CHUNK,
	C_DATA_WORLD_END_OF_CHUNKS
};

class World
{
public:

    World();
    ~World();

    void Kill();
    void Init(const CL_Rect worldRect = CL_Rect(C_DEFAULT_SCREEN_ID,C_DEFAULT_SCREEN_ID,C_DEFAULT_SCREEN_ID+1,C_DEFAULT_SCREEN_ID+1)); //you can send in a requested worldrect if you want,
	//other it calculates the middle and puts a screen there to start you off

    const CL_Rect *GetWorldRect(); //rect in screenchunks
	CL_Rect GetWorldRectInPixels(); //full world bounds in rect coordinates
    int GetWorldX(){return m_worldRect.get_width();}
    int GetWorldY(){return m_worldRect.get_height();}
    int GetXFromScreenID(ScreenID screenID);
    int GetYFromScreenID(ScreenID screenID);
    WorldChunk * DoesWorldChunkExist(ScreenID screenID);
    ScreenID GetScreenID(short x, short y);
	WorldChunk * GetWorldChunk(ScreenID screenID); //world chunk id
	WorldChunk * GetWorldChunk(CL_Vector2 vecWorld); //world coordinates
    Screen * GetScreen(ScreenID screenID); //a screen is a member of worldchunk, this inits it if needed
    Screen * GetScreen(int x, int y); //like above but automatically converts x/y into a "screenID" for you
	Screen * GetScreen(const CL_Vector2 &vecPos); //like above but from world coords
	CL_Vector2 ScreenIDToWorldPos(ScreenID screenID);
    ScreenID GetScreenIDFromWorld(CL_Vector2 vecPos);
	//tile_list * GetTileListByWorld(CL_Vector2 vecPos, int layer, Screen **ppScreenOut);
	CL_Vector2 SnapWorldCoords(CL_Vector2 vecWorld, int snapSize);
	const string &GetDirPath(){return m_strDirPath;};//it includes the trailing backslash
	void SetDirPath(const string &str);
	bool Load(string dirPath=""); //attempts to load, returns false if nothing was loaded. It also
	//calls SetDirPath on what is sent in
	bool Save(bool bSaveTagCacheAlso);
	WorldMap * GetWorldMap() {return &m_worldMap;}
	int GetWorldChunkPixelSize() {return m_worldChunkPixelSize;}
	int GetDefaultTileSize() {return m_defaultTileSize;}
	void DeleteExistingMap();
	bool IsInitted() {return m_defaultTileSize != 0;}
	void SetDefaultTileSize(int size);
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
	float GetCacheSensitivity() {return m_floatArray[e_floatCacheSensitivity]; m_bDataChanged = true;}
	void SetCacheSensitivity(float sensitivity) {m_floatArray[e_floatCacheSensitivity] = sensitivity;}
	void InvalidateAllThumbnails();
	void AddTile(Tile *pTile); //adds a tile automatically based on its pos/layer info
	void GetAllWorldChunksWithinThisRect(std::vector<WorldChunk*> &wcVector, CL_Rect rec, bool bIncludeBlanks);
	void PreloadMap();
	LayerManager & GetLayerManager() {return m_layerManager;}
	bool IsValidCoordinate(CL_Vector2 vec);
	cl_uint8 GetVersion(){return m_version;}
	const string & GetName(){return m_strMapName;}
	bool GetSnapEnabled() {return m_byteArray[e_byteSnapOn] != 0;}
	void SetSnapEnabled(bool bNew) {m_byteArray[e_byteSnapOn] = bNew; m_bDataChanged = true;}
	void SetMyWorldCache(EntWorldCache *pWorldCache) {m_pWorldCache = pWorldCache;}
	bool GetPersistent() {return m_byteArray[e_byteNotPersistent] == 0;}
	void SetPersistent(bool bNew) {m_byteArray[e_byteNotPersistent] = !bNew; m_bDataChanged = true;}
	bool GetAutoSave() {return m_byteArray[e_byteAutoSave] == 0;}
	void SetAutoSave(bool bNew) {m_byteArray[e_byteAutoSave] = !bNew; m_bDataChanged = true;}
	bool SaveRequested();
	void ForceSaveNow();
	void RemoveUnusedFileChunks();

	EntWorldCache * GetMyWorldCache();
	void ReInitEntities(); //reinits all cached entities in this world, useful after 
	//changing a script
	void ReInitCollisionOnTilePics(); 
	NavGraphManager * GetNavGraph();
	bool NavGraphDataExists() {return m_pNavGraphManager != 0;}
	int GetMasterNavMapID();
	void SetMasterNavMapID(int newID) {m_masterNavMapID = newID;}

	void AddWarpTagHashID(unsigned int hashID);
	void RemoveWarpTagHashID(unsigned int hashID);
	tag_hash_list & GetWarpTagHashList() {return m_warpTagHashIDList;}

private:

	bool TestCoordPacker(int x, int y);

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
	CL_Rect m_worldRect;
	int m_worldChunkPixelSize;
	int m_defaultTileSize; //default is 64
	//stuff we don't
	WorldMap m_worldMap;

	string m_strDirPath; //where we're located, not saved, set upon loading, I mean, we'd know if we know where to load it, right?
	string m_strMapName; //we get this by cutting off the last part of the dir name
	CameraSetting m_cameraSetting; //to remember out last camera position
	LayerManager m_layerManager;
	EntWorldCache *m_pWorldCache; //cached here for speed

	bool m_bDataChanged; //only applicable to what is in this file
	NavGraphManager *m_pNavGraphManager;
	int m_masterNavMapID; //a central node that connects to all warps on this map
	tag_hash_list m_warpTagHashIDList; //keep track of the hash's of named tagobjects WARPS that exist and belong to this map (note, only warps!)

};

void RemoveWorldFiles(const string &path); //util for deleting stuff

