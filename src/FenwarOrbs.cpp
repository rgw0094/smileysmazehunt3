#include "FenwarBoss.h"

FenwarOrbs::FenwarOrbs(FenwarBoss *_fenwar)
{
	fenwar = _fenwar;
}

FenwarOrbs::~FenwarOrbs()
{
}

//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~
// Public Methods
//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~

void FenwarOrbs::update(float dt)
{
	for (std::list<FenwarOrb>::iterator i = orbList.begin(); i != orbList.end(); i++) 
	{
		i->angle += PI * dt;

		i->x = fenwar->x + cos(i->angle) * i->distFromFenwar;
		i->y = fenwar->y + sin(i->angle) * i->distFromFenwar;
	}
}

void FenwarOrbs::draw(float dt)
{
	for (std::list<FenwarOrb>::iterator i = orbList.begin(); i != orbList.end(); i++) 
	{
		smh->resources->GetSprite("fenwarOrb")->Render(smh->getScreenX(i->x), smh->getScreenY(i->y));
	}
}

void FenwarOrbs::spawnOrbs()
{
	float angle = 0;
	for (int i = 0; i < 8; i++) 
	{
		spawnOrb(50.0, angle, false);
		angle += PI / 4.0;
	}
}

//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~
// Helper Methods
//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~

void FenwarOrbs::spawnOrb(float distFromFenwar, float angle, bool isRedOrb)
{
	FenwarOrb newOrb;

	newOrb.distFromFenwar = distFromFenwar;
	newOrb.angle = angle;
	newOrb.isRedOrb = isRedOrb;
	newOrb.radius = 60.0;

	orbList.push_back(newOrb);
}