#pragma once

#include "BroadPhase.h"

#include "Polygon.h"
#include "GlobalVariables.h"
#include "World.h"

#include <algorithm>

// My custom broadPhase SAP
class CBroadPhaseSAP : public IBroadPhase
{
public:
	virtual void GetCollidingPairsToCheck(std::vector<SPolygonPair>& pairsToCheck) override
	{

		std::vector<CPolygonPtr> poly;
		poly.reserve(gVars->pWorld->GetPolygonCount());

		// Add to list, and rebuild aabb
		for (size_t i = 0; i < gVars->pWorld->GetPolygonCount(); i++)
		{
			poly.push_back(gVars->pWorld->GetPolygon(i));
			poly[i]->boxAABB.isCollide = false;

			std::vector<Vec2> newPoints;

			for (Vec2 point : poly[i]->points)
				newPoints.push_back(poly[i]->rotation * point);

			poly[i]->boxAABB.Build(newPoints, poly[i]->position);

		}


		// Sort by min x to max x the list of poly
		std::sort(poly.begin(), poly.end(), [](CPolygonPtr& A, CPolygonPtr& B)
			{ return  A->GetWolrdMinAABB().x < B->GetWolrdMinAABB().x; });

		for (size_t i = 0; i < gVars->pWorld->GetPolygonCount() ; i++)
		{

			for (size_t j = i + 1; j < gVars->pWorld->GetPolygonCount() ; j++)
			{	
				if (poly[i]->GetWolrdMaxAABB().x < poly[j]->GetWolrdMinAABB().x)
					break;
			
				if (poly[i]->boxAABB.isColliding(poly[j]->boxAABB, poly[j]->position))
				{
					pairsToCheck.push_back(SPolygonPair(poly[i], poly[j]));

					poly[i]->boxAABB.isCollide = true;
					poly[j]->boxAABB.isCollide = true;
				}

			}
		}
	}
};
