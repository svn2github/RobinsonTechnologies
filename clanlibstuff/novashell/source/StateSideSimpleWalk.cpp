#include "AppPrecomp.h"
#include "StateSideSimpleWalk.h"

StateSideSimpleWalk registryInstanceSimpleWalk(NULL); //self register ourselves in the brain registry

StateSideSimpleWalk::StateSideSimpleWalk(MovingEntity * pParent):State(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}
	
}

void StateSideSimpleWalk::OnAdd()
{
	//set some defaults
	m_weight = 0.6f;

	if (m_pParent->GetVisualProfile()->IsActive(VisualProfile::WALK_LEFT))
	{
		m_pParent->SetVisualState(VisualProfile::VISUAL_STATE_WALK);
	} else
	{
		m_pParent->SetVisualState(VisualProfile::VISUAL_STATE_RUN);
	}
	
	m_force = FacingToVector(m_pParent->GetFacing());
	ResetSlowThinkTimer();
	m_thinkTimer.SetInterval(400);
}

void StateSideSimpleWalk::ResetSlowThinkTimer()
{
	m_slowThinkTimer.SetInterval(random_range(4*1000, 5*1000));
}

StateSideSimpleWalk::~StateSideSimpleWalk()
{
}

void StateSideSimpleWalk::TurnAround()
{
	if (m_pParent->GetFacing() == VisualProfile::FACING_LEFT)
	{
		m_pParent->SetFacing(VisualProfile::FACING_RIGHT);
	} else
	{
		m_pParent->SetFacing(VisualProfile::FACING_LEFT);
	}

	m_force = FacingToVector(m_pParent->GetFacing());
}

void StateSideSimpleWalk::LookAround()
{
	//first check straight ahead of us
	bool bNeedToTurn = false;

	CL_Vector2 vFacing = FacingToVector(m_pParent->GetFacing());
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

	if (GetTileLineIntersection(vStartPos, vEndPos, m_pParent->GetNearbyTileList(), &vColPos, pTile, m_pParent->GetTile()))
	{
		//LogMsg("Found tile at %.2f, %.2f", vColPos.x, vColPos.y);
		switch ((pTile)->GetType())
		{
		case C_TILE_TYPE_PIC:
			//wall in front of us it appears
			bNeedToTurn = true;	
			break;
		case C_TILE_TYPE_ENTITY:
			if (GetPlayer != ((TileEntity*)pTile)->GetEntity())
			{
				//wall in front of us it appears
				bNeedToTurn = true;	
			}
			break;
		}
	} 

	if (!bNeedToTurn)
	{
		//do another check for missing ground ahead of us
		vStartPos.x = vEndPos.x;
		vEndPos.y += colRect.get_height();
		bNeedToTurn = true;	

		if (GetTileLineIntersection(vStartPos, vEndPos, m_pParent->GetNearbyTileList(), &vColPos, pTile, m_pParent->GetTile()))
		{
			//if a floor ahead is NOT found, let's turn around.
			//LogMsg("Found tile at %.2f, %.2f", vColPos.x, vColPos.y);
			switch ((pTile)->GetType())
			{

			case C_TILE_TYPE_ENTITY:
			case C_TILE_TYPE_PIC:
				//looks there is a floor up there
				bNeedToTurn = false;	
				break;
			}
		} 
	}

	if (bNeedToTurn)
	{
		TurnAround();
	}
}

void StateSideSimpleWalk::Update(float step)
{
	m_pParent->GetBrainManager()->GetBrainBase()->AddWeightedForce(m_force * m_weight);
}

void StateSideSimpleWalk::PostUpdate(float step)
{
	//turn around if we hit something or we're about to fall
	//if we played the anim all the way through, let's "think" a bit
	if (AnimIsLooping())
	{

		//we do a further check here to limit CPU cycles, especially useful for 1 frame anims
		if (m_thinkTimer.IntervalReached())
		{
			if (m_pParent->IsOnGround())
			{
				LookAround();
			}
		}

		if (m_slowThinkTimer.IntervalReached())
		{
			ResetSlowThinkTimer();
			m_pParent->GetBrainManager()->SetStateByName("SideSimpleIdle");
		}
	}

}
