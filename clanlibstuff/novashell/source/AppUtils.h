#ifndef AppUtils_HEADER_INCLUDED // include guard
#define AppUtils_HEADER_INCLUDED  // include guard

#include "Screen.h"

void BlitMessage(string msg);
string ColorToString(const CL_Color &colr);
CL_Color StringToColor(const string &stColor);
CL_Rect StringToRect(const string &stColor);
string RectToString(const CL_Rect &r);

void RenderVertexList(const CL_Vector2 &pos, CL_Vector2 *pVertArray, int vertCount, CL_Color &colr, CL_GraphicContext *pGC);
CL_Vector2 MakeNormal(CL_Vector2 &a, CL_Vector2 &b);
void RenderVertexListRotated(const CL_Vector2 &pos, CL_Vector2 *pVertArray, int vertCount, CL_Color &colr, CL_GraphicContext *pGC,  float angleRad);
void DrawRectFromWorldCoordinates(CL_Vector2 vecStart, CL_Vector2 vecStop, CL_Color borderColor, CL_GraphicContext *pGC);
void DrawRectFromWorldCoordinatesRotated(CL_Vector2 vecStart, CL_Vector2 vecStop, CL_Color borderColor, CL_GraphicContext *pGC, float angleRad);
string GetStrippedFilename(string str);
unsigned int FileNameToID(const char * filename);
bool GetTileLineIntersection(const CL_Vector2 &vStart, const CL_Vector2 &vEnd, tile_list &tList, CL_Vector2 *pvColPos, Tile* &pTileOut, const Tile * const pTileToIgnore  = NULL, int limitSearchToThisTileType = C_TILE_TYPE_BLANK);
string PrintVector(CL_Vector2 v);
string PrintRect(CL_Rectf r);
void ResetFont(CL_Font *pFont); //set the centering, color and alpha back to default 
string ExtractFinalDirName(string path); //not tested with paths with filenames yet
bool compareLayerBySort(unsigned int pA, unsigned int pB); //for use with stl::sort

typedef cl_uint32 CL_DWORD_PTR; //note, for a 64 bit compiler we'll need to make sure this is 64 bits somehow...

#define CL_MAKELONG(a, b)      ((cl_uint32)(((cl_uint16)((CL_DWORD_PTR)(a) & 0xffff)) | ((cl_uint32)((cl_uint16)((CL_DWORD_PTR)(b) & 0xffff))) << 16))
#define CL_LOWORD(l)           ((cl_uint16)((cl_uint32)(l) & 0xffff))
#define CL_HIWORD(l)           ((cl_uint16)((cl_uint32)(l) >> 16))


class GameTimer
{
public:

	GameTimer()
	{
		m_timer = 0;
	}
	void Reset();
	bool IntervalReached();

	bool IsActive() {return m_timer != 0;}

	void SetInterval(int interval)
	{
		m_interval = interval;
		Reset();
	}

	unsigned int m_timer;
	int m_interval;
};


#endif                  // include guard



