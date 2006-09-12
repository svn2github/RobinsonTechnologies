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

	m_animArray[IDLE_UP].m_name = "idle_up";
	m_animArray[IDLE_DOWN].m_name = "idle_down";

	m_animArray[IDLE_UP_LEFT].m_name = "idle_up_left";
	m_animArray[IDLE_DOWN_LEFT].m_name = "idle_down_left";

	m_animArray[IDLE_UP_RIGHT].m_name = "idle_up_right";
	m_animArray[IDLE_DOWN_RIGHT].m_name = "idle_down_right";

	m_animArray[WALK_LEFT].m_name = "walk_left";
	m_animArray[WALK_RIGHT].m_name = "walk_right";

	m_animArray[WALK_UP].m_name = "walk_up";
	m_animArray[WALK_DOWN].m_name = "walk_down";

	m_animArray[WALK_UP_LEFT].m_name = "walk_up_left";
	m_animArray[WALK_DOWN_LEFT].m_name = "walk_down_left";

	m_animArray[WALK_UP_RIGHT].m_name = "walk_up_right";
	m_animArray[WALK_DOWN_RIGHT].m_name = "walk_down_right";

	m_animArray[RUN_LEFT].m_name = "run_left";
	m_animArray[RUN_RIGHT].m_name = "run_right";

	m_animArray[RUN_UP].m_name = "run_up";
	m_animArray[RUN_DOWN].m_name = "run_down";

	m_animArray[RUN_UP_LEFT].m_name = "run_up_left";
	m_animArray[RUN_DOWN_LEFT].m_name = "run_down_left";

	m_animArray[RUN_UP_RIGHT].m_name = "run_up_right";
	m_animArray[RUN_DOWN_RIGHT].m_name = "run_down_right";

	m_animArray[PAIN_LEFT].m_name = "pain_left";
	m_animArray[PAIN_RIGHT].m_name = "pain_right";

	m_animArray[DIE_LEFT].m_name = "die_left";
	m_animArray[DIE_RIGHT].m_name = "die_right";
	m_animArray[DIE_UP].m_name = "die_up";
	m_animArray[DIE_DOWN].m_name = "die_down";
	m_animArray[DIE_UP_LEFT].m_name = "die_up_left";
	m_animArray[DIE_DOWN_LEFT].m_name = "die_down_left";

	m_animArray[DIE_UP_RIGHT].m_name = "die_up_right";
	m_animArray[DIE_DOWN_RIGHT].m_name = "die_down_right";

	m_animArray[TURN_LEFT].m_name = "turn_left";
	m_animArray[TURN_RIGHT].m_name = "turn_right";

	m_animArray[ATTACK1_LEFT].m_name = "attack1_left";
	m_animArray[ATTACK1_RIGHT].m_name = "attack1_right";

	m_animArray[ATTACK1_UP].m_name = "attack1_up";
	m_animArray[ATTACK1_DOWN].m_name = "attack1_down";

	m_animArray[ATTACK1_UP_LEFT].m_name = "attack1_up_left";
	m_animArray[ATTACK1_DOWN_LEFT].m_name = "attack1_down_left";

	m_animArray[ATTACK1_UP_RIGHT].m_name = "attack1_up_right";
	m_animArray[ATTACK1_DOWN_RIGHT].m_name = "attack1_down_right";

}


VisualProfile::~VisualProfile()
{
}


bool VisualProfile::IsActive(int stateID)
{
	return m_animArray[stateID].m_pSprite != NULL;
}

vector<string> VisualProfile::GetListOfActiveAnims()
{
	vector<string> anims;

	for (unsigned int i=0; i < m_animArray.size(); i++)
	{
		if (IsActive(i))
		{
			//bingo
			anims.push_back(m_animArray[i].m_name);			
		}
	}
	return anims;
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
			
			if (!IsActive(IDLE_LEFT))
			{
				throw CL_Error("Missing animation data for visual profile " + GetName() + " at index " + CL_String::from_int(animID));
			} else
			{
				LogError( ("Missing animation data for visual profile " + GetName() + " at index " + CL_String::from_int(animID)).c_str() );
			}
			return m_animArray[IDLE_LEFT].m_pSprite;
		}
	}

	return m_animArray[animID].m_pSprite;
}

int VisualProfile::GetAnimID(int eState, int eFacing)
{
	int animID = 0;

	switch (eState)
	{
	case VISUAL_STATE_IDLE:

		switch(eFacing)
		{
		case FACING_LEFT: animID = IDLE_LEFT; break;
		case FACING_RIGHT: animID = IDLE_RIGHT;break;
		case FACING_UP: animID = IDLE_UP;break;
		case FACING_DOWN: animID = IDLE_DOWN;break;

		case FACING_UP_LEFT: animID = IDLE_UP_LEFT; break;
		case FACING_DOWN_LEFT: animID = IDLE_DOWN_LEFT; break;
		case FACING_UP_RIGHT: animID = IDLE_UP_RIGHT; break;
		case FACING_DOWN_RIGHT: animID = IDLE_DOWN_RIGHT; break;
		}
		if (!IsActive(animID))
		{
			//if it's something small, let's fix it ourself
			switch (animID)
			{
			case IDLE_UP_RIGHT:
			case IDLE_UP_LEFT:
				animID = IDLE_UP;
				break;

			case IDLE_DOWN_RIGHT:
			case IDLE_DOWN_LEFT:
				animID = IDLE_DOWN;
				break;
			}
		}

		break;

	case VISUAL_STATE_RUN:
		switch(eFacing)
		{
		case FACING_LEFT: animID = RUN_LEFT; break;
		case FACING_RIGHT: animID = RUN_RIGHT; break;
		case FACING_UP: animID = RUN_UP; break;
		case FACING_DOWN: animID = RUN_DOWN; break;

		case FACING_UP_LEFT: animID = RUN_UP_LEFT; break;
		case FACING_DOWN_LEFT: animID = RUN_DOWN_LEFT; break;
		case FACING_UP_RIGHT: animID = RUN_UP_RIGHT; break;
		case FACING_DOWN_RIGHT: animID = RUN_DOWN_RIGHT; break;
		}
		if (!IsActive(animID))
		{
			//if it's something small, let's fix it ourself
			switch (animID)
			{
			case RUN_UP_RIGHT:
			case RUN_UP_LEFT:
				animID = RUN_UP;
				break;

			case RUN_DOWN_RIGHT:
			case RUN_DOWN_LEFT:
				animID = RUN_DOWN;
				break;

			}
		}	

		break;

	case VISUAL_STATE_WALK:
		switch(eFacing)
		{
		case FACING_LEFT: animID = WALK_LEFT; break;
		case FACING_RIGHT: animID = WALK_RIGHT; break;
		case FACING_UP: animID = WALK_UP; break;
		case FACING_DOWN: animID = WALK_DOWN; break;

		case FACING_UP_LEFT: animID = WALK_UP_LEFT; break;
		case FACING_DOWN_LEFT: animID = WALK_DOWN_LEFT; break;
		case FACING_UP_RIGHT: animID = WALK_UP_RIGHT; break;
		case FACING_DOWN_RIGHT: animID = WALK_DOWN_RIGHT; break;
		}	

		if (!IsActive(animID))
		{
			//if it's something small, let's fix it ourself
			switch (animID)
			{
			case WALK_UP_RIGHT:
			case WALK_UP_LEFT:
				animID = WALK_UP;
				break;

			case WALK_DOWN_RIGHT:
			case WALK_DOWN_LEFT:
				animID = WALK_DOWN;
				break;

			}
		}

		break;

	case VISUAL_STATE_PAIN:
		if (eFacing == FACING_LEFT) animID = PAIN_LEFT; else animID = PAIN_RIGHT;
		break;

	case VISUAL_STATE_TURN:
		if (eFacing == FACING_LEFT) animID = TURN_LEFT; else animID = TURN_RIGHT;
		break;

	case VISUAL_STATE_ATTACK1:
		switch(eFacing)
		{
		case FACING_LEFT: animID = ATTACK1_LEFT; break;
		case FACING_RIGHT: animID = ATTACK1_RIGHT; break;
		case FACING_UP: animID = ATTACK1_UP; break;
		case FACING_DOWN: animID = ATTACK1_DOWN; break;

		case FACING_UP_LEFT: animID = ATTACK1_UP_LEFT; break;
		case FACING_DOWN_LEFT: animID = ATTACK1_DOWN_LEFT; break;
		case FACING_UP_RIGHT: animID = ATTACK1_UP_RIGHT; break;
		case FACING_DOWN_RIGHT: animID = ATTACK1_DOWN_RIGHT; break;
		}
		if (!IsActive(animID))
		{
			//if it's something small, let's fix it ourself
			switch (animID)
			{
			case ATTACK1_UP_RIGHT:
			case ATTACK1_UP_LEFT:
				animID = ATTACK1_UP;
				break;

			case ATTACK1_DOWN_RIGHT:
			case ATTACK1_DOWN_LEFT:
				animID = ATTACK1_DOWN;
				break;

			}
		}	

		break;


	case VISUAL_STATE_DIE:
		switch(eFacing)
		{
		case FACING_LEFT: animID = DIE_LEFT; break;
		case FACING_RIGHT: animID = DIE_RIGHT; break;
		case FACING_UP: animID = DIE_UP; break;
		case FACING_DOWN: animID = DIE_DOWN; break;


		case FACING_UP_LEFT: animID = DIE_UP_LEFT; break;
		case FACING_DOWN_LEFT: animID = DIE_DOWN_LEFT; break;
		case FACING_UP_RIGHT: animID = DIE_UP_RIGHT; break;
		case FACING_DOWN_RIGHT: animID = DIE_DOWN_RIGHT; break;

		}
		if (!IsActive(animID))
		{
			//if it's something small, let's fix it ourself
			switch (animID)
			{

			case DIE_UP_RIGHT:
			case DIE_UP_LEFT:
				animID = DIE_UP;
				break;

			case DIE_DOWN_RIGHT:
			case DIE_DOWN_LEFT:
				animID = DIE_DOWN;
				break;

			}
		}	

		if (!IsActive(animID))
		{
			//It's still broke?   Must be a two way death only
			switch (animID)
			{

			case DIE_UP:
				animID = DIE_LEFT;
				break;

			case DIE_DOWN:
				animID = DIE_RIGHT;
				break;
			}
		}	
		break;

	default:
		throw CL_Error("Unknown state:" +CL_String::from_int(eState));
	}

	if (!IsActive(animID))
	{

		throw CL_Error("Missing animation data for visual profile " + GetName() + " at index " + CL_String::from_int(animID)+" (" + m_animArray[animID].m_name+")" );
	}

	return animID;
}

CL_Sprite * VisualProfile::GetSprite(int eState, int eFacing)
{
	return m_animArray[GetAnimID(eState, eFacing)].m_pSprite;
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
	LogError("Unknown anim type: %s.  Keep in mind they are case sensitive.", stState.c_str());
	return -1;
}


int VisualProfile::SpriteToAnimID(const string & stState)
{
	//check to see if it exists
	for (unsigned int i=0; i < m_animArray.size(); i++)
	{
		if (stState == m_animArray[i].m_spriteName)
		{
			//bingo
			return i;
		}
	}
	LogError("Unknown anim type: %s.  Keep in mind they are case sensitive.", stState.c_str());
	return -1;
}

int VisualProfile::SpriteToAnimID(CL_Sprite *pSprite)
{
	//check to see if it exists
	for (unsigned int i=0; i < m_animArray.size(); i++)
	{
		if (pSprite == m_animArray[i].m_pSprite)
		{
			//bingo
			return i;
		}
	}

	LogError("Can't find anim by sprite address");
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
		LogError("Error, state %s already has a sprite attached to it in profile %s.", stState.c_str(), GetName().c_str());
	}

	try
	{
		m_animArray[animID].m_pSprite = new CL_Sprite(stSpriteName, m_pParent->m_pResourceManager);
		clTexParameteri(CL_TEXTURE_2D, CL_TEXTURE_MAG_FILTER, CL_NEAREST);
		//clTexParameteri(CL_TEXTURE_2D, CL_TEXTURE_MIN_FILTER, CL_NEAREST);
	} catch(CL_Error error)
	{
		LogError("Error with putting anim %s as state %s while building profile %s.  Make sure you spelled it right and it's in the xml.\n(%s)",
			stSpriteName.c_str(), stState.c_str(), GetName().c_str(), error.message.c_str());
		SAFE_DELETE(m_animArray[animID].m_pSprite);
		return;
	}

	m_animArray[animID].m_spriteName = stSpriteName;
	
	CL_Origin o;
	int x,y;

	if (mirrory)
	{
		m_animArray[animID].m_pSprite->set_angle_pitch(-180);

		m_animArray[animID].m_pSprite->get_alignment(o, x, y);
		m_animArray[animID].m_pSprite->set_alignment(o, x, -y);

	}

	if (mirrorx)
	{
		m_animArray[animID].m_pSprite->set_angle_yaw(-180);
		m_animArray[animID].m_pSprite->get_alignment(o, x, y);
		m_animArray[animID].m_pSprite->set_alignment(o, -x, y);

	}

	m_animArray[animID].m_name = stState;
}

void VisualProfile::UpdateDocumentSpriteFromAnim(CL_DomNode *node, ProfileAnim *anim)
{
	//LogMsg("Updating %s", node.get_attributes().get_named_item("name").get_node_value().c_str());
	CL_DomNode cur_node;

	for (
		cur_node = node->get_first_child();
		!cur_node.is_null();
	cur_node = cur_node.get_next_sibling())
	{
		if (!cur_node.is_element()) continue;

		CL_DomElement cur_element = cur_node.to_element();

		if (cur_element.get_tag_name() == "translation")
		{
			CL_Origin o;
			int x,y;
			anim->m_pSprite->get_alignment(o, x,y);
			
			cur_element.set_attribute("x", CL_String::from_int(x));
			cur_element.set_attribute("y", CL_String::from_int(y));
		}
	} 
}

void VisualProfile::UpdateToDocument(CL_DomDocument &document)
{
	CL_DomElement domRoot = document.get_document_element();
	CL_DomNodeList domList;
	domList = domRoot.get_elements_by_tag_name("sprite");
	for (int c=0; c < domList.get_length(); c++)
	{
		string name = domList.item(c).get_attributes().get_named_item("name").get_node_value().c_str();

		int animID = SpriteToAnimID(name);
		if (animID != -1)
		{
			//located it.  Let's update it
			UpdateDocumentSpriteFromAnim(&domList.item(c), &m_animArray[animID]);
		}
	}
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
