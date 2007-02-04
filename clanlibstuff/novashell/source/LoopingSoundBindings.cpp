
#include "AppPrecomp.h"
#include "MovingEntity.h"
#include "LoopingSoundBindings.h"

#ifndef WIN32
//windows already has this in the precompiled header for speed, I couldn't get that to work on mac..
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#endif

using namespace luabind;

LoopingSound::LoopingSound()
{	
	m_handle = 0;
}

LoopingSound::LoopingSound(const string &file)
{	
	m_handle = 0;
	m_file = file;
	LogMsg("Initting sound");
}

LoopingSound::~LoopingSound()
{
	
	Play(false);
	LogMsg("Killing sound");
}


void LoopingSound::Init(const string &file)
{
	Play(false);
	
	m_file = file;
}

void LoopingSound::Play(bool bOn)
{
	if (m_file.empty()) return;
	if (bOn)
	{
		if (m_handle == 0)
		{
			if (g_pSoundManager)
				m_handle = g_pSoundManager->PlayLooping(m_file.c_str());
		}
	} else
	{
		if (m_handle != 0)
		{
			//let's shut it off
			if (g_pSoundManager)
				g_pSoundManager->KillChannel(m_handle);
			m_handle = C_SOUND_NONE;
		}
	}
}


void luabindLoopingSound(lua_State *pState)
{
	module(pState)
		[

			class_<LoopingSound>("LoopingSound")
			.def(constructor<>())
			.def(constructor<const string &>())
			.def("Init", &LoopingSound::Init)
			.def("Play", &LoopingSound::Play)
	
		];
}


