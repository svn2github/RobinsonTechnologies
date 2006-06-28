
/* -------------------------------------------------
* Copyright 2006 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 25:2:2006   15:07
*/


#ifndef BrainPlayer_HEADER_INCLUDED // include guard
#define BrainPlayer_HEADER_INCLUDED  // include guard

#include "Brain.h"
#include "MovingEntity.h"

enum
{
	C_KEY_UP  = 0x0001,
	C_KEY_DOWN = 0x0002,
	C_KEY_LEFT  =  0x0004,
	C_KEY_RIGHT =  0x0008,
	C_KEY_SELECT =  0x0010,
	C_KEY_ATTACK =  0x0020,
	C_KEY_STRAFE_LEFT =  0x0040,
	C_KEY_STRAFE_RIGHT =  0x0080,
	C_KEY_PREV_WEAPON =  0x0100,
	C_KEY_NEXT_WEAPON =  0x0200

};

class BrainPlayer: public Brain
{
public:

    BrainPlayer(MovingEntity * pParent);
    virtual ~BrainPlayer();
	
	virtual void Update(float step);
	virtual const char * GetName(){return "SidePlayer";};
	virtual Brain * CreateInstance(MovingEntity *pParent) {return new BrainPlayer(pParent);}
	virtual string HandleMsg(const string &msg);

protected:
	
	void SetFreeze(bool freeze) {m_bFrozen = freeze;}
	bool GetFreeze() {return m_bFrozen;}
	void ResetKeys();
	void OnKeyUp(const CL_InputEvent &key);
	void OnKeyDown(const CL_InputEvent &key);
	void OnMouseUp(const CL_InputEvent &key);
	void UpdateMovement(float step);
	void AttemptToJump();
	void PostUpdate(float step);
	void CheckForLadder();
	void OnAction();
	void CheckForDoor();
	void AssignPlayerToCameraIfNeeded();
	void CheckForMovement();
	void CalculateForce(CL_Vector2 &force, float step);
	void CheckForAttack();

	unsigned int m_Keys; //holds current state of player movement keys
	CL_Slot m_SlotKeyUp;
	CL_Slot m_SlotKeyDown;

	unsigned int m_jumpTimer;
	unsigned int m_jumpBurstTimer;

	float m_curJumpForce; //the start of the jump has stronger force then the end

	bool m_bAppliedSecondJumpForce;
	CL_Vector2 m_moveAngle;
	bool m_bRequestJump;
	LoopingSound m_climbSound;
	LoopingSound m_walkSound;
	string m_jumpSound;
	bool m_bFrozen;

	GameTimer m_attackTimer;
	

};

#endif                  // include guard
