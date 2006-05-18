#include "CL_SoundManager.h"


CL_SoundManager::CL_SoundManager()
{
	
}

void CL_SoundManager::MuteAll(bool b_new)
{

}

bool CL_SoundManager::Init()
{
return true;
}


CL_SoundManager::~CL_SoundManager()
{
	Kill(); //deallocate everything we had going
}


int CL_SoundManager::Play(const char *p_fname)
{
	return 0;
}


int CL_SoundManager::PlayMixed(const char *p_fname)
{
	return 0;
}


int CL_SoundManager::PlayLooping(const char *p_fname)
{
	return 0;
}

void CL_SoundManager::KillChannel(int i_channel)
{
	
}


int CL_SoundManager::PlayMusic( const char *p_fname, int i_loop_count)
{
	return 0;
}

bool CL_SoundManager::IsMusicPlaying()
{
	return false;
}

void CL_SoundManager::KillMusic()
{
	
}

void CL_SoundManager::Kill()
{
	
}