#include "AppPrecomp.h"
#include "ScriptKeyManager.h"
#include "GameLogic.h"
#include "ScriptManager.h"

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

	int keyId = 0;

	//parse key string into virtual id
	for (unsigned int i=0; i < word.size(); i++)
	{
		int id = pKeyboard->string_to_keyid(word[i]);

		if ( id != CL_KEY_CONTROL && id != CL_KEY_SHIFT)
		{
			keyId = id;
			break;
		}
	}

	if (keyId == 0)
	{
		LogError("AssignKey: Error assigning key %s. (must be lower case!)", keyName.c_str());
		return;

	}


	if (callbackFunction.empty())
	{
		//remove it completely
		m_map.erase(m_map.find(keyId));
		return;
	}

	KeyInfo k;
	k.m_callback = callbackFunction;

	//check for special keys
	for (unsigned int i=0; i < word.size(); i++)
	{
		int specialID = pKeyboard->string_to_keyid(word[i]);

		if (specialID == keyId) continue; //we already handled this one..

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

	//otherwise, send all the callbacks

	for (unsigned int i=0; i < itor->second.size(); i++)
	{
		KeyInfo *pKeyInfo = &itor->second.at(i);

		if (pKeyInfo->m_bCtrl == CL_Keyboard::get_keycode(CL_KEY_CONTROL)
			&& pKeyInfo->m_bShifted == CL_Keyboard::get_keycode(CL_KEY_SHIFT))
		{


			try {luabind::call_function<void>(GetScriptManager->GetMainState(), 
				pKeyInfo->m_callback.c_str(), bKeyDown);
			}  LUABIND_CATCH(pKeyInfo->m_callback);
		}
	}

	return false; //didn't handle event
}

void ScriptKeyManager::Init()
{
	m_map.clear();
}