#include "AppPrecomp.h"
#include "BrainColorFlash.h"
#include "MovingEntity.h"

BrainColorFlash registryInstance(NULL); //self register ourselves i nthe brain registry

BrainColorFlash::BrainColorFlash(MovingEntity * pParent):Brain(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}

	SetPulseRate(0);
	m_bOn = true;

	m_r = 0;
	m_g = 0;
	m_b = 0;
	m_a = 0;

	m_remove_brain_by_pulses = 0; //disable this
}

void BrainColorFlash::SetPulseRate(int pulseRate)
{
	m_pulseRate = pulseRate;
	m_flashTimer.SetInterval(m_pulseRate);
}

void BrainColorFlash::HandleMsg(const string &msg)
{
	vector<string> messages = CL_String::tokenize(msg, ";",true);

	for (unsigned int i=0; i < messages.size(); i++)
	{
		vector<string> words = CL_String::tokenize(messages[i], "=",true);

		if (words[0] == "r")
		{
			m_r = CL_String::to_int(words[1]);
		} else
			if (words[0] == "g")
			{
				m_g = CL_String::to_int(words[1]);
			} else
				if (words[0] == "b")
				{
					m_b = CL_String::to_int(words[1]);
				} else
					if (words[0] == "a")
					{
						m_a = CL_String::to_int(words[1]);
					} else
			if (words[0] == "pulse_rate")
			{
				SetPulseRate(CL_String::to_int(words[1]));
			} else
				if (words[0] == "remove_brain_by_pulses")
				{
					m_remove_brain_by_pulses = CL_String::to_int(words[1]);
				} else
				{
				LogMsg("Brain %s doesn't understand keyword %s", GetName(), words[0].c_str());
			}
	}

}

BrainColorFlash::~BrainColorFlash()
{
}

void BrainColorFlash::Update(float step)
{
	if (m_bOn)
	{
		m_pParent->AddColorModRed(m_r);
		m_pParent->AddColorModGreen(m_g);
		m_pParent->AddColorModBlue(m_b);
		m_pParent->AddColorModAlpha(m_a);
	}

	if (m_pulseRate != 0)
	if (m_flashTimer.IntervalReached())
	{
		m_bOn = !m_bOn;
		if (m_remove_brain_by_pulses != 0)
		{
			m_remove_brain_by_pulses--;
			
			if (m_remove_brain_by_pulses == 0)
			{
				//remove ourself
				SetDeleteFlag(true);
			}
		}
	}

}