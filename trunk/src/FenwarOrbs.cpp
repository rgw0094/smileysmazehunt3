#include "FenwarBoss.h"

FenwarOrbs::FenwarOrbs(FenwarBoss *_fenwar)
{
	fenwar = _fenwar;
}

FenwarOrbs::~FenwarOrbs()
{
}

void FenwarOrbs::update(float dt)
{
	for (std::list<FenwarOrb>::iterator i = orbList.begin(); i != orbList.end(); i++) 
	{
	}
}

void FenwarOrbs::draw(float dt)
{
	for (std::list<FenwarOrb>::iterator i = orbList.begin(); i != orbList.end(); i++) 
	{
	}
}

void FenwarOrbs::spawnOrb(float distFromFenwar, float angle, bool isRedOrb)
{
	FenwarOrb newOrb;

	newOrb.distFromFenwar = distFromFenwar;
	newOrb.angle = angle;
	newOrb.isRedOrb = isRedOrb;
	newOrb.radius = 60.0;

	orbList.push_back(newOrb);
}