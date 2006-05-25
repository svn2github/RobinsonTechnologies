#include "AppPrecomp.h"
#include "ScriptKeyManager.h"

ScriptKeyManager::ScriptKeyManager()
{
	Init();
}

ScriptKeyManager::~ScriptKeyManager()
{
}

void ScriptKeyManager::AssignKey(const string &keyName, const string &callbackFunction)
{
	vector<string> word = CL_String::tokenize(keyName, ",");
	
	CL_InputDevice *pKeyboard = &CL_Display::get_current_window()->get_ic()->get_keyboard();

	if (word.size() < 1)
	{
		LogError("AssignKey: Didn't specify a key?");
		return;
	}
	//parse key string into virtual id
	int keyId =  pKeyboard->string_to_keyid(word[0]);
	
	if (callbackFunction.empty())
	{
		//remove it completely
		m_map.erase(m_map.find(keyId));
		return;
	}

	KeyInfo k;
	k.m_callback = callbackFunction;

	//check for special keys
	for (unsigned int i=1; i < word.size(); i++)
	{
		int specialID = pKeyboard->string_to_keyid(word[i]);

		switch (specialID)
		{
		case CL_KEY_CONTROL:
		//case CL_KEY_LCONTROL:
		//case CL_KEY_RCONTROL:
			k.m_bCtrl = true;
			break;

		case CL_KEY_SHIFT:
		//case CL_KEY_LSHIFT:
		//case CL_KEY_RSHIFT:
			k.m_bShifted = true;
			break;
		}
	}
	
	vector<KeyInfo> &ki = m_map[keyId];
	ki.push_back(k);
}

//returns true if handles, false if not
bool ScriptKeyManager::HandleEvent(const CL_InputEvent &key, bool bKeyDown)
{
	ScriptKeyMap::iterator itor = m_map.find(key.id);
	if (itor == m_map.end()) return false; //we don't have it

	//otherwise
	LogMsg("We have something here!");

	return false; //didn't handle event
}

void ScriptKeyManager::Init()
{
	m_map.clear();
}