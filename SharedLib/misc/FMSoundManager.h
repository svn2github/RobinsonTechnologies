/* -------------------------------------------------
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created Wednesday, August 24, 2004 7:24:40 PM
*/

#ifndef FMSoundManager_HEADER_INCLUDED // include guard
#define FMSoundManager_HEADER_INCLUDED  // include guard

#include <vector>
#include <string>
#include "ISoundManager.h"

using namespace std;
#include <fmod.h>

class CFMSound
{
public:
    CFMSound()
    {
        p_sample = NULL;
    }
    FSOUND_SAMPLE *p_sample;
    string st_fname;
};


class CFMSoundManager: public ISoundManager
{
public:

    CFMSoundManager();
    virtual ~CFMSoundManager();
    
    virtual int PlayMixed(const char *p_fname);
    virtual int Play(const char *p_fname);
    virtual int PlayMusic(const char *p_fname, int i_loop_count);
    
    virtual bool Init();
    virtual void UpdateSounds() {}; //called every frame, only need for fading/3d
    virtual void MuteAll(bool b_new);

    virtual void Kill(); //kills everything, unloads all sounds and main driver
    virtual void KillMusic(); //unloads just the music

    virtual int PlayLooping(const char *p_fname);
    virtual void KillChannel(int i_channel);
    virtual bool IsInitted() {return m_b_ready;}
    virtual bool IsMusicPlaying();

private:

	FSOUND_SAMPLE * GetSoundSampleByName(const char *p_fname);
	CFMSound * PreloadSound(const char *p_fname);
	
  bool m_b_ready;

  FMUSIC_MODULE *m_p_music_mod; //used only for music
  FSOUND_SAMPLE *m_p_music_sample; //used only for music
  vector<CFMSound> m_a_sounds; //array of our loaded sounds
};



#endif                  // include guard
