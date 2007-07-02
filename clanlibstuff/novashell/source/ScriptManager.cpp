
#include "AppPrecomp.h"
#include "ScriptManager.h"
#include "main.h"
#include "MovingEntity.h"

#ifndef WIN32
//windows already has this in the precompiled header for speed, I couldn't get that to work on mac..
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#endif

using namespace luabind;

InputManager g_inputManager;

// lifted from the lua book, prints contents of the lua stack
static void stackDump (lua_State *L) 
{ 
	int i=0, top = lua_gettop(L);
LogError("---------------- Lua Error! Stack Dump ----------------\n");
for (i = 1; i <= top; i++) {
	int t = lua_type(L, i);
	switch (t) {
		  case LUA_TSTRING:  LogError("-%d: %s", i, lua_tostring(L, i));        break;
		  case LUA_TBOOLEAN: LogError("-%d: %s",  i, lua_toboolean(L, i) ? "true" : "false");     break;
		  case LUA_TNUMBER:  LogError("-%d: %g",  i, lua_tonumber(L, i));         break;
		  default:           LogError("-%d: %s",  i, lua_typename(L, t));       break;
	}
//now print all globals?


}
//LogMsg("--------------- Lua Stack Dump Finished ---------------\n");
}


void ShowLUAMessagesIfNeeded(lua_State *pState, int result)
{
	
	if (result == 0) return;
	if (lua_gettop(pState) == 0) return;
	stackDump(pState);
	lua_pop(pState, 1);
}

string LuaObjToString(lua_State *L, int stackOffset)
{
	int t = lua_type(L, stackOffset);
	string st = lua_typename(L, t);
	st += " - ";

	string v;
	switch (t)
	{
		case LUA_TSTRING:  v = lua_tostring(L, stackOffset);        break;
		case LUA_TBOOLEAN: v = (lua_toboolean(L, stackOffset) ? "true" : "false");     break;
		case LUA_TNUMBER:  v = CL_String::to(lua_tonumber(L, stackOffset));         break;
		default:
			v = CL_String::to(lua_topointer(L, stackOffset));   
	}

	return st + (string)v;
}

#define C_DEPTH_TO_SEARCH (2-1)

void WalkTable(string tableName, lua_State *L, int depth)
{
	if(!lua_istable(L, -1))
	{
		LogMsg("Table not found");
		return;
	}

	lua_pushnil(L); // first key
	while (lua_next(L, -2))
	{
		int valueType = lua_type(L, -1);
		string value = LuaObjToString(L, -1);
		string key;

		if (lua_type(L, -2) == LUA_TSTRING)
		{
			key = lua_tostring(L, -2);
		} else
		{
			key = LuaObjToString(L, -2);
		}
		
		string spacing;
		int spaces = C_DEPTH_TO_SEARCH-depth;
		for (int i=0; i < spaces; i++) spacing += "    ";
		LogMsg("%s(%s) - Key: %s, value: %s", spacing.c_str(), tableName.c_str(), key.c_str(), value.c_str());
		
		if (valueType == LUA_TTABLE && depth > 0) 
		{
			//recursive thingy
			lua_pushvalue(L, -1); //another copy of the table, because we don't want WalkTable to delete ours
			WalkTable(key, L, depth-1);
			lua_pop(L, 1);  // remove value
		}
	
		lua_pop(L, 1);  // remove value
	}


}

void DumpTable( lua_State *L, const char *pTableName, int tableIndex) //null defaults to the globalsindex
{
	
	if (pTableName)
	{
		lua_pushvalue(L, tableIndex); //try LUA_ENVIRONINDEX later?
		lua_pushlstring(L, pTableName, strlen(pTableName));
		lua_gettable(L,-2);
		stackDump(L);
	    lua_pop(L, 1);
		stackDump(L);
		WalkTable(pTableName, L, C_DEPTH_TO_SEARCH);

	} else
	{
		lua_pushvalue(L, tableIndex); //try LUA_ENVIRONINDEX later?
		WalkTable("Root", L, C_DEPTH_TO_SEARCH);
	}

	lua_pop(L, 1);  // remove the last thing
}

ScriptObject::ScriptObject()
{
	m_bGarbageCollectOnKill = false;
	m_pLuaState = NULL;
	m_threadReference = LUA_NOREF;
	assert(GetScriptManager && "Threads can't be created before the script manager!");

    m_pLuaState = lua_newthread( GetScriptManager->GetMainState());
	if (!m_pLuaState) throw CL_Error("Unable to init lua thread");

	//LB is an independent thread, but its globals
	//table is still the same as LA's globals table,
	//so we'll replace it with a new table.
	//The new table is set up so it will still look
	//to LA's globals for items it doesn't contain.

	lua_newtable( m_pLuaState );    //new globals table
	lua_newtable( m_pLuaState );    //metatable
	lua_pushliteral( m_pLuaState, "__index" );
	lua_pushvalue( m_pLuaState, LUA_GLOBALSINDEX );  //original globals
	lua_settable( m_pLuaState, -3 );
	lua_setmetatable( m_pLuaState, -2 );
	lua_replace( m_pLuaState, LUA_GLOBALSINDEX );    //replace LB's globals
	m_threadReference = luaL_ref(GetScriptManager->GetMainState(),LUA_GLOBALSINDEX); //store this so it won't GC itself
  
	//DumpTable(GetScriptManager->GetMainState());
	//DumpTable(m_pLuaState, "__index");

    //not needed, make the ref pops it
    //lua_pop(GetScriptManager->GetMainState(), lua_gettop(GetScriptManager->GetMainState())); //pop the thread off
}	

static int luaB_collectgarbage (lua_State *L);

ScriptObject::~ScriptObject()
{
	//clear the only reference needed for garbage collection to occur
	luaL_unref (GetScriptManager->GetMainState(),LUA_GLOBALSINDEX, m_threadReference);
	if (m_bGarbageCollectOnKill)
	{
		lua_gc(m_pLuaState, LUA_GCCOLLECT, 0);
	}
}

void ScriptObject::SetGlobal(const string& key, int value)
{
	lua_pushstring(m_pLuaState, key.c_str());
	lua_pushnumber(m_pLuaState, (lua_Number)value);
	lua_settable(m_pLuaState, LUA_GLOBALSINDEX);
}



bool ScriptObject::Load(const char *pFileName)
{
	int result = luaL_loadfile(m_pLuaState, pFileName);
	if (result == 0) result = lua_pcall(m_pLuaState, 0,0,0);

	switch (result)
	{
	case 0:
		//no errors
		break;
	case LUA_ERRSYNTAX: 
		LogMsg("Syntax error in script %s", pFileName);
		ShowLUAMessagesIfNeeded(m_pLuaState, result);
		return false;
		break;
	case LUA_ERRMEM:
		LogMsg("Memory error loading script %s", pFileName);
		ShowLUAMessagesIfNeeded(m_pLuaState, result);
		return false;
		break;

	default:
		LogMsg("Error trying to load script %s", pFileName);
		ShowLUAMessagesIfNeeded(m_pLuaState, result);
		return false;
	}
		
	#ifdef _DEBUG
	//lua_gc(GetScriptManager->GetMainState(), LUA_GCCOLLECT, NULL);
	

	#endif
	
	return true; //success
}

void ScriptObject::RunString(const char *pString)
{
	//LogMsg("Running %s", pString);
	int result = luaL_dostring(m_pLuaState, pString);
	ShowLUAMessagesIfNeeded(m_pLuaState, result);
}


bool ScriptObject::FunctionExists(const char *pFuncName)
{
	GetScriptManager->SetStrict(false);
	bool bTemp = luabind::type(luabind::globals(m_pLuaState)[pFuncName]) == LUA_TFUNCTION;
	GetScriptManager->SetStrict(true);
	return bTemp;
}

bool ScriptObject::VariableExists(const char *pFuncName)
{
	GetScriptManager->SetStrict(false);
	bool bTemp = luabind::type(luabind::globals(m_pLuaState)[pFuncName]) != LUA_TNIL;
	GetScriptManager->SetStrict(true);
	return bTemp;
}

void ScriptObject::RunFunction(const char *pFuncName)
{
//	DumpTable(m_pLuaState, NULL, LUA_REGISTRYINDEX);
	GetScriptManager->SetStrict(false);

	try { 
		luabind::call_function<void>(m_pLuaState, pFuncName);
	} catch 
		(luabind::error &e)
	{
		ShowLUAMessagesIfNeeded(e.state(), 1);
	
		MovingEntity *pEnt = object_cast<MovingEntity*>(luabind::globals(m_pLuaState)["this"]);

		if (pEnt)
		{
			LogMsg("Entity %d, function %s", pEnt->ID(), pFuncName);
		}
		
	}

	GetScriptManager->SetStrict(true);

}

ScriptManager::ScriptManager()
{
	m_pMainState = NULL;
	Init();
}

ScriptManager::~ScriptManager()
{
	Kill();
} 


int DumpInfo(lua_State *L)
{
	LogMsg("Dumping base environment");

	if (L != GetScriptManager->GetMainState())
	{
		DumpTable(GetScriptManager->GetMainState());
		LogMsg("Dumping Script environment");
	}
	DumpTable(L);
	return 0;
}

bool ScriptManager::Init()
{
	Kill();
	
	m_pMainState = luaL_newstate();
	if (!m_pMainState)
	{
		throw CL_Error("Error initting scripting engine");
	}
	//luaopen_base(m_pMainState);
	luaL_openlibs(m_pMainState);

	lua_register(m_pMainState, "print", luaPrint);
	lua_register(m_pMainState, "LogMsg", luaPrint);
	lua_register(m_pMainState, "DumpInfo", DumpInfo);
	 
	UpdateAfterScreenChange(false);
	open(m_pMainState);
	

	return true; //success
}

void  ScriptManager::UpdateAfterScreenChange(bool bActuallyChanged)
{
	if (m_pMainState)
	{
		luabind::globals(m_pMainState)["C_SCREEN_X"] = GetScreenX;
		luabind::globals(m_pMainState)["C_SCREEN_Y"] = GetScreenY;
	}
}


void ScriptManager::SetStrict(bool bStrict)
{
	if (bStrict)
	{
		SetGlobalBool("g_allowStrict", true);

	} else
	{
		SetGlobalBool("g_allowStrict", false);
	}
}

void ScriptManager::SetGlobal(const char * pGlobalName, int value)
{
	lua_pushstring(m_pMainState, pGlobalName);
	lua_pushnumber(m_pMainState, (lua_Integer)value);
	lua_settable(m_pMainState, LUA_GLOBALSINDEX);

}

bool ScriptManager::VariableExists(const char *pFuncName)
{
	GetScriptManager->SetStrict(false);
	bool bTemp = luabind::type(luabind::globals(m_pMainState)[pFuncName]) != LUA_TNIL;
	GetScriptManager->SetStrict(true);
	return bTemp;
}

bool ScriptManager::FunctionExists(const char *pFuncName)
{
	GetScriptManager->SetStrict(false);
	bool bTemp = luabind::type(luabind::globals(m_pMainState)[pFuncName]) == LUA_TFUNCTION;
	GetScriptManager->SetStrict(true);
	return bTemp;
}

void ScriptManager::SetGlobalBool(const char * pGlobalName, bool value)
{
	lua_pushstring(m_pMainState, pGlobalName);
	lua_pushboolean(m_pMainState, value);
	lua_settable(m_pMainState, LUA_GLOBALSINDEX);
}

void ScriptManager::LoadMainScript(const char *pScriptName)
{
	int result = luaL_dofile(m_pMainState, pScriptName);
	ShowLUAMessagesIfNeeded(m_pMainState, result);
}
void ScriptManager::RunFunction(const char *pFuncName)
{
	GetScriptManager->SetStrict(false);
	try { 
		luabind::call_function<void>(m_pMainState, pFuncName);
	} catch 
		(luabind::error &e)
	{
		ShowLUAMessagesIfNeeded(e.state(), 1);
	}
	GetScriptManager->SetStrict(true);

}

void ScriptManager::RunFunction(const char *pFuncName, bool bBool)
{
	GetScriptManager->SetStrict(false);
	try { 
		luabind::call_function<void>(m_pMainState, pFuncName, bBool);
	} catch 
		(luabind::error &e)
	{
		ShowLUAMessagesIfNeeded(e.state(), 1);
	}
	GetScriptManager->SetStrict(true);
}

void ScriptManager::RunFunction(const char *pFuncName, BaseGameEntity *pBaseGameEntity)
{
	GetScriptManager->SetStrict(false);
	try { 
		luabind::call_function<void>(m_pMainState, pFuncName, pBaseGameEntity);
	} catch 
		(luabind::error &e)
	{
		ShowLUAMessagesIfNeeded(e.state(), 1);
	}
	GetScriptManager->SetStrict(true);
}

void ScriptManager::RunString(const char *pString)
{
	int result = luaL_dostring(m_pMainState, pString);
	ShowLUAMessagesIfNeeded(m_pMainState, result);
}

void ScriptManager::Kill()
{

	g_inputManager.Init(); //clear out any hotkeys that have been associated with script

	if (m_pMainState)
	{

		if (lua_gettop(m_pMainState) != 0)
		{
			stackDump(m_pMainState);
			assert(!"Stack should be empty!");
		}

		lua_close(m_pMainState);
		m_pMainState = NULL;
	}
}

int luaPrint(lua_State *L)
{
	string str;

	int n = lua_gettop(L);  /* number of arguments */
	int i;
	lua_getglobal(L, "tostring");
	for (i=1; i<=n; i++) {
		const char *s;
		lua_pushvalue(L, -1);  /* function to be called */
		lua_pushvalue(L, i);   /* value to print */
		lua_call(L, 1, 1);
		s = lua_tostring(L, -1);  /* get result */
		if (s == NULL)
			return luaL_error(L, LUA_QL("tostring") " must return a string to "
			LUA_QL("print"));
		if (i>1) str += "\t";
		str += s;
		lua_pop(L, 1);  /* pop result */
	}
	//str += "\n";
	if (str.length() > C_LOGGING_BUFFER_SIZE)
	{
		str.resize(C_LOGGING_BUFFER_SIZE);
	}

	LogMsg(str.c_str());
	return 0;
	
}

