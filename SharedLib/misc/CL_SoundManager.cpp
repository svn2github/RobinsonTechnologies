#include "CL_SoundManager.h"
#include "CL_VirtualFileManager.h"


//well, there are many betters way to get access to the virtual file manager but this will do
extern CL_VirtualFileManager g_VFManager;

CL_SoundManager::CL_SoundManager()
{
	m_baseID = 0;
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
	SoundResource *pBuff = LocateSound(p_fname);
	if (!pBuff) return 0;
	
	int id = GetUniqueID();
	SoundSession s;
	
	m_soundSessions[id] = s;
	m_soundSessions[id].m_session = pBuff->m_buffer.play(false);
	
	return id;
}

SoundResource * CL_SoundManager::LocateSound(const char *pFname)
{
	string fileName = pFname;
	if (!g_VFManager.LocateFile(fileName))
	{
		LogError("Unable to locate audio file %s.", pFname);
		return NULL;
	}

	soundResourceMap::iterator itor = m_soundResources.find(pFname);

	if (itor != m_soundResources.end())
	{
		//bingo!
		return &(itor->second);
	}

	//else, we need to create it

	SoundResource s;
	s.m_buffer = CL_SoundBuffer(fileName);
	m_soundResources[pFname] = s;
	
	return &m_soundResources[pFname];

}

int CL_SoundManager::PlayMixed(const char *p_fname)
{
	SoundResource *pBuff = LocateSound(p_fname);
	if (!pBuff) return 0;
	int id = GetUniqueID();
	SoundSession s;
	
	m_soundSessions[id] = s;
	m_soundSessions[id].m_session = pBuff->m_buffer.play(false);
	return id;
}

int CL_SoundManager::PlayLooping(const char *p_fname)
{
	SoundResource *pBuff = LocateSound(p_fname);
	if (!pBuff) return 0;
	int id = GetUniqueID();
	SoundSession s;

	m_soundSessions[id] = s;
	m_soundSessions[id].m_session = pBuff->m_buffer.play(true);
	return id;
}


SoundSession * CL_SoundManager::GetSessionFromID(int id)
{
	soundSessionMap::iterator itor = m_soundSessions.find(id);
	if (itor != m_soundSessions.end())
	{
		//bingo!
		return &(itor->second);
	}
	
	//doesn't exist
	return NULL;
}

void CL_SoundManager::RemoveAllEffects(int soundID)
{
	SoundSession *pSession = GetSessionFromID(soundID);

	if (!pSession)
	{
		LogMsg("SoundID %d invalid", soundID);
		return;
	}

	pSession->m_session.remove_all_filters();
}

void CL_SoundManager::AddEffect(int soundID, int effectID, float parmA, float parmB, float parmC)
{
	SoundSession *pSession = GetSessionFromID(soundID);

	if (!pSession)
	{
		LogMsg("SoundID %d invalid", soundID);
		return;
	}

	switch (effectID)
	{
		case C_EFFECT_FADE:
			
			{
			CL_FadeFilter *pFade = new CL_FadeFilter(parmA);
			//The CL_FadeFilter assumes the speed is 22.05
			parmC = (parmC * ((pSession->m_session.get_frequency()/1000) /22.05f));
			pSession->m_session.add_filter(pFade, true);
			pFade->fade_to_volume(parmB, int(parmC));
			break;
			}

		default:
			LogError("Unknown sound effect: %d", effectID);
			return;
	}
	

	
}

void CL_SoundManager::SetSpeedFactor(int soundID, float mod)
{
	SoundSession *pSession = GetSessionFromID(soundID);

	if (!pSession)
	{
		LogMsg("SoundID %d invalid", soundID);
		return;
	}
	pSession->m_session.set_speedfactor(mod);
}

void CL_SoundManager::SetVolume(int soundID, float volume)
{
	SoundSession *pSession = GetSessionFromID(soundID);

	if (!pSession)
	{
		LogMsg("SetVolume: SoundID %d invalid", soundID);
		return;
	}
	pSession->m_session.set_volume(volume);
}

void CL_SoundManager::SetPan(int soundID, float pan) //-1 to 1
{
	SoundSession *pSession = GetSessionFromID(soundID);

	if (!pSession)
	{
		LogMsg("SetPan: SoundID %d invalid", soundID);
		return;
	}
	pSession->m_session.set_pan(pan);
}


void CL_SoundManager::KillChannel(int i_channel)
{
	m_soundSessions.erase(i_channel);
}

int CL_SoundManager::PlayMusic( const char *p_fname, int i_loop_count)
{
	SoundResource *pBuff = LocateSound(p_fname);
	if (!pBuff) return 0;
	m_music.stop();
	m_music = pBuff->m_buffer.prepare(i_loop_count != 0);

	m_music.play();

	
	return 0;
}

bool CL_SoundManager::IsMusicPlaying()
{
	return m_music.is_playing();
}

void CL_SoundManager::KillMusic()
{
	m_music.stop();
}

void CL_SoundManager::Kill()
{
	KillMusic();
	m_soundSessions.clear();
	m_soundResources.clear();
}

void CL_SoundManager::UpdateSounds()
{
	//kill any dead sounds

	soundSessionMap::iterator itor;
	for (itor = m_soundSessions.begin(); itor != m_soundSessions.end();)
	{
		if (!itor->second.m_session.is_playing())
		{
			//delete it I guess
			soundSessionMap::iterator itorSave = itor;
			itorSave++;
			m_soundSessions.erase(itor);
			itor = itorSave;
			//LogMsg("Deleting sound. %d left", m_soundSessions.size());
			continue;
		}

		itor++;
	}

}
