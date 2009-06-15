#include "SmileyEngine.h"
#include "CollisionCircle.h"
#include "FenwarBoss.h"
#include "Player.h"
#include "ProjectileManager.h"

#define RING_RADIUS 150.0
#define ORB_RADIUS 30.0

FenwarOrbs::FenwarOrbs(FenwarBoss *_fenwar)
{
	fenwar = _fenwar;
}

FenwarOrbs::~FenwarOrbs()
{
	for (std::list<FenwarOrb>::iterator i = orbList.begin(); i != orbList.end(); i++)
	{
		delete i->collisionCircle;
	}
	orbList.clear();
}

//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~
// Public Methods
//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~

void FenwarOrbs::update(float dt)
{
	updateState(dt);

	int count = -1;
	for (std::list<FenwarOrb>::iterator i = orbList.begin(); i != orbList.end(); i++) 
	{
		count++;

		float angle = angleOffset + count * ((2.0*PI)/FenwarAttributes::ORB_COUNT);
		i->x = fenwar->x + cos(angle) * distFromFenwar;
		i->y = fenwar->y + sin(angle) * distFromFenwar;

		i->collisionCircle->set(i->x, i->y, ORB_RADIUS * scale);

		//Player collision
		if (smh->player->collisionCircle->testCircle(i->collisionCircle))
		{
			smh->player->dealDamageAndKnockback(FenwarAttributes::ORB_COLLISION_DAMAGE, true, false, FenwarAttributes::ORB_COLLISION_KNOCKBACK, i->x, i->y);
		}

		//Frisbee collision
		smh->projectileManager->killProjectilesInCircle(i->x, i->y, ORB_RADIUS * scale, PROJECTILE_FRISBEE);

		//Lightning orb collision
		if (smh->projectileManager->killProjectilesInCircle(i->x, i->y, ORB_RADIUS * scale, PROJECTILE_LIGHTNING_ORB))
		{
			//If this is a red orb it can be damaged
			if (i->isRedOrb)
			{
				i->health -= smh->player->getLightningOrbDamage();
				if (i->health <= 0.0)
				{
					delete i->collisionCircle;
					i = orbList.erase(i);
					continue;
				}
			} 
			else
			{
				smh->soundManager->playSound("snd_HitInvulnerable");
			}
		}
	}
}

void FenwarOrbs::updateState(float dt)
{
	timeInState += dt;

	if (state == OrbStates::EXPANDING)
	{
		scale += 0.33333 * dt;
		distFromFenwar += 50.0 * dt;
		if (distFromFenwar >= RING_RADIUS)
		{
			enterState(OrbStates::SPEEDING_UP);
		}
	}
	else if (state == OrbStates::SPEEDING_UP)
	{
		angularSpeed += FenwarAttributes::ORB_SPIN_SPEED * dt;
		angleOffset += angularSpeed * dt;
		if (angularSpeed >= FenwarAttributes::ORB_SPIN_SPEED)
		{
			angularSpeed = FenwarAttributes::ORB_SPIN_SPEED;
			enterState(OrbStates::SPINNING);
		}
	}
	else if (state == OrbStates::SLOWING_DOWN)
	{
		angularSpeed -= FenwarAttributes::ORB_SPIN_SPEED * dt;
		angleOffset += angularSpeed * dt;
		if (angularSpeed < 0.0)
		{
			angularSpeed = 0.0;
			enterState(OrbStates::ATTACKING);
		}
	}
	else if (state == OrbStates::ATTACKING)
	{
		distFromFenwar = RING_RADIUS + 600.0 * sin(timeInState/FenwarAttributes::ORB_ATTACK_DURATION * PI);
		if (timeInState > FenwarAttributes::ORB_ATTACK_DURATION)
		{
			enterState(OrbStates::SPEEDING_UP);
		}
	} 
	else if (state == OrbStates::SPINNING)
	{
		angleOffset += angularSpeed * dt;
	}

	//Equilibrium values
	if (state != OrbStates::EXPANDING)
	{
		scale = 1.0;
	}
	if (state != OrbStates::ATTACKING && state != OrbStates::EXPANDING)
	{
		distFromFenwar = RING_RADIUS;
	}
}

void FenwarOrbs::drawBeforeFenwar(float dt)
{
	for (std::list<FenwarOrb>::iterator i = orbList.begin(); i != orbList.end(); i++) 
	{
		if (i->y < fenwar->y)
		{
			drawOrb(i);
		}
	}
}

void FenwarOrbs::drawAfterFenwar(float dt)
{
	for (std::list<FenwarOrb>::iterator i = orbList.begin(); i != orbList.end(); i++) 
	{
		if (i->y >= fenwar->y)
		{
			drawOrb(i);
		}
	}
}

void FenwarOrbs::drawOrb(std::list<FenwarOrb>::iterator orb)
{
	if (orb->isRedOrb)
		{
			smh->resources->GetSprite("redFenwarOrb")->RenderEx(smh->getScreenX(orb->x), smh->getScreenY(orb->y), 0.0, scale, scale);
		} 
		else
		{
			smh->resources->GetSprite("normalFenwarOrb")->RenderEx(smh->getScreenX(orb->x), smh->getScreenY(orb->y), 0.0, scale, scale);
		}

		if (smh->isDebugOn())
		{
			orb->collisionCircle->draw();
		}
}

void FenwarOrbs::killOrbs()
{
	for (std::list<FenwarOrb>::iterator i = orbList.begin(); i != orbList.end(); i++)
	{
		delete i->collisionCircle;
	}
	orbList.clear();
}

void FenwarOrbs::spawnOrbs()
{
	float angleOffset = 0;
	for (int i = 0; i < FenwarAttributes::ORB_COUNT; i++) 
	{
		spawnOrb(i % 4 == 0);
	}

	distFromFenwar = 0.0;
	scale = 0.0;
	angularSpeed = 0.0;
	angleOffset = smh->randomFloat(0.0, 2.0 * PI);

	enterState(OrbStates::EXPANDING);
}

int FenwarOrbs::getState()
{
	return state;
}

void FenwarOrbs::doAttack()
{
	enterState(OrbStates::SLOWING_DOWN);
}

//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~
// Helper Methods
//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~

void FenwarOrbs::enterState(int newState)
{
	state = newState;
	timeInState = 0.0;
}

void FenwarOrbs::spawnOrb(bool isRedOrb)
{
	FenwarOrb newOrb;

	newOrb.isRedOrb = isRedOrb;
	newOrb.collisionCircle = new CollisionCircle();
	newOrb.health = FenwarAttributes::ORB_HEALTH;

	orbList.push_back(newOrb);
}