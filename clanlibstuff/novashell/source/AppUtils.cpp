#include "AppPrecomp.h"
#include "GameLogic.h"
#include "CollisionData.h"
#include "MaterialManager.h"



bool compareLayerBySort(unsigned int pA, unsigned int pB) 
{
	LayerManager *pLayerManager = &GetWorld->GetLayerManager();

	return pLayerManager->GetLayerInfo(pA).GetSort() <
		pLayerManager->GetLayerInfo(pB).GetSort();
}

string ExtractFinalDirName(string path)
{
	int offset;

	if (path[path.size()-1] == '\\' ||
		path[path.size()-1] == '/')
	{
		//remove trailing slash
		path = CL_String::left(path, path.size()-1);
	}

	//snip off the final directory name
	offset=path.find_last_of("/\\");
	if (offset != -1)
	{
		path = CL_String::right(path, (path.size()-offset)-1);
	} 

	return path;
}

string PrintRect(CL_Rectf r)
{
	char stTemp[256];

	sprintf(stTemp, "Top: %.2f, Left: %.2f, Bottom: %.2f, Right: %.2f",
		r.top, r.left, r.bottom, r.right);

	return string(stTemp);
}


string PrintVector(CL_Vector2 v)
{
	char stTemp[256];
	sprintf(stTemp, "X: %.3f, Y: %.3f", v.x, v.y);
	return string(stTemp);
}

//cuts off the path and extension, useful for getting a string to make hashes with
string GetStrippedFilename(string str)
{
	string temp = CL_String::get_filename(str);
	string::size_type pos = temp.find_last_of('.');
	if (pos != temp.npos) 
	{
		temp = temp.substr(0, pos);
	}
	return CL_String::to_lower(temp);
}


unsigned int FileNameToID(const char * filename)
{
	unsigned int resourceID = 0;
	string str = GetStrippedFilename(filename);
	resourceID = HashString(str.c_str());
	return resourceID;
}

CL_Vector2 MakeNormal(CL_Vector2 &a, CL_Vector2 &b)
{
	CL_Vector2 vNorm;
	CL_Vector2 vTemp = b-a;
	vTemp.unitize();
 
    vNorm.x = -vTemp.y;
	vNorm.y = vTemp.x;
	return vNorm;
}
void BlitMessage(string msg)
{
	CL_GlyphBuffer gb;
	CL_TextStyler ts;
	ts.add_font("default", *GetApp()->GetFont(C_FONT_NORMAL));
	ts.draw_to_gb(msg, gb);
	gb.set_alignment(origin_center);
	gb.set_scale(1.5,1.5);
	gb.draw(GetScreenX/2,GetScreenY/2);
	CL_Display::flip(2); //show it now
}


string ColorToString(const CL_Color &colr)
{
	return CL_String::from_int(colr.get_red()) +" "
		+CL_String::from_int(colr.get_green()) +" "
		+CL_String::from_int(colr.get_blue()) +" "
		+CL_String::from_int(colr.get_alpha());
}

string RectToString(const CL_Rect &r)
{
	return CL_String::from_int(r.top) +" "
		+CL_String::from_int(r.left) +" "
		+CL_String::from_int(r.bottom) +" "
		+CL_String::from_int(r.right);
}


CL_Color StringToColor(const string &stColor)
{
	//break it apart and recreate it as a color
#define C_CL_COLOR_COUNT 4

	cl_uint8 colors[C_CL_COLOR_COUNT];
	unsigned int i;
	for (i=0; i < C_CL_COLOR_COUNT; i++)
	{
		colors[i] = 0;
	}
	std::vector<string> stTok = CL_String::tokenize(stColor, " ", true);

	for (i=0; i < stTok.size(); i++ )
	{
		colors[i] = CL_String::to_int(stTok[i]);
	}

	return CL_Color(colors[0], colors[1], colors[2], colors[3]);
}


CL_Rect StringToRect(const string &stColor)
{
	//break it apart and recreate it as a color
	std::vector<string> stTok = CL_String::tokenize(stColor, " ", true);

	assert(stTok.size() == 4);

	CL_Rect r;
	r.top = CL_String::to_int(stTok[0]);
	r.left = CL_String::to_int(stTok[1]);
	r.bottom = CL_String::to_int(stTok[2]);
	r.right = CL_String::to_int(stTok[3]);
	return r;
}



void RenderVertexList(const CL_Vector2 &pos, CL_Vector2 *pVertArray, int vertCount, CL_Color &colr, CL_GraphicContext *pGC)
{
	CL_Vector2 a, b, first;
	
	assert(vertCount > 0);

	a = pos + pVertArray[0];
	a = GetWorldCache->WorldToScreen(a);
	first = a;

	for (int i=1; i < vertCount; i++)
	{
		b = pos + pVertArray[i];
		b = GetWorldCache->WorldToScreen(b);
		pGC->draw_line(a.x, a.y, b.x, b.y, colr);
		//get ready for next line
		a = b;
	}

	b = first;
	pGC->draw_line(a.x, a.y, b.x, b.y, colr);
}

void RenderVertexListRotated(const CL_Vector2 &pos, CL_Vector2 *pVertArray, int vertCount, CL_Color &colr, CL_GraphicContext *pGC,  float angleRad)
{
	CL_Vector2 a, b, first;

	assert(vertCount > 0);
    CL_Vector2 v = GetWorldCache->WorldToScreen(pos);
	CL_Pointf rotPt(v.x, v.y);
	a = pos + pVertArray[0];
	a = GetWorldCache->WorldToScreen(a);
	first = a;

	CL_Pointf ra, rb;
	for (int i=1; i < vertCount; i++)
	{
		b = pos + pVertArray[i];
		b = GetWorldCache->WorldToScreen(b);
	
		//create rotated version
		ra = *(CL_Pointf*)&a;
		rb = *(CL_Pointf*)&b;

		ra = ra.rotate(rotPt, -angleRad);
		rb = rb.rotate(rotPt, -angleRad);
		

		pGC->draw_line(ra.x, ra.y, rb.x, rb.y, colr);
		//get ready for next line
		a = b;
	}

	b = first;
	rb = *(CL_Pointf*)&b;
	rb = rb.rotate(rotPt, -angleRad);
	ra = *(CL_Pointf*)&a;
	ra = ra.rotate(rotPt, -angleRad);

	pGC->draw_line(ra.x, ra.y, rb.x, rb.y, colr);
}


void DrawRectFromWorldCoordinates(CL_Vector2 vecStart, CL_Vector2 vecStop, CL_Color borderColor, CL_GraphicContext *pGC)
{
	vecStart = GetWorldCache->WorldToScreen(vecStart);
	vecStop = GetWorldCache->WorldToScreen(vecStop);

	static CL_Rectf rec;
	rec.left = vecStart.x;
	rec.top = vecStart.y;
	rec.right = vecStop.x;
	rec.bottom = vecStop.y;
	pGC->draw_rect(rec, borderColor);
}

void DrawRectFromWorldCoordinatesRotated(CL_Vector2 vecStart, CL_Vector2 vecStop, CL_Color borderColor, CL_GraphicContext *pGC, float angleRad)
{
	vecStart = GetWorldCache->WorldToScreen(vecStart);
	vecStop = GetWorldCache->WorldToScreen(vecStop);

	static CL_Rectf rec;
	rec.left = vecStart.x;
	rec.top = vecStart.y;
	rec.right = vecStop.x;
	rec.bottom = vecStop.y;

	CL_Pointf rspot(rec.get_width(), rec.get_height()/2); //hotspot to rotate around
	CL_Pointf a,b;
	a = CL_Pointf(rec.left, rec.top);
	b = CL_Pointf(rec.right, rec.top);

	//rotate them
	a = a.rotate(rspot, angleRad);
	b = b.rotate(rspot, angleRad);

	pGC->draw_line(a.x, a.y, b.x, b.y, borderColor);
	//pGC->draw_rect(rec, borderColor);
}


//get it drawing normally
void ResetFont(CL_Font *pFont)
{
	pFont->set_alpha(1);
	pFont->set_color(CL_Color(255,255,255,255));
	pFont->set_alignment(origin_top_left);
}

//given a start and end point it will tell you the closest tile and where the collision point is.

bool GetTileLineIntersection(const CL_Vector2 &vStart, const CL_Vector2 &vEnd, tile_list &tList, CL_Vector2 *pvColPos, Tile* &pTileOut, const Tile * const pTileToIgnore /* = NULL*/)
{
	float lineA[4], lineB[4];
	lineA[0] = vStart.x;
	lineA[1] = vStart.y;

	lineA[2] = vEnd.x;
	lineA[3] = vEnd.y;

	CL_Pointf ptCol;
	float closestDistance = FLT_MAX;
	float distance;

	CL_Vector2 vTilePos;
	if (tList.empty()) return false;

	tile_list::iterator listItor = tList.begin();

	for (;listItor != tList.end(); listItor++)
	{

		CollisionData *pCol = (*listItor)->GetCollisionData();
		if (!pCol || pCol->GetLineList()->empty()) continue;
		if ((*listItor) == pTileToIgnore) continue; //don't process ourself, how much sense would that make?

		//LogMsg("Checking tile");

		vTilePos = (*listItor)->GetPos();

		line_list::iterator lineListItor = pCol->GetLineList()->begin();

		while (lineListItor != pCol->GetLineList()->end())
		{
			if (lineListItor->GetPointList()->size() > 0 && g_materialManager.GetMaterial(lineListItor->GetType())->GetType()
				== CMaterial::C_MATERIAL_TYPE_NORMAL)
			{
				//examine each line list for suitability
				//LogMsg("Checking a list with %d points", lineListItor->GetPointList()->size());

				//LogMsg("Offset is %.2f, %.2f", lineListItor->GetOffset().x,lineListItor->GetOffset().y );
				for (unsigned int i=0; i < lineListItor->GetPointList()->size();)
				{
					lineB[0] = vTilePos.x + lineListItor->GetPointList()->at(i).x + lineListItor->GetOffset().x;
					lineB[1] = vTilePos.y + lineListItor->GetPointList()->at(i).y+ lineListItor->GetOffset().y;

					int endLineIndex;

					if (i == lineListItor->GetPointList()->size()-1)
					{
						endLineIndex=0;
					} else
					{
						endLineIndex = i+1;
					}

					lineB[2] = vTilePos.x + lineListItor->GetPointList()->at(endLineIndex).x+ lineListItor->GetOffset().x;
					lineB[3] = vTilePos.y + lineListItor->GetPointList()->at(endLineIndex).y+ lineListItor->GetOffset().y;

					//LogMsg("Line B is %.2f, %.2f - %.2f, %.2f", lineB[0],lineB[1],lineB[2],lineB[3] );
					if (CL_LineMath::intersects(lineA, lineB))
					{
						//LogMsg("Got intersection");
						ptCol = CL_LineMath::get_intersection(lineA, lineB);
						distance = ptCol.distance(*(CL_Pointf*)&vStart);
						if (distance < closestDistance)
						{
							pTileOut = (*listItor);
							closestDistance = distance;
							*pvColPos = *(CL_Vector2*)&ptCol;
						}
					}

					i++;
				}
			}

			lineListItor++;
		}

	}

	return closestDistance != FLT_MAX; 
}


