
/* -------------------------------------------------
* Copyright 2006 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 27:2:2006   11:04
*/


#ifndef VisualResource_HEADER_INCLUDED // include guard
#define VisualResource_HEADER_INCLUDED  // include guard

class VisualProfile;

typedef vector <VisualProfile*> vec_visual_profile;

class VisualResource
{
public:

    VisualResource();
    virtual ~VisualResource();

	bool Init(const string &fileName);
	VisualProfile * GetProfile(const string &profileName);
	void Save();

protected:

	void CopyFromProfilesToDocument(CL_DomDocument &document);

	friend class VisualProfile;
	string m_fileName;
	//we create our own resourcemanager to load the XML stuff
	CL_ResourceManager *m_pResourceManager;
	vec_visual_profile m_profileVec;
	
};

#endif                  // include guard


//natural docs stuff
/*
Object: SoundManager
Controls all game audio.

About:

This is a global object that can always be accessed.

Usage:

(code)
GetSoundManager:Play("audio/warp.ogg");
(end)

Group: Member Functions

func: Play
(code)
number Play(string fileName)
(end)
Plays a sound.  Audio files are automatically cached after use for speed.

Parameters:

fileName - A .wav or .ogg sound file to play.

Returns:

A numerical handle to the sound created.  Save it if you want to <Kill> it later or adjust its properties.

func: PlayLooping
(code)
number PlayLooping(string fileName)
(end)
Like <Play> but the sound loops for eternity.  You must use <Kill> to make it stop.

Parameters:

fileName - A .wav or .ogg sound file to play.

Returns:

A numerical handle to the sound created.  Save it if you want to <Kill> it later or adjust its properties.

func: SetSpeedFactor
(code)
nil SetSpeedFactor(number soundID, number speedMod)
(end)
Allows you to adjust the speed/pitch of a sound.

Usage:
(code)
local soundID = this:PlaySoundPositioned("~/audio/quack.ogg");
//randomize the pitch
GetSoundManager:SetSpeedFactor(soundID, 0.90 + (random() * 0.2) );
(end)

Parameters:

soundID - A valid sound handle.
speedMod - A number indicating the speed modification, 1 is normal speed, 1.1 is faster, 0.9 is slower.

func: SetVolume
(code)
nil SetVolume(number soundID, number volume)
(end)

Parameters:

soundID - A valid sound handle.
volume - The new volume of the sound.  0 to mute, 1 for max volume.

func: SetPan
(code)
nil SetPan(number soundID, number pan)
(end)

Parameters:

soundID - A valid sound handle.
pan - A range between -1 and 1.  (0 would be normal stereo with neither speaker getting favored)

func: IsPlaying
(code)
boolean IsPlaying(number soundID)
(end)

Parameters:

soundID - A valid sound handle.

Returns:

True if the specified sound is currently playing.

func: Kill
(code)
nil Kill(number soundID)
(end)

Stops a sound from playing.

Parameters:

soundID - A valid sound handle.

func: MuteAll
(code)
nil MuteAll(boolean bMuteActive)
(end)

Easy way to turn the volume of ALL sounds off and on at the same time, without actually stopping the sounds.

Parameters:

bMuteActive - If true, the sound is muted.  If false, sounds can be heard.

Section: Related Constants

Group: C_SOUND_CONSTANTS
Used with sound functions.

constant: C_SOUND_NONE
Sound functions return this if no sound was created.

*/