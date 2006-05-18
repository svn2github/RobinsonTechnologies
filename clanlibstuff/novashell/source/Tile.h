
/* -------------------------------------------------
* Copyright 2005 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 28:12:2005   11:45
*/


#ifndef Tile_HEADER_INCLUDED // include guard
#define Tile_HEADER_INCLUDED  // include guard

#include "misc/CBit8.h"

class Screen;
class MovingEntity;
class CollisionData;
class CL_Surface;
class CBody;

enum
{
	//only add to the BOTTOM!!
	C_TILE_TYPE_BLANK = 0, //used as a place holder in some edit operations
	C_TILE_TYPE_PIC,
	C_TILE_TYPE_REFERENCE, //not a real tile, but a ghost pointing to the real one.
	C_TILE_TYPE_ENTITY
};

class Tile
{
public:

    Tile();
	virtual ~Tile();
	cl_uint8 GetType() {return m_type;}
	const cl_uint8 GetLayer() const {return m_layer;}
	void SetLayer(cl_uint8 layer) {m_layer = layer;}
	bool GetBit(cl_uint8 bit) {return m_bitField.get_bit(bit);}
	void SetBit(cl_uint8 bit, bool bNew) {m_bitField.set_bit(bit, bNew);}
	const CL_Vector2 & GetScale() {return m_vecScale;}
	virtual const CL_Vector2 & GetPos() {return m_vecPos;}
	virtual void SetPos(const CL_Vector2 &vecPos) {m_vecPos = vecPos;}
	bool IsEdgeCase() {return m_pEdgeCaseList != NULL;}
	virtual Tile * CreateClone(); //must be handled in derived class
	Tile * CopyFromBaseToClone(Tile *pNew);

	virtual void Serialize(CL_FileHelper &helper){}; //must be handled in derived class
	virtual void SerializeBase(CL_FileHelper &helper);
	virtual CL_Vector2 GetBoundsSize() {return CL_Vector2(64,64);}
	virtual CL_Rect GetBoundsRect() {return CL_Rect(int(m_vecPos.x), int(m_vecPos.y), int(m_vecPos.x+64),int(m_vecPos.y+64));}
	virtual CL_Rectf GetWorldRect() {return CL_Rectf(m_vecPos.x, m_vecPos.y, m_vecPos.x+64,m_vecPos.y+64);}
	Tile * CreateReference(Screen *pScreen);
	void RemoveReference(Tile *pTileRef);
	Tile * GetTileWereAReferenceFrom(){return m_pFatherTile;}
	unsigned int GetLastScanID() {return m_lastScanID;};
	void SetLastScanID(unsigned int scanID) {m_lastScanID = scanID;}
	virtual CollisionData * GetCollisionData() {return NULL; } //by default tiles don't have collision data
	bool UsesTileProperties();
	virtual void Update(float step) {return;}
	virtual void Render(CL_GraphicContext *pGC) {return;}
	Screen * GetParentScreen();
	void SetParentScreen(Screen *pScreen) {m_pParentScreen = pScreen;}
	virtual CBody * GetCustomBody() {return NULL;}
	
enum
{
 e_flippedX = D_BIT_0,
 e_flippedY = D_BIT_1,
 e_customCollision = D_BIT_2,
 e_needsUpdate = D_BIT_3,
 e_notPersistant = D_BIT_4 //if true, won't be saved to disk
};

protected:
	
	cl_uint8 m_type;

	//this data is around for every tile type wether you need it or not, but actually gets saved and
	//loaded in subclasses
	CL_Vector2 m_vecPos; //where we're located if applicable
	CL_Vector2 m_vecScale;
	unsigned int m_lastScanID; //helps to avoid duplicates when doing certain kinds of scans
	std::list<Tile*> *m_pEdgeCaseList; //list of references a tile has
	Tile *m_pFatherTile; //if a reference, this holds its father
	Screen *m_pParentScreen; //only valid sometimes, like for reference tiles, not kept when
	//making clones and things
	cl_uint8 m_layer; //higher means on top
	CBit8 m_bitField;
	CollisionData * m_pCollisionData; //null if it hasn't been researched yet
};

class TilePic: public Tile
{
public:

	TilePic()
	{
		m_type = C_TILE_TYPE_PIC;
		m_rot = 0;
	}
	
	virtual Tile * CreateClone();
	virtual void Serialize(CL_FileHelper &helper);
	//TODO optimize these to be precached
	virtual CL_Vector2 GetBoundsSize() {return CL_Vector2(m_rectSrc.get_width(), m_rectSrc.get_height());}
	virtual CL_Rect GetBoundsRect() {return CL_Rect(int(m_vecPos.x), int(m_vecPos.y), 
		int(m_vecPos.x + m_rectSrc.get_width()), int(m_vecPos.y + m_rectSrc.get_height()));}
	virtual CL_Rectf GetWorldRect() {return CL_Rectf(m_vecPos.x, m_vecPos.y, 
		m_vecPos.x + m_rectSrc.get_width(), m_vecPos.y + m_rectSrc.get_height());}
	virtual CollisionData * GetCollisionData();
	virtual void Render(CL_GraphicContext *pGC);
	unsigned int m_resourceID;
	CL_Rect m_rectSrc;
	float m_rot; //rotation

};

#endif                  // include guard
