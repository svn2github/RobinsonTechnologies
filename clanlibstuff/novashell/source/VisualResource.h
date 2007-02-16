
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


Section: Related Constants

Group: C_SOUND_CONSTANTS
Used with sound functions.

constant: C_SOUND_NONE
Sound functions return this if no sound was created.

*/