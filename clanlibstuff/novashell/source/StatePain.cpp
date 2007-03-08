#include "AppPrecomp.h"
#include "StatePain.h"
#include "MovingEntity.h"

StatePain registryInstanceStatePain(NULL); //self register ourselves in the brain registry

StatePain::StatePain(MovingEntity * pParent):State(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}

}

StatePain::~StatePain()
{
}

void StatePain::OnAdd()
{
	m_pParent->SetVisualState(VisualProfile::VISUAL_STATE_PAIN);
	
}

void StatePain::OnRemove()
{
}

void StatePain::Update(float step)
{

}

void StatePain::PostUpdate(float step)
{

}

/*
Object: Pain
A state that causes an entity to stop moving and play his attack animation.


An attack animation is detected from the visual profile .xml when anim states named <pain_left>, <pain_down_left>, and so on are found.  The best match for the <Entity>'s current direction is used.

If you want only one animation for all directions, only set an animation for <pain_left>.

*/

