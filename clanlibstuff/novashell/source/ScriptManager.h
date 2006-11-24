
/* -------------------------------------------------
* Copyright 2006 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 20:2:2006   21:11
*/


#ifndef ScriptManager_HEADER_INCLUDED // include guard
#define ScriptManager_HEADER_INCLUDED  // include guard

//a few macros to reduce typing

//this works inside of MovingEntity
#define LUABIND_CATCH(a) catch (luabind::error &e) { ShowLUAMessagesIfNeeded(e.state(), 1); \
	LogError(a.c_str());} catch (...) {LogError("Unknown LUA error, inalid return type maybe? : %s", a.c_str());}
//this works inside of MovingEntity
#define LUABIND_ENT_CATCH(a) catch (luabind::error &e) { ShowLUAMessagesIfNeeded(e.state(), 1); \
	LogError("Entity %d (%s) : %s", ID(), GetName().c_str(), a);} catch (...) {LogError("Unknown LUA error in Entity %d (%s) : %s.  Invalid return type maybe?", ID(), GetName().c_str(), a);}

//this works inside of brains
#define LUABIND_ENT_BRAIN_CATCH(a) catch (luabind::error &e) { ShowLUAMessagesIfNeeded(e.state(), 1); \
	LogError("Entity %d (%s) : %s", m_pParent->ID(), m_pParent->GetName().c_str(), a);} catch (...) {LogError("Unknown LUA error (Invalid return type maybe?) in Entity %d (%s) : %s", m_pParent->ID(), m_pParent->GetName().c_str(), a);}

#include "ScriptKeyManager.h"

class ScriptObject
{
public:

	ScriptObject();
	~ScriptObject();
	bool Load(const char *pFileName);
	void RunFunction(const char *pFuncName);
	void SetGlobal(const string& key, int value);
	void RunString(const char *pString);
	bool FunctionExists(const char *pFuncName);

	lua_State * GetState() {return m_pLuaState;}

protected:

	lua_State* m_pLuaState;
	int m_threadReference;

};

class ScriptManager
{
public:

    ScriptManager();
    virtual ~ScriptManager();

	bool Init();
	void Kill();
	void LoadMainScript(const char *pScriptName);
    void RunFunction(const char *pFuncName);
	void RunString(const char *pString);

	lua_State * GetMainState() {return m_pMainState;}

protected:

lua_State * m_pMainState;

};

extern ScriptKeyManager g_keyManager;

int luaPrint(lua_State *L);
void ShowLUAMessagesIfNeeded(lua_State *pState, int result);
void DumpTable( lua_State *L, const char *pTableName = NULL, int tableIndex = LUA_GLOBALSINDEX);

#endif                  // include guard
