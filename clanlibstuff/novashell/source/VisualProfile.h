
/* -------------------------------------------------
* Copyright 2006 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 27:2:2006   14:00
*/


#ifndef VisualProfile_HEADER_INCLUDED // include guard
#define VisualProfile_HEADER_INCLUDED  // include guard

class VisualResource;

class ProfileAnim
{
public:

	//assignment and copy constructor for use by the stl vector
	ProfileAnim& operator=(const ProfileAnim &rhs)
   {
	   SAFE_DELETE(m_pSprite);
		if (rhs.m_pSprite)
		{
			m_pSprite = new CL_Sprite(*rhs.m_pSprite);
		}
		m_name = rhs.m_name;
	   return *this;
   }
	
	ProfileAnim(const ProfileAnim &copyFrom)
	{
		m_pSprite = NULL;
		*this = copyFrom;
	}

	ProfileAnim()
	{
		m_pSprite = NULL;
	}

	
	~ProfileAnim()
	{
		SAFE_DELETE(m_pSprite);
	}
	
	CL_Sprite * m_pSprite;
	string m_name;
};


class VisualProfile
{
public:


	enum eFacing
	{
	 FACING_LEFT,
	 FACING_RIGHT
	};

	enum eState
	{
	 STATE_IDLE,
	 STATE_RUN
	};

	enum eAnim
	{
		IDLE_LEFT = 0,
		IDLE_RIGHT,
		RUN_LEFT,
		RUN_RIGHT,

		BUILTIN_ANIM_COUNT
	};

    VisualProfile();
    virtual ~VisualProfile();

	bool Init(VisualResource *pVisualResource, const string &profileName);
	CL_Sprite * GetSprite(int eStatem, int eFacing);
	CL_Sprite * GetSpriteByAnimID(int animID);

	const string & GetName() {return m_name;}
	bool IsActive(int stateID); //check if a certain anim state actually has data in it or not
	int TextToAnimID(const string & stState); //returns -1 if anim id doesn't exist

protected:

	void AddAnimInfo(CL_DomElement &node);
	int TextToAnimIDCreatedIfNeeded(const string & stState);

	string m_name;
	VisualResource *m_pParent;
	vector<ProfileAnim> m_animArray;
};

#endif                  // include guard
