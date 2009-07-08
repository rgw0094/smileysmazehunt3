#include "SmileyEngine.h"
#include "EnemyFramework.h"
#include "hgeresource.h"
#include "ProjectileManager.h"
#include "player.h"
#include "environment.h"
#include "CollisionCircle.h"

extern SMH *smh;

#define TIME_BETWEEN_SPAWNS 8.0
#define MAX_DISTANCE_FROM_SMILEY_TO_SPAWN 400

#define SPAWN_STATE_NOT_SPAWNING 0
#define SPAWN_STATE_ENEMY_FALLING 1
#define SPAWN_STATE_ENEMY_GROWING 2


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
	spawnState = SPAWN_STATE_NOT_SPAWNING;
	newEnemySize = 0.2;
	newEnemyY = 0.0;

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
	if (spawnState != SPAWN_STATE_NOT_SPAWNING) {
		graphic[facing]->RenderEx(smh->getScreenX(x),smh->getScreenY(y) - newEnemyY,0.0,newEnemySize,newEnemySize);
	}

	graphic[facing]->Update(dt);
	graphic[facing]->Render(screenX, screenY - shadowOffset);
	smh->resources->GetSprite("playerShadow")->Render(screenX, screenY);

	if (smh->isDebugOn()) {
		smh->drawCollisionBox(collisionBox, Colors::RED);
	}
}

/**
 * Updates the spawner.
 */
void E_Spawner::update(float dt) {
	
	doAStar();

	if (smh->timePassedSince(timeOfLastSpawn) >= TIME_BETWEEN_SPAWNS &&
		Util::distance(x,y,smh->player->x,smh->player->y) <= MAX_DISTANCE_FROM_SMILEY_TO_SPAWN &&
		smh->environment->validPath(x, y, smh->player->x, smh->player->y, 13, canPass) &&
		spawnState == SPAWN_STATE_NOT_SPAWNING) 
	{
		spawnEnemy();
	}

	//Update spawning
	updateSpawn(dt);

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

	//Don't move while spawning an enemy
	if (spawnState != SPAWN_STATE_NOT_SPAWNING) {
		dx=dy=0.0;
	}	
	move(dt);
}

/**
 * Spawns an enemy.
 */
void E_Spawner::spawnEnemy() {
	
	int randomInt;
	
	randomInt = smh->hge->Random_Int(0,100);

	if (randomInt <= 45) {				//45% chance of enemy type 1
		newEnemyID = enemyTypeToSpawn1;
	} else if (randomInt <= 80) {		//35% chance of enemy type 2
		newEnemyID = enemyTypeToSpawn2;
	} else {							//20% chance of enemy type 3
		newEnemyID = enemyTypeToSpawn3;
	}

	newEnemySize = 0.32;
	newEnemyY = shadowOffset;
	spawnState = SPAWN_STATE_ENEMY_FALLING;
	
	std::string debugText;
	debugText = "E_Spawner.cpp begins to spawn enemy at";
	smh->setDebugText(debugText);

	smh->hge->System_Log("%d", groupID);
}

/**
 * Updates the spawning (enemy falling to ground, enemy growing)
 */
void E_Spawner::updateSpawn(float dt) {
	if (spawnState == SPAWN_STATE_NOT_SPAWNING) {
		return;
	} else if (spawnState == SPAWN_STATE_ENEMY_FALLING) {
		newEnemyY -= dt*40.0;
		if (newEnemyY <= 0.0) {
			newEnemyY = 0.0;
			spawnState = SPAWN_STATE_ENEMY_GROWING;
		}
	} else { //enemy_growing
		newEnemySize += dt;
		if (newEnemySize >= 1.0) {
			newEnemySize = 1.0;
			spawnState = SPAWN_STATE_NOT_SPAWNING;

			smh->enemyManager->addEnemy(newEnemyID,Util::getGridX(x),Util::getGridY(y),0.15,0.15,groupID);
			smh->enemyGroupManager->addEnemy(groupID);

			timeOfLastSpawn = smh->getGameTime();
		}
	}
}





