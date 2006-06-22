#include "AppPrecomp.h"
#include "BrainAISideSimpleWalk.h"
#include "MovingEntity.h"
#include "BrainSideBase.h"
#include "VisualProfileManager.h"

BrainAISideSimpleWalk registryInstance(NULL); //self register ourselves in the brain registry

BrainAISideSimpleWalk::BrainAISideSimpleWalk(MovingEntity * pParent):Brain(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}

	m_thinkTimer = 0;
	m_pSideBase = NULL;
}

void BrainAISideSimpleWalk::OnAdd()
{
	m_pSideBase = (BrainSideBase*)m_pParent->GetBrainManager()->GetBrainByName("SideBase");

	if (!m_pSideBase)
	{
		LogError("SideSimpleWalk requires that brain SideBase be added first!");
		return;
	}

	//set some defaults
	m_weight = 0.9f;
	
	WalkLeft();
}

void BrainAISideSimpleWalk::WalkLeft()
{
	m_pSideBase->SetFacing(VisualProfile::FACING_LEFT);
	m_force = FacingToVector(m_pSideBase->GetFacing());
}

void BrainAISideSimpleWalk::WalkRight()
{
	m_pSideBase->SetFacing(VisualProfile::FACING_RIGHT);
	m_force = FacingToVector(m_pSideBase->GetFacing());
}

BrainAISideSimpleWalk::~BrainAISideSimpleWalk()
{

}

void BrainAISideSimpleWalk::LookAround()
{

  //first check straight ahead of us
	

	bool bNeedToTurn = false;

	CL_Vector2 vFacing = FacingToVector(m_pSideBase->GetFacing());
	CL_Vector2 vStartPos = m_pParent->GetPos();
	CL_Vector2 vEndPos;

	CL_Rectf colRect = m_pParent->GetCollisionData()->GetLineList()->begin()->GetRect();

#define C_WALL_SEE_PADDING 20

	const int actionRange = (colRect.get_width()/2) + C_WALL_SEE_PADDING;
	vEndPos = vStartPos + (vFacing * actionRange);

	//shoot it at an angle starting from our knees
	vStartPos.y += colRect.get_height()/3;
	
	//	LogMsg("Starting ground check from %.2f, %.2f, to %.2f, %.2f", 
	//		vStartPos.x, vStartPos.y, vEndPos.x, vEndPos.y);

	CL_Vector2 vColPos;
	Tile * pTile = NULL;

	if (GetTileLineIntersection(vStartPos, vEndPos, m_pParent->GetNearbyTileList(), &vColPos, pTile, m_pParent->GetTile() ))
	{
		//LogMsg("Found tile at %.2f, %.2f", vColPos.x, vColPos.y);
		switch ((pTile)->GetType())
		{

		case C_TILE_TYPE_ENTITY:
			LogMsg("Bumped ent");
			break;
		case C_TILE_TYPE_PIC:
			//wall in front of us it appears
			bNeedToTurn = true;	
			break;
		}
	} 

	/*
if (!bNeedToTurn)
{
	//do another check for missing ground ahead of us
	vStartPos.x = vEndPos.x;
	vEndPos.y += colRect.get_height();

	bNeedToTurn = true;	

	if (GetTileLineIntersection(vStartPos, vEndPos, m_pParent->GetNearbyTileList(), &vColPos, pTile, m_pParent->GetTile() ))
	{
		//if a floor ahead is NOT found, let's turn around.

		//LogMsg("Found tile at %.2f, %.2f", vColPos.x, vColPos.y);
		switch ((pTile)->GetType())
		{

		case C_TILE_TYPE_PIC:
			//wall in front of us it appears
			bNeedToTurn = false;	
			break;
		}
	} 
}
*/
	if (bNeedToTurn)
	{
		if (m_pSideBase->GetFacing() == VisualProfile::FACING_LEFT)
		{
			WalkRight();
		} else
		{
			WalkLeft();
		}
	}
	
}

void BrainAISideSimpleWalk::Update(float step)
{
	if (!m_pSideBase) return;
	
	m_pSideBase->AddWeightedForce(m_force * m_weight);
}

void BrainAISideSimpleWalk::PostUpdate(float step)
{

	//turn around if we hit something or we're about to fall

	if (m_thinkTimer < GetApp()->GetGameTick())
	{
		if (m_pParent->IsOnGround())
		{
			LookAround();
		}
		m_thinkTimer = GetApp()->GetGameTick()+400;
	}

}
