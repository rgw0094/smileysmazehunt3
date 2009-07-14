#include "SmileyEngine.h"
#include "EnemyFramework.h"
#include "environment.h"
#include "player.h"

#include "hgeresource.h"
#include "hgeanim.h"

extern SMH *smh;

#define NORTH 0
#define EAST 1
#define SOUTH 2
#define WEST 3
#define NORTHEAST 4
#define SOUTHEAST 5
#define SOUTHWEST 6
#define NORTHWEST 7
#define CENTER 8

#define FALL_TIME 1.3


/** 
 * Constructor
 */
E_FenwarEyeSpider::E_FenwarEyeSpider(int id, int x, int y, int groupID) {
		
	//Call parent initialization routine
	initEnemy(id, x, y, groupID);
	
	//Set initial state
	facing = DOWN;
	timeStartedHop = timeStoppedHop = smh->getGameTime();
	hopYOffset = 0.0;
	hopping = false;
	dx = dy = 0.0;

	graphic[0]->SetSpeed(0.5);
	graphic[1]->SetSpeed(0.5);
	graphic[2]->SetSpeed(0.5);
	graphic[3]->SetSpeed(0.5);

	jumpTimeIntervalMin = 0.5;
	jumpTimeIntervalMax = 1.5;
	currentJumpTimeInterval = 1.3; //will be Random

	centerX = variable1;
	centerY = variable2;

	figureOutPosition(); //where are we in the "tic-tac-toe board" of Hoverpads?

	canPass[PIT] = true;
}

/**
 * Destructor
 */
E_FenwarEyeSpider::~E_FenwarEyeSpider() {

}

void E_FenwarEyeSpider::update(float dt) {
	if (falling) {
		fallingRot += dt;
		fallingScale -= 0.75*dt;
		move(dt);
		if (fallingScale < 0.1) fallingScale = 0.1;
		if (smh->timePassedSince(beganFalling) >= FALL_TIME) {
			health = 0.0;
		}
		return;
	}

	if (!hopping && smh->timePassedSince(timeStoppedHop) > currentJumpTimeInterval) {
		
		//Start next hop
		hopping = true;
		timeStartedHop = smh->getGameTime();

		// Find a new destination square and hop toward it
		figureOutPosition();
		chooseNewDestination();
		hopDistance = Util::distance(x,y,destX,destY);
		hopAngle = Util::getAngleBetween(x,y,destX,destY);
			
		timeToHop = hopDistance / float(speed);
		dx = speed * cos(hopAngle);
		dy = speed * sin(hopAngle);

		setFacing();
	}

	if (hopping) {		
		hopYOffset = (hopDistance / 3.0) * sin((smh->timePassedSince(timeStartedHop)/timeToHop) * PI);
		collisionBox->SetRadius(x, y - hopYOffset, radius);
		
		if (smh->timePassedSince(timeStartedHop) > timeToHop) {
			hopping = false;
			timeStoppedHop = smh->getGameTime();
			dx = dy = hopYOffset = 0.0;
			//currentJumpTimeInterval = smh->randomFloat(jumpTimeIntervalMin,jumpTimeIntervalMax);
		}
	} else { //if not hopping, face smiley
        //setFacingPlayer();
	}

	move(dt);

	if (!hopping && smh->environment->collision[gridX][gridY] == PIT) { //fall to doom
		falling = true;
		beganFalling = smh->getGameTime();
		fallingRot=0.0;
		fallingScale=1.0;
	}


}

void E_FenwarEyeSpider::draw(float dt) {
	int i;

	if (smh->environment->collision[gridX][gridY] != PIT) smh->resources->GetSprite("playerShadow")->Render(screenX, screenY + 32.0);

	if (falling) {
			graphic[0]->RenderEx(screenX,screenY,fallingRot,fallingScale,fallingScale);

	} else {
		
		if (graphic[0]->GetFrames() > 1) { //animate by 'crouching down' just before a jump
			if (!hopping && smh->timePassedSince(timeStoppedHop) > currentJumpTimeInterval - 0.3)
				//Set frame to 'crouched' position -- as if ready to hop
				for (i=0; i<4; i++) graphic[i]->SetFrame(1);
			else
				//Set frame to normal position
				for (i=0; i<4; i++) graphic[i]->SetFrame(0);
		}

		//Render the graphic
		graphic[facing]->Render(screenX, screenY - hopYOffset);
	} //not falling
}


void E_FenwarEyeSpider::figureOutPosition() {
	int xx = gridX - centerX;
	int yy = gridY - centerY;

	if (xx < -3) {
		if (yy < -3) position = NORTHWEST;
		else if (yy < 3) position = WEST;
		else position = SOUTHWEST;
	} else if (xx < 3) {
		if (yy < -3) position = NORTH;
		else if (yy < 3) position = CENTER;
		else position = SOUTH;
	} else {
		if (yy < -3) position = NORTHEAST;
		else if (yy < 3) position = EAST;
		else position = SOUTHEAST;
	}
}

void E_FenwarEyeSpider::chooseNewDestination() {
	int rnd;
	switch (position) {
		case NORTH:
			rnd = smh->randomInt(0,3);
			if (rnd == 0)		{destinationX = centerX-6; destinationY = centerY-6;}
			else if (rnd == 1)	{destinationX = centerX+6; destinationY = centerY-6;}
			else if (rnd == 2)	{destinationX = centerX-6; destinationY = centerY;}
			else				{destinationX = centerX+6; destinationY = centerY;}
			break;
		case EAST:
			rnd = smh->randomInt(0,3);
			if (rnd == 0)		{destinationX = centerX+6; destinationY = centerY-6;}
			else if (rnd == 1)	{destinationX = centerX+6; destinationY = centerY+6;}
			else if (rnd == 2)	{destinationX = centerX; destinationY = centerY-6;}
			else				{destinationX = centerX; destinationY = centerY+6;}
			break;
		case SOUTH:
			rnd = smh->randomInt(0,3);
			if (rnd == 0)		{destinationX = centerX-6; destinationY = centerY+6;}
			else if (rnd == 1)	{destinationX = centerX+6; destinationY = centerY+6;}
			else if (rnd == 2)	{destinationX = centerX-6; destinationY = centerY;}
			else				{destinationX = centerX+6; destinationY = centerY;}
			break;
		case WEST:
			rnd = smh->randomInt(0,3);
			if (rnd == 0)		{destinationX = centerX-6; destinationY = centerY-6;}
			else if (rnd == 1)	{destinationX = centerX-6; destinationY = centerY+6;}
			else if (rnd == 2)	{destinationX = centerX; destinationY = centerY-6;}
			else				{destinationX = centerX; destinationY = centerY+6;}
			break;
		case NORTHEAST:
			rnd = smh->randomInt(0,1);
			if (rnd == 0)	{destinationX = centerX; destinationY = centerY-6;}
			else			{destinationX = centerX+6; destinationY = centerY;}
			break;
		case SOUTHEAST:
			rnd = smh->randomInt(0,1);
			if (rnd == 0)	{destinationX = centerX; destinationY = centerY+6;}
			else			{destinationX = centerX+6; destinationY = centerY;}
			break;
		case SOUTHWEST:
			rnd = smh->randomInt(0,1);
			if (rnd == 0)	{destinationX = centerX; destinationY = centerY+6;}
			else			{destinationX = centerX-6; destinationY = centerY;}
			break;
		case NORTHWEST:
			rnd = smh->randomInt(0,1);
			if (rnd == 0)	{destinationX = centerX; destinationY = centerY-6;}
			else			{destinationX = centerX-6; destinationY = centerY;}
			break;
		default:
			destinationX = centerX-6; destinationY = centerY-6;
			break;
	};
	destX = destinationX*64+32;
	destY = destinationY*64+32;
}
/**
 * Overrides the tongue collision method so that the hopper's hop can be
 * interrupted when hit by Smiley's tongue.
 *
 * Actually I'm probably just going to delete this
bool E_Hopper::doTongueCollision(Tongue *tongue, float damage) {
	
	//Check collision
	if (tongue->testCollision(collisionBox)) {
			
		//Make sure the enemy wasn't already hit by this attack
		if (smh->timePassedSince(lastHitByWeapon) > .5) {
			lastHitByWeapon = smh->getGameTime();
			//If hopping, stop
			if (hopping) {
				dx = dy = 0.0;
			}
			dealDamageAndKnockback(damage, 65.0, smh->player->x, smh->player->y);
			startFlashing();
			return true;
		}

	}

	return false;
}
*/