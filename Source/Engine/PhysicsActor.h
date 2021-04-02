#pragma once

class String;
class ContactListener;

//! BodyType enumeration
//! @enum Box2D enumerations
enum class BodyType
{
	STATIC, DYNAMIC, KINEMATIC
};

//-----------------------------------------------------------------
// PhysicsActor Class
//-----------------------------------------------------------------
class PhysicsActor
{
public:
	//! constructor
	//! @param pos the position of the actor
	//! @param angle the angle of rotation of this actor
	//! @param bodyType BodyType is an Enumeration with three possible values. Eg. use BodyType::DYNAMIC
	PhysicsActor(float2 pos, double angle, BodyType bodyType);

	//! Destructor
	virtual ~PhysicsActor();

	// C++11 make the class non-copyable
	PhysicsActor(const PhysicsActor&) = delete;
	PhysicsActor& operator=(const PhysicsActor&) = delete;

	//! Adds a box shape to this actor, the box is centered around the position of the actor
	//! @param width represents the width of the physics box 
	//! @param height represents the height of the physics box 
	//! @param restitution is the a value between 0 (full damping) and 1 (no energy loss). The default value is 0.8
	//! @param friction represents the static and dynamic friction.  The friction parameter is
	//! usually set between 0 and 1, but can be any non - negative value. A friction value of 0 turns off friction
	//!	and a value of 1 makes the friction strong. When the friction force is computed between two shapes,
	//!	Box2D must combine the friction parameters of the two parent fixtures.
	//! @param density represents the density of this shape.  The default density is 1 
	//! @return true if the operation was successful



    bool AddBoxShape(double width, double height, double restituion = 0.8, double friction = 0.7, double density = 1);
    bool AddBoxShapeWithOffset(double width, double height,float2 offset = float2(0,0), double restituion = 0.8, double friction = 0.7, double density = 1);
	//! Adds a circle shape to the actor, the circle is centered around the position of the actor
	//! @param radius is the radius of the circle
	//! @param offset is position, relative to the body position
	//! @param restitution is the a value between 0 (full damping) and 1 (no energy loss). The default value is 0.8
	//! @param friction represents the static and dynamic friction.  The friction parameter is
	//! usually set between 0 and 1, but can be any non - negative value. A friction value of 0 turns off friction
	//!	and a value of 1 makes the friction strong. When the friction force is computed between two shapes,
	//!	Box2D must combine the friction parameters of the two parent fixtures.
	//! @param density represents the density of this shape.  The default density is 1 
	//! @return true if the operation was successful
	bool AddCircleShape(double radius, float2 offset = float2(0,0), double restitution = 0.8, double friction = 0.7, double density = 1);

	//! Adds a polygon shape to the actor, the polygon coordinates are relative to the position of the actor. Polygons must be convex. 
	//! @param vertexArrRef is a std::vector containing float2 values that are representing the coordinates of the points of the polygon. There is a limit of 8 vertices per polygon!
	//! @param restitution is the a value between 0 (full damping) and 1 (no energy loss)
	//! @param friction represents the static and dynamic friction.  The friction parameter is
	//! usually set between 0 and 1, but can be any non - negative value. A friction value of 0 turns off friction
	//!	and a value of 1 makes the friction strong. When the friction force is computed between two shapes,
	//!	Box2D must combine the friction parameters of the two parent fixtures.
	//! @param density represents the density of this shape.  The default density is 1 
	//! @return true if the operation was successful
	bool AddPolygonShape(const std::vector<float2> & vertexArrRef, double restitution = 0.8, double friction = 0.7, double density = 1);

	//! Adds a chain shape to the actor. A chain is a line list that provides two-sided collision.
	//! Torque can not be applied to this shape!!
	//! @param vertexArrRef is a std::vector containing float2 values that are representing the coordinates of the points of the line list
	//! @param restitution is the a value between 0 (full damping) and 1 (no energy loss)
	//! @param friction represents the static and dynamic friction.  The friction parameter is
	//! usually set between 0 and 1, but can be any non - negative value. A friction value of 0 turns off friction
	//!	and a value of 1 makes the friction strong. When the friction force is computed between two shapes,
	//!	Box2D must combine the friction parameters of the two parent fixtures.
	//! @param density represents the density of this shape.  The default density is 1 
	//! @return true if the operation was successful
	bool AddChainShape(const std::vector<float2> & vertexArrRef, bool closed, double restitution = 0.8, double friction = 0.7, double density = 1);

	//! Sets the name of this actor
	//! @param name is the String containing the name
	void SetName(string const& name);

	//! Returns the name of this actor
	string GetName();

	//! Returns the position of this actor
	float2 GetPosition();

	//! Sets the positon of this actor
	//! param position is the new position for this actor
	void SetPosition(const float2& positionRef);

	//! Returns the angle of this actor
	double GetAngle();

	//! Sets the actors angle
	void SetAngle(double angle);

	//! Sets velocity of the actor
	//! @param velocity is the new velocity
	void SetLinearVelocity(float2 const& velocity);

	//! Gets velocity of the actor
	//! @return the velocity vector
	float2 GetLinearVelocity();

	//! Sets angular velocity of the actor
	//! @param velocity is the new angular velocity
	void SetAngularVelocity(double velocity);

	//! Gets the angular velocity of the actor
	//! @return the angular velocity
	double GetAngularVelocity();

	//! Returns the total mass of the actor
	double GetMass();

	//! Enables or disables rotation of this actor
	//! @param fixedRotation when set to true, it disables rotation of this actor
	void SetFixedRotation(bool fixedRotation);

	//! Returns the state of fixed rotation.
	//! true: rotation is fixed
	//! false: rotation enabled
	bool IsFixedRotation();

	//! prevent dynamic bodies from tunneling through dynamic bodies. Use it wisely for performance reasons.
	//! @param bullet if true, this treats the actor as a fast moving entity
	void SetBullet(bool bullet);

	//! You may wish a body to be created but not participate in collision or dynamics. 
	//! @param active if false this will prevent the actor to participate in collision or dynamics
	void SetActive(bool active);

	//! each Actor has a 'gravity scale' to strengthen or weaken the effect of the world's gravity on it.
	//! @param scale e.g. when set to 0 the object will not react to gravity
	void SetGravityScale(double scale);

	// adds this actor to the list of actors that can report via the BeginContact and EndContact methods
	// @ptr is the pointer of an object of a class that is derived from the PhysicsBase class
	//void AddContact(void* ptr);

	//! test a point for overlap 
	bool IsPointInActor(const float2& pointRef);

	//! test a point for overlap 
	bool IsPointInActor(const POINT &pointRef);

	//! setting to true will cause the actor to be like a ghost, no collisions with other actors.
	//! Trigger events will be generated when another actor overlaps.
	//! @param trigger set to true will cause the actor to be like a trigger
	 void SetTrigger(bool trigger);

     void SetFriction(double friction);
     double GetFriction();
	//! setting to true will cause the actor to be like a ghost, no collisions with other actors.
	//! NO trigger events will be generated when another actor overlaps.
	//! @param trigger set to true will cause the actor to be like a ghost
	void SetGhost(bool ghost);

	//! cast a ray at a shape to get the point of first intersection and normal vector. No hit will register
	//! if the ray starts inside the shape.
	//! returns true if an intersection point was found
	//! @param point1 first point of the ray
	//! @param point2 second point of the ray
	//! @param intersectionRef resulting intersection point
	//! @param normalRef resulting normal on the surface of intersection point
	//! @param fractionRef resulting fraction of the intersection is a value between 0 and 1 where
	//!        0 -> intersection at point1 
	//!        1 -> intersection at point2
	bool Raycast(float2 point1, float2 point2, float2& intersectionRef, float2& normalRef, double& fractionRef);

	//! forces act gradually over time to change the velocity of a body. Use this with the KeyboardKeyDown.
	//! @param force is the force vector that is to be applied on the actor
	//! @param offsetPoint is the point at which we apply the force/impulse in object space (according to the position of the physics actor) 
	void ApplyForce(float2 force, float2 offsetPoint = float2());

	//! forces act gradually over time to change the velocity of a body. Use this with the KeyboardKeyDown.
	//! @param torque is the torque that is to be applied on the actor
	void ApplyTorque(double torque);

	//! impulses can change a body's velocity immediately. Use Impulse with KeyBoardKeyPressed.
	//! @param force is the force vector that is to be applied on the actor
	//! @param offsetPoint is the point at which we apply the force/impulse in object space (according to the position of the physics actor) 
	void ApplyLinearImpulse(float2 impulse, float2 offsetPoint = float2());

	//! impulses can change a body's velocity immediately. Use Impulse with KeyBoardKeyPressed.
	//! @param impulse is the impulse that is to be applied on the actor
	void ApplyAngularImpulse(double impulse);

	//! if one of this actors shapes overlap with one of the other actor this will return true
	//! @param otherActor is the pointer of the other actor object
	bool IsOverlapping(PhysicsActor* otherActor);

	//! retrieve a list of all the touching contact points
	//! @return a std::vector containing all the touching contact points
	std::vector<float2> GetContactList();

	//! add this object to the contactlistener. This must be an object of a class that is derived from the ContactListener Class
	//! listener is "this": the pointer of the object derived from the contactlistener class
	void AddContactListener(ContactListener *listenerPtr);

	//! remove this object from the contactlistener. This must be an object of a class that is derived from the ContactListener Class
	//! listener is "this": the pointer of the object derived from the contactlistener class
	void RemoveContactListener(ContactListener *listenerPtr);

	//! return the ContactListener pointer that was stored through AddContactListener. This can be a nullptr.
	ContactListener *GetContactListener();

	// TODOOOO
	//! Sets the category for a collision filtering
	//! see http://www.iforce2d.net/b2dtut/collision-filtering
	void SetCollisionFilter(const b2Filter &filterRef);
	b2Filter GetCollisionFilter();

	// Internal use. Box2D has constrains to dimensions of objects, pixels are not meters
	static const int SCALE = 100;

private:
	// apply the state of m_bGhost to this fixture
	void ApplyGhost(b2Fixture * fixturePtr);
	void ApplyTrigger(b2Fixture * fixturePtr);
	//! we have a lot of friends here
	friend class PhysicsRevoluteJoint;
	friend class PhysicsPrismaticJoint;
	friend class PhysicsDistanceJoint;
	friend class PhysicsWeldJoint;

	//! private internal member function, not for students
	bool SetBody(float2 pos, double angle, BodyType bodyType);

	bool m_bGhost = false, m_bTrigger = false;
	std::string m_Name;
	b2Body* m_BodyPtr = nullptr;
	// pointer is held by body -> b2Fixture *m_FixturePtr = nullptr;

};

