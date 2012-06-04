#include "FenwarBoss.h"
#include "Player.h"
#include "CollisionCircle.h"

FenwarBullets::FenwarBullets(FenwarBoss *_fenwar)
{
	fenwar = _fenwar;
}

FenwarBullets::~FenwarBullets()
{
	for (std::list<FenwarBullet>::iterator i = bulletList.begin(); i != bulletList.end(); i++)
	{
	}
	bulletList.end();
}

//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~
// Public Methods
//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~

void FenwarBullets::update(float dt)
{
	for (std::list<FenwarBullet>::iterator i = bulletList.begin(); i != bulletList.end(); i++)
	{
		i->x += i->dx * dt;
		i->y += i->dy * dt;

		i->collisionBox->SetRadius(i->x, i->y, 10.0);

		if (smh->player->collisionCircle->testBox(i->collisionBox))
		{
			smh->player->dealDamage(FenwarAttributes::BULLET_DAMAGE, true);
		}

		if (Util::distance(i->x, i->y, smh->player->x, smh->player->y) > 2000.0)
		{
			i = bulletList.erase(i);
			continue;
		}

		if (smh->getGameTime() > i->nextSplitTime && i->n < 3)
		{
			int numNewBullets = 5 - i->n;
			float angle = smh->randomFloat(0.0, 2.0 * PI);
			for (int k = 0; k < numNewBullets; k++)
			{
				spawnBullet(i->x, i->y, angle + ((2.0*PI)/numNewBullets)*k, i->n + 1);
			}

			i = bulletList.erase(i);

			//Play a sound
			smh->soundManager->playSound("snd_FenwarYellowDotSplit");
			continue;
		}
	}
}

void FenwarBullets::draw(float dt)
{
	for (std::list<FenwarBullet>::iterator i = bulletList.begin(); i != bulletList.end(); i++)
	{
		float percentToNextSplit = (smh->getGameTime() - i->timeOfLastSplit) / (i->nextSplitTime - i->timeOfLastSplit);
		if (percentToNextSplit <= 0.80 || i->n >= 3) {
			smh->resources->GetSprite("fenwarBullet")->Render(smh->getScreenX(i->x), smh->getScreenY(i->y));
		}
		else {
			float subtractFromGreen=255.0*(percentToNextSplit - 0.80) * 5.0;
			float green=255.0-subtractFromGreen;
			if (green < 0.0) green=0.0;
			if (green > 255.0) green=255.0;
			
			smh->resources->GetSprite("fenwarBullet")->SetColor(ARGB(255.0, 255.0, green, 255.0));
			smh->resources->GetSprite("fenwarBullet")->Render(smh->getScreenX(i->x), smh->getScreenY(i->y));
			smh->resources->GetSprite("fenwarBullet")->SetColor(ARGB(255.0,255.0,255.0,255.0));
		}

		

		if (smh->isDebugOn())
		{
			smh->drawCollisionBox(i->collisionBox, Colors::RED);
		}
	}
}

void FenwarBullets::shootBullet(float angle)
{
	spawnBullet(fenwar->x, fenwar->y, angle, 0);
}

void FenwarBullets::killBullets()
{
	for (std::list<FenwarBullet>::iterator i = bulletList.begin(); i != bulletList.end(); i++)
	{
		i=bulletList.erase(i);
	}
	bulletList.end();
}

//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~
// Helper Methods
//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~

void FenwarBullets::spawnBullet(float x, float y, float angle, int n)
{
	FenwarBullet bullet;

	bullet.x = x;
	bullet.y = y;
	bullet.dx = FenwarAttributes::BULLET_SPEED * cos(angle);
	bullet.dy = FenwarAttributes::BULLET_SPEED * sin(angle);
	bullet.n = n;
	bullet.collisionBox = new hgeRect();
	bullet.collisionBox->SetRadius(x, y, 10.0);
	bullet.nextSplitTime = smh->getGameTime() + smh->randomFloat(1.0, 1.5);
	bullet.timeOfLastSplit = smh->getGameTime();
	
	bulletList.push_back(bullet);
}