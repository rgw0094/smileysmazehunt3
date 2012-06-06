#include "SmileyEngine.h"
#include "environment.h"
#include "ProjectileManager.h"
#include "player.h"
#include "hgeresource.h"
#include "EnemyFramework.h"
#include "CollisionCircle.h"
#include "ExplosionManager.h"

extern SMH *smh;

#define LASER_LENGTH 20.0
#define MINI_MUSHROOM_ENEMYID 43

#define ACC 350 //used for boomerang acceleration
#define BOOMERANG_LIFE 4 //how long before it self-destructs (necessary b/c boomerangs don't have collision to the terrain)

ProjectileManager::ProjectileManager() 
{
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

ProjectileManager::~ProjectileManager() 
{
	//Should never be deleted
}


void ProjectileManager::addFrisbee(float x, float y, float speed, float angle, float stunPower) 
{
	addProjectile(x, y, speed, angle, 0.0, false, false, PROJECTILE_FRISBEE, false, false, 0.0, 0.0, 0.0, stunPower);
}

void ProjectileManager::addProjectile(float x, float y, float speed, float angle, float damage, bool hostile, 
									  bool homing, int id, bool makesSmileyFlash) 
{
	addProjectile(x, y, speed, angle, damage, hostile, homing, id, makesSmileyFlash, false, 0.0, 0.0, 0.0, 0.0);
}

void ProjectileManager::addProjectile(float x, float y, float speed, float angle, float damage, bool hostile, bool homing,
									  int id, bool makesSmileyFlash, bool hasParabola, float parabolaLength, 
									  float parabolaDuration, float parabolaHeight) 
{
	addProjectile(x, y, speed, angle, damage, hostile, homing, id, makesSmileyFlash, hasParabola, parabolaLength, parabolaDuration, parabolaHeight, 0.0);
}

int ProjectileManager::getProjectileRadius(int id) 
{
	if (id > NUM_PROJECTILES-1)
	{
		std::string exceptionString = "ProjectileManager.getProjectileRadius() received invalid projectile id: " + Util::intToString(id);
		throw new System::Exception(new System::String(exceptionString.c_str()));
	}

	return projectileTypes[id].radius;
}

void ProjectileManager::addProjectile(float x, float y, float speed, float angle, float damage, bool hostile, bool homing,
									  int id, bool makesSmileyFlash, bool hasParabola, float parabolaLength, 
									  float parabolaDuration, float parabolaHeight, float stunPower) 
{

	//Create new projectile struct
	Projectile newProjectile;
	newProjectile.x = newProjectile.startX = x;
	newProjectile.y = newProjectile.startY = y;
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
	newProjectile.homing = homing;
	newProjectile.hasParabola = hasParabola;
	newProjectile.timeAlive = 0.0;
	newProjectile.parabolaDistance = parabolaLength;
	newProjectile.parabolaYOffset = 0.0;
	newProjectile.parabolaDuration = parabolaDuration;
	newProjectile.parabolaHeight = parabolaHeight;
	if (hasParabola) newProjectile.speed = parabolaLength / parabolaDuration;
	newProjectile.dx = newProjectile.speed * cos(angle);
	newProjectile.dy = newProjectile.speed * sin(angle);
	newProjectile.stunPower = stunPower;

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
	}

	if (id == PROJECTILE_BARV_COMET) {
		newProjectile.particle = new hgeParticleSystem(&smh->resources->GetParticleSystem("bigWhiteBarv")->info);
		newProjectile.particle->Fire();
	}

	if (id == PROJECTILE_BARV_YELLOW) {
		newProjectile.particle = new hgeParticleSystem(&smh->resources->GetParticleSystem("smallYellowBarv")->info);
		newProjectile.particle->Fire();
	}

	if (id == PROJECTILE_SKULL) {
		newProjectile.particle = new hgeParticleSystem(&smh->resources->GetParticleSystem("skullProjectileParticle")->info);
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

		//if boomerang
		if (i->id == PROJECTILE_BOOMERANG) {
			if (i->x > i->startX) i->dx -= ACC*dt;
			if (i->x < i->startX) i->dx += ACC*dt;
			if (i->y > i->startY) i->dy -= ACC*dt;
			if (i->y < i->startY) i->dy += ACC*dt;
			if (i->timeAlive >= BOOMERANG_LIFE) deleteProjectile = true;

			i->angle = 2*3.14159*sin(i->timeAlive*3);
		}

		i->changedGridSquare = (Util::getGridX(i->x) != i->gridX || Util::getGridY(i->y) != i->gridY);
		i->gridX = Util::getGridX(i->x);
		i->gridY = Util::getGridY(i->y);
		i->timeAlive += dt;

		//If the projectile goes off the map, delete it
		if (i->gridX < 0 || i->gridX > 255 || i->gridY < 0 || i->gridY > 255) deleteProjectile = true;
		
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
				if (i->id == PROJECTILE_SLIME) {
					smh->explosionManager->addSlimeExplosion(i->x, i->y, 1.0, 0.5, 0.0);
				}
			}
		} else {
			i->collisionBox->SetRadius(i->x, i->y, projectileTypes[i->id].radius);
		}

		//Delete projectiles that hit walls and shit
		i->terrainCollisionBox->SetRadius(i->x, i->y, projectileTypes[i->id].radius/2.0);
		if ((i->id == PROJECTILE_FRISBEE || i->id == PROJECTILE_LIGHTNING_ORB) && !i->hostile) {
			//For friendly frisbees and lightning orbs, ignore silly pads when testing
			//collision. They will be taken care of in the environment class when it
			//tests collision with silly pads.
			if (!deleteProjectile && smh->environment->testCollision(i->terrainCollisionBox, canPass, true)) {
				if (i->id == PROJECTILE_FRISBEE) {
					smh->soundManager->playSound("snd_FrisbeeHitWall");
				}
				deleteProjectile = true;
			}
		} else if (i->id == PROJECTILE_BOOMERANG || i->id == PROJECTILE_LASER) {
			//Boomerang and Calypso's laser do not hit collision layer!! They go through stuff
		} else {
			//For all other projectiles test collision normally
			if (!deleteProjectile && smh->environment->testCollision(i->terrainCollisionBox, canPass)) {
				deleteProjectile = true;
			}
		}

		//Do collision with Smiley
		if (!deleteProjectile && i->hostile && smh->player->collisionCircle->testBox(i->collisionBox)) 
		{
			//Test to see if the reflection shield is active. Never reflect parabola projectiles
			//because it looks stupid.
			if (smh->player->isReflectingProjectiles() && !i->hasParabola) 
			{
				reflectProjectile(i);
			} 
			else 
			{
				smh->player->dealDamage(i->damage, i->makesSmileyFlash);
				std::string debugString;
				debugString = "Smiley hit by projectile of type " + Util::intToString(i->id) + 
					" at pos (" + Util::intToString((int)i->x) + "," + Util::intToString((int)i->y) + 
					") grid ("	+ Util::intToString(i->gridX) + "," + Util::intToString(i->gridY) +")";
				//smh->setDebugText(debugString.c_str());
				deleteProjectile = true;
			}
		}

		//Do collision with enemies
		if (!i->hostile || i->id==PROJECTILE_TURRET_CANNONBALL) {
			if (smh->enemyManager->hitEnemiesWithProjectile(i->collisionBox, i->damage, i->id, i->stunPower)) {
				
				deleteProjectile = true;
			}
		}

		//Orbs, Frisbees, and Cannonballs can toggle switches
		if (!deleteProjectile && i->id == PROJECTILE_LIGHTNING_ORB || i->id == PROJECTILE_FRISBEE || i->id == PROJECTILE_TURRET_CANNONBALL) {	
			if (smh->environment->toggleSwitches(i->collisionBox, i->id != PROJECTILE_TURRET_CANNONBALL, i->id != PROJECTILE_TURRET_CANNONBALL)) {
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
			int rotateDir = Util::rotateLeftOrRightForMinimumRotation(i->angle,angleToSmiley);
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

			int radius = projectileTypes[i->id].radius;
		

			//If the projectile is on the very edge of the tile, it can reflect and hit switches one square over.
			//This fixes that by making it so it's a max of (radius+3) away from the edge

            int mirrorGridX = i->x / (float)64.0;
			int mirrorGridY = i->y / (float)64.0;

			float reflectXToUse = i->x;
			float reflectYToUse = i->y;

			float offsetX = reflectXToUse - mirrorGridX*64;
			float offsetY = reflectYToUse - mirrorGridY*64;

			//moving up/down, so check its left-right position in the tile
			if (i->facing == DOWN || i->facing == UP) {
				if (offsetX > 64-(radius*2+3)) reflectXToUse = mirrorGridX*64+(64-(radius+3));
				else if (offsetX < (radius*2+3)) reflectXToUse = mirrorGridX*64+(radius+3);                				
			//moving left/right, so check its up-down position in the tile
			} else {
				if (offsetY > 64-(radius+3)) reflectYToUse = mirrorGridY*64+(64-(radius+3));
				else if (offsetY < (radius+3)) reflectYToUse = mirrorGridY*64+(radius+3);                				
			}

			offsetX = reflectXToUse - mirrorGridX*64;
			offsetY = reflectYToUse - mirrorGridY*64;
			
			if (i->changedGridSquare && (mirror == MIRROR_UP_LEFT || mirror == MIRROR_UP_RIGHT || mirror == MIRROR_DOWN_LEFT || mirror == MIRROR_DOWN_RIGHT)) {
				i->waitingToReflect = true;		
				if (mirror == MIRROR_UP_LEFT) {
					if (i->facing == DOWN) {
						i->reflectX = reflectXToUse;
						i->reflectY = i->gridY*64 + 64 - offsetX;
						i->reflectDirection = LEFT;
						smh->setDebugText("orb reflected, was moving down now moving left");
					} else if (i->facing == RIGHT) {
						i->reflectX = i->gridX*64 + 64 - offsetY;
						i->reflectY = reflectYToUse;
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
						i->reflectX = reflectXToUse;
						i->reflectY = i->gridY*64 + offsetX;
						i->reflectDirection = RIGHT;
					} else if (i->facing == LEFT) {
						i->reflectX = i->gridX*64 + offsetY;
						i->reflectY = reflectYToUse;
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
						i->reflectX = reflectXToUse;
						i->reflectY = i->gridY*64 + offsetX;
						i->reflectDirection = LEFT;
					} else if (i->facing == RIGHT) {
						i->reflectX = i->gridX*64 + offsetY;
						i->reflectY = reflectYToUse;
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
						i->reflectX = reflectXToUse;
						i->reflectY = i->gridY*64 + 64 - offsetX;
						i->reflectDirection = RIGHT;
					} else if (i->facing == LEFT) {
						i->reflectX = i->gridX*64 + 64 - offsetY;
						i->reflectY = reflectYToUse;
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

		//If the projectile was marked for deletion, delete it now
		if (deleteProjectile) {
			//if it was a mushroom, and it is hostile, spawn an enemy mushroomlet
			if (i->id == PROJECTILE_MINI_MUSHROOM && i->hostile) {
				smh->enemyManager->addEnemy(MINI_MUSHROOM_ENEMYID,i->x/64,i->y/64,0.25,0.75,-1, false);
			}
			//if it was a mummy, spawn a random mummy to spawn
			if (i->id == PROJECTILE_TUT_MUMMY) {
				int random = smh->hge->Random_Int(0, 10000);
				int enemy = 0;
				if (random < 1000) {
					enemy = FLAIL_MUMMY;
				} else if (random < 6000) {
					enemy = CHARGER_MUMMY;
				} else {
					enemy = RANGED_MUMMY;
				}
				smh->enemyManager->addEnemy(enemy,i->x/64,i->y/64,0.25,0.75,-1, false);
			}
			if (i->id == PROJECTILE_SLIME) {
				smh->soundManager->playSound("snd_SlimeSplat",0.02);
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
		
		// Barvinoid comet
		} else if (i->id == PROJECTILE_BARV_COMET) {
			i->particle->Update(dt);
			i->particle->MoveTo(smh->getScreenX(i->x), smh->getScreenY(i->y), true);
			i->particle->Render();
			projectileTypes[i->id].sprite->RenderEx(smh->getScreenX(i->x), smh->getScreenY(i->y), i->angle, 1.0f, 1.0f);

		// Barvinoid yellow sphere
		} else if (i->id == PROJECTILE_BARV_YELLOW) {
			i->particle->Update(dt);
			i->particle->MoveTo(smh->getScreenX(i->x), smh->getScreenY(i->y), true);
			i->particle->Render();
			projectileTypes[i->id].sprite->RenderEx(smh->getScreenX(i->x), smh->getScreenY(i->y), i->angle, 1.0f, 1.0f);
		
		//Figure 8 (width based on distance from its origin so it gets wider & skinnier as it travels)
		} else if (i->id == PROJECTILE_FIGURE_8) {
			float distanceFromOrigin = Util::distance(i->x,i->y,i->startX,i->startY);
			float width = sin(distanceFromOrigin/30.0); //from -1 to 1
			projectileTypes[i->id].sprite->RenderEx(smh->getScreenX(i->x),smh->getScreenY(i->y),i->angle,1.0,width);		
		
		//Skull (always facing up, has particle effect)
		}else if (i->id == PROJECTILE_SKULL) {
			i->particle->Update(dt);
			i->particle->MoveTo(smh->getScreenX(i->x), smh->getScreenY(i->y), true);
			i->particle->Render();
			projectileTypes[i->id].sprite->Render(smh->getScreenX(i->x), smh->getScreenY(i->y));

		//Acorn (rotates)
		} else if (i->id == PROJECTILE_ACORN) {
			projectileTypes[i->id].sprite->RenderEx(smh->getScreenX(i->x), smh->getScreenY(i->y),10*i->timeAlive);


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
			smh->drawCollisionBox(i->collisionBox, Colors::RED);
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

int ProjectileManager::killProjectiles(int type) {
	int num = 0;
	for (std::list<Projectile>::iterator i = theProjectiles.begin(); i != theProjectiles.end(); i++) {
		if (i->id == type) {
			delete i->collisionBox;
			i = theProjectiles.erase(i);
			num++;
		}
	}
	return num;
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
 * box and are of the specified type. Returns whether or not any
 * projectiles were reflected.
 */
bool ProjectileManager::reflectProjectilesInBox(hgeRect *collisionBox, int type) 
{
	bool retVal = false;
	for (std::list<Projectile>::iterator i = theProjectiles.begin(); i != theProjectiles.end(); i++) 
	{
		if (i->id == type && collisionBox->Intersect(i->collisionBox) && 
			smh->timePassedSince(i->timeReflected) > 1.0) 
		{
			reflectProjectile(i);
			retVal = true;
		}
	}
	return retVal;
}

/**
 * Reflects any projectiles that collide with the provided circle.
 * 
 * Parameters:
 *	x		- X coordinate of the center of the circle
 *  y		- Y coordinate of the center of the circle
 *	radius	- Radius of the circle
 *	type	- The type of the projectile to kill
 *
 * Returns: whether or not any projectiles were reflected
 */
bool ProjectileManager::reflectProjectilesInCircle(float x, float y, float radius, int type) 
{
	bool retVal = false;
	for (std::list<Projectile>::iterator i = theProjectiles.begin(); i != theProjectiles.end(); i++) 
	{
		if (i->id == type && Util::distance(i->x, i->y, x, y) < radius && 
			smh->timePassedSince(i->timeReflected) > 1.0) 
		{
			reflectProjectile(i);
			retVal = true;
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

	projectileTypes[PROJECTILE_TUT_MUMMY].radius = 27;
	projectileTypes[PROJECTILE_TUT_MUMMY].sprite = smh->resources->GetSprite("tutProjectileMummy");

	projectileTypes[PROJECTILE_CANDY].radius = 23;
	projectileTypes[PROJECTILE_CANDY].sprite = smh->resources->GetSprite("CandyProjectile");

	projectileTypes[PROJECTILE_FIGURE_8].radius=16;
	projectileTypes[PROJECTILE_FIGURE_8].sprite = smh->resources->GetSprite("Figure8Projectile");

	projectileTypes[PROJECTILE_SLIME].radius = 12;
	projectileTypes[PROJECTILE_SLIME].sprite = smh->resources->GetSprite("SlimeProjectile");

	projectileTypes[PROJECTILE_ORANGE].radius = 8;
	projectileTypes[PROJECTILE_ORANGE].sprite = smh->resources->GetSprite("OrangeProjectile");

	projectileTypes[PROJECTILE_BOOMERANG].radius = 16;
	projectileTypes[PROJECTILE_BOOMERANG].sprite = smh->resources->GetSprite("BoomerangProjectile");

	projectileTypes[PROJECTILE_BARV_COMET].radius = 32;
	projectileTypes[PROJECTILE_BARV_COMET].sprite = smh->resources->GetSprite("lightningOrbProjectile");

	projectileTypes[PROJECTILE_BARV_YELLOW].radius = 30;
	projectileTypes[PROJECTILE_BARV_YELLOW].sprite = smh->resources->GetSprite("floatingEyeShot");

	projectileTypes[PROJECTILE_SKULL].radius = 27;
	projectileTypes[PROJECTILE_SKULL].sprite = smh->resources->GetSprite("SkullProjectile");

	projectileTypes[PROJECTILE_ACORN].radius = 22;
	projectileTypes[PROJECTILE_ACORN].sprite = smh->resources->GetSprite("AcornProjectile");

}
