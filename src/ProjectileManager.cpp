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

ProjectileManager::ProjectileManager() {

	projectileTexture = smh->hge->Texture_Load("Graphics/projectiles.PNG");
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
	newProjectile.dx = speed * cos(angle);
	newProjectile.dy = speed * sin(angle);
	newProjectile.makesSmileyFlash = makesSmileyFlash;
	newProjectile.timeReflected = -10.0;
	
	if (id == PROJECTILE_LIGHTNING_ORB) {
		newProjectile.waitingToReflect = false;
		newProjectile.facing = smh->player->facing;
	}

	if (id == PROJECTILE_FIREBALL) {
		newProjectile.particle = new hgeParticleSystem(&smh->resources->GetParticleSystem("fireBall")->info);
		newProjectile.particle->Fire();
	}

	//Add it to the list
	theProjectiles.push_back(newProjectile);

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
		i->collisionBox->SetRadius(i->x, i->y, projectileTypes[i->id].radius);
		
		//Do collision with Smiley
		if (i->hostile && smh->player->collisionCircle->testBox(i->collisionBox)) {
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

		//Laser - draw a line
		} else if (i->id == PROJECTILE_LASER) {

			projectileTypes[i->id].sprite->RenderEx(smh->getScreenX(i->x), smh->getScreenY(i->y), i->angle + (PI/2.0), 1.0f, 1.0f);

		//Normal projectiles - rotated to face the direction its travelling
		} else {
			projectileTypes[i->id].sprite->RenderEx(smh->getScreenX(i->x), smh->getScreenY(i->y), i->angle, 1.0f, 1.0f);
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

	//Generic projectile 1
	projectileTypes[PROJECTILE_1].radius = 10;
	projectileTypes[PROJECTILE_1].sprite = smh->resources->GetSprite("basicProjectile");

	//Smiley's Frisbee ability
	projectileTypes[PROJECTILE_FRISBEE].radius = 25;
	projectileTypes[PROJECTILE_FRISBEE].sprite = smh->resources->GetSprite("frisbeeProjectile");

	//Smiley's Power Beam ability
	projectileTypes[PROJECTILE_LIGHTNING_ORB].radius = 15;
	projectileTypes[PROJECTILE_LIGHTNING_ORB].sprite = smh->resources->GetSprite("lightningOrbProjectile");

	//Cactus spikes
	projectileTypes[PROJECTILE_CACTUS_SPIKE].radius = 10;
	projectileTypes[PROJECTILE_CACTUS_SPIKE].sprite = smh->resources->GetSprite("spikeProjectile");

	//Fish
	projectileTypes[PROJECTILE_PENGUIN_FISH].radius = 16;
	projectileTypes[PROJECTILE_PENGUIN_FISH].sprite = smh->resources->GetSprite("fishProjectile");

	//Fireball
	projectileTypes[PROJECTILE_FIREBALL].radius = 16;

	//Turret cannonball
	projectileTypes[PROJECTILE_TURRET_CANNONBALL].radius=16;
	projectileTypes[PROJECTILE_TURRET_CANNONBALL].sprite = smh->resources->GetSprite("cannonballProjectile");

	//Mini mushroom
	projectileTypes[PROJECTILE_MINI_MUSHROOM].radius=32;
	projectileTypes[PROJECTILE_MINI_MUSHROOM].sprite = smh->resources->GetSprite("mushroomletProjectile");

	//Laser
	projectileTypes[PROJECTILE_LASER].radius=3;
	projectileTypes[PROJECTILE_LASER].sprite = smh->resources->GetSprite("laserProjectile");

}
