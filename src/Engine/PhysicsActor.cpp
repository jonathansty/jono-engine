#include "stdafx.h" //for compiler

#include "PhysicsActor.h"


// http://www.iforce2d.net/b2dtut
// http://box2d.org/manual.pdf

PhysicsActor::PhysicsActor(float2 pos, double angle, BodyType bodyType)
	: m_BodyPtr(nullptr)
{
	SetBody(pos, angle, bodyType);
}



PhysicsActor::~PhysicsActor()
{
    for (b2Fixture* fixturePtr = m_BodyPtr->GetFixtureList(); fixturePtr != nullptr; fixturePtr = fixturePtr->GetNext())
    {
        fixturePtr->SetUserData(nullptr);
    }

    // remove the body from the scene
    (GameEngine::instance())->GetBox2DWorld()->DestroyBody(m_BodyPtr);
    // from here, there can be a jump to GameEngine::EndContact !!
    m_BodyPtr = nullptr;
}

void PhysicsActor::SetName(String name)
{
	m_Name = name.C_str();
}

String PhysicsActor::GetName()
{
	return String(m_Name.c_str());
}


bool PhysicsActor::SetBody(float2 pos, double angle, BodyType bodyType)
{
	b2BodyDef bodyDef;
	// Define the dynamic body. We set its position and call the body factory.
	switch (bodyType)
	{
	case BodyType::DYNAMIC:
		bodyDef.type = b2_dynamicBody;
		break;
	case BodyType::STATIC:
		bodyDef.type = b2_staticBody;
		break;
	case BodyType::KINEMATIC:
		bodyDef.type = b2_kinematicBody;
		break;

	}

	// rescale to Box2D coordinates
	pos /= SCALE;

	bodyDef.position.Set((float)(pos.x), (float)(pos.y));
	bodyDef.angle = (float)angle;

	m_BodyPtr = (GameEngine::instance())->GetBox2DWorld()->CreateBody(&bodyDef);

	if (m_BodyPtr == nullptr) return false;
	return true;
}

bool PhysicsActor::AddBoxShape(double width, double height, double restitution, double friction, double density)
{
	b2FixtureDef fixtureDef;
	width /= SCALE; height /= SCALE;

	// The extents are the half-widths of the box.
	b2PolygonShape shape;
	shape.SetAsBox((float)width / 2, (float)height / 2);
	// Define the dynamic body fixture.	
	fixtureDef.shape = &shape;
    

	fixtureDef.restitution = (float)restitution;
	// Set the box density to be non-zero, so it will be dynamic.
	if (m_BodyPtr->GetType() == b2_dynamicBody)fixtureDef.density = (float)density;
	else fixtureDef.density = 0.0f;

	// Override the default friction.
	fixtureDef.friction = (float)friction;

	// store this for contact information
	fixtureDef.userData = (void *) this;

	// Add the shape to the body.
	b2Fixture *fixturePtr = m_BodyPtr->CreateFixture(&fixtureDef);
	if (fixturePtr == nullptr)return false;
	ApplyGhost(fixturePtr);
	ApplyTrigger(fixturePtr);
	return true;
}
bool PhysicsActor::AddBoxShapeWithOffset(double width, double height, float2 offset, double restitution, double friction, double density)
{
    b2FixtureDef fixtureDef;
    width /= SCALE; height /= SCALE;

    // The extents are the half-widths of the box.
    b2PolygonShape shape;
    shape.SetAsBox((float)width / 2, (float)height / 2,b2Vec2((float)offset.x,(float)offset.y),0);
    // Define the dynamic body fixture.	
    fixtureDef.shape = &shape;


    fixtureDef.restitution = (float)restitution;
    // Set the box density to be non-zero, so it will be dynamic.
    if (m_BodyPtr->GetType() == b2_dynamicBody)fixtureDef.density = (float)density;
    else fixtureDef.density = 0.0f;

    // Override the default friction.
    fixtureDef.friction = (float)friction;

    // store this for contact information
    fixtureDef.userData = (void *) this;

    // Add the shape to the body.
    b2Fixture *fixturePtr = m_BodyPtr->CreateFixture(&fixtureDef);
    if (fixturePtr == nullptr)return false;
    ApplyGhost(fixturePtr);
    ApplyTrigger(fixturePtr);
    return true;
}

bool PhysicsActor::AddCircleShape(double radius, float2 offset, double restitution, double friction, double density)
{
	b2FixtureDef fixtureDef;
	radius /= SCALE;
	// The extents are the half-widths of the box.
	b2CircleShape circle;
	// position, relative to body position
	circle.m_p.Set((float)offset.x / SCALE, (float)offset.y / SCALE);
	// radius 
	circle.m_radius = (float)radius;

	// Define the dynamic body fixture.	
	fixtureDef.shape = &circle;

	fixtureDef.restitution = (float)restitution;
	// Set the box density to be non-zero, so it will be dynamic.
	if (m_BodyPtr->GetType() == b2_dynamicBody)fixtureDef.density = (float)density;
	else fixtureDef.density = 0.0f;

	// Override the default friction.
	fixtureDef.friction = (float)friction;

	// store this for contact information
	fixtureDef.userData = (void *) this;

	// Add the shape to the body.
	b2Fixture *fixturePtr = m_BodyPtr->CreateFixture(&fixtureDef);
	if (fixturePtr == nullptr)return false;
	ApplyGhost(fixturePtr);
	ApplyTrigger(fixturePtr);
	return true;
}

bool PhysicsActor::AddPolygonShape(const std::vector<float2>& vertexArrRef, double restitution, double friction, double density)
{
	b2FixtureDef fixtureDef;
	std::vector<b2Vec2> vecArr;
	// scale to phyics units
	for (size_t i = 0; i < vertexArrRef.size(); i++)
	{
		vecArr.push_back(b2Vec2((float)vertexArrRef[i].x / SCALE, (float)vertexArrRef[i].y / SCALE));
	}

	//pass array to the shape
	b2PolygonShape polygonShape;
	polygonShape.Set(vecArr.data(), int32(vecArr.size()));

	// Define the dynamic body fixture.	
	fixtureDef.shape = &polygonShape;

	fixtureDef.restitution = (float)restitution;
	// Set the box density to be non-zero, so it will be dynamic.
	if (m_BodyPtr->GetType() == b2_dynamicBody)fixtureDef.density = (float)density;
	else fixtureDef.density = 0.0f;

	// Override the default friction.
	fixtureDef.friction = (float)friction;

	// store this for contact information
	fixtureDef.userData = (void *) this;

	// Add the shape to the body.
	b2Fixture *fixturePtr = m_BodyPtr->CreateFixture(&fixtureDef);
	if (fixturePtr == nullptr)return false;
	ApplyGhost(fixturePtr);
	ApplyTrigger(fixturePtr);
	return true;
}

bool PhysicsActor::AddChainShape(const std::vector<float2>& vertexArrRef, bool closed, double restitution, double friction, double density)
{
	b2FixtureDef fixtureDef;
	std::vector<b2Vec2> vecArr;

	// seems that svg start vectex and end vertex are sometimes too close together or coincide, 
	// causing a crash in Box2D -> remove the last element
	// check the distance between begin and end points, omit the end point
	int omitLastVertex = 0;
	float2 v = vertexArrRef[0] - vertexArrRef[vertexArrRef.size() - 1];
	if ( float(hlslpp::sqrt(hlslpp::dot(v,v))) < 0.1)
	{
		omitLastVertex = 1;
	}

	for (size_t i = 0; i < (vertexArrRef.size() - omitLastVertex); i++)
	{
		vecArr.push_back(b2Vec2((float)vertexArrRef[i].x / SCALE, (float)vertexArrRef[i].y / SCALE));
	}

	//pass array to the shape
	b2ChainShape chainShape;
	if (closed)
	{
		chainShape.CreateLoop(vecArr.data(), int32(vecArr.size()));
	}
	else 
	{
		chainShape.CreateChain(vecArr.data(), int32(vecArr.size()), vecArr[0], vecArr[vecArr.size() - 1]);
	};

	// Define the dynamic body fixture.	
	fixtureDef.shape = &chainShape;

	fixtureDef.restitution = (float)restitution;
	// Set the box density to be non-zero, so it will be dynamic.
	if (m_BodyPtr->GetType() == b2_dynamicBody)fixtureDef.density = (float)density;
	else fixtureDef.density = 0.0f;

	// Override the default friction.
	fixtureDef.friction = (float)friction;

	// store this for contact information
	fixtureDef.userData = (void *) this;

	// Add the shape to the body.
	b2Fixture *fixturePtr = m_BodyPtr->CreateFixture(&fixtureDef);
	if (fixturePtr == nullptr)return false;

	ApplyGhost(fixturePtr);
	ApplyTrigger(fixturePtr);

	return true;
}

bool PhysicsActor::AddSVGShape(const String & svgFilePathRef, double restitution, double friction, double density)
{
	// a vector containing chains
	std::vector<std::vector<float2>> verticesArr;

	//parse the svg file
	SVGParser svgParser(svgFilePathRef, verticesArr);
	//svgParser.LoadGeometryFromSvgFile(svgFilePathRef, verticesArr);

	// process the chains
	for (size_t i = 0; i < verticesArr.size(); i++)
	{
		std::vector<float2> &chain = verticesArr[i];
		bool result = AddChainShape(chain, true, restitution, friction, density);
		if (!result) OutputDebugStringA("svg Chain creation failed");
	}
	return true;
}

float2 PhysicsActor::GetPosition() {
	b2Vec2 position = m_BodyPtr->GetPosition();
	return float2(position.x, position.y) * SCALE;
}


void PhysicsActor::SetPosition(float2 const& positionRef) {
	m_BodyPtr->SetTransform(b2Vec2((float)(positionRef.x / SCALE), (float)(positionRef.y / SCALE)), m_BodyPtr->GetAngle());
	m_BodyPtr->SetAwake(true);
}

double PhysicsActor::GetAngle()
{
	float angle = m_BodyPtr->GetAngle(); 
	return angle;
}

void PhysicsActor::SetAngle(double angle)
{
	m_BodyPtr->SetTransform(m_BodyPtr->GetPosition(), (float)angle);
	m_BodyPtr->SetAwake(true);
}

void PhysicsActor::SetLinearVelocity(float2 const& velocity) {
	float2 vel = velocity / SCALE;
	m_BodyPtr->SetLinearVelocity(b2Vec2((float)velocity.x, (float)velocity.y));
	m_BodyPtr->SetAwake(true);
}


float2  PhysicsActor::GetLinearVelocity()
{
	b2Vec2 v = m_BodyPtr->GetLinearVelocity();
	return float2(v.x, v.y) * SCALE;
}

void PhysicsActor::SetAngularVelocity(double velocity)
{
 	m_BodyPtr->SetAngularVelocity((float)velocity);
	m_BodyPtr->SetAwake(true);
}

double  PhysicsActor::GetAngularVelocity()
{
	return m_BodyPtr->GetAngularVelocity(); 
}

double PhysicsActor::GetMass()
{
	return m_BodyPtr->GetMass() * SCALE * SCALE;
}

void PhysicsActor::SetFixedRotation(bool fixedRotation)
{
	m_BodyPtr->SetFixedRotation(fixedRotation);
}

bool PhysicsActor::IsFixedRotation()
{
	return m_BodyPtr->IsFixedRotation();
}

void PhysicsActor::SetBullet(bool bullet)
{
	m_BodyPtr->SetBullet(bullet);
}

void PhysicsActor::SetActive(bool active)
{
	m_BodyPtr->SetEnabled(active);
}

void PhysicsActor::SetGravityScale(double scale)
{
	m_BodyPtr->SetGravityScale((float)scale);
}

bool PhysicsActor::Raycast(float2 point1, float2 point2, float2& intersectionRef, float2& normalRef, double& fractionRef) {
	point1 /= SCALE;
	point2 /= SCALE;
	b2Transform transform;
	//transform.SetIdentity();
	transform.Set(m_BodyPtr->GetPosition(), m_BodyPtr->GetAngle());
	b2RayCastInput input;
	input.p1.Set((float)point1.x, (float)point1.y);
	input.p2.Set((float)point2.x, (float)point2.y);
	input.maxFraction = 1.0f;
	b2RayCastOutput output, closestOutput;
	closestOutput.fraction = 1; //start with end of line as p2
	//check every fixture of every body to find closest 
	for (b2Fixture* fixturePtr = m_BodyPtr->GetFixtureList(); fixturePtr != nullptr; fixturePtr = fixturePtr->GetNext())
	{
		//A child index is included for chain shapes because the ray cast will only	check a single edge at a time.
		for (int edgeIndex = 0; edgeIndex < fixturePtr->GetShape()->GetChildCount(); ++edgeIndex)
		{
			bool hit = fixturePtr->GetShape()->RayCast(&output, input, transform, edgeIndex);
			if (hit)
			{
				if (output.fraction < closestOutput.fraction)
					closestOutput = output;
			}
		}
	}
	if (closestOutput.fraction < 1)
	{
		b2Vec2 hitPoint = input.p1 + closestOutput.fraction * (input.p2 - input.p1);
		intersectionRef = float2(hitPoint.x, hitPoint.y) * SCALE;
		normalRef = float2(output.normal.x, output.normal.y);
		fractionRef = output.fraction;
		return true;
	}
	// no intersection
	return false;
}

void PhysicsActor::ApplyForce(float2 force, float2 offsetPoint)
{
	force /= (SCALE * SCALE);
	if (offsetPoint.x == 0 && offsetPoint.y == 0)
	{
		m_BodyPtr->ApplyForceToCenter(b2Vec2((float)force.x, (float)force.y), true);
	}
	else
	{
		b2Vec2 p = m_BodyPtr->GetWorldPoint(b2Vec2((float)offsetPoint.x / SCALE, (float)offsetPoint.y / SCALE));
		m_BodyPtr->ApplyForce(b2Vec2((float)force.x, (float)force.y), p, true);
	}
}

void PhysicsActor::ApplyTorque(double torque)
{
	torque /= (SCALE * SCALE);
	m_BodyPtr->ApplyTorque((float)torque, true);
}

void PhysicsActor::ApplyLinearImpulse(float2 impulse, float2 offsetPoint)
{
	impulse /= (SCALE * SCALE);
	b2Vec2 p = m_BodyPtr->GetWorldPoint(b2Vec2((float)offsetPoint.x / SCALE, (float)offsetPoint.y / SCALE));
	m_BodyPtr->ApplyLinearImpulse(b2Vec2((float)impulse.x, (float)impulse.y), p, true);
}

void PhysicsActor::ApplyAngularImpulse(double impulse)
{
	impulse /= (SCALE * SCALE);
	m_BodyPtr->ApplyAngularImpulse((float)impulse, true);
}
void PhysicsActor::SetFriction(double friction)
{
    double tmpFriction = friction;
    
    for (b2Fixture* fixturePtr = m_BodyPtr->GetFixtureList(); fixturePtr != nullptr; fixturePtr = fixturePtr->GetNext())
    {
        if (friction > 0)
        {
            fixturePtr->SetFriction((float)friction);
        }
        
    }
}
double PhysicsActor::GetFriction()
{
    double friction = 0;
    for (b2Fixture* fixturePtr = m_BodyPtr->GetFixtureList(); fixturePtr != nullptr; fixturePtr = fixturePtr->GetNext())
    {
        friction = fixturePtr->GetFriction();
    }
    return friction;
}
void PhysicsActor::SetTrigger(bool trigger)
{
	m_bTrigger = trigger;
	for (b2Fixture* fixturePtr = m_BodyPtr->GetFixtureList(); fixturePtr != nullptr; fixturePtr = fixturePtr->GetNext())
	{
		ApplyTrigger(fixturePtr);
	}
}

void PhysicsActor::ApplyTrigger(b2Fixture * fixturePtr)
{
	fixturePtr->SetSensor(m_bTrigger);
}

void PhysicsActor::SetGhost(bool ghost)
{
	m_bGhost = ghost;

	// apply the setting to all fixtures
	for (b2Fixture* fixturePtr = m_BodyPtr->GetFixtureList(); fixturePtr != nullptr; fixturePtr = fixturePtr->GetNext())
	{
		ApplyGhost(fixturePtr);
	}
}

void PhysicsActor::ApplyGhost(b2Fixture * fixturePtr)
{
	if (m_bGhost)
	{
		b2Filter filter;
		filter.maskBits = 0;
		fixturePtr->SetFilterData(filter);
	}
	else
	{
		b2Filter filter;
		filter.maskBits = 0xFFFF;
		fixturePtr->SetFilterData(filter);
	}
}

bool PhysicsActor::IsPointInActor(const float2 &pointRef)
{
	bool hit = false;
	for (b2Fixture* fixturePtr = m_BodyPtr->GetFixtureList(); fixturePtr != nullptr && !hit; fixturePtr = fixturePtr->GetNext())
	{
		hit = fixturePtr->TestPoint(b2Vec2((float)pointRef.x / SCALE, (float)pointRef.y / SCALE));

	}
	return hit;
}

bool PhysicsActor::IsPointInActor(const POINT &pointRef)
{
	bool hit = false;
	for (b2Fixture* fixturePtr = m_BodyPtr->GetFixtureList(); fixturePtr != nullptr && !hit; fixturePtr = fixturePtr->GetNext())
	{
		hit = fixturePtr->TestPoint(b2Vec2((float)pointRef.x / SCALE, (float)pointRef.y / SCALE));

	}
	return hit;
}

bool PhysicsActor::IsOverlapping(PhysicsActor* otherActor)
{
	b2Transform transform;
	transform.Set(m_BodyPtr->GetPosition(), m_BodyPtr->GetAngle());
	b2Transform otherTransform;
	otherTransform.Set(otherActor->m_BodyPtr->GetPosition(), otherActor->m_BodyPtr->GetAngle());

	for (b2Fixture* fixturePtr = m_BodyPtr->GetFixtureList(); fixturePtr != nullptr; fixturePtr = fixturePtr->GetNext())
	{
		for (int edgeIndex = 0; edgeIndex < fixturePtr->GetShape()->GetChildCount(); ++edgeIndex)
		{
			for (b2Fixture* otherfixturePtr = otherActor->m_BodyPtr->GetFixtureList(); otherfixturePtr != nullptr; otherfixturePtr = otherfixturePtr->GetNext())
			{
				for (int otherEdgeIndex = 0; otherEdgeIndex < otherfixturePtr->GetShape()->GetChildCount(); ++otherEdgeIndex)
				{
					if (b2TestOverlap(fixturePtr->GetShape(), edgeIndex, otherfixturePtr->GetShape(), otherEdgeIndex, transform, otherTransform))
					{
						return true;
					}
				}
			}
		}
	}
	return false;
}

std::vector<float2> PhysicsActor::GetContactList()
{
	std :: vector<float2> contactPoints;
	for (b2ContactEdge* edgePtr = m_BodyPtr->GetContactList(); edgePtr; edgePtr = edgePtr->next)
	{
		if (edgePtr->contact->IsTouching())
		{
			//...world manifold is helpful for getting locations
			b2WorldManifold worldManifold;
			edgePtr->contact->GetWorldManifold(&worldManifold);

			int numPoints = edgePtr->contact->GetManifold()->pointCount;

			for (int i = 0; i < numPoints; i++)
			{
				contactPoints.push_back(float2(worldManifold.points[i].x * SCALE, worldManifold.points[i].y * SCALE));
			}
		}
	}
	return contactPoints;
}

void PhysicsActor::AddContactListener(ContactListener *listenerPtr)
{
	//store the pointer in userdata to be used by the ContactCaller
	m_BodyPtr->SetUserData((void*)listenerPtr);
}

void PhysicsActor::RemoveContactListener(ContactListener *listenerPtr)
{
	//reset the pointer in userdata to be used by the ContactCaller
	m_BodyPtr->SetUserData((void*)nullptr);
}

ContactListener *PhysicsActor::GetContactListener()
{
	return reinterpret_cast <ContactListener *>(m_BodyPtr->GetUserData());
}

void PhysicsActor::SetCollisionFilter(const b2Filter &filterRef)
{
	for (b2Fixture* fixturePtr = m_BodyPtr->GetFixtureList(); fixturePtr != nullptr; fixturePtr = fixturePtr->GetNext())
	{
		fixturePtr->SetFilterData(filterRef);
	}
}

b2Filter PhysicsActor::GetCollisionFilter()
{
	return m_BodyPtr->GetFixtureList()->GetFilterData();
}