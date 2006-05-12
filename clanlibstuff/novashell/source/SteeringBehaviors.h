#ifndef STEERINGBEHAVIORS_H
#define STEERINGBEHAVIORS_H
#pragma warning (disable:4786)



/* -------------------------------------------------
* Copyright 2005 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 1/19/2005 9:51:36 PM
*/

//based on source from

//------------------------------------------------------------------------
//
//  Name:   SteeringBehaviorsBehavior.h
//
//  Desc:   class to encapsulate steering behaviors for a Raven_Bot
//
//  Author: Mat Buckland 2002 (fup@ai-junkie.com)
//
//------------------------------------------------------------------------

class Wall2D;
class MovingEntity;

class SteeringBehaviors
{
public:

	SteeringBehaviors(MovingEntity* agent);

	virtual ~SteeringBehaviors();

	//calculates and sums the steering forces from any active behaviors
	Vector2D Calculate(float step);

	//calculates the component of the steering force that is parallel
	//with the Raven_Bot heading
	double    ForwardComponent();
	void CalculatePlatformer(float step);
	void CreateFeelersPlatform(Vector2D vel);

	//calculates the component of the steering force that is perpendicuar
	//with the Raven_Bot heading
	double    SideComponent();

	void      SetTarget(Vector2D t){m_vTarget = t;}
	Vector2D  Target()const{return m_vTarget;}

	void      SetTargetAgent1(MovingEntity* Agent){m_pTargetAgent1 = Agent;}
	void      SetTargetAgent2(MovingEntity* Agent){m_pTargetAgent2 = Agent;}

	Vector2D  Force()const{return m_vSteeringForce;}

	
	void SeekOn(){m_iFlags |= seek;}
	void ArriveOn(){m_iFlags |= arrive;}
	void WallAvoidanceOn(){m_iFlags |= wall_avoidance;}
	void GravityOn(){m_iFlags |= gravity;}

	void SeekOff()  {if(On(seek))   m_iFlags ^=seek;}
	void ArriveOff(){if(On(arrive)) m_iFlags ^=arrive;}
	void WallAvoidanceOff(){if(On(wall_avoidance)) m_iFlags ^=wall_avoidance;}

	bool SeekIsOn(){return On(seek);}
	bool ArriveIsOn(){return On(arrive);}
	bool WallAvoidanceIsOn(){return On(wall_avoidance);}
	CL_Rect & GetScanRect() {return m_scanArea;}
	void FindClosestWall(Tile *pTile, double distToClosestIPout, Vector2D &steeringForceOut);
	Vector2D Gravity();

	const std::vector<Vector2D>& GetFeelers()const{return m_Feelers;}
	tile_list & GetNearbyTileList() {return m_nearbyTileList;};
	double GetHeightFromGround() {return m_dHeightFromGround;}
	Vector2D m_posWhenCreatingFeelers;
	//this function tests if a specific bit of m_iFlags is set
	
	enum behavior_type
	{
		none               = 0x00000,
		seek               = 0x00002,
		arrive             = 0x00008,
		wander             = 0x00010,
		separation         = 0x00040,
		wall_avoidance     = 0x00200,
		gravity     = 0x00400,
	};
	bool      On(behavior_type bt){return (m_iFlags & bt) == bt;}

	
private:





	//a pointer to the owner of this instance
	MovingEntity*     m_pEnt; 

	//the steering force created by the combined effect of all
	//the selected behaviors
	Vector2D       m_vSteeringForce;
	double m_collisionDistanceRequired;
	//these can be used to keep track of friends, pursuers, or prey
	MovingEntity*    m_pTargetAgent1;
	MovingEntity*     m_pTargetAgent2;

	//the current target
	Vector2D    m_vTarget;
	//any offset used for formations or offset pursuit
	Vector2D     m_vOffset;
	//the distance (squared) a vehicle has to be from a path waypoint before
	//it starts seeking to the next waypoint
	double        m_dWaypointSeekDistSq;
	CL_Rect m_scanArea;

	//a vertex buffer to contain the feelers rqd for wall avoidance  
	std::vector<Vector2D> m_Feelers;

	//the length of the 'feeler/s' used in wall detection
	double                 m_dWallDetectionFeelerLength;

	//multipliers. These can be adjusted to effect strength of the  
	//appropriate behavior. Useful to get flocking the way you require
	//for example.
	double        m_dWeightObstacleAvoidance;
	double        m_dWeightWallAvoidance;
	double        m_dWeightSeek;
	double        m_dWeightFlee;
	double        m_dWeightArrive;
	double        m_dWeightPursuit;
	double        m_dWeightOffsetPursuit;
	double m_dHeightFromGround;
	double m_dWeightGravity;
	Vector2D m_groundPoint;
	Vector2D m_wallNormal, m_pointOfContact;
	Wall2D m_wallHit;

	//how far the agent can 'see'
	double        m_dViewDistance;

	//binary flags to indicate whether or not a behavior should be active
	int           m_iFlags;


	//Arrive makes use of these to determine how quickly a Raven_Bot
	//should decelerate to its target
	enum Deceleration{slow = 3, normal = 2, fast = 1};

	//default
	Deceleration m_Deceleration;

	
	bool      AccumulateForce(Vector2D &sf, Vector2D ForceToAdd);

	tile_list m_nearbyTileList;
	//creates the antenna utilized by the wall avoidance behavior
	void      CreateFeelers(float step);



	/* .......................................................

	BEGIN BEHAVIOR DECLARATIONS

	.......................................................*/


	//this behavior moves the agent towards a target position
	Vector2D Seek(const Vector2D &target);

	//this behavior is similar to seek but it attempts to arrive 
	//at the target with a zero velocity
	Vector2D Arrive(const Vector2D    &target,
		const Deceleration deceleration);


	//this returns a steering force which will keep the agent away from any
	//walls it may encounter
	Vector2D WallAvoidance(tile_list &tList);


	//calculates and sums the steering forces from any active behaviors
	Vector2D CalculatePrioritized(float step);
	void CalculatePlatformer();

	
};




#endif