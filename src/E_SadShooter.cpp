#include "SmileyEngine.h"
#include "EnemyFramework.h"
#include "player.h"
#include "environment.h"
#include "hgeresource.h"
#include "ProjectileManager.h"
#include "CollisionCircle.h"

extern SMH *smh;

#define ANGULAR_ACC 22.0
#define SAD_SHOOTER_FRICTION 1.5
#define NUM_BLOCKERS 5
#define BLOCKER_RADIUS 32.0

E_SadShooter::E_SadShooter(int id, int x, int y, int groupID) {

	//Call parent's init function
	initEnemy(id, x, y, groupID);

	collisionBlocker = new hgeRect;

	double angleToSmiley = Util::getAngleBetween(x,y,smh->player->x,smh->player->y);
	int i;
	
	for (i=0;i<NUM_BLOCKERS;i++) {
		sadBlockers[i].desiredAngleOffset=i*2*PI/NUM_BLOCKERS;
		sadBlockers[i].distance=180;
	
		sadBlockers[i].angle = angleToSmiley + sadBlockers[i].desiredAngleOffset;
		sadBlockers[i].angularVelocity = 0;

		//calculate x and y
		sadBlockers[i].x = x*64+32 + sadBlockers[i].distance*cos(sadBlockers[i].angle);
		sadBlockers[i].y = y*64+32 + sadBlockers[i].distance*sin(sadBlockers[i].angle);
	}

    
}

E_SadShooter::~E_SadShooter() {
	if (collisionBlocker) delete collisionBlocker;

}

void E_SadShooter::draw(float dt) {
	float angleToSmiley = Util::getAngleBetween(x,y,smh->player->x,smh->player->y);
	
	graphic[1]->RenderEx(screenX,screenY,angleToSmiley+PI/2);

	for (int i=0; i< NUM_BLOCKERS; i++) {
		graphic[2]->RenderEx(smh->getScreenX(sadBlockers[i].x),smh->getScreenY(sadBlockers[i].y),sadBlockers[i].angle+PI/2);
	}

}


void E_SadShooter::update(float dt) {

	float angleToSmiley = Util::getAngleBetween(x,y,smh->player->x,smh->player->y);

	for(int i=0;i<NUM_BLOCKERS;i++) {
		double angleSubtracted = angleToSmiley - (sadBlockers[i].angle+sadBlockers[i].desiredAngleOffset);
		while (angleSubtracted < 0) angleSubtracted += 2*PI;
		while (angleSubtracted > 2*PI) angleSubtracted -= 2*PI;

		if (angleSubtracted < PI) {
			double angleDist = angleSubtracted;
			double acc;
			sadBlockers[i].angularVelocity += angleDist*ANGULAR_ACC * dt;
		} else {
			double angleDist = 2*PI-angleSubtracted;
			double acc;
			sadBlockers[i].angularVelocity -= angleDist*ANGULAR_ACC * dt;
		}

		sadBlockers[i].angle += sadBlockers[i].angularVelocity * dt;

		//"Friction"
		double decrement = sadBlockers[i].angularVelocity * SAD_SHOOTER_FRICTION;
		sadBlockers[i].angularVelocity -= decrement*dt;

		//Recalculate x and y
		sadBlockers[i].x = x + sadBlockers[i].distance*cos(sadBlockers[i].angle);
		sadBlockers[i].y = y + sadBlockers[i].distance*sin(sadBlockers[i].angle);

		if (Util::distance(sadBlockers[i].x,sadBlockers[i].y,smh->player->x,smh->player->y) <= BLOCKER_RADIUS + smh->player->collisionCircle->radius) {
			smh->player->dealDamageAndKnockback(damage,true,100,sadBlockers[i].x,sadBlockers[i].y);	
			smh->setDebugText("Smiley hit by a Sad Blocker");
		}

		collisionBlocker->x1=sadBlockers[i].x-BLOCKER_RADIUS+3; //add 3 to make the rect a bit smaller so it better approximates the circle
		collisionBlocker->y1=sadBlockers[i].y-BLOCKER_RADIUS+3; //add 3 to make the rect a bit smaller so it better approximates the circle
		collisionBlocker->x2=sadBlockers[i].x+BLOCKER_RADIUS-3; //subtract 3 to make the rect a bit smaller so it better approximates the circle
		collisionBlocker->y2=sadBlockers[i].y+BLOCKER_RADIUS-3; //subtract 3 to make the rect a bit smaller so it better approximates the circle

		
		//Reflect different types of projectiles
		smh->projectileManager->reflectProjectilesInBox(collisionBlocker,PROJECTILE_FRISBEE);
		smh->projectileManager->reflectProjectilesInBox(collisionBlocker,PROJECTILE_LIGHTNING_ORB);
	
	}
}

/**
 * When the sad shooter dies we want to spawn additional death particles
 * at each of its blockers
 */
void E_SadShooter::notifyOfDeath()
{
	for(int i=0;i<NUM_BLOCKERS;i++) 
	{
		smh->enemyManager->spawnDeathParticle(sadBlockers[i].x, sadBlockers[i].y);
	}
}