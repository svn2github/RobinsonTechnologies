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

enum
{
	C_INPUT_GAME_ONLY = 0,
	C_INPUT_EDITOR_ONLY,
	C_INPUT_GAME_AND_EDITOR
};

class MovingEntity;

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
		m_inputMode = C_INPUT_GAME_ONLY;
	}

	bool m_bShifted;
	bool m_bAlt;
	bool m_bCtrl;
	bool m_bAlways;
	string m_callback;
	int m_entityID;
	int m_inputMode;
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
	void RemoveBindingsByEntity(MovingEntity *pEnt);

protected:
	
	ScriptKeyMap m_map;

private:
};

#endif
