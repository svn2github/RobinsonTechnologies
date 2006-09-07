#include "AppPrecomp.h"
#include "GameLogic.h"
#include "CollisionData.h"
#include "MaterialManager.h"


CL_Vector2 Vector2Perp(const CL_Vector2 &v)
{
	return CL_Vector2(-v.y, v.x);
}

void DrawLineWithArrow(CL_Vector2 from, CL_Vector2 to, double size, CL_Color &col, CL_GraphicContext* pGC)
{

	CL_Vector2 norm = to-from;
	norm.unitize();

	//calculate where the arrow is attached
	CL_Vector2 CrossingPoint = to - (norm * size);

	//calculate the two extra points required to make the arrowhead
	CL_Vector2 ArrowPoint1 = CrossingPoint + (Vector2Perp(norm) * 0.4f * size); 
	CL_Vector2 ArrowPoint2 = CrossingPoint - (Vector2Perp(norm) * 0.4f * size); 

	//draw the line
	pGC->draw_line(from.x, from.y, CrossingPoint.x, CrossingPoint.y, col);
	pGC->fill_triangle(ArrowPoint1.x, ArrowPoint1.y, ArrowPoint2.x, ArrowPoint2.y, to.x, to.y,
		CL_Gradient(col,col,col,col));
}

void DrawLineWithArrowWorld(CL_Vector2 from, CL_Vector2 to, double size, CL_Color &col, CL_GraphicContext* pGC)
{
	DrawLineWithArrow(GetWorldCache->WorldToScreen(from), GetWorldCache->WorldToScreen(to), size, col, pGC);
}


double GetAngleBetweenVectorFacings(CL_Vector2 v1, CL_Vector2 v2)
{
	double dot = v1.dot(v2);

	//clamp to rectify any rounding errors
	Clamp(dot, -1, 1);

	//determine the angle between the heading vector and the target
	return acos(dot);
}


bool ConfirmMessage(string title, string msg)
{
	CL_MessageBox message(title, msg, "Continue", "Abort", "", GetApp()->GetGUI());
	message.set_event_passing(false);
	message.run();
	return message.get_result_button() == 0;
}

void GameTimer::Reset()
{
	m_timer = GetApp()->GetGameTick() + m_interval;
}

void GameTimer::SetIntervalReached()
{
	m_timer = 1; //make sure the next tick will cause it to go. I used 1 in case I want to use 0 to mean
	//a disabled timer later
}

bool GameTimer::IntervalReached()
{
	if (m_timer < GetApp()->GetGameTick())
	{
		Reset();
		return true;
	}

	return false;
}

//snippet from Zahlman's post on gamedev:  http://www.gamedev.net/community/forums/topic.asp?topic_id=372125
void StringReplace(const std::string& what, const std::string& with, std::string& in)
{
	int pos = 0;
	int whatLen = what.length();
	int withLen = with.length();
	while ((pos = in.find(what, pos)) != std::string::npos)
	{
		in.replace(pos, whatLen, with);
		pos += withLen;
	}
}

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

	CL_Font *pFont = GetApp()->GetFont(C_FONT_NORMAL);

	ResetFont(pFont);
	CL_GlyphBuffer gb;
	CL_TextStyler ts;
	ts.add_font("default", *pFont);
	ts.draw_to_gb(msg, gb);
	gb.set_alignment(origin_center);
	gb.set_scale(1.5,1.5);
	gb.draw(GetScreenX/2,GetScreenY/2);
	CL_Display::flip(2); //show it now
}



string VectorToStringEx(const CL_Vector2 * pVec)
{
	char stTemp[256];
	sprintf(stTemp, "X:%.2f Y: %.2f", pVec->x, pVec->y);
	return string(stTemp);
}

string VectorToString(const CL_Vector2 * pVec)
{
	return CL_String::from_float(pVec->x) + " " + CL_String::from_float(pVec->y);
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

CL_Vector2 StringToVector(const string &stVec)
{
	//break it apart and recreate it as a vector
	CL_Vector2 v = CL_Vector2::ZERO;

	std::vector<string> stTok = CL_String::tokenize(stVec, " ", true);

	if (stTok.size() < 2)
	{
		LogError("StringToVector: Malformed string, can't convert it");
		return v;
	}

	v.x = CL_String::to_float(stTok[0]);
	v.y = CL_String::to_float(stTok[1]);

	return v;
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



void DrawCenteredBox(const CL_Vector2 &a, int size, CL_Color col, CL_GraphicContext *pGC)
{
	CL_Rect rec;
	rec.left = a.x - size;
	rec.top = a.y - size;

	rec.right = a.x + size;
	rec.bottom = a.y + size;
	pGC->draw_rect(rec, col);

}

void DrawCenteredBoxWorld(const CL_Vector2 &a, int size, CL_Color col, CL_GraphicContext *pGC)
{
	DrawCenteredBox( GetWorldCache->WorldToScreen(a), size, col, pGC);
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

bool GetTileLineIntersection(const CL_Vector2 &vStart, const CL_Vector2 &vEnd, tile_list &tList, CL_Vector2 *pvColPos, Tile* &pTileOut, const Tile * const pTileToIgnore /* = NULL*/, int limitSearchToThisTileType, bool bIgnoreMovingCreatures)
{
	float lineA[4], lineB[4];
	lineA[0] = vStart.x;
	lineA[1] = vStart.y;

	lineA[2] = vEnd.x;
	lineA[3] = vEnd.y;

	CL_Pointf ptCol;
	float closestDistance = FLT_MAX;
	float distance;

	static CollisionData col;
	pTileOut = NULL;
	CL_Vector2 vTilePos;
	int tileType;
	if (tList.empty()) return false;

	tile_list::iterator listItor = tList.begin();

	for (;listItor != tList.end(); listItor++)
	{
		tileType = (*listItor)->GetType();
		
		if (limitSearchToThisTileType != C_TILE_TYPE_BLANK)
		{
			if ( tileType != limitSearchToThisTileType)
			{
				//LogMsg("Skipping");
				continue; //skip this one
			}
		}
		CollisionData *pCol = (*listItor)->GetCollisionData();
		if (!pCol || pCol->GetLineList()->empty()) continue;
		if ((*listItor) == pTileToIgnore) continue; //don't process ourself, how much sense would that make?

		
		if (bIgnoreMovingCreatures && tileType == C_TILE_TYPE_ENTITY)
		{
			//the pathfinding may be building nodes, we don't want it to build an incorrect base path because
			//the player or monster was sitting in the way.  Somehow we have to figure out a way to ignore
			//those kinds of things.  For now, assume if it has a visual profile assigned it should be ignored

			if (  ((TileEntity*)(*listItor))->GetEntity()->GetVisualProfile())
			{
				continue;
			}
		}

		//LogMsg("Checking tile");

		vTilePos = (*listItor)->GetPos();

		if ((*listItor)->UsesTileProperties() && (*listItor)->GetType() == C_TILE_TYPE_PIC )
		{
			//we need a customized version
			CreateCollisionDataWithTileProperties((*listItor), col);
			pCol = &col;
		}


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
					
					if ((*listItor)->GetType() == C_TILE_TYPE_PIC)
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

					} else
					{

						//assume it's an entity
						assert((*listItor)->GetType() == C_TILE_TYPE_ENTITY);

						


						lineB[0] = vTilePos.x + lineListItor->GetPointList()->at(i).x;// + lineListItor->GetOffset().x;
						lineB[1] = vTilePos.y + lineListItor->GetPointList()->at(i).y;//+ lineListItor->GetOffset().y;

						int endLineIndex;

						if (i == lineListItor->GetPointList()->size()-1)
						{
							endLineIndex=0;
						} else
						{
							endLineIndex = i+1;
						}

						lineB[2] = vTilePos.x + lineListItor->GetPointList()->at(endLineIndex).x;//+ lineListItor->GetOffset().x;
						lineB[3] = vTilePos.y + lineListItor->GetPointList()->at(endLineIndex).y;//+ lineListItor->GetOffset().y;

					}

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



//------------------------ Sign ------------------------------------------
//
//  returns positive if v2 is clockwise of this vector,
//  minus if anticlockwise (Y axis pointing down, X axis to right)
//------------------------------------------------------------------------
enum {clockwise = 1, anticlockwise = -1};

int VectorSign(const CL_Vector2 &v1, const CL_Vector2 &v2)
{
	if (v1.y*v2.x > v1.x*v2.y)
	{ 
		return anticlockwise;
	}
	else 
	{
		return clockwise;
	}
}

bool RotateVectorTowardsAnotherVector(CL_Vector2 &vecFacing, CL_Vector2 vecFacingTarget, float maxTurn)
{
	if (vecFacingTarget == vecFacing) return false; //no change needed

	double dot = vecFacing.dot(vecFacingTarget);

	//clamp to rectify any rounding errors
	Clamp(dot, -1, 1);

	//determine the angle between the heading vector and the target
	double angle = acos(dot);

	const double WeaponAimTolerance = 0.01; //2 degs approx

	if (angle < WeaponAimTolerance)
	{
		vecFacing = vecFacingTarget;
		return true;
	}

	//clamp the amount to turn to the max turn rate
	if (angle > maxTurn) angle = maxTurn;

	//The next few lines use a rotation matrix to rotate the player's facing
	//vector accordingly
	C2DMatrix RotationMatrix;

	//notice how the direction of rotation has to be determined when creating
	//the rotation matrix
	RotationMatrix.Rotate(angle * VectorSign(vecFacing,vecFacingTarget));	
	RotationMatrix.TransformVector(vecFacing);

	return true;
}
