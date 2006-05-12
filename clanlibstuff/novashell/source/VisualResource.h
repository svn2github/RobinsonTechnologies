
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

protected:

	friend VisualProfile;

	CL_ResourceManager *m_pResourceManager;
	vec_visual_profile m_profileVec;
	
};

#endif                  // include guard
