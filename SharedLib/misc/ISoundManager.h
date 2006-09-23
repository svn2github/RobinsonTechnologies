//  ***************************************************************
//  ISoundManager - Creation date: 05/12/2006
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2006 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef ISoundManager_h__
#define ISoundManager_h__

#include "MiscUtils.h"

#include <cassert>

class ISoundManager
{
public:
	virtual ~ISoundManager() {};

	virtual int PlayMixed(const char *p_fname)=0;
	virtual int Play(const char *p_fname)=0;
	virtual int PlayMusic(const char *p_fname, int i_loop_count)=0;

	virtual bool Init()=0;
	virtual void UpdateSounds()=0; //called every frame, only need for fading/3d
	virtual void MuteAll(bool b_new)=0;

	virtual void Kill()=0; //kills everything, unloads all sounds and main driver
	virtual void KillMusic()=0; //unloads just the music

	virtual int PlayLooping(const char *p_fname)=0;
	virtual void KillChannel(int i_channel)=0;
	virtual bool IsInitted()=0;
	virtual bool IsMusicPlaying()=0;

	virtual void SetSpeedFactor(int soundID, float mod) { assert(!"Not implemented with this driver yet");}; //a mod of 2 would play twice as fast
	virtual void SetVolume(int soundID, float volume) { assert(!"Not implemented with this driver yet");};
	virtual void AddEffect(int soundID, int effectID, float parmA, float parmB, float parmC) { assert(!"Not implemented with this driver yet");};
	virtual	void RemoveAllEffects(int soundID) { assert(!"Not implemented with this driver yet");};


	enum
	{
		C_EFFECT_FADE = 0,

		//add more above here
		C_EFFECT_COUNT
	};

protected:
	

private:
};

//helper for looping sounds
class LoopingSound
{
public:

	LoopingSound()
	{	
		m_handle = 0;
	}

	~LoopingSound()
	{
		Play(false);
	}

	void Init(ISoundManager *pSoundManager, string file)
	{
		m_pSoundManager = pSoundManager;
		m_file = file;
	}

	void Play(bool bOn)
	{
		if (bOn)
		{
			if (m_handle == 0)
			{
				if (m_pSoundManager)
				m_handle = m_pSoundManager->PlayLooping(m_file.c_str());
			}
		} else
		{
			if (m_handle != 0)
			{
				//let's shut it off
				if (m_pSoundManager)
				m_pSoundManager->KillChannel(m_handle);
				m_handle = 0;
			}
		}
	}

private:

	ISoundManager *m_pSoundManager;
	string m_file;
	int m_handle;

};



#endif // ISoundManager_h__
