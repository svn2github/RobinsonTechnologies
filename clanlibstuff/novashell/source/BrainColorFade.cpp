#include "AppPrecomp.h"
#include "BrainColorFade.h"
#include "MovingEntity.h"


BrainColorFade::ColorData::ColorData()
{
	Reset();
}

void BrainColorFade::ColorData::Reset()
{
	m_bActive = false;
}

void BrainColorFade::ColorData::SetColorTarget(short target, int startingColor)
{
	m_curColor = startingColor;
	m_colorTarget = target;
	m_bActive = true;
}

BrainColorFade registryInstanceBrainColorFade(NULL); //self register ourselves i nthe brain registry

BrainColorFade::BrainColorFade(MovingEntity * pParent):Brain(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}

	m_fadeTimeMS = 1000;
	m_timeCreated = GetApp()->GetGameTick();
	m_bRemove_brain = false;

}

BrainColorFade::~BrainColorFade()
{
}

void BrainColorFade::HandleMsg(const string &msg)
{
	vector<string> messages = CL_String::tokenize(msg, ";",true);

	for (unsigned int i=0; i < messages.size(); i++)
	{
		vector<string> words = CL_String::tokenize(messages[i], "=",true);
	
		if (words[0] == "fade_speed_ms")
		{
			m_fadeTimeMS = CL_String::to_int(words[1]);
		} else
			if (words[0] == "remove_brain_when_done")
			{
				m_bRemove_brain = CL_String::to_bool(words[1]);
			} else

		if (words[0] == "r")
		{
			m_color[COLOR_R].SetColorTarget(CL_String::to_int(words[1]), m_pParent->GetBaseColor().get_red());
			
		} else
			if (words[0] == "g")
			{
				m_color[COLOR_G].SetColorTarget(CL_String::to_int(words[1]), m_pParent->GetBaseColor().get_green());
			} else
				if (words[0] == "b")
				{
					m_color[COLOR_B].SetColorTarget(CL_String::to_int(words[1]), m_pParent->GetBaseColor().get_blue());
				} else
					if (words[0] == "a")
					{
						m_color[COLOR_A].SetColorTarget(CL_String::to_int(words[1]), m_pParent->GetBaseColor().get_alpha());
					} else
							{
								LogMsg("Brain %s doesn't understand keyword %s", GetName(), words[0].c_str());
							}
	}

}

int BrainColorFade::ColorData::Update(float modSpeed)
{
	
	set_float_with_target(&m_curColor, m_colorTarget, modSpeed);
	if (m_curColor == m_colorTarget)
	{
		m_bActive = false;

	}
	return m_curColor;
}

void BrainColorFade::Update(float step)
{

	//make our changes

	CL_Color col = m_pParent->GetBaseColor();
	float fAmount = 255.0/ (m_fadeTimeMS/GetApp()->GetGameLogicSpeed());

	bool bStillGoing = false;

	if (m_color[COLOR_A].m_bActive)
	{
		col.set_alpha(m_color[COLOR_A].Update(fAmount));
		bStillGoing = true;
	}
	m_pParent->SetBaseColor(col);

	if (m_bRemove_brain)
	{
		if (!bStillGoing)
		{
			SetDeleteFlag(true);
		}
	}

}