#include "AppPrecomp.h"
#include "VisualProfile.h"
#include "VisualResource.h"

VisualProfile::VisualProfile()
{

	//build up our default anim data, helps us to quickly understand what NPC's can and can't support, custom
	//data can also be added by its profile .xml file

	m_animArray.resize(BUILTIN_ANIM_COUNT);

	m_animArray[IDLE_LEFT].m_name = "idle_left";
	m_animArray[IDLE_RIGHT].m_name = "idle_right";
	m_animArray[RUN_LEFT].m_name = "run_left";
	m_animArray[RUN_RIGHT].m_name = "run_right";

}

VisualProfile::~VisualProfile()
{
}

bool VisualProfile::IsActive(int stateID)
{
	return m_animArray[stateID].m_pSprite != NULL;
}

CL_Sprite * VisualProfile::GetSpriteByAnimID(int animID)
{
	if (animID < 0 || animID >= int(m_animArray.size()))
	{
		throw CL_Error("Bad anim id:" + CL_String::from_int(animID)+" in visual profile " + GetName());
	} else
	{

		if (!IsActive(animID))
		{
			throw CL_Error("Missing animation data for visual profile " + GetName() + " at index " + CL_String::from_int(animID) );
		}
	}

	return m_animArray[animID].m_pSprite;
}

CL_Sprite * VisualProfile::GetSprite(int eState, int eFacing)
{
	int animID = 0;

	switch (eState)
	{
	case STATE_IDLE:
		if (eFacing == FACING_LEFT) animID = IDLE_LEFT; else animID = IDLE_RIGHT;
		break;

	case STATE_RUN:
		if (eFacing == FACING_LEFT) animID = RUN_LEFT; else animID = RUN_RIGHT;
		break;

	default:
		throw CL_Error("Unknown state:" +CL_String::from_int(eState));
	}

	if (!IsActive(animID))
	{
		throw CL_Error("Missing animation data for visual profile " + GetName() + " at index " + CL_String::from_int(animID) );
	}
	return m_animArray[animID].m_pSprite;

}

int VisualProfile::TextToAnimID(const string & stState)
{
	//check to see if it exists
	for (unsigned int i=0; i < m_animArray.size(); i++)
	{
		if (stState == m_animArray[i].m_name)
		{
			//bingo
			return i;
		}
	}
	LogMsg("Unknown anim type: %s.  Keep in mind they are case sensitive.", stState.c_str());
	return -1;
}

int VisualProfile::TextToAnimIDCreatedIfNeeded(const string & stState)
{
	//check to see if it exists
	for (unsigned int i=0; i < m_animArray.size(); i++)
	{
		if (stState == m_animArray[i].m_name)
		{
			//bingo
			return i;
		}
	}

	//add it
	int id = m_animArray.size();
	ProfileAnim newProfile;
	newProfile.m_name = stState;

	m_animArray.push_back(newProfile);
	m_animArray[id].m_name = stState;
	return id;
}

void VisualProfile::AddAnimInfo(CL_DomElement &node)
{
	string stState = node.get_attribute("state");
	int animID = TextToAnimIDCreatedIfNeeded(stState);
	string stSpriteName = node.get_attribute("spritename");
	bool mirrorx = CL_String::to_bool(node.get_attribute("mirrorx"));
	bool mirrory = CL_String::to_bool(node.get_attribute("mirrory"));
	if (IsActive(animID))
	{
		LogMsg("Error, state %s already has a sprite attached to it in profile %s.", stState.c_str(), GetName().c_str());
	}

	try
	{
		m_animArray[animID].m_pSprite = new CL_Sprite(stSpriteName, m_pParent->m_pResourceManager);
	} catch(CL_Error error)
	{
		LogMsg("Error with putting anim %s as state %s while building profile %s.  Make sure you spelled it right and it's in the xml. (%s)",
			stSpriteName.c_str(), stState.c_str(), GetName().c_str(), error.message.c_str());
		SAFE_DELETE(m_animArray[animID].m_pSprite);
		return;
	}


	if (mirrory)
	{
		m_animArray[animID].m_pSprite->set_angle_pitch(-180);
	}

	if (mirrorx)
	{
		m_animArray[animID].m_pSprite->set_angle_yaw(-180);
	}

	m_animArray[animID].m_name = stState;
}

bool VisualProfile::Init(VisualResource *pVisualResource, const string &profileName)
{
	m_pParent = pVisualResource;
	m_name = profileName;

	CL_Resource resource = m_pParent->m_pResourceManager->get_resource(profileName);

	CL_DomNode cur_node;
	for (
		cur_node = resource.get_element().get_first_child();
		!cur_node.is_null();
	cur_node = cur_node.get_next_sibling())
	{
		if (!cur_node.is_element()) continue;
		CL_DomElement cur_element = cur_node.to_element();
		string type = cur_element.get_tag_name();

		if (type == "anim")
		{
			AddAnimInfo(cur_element);
		} else
		{
			LogMsg("Warning: Don't know what %s is in the %s profile xml", type.c_str(), profileName.c_str());
		}
	}
	return true; //success
}
