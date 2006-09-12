//  ***************************************************************
//  EntVisualProfileEditor - Creation date: 08/21/2006
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2006 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef EntVisualProfileEditor_h__
#define EntVisualProfileEditor_h__

#include "CollisionData.h"
#include "BaseGameEntity.h"

class MovingEntity;

class EntVisualProfileEditor: public BaseGameEntity
{
public:
	EntVisualProfileEditor();
	virtual ~EntVisualProfileEditor();

	bool Init(MovingEntity *pEnt);
	void OnEditorClosed(int entID);

protected:
	
	void OnButtonUp(const CL_InputEvent &key);
	void OnButtonDown(const CL_InputEvent &key);
	void OnMouseMove(const CL_InputEvent &key);

	void OnClose();
	
	void ModifyActiveAnim(CL_Point pt);

	void OnChangeAnim();
	
	//GUI
	CL_SlotContainer m_slots;
	CL_Window * m_pWindow;
	CL_ListBox * m_pListAnims;

	MovingEntity *m_pEnt; //the object we're working with

	bool m_bShowColDataSave;

private:
};

#endif // EntVisualProfileEditor_h__
