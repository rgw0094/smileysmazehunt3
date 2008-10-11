#include "SMH.h"
#include "Tongue.h"
#include "Player.h"
#include "smiley.h"
#include "Environment.h"
#include "EnemyManager.h"
#include "NPCManager.h"
#include "WindowFramework.h"
#include "CollisionCircle.h"

#include "hgeresource.h"
#include "hgeanim.h"

extern EnemyManager *enemyManager;
extern HGE *hge;
extern SMH *smh;

#define ATTACK_RADIUS (PI / 3.0)
#define NUM_COLLISION_POINTS 8
#define TONGUE_LENGTH 55.0

//Tongue states
#define STATE_EXTENDING 0
#define STATE_SWINGING 1
#define STATE_RETRACTING 2

/**
 * Constructor
 */
Tongue::Tongue() {
	attacking = false;
	timeStartedAttack = -10.0;
	smh->resources->GetAnimation("smileyTongue")->Play();
	collisionBox = new hgeRect();
}

/**
 * Destructor
 */
Tongue::~Tongue() {
	if (collisionBox) delete collisionBox;
}

/**
 * Start the tongue attack.
 */
void Tongue::startAttack() {

	//Make sure it is ok to attack
	if (attacking || smh->windowManager->isTextBoxOpen()) {
		return;
	}

	hasActivatedSomething = false;
	timeStartedAttack = smh->getGameTime();
	tongueOffsetAngle = -ATTACK_RADIUS / 2.0;
	attacking = true;
	smh->resources->GetAnimation("smileyTongue")->SetFrame(0);
	smh->resources->GetAnimation("smileyTongue")->SetMode(HGEANIM_FWD);
	smh->resources->GetAnimation("smileyTongue")->Play();
	tongueState = STATE_EXTENDING;

}

void Tongue::update(float dt) {

	if (!attacking) return;

	//Hit Enemies
	enemyManager->tongueCollision(this, smh->player->getDamage());
	
	//Activate stuff - only one thing can be activated per attack
	if (!hasActivatedSomething) {
		if (smh->environment->toggleSwitches(this) ||
				smh->npcManager->talkToNPCs(this) ||
				(!smh->windowManager->isOpenWindow() && smh->environment->hitSaveShrine(this)) ||
				(!smh->windowManager->isTextBoxOpen() && smh->environment->hitSigns(this))) {
			hasActivatedSomething = true;
		}
	}

	if (tongueState == STATE_EXTENDING) {
		smh->resources->GetAnimation("smileyTongue")->Update(dt);

		//Once the tongue is fully extended enter swinging state
		if (smh->resources->GetAnimation("smileyTongue")->GetFrame() >= 10) {
			tongueState = STATE_SWINGING;
		}

	} else if (tongueState == STATE_SWINGING) {
		tongueOffsetAngle += 8.0 * PI * dt;

		//When tongue finishes swinging, start retracting it
		if (tongueOffsetAngle > ATTACK_RADIUS / 2.0) {
			tongueOffsetAngle = ATTACK_RADIUS / 2.0;
			smh->resources->GetAnimation("smileyTongue")->SetFrame(10);
			smh->resources->GetAnimation("smileyTongue")->SetMode(HGEANIM_REV);
			smh->resources->GetAnimation("smileyTongue")->Play();
			tongueState = STATE_RETRACTING;
		}

	} else if (tongueState == STATE_RETRACTING) {
		smh->resources->GetAnimation("smileyTongue")->Update(dt);

		//Once the tongue is fully retracted the attack is done
		if (smh->resources->GetAnimation("smileyTongue")->GetFrame() < 1) {
			attacking = false;
		}

	}

}

/**
 * Draws smiley's tongue.
 */
void Tongue::draw(float dt) {
	if (attacking) {
		smh->resources->GetAnimation("smileyTongue")->RenderEx(
			getScreenX(smh->player->x) + smh->player->mouthXOffset[smh->player->facing],
			getScreenY(smh->player->y) - smh->player->springOffset + smh->player->mouthYOffset[smh->player->facing] - smh->player->hoveringYOffset,
			smh->player->angles[smh->player->facing] + (smh->player->facing == LEFT ? -1 : 1) * tongueOffsetAngle, 
			smh->player->scale, smh->player->scale);

		//Draw tongue collision for debug mode
		if (smh->isDebugOn()) {
			numPoints = int((float)smh->resources->GetAnimation("smileyTongue")->GetFrame() / (float)smh->resources->GetAnimation("smileyTongue")->GetFrames() * (float)NUM_COLLISION_POINTS) + 1;
			for (int i = 0; i < numPoints; i++) {
				testAngle = -(PI / 2.0) + smh->player->angles[smh->player->facing] + (smh->player->facing == LEFT ? -1 : 1) * tongueOffsetAngle;
				pointX = smh->player->x + smh->player->mouthXOffset[smh->player->facing] + (i+1)*(TONGUE_LENGTH / (NUM_COLLISION_POINTS-1)) * cos(testAngle);
				pointY = smh->player->y + smh->player->mouthYOffset[smh->player->facing] + (i+1)*(TONGUE_LENGTH / (NUM_COLLISION_POINTS-1)) * sin(testAngle);
				collisionBox->SetRadius(pointX, pointY, 5.0);
				drawCollisionBox(collisionBox, GREEN);
			}
		}

	}
}

/**
 * Returns whether or not the tongue is currently attacking
 */
bool Tongue::isAttacking() {
	return attacking;
}

/**
 * Returns whether or not a rectangle collides with the tongue.
 */
bool Tongue::testCollision(hgeRect *collisionBox) {
	
	if (!isAttacking()) return false;

	//Determine how many collision points to test based on the current length of the tongue
	numPoints = int((float)smh->resources->GetAnimation("smileyTongue")->GetFrame() / (float)smh->resources->GetAnimation("smileyTongue")->GetFrames() * (float)NUM_COLLISION_POINTS) + 1;
	
	for (int i = 0; i < numPoints; i++) {
		testAngle = -(PI / 2.0) + smh->player->angles[smh->player->facing] + (smh->player->facing == LEFT ? -1 : 1) * tongueOffsetAngle;
		pointX = smh->player->x + smh->player->mouthXOffset[smh->player->facing] + (i+1)*(TONGUE_LENGTH / (NUM_COLLISION_POINTS-1)) * cos(testAngle);
		pointY = smh->player->y + smh->player->mouthYOffset[smh->player->facing] + (i+1)*(TONGUE_LENGTH / (NUM_COLLISION_POINTS-1)) * sin(testAngle);
		if (collisionBox->TestPoint(pointX, pointY)) return true;
	}
	return false;
}


/**
 * Returns whether or not a circle collides with the tongue.
 */
bool Tongue::testCollision(CollisionCircle *collisionCircle) {
	
	if (!isAttacking()) return false;

	//Determine how many collision points to test based on the current length of the tongue
	numPoints = int((float)smh->resources->GetAnimation("smileyTongue")->GetFrame() / (float)smh->resources->GetAnimation("smileyTongue")->GetFrames() * (float)NUM_COLLISION_POINTS) + 1;
	
	for (int i = 0; i < numPoints; i++) {
		testAngle = -(PI / 2.0) + smh->player->angles[smh->player->facing] + (smh->player->facing == LEFT ? -1 : 1) * tongueOffsetAngle;
		pointX = smh->player->x + smh->player->mouthXOffset[smh->player->facing] + (i+1)*(TONGUE_LENGTH / (NUM_COLLISION_POINTS-1)) * cos(testAngle);
		pointY = smh->player->y + smh->player->mouthYOffset[smh->player->facing] + (i+1)*(TONGUE_LENGTH / (NUM_COLLISION_POINTS-1)) * sin(testAngle);
		if (collisionCircle->testPoint(pointX, pointY)) return true;
	}
	return false;
}


