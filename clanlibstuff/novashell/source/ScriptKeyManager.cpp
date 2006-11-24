#include "AppPrecomp.h"
#include "ScriptKeyManager.h"
#include "GameLogic.h"
#include "ScriptManager.h"

#ifndef WIN32
//windows already has this in the precompiled header for speed, I couldn't get that to work on mac..
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#endif

#define C_KEY_UNASSIGNED -1


ScriptKeyManager::ScriptKeyManager()
{
	Init();
}

ScriptKeyManager::~ScriptKeyManager()
{
}

void ScriptKeyManager::PrintStatistics()
{
	LogMsg("");
	LogMsg("  ** InputManager Statistics **");

	ScriptKeyMap::iterator itor = m_map.begin();
	int totalBinds = 0;

	for (;itor != m_map.end(); itor++)
		totalBinds += itor->second.size();

	LogMsg("    %d input bindings active.", totalBinds);
}

int ScriptKeyManager::StringToInputID(vector<string> & word, const string & keyName)
{
	
	word = CL_String::tokenize(keyName, ",");

	int keyId = C_KEY_UNASSIGNED;

	CL_InputDevice *pKeyboard = &CL_Display::get_current_window()->get_ic()->get_keyboard();

	if (word.size() < 1)
	{
		LogError("Bad input manager string: Didn't specify anything?");
		return keyId;
	}
	
	//parse key string into virtual id
	for (unsigned int i=0; i < word.size(); i++)
	{
		int id = pKeyboard->string_to_keyid(word[i]);

		if ( id != CL_KEY_CONTROL && id != CL_KEY_SHIFT)
		{
			keyId = id;
			break;
		}

		//check for mouse stuff
		if (word[0] == "mouse_left")
		{
			keyId = CL_MOUSE_LEFT;
		} else
			if (word[0] == "mouse_right")
			{
				keyId = CL_MOUSE_RIGHT;
			} else
				if (word[0] == "mouse_middle")
				{
					keyId = CL_MOUSE_MIDDLE;
				} else
					if (word[0] == "mouse_wheel_down")
					{
						keyId = CL_MOUSE_WHEEL_DOWN;
					} else
						if (word[0] == "mouse_wheel_up")
						{
							keyId = CL_MOUSE_WHEEL_UP;
						} else
							if (word[0] == "mouse_xbutton1")
							{
								keyId = CL_MOUSE_XBUTTON1;
							} else
								if (word[0] == "mouse_xbutton2")
								{
									keyId = CL_MOUSE_XBUTTON2;
								} 

								if (keyId != C_KEY_UNASSIGNED)
								{
									break;
								}

	}


	if (keyId == C_KEY_UNASSIGNED)
	{

		LogError("Error converting %s to an input ID. (must be lower case!)", keyName.c_str());
	}

	return keyId;
}

bool ScriptKeyManager::RemoveBinding(const string &keyName, const string &callbackFunction, int entityID)
{
	vector<string> word;
	
	int keyID = StringToInputID(word, keyName);
	if (keyID == C_KEY_UNASSIGNED) return false;

	ScriptKeyMap::iterator itor = m_map.find(keyID);

	if (m_map.end() == itor)
	{
		LogMsg("GetInputManager:RemovingBinding failed to find binding %s, %s.", keyName, callbackFunction);
		return false;
	}

	CL_InputDevice *pKeyboard = &CL_Display::get_current_window()->get_ic()->get_keyboard();

	bool bCtrl = false;
	bool bShifted = false;

	//check for special keys
	for (unsigned int i=0; i < word.size(); i++)
	{
	int specialID = pKeyboard->string_to_keyid(word[i]);

	if (specialID == keyID) continue; //we already handled this one..

		switch (specialID)
		{
		case CL_KEY_CONTROL:
			bCtrl = true;
			break;

		case CL_KEY_SHIFT:
			bShifted = true;
			break;
		}
	}

	vector<KeyInfo> &ki = itor->second;
	vector<KeyInfo>::reverse_iterator ritor;
	
	ritor = ki.rbegin();

	for (;ritor != ki.rend(); ritor++)
	{

		if ((*ritor).m_bShifted == bShifted && (*ritor).m_bCtrl == bCtrl && entityID == (*ritor).m_entityID)
		{
			//we found it!
			ki.erase( (++ritor).base()); //weirdness to convert it to a normal iterator
			return true;
		}

	}

	return false;
}

void ScriptKeyManager::AddBinding(const string &keyName, const string &callbackFunction, int entityID)
{
	vector<string> word;
	
	int keyID = StringToInputID(word, keyName);
	
	if (keyID == C_KEY_UNASSIGNED) return;

	if (callbackFunction.empty())
	{
		//remove it completely
		m_map.erase(m_map.find(keyID));
		return;
	}

	KeyInfo k;
	k.m_callback = callbackFunction;

	CL_InputDevice *pKeyboard = &CL_Display::get_current_window()->get_ic()->get_keyboard();

	//check for special keys
	for (unsigned int i=0; i < word.size(); i++)
	{
		
	
		int specialID = pKeyboard->string_to_keyid(word[i]);

		if (specialID == keyID) continue; //we already handled this one..

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
	
	k.m_entityID = entityID;

	vector<KeyInfo> &ki = m_map[keyID];
	ki.push_back(k);
}

CL_Vector2 ScriptKeyManager::GetMousePos()
{
	return CL_Vector2(CL_Mouse::get_x(), CL_Mouse::get_y());
}

void ScriptKeyManager::SetMousePos(const CL_Vector2 &pos)
{
	CL_Mouse::set_position(pos.x, pos.y);
}



//returns true if handled, false if not
bool ScriptKeyManager::HandleEvent(const CL_InputEvent &key, bool bKeyDown)
{
	ScriptKeyMap::iterator itor = m_map.find(key.id);
	if (itor == m_map.end()) return false; //we don't have it

	//otherwise, send all the callbacks
	vector<KeyInfo> v = itor->second; //yes, we'll make a copy.  This is so it's ok to delete things inside the
	//callback without screwing anything up.

	vector<KeyInfo>::reverse_iterator ritor;

	ritor = v.rbegin();

	for (;ritor != v.rend(); ritor++)
	{
		KeyInfo *pKeyInfo = &(*ritor);

		if (pKeyInfo->m_bCtrl == CL_Keyboard::get_keycode(CL_KEY_CONTROL)
			&& pKeyInfo->m_bShifted == CL_Keyboard::get_keycode(CL_KEY_SHIFT))
		{
			bool bKeepPassingItOn = true;
			
			if (pKeyInfo->m_entityID == C_ENTITY_NONE)
			{
				//global kind of thing

				try {bKeepPassingItOn = luabind::call_function<bool>(GetScriptManager->GetMainState(), 
					pKeyInfo->m_callback.c_str(), bKeyDown);
				}  LUABIND_CATCH(pKeyInfo->m_callback);
			} else
			{
				MovingEntity *m_pParent = (MovingEntity*) EntityMgr->GetEntityFromID(pKeyInfo->m_entityID);
				if (!m_pParent)
				{
					LogMsg("InputManager - Can't locate entity %d to run %s", pKeyInfo->m_entityID, pKeyInfo->m_callback.c_str());
				} else
				{
					//let's actually run this function in this entities namespace

					try {bKeepPassingItOn = luabind::call_function<bool>(m_pParent->GetScriptObject()->GetState(), pKeyInfo->m_callback.c_str(),bKeyDown);
					} LUABIND_ENT_BRAIN_CATCH(pKeyInfo->m_callback.c_str());

				}

			}

			if (!bKeepPassingItOn) return true; //true as in we handled it and don't want anyone else to
		}
	}

	return false; //didn't handle event
}


void ScriptKeyManager::Init()
{
	m_map.clear();
}


