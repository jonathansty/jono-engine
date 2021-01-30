#include "engine.stdafx.h"
#include "GameEngine.h"

#include "PhysicsDistanceJoint.h"
#include "PhysicsActor.h"

PhysicsDistanceJoint::PhysicsDistanceJoint(PhysicsActor *actAPtr, float2 anchorA, PhysicsActor *actBPtr, float2 anchorB, double length, double dampingRatio)
{
	b2DistanceJointDef DistanceJointDef;
	DistanceJointDef.bodyA = actAPtr->m_BodyPtr;
	DistanceJointDef.bodyB = actBPtr->m_BodyPtr;
	DistanceJointDef.localAnchorA = b2Vec2((float)anchorA.x / (float)PhysicsActor::SCALE, (float)anchorA.y / (float)PhysicsActor::SCALE);
	DistanceJointDef.localAnchorB = b2Vec2((float)anchorB.x / (float)PhysicsActor::SCALE, (float)anchorB.y / (float)PhysicsActor::SCALE);
	DistanceJointDef.length = (float)length / (float)PhysicsActor::SCALE;

	// #TODO: Add stiffness
	DistanceJointDef.damping = (float)dampingRatio;

	m_DistanceJointPtr = reinterpret_cast<b2DistanceJoint*>((GameEngine::instance())->GetBox2DWorld()->CreateJoint(&DistanceJointDef));

}

PhysicsDistanceJoint::~PhysicsDistanceJoint()
{
	(GameEngine::instance())->GetBox2DWorld()->DestroyJoint(m_DistanceJointPtr);
}

double PhysicsDistanceJoint::GetLength() const
{
	return m_DistanceJointPtr->GetLength() * (float)PhysicsActor::SCALE;
}

void PhysicsDistanceJoint::SetLength(double length)
{
	if (length < 0.1)
	{
		GameEngine::instance()->message_box(String("Length can not be a very small number "));
	}
	m_DistanceJointPtr->SetLength((float)length / (float)PhysicsActor::SCALE);
}


double PhysicsDistanceJoint::GetDampingRatio() const
{
	return m_DistanceJointPtr->GetDamping();
}

void PhysicsDistanceJoint::SetDampingRatio(double dampingRatio)
{
	m_DistanceJointPtr->SetDamping((float)dampingRatio);
}

float2 PhysicsDistanceJoint::GetReactionForce(double deltaTime) const
{
	b2Vec2 vec2 = m_DistanceJointPtr->GetReactionForce(1/(float)deltaTime);
	return float2(vec2.x, vec2.y);
}

double PhysicsDistanceJoint::GetReactionTorque(double deltaTime) const
{
	return m_DistanceJointPtr->GetReactionTorque(1 / (float)deltaTime);
}
