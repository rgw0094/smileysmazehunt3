#include "FenwarBoss.h"
#include "ExplosionManager.h"

FenwarBombs::FenwarBombs(FenwarBoss *_fenwar)
{
	fenwar = _fenwar;
}

FenwarBombs::~FenwarBombs()
{
}

void FenwarBombs::throwBomb()
{
	FenwarBomb bomb;
	bomb.x = fenwar->x;
	bomb.y = fenwar->y;
	bomb.timeToTravel = smh->randomFloat(1.0, 1.5);
	bomb.timeLaunched = smh->getGameTime();
	bomb.yOffset = 0.0;

	//Choose a random platform to which to throw
	int r = smh->randomInt(1, 8);
	float targetX = fenwar->platformLocations[r].x * 64.0 + 32.0;
	float targetY = fenwar->platformLocations[r].y * 64.0 + 32.0;

	//Calculate shit to set up the bomb's path
	float angle = Util::getAngleBetween(targetX, targetY, bomb.x, bomb.y);
	float distance = Util::distance(bomb.x, bomb.y, targetX, targetY);
	float speed = distance / bomb.timeToTravel;

	bomb.dx = speed * cos(angle);
	bomb.dy = speed * sin(angle);

	bombList.push_back(bomb);
}


void FenwarBombs::update(float dt)
{
	for (std::list<FenwarBomb>::iterator i = bombList.begin(); i != bombList.end(); i++)
	{	
		if (smh->timePassedSince(i->timeLaunched) > i->timeToTravel)
		{
			i->yOffset = 0.0;

			//The fuse has run out!!!!!
			if (smh->timePassedSince(i->timeLaunched) > i->timeToTravel + 1.5)
			{
				smh->explosionManager->addExplosion(i->x, i->y, 1.0, FenwarAttributes::BOMB_DAMAGE, 200.0);
				i = bombList.erase(i);
				continue;
			}
		}
		else
		{
			i->x += i->dx * dt;
			i->y += i->dy * dt;
			i->yOffset = 175.0 * sin(smh->timePassedSince(i->timeLaunched) / i->timeToTravel * PI);
		}
	}
}

void FenwarBombs::draw(float dt)
{
	for (std::list<FenwarBomb>::iterator i = bombList.begin(); i != bombList.end(); i++)
	{
		smh->drawGlobalSprite("fenwarBomb", i->x, i->y - i->yOffset);

		smh->resources->GetSprite("playerShadow")->RenderEx(i->x, i->y, 0.0, 0.75, 0.75);
	}
}



