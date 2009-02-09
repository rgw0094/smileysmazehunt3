#include "SmileyEngine.h"
#include "EnemyFramework.h"
#include "hgeresource.h"
#include "ProjectileManager.h"
#include "player.h"
#include "environment.h"
#include "CollisionCircle.h"

extern SMH *smh;

#define TIME_BETWEEN_SPAWNS 15.0
#define MAX_DISTANCE_FROM_SMILEY_TO_SPAWN 300


/**
 * Constructor
 */
E_Spawner::E_Spawner(int id, int gridX, int gridY, int groupID) {

	//Call parent init method
	initEnemy(id, gridX, gridY, groupID);

	//Start in wander state
	setState(new ES_Wander(this));

	facing = DOWN;
	timeOfLastSpawn = -10.0;

	//Framework values
	chases = false;
	hasRangedAttack = false;

	enemyTypeToSpawn1 = variable1;
	enemyTypeToSpawn2 = variable2;
	enemyTypeToSpawn3 = variable3;

	//These are copied and pasted from E_Floater's init
	dealsCollisionDamage = false;
	facing = LEFT;
	shadowOffset = 25.0;
	lastDirChange = 0.0;
	dirChangeDelay = 0.0;
}

/**
 * Destructor
 */
E_Spawner::~E_Spawner() {

}


/**
 * Draws the spawner.
 */
void E_Spawner::draw(float dt) {
	graphic[facing]->Update(dt);
	graphic[facing]->Render(screenX, screenY - shadowOffset);
	smh->resources->GetSprite("playerShadow")->Render(screenX, screenY);

	if (smh->isDebugOn()) {
		smh->drawCollisionBox(collisionBox, RED);
	}
}

/**
 * Updates the spawner.
 */
void E_Spawner::update(float dt) {
	
	doAStar();

	if (smh->timePassedSince(timeOfLastSpawn) >= TIME_BETWEEN_SPAWNS &&
		Util::distance(x,y,smh->player->x,smh->player->y) <= MAX_DISTANCE_FROM_SMILEY_TO_SPAWN &&
		smh->environment->validPath(x, y, smh->player->x, smh->player->y, 13, canPass)) {
	
		spawnEnemy();
	}

	//Update angle
	angleVel = angleCoefficient * cos(smh->getGameTime()) * dt;
	angle += angleVel * dt;

	//Update floating shit
	shadowOffset = 35.0 + 12.0 * cos(smh->getGameTime() * 2.0);
	projectileYOffset = shadowOffset;
	collisionBox->SetRadius(x,y-shadowOffset,radius);
	
	//Update position
	if (!stunned) {
		dx = speed * cos(angle);
		dy = speed * sin(angle);
	}

	boolean changeDir = false;

	//Change angle coefficient periodically
	if (smh->timePassedSince(lastDirChange) > dirChangeDelay) {

		angleCoefficient = smh->randomFloat(50.0, 100.0);
		if (smh->randomInt(0,1) == 1) angleCoefficient *= -1;

		dirChangeDelay = smh->randomFloat(2.0,3.0);
		lastDirChange = smh->getGameTime();
	}

	//Change direction if the enemy is going to hit a wall next frame
	futureCollisionBox->SetRadius(max(4.0, x + dx*dt), max(4.0, y + dy*dt), 28.0f);
	if (smh->environment->enemyCollision(futureCollisionBox,this,dt)) {
		//Bounce 180 degrees off the wall
		angle += PI;
	}
		
	//Collision with player
	if (smh->player->collisionCircle->testBox(collisionBox)) {
		smh->player->dealDamageAndKnockback(damage, true, 115, x, y);
		std::string debugText;
		debugText = "E_Spawner.cpp Smiley hit by enemy type " + Util::intToString(id) +
			" at grid (" + Util::intToString(gridX) + "," + Util::intToString(gridY) +
			") pos (" + Util::intToString((int)x) + "," + Util::intToString((int)y) + ")";
		smh->setDebugText(debugText);
	}

	//see if close enough to player to shoot
	if (hasRangedAttack && canShootPlayer()) {
		setState(new ES_RangedAttack(this));
	} else {
		setState(new ES_Wander(this));
	}

	
	move(dt);
}

/**
 * Spawns an enemy.
 */
void E_Spawner::spawnEnemy() {
	int enemy=0;
	int randomInt;
	
	randomInt = smh->hge->Random_Int(0,100);

	if (randomInt <= 45) {				//45% chance of enemy type 1
		enemy = enemyTypeToSpawn1;
	} else if (randomInt <= 80) {		//35% chance of enemy type 2
		enemy = enemyTypeToSpawn2;
	} else {							//20% chance of enemy type 3
		enemy = enemyTypeToSpawn3;
	}

	std::string debugText;
	debugText = "E_Spawner.cpp E1= " + Util::intToString(enemyTypeToSpawn1) + 
		" E2= " + Util::intToString(enemyTypeToSpawn2) +
		" E3= " + Util::intToString(enemyTypeToSpawn3);
	smh->setDebugText(debugText);
	
	smh->enemyManager->addEnemy(enemy,Util::getGridX(x),Util::getGridY(y),0.15,0.15,-1);
	timeOfLastSpawn = smh->getGameTime();
}






