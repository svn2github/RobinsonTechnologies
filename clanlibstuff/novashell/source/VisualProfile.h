
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
			clTexParameteri(CL_TEXTURE_2D, CL_TEXTURE_MAG_FILTER, CL_NEAREST);
			//clTexParameteri(CL_TEXTURE_2D, CL_TEXTURE_MIN_FILTER, CL_NEAREST);

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
	 VISUAL_STATE_IDLE,
	 VISUAL_STATE_WALK,
	 VISUAL_STATE_RUN,
	 VISUAL_STATE_PAIN,
	 VISUAL_STATE_DIE,
	 VISUAL_STATE_ATTACK1,
	 VISUAL_STATE_SPECIAL,
	 VISUAL_STATE_TURN,

	};

	enum eAnim
	{
		IDLE_LEFT = 0,
		IDLE_RIGHT,
		WALK_LEFT,
		WALK_RIGHT,
		RUN_LEFT,
		RUN_RIGHT,
		PAIN_LEFT,
		PAIN_RIGHT,
		TURN_LEFT,
		TURN_RIGHT,
		DIE_LEFT,
		DIE_RIGHT,
		ATTACK1_LEFT,
		ATTACK1_RIGHT,


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
