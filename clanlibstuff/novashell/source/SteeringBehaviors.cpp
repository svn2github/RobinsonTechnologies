#include "AppPrecomp.h"
#include "Screen.h"
#include "SteeringBehaviors.h"
#include "GameLogic.h"
#include "CollisionData.h"

#define C_NOT_CLOSE_TO_GROUND 20000
using std::vector;

//------------------------- ctor -----------------------------------------
//
//------------------------------------------------------------------------
SteeringBehaviors::SteeringBehaviors(MovingEntity* agent)
{
 			 m_pEnt = agent;
             m_iFlags = 0;
             m_dViewDistance = 15;
             m_dWallDetectionFeelerLength = 4.0f;
             m_Feelers.resize(3);
             m_Deceleration = normal;
             m_pTargetAgent1 = NULL;
             m_pTargetAgent2 = NULL;
             m_dWeightSeek = 0.7;
             m_dWeightArrive = 1;
			 m_dWeightGravity = 1.0;
			 m_dWeightWallAvoidance = agent->MaxSpeed();
			 m_scanArea = CL_Rect(0,0,0,0);
			 m_dHeightFromGround = C_NOT_CLOSE_TO_GROUND; //unknown
			 GravityOn();
}

//---------------------------------dtor ----------------------------------
SteeringBehaviors::~SteeringBehaviors(){}


/////////////////////////////////////////////////////////////////////////////// CALCULATE METHODS 


//----------------------- Calculate --------------------------------------
//
//  calculates the accumulated steering force according to the method set
//  in m_SummingMethod
//------------------------------------------------------------------------
Vector2D SteeringBehaviors::Calculate(float step)
{ 
  //reset the steering force
  m_vSteeringForce.Zero();
  m_vSteeringForce = CalculatePrioritized(step);
  return m_vSteeringForce;
}

//------------------------- ForwardComponent -----------------------------
//
//  returns the forward component of the steering force
//------------------------------------------------------------------------
double SteeringBehaviors::ForwardComponent()
{
  return m_pEnt->Heading().Dot(m_vSteeringForce);
}

//--------------------------- SideComponent ------------------------------
//  returns the side component of the steering force
//------------------------------------------------------------------------
double SteeringBehaviors::SideComponent()
{
  return m_pEnt->Side().Dot(m_vSteeringForce);
}


//--------------------- AccumulateForce ----------------------------------
//
//  This function calculates how much of its max steering force the 
//  vehicle has left to apply and then applies that amount of the
//  force to add.
//------------------------------------------------------------------------
bool SteeringBehaviors::AccumulateForce(Vector2D &RunningTot,
                                       Vector2D ForceToAdd)
{  
  //calculate how much steering force the vehicle has used so far
  double MagnitudeSoFar = RunningTot.Length();

  //calculate how much steering force remains to be used by this vehicle
  double MagnitudeRemaining = m_pEnt->MaxSpeed() - MagnitudeSoFar;


  //return false if there is no more force left to use
  if (MagnitudeRemaining <= 0.0) return false;

  //calculate the magnitude of the force we want to add
  double MagnitudeToAdd = ForceToAdd.Length();
  
  //if the magnitude of the sum of ForceToAdd and the running total
  //does not exceed the maximum force available to this vehicle, just
  //add together. Otherwise add as much of the ForceToAdd vector is
  //possible without going over the max.
  if (MagnitudeToAdd < MagnitudeRemaining)
  {
    RunningTot += ForceToAdd;
  }

  else
  {
    MagnitudeToAdd = MagnitudeRemaining;

    //add it to the steering force
    RunningTot += (Vec2DNormalize(ForceToAdd) * MagnitudeToAdd); 
  }

  return true;
}



//---------------------- CalculatePrioritized ----------------------------
//
//  this method calls each active steering behavior in order of priority
//  and acumulates their forces until the max steering force magnitude
//  is reached, at which time the function returns the steering force 
//  accumulated to that  point
//------------------------------------------------------------------------
Vector2D SteeringBehaviors::CalculatePrioritized(float step)
{       
  Vector2D force;
/*
  if (!On(seek) && m_pEnt->Velocity().Length() < 1)
  {
	  if (m_dHeightFromGround < 5)
	  {
		  return force;
	  }
  }
  */


  if (On(wall_avoidance))
  {
 
	  //tile_list tList;
	  m_collisionDistanceRequired = ((m_pEnt->Velocity().Length())*8)+10;
	  float padding = m_collisionDistanceRequired;
	  m_scanArea.left = m_pEnt->GetPos().x - padding;
	  m_scanArea.top = m_pEnt->GetPos().y - padding;
	  m_scanArea.right = m_pEnt->GetPos().x + padding;
	  m_scanArea.bottom = m_pEnt->GetPos().y + padding;

	  m_nearbyTileList.clear();
	  CreateFeelers(step);
	  GetWorldCache->AddTilesByRect(m_scanArea, &m_nearbyTileList, C_DEFAULT_LAYER);
	    
      force = WallAvoidance(m_nearbyTileList) * m_dWeightWallAvoidance;
	  if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;

/*	 
	  if (!m_vSteeringForce.isZero())
	  {
		  LogMsg("Force returning..");
		  return force;
	  }
	  */
  }

  if (On(gravity) && GetWorld->GetGravity())
  {
		//  LogMsg("Height from ground: %.2f", m_dHeightFromGround);
		  force = Gravity();
		  if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
	  
  }

  if (On(seek))
  {
    force = Seek(m_vTarget) * m_dWeightSeek;
    if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }

/*
  if (On(arrive))
  {
    force = Arrive(m_vTarget, m_Deceleration) * m_dWeightArrive;

    if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }
*/
  return m_vSteeringForce;
}


/////////////////////////////////////////////////////////////////////////////// START OF BEHAVIORS

Vector2D SteeringBehaviors::Gravity()
{
		return Vector2D(0,1) * GetWorld->GetGravity();
}

//------------------------------- Seek -----------------------------------
//
//  Given a target, this behavior returns a steering force which will
//  direct the agent towards the target
//------------------------------------------------------------------------
Vector2D SteeringBehaviors::Seek(const Vector2D &target)
{
  Vector2D DesiredVelocity = Vec2DNormalize(target - m_pEnt->GetPos())
                            * m_pEnt->MaxSpeed();

  return (DesiredVelocity - m_pEnt->Velocity());
}


//--------------------------- Arrive -------------------------------------
//
//  This behavior is similar to seek but it attempts to arrive at the
//  target with a zero velocity
//------------------------------------------------------------------------
Vector2D SteeringBehaviors::Arrive(const Vector2D    &target,
                                const Deceleration deceleration)
{
  Vector2D ToTarget = target - m_pEnt->GetPos();

  //calculate the distance to the target
  double dist = ToTarget.Length();

  if (dist > 0)
  {
    //because Deceleration is enumerated as an int, this value is required
    //to provide fine tweaking of the deceleration..
    const double DecelerationTweaker = 0.3;

    //calculate the speed required to reach the target given the desired
    //deceleration
    double speed =  dist / ((double)deceleration * DecelerationTweaker);     

    //make sure the velocity does not exceed the max
    speed = MinOf(speed, m_pEnt->MaxSpeed());

    //from here proceed just like Seek except we don't need to normalize 
    //the ToTarget vector because we have already gone to the trouble
    //of calculating its length: dist. 
    Vector2D DesiredVelocity =  ToTarget * speed / dist;

    return (DesiredVelocity - m_pEnt->Velocity());
  }

  return Vector2D(0,0);
}

void SteeringBehaviors::FindClosestWall(Tile *pTile, double distToClosestIPout, Vector2D &steeringForceOut)
{
	CollisionData *pCol = pTile->GetCollisionData();
	Vector2D point, closestPoint;
	point_list *wallList;
	unsigned int i;
	unsigned int flr, flrSave;
	Wall2D *pClosestWall = NULL;
	double DistToThisIP    = 0.0;

	Vector2D tilePos(pTile->GetPos().x, pTile->GetPos().y);

	static CollisionData colCustomized;

	if (pTile->UsesTileProperties())
	{
		//we need a customized version
		CreateCollisionDataWithTileProperties(pTile, colCustomized);
		pCol = &colCustomized; //replacement version
	}

	line_list::iterator lineListItor = pCol->GetLineList()->begin();

	while (lineListItor != pCol->GetLineList()->end())
	{
		//examine each line list for suitability
		wallList = lineListItor->GetPointList();
		for (i = 0; i < wallList->size(); i++)
		{
			//one scan for each feeler we're using
			for (flr = 0; flr < m_Feelers.size(); flr++)
			{
			
			   if (GetWorld->GetGravity() != 0)
			   {
					//check to see if this is the ground so we can get the height.  Huge waste, but do you know a better way?
				   if (LineIntersection2D(m_posWhenCreatingFeelers, 
					   m_pEnt->GetPos()+Vector2D(0,300),
					   wallList->at(i).From()+tilePos,
					   wallList->at(i).To() + tilePos,
					   DistToThisIP,
					   point))
				   {
					 //  double dot = wallList->at(i).Normal().Dot(Vector2D(0,1));
					  // if (dot < 0.45)
					   {
						   m_dHeightFromGround = min(m_dHeightFromGround, DistToThisIP);
						   //	LogMsg("Dot is %.2f, height from ground is %.2f", dot, m_dHeightFromGround);
						   //	LogMsg("Steering force out is %.2f", steeringForceOut.Length());
						   //this can count as a ground plane
						   m_groundPoint = point;
					   }

				   }



			   }

				if (LineIntersection2D(m_pEnt->GetPos(), 
					m_Feelers[flr],
					wallList->at(i).From()+tilePos,
					wallList->at(i).To() + tilePos,
					DistToThisIP,
					point))
				{
					//is this the closest found so far? If so keep a record
					if (DistToThisIP < distToClosestIPout/* && DistToThisIP < m_collisionDistanceRequired*/ ) 
					{
						flrSave = flr; //remember which feeler we used
						distToClosestIPout = DistToThisIP;
						pClosestWall = &wallList->at(i);
						closestPoint = point;
					}	
				}
			}

			}

		lineListItor++;
	}

	if (pClosestWall)
	{
		//calculate by what distance the projected position of the agent
		//will overshoot the wall
		Vector2D OverShoot = m_Feelers[flrSave] - closestPoint;

		//create a force in the direction of the wall normal, with a 
		//magnitude of the overshoot
		steeringForceOut = pClosestWall->Normal() * (OverShoot.Length());
		m_pointOfContact = closestPoint;
		m_wallNormal = pClosestWall->Normal();
		m_wallHit = *pClosestWall;
	}


}

//--------------------------- WallAvoidance --------------------------------
//
//  This returns a steering force that will keep the agent away from any
//  walls it may encounter
//------------------------------------------------------------------------
Vector2D SteeringBehaviors::WallAvoidance(tile_list &tList)
{
	Vector2D SteeringForce;
	m_dHeightFromGround = C_NOT_CLOSE_TO_GROUND;


	if (tList.empty()) return SteeringForce;

  double DistToClosestIP = MaxDouble;

  tile_list::iterator listItor = tList.begin();

  CollisionData *pCol;
  while (listItor != tList.end())
  {

	  pCol = (*listItor)->GetCollisionData();
	  if (pCol && !pCol->GetLineList()->empty())
	  {
		//looks like it may have something
		FindClosestWall(*listItor, DistToClosestIP, SteeringForce);
	  }
	  
	  listItor++;
  }
  return SteeringForce;
}

//------------------------------- CreateFeelers --------------------------
//
//  Creates the antenna utilized by WallAvoidance
//------------------------------------------------------------------------
void SteeringBehaviors::CreateFeelers(float step)
{
  //feeler pointing straight in front
   m_posWhenCreatingFeelers = m_pEnt->GetPos();
   double mult = (m_pEnt->Velocity().Length());
   
   if (On(gravity))
   {
		mult += Gravity().Length();
   }
   if (On(seek))
   {
	   mult+= (Seek(m_vTarget).Length() * m_dWeightSeek);
   }

   //mult *= step;

   mult += m_dWallDetectionFeelerLength;
	//LogMsg("Vel length is %.2f", mult);
   mult = max(mult, m_dWallDetectionFeelerLength);
   Vector2D VelHeading = m_pEnt->Velocity();
   VelHeading.Normalize();
   //VelHeading = VelHeading.GetReverse();
   

   m_Feelers[0] = m_pEnt->GetPos() + (mult * VelHeading);

 
//weaken these feelers a bit
 mult /= 2;

  //feeler to left
  Vector2D temp = VelHeading;
  Vec2DRotateAroundOrigin(temp, HalfPi/4);
  m_Feelers[1] = m_pEnt->GetPos() + mult * temp;

  //feeler to right
  temp = VelHeading;
  Vec2DRotateAroundOrigin(temp, -(HalfPi/4));
  m_Feelers[2] = m_pEnt->GetPos() + mult* temp;
  
}

void SteeringBehaviors::CreateFeelersPlatform(Vector2D vel)
{
	//feeler pointing straight in front
	m_posWhenCreatingFeelers = m_pEnt->GetPos();
	double mult = vel.Length();

	//mult *= 1.5;
	Vector2D VelHeading = vel;
	VelHeading.Normalize();
	m_Feelers.resize(3);

	//m_Feelers[0] = m_pEnt->GetPos() + (mult * VelHeading);

	
	//but how much of this so called power is going straight down?
	double downpower = VelHeading.Dot(Vector2D(0,1));
	double rightpower = VelHeading.Dot(Vector2D(1,0));
	LogMsg("Downpower is %.2f, rightpower is %.2f", downpower, rightpower);


		m_Feelers[0] = m_pEnt->GetPos() + Vector2D(0,1)* (downpower * mult);

		
		m_Feelers[1] = m_pEnt->GetPos() + Vector2D(1,0)* (rightpower * mult);

		
		
		m_Feelers[2] = m_pEnt->GetPos();
		
		m_Feelers[2] += Vector2D(1,0)* ((rightpower * mult) * 0.5f);
		m_Feelers[2] += Vector2D(0,1)* ((downpower * mult) * 0.5f);
		
		

//second feeler especially for ground checks


	/*
	//weaken these feelers a bit
	mult /= 8;

	//feeler to left
	Vector2D temp = VelHeading;
	Vec2DRotateAroundOrigin(temp, HalfPi/4);
	m_Feelers[1] = m_pEnt->GetPos() + mult * temp;

	//feeler to right
	temp = VelHeading;
	Vec2DRotateAroundOrigin(temp, -(HalfPi/4));
	m_Feelers[2] = m_pEnt->GetPos() + mult* temp;
*/
}
void SteeringBehaviors::CalculatePlatformer(float step)
{
	//if no steering force is produced decelerate the player by applying a
	//braking force

	Vector2D vel = m_pEnt->Velocity();
	//if no steering force is produced decelerate the player by applying a
	//braking force
	Vector2D vPlayerMovement, force;
	if (On(seek))
	{
		vPlayerMovement = Seek(m_vTarget);
	}
	
	force += vPlayerMovement;

	if (!On(seek))
	{
		const double BrakingRate = 0.75;  //lower means stop faster
		vel = ( vel * BrakingRate);                                     
	}
   //LogMsg("Height from ground is %.2f", m_dHeightFromGround);
	if (On(gravity) && m_dHeightFromGround > 0.2)
	{
		force += Gravity();
	} 

	//calculate the acceleration
	Vector2D accel = force / m_pEnt->Mass();

	accel.Truncate(m_pEnt->MaxForce());
	//update the velocity
	vel += accel;

	//make sure vehicle does not exceed maximum velocity
	vel.Truncate(m_pEnt->MaxSpeed());

	CreateFeelersPlatform(vel*(1.01));
	
	//see if any of our feelers hit a wall?
	if (On(wall_avoidance))
	{
		//tile_list tList;
		m_collisionDistanceRequired = vel.Length()+10;
		float padding = m_collisionDistanceRequired;
		m_scanArea.left = m_pEnt->GetPos().x - padding;
		m_scanArea.top = m_pEnt->GetPos().y - padding;
		m_scanArea.right = m_pEnt->GetPos().x + padding;
		m_scanArea.bottom = m_pEnt->GetPos().y + padding;
		m_nearbyTileList.clear();
		GetWorldCache->AddTilesByRect(m_scanArea, &m_nearbyTileList, C_DEFAULT_LAYER);
   	    force = WallAvoidance(m_nearbyTileList);
		
		if (!force.isZero())
		{
			vel += (force);
		}
	}

	//update the position
	Vector2D pos = m_pEnt->GetPos();

	pos += vel;//*step
	
	if (m_dHeightFromGround != C_NOT_CLOSE_TO_GROUND && pos.y > m_groundPoint.y)
	{
		pos.y = m_groundPoint.y-20.1f;
	}
	
	m_pEnt->SetPos(pos);
	//if the vehicle has a non zero velocity the heading and side vectors must 
	//be updated
	if (!vel.isZero())
	{    
		m_pEnt->SetHeading(Vec2DNormalize(vel));
		m_pEnt->SetSide(vel.Perp());
	}
}

