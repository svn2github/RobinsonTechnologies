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
