#include "AppPrecomp.h"
#include "StateDie.h"
#include "MovingEntity.h"

StateDie registryInstanceDie(NULL); //self register ourselves i nthe brain registry

StateDie::StateDie(MovingEntity * pParent):State(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}

}

StateDie::~StateDie()
{
}

void StateDie::OnAdd()
{
	m_pParent->SetVisualState(VisualProfile::VISUAL_STATE_DIE);
}

void StateDie::OnRemove()
{
}

void StateDie::Update(float step)
{

}

void StateDie::PostUpdate(float step)
{

}

/*
Object: Die
A state that causes an entity to stop moving and play his die animation.


The die animation is detected from the visual profile .xml when anim states named *die_left*, *die_down_left*, and so on are found.  The best match for the <Entity>'s current direction is used.

If you want only one animation for all directions, only set an animation for *die_left*.

Note:

The die animation should be setup to end on the last frame and not to loop.

Example .xml for a die animation:

(code)
<profile name="top_troll">
	<anim state="die_left" spritename="the_die_anim"/>
</profile>

<sprite name="the_die_anim">
	<image fileseq="art/die_left_.png" leading_zeroes="3"/>
	<translation origin="center"/>
	<animation on_finish="last_frame" pingpong="no" loop="no" speed="50"/>
</sprite>
(end)
*/
