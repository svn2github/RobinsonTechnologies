#include "AppPrecomp.h"
#include "Trigger.h"
#include "MovingEntity.h"
#include "VisualProfile.h"

Trigger::Trigger()
{
	Reset();
}

Trigger::~Trigger()
{
}

void Trigger::Reset()
{
	//disable the trigger
	Set(0, TYPE_NONE,0,BEHAVIOR_NORMAL,0);
}

void Trigger::Set(MovingEntity *pParent, int triggerType, int typeVar, int triggerBehavior, int behaviorVar)
{
	m_pParent = pParent;

	m_triggerType = (eTriggerType)triggerType;

	if (m_triggerType == TYPE_NONE)
	{
		//it's been disabled.
		return;
	}

	m_behaviorType = (eBehaviorType)triggerBehavior;

	m_typeVar = typeVar;
	m_behaviorVar = behaviorVar;

	//do any extra init code

	switch(m_triggerType)
	{
	case TYPE_REGION_IMAGE:
		
		m_state = STATE_OUTSIDE; //default state

		switch (m_behaviorType)
		{
		case BEHAVIOR_PULSE:
			//set the timer
			m_pulseTimer.SetInterval(m_behaviorVar);
			m_pulseTimer.SetIntervalReached(); //make sure it goes ASAP
			break;
		}

		break;
	}

}

void Trigger::SetTriggerState(bool bInsideRegion)
{
	if (bInsideRegion)
	{		
			//currently inside it
			if (m_state == STATE_OUTSIDE)
			{
				m_state = STATE_INSIDE;
				try {luabind::call_function<luabind::object>(m_pParent->GetScriptObject()->GetState(), "OnTriggerEnter");
				} LUABIND_ENT_BRAIN_CATCH( "Error while calling function OnTriggerEnter");
			}

			if (m_behaviorType == BEHAVIOR_PULSE)
			{
				if (m_pulseTimer.IntervalReached() || m_behaviorVar == 0)
				{
					try {luabind::call_function<luabind::object>(m_pParent->GetScriptObject()->GetState(), "OnTriggerInside");
					} LUABIND_ENT_BRAIN_CATCH( "Error while calling function OnTriggerInside");
				}
			}
	} else
	{
		if (m_state == STATE_INSIDE)
		{
			//must have just exited
			m_state = STATE_OUTSIDE;
			m_pParent->RunFunction("OnTriggerExit");
		}
		//currently outside
	}

}


void Trigger::Update(float step)
{

	switch (m_triggerType)
	{
	case TYPE_NONE:
		break;

	case TYPE_REGION_IMAGE:
		
		if (GetPlayer)
		SetTriggerState(m_pParent->GetWorldRect().is_overlapped(GetPlayer->GetWorldRect()));
		break;

	}
}