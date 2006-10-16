//  ***************************************************************
//  EntWorldDialog - Creation date: 09/15/2006
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2006 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef EntWorldDialog_h__
#define EntWorldDialog_h__

#include "World.h"
#include "BaseGameEntity.h"


class ModInfoItem
{
public:

	string m_stDirName;
	string m_stDisplayName;
};


class EntWorldDialog: public BaseGameEntity
{
public:
	EntWorldDialog();
	virtual ~EntWorldDialog();
	virtual void HandleMessageString(const std::string &msg);


private:

	void OnButtonDown(const CL_InputEvent &key);
	void BuildWorldListBox();
	void ScanDirectoryForModInfo(const string &path);
	void ScanDirectoriesForModInfo();
	void OnClickLoad();
	void OnSelected(int selItem);
	void ChangeSelection(int offset);
	bool WorldAlreadyInList(const ModInfoItem &m);

	CL_Window *m_pWindow;
	CL_ListBox *m_pListWorld; //control which worlds are drawn

	vector<ModInfoItem> m_modInfo;
	CL_SlotContainer m_slots; //generic one, easier

};

#endif // EntWorldDialog_h__