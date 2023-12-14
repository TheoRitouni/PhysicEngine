#include "PhysicEngine.h"

#include <iostream>
#include <string>
#include "GlobalVariables.h"
#include "World.h"
#include "Renderer.h" // for debugging only
#include "Timer.h"

#include "BroadPhase.h"
#include "BroadPhaseBrut.h"
#include "BroadPhaseSAP.h"


void	CPhysicEngine::Reset()
{
	m_pairsToCheck.clear();
	m_collidingPairs.clear();

	m_active = true;

	m_broadPhase = new CBroadPhaseSAP();

}

void	CPhysicEngine::Activate(bool active)
{
	m_active = active;
}

void	CPhysicEngine::DetectCollisions()
{
	CTimer timer;
	timer.Start();
	CollisionBroadPhase();
	timer.Stop();
	if (gVars->bDebug)
	{
		gVars->pRenderer->DisplayText("Collision broadphase duration " + std::to_string(timer.GetDuration() * 1000.0f) + " ms");
	}

	timer.Start();
	CollisionNarrowPhase();
	timer.Stop();
	if (gVars->bDebug)
	{
		gVars->pRenderer->DisplayText("Collision narrowphase duration " + std::to_string(timer.GetDuration() * 1000.0f) + " ms, collisions : " + std::to_string(m_collidingPairs.size()));
	}
}

void	CPhysicEngine::Step(float deltaTime)
{
	deltaTime = Min(deltaTime, 1.0f / 15.0f);

	if (!m_active)
		return;

	DetectCollisions();
	ResponseCollisions(deltaTime);
}


void	CPhysicEngine::CollisionBroadPhase()
{
	if (m_pairsToCheck.size() != 0)
	{
		for (const SPolygonPair& pair : m_pairsToCheck)
		{
			pair.polyA->isCollide = false;
			pair.polyB->isCollide = false;
		}
	}

	m_pairsToCheck.clear();
	m_broadPhase->GetCollidingPairsToCheck(m_pairsToCheck);
}

void	CPhysicEngine::CollisionNarrowPhase()
{
	if (gVars->bDebug)
	{
		gVars->pRenderer->DrawLine(Vec2(-0.5, 0), Vec2(0.5, 0), 1.0, 0.0, 0);
		gVars->pRenderer->DrawLine(Vec2(0, -0.5), Vec2(0, 0.5), 0.0, 1.0, 0);
	}

	m_collidingPairs.clear();
	for (const SPolygonPair& pair : m_pairsToCheck)
	{
		SCollision collision;
		collision.polyA = pair.polyA;
		collision.polyB = pair.polyB;

		if (pair.polyA->CheckCollision(*(pair.polyB), collision.point, collision.normal, collision.distance)) 
		{
			pair.polyA->isCollide = true;
			pair.polyB->isCollide = true;

			m_collidingPairs.push_back(collision);
		}
		
	}
}

void CPhysicEngine::ResponseCollisions(float deltaTime)
{

	Vec2 gravity(0, -9.8f);

	float frictionFactor = 0.6f;
	float elasticity = 0.1f;
	float damping = 0.2f;

	for (const SCollision& pair : m_collidingPairs)
	{
		float invMassA = pair.polyA->density == 0 ? 0.0f : 1.0f / (pair.polyA->density );
		float invMassB = pair.polyB->density == 0 ? 0.0f : 1.0f / (pair.polyB->density );
		float invMassAB = invMassA + invMassB;

		// rotation utility
		Vec2 rA = pair.point - pair.polyA->position, 
			rB = pair.point - pair.polyB->position;
		
		Vec2 vAi = pair.polyA->speed + Vec2::Cross(pair.polyA->angularVelocity, rA), 
			vBi = pair.polyB->speed + Vec2::Cross(pair.polyB->angularVelocity, rB);
		
		float invTensorA = 1.0f / pair.polyA->GetInertiaTensor(),
			invTensorB = 1.0f / pair.polyB->GetInertiaTensor();

		float momentumA = invTensorA * (rA ^ pair.normal),
			momentumB = invTensorB * (rB ^ pair.normal);
		
		float weightRotA = (Vec2::Cross(momentumA, rA) | pair.normal), 
			weightRotB = (Vec2::Cross(momentumB, rB) | pair.normal);


		Vec2 relSpeed = (pair.polyB->speed - pair.polyA->speed);

		//impulsion 
		float impulse = 0.0f;
		{
			float relSpeedNorm = relSpeed | (-pair.normal);

			if (relSpeedNorm > 0)
			{
				float elasticFactor = -(elasticity + 1.0f);

				bool rotationOn = true;

				impulse = (elasticFactor * relSpeedNorm) / (invMassAB);

				if(rotationOn)
					impulse = (elasticFactor * relSpeedNorm) / (invMassAB+ weightRotA + weightRotB);

				pair.polyA->speed += (pair.normal) * impulse * invMassA;
				pair.polyB->speed -= (pair.normal) * impulse * invMassB;

				if (rotationOn)
				{
					pair.polyA->angularVelocity +=  impulse * momentumA;
					pair.polyB->angularVelocity -=  impulse * momentumB;
				}
				
			}
		}

		//position correction 
		{
			{
				float correction = (pair.distance * damping) / (invMassAB);

				pair.polyA->position -= pair.normal * invMassA * correction;
				pair.polyB->position += pair.normal * invMassB * correction;
			}
		}

		// friction 
		if (relSpeed.GetSqrLength() > 0.0f)
		{
			Vec2 tangent = ((pair.normal) + relSpeed.Normalized());

			if (tangent.GetSqrLength() != 0.0f)
			{
				float friction = Clamp(-(relSpeed | tangent.Normalized()) / invMassAB, -std::abs(impulse) * frictionFactor, std::abs(impulse) * frictionFactor);
				pair.polyA->speed -= tangent * friction * invMassA;
				pair.polyB->speed += tangent * friction * invMassB;
			}

		}
	}


	gVars->pWorld->ForEachPolygon([&](CPolygonPtr poly)
	{
		if (poly->density == 0.0f)
			return;

		poly->rotation.Rotate(RAD2DEG(poly->angularVelocity * deltaTime));
		poly->position += poly->speed * deltaTime;
		poly->speed += gravity * deltaTime;
	});

}
