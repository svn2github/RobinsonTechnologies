#include "AppPreComp.h"
#include "CollisionData.h"

CollisionData::CollisionData()
{
	m_dataChanged = false;
}
CollisionData::~CollisionData()
{
	SaveIfNeeded();
}

void CollisionData::Load(const string &fileName)
{

	m_dataChanged = false;
	m_fileName = fileName;

	CL_InputSource_File *pFile = NULL;

	try
	{
		pFile = new CL_InputSource_File(fileName);
	} catch(CL_Error error)
	{
		SAFE_DELETE(pFile);
		return;
	}

	CL_FileHelper helper(pFile); //will autodetect if we're loading or saving

    Serialize(helper);
	SAFE_DELETE(pFile);

}

void CollisionData::SaveIfNeeded()
{
	if (m_fileName.empty() || !m_dataChanged) return;
	
	CL_OutputSource_File file(m_fileName);
	CL_FileHelper helper(&file); 

	Serialize(helper);

	m_dataChanged = false;
}


void CollisionData::RecalculateOffsets()
{
	line_list::iterator listItor = m_lineList.begin();

	while (listItor != m_lineList.end())
	{
		listItor->CalculateOffsets();
		listItor++;
	}
}
void CollisionData::RemoveOffsets()
{
	line_list::iterator listItor = m_lineList.begin();

	while (listItor != m_lineList.end())
	{
		listItor->RemoveOffsets();
		listItor++;
	}
}

bool CollisionData::HasData()
{
	line_list::iterator listItor = m_lineList.begin();

	while (listItor != m_lineList.end())
	{
		if (!listItor->GetPointList()->empty()) return true;
		listItor++;
	}

	return false;
}

void CollisionData::Serialize(CL_FileHelper &helper)
{
  //save out our rect and points
	helper.process(m_rect);


	if (helper.IsWriting())
	{
		//write how many linelists are coming
		helper.process_const(cl_uint32(m_lineList.size()));

		line_list::iterator listItor = m_lineList.begin();

		while (listItor != m_lineList.end())
		{
			//write data about this PointList
			helper.process_const(listItor->GetType());
			
			//write how many points are in this list
			helper.process_const(cl_uint32(listItor->GetPointList()->size()));
			helper.process(listItor->GetOffset());
			//write 'em
			helper.process_array(&listItor->GetPointList()->at(0), listItor->GetPointList()->size());
		
			listItor++;
		}
	} else
	{
		
		//we're reading
		//first read how many lists we've got
		unsigned int listCount;
		helper.process(listCount);
		unsigned int vecCount;
		int pointListType;

		CL_Vector2 a,b;

		for (unsigned int i=0; i < listCount; i++)
		{
			PointList ptList;
			helper.process(pointListType);
			ptList.SetType(pointListType);
			helper.process(vecCount); //how many points this list has
			if (vecCount != 0)
			{

				CL_Vector2 vTemp;
				helper.process(vTemp);
				m_lineList.push_back(ptList);
				m_lineList.rbegin()->SetOffset(vTemp);

				m_lineList.rbegin()->GetPointList()->resize(vecCount);
				helper.process_array(&m_lineList.rbegin()->GetPointList()->at(0), vecCount);
			}
		}
		
	}
}
 
void CollisionData::Clear()
{
	m_lineList.clear();
}


void CreateCollisionDataWithTileProperties(Tile *pTile, CollisionData &colOut)
{
	colOut = *pTile->GetCollisionData();

	line_list *pLineList = colOut.GetLineList();
	line_list::iterator itor = pLineList->begin();
//	int needsFlip;
	CL_Vector2 vecTemp;
	
	/*
	while (itor != pLineList->end())
	{
		//modify each vert on each line

		for (unsigned int i=0; i < itor->GetPointList()->size(); i++)
		{
			needsFlip = 0;
			pWall = &itor->GetPointList()->at(i);
			//do each vert
			if (pTile->GetBit(Tile::e_flippedX))
			{
				pWall->m_vA.x = pTile->GetBoundsSize().x - pWall->m_vA.x;
				pWall->m_vB.x = pTile->GetBoundsSize().x - pWall->m_vB.x;
				needsFlip++;
			}

			if (pTile->GetBit(Tile::e_flippedY))
			{
				pWall->m_vA.y = pTile->GetBoundsSize().y - pWall->m_vA.y;
				pWall->m_vB.y = pTile->GetBoundsSize().y - pWall->m_vB.y;
				needsFlip++;
			}

			if (needsFlip == 1)
			{
				//flip the normals
				vecTemp = pWall->m_vA;
				pWall->m_vA = pWall->m_vB;
				pWall->m_vB = vecTemp;

			} 

			pWall->CalculateNormal();
		}

		itor++;
	}
	*/

}