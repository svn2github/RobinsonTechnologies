#include "AppPrecomp.h"
#include "PhysicsManager.h"
#include "Map.h"
#include "GameLogic.h"
#include "MovingEntity.h"

PhysicsManager::PhysicsManager()
{
	m_pWorld = NULL;
}

PhysicsManager::~PhysicsManager()
{
}

void PhysicsManager::Kill()
{
	SAFE_DELETE(m_pWorld);
}

bool PhysicsManager::Init( Map *pMap )
{
	Kill();
	
	m_pParent = pMap;
	m_worldAABB.lowerBound.Set(-500.0f, -500.0f);
	m_worldAABB.upperBound.Set(500.0f, 500.0f);

	m_pWorld = new b2World(m_worldAABB, b2Vec2(0, pMap->GetGravity()), true);

	m_pWorld->SetDebugDraw(&m_debugDraw);

	m_pWorld->SetContactListener(&m_contactListener);
	m_pWorld->SetContactFilter(&m_contactFilter);
	m_pWorld->SetDestructionListener(&m_destructionLister);


	return true;
}

void PhysicsManager::SetDrawDebug( bool bNew )
{
	uint32 flags = 0;

	if (bNew)
	{
		flags += b2DebugDraw::e_shapeBit;
		
		//flags += b2DebugDraw::e_jointBit;
		//flags += b2DebugDraw::e_coreShapeBit;
		//flags +=  b2DebugDraw::e_aabbBit;
		//flags +=  b2DebugDraw::e_obbBit;
		//flags +=  b2DebugDraw::e_pairBit;
		//flags +=  b2DebugDraw::e_centerOfMassBit;
		
	}

	m_debugDraw.SetFlags(flags);
}


void PhysicsManager::SetGravity( CL_Vector2 vGrav )
{
	if (m_pWorld)
	{
		m_pWorld->SetGravity(b2Vec2(vGrav.x, vGrav.y));
	}
}

b2Vec2 ToPhysicsSpace( CL_Vector2 v )
{
	return  *(b2Vec2*) &(v / C_PHYSICS_PIXEL_SIZE);
}

CL_Vector2 FromPhysicsSpace( b2Vec2 v )
{
return  *(CL_Vector2*) &(C_PHYSICS_PIXEL_SIZE * v);
}


float ToPhysicsSpace( float f )
{
	return  f / C_PHYSICS_PIXEL_SIZE;
}

float FromPhysicsSpace( float f )
{
	return  C_PHYSICS_PIXEL_SIZE * f;
}

#define SetBit(n) (1<<(n))

void SetCollisionCategoryBit(b2FilterData &filterData, int bitNum, bool bOn)
{
	assert(bitNum >= 0 && bitNum < 16);
		if (bOn)
			{
				//set it
				filterData.categoryBits  |= SetBit(bitNum);

			} else
			{
				//clear it
				filterData.categoryBits &= ~SetBit(bitNum);
			}
}

void UpdateBodyFilterData(b2Body *pBody, const b2FilterData &filterData)
{
	b2Shape *pShape = pBody->GetShapeList();
		for (;pShape; pShape = pShape->GetNext())
	{
		pShape->SetFilterData(filterData);
		pBody->GetWorld()->Refilter(pShape);
	}
}

void SetCollisionMaskBit(b2FilterData &filterData, int bitNum, bool bOn)
{
	
	assert(bitNum >= 0 && bitNum < 16);
	if (bOn)
	{
		//set it
		filterData.maskBits |= SetBit(bitNum);

	} else
	{
		//clear it
		filterData.maskBits &= ~SetBit(bitNum);
	}
}


void ContactListener::Add(const b2ContactPoint* point)
{
	//if (!point->shape1->IsSensor() || !point->shape2->IsSensor()) return;
	//LogMsg("Got col");
	ContactPoint cp;
	
		cp.position = point->position;
	cp.normal = point->normal;
	cp.id = point->id;
	cp.separation = -point->separation;
	cp.velocity  = point->velocity;
	cp.state = e_contactAdded;
	ShapeUserData *pShape1UserData = (ShapeUserData*)point->shape1->GetUserData();
	ShapeUserData *pShape2UserData = (ShapeUserData*)point->shape2->GetUserData();

	if (pShape2UserData->pOwnerEnt)
	{
		cp.shape1 = point->shape2;
		cp.shape2 = point->shape1;
		cp.pOwnerTile = pShape1UserData->pOwnerTile;
		cp.pOwnerEnt = pShape1UserData->pOwnerEnt;
		pShape2UserData->pOwnerEnt->AddContact(cp);
	}


	if (pShape1UserData->pOwnerEnt)
	{
		cp.shape1 = point->shape1;
		cp.shape2 = point->shape2;
		cp.normal = -1*cp.normal;
		cp.velocity = -1*cp.velocity;
		//cp.separation = -1*cp.separation;
		cp.pOwnerTile = pShape2UserData->pOwnerTile;
		cp.pOwnerEnt = pShape2UserData->pOwnerEnt;

		pShape1UserData->pOwnerEnt->AddContact(cp);
	}

	
	//LogMsg("Adding %d", point->id);
}

void ContactListener::Persist(const b2ContactPoint* point)
{

//	if (!point->shape1->IsSensor() || !point->shape2->IsSensor()) return;
	
	
	ContactPoint cp;

	cp.position = point->position;
	cp.normal = point->normal;
	cp.id = point->id;
	cp.separation = -point->separation;
	cp.velocity  = point->velocity;
	cp.state = e_contactPersisted;
	ShapeUserData *pShape1UserData = (ShapeUserData*)point->shape1->GetUserData();
	ShapeUserData *pShape2UserData = (ShapeUserData*)point->shape2->GetUserData();

	if (pShape2UserData->pOwnerEnt)
	{
		cp.shape1 = point->shape2;
		cp.shape2 = point->shape1;
		cp.pOwnerTile = pShape1UserData->pOwnerTile;
		cp.pOwnerEnt = pShape1UserData->pOwnerEnt;
		pShape2UserData->pOwnerEnt->AddContact(cp);
	}


	if (pShape1UserData->pOwnerEnt)
	{
		cp.shape1 = point->shape1;
		cp.shape2 = point->shape2;
		cp.normal = -1*cp.normal;
		cp.velocity = -1*cp.velocity;
		//cp.separation = -1*cp.separation;
		cp.pOwnerTile = pShape2UserData->pOwnerTile;
		cp.pOwnerEnt = pShape2UserData->pOwnerEnt;

		pShape1UserData->pOwnerEnt->AddContact(cp);
	}

	//LogMsg("Persist %d", point->id);

	
}

void ContactListener::Remove(const b2ContactPoint* point)
{

	
	ContactPoint cp;

	cp.position = point->position;
	cp.normal = point->normal;
	cp.id = point->id;
	cp.separation = point->separation;
	cp.velocity  = point->velocity;
	cp.state = e_contactRemoved;
	
	ShapeUserData *pShape1UserData = (ShapeUserData*)point->shape1->GetUserData();
	ShapeUserData *pShape2UserData = (ShapeUserData*)point->shape2->GetUserData();

	if (pShape2UserData->pOwnerEnt)
	{
		cp.shape1 = point->shape2;
		cp.shape2 = point->shape1;
		cp.pOwnerTile = pShape1UserData->pOwnerTile;
		cp.pOwnerEnt = pShape1UserData->pOwnerEnt;
		pShape2UserData->pOwnerEnt->AddContact(cp);
	}


	if (pShape1UserData->pOwnerEnt)
	{
		cp.shape1 = point->shape1;
		cp.shape2 = point->shape2;
		cp.normal = -1*cp.normal;
		cp.velocity = -1*cp.velocity;
		//cp.separation = -1*cp.separation;
		cp.pOwnerTile = pShape2UserData->pOwnerTile;
		cp.pOwnerEnt = pShape2UserData->pOwnerEnt;

		pShape1UserData->pOwnerEnt->AddContact(cp);
	}

	//LogMsg("Remove %d", point->id);

}

bool ContactFilter::ShouldCollide(b2Shape* shape1, b2Shape* shape2)
{

	if (shape1->IsSensor() || shape2->IsSensor())
	{
		//LogMsg("one is a sensor..");
		//sensors are never aborted, because they aren't real collisions, just for information
		return true;
	}

	const b2FilterData& filter1 = shape1->GetFilterData();
	const b2FilterData& filter2 = shape2->GetFilterData();
	//LogMsg("Checking col..");
	if (filter1.groupIndex == filter2.groupIndex && filter1.groupIndex != 0)
	{
		return filter1.groupIndex > 0;
	}

	bool collide = (filter1.maskBits & filter2.categoryBits) != 0 && (filter1.categoryBits & filter2.maskBits) != 0;
	
	if (!collide) return collide;

	//last chance to reject stuff

	ShapeUserData *pShape1UserData = (ShapeUserData*)shape1->GetUserData();
	ShapeUserData *pShape2UserData = (ShapeUserData*)shape2->GetUserData();

		//first order culling
		MovingEntity *pEnt1, *pEnt2;

		//setup data about what kind of things are colliding
		if (pShape1UserData->pOwnerEnt)
		{
			pEnt1 = pShape1UserData->pOwnerEnt;
			
			if (pShape1UserData->pOwnerEnt)
			{
				pEnt2 = pShape2UserData->pOwnerEnt;
			} else
			{
				pEnt2 = NULL;
			}

		} else if (pShape2UserData->pOwnerEnt) 
		{
			pEnt1 = pShape2UserData->pOwnerEnt;
			pEnt2 = NULL;
		} else
		{
			LogMsg("Two statics colliding, huh?");
		}

		if (pEnt1->GetCollisionMode() == MovingEntity::COLLISION_MODE_NONE) return false;
		if (pEnt2 && pEnt2->GetCollisionMode() == MovingEntity::COLLISION_MODE_NONE) return false;
		
		if (pEnt2 == NULL)
		{
			//ent1 is testing against a static, do we allow this collision?
			if (pEnt1->GetCollisionMode() == MovingEntity::COLLISION_MODE_ALL || pEnt1->GetCollisionMode() == MovingEntity::COLLISION_MODE_STATIC_ONLY)
			{	
				//allowed

			} else
			{
				return false;
			}
		} else
		{
			//both are dynamic entities.
			//LogMsg("Checking %d (%s) and %d(%s)", pEnt1->ID(), pEnt1->GetName().c_str(), pEnt2->ID(), pEnt2->GetName().c_str());
		
			if (
				(pEnt1->GetCollisionMode() == MovingEntity::COLLISION_MODE_STATIC_ONLY
				|| pEnt2->GetCollisionMode() == MovingEntity::COLLISION_MODE_STATIC_ONLY )
				)
			{
				return false;
			} else
			{
				//so far so good..

				if (pEnt1->GetCollisionMode() == MovingEntity::COLLISION_MODE_PLAYER_ONLY)
				{
					if (GetPlayer != pEnt2) return false;
				} if (pEnt2->GetCollisionMode() == MovingEntity::COLLISION_MODE_PLAYER_ONLY)
				{
					if (GetPlayer != pEnt1) return false;
				}
			}


		}

	

	return collide;


}


void DestructionListener::SayGoodbye( b2Joint* joint ) 
{
	
}

void DestructionListener::SayGoodbye( b2Shape* shape )
{
	delete shape->GetUserData();
}

void PhysicsManager::Update( float step )
{
		if (!GetGameLogic()->GetGamePaused())
	{
		//LogMsg("Running physics round..");
	float stepSize = GetApp()->GetGameLogicSpeed()/1000;
		//LogMsg("Step is %.2f", stepSize);
		m_pWorld->Step(stepSize, 10);


		

#ifdef _DEBUG
	m_pWorld->Validate();
#endif

	for (b2Body* pBody = m_pWorld->GetBodyList(); pBody; pBody = pBody->GetNext())
	{
		if (pBody->GetUserData())
			if (!pBody->IsSleeping())
			{


				g_pMapManager->AddToEntityUpdateList((MovingEntity*)pBody->GetUserData());
			}
	}


	if (m_debugDraw.GetFlags() != 0)
	CL_Display::flip(-1); //show it now
		
		
		}

}