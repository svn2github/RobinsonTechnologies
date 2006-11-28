//  ***************************************************************
//  ScriptKeyManager - Creation date: 05/10/2006
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2006 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef ScriptKeyManager_h__
#define ScriptKeyManager_h__

class KeyInfo
{
public:

//where do I save the actual key?  It's the index used for the map.

	KeyInfo()
	{
		m_bShifted = false;
		m_bAlt = false;
		m_bCtrl = false;
		m_bAlways = false;
	}
	bool m_bShifted;
	bool m_bAlt;
	bool m_bCtrl;
	bool m_bAlways;
	string m_callback;
	int m_entityID;
};


typedef std::map<unsigned int, vector<KeyInfo> > ScriptKeyMap;


class ScriptKeyManager
{
public:
	ScriptKeyManager();
	virtual ~ScriptKeyManager();
	void Init(); //clear it

	//Send "" as the callback to remove assignment.
	//Send "g,shift" to mean shifted-g.
	void AddBinding(const string &keyName, const string &callbackFunction, int entityID);
	bool HandleEvent(const CL_InputEvent &key, bool bKeyDown);
	CL_Vector2 GetMousePos();
	void SetMousePos(const CL_Vector2 &pos);
	int StringToInputID(vector<string> & word, const string & keyName);
	bool RemoveBinding(const string &keyName, const string &callbackFunction, int entityID);
	void PrintStatistics();

protected:
	
	ScriptKeyMap m_map;

private:
};

#endif
