#include "PlatformPrecomp.h"
#include "CursorComponent.h"
#include "util/GLESUtils.h"
#include "Entity/EntityUtils.h"
#include "BaseApp.h"

CursorComponent::CursorComponent()
{
	SetName("Cursor");
}

CursorComponent::~CursorComponent()
{
}

void CursorComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);
	m_pArrowEnt = NULL;
	m_bDisable = false;
	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();


	//register ourselves to render if the parent does
	GetParent()->GetFunction("OnRender")->sig_function.connect(1, boost::bind(&CursorComponent::OnRender, this, _1));
	GetParent()->GetFunction("OnUpdate")->sig_function.connect(1, boost::bind(&CursorComponent::OnUpdate, this, _1));
	AddInputMovementFocusIfNeeded(GetParent());
	GetParent()->GetFunction("OnInput")->sig_function.connect(1, boost::bind(&CursorComponent::OnInput, this, _1));
	GetParent()->GetParent()->GetFunction("OnKillingControls")->sig_function.connect(1, boost::bind(&CursorComponent::OnKillingControls, this, _1));	

}

void CursorComponent::OnRemove()
{
	EntityComponent::OnRemove();
}
void CursorComponent::OnKillingControls(VariantList *pVList)
{
	RemoveFocusIfNeeded(this->GetParent());
	m_bDisable = true;
}

void CursorComponent::OnUpdatePos(CL_Vec2f vPos)
{
	//LogMsg("Got %s", PrintVector2(vPos).c_str());
	DinkSetCursorPosition(NativeToDinkCoords(vPos));
}

void CursorComponent::OnRender(VariantList *pVList)
{
	//CL_Vec2f vFinalPos = pVList->m_variant[0].GetVector2()+*m_pPos2d;
}

void CursorComponent::OnUpdate(VariantList *pVList)
{
}


void CursorComponent::OnInput( VariantList *pVList )
{
	//0 = message type, 1 = parent coordinate offset
	CL_Vec2f pt = pVList->Get(1).GetVector2();
	//pt += GetAlignmentOffset(*m_pSize2d, eAlignment(*m_pAlignment));

	switch (eMessageType( int(pVList->Get(0).GetFloat())))
	{
	case MESSAGE_TYPE_GUI_CLICK_START:
		//HandleClickStart(pt);
		OnUpdatePos(pt);
		break;
	case MESSAGE_TYPE_GUI_CLICK_END:
		if (!m_bDisable)
		{
			OnUpdatePos(pt);
			g_dglo.m_dirInput[DINK_INPUT_BUTTON1] = true;
			g_dglo.m_dirInputFinished[DINK_INPUT_BUTTON1] = true;
		}
		
		//HandleClickEnd(pt);
		break;
	case MESSAGE_TYPE_GUI_CLICK_MOVE:
	case MESSAGE_TYPE_GUI_CLICK_MOVE_RAW:
		OnUpdatePos(pt);
		break;
	}	

}
