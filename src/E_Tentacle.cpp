#include "SmileyEngine.h"
#include "EnemyFramework.h"
#include "player.h"
#include "environment.h"
#include "hgeresource.h"
#include "hgevector.h"
#include "ProjectileManager.h"
#include "CollisionCircle.h"

extern SMH *smh;

#define NUM_NODES 20
#define NUM_SINE_WAVES 1
#define ANGLE_ACC 0.0081
#define FRICTION_CONSTANT 2.0

#define TIME_BETWEEN_GROWLS 10.0
#define GROWL_DISTANCE 800.0

#define TENTACLE_IMMUNE_TIME 0.350
#define TENTACLE_KNOCKBACK_VEL 5.0



E_Tentacle::E_Tentacle(int id, int x, int y, int groupID) {

	//Call parent's init function
	initEnemy(id, x, y, groupID);

	double angleToSmiley = Util::getAngleBetween(x*64+32,y*64+32,smh->player->x,smh->player->y);
	int i;

	for (i=0;i<NUM_NODES;i++) {
		tentacleNodes[i].position.x=x*64+32;
		tentacleNodes[i].position.y=y*64+32+i*this->radius;
		tentacleNodes[i].angle=angleToSmiley;		
	}
	angle=angleToSmiley;
	angleVel=0;

	timeOfLastGrowl=smh->getGameTime()-TIME_BETWEEN_GROWLS*2;

	collisionBox = new hgeRect;
	   
}

E_Tentacle::~E_Tentacle() {	
    if (collisionBox) delete collisionBox;
}

void E_Tentacle::draw(float dt) {
	
	//Render blocks between nodes
	/*
	for(int i=0;i<NUM_NODES-1;i++) {
		graphic[3]->Update(dt);
		graphic[3]->RenderEx(smh->getScreenX((tentacleNodes[i].position.x+tentacleNodes[i+1].position.x)/2),smh->getScreenY((tentacleNodes[i].position.y+tentacleNodes[i+1].position.y)/2),Util::getAngleBetween(tentacleNodes[i].position.x,tentacleNodes[i].position.y,tentacleNodes[i+1].position.x,tentacleNodes[i+1].position.y));
	}
	*/
	
	graphic[1]->Update(dt);
	graphic[1]->Render(smh->getScreenX(tentacleNodes[0].position.x),smh->getScreenY(tentacleNodes[0].position.y));
	
	for (int i=1;i<NUM_NODES;i++) {
		graphic[2]->Update(dt);
		graphic[2]->Render(smh->getScreenX(tentacleNodes[i].position.x),smh->getScreenY(tentacleNodes[i].position.y));
	}

}

void E_Tentacle::update(float dt) {
	int i;

	if (knockback) {
		if (smh->timePassedSince(beginKnockbackTime) >= TENTACLE_IMMUNE_TIME) {
			knockback=false;
		} else {
			angle += knockbackDirection * TENTACLE_KNOCKBACK_VEL * dt;
		}		
	}

	double angleToSmiley = Util::getAngleBetween(x,y,smh->player->x,smh->player->y);
	double angleSubtracted = angleToSmiley - angle;

	while (angleSubtracted < 0) angleSubtracted += 2*PI;
	while (angleSubtracted > 2*PI) angleSubtracted -= 2*PI;

	angleVel=0;
	if (angleSubtracted < PI) {
		double angleDist = angleSubtracted;
		if (angleDist > 0.1) angleVel = speed*ANGLE_ACC * dt;
	} else {
		double angleDist = 2*PI-angleSubtracted;
		if (angleDist > 0.1) angleVel = -speed*ANGLE_ACC * dt;
	}
	
	if (!knockback) angle += angleVel;

    
	for (i=0;i<NUM_NODES;i++) {
		//make a certain number of sine waves based on how many nodes there are

		tentacleNodes[i].angle= PI/6*sin(smh->getGameTime()*speed/17+2*PI*i*NUM_SINE_WAVES/NUM_NODES)+angle;
		
		//Recalculate x and y
		tentacleNodes[i].position.x = x+i*variable1*cos(tentacleNodes[i].angle);
		tentacleNodes[i].position.y = y+i*variable1*sin(tentacleNodes[i].angle);

		//Collision with smiley
		if (Util::distance(tentacleNodes[i].position.x,tentacleNodes[i].position.y,smh->player->x,smh->player->y) <= radius + smh->player->collisionCircle->radius) {
			smh->player->dealDamageAndKnockback(damage,true,100,tentacleNodes[i].position.x,tentacleNodes[i].position.y);			
		}		

	}

	//Growl, if variable > 10 (ensuring it's a large and not a small tentacle
	if (smh->timePassedSince(timeOfLastGrowl) >= TIME_BETWEEN_GROWLS && variable1 >= 10 && distanceFromPlayer() <= GROWL_DISTANCE) {
		timeOfLastGrowl = smh->getGameTime();
		smh->soundManager->playSound("snd_fireWorm");
	}


}

/**
 * Overrides base tongue collision logic in order to implement collision
 * for each node of the tentacle.
 */ 
bool E_Tentacle::doTongueCollision(Tongue *tongue, float damage) {
	
	for (int i=0; i<NUM_NODES; i++) {
		//Set up collision box
		collisionBox->x1=tentacleNodes[i].position.x-radius;
		collisionBox->y1=tentacleNodes[i].position.y-radius;
		collisionBox->x2=tentacleNodes[i].position.x+radius;
		collisionBox->y2=tentacleNodes[i].position.y+radius;

		//Collision with tongue
		if (!immuneToTongue && tongue->testCollision(collisionBox)) {
			//Make sure the enemy wasn't already hit by this attack
			if (smh->timePassedSince(lastHitByWeapon) > TENTACLE_IMMUNE_TIME) {
				
				lastHitByWeapon = smh->getGameTime();
				dealDamageAndKnockback(smh->player->getDamage(), 0.0, smh->player->x, smh->player->y);
				startFlashing();
				
				//Knockback by rotating it away from smiley a bit
				double angleToSmiley = Util::getAngleBetween(x,y,smh->player->x,smh->player->y);
				double angleSubtracted = angleToSmiley - angle;

				while (angleSubtracted < 0) angleSubtracted += 2*PI;
				while (angleSubtracted > 2*PI) angleSubtracted -= 2*PI;

				if (angleSubtracted < PI) {
					//neg
					knockbackDirection=-1;
				} else {
					//pos
					knockbackDirection=+1;
				}

				beginKnockbackTime=smh->getGameTime();
				knockback=true;
				return true;

			}
		}
	}
	return false;
}
