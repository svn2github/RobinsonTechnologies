
/* -------------------------------------------------
* Copyright 2006 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 9:3:2006   12:58
*/


#ifndef Message_HEADER_INCLUDED // include guard
#define Message_HEADER_INCLUDED  // include guard

#include <ClanLib/signals.h>
#include "MovingEntity.h"

class Message
{
public:

    Message();
    virtual ~Message();

	enum TimingType
	{
		GAME_TIME,
		SYSTEM_TIME
	};

	unsigned int m_deliveryTime;
	string m_text;
	void SetTarget(unsigned int entID);
	void EntDeleted(int ID);
	void Deliver();
	void SetTimingType(TimingType type) {m_timingType = type;}
	TimingType GetTimingType(){return m_timingType;}



private:
	int m_targetID; //the entity we will deliver too
	CL_SlotContainer m_slots;
	BaseGameEntity *m_pEnt;
	TimingType m_timingType;
};

#endif                  // include guard
