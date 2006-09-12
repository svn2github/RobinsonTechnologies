//  ***************************************************************
//  CL_SoundManager - Creation date: 05/12/2006
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2006 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef CL_SoundManager_h__
#define CL_SoundManager_h__

#include "ISoundManager.h"
#include <ClanLib/sound.h>

//these are saved and cached out
class SoundResource
{
public:
	
	CL_SoundBuffer m_buffer;
};

class SoundSession
{
public:

	~SoundSession()
	{
		m_session.stop();
	}
	CL_SoundBuffer_Session m_session;
};

typedef std::map<string, SoundResource> soundResourceMap;

typedef std::map<int, SoundSession> soundSessionMap;

class CL_SoundManager:public ISoundManager
{
public:
	CL_SoundManager();
	virtual ~CL_SoundManager();


	virtual int PlayMixed(const char *p_fname);
	virtual int Play(const char *p_fname);
	virtual int PlayMusic(const char *p_fname, int i_loop_count);

	virtual bool Init();
	virtual void UpdateSounds(); //called every frame, only need for fading/3d
	virtual void MuteAll(bool b_new);

	virtual void Kill(); //kills everything, unloads all sounds and main driver
	virtual void KillMusic(); //unloads just the music

	virtual int PlayLooping(const char *p_fname);
	virtual void KillChannel(int i_channel);
	virtual bool IsInitted() {return false;}
	virtual bool IsMusicPlaying();

protected:

	SoundResource * LocateSound(const char *pFname);
	int GetUniqueID() {return ++m_baseID;}

	soundResourceMap m_soundResources;
	soundSessionMap m_soundSessions;

	CL_SoundBuffer_Session m_music;
	int m_baseID;

private:
};

#endif // CL_SoundManager_h__
