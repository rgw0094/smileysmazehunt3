#include "SmileyEngine.h"
#include "environment.h"
#include "ProjectileManager.h"
#include "player.h"
#include "hgeresource.h"
#include "EnemyFramework.h"
#include "CollisionCircle.h"

extern SMH *smh;

#define LASER_LENGTH 20.0
#define MINI_MUSHROOM_ENEMYID 43
#define MUMMY_ENEMYID 64

ProjectileManager::ProjectileManager() {

	initProjectiles();

	//Set up which collision types projectiles can go through
	for (int i = 0; i < 256; i++) canPass[i] = !Util::isCylinderUp(i);
	canPass[UNWALKABLE] = false;
	canPass[SIGN] = false;
	canPass[FOUNTAIN] = false;
	canPass[SAVE_SHRINE] = false;
	canPass[RED_KEYHOLE] = false;
	canPass[YELLOW_KEYHOLE] = false;
	canPass[GREEN_KEYHOLE] = false;
	canPass[BLUE_KEYHOLE] = false;
	canPass[EVIL_DOOR] = false;
	canPass[BOMBABLE_WALL] = false;
	canPass[FIRE_DESTROY] = false;
	canPass[BOMBABLE_WALL] = false;
	canPass[FLAME] = false;

}

ProjectileManager::~ProjectileManager() {

}


/**
 * Add a projectile to the manager
 */
void ProjectileManager::addProjectile(float x, float y, float speed, float angle, float damage, bool hostile, int id, bool makesSmileyFlash) {
	addProjectile(x, y, speed, angle, damage, hostile, id, makesSmileyFlash, false, 0.0, 0.0, 0.0);
}

/**
 * Add a projectile to the manager
 */
void ProjectileManager::addProjectile(float x, float y, float speed, float angle, float damage, bool hostile, 
									  int id, bool makesSmileyFlash, bool hasParabola, float parabolaLength, 
									  float parabolaDuration, float parabolaHeight) {

	//Create new projectile struct
	Projectile newProjectile;
	newProjectile.x = x;
	newProjectile.y = y;
	newProjectile.speed = speed;
	newProjectile.angle = angle;
	newProjectile.id = id;
	newProjectile.damage = damage;
	newProjectile.frisbeeAngle = angle;
	newProjectile.hostile = hostile;
	newProjectile.collisionBox = new hgeRect();
	newProjectile.collisionBox->SetRadius(x,y,projectileTypes[id].radius);
	newProjectile.terrainCollisionBox = new hgeRect();
	newProjectile.terrainCollisionBox->SetRadius(x,y,projectileTypes[id].radius/2.0);
	newProjectile.makesSmileyFlash = makesSmileyFlash;
	newProjectile.timeReflected = -10.0;
	newProjectile.homing = false;
	newProjectile.hasParabola = hasParabola;
	newProjectile.timeAlive = 0.0;
	newProjectile.parabolaDistance = parabolaLength;
	newProjectile.parabolaYOffset = 0.0;
	newProjectile.parabolaDuration = parabolaDuration;
	newProjectile.parabolaHeight = parabolaHeight;
	if (hasParabola) newProjectile.speed = parabolaLength / parabolaDuration;
	newProjectile.dx = newProjectile.speed * cos(angle);
	newProjectile.dy = newProjectile.speed * sin(angle);

	if (id == PROJECTILE_LIGHTNING_ORB) {
		newProjectile.waitingToReflect = false;
		newProjectile.facing = smh->player->facing;
	}

	if (id == PROJECTILE_FIREBALL) {
		newProjectile.particle = new hgeParticleSystem(&smh->resources->GetParticleSystem("fireBall")->info);
		newProjectile.particle->Fire();
	}

	if (id == PROJECTILE_TUT_LIGHTNING) {
		newProjectile.particle = new hgeParticleSystem(&smh->resources->GetParticleSystem("tutLightning")->info);
		newProjectile.particle->Fire();
		newProjectile.homing = true;
	}
	//Add it to the list
	theProjectiles.push_back(newProjectile);

}

/**
 * Figures out if a projectile should rotate CW or CCW to point to the target
 */
int ProjectileManager::rotateLeftOrRightForMinimumRotation(float projectileAngle, float angleToTarget) {
	float angleDifference = projectileAngle - angleToTarget;
	while (angleDifference < 0) angleDifference += 2*PI;
	while (angleDifference > 2*PI) angleDifference -= 2*PI;
	if (angleDifference > PI) return 1;
	return -1;
}


/**
 * Update all the projectiles
 */
void ProjectileManager::update(float dt) {
	//Loop through the projectiles
	bool deleteProjectile;
	std::list<Projectile>::iterator i;
	for (i = theProjectiles.begin(); i != theProjectiles.end(); i++) {
		
		deleteProjectile = false;

		//Update projectile stuff
		i->x += i->dx * dt;
		i->y += i->dy * dt;

		i->changedGridSquare = (Util::getGridX(i->x) != i->gridX || Util::getGridY(i->y) != i->gridY);
		i->gridX = Util::getGridX(i->x);
		i->gridY = Util::getGridY(i->y);
		i->timeAlive += dt;
		
		//Parabola stuff.
		if (i->hasParabola) {
			i->parabolaYOffset = i->parabolaHeight * sin((i->timeAlive/i->parabolaDuration) * PI);
			
			if (i->parabolaYOffset > projectileTypes[i->id].radius * 2) {
				//The projectile can't hit smiley when it is up in the air
				i->collisionBox->SetRadius(-1.0,-1.0,0.0);
			} else {
				i->collisionBox->SetRadius(i->x, i->y-i->parabolaYOffset, projectileTypes[i->id].radius);
			}

			if (i->timeAlive > i->parabolaDuration) {
				deleteProjectile = true;
			}
		} else {
			i->collisionBox->SetRadius(i->x, i->y, projectileTypes[i->id].radius);
		}

		//Do collision with Smiley
		if (!deleteProjectile && i->hostile && smh->player->collisionCircle->testBox(i->collisionBox)) {
			if (smh->player->isReflectingProjectiles()) {
				reflectProjectile(i);
			} else {
				smh->player->dealDamage(i->damage, i->makesSmileyFlash);
				deleteProjectile = true;
			}
		}

		//Do collision with enemies
		if (!i->hostile || i->id==PROJECTILE_TURRET_CANNONBALL) {
			if (smh->enemyManager->hitEnemiesWithProjectile(i->collisionBox, i->damage, i->id)) {
				deleteProjectile = true;
			}
		}

		//Orbs, Frisbees, and Cannonballs can toggle switches
		if (!deleteProjectile && i->id == PROJECTILE_LIGHTNING_ORB || i->id == PROJECTILE_FRISBEE || i->id == PROJECTILE_TURRET_CANNONBALL) {	
			if (smh->environment->toggleSwitches(i->collisionBox, i->id != PROJECTILE_TURRET_CANNONBALL)) {
				deleteProjectile = true;
			}
		}

		//Update frisbees
		if (!deleteProjectile && i->id == PROJECTILE_FRISBEE) {
			//Spin
			i->frisbeeAngle += 4.0f*PI*dt;
			//Destroy frisbees that get too far away from Smiley
			if (abs(i->x - smh->player->x) > 1080 || abs(i->y - smh->player->y) > 810) {
				deleteProjectile = true;
			}
		}

		//Update homing projectiles
		if (!deleteProjectile && i->homing) {
			float angleToSmiley = Util::getAngleBetween(i->x,i->y,smh->player->x,smh->player->y);
			int rotateDir = rotateLeftOrRightForMinimumRotation(i->angle,angleToSmiley);
			i->angle += rotateDir*1.7*dt;
			i->dx = i->speed * cos(i->angle);
			i->dy = i->speed * sin(i->angle);
		}

		//Lightning orb shit
		if (!deleteProjectile && i->id == PROJECTILE_LIGHTNING_ORB) {

			//If the orb entered a square with a mirror on it this frame,
			//calculate when the orb should reflect, and what direction it
			//should go when it does
			int mirror = smh->environment->collisionAt(i->x, i->y);
			if (i->changedGridSquare && (mirror == MIRROR_UP_LEFT || mirror == MIRROR_UP_RIGHT || mirror == MIRROR_DOWN_LEFT || mirror == MIRROR_DOWN_RIGHT)) {
				i->waitingToReflect = true;		
				if (mirror == MIRROR_UP_LEFT) {
					if (i->facing == DOWN) {
						i->reflectX = i->x;
						i->reflectY = i->gridY*64 + 64 - (int)i->x % 64;
						i->reflectDirection = LEFT;
					} else if (i->facing == RIGHT) {
						i->reflectX = i->gridX*64 + 64 - (int)i->y % 64;
						i->reflectY = i->y;
						i->reflectDirection = UP;
					} else if (i->facing == DOWN_RIGHT) {
						i->reflectX = i->gridX*64 + (64 - (int)i->y % 64)/2;
						i->reflectY = i->gridY*64 + (64 - (int)i->x % 64)/2;
						i->reflectDirection = UP_LEFT;
					} else {
						//Collision with non-reflective surface
						deleteProjectile = true;
					}
				} else if (mirror == MIRROR_UP_RIGHT) {
					if (i->facing == DOWN) {
						i->reflectX = i->x;
						i->reflectY = i->gridY*64 + (int)i->x % 64;
						i->reflectDirection = RIGHT;
					} else if (i->facing == LEFT) {
						i->reflectX = i->gridX*64 + (int)i->y % 64;
						i->reflectY = i->y;
						i->reflectDirection = UP;
					} else if (i->facing == DOWN_LEFT) {
						i->reflectX = i->gridX*64 + ((int)i->y % 64)/2;
						i->reflectY = i->gridY*64 + ((int)i->x % 64)/2;
						i->reflectDirection = UP_RIGHT;
					} else {
						//Collision with non-reflective surface
						deleteProjectile = true;
					}
				} else if (mirror == MIRROR_DOWN_LEFT) {
					if (i->facing == UP) {
						i->reflectX = i->x;
						i->reflectY = i->gridY*64 + (int)i->x % 64;
						i->reflectDirection = LEFT;
					} else if (i->facing == RIGHT) {
						i->reflectX = i->gridX*64 + (int)i->y % 64;
						i->reflectY = i->y;
						i->reflectDirection = DOWN;
					} else if (i->facing == UP_RIGHT) {
						i->reflectX = i->gridX*64 + ((int)i->y % 64)/2;
						i->reflectY = i->gridY*64 + 32 - ((int)i->x % 64)/2;
						i->reflectDirection = DOWN_LEFT;
					} else {
						//Collision with non-reflective surface
						deleteProjectile = true;
					}
				} else if (mirror == MIRROR_DOWN_RIGHT) {
					if (i->facing == UP) {
						i->reflectX = i->x;
						i->reflectY = i->gridY*64 + 64 - (int)i->x % 64;
						i->reflectDirection = RIGHT;
					} else if (i->facing == LEFT) {
						i->reflectX = i->gridX*64 + 64 - (int)i->y % 64;
						i->reflectY = i->y;
						i->reflectDirection = DOWN;
					} else if (i->facing == UP_LEFT) {
						i->reflectX = i->gridX*64 + (64 - (int)i->x % 64)/2;
						i->reflectY = i->gridY*64 + (64 - (int)i->y % 64)/2;
						i->reflectDirection = DOWN_RIGHT;
					} else {
						//Collision with non-reflective surface
						deleteProjectile = true;
					}
				}
			}

			//Check to see if reflection cases are met THIS FUCKING SUCKS
			if (!deleteProjectile && i->waitingToReflect && 
					(i->facing == DOWN			&& i->y > i->reflectY ||
					 i->facing == UP			&& i->y < i->reflectY ||
					 i->facing == RIGHT		&& i->x > i->reflectX ||
					 i->facing == LEFT			&& i->x < i->reflectX ||
					 i->facing == UP_LEFT		&& (i->y < i->reflectY || i->x < i->reflectX) ||
					 i->facing == UP_RIGHT		&& (i->y < i->reflectY || i->x > i->reflectX) ||
					 i->facing == DOWN_LEFT	&& (i->y > i->reflectY || i->x < i->reflectX) ||
					 i->facing == DOWN_RIGHT	&& (i->y > i->reflectY || i->x > i->reflectX))) {

				i->dx = 0.0;
				i->dy = 0.0;
				i->x = i->reflectX;
				i->y = i->reflectY;
				i->waitingToReflect = false;
				i->facing = i->reflectDirection;

				//Set dx and dy based on direction
				if (i->reflectDirection == RIGHT || i->reflectDirection == UP_RIGHT || i->reflectDirection == DOWN_RIGHT) {
					i->dx = LIGHTNING_ORB_SPEED;
				}
				if (i->reflectDirection == LEFT || i->reflectDirection == UP_LEFT || i->reflectDirection == DOWN_LEFT) {
					i->dx = -LIGHTNING_ORB_SPEED;
				}
				if (i->reflectDirection == UP || i->reflectDirection == UP_LEFT || i->reflectDirection == UP_RIGHT) {
					i->dy = -LIGHTNING_ORB_SPEED;
				}
				if (i->reflectDirection == DOWN || i->reflectDirection == DOWN_LEFT || i->reflectDirection == DOWN_RIGHT) {
					i->dy = LIGHTNING_ORB_SPEED;
				}

			}
		} // end update lightning orb	

		//Delete projectiles that hit walls and shit
		i->terrainCollisionBox->SetRadius(i->x, i->y, projectileTypes[i->id].radius/2.0);
		if ((i->id == PROJECTILE_FRISBEE || i->id == PROJECTILE_LIGHTNING_ORB) && !i->hostile) {
			//For friendly frisbees and lightning orbs, ignore silly pads when testing
			//collision. They will be taken care of in the environment class when it
			//tests collision with silly pads.
			if (!deleteProjectile && smh->environment->testCollision(i->terrainCollisionBox, canPass, true)) {
				deleteProjectile = true;
			}
		} else {
			//For all other projectiles test collision normally
			if (!deleteProjectile && smh->environment->testCollision(i->terrainCollisionBox, canPass)) {
				deleteProjectile = true;
			}
		}

		//If the projectile was marked for deletion, delete it now
		if (deleteProjectile) {
			//if it was a mushroom, and it is hostile, spawn an enemy mushroomlet
			if (i->id == PROJECTILE_MINI_MUSHROOM && i->hostile) {
				smh->enemyManager->addEnemy(MINI_MUSHROOM_ENEMYID,i->x/64,i->y/64,0.25,0.75,-1);
			}
			//if it was a mummy, spawn an enemy mummy
			if (i->id == PROJECTILE_TUT_MUMMY) {
				smh->enemyManager->addEnemy(MUMMY_ENEMYID,i->x/64,i->y/64,0.25,0.75,-1);
			}
			delete i->collisionBox;
			delete i->terrainCollisionBox;
			i = theProjectiles.erase(i);
		}

	}
}


/**
 * Draw all the projectiles
 */
void ProjectileManager::draw(float dt) {
	
	//Loop through the projectiles
	std::list<Projectile>::iterator i;
	for (i = theProjectiles.begin(); i != theProjectiles.end(); i++) {
		
		//Frisbee (spins)
		if (i->id == PROJECTILE_FRISBEE) {
			projectileTypes[i->id].sprite->RenderEx(smh->getScreenX(i->x), smh->getScreenY(i->y), i->frisbeeAngle, 1.0f, 1.0f);

		//Projectiles with particle effects
		} else if (i->id == PROJECTILE_FIREBALL) {
			i->particle->Update(dt);
			i->particle->MoveTo(smh->getScreenX(i->x), smh->getScreenY(i->y), true);
			i->particle->Render();

		//Laser - sprite is rotated 90 degrees (this is gay, change the graphic so theres not a special case)
		} else if (i->id == PROJECTILE_LASER) {
			projectileTypes[i->id].sprite->RenderEx(smh->getScreenX(i->x), smh->getScreenY(i->y), i->angle + (PI/2.0), 1.0f, 1.0f);
		
		//Tut laser
		} else if (i->id == PROJECTILE_TUT_LIGHTNING) {
			i->particle->Update(dt);
			i->particle->MoveTo(smh->getScreenX(i->x), smh->getScreenY(i->y), true);
			i->particle->Render();
			projectileTypes[i->id].sprite->RenderEx(smh->getScreenX(i->x), smh->getScreenY(i->y), i->angle, 1.0f, 1.0f);			
		
		//Normal projectiles
		} else {
			projectileTypes[i->id].sprite->RenderEx(smh->getScreenX(i->x), smh->getScreenY(i->y - i->parabolaYOffset), i->hasParabola ? 0.0 : i->angle, 1.0f, 1.0f);
		}
		
		//If this is a parabola projectile, draw its shadow
		if (i->hasParabola) {
			smh->resources->GetSprite("mushboomBombShadow")->Render(smh->getScreenX(i->x), smh->getScreenY(i->y));
		}

		//Debug stuff
		if (smh->isDebugOn()) {
			smh->drawCollisionBox(i->collisionBox, RED);
		}
	}
}


/**
 * Deletes all managed projectiles.
 */
void ProjectileManager::reset() {
	//Loop through the projectiles
	std::list<Projectile>::iterator i;
	for (i = theProjectiles.begin(); i != theProjectiles.end(); i++) {
		i = theProjectiles.erase(i);
	}
	theProjectiles.clear();
}


/**
 * Reflects a single projectile
 */
void ProjectileManager::reflectProjectile(std::list<Projectile>::iterator projectile) {
	
	if (smh->timePassedSince(projectile->timeReflected) < 2.0) return;

	projectile->angle += PI;

	projectile->timeReflected = smh->getGameTime();
	projectile->dx = -1 * projectile->dx;
	projectile->dy = -1 * projectile->dy;
	projectile->hostile = !projectile->hostile;

	//Reflected frisbees do damage
	if (projectile->id == PROJECTILE_FRISBEE) {
		projectile->damage = 0.25;
	}

}


/**
 * Returns whether or not there is currently a frisbee in the air
 */
bool ProjectileManager::frisbeeActive() {
	std::list<Projectile>::iterator i;
	for (i = theProjectiles.begin(); i != theProjectiles.end(); i++) {
		if (i->id == PROJECTILE_FRISBEE) {
			return true;
		}
	}
	return false;
}

/**
 * Destroys any projectiles that collide with the provided
 * collision box and are of the specified type. 
 * 
 * Returns the number of projectiles that were killed.
 */
int ProjectileManager::killProjectilesInBox(hgeRect *collisionBox, int type) {
	return killProjectilesInBox(collisionBox,type,true,true);
}


int ProjectileManager::killProjectilesInBox(hgeRect *collisionBox, int type, bool killHostile, bool killNonHostile) {
	int numCollisions = 0;
	std::list<Projectile>::iterator i;
	for (i = theProjectiles.begin(); i != theProjectiles.end(); i++) {
		if ((i->hostile && killHostile) || (!i->hostile && killNonHostile)) {
			if ((i->id == type || type == PROJECTILE_ALL) && collisionBox->Intersect(i->collisionBox)) {
				delete i->collisionBox;
				i = theProjectiles.erase(i);
				numCollisions++;
			}
		}
	}
	return numCollisions;
}

/**
 * Destroys any projectiles that collide with the provided circle
 * 
 * Parameters:
 *	x		- X coordinate of the center of the circle
 *  y		- Y coordinate of the center of the circle
 *	radius	- Radius of the circle
 *	type	- The type of the projectile to kill
 *
 * Returns: the number of projectiles that were killed.
 */
int ProjectileManager::killProjectilesInCircle(float x, float y, float radius, int type) {
	int numCollisions = 0;
	std::list<Projectile>::iterator i;
	for (i = theProjectiles.begin(); i != theProjectiles.end(); i++) {
		if (i->id == type && Util::distance(i->x, i->y, x, y) < radius) {
			delete i->collisionBox;
			i = theProjectiles.erase(i);
			numCollisions++;
		}
	}
	return numCollisions;
}

/**
 * Reflects any projectiles that collide with the provided collision
 * box and are of the specified type.
 */
bool ProjectileManager::reflectProjectilesInBox(hgeRect *collisionBox, int type) {
	bool retVal = false;
	std::list<Projectile>::iterator i;
	for (i = theProjectiles.begin(); i != theProjectiles.end(); i++) {
		if (i->id == type && collisionBox->Intersect(i->collisionBox) && smh->timePassedSince(i->timeReflected) > 1.0) {
			reflectProjectile(i);
		}
	}
	return retVal;
}

/**
 * Reflects any projectiles that collide with the provided circle
 * 
 * Parameters:
 *	x		- X coordinate of the center of the circle
 *  y		- Y coordinate of the center of the circle
 *	radius	- Radius of the circle
 *	type	- The type of the projectile to kill
 *
 * Returns: whether or not any projectiles were reflected
 */
bool ProjectileManager::reflectProjectilesInCircle(float x, float y, float radius, int type) {
	bool retVal = false;
	std::list<Projectile>::iterator i;
	for (i = theProjectiles.begin(); i != theProjectiles.end(); i++) {
		if (i->id == type && Util::distance(i->x, i->y, x, y) < radius && smh->timePassedSince(i->timeReflected) > 1.0) {
			reflectProjectile(i);
		}
	}
	return retVal;
}


/**
 * Initialize projectile types
 */
void ProjectileManager::initProjectiles() {

	projectileTypes[PROJECTILE_1].radius = 10;
	projectileTypes[PROJECTILE_1].sprite = smh->resources->GetSprite("basicProjectile");

	projectileTypes[PROJECTILE_FRISBEE].radius = 25;
	projectileTypes[PROJECTILE_FRISBEE].sprite = smh->resources->GetSprite("frisbeeProjectile");

	projectileTypes[PROJECTILE_LIGHTNING_ORB].radius = 15;
	projectileTypes[PROJECTILE_LIGHTNING_ORB].sprite = smh->resources->GetSprite("lightningOrbProjectile");

	projectileTypes[PROJECTILE_CACTUS_SPIKE].radius = 10;
	projectileTypes[PROJECTILE_CACTUS_SPIKE].sprite = smh->resources->GetSprite("spikeProjectile");

	projectileTypes[PROJECTILE_PENGUIN_FISH].radius = 16;
	projectileTypes[PROJECTILE_PENGUIN_FISH].sprite = smh->resources->GetSprite("fishProjectile");

	projectileTypes[PROJECTILE_FIREBALL].radius = 16;

	projectileTypes[PROJECTILE_TURRET_CANNONBALL].radius=16;
	projectileTypes[PROJECTILE_TURRET_CANNONBALL].sprite = smh->resources->GetSprite("cannonballProjectile");

	projectileTypes[PROJECTILE_MINI_MUSHROOM].radius=32;
	projectileTypes[PROJECTILE_MINI_MUSHROOM].sprite = smh->resources->GetSprite("mushroomletProjectile");

	projectileTypes[PROJECTILE_LASER].radius=3;
	projectileTypes[PROJECTILE_LASER].sprite = smh->resources->GetSprite("laserProjectile");

	projectileTypes[PROJECTILE_TUT_LIGHTNING].radius=10;
	projectileTypes[PROJECTILE_TUT_LIGHTNING].sprite = smh->resources->GetSprite("tutProjectile");

	projectileTypes[PROJECTILE_TUT_MUMMY].radius = 32;
	projectileTypes[PROJECTILE_TUT_MUMMY].sprite = smh->resources->GetSprite("tutProjectileMummy");

	projectileTypes[PROJECTILE_CANDY].radius = 23;
	projectileTypes[PROJECTILE_CANDY].sprite = smh->resources->GetSprite("CandyProjectile");

}
