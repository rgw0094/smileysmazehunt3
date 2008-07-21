#include "Tongue.h"
#include "Player.h"
#include "textbox.h"
#include "smiley.h"

#include "hgeresource.h"
#include "hgeanim.h"

extern hgeResourceManager *resources;
extern Player *thePlayer;
extern TextBox *theTextBox;
extern HGE *hge;
extern float gameTime;
extern bool debugMode;

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
	resources->GetAnimation("smileyTongue")->Play();
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
	if (attacking || theTextBox->visible || thePlayer->falling || thePlayer->springing || 
			thePlayer->cloaked || thePlayer->shrinkActive || thePlayer->drowning || 
			thePlayer->reflectionShieldActive) {
		return;
	}

	timeStartedAttack = gameTime;
	tongueOffsetAngle = -ATTACK_RADIUS / 2.0;
	attacking = true;
	resources->GetAnimation("smileyTongue")->SetFrame(0);
	resources->GetAnimation("smileyTongue")->SetMode(HGEANIM_FWD);
	resources->GetAnimation("smileyTongue")->Play();
	tongueState = STATE_EXTENDING;

}

void Tongue::update(float dt) {

	if (!attacking) return;

	if (tongueState == STATE_EXTENDING) {
		resources->GetAnimation("smileyTongue")->Update(dt);

		//Once the tongue is fully extended enter swinging state
		if (resources->GetAnimation("smileyTongue")->GetFrame() >= 10) {
			tongueState = STATE_SWINGING;
		}

	} else if (tongueState == STATE_SWINGING) {
		tongueOffsetAngle += 5.0 * PI * dt;
		if (tongueOffsetAngle > ATTACK_RADIUS / 2.0) tongueOffsetAngle = ATTACK_RADIUS / 2.0;

		//When tongue finishes swinging, start retracting it
		if (tongueOffsetAngle > ATTACK_RADIUS / 2.0) {
			resources->GetAnimation("smileyTongue")->SetFrame(10);
			resources->GetAnimation("smileyTongue")->SetMode(HGEANIM_REV);
			resources->GetAnimation("smileyTongue")->Play();
			tongueState = STATE_RETRACTING;
		}

	} else if (tongueState == STATE_RETRACTING) {
		resources->GetAnimation("smileyTongue")->Update(dt);

		//Once the tongue is fully retracted the attack is done
		if (resources->GetAnimation("smileyTongue")->GetFrame() < 1) {
			attacking = false;
		}

	}

}

/**
 * Draws smiley's tongue.
 */
void Tongue::draw(float dt) {
	if (attacking) {
		resources->GetAnimation("smileyTongue")->RenderEx(
			thePlayer->screenX + thePlayer->mouthXOffset[thePlayer->facing],
			thePlayer->screenY + thePlayer->mouthYOffset[thePlayer->facing] - thePlayer->hoveringYOffset,
			thePlayer->angles[thePlayer->facing] + (thePlayer->facing == LEFT ? -1 : 1) * tongueOffsetAngle, 
			thePlayer->scale, thePlayer->scale);

		//Draw tongue collision for debug mode
		if (debugMode) {
			numPoints = int((float)resources->GetAnimation("smileyTongue")->GetFrame() / (float)resources->GetAnimation("smileyTongue")->GetFrames() * (float)NUM_COLLISION_POINTS) + 1;
			for (int i = 0; i < numPoints; i++) {
				testAngle = -(PI / 2.0) + thePlayer->angles[thePlayer->facing] + (thePlayer->facing == LEFT ? -1 : 1) * tongueOffsetAngle;
				pointX = thePlayer->x + thePlayer->mouthXOffset[thePlayer->facing] + (i+1)*(TONGUE_LENGTH / (NUM_COLLISION_POINTS-1)) * cos(testAngle);
				pointY = thePlayer->y + thePlayer->mouthYOffset[thePlayer->facing] + (i+1)*(TONGUE_LENGTH / (NUM_COLLISION_POINTS-1)) * sin(testAngle);
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
	
	//Determine how many collision points to test based on the current length of the tongue
	numPoints = int((float)resources->GetAnimation("smileyTongue")->GetFrame() / (float)resources->GetAnimation("smileyTongue")->GetFrames() * (float)NUM_COLLISION_POINTS) + 1;
	
	for (int i = 0; i < numPoints; i++) {
		testAngle = -(PI / 2.0) + thePlayer->angles[thePlayer->facing] + (thePlayer->facing == LEFT ? -1 : 1) * tongueOffsetAngle;
		pointX = thePlayer->x + thePlayer->mouthXOffset[thePlayer->facing] + (i+1)*(TONGUE_LENGTH / (NUM_COLLISION_POINTS-1)) * cos(testAngle);
		pointY = thePlayer->y + thePlayer->mouthYOffset[thePlayer->facing] + (i+1)*(TONGUE_LENGTH / (NUM_COLLISION_POINTS-1)) * sin(testAngle);
		if (collisionBox->TestPoint(pointX, pointY)) return true;
	}
	return false;
}


