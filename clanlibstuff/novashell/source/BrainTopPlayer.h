//  ***************************************************************
//  BrainTopPlayer - Creation date: 07/12/2006
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2006 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//this is a brain file.  By including it in the project,
//it will automatically register itself and be available
//from lua script as a brain behavior.

#ifndef BrainTopPlayer_h__
#define BrainTopPlayer_h__

#include "Brain.h"
#include "BrainPlayer.h" //to avoid duplication we reuse some of his stuff

class BrainTopPlayer: public Brain
{
public:
	BrainTopPlayer(MovingEntity *pParent);
	virtual ~BrainTopPlayer();
	virtual void Update(float step);
	virtual void PostUpdate(float step);;
	virtual const char * GetName() {return "TopPlayer";};
	virtual Brain * CreateInstance(MovingEntity *pParent) {return new BrainTopPlayer(pParent);}
	virtual	void OnAdd();

	//for use by other brains directly
	virtual void AddWeightedForce(const CL_Vector2 & force);
	virtual unsigned int GetKeys() {return m_Keys;}
	virtual void HandleMsg(const string &msg);


protected:

private:

	void ResetKeys();
	void OnKeyUp(const CL_InputEvent &key);
	void OnKeyDown(const CL_InputEvent &key);
	bool CheckForMovement();
	void CalculateForce(float step);
	void UpdateMovement(float step);
	void CheckForWarp();
	void CheckForAttack();
	void ResetForNextFrame();
	void OnAction();

	unsigned int m_Keys; //holds current state of player movement keys
	CL_Slot m_SlotKeyUp;
	CL_Slot m_SlotKeyDown;
	LoopingSound m_walkSound;
	
	//CL_Vector2 m_moveAngle;

	GameTimer m_attackTimer;

	CL_Vector2 m_force;
	float m_maxForce;


};

#endif // BrainTopPlayer_h__
