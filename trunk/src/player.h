#ifndef PLAYER_H_
#define PLAYER_H_

#define KNOCKBACK_DURATION 0.2
#define INITIAL_MANA 50.0
#define MANA_PER_UPGRADE 20.0
#define MANA_REGENERATE_RATE 5.0
#define MANA_PER_ITEM 25.0

#include <list>

class CollisionCircle;
class Tongue;
class WeaponParticleSystem;
class hgeRect;
class Worm;
class hgeRect;
class GUI;
struct WormNode;

//----------------------------------------------------------------
//------------------ PLAYER --------------------------------------
//----------------------------------------------------------------
class Player {

public:
	Player();
	~Player();

	void draw(float dt);
	void drawGUI(float dt);
	void update(float dt);
	void moveTo(int gridX, int gridY);
	bool canPass(int collision);
	void dealDamage(float damage, bool makesFlash);
	void dealDamageAndKnockback(float damage, bool makesFlash, float knockbackDist, float knockbackerX, float knockbackerY);
	void dealDamageAndKnockback(float damage, bool makesFlash, bool alwaysKnockback, float knockbackDist, float knockbackerX, float knockbackerY);
	void freeze(float duration);
	void stun(float duration);
	void heal(float amount);
	void immobilize(float duration);
	void reset();

	//Accessors/mutators
	bool isInvisible();
	bool isReflectingProjectiles();
	bool isOnIce();
	bool isShrunk();
	bool isFlashing();
	bool isInShrinkTunnel();
	void setHealth(float amount);
	void setMana(float amount);
	float getMana();
	float getHealth();
	float getMaxHealth();
	float getMaxMana();
	float getDamage();
	float getFireBreathDamage();
	float getLightningOrbDamage();
	bool isSmileyTouchingWater();

	WormNode getWormNode(int num);
	Tongue *getTongue();

	//variables that might be directly accessed by other objects (which isn't great)
	float scale,hoverScale,shrinkScale;		//Scales to draw smiley
	float rotation;							//Smiley's sprite rotation angle in radians
	float radius;
	int facing;								//Direction Smiley is facing
	float x, y;								//Global coordinates
	int baseX, baseY;						//The coordinate of the center of smiley's shadow
	int baseGridX, baseGridY;				//The grid coordinate of the center of smiley's shadow
	int gridX,gridY;						//Global grid coordinates
	int lastGridX, lastGridY;
	float springOffset;
	float hoveringYOffset;
	bool shrinkActive;

	//Constants
	float angles[8];
	float mouthXOffset[8];
	float mouthYOffset[8];

	bool invincible;

	CollisionCircle *collisionCircle;
	WeaponParticleSystem *fireBreathParticle;
	WeaponParticleSystem *iceBreathParticle;
	GUI *gui;

private:

	//Private methods
	void updateLocation();
	void doMove(float dt);
	void doAbility(float dt);
	void doWarps();
	void doFalling(float dt);
	void doSprings(float dt);
	void doArrowPads(float dt);
	void doItems();
	void doWater();
	void setFacingStraight();
	void setFacingDirection();
	void updateVelocities(float dt);
	void doIce(float dt);
	void startPuzzleIce();
	void doShrinkTunnels(float dt);
	void updateSmileyColor(float dt);
	bool doGayMovementFix(int xDist, int yDist);

	Tongue *tongue;
	Worm *worm;
	
	float health, mana, 
	float springVelocity;
	int startedFallingX;					//X position where the player started falling
	int startedFallingY;					//Y position where the player started falling
	int enteredWaterX, enteredWaterY;		//Grid position the player was on before entering water
	int enteredSpringX, enteredSpringY;
	bool usingManaItem;
	float fallingDx, fallingDy;
	float dx, dy;

	//Time variables
	float startedFlashing;
	float startedKnockBack;
	float timeStartedCane;
	float startedSpringing;
	float startedAttacking;
	float stoppedAttacking;
	float startedIceBreath;
	float startedSliding;
	float startedFalling;
	float lastLavaDamage;			//Last time the player took damage from lava
	float startedWaterWalk;
	float startedDrowning;
	float lastOrb;
	float timeToSlide;
	float springTime;				//How long to be in the air after touching a spring pad
	float timeEnteredShrinkTunnel;  //Time smiley entered the shrink tunnel
	float timeInShrinkTunnel;		//Time to take to go through the shrink tunnel
	float timeStartedHovering;
	float timeFrozen;
	float freezeDuration;
	float stunDuration;
	float timeStartedStun;
	float timeStartedHeal;
	float timeStartedImmobilize;
	float immobilizeDuration;

	//State info
	bool breathingFire;				//If using Fire Breath ability
	bool breathingIce;				//If using Ice Breath ability
	bool onWarp;					//If on a warp square
	bool flashing;					//If flashing after being hit
	bool knockback;					//If being knocked back by enemy
	bool sliding;					//If sliding from arrow pads
	bool iceSliding;				//If sliding from ice
	bool springing;					//If airborne from spring pad
	bool reflectionShieldActive;
	bool falling;					//If falling into a pit
	bool inLava;					//If the player is on a lava tile
	bool inShallowWater;			//If the player is on a shallow water tile
	bool waterWalk;					//If the player is in water walk mode	
	bool onWater;					//If the player is on a water tile
	bool drowning;
	bool sprinting;					//If sprint boots are being used
	bool isHovering;
	bool cloaked;
	bool usingCane;
	bool inShrinkTunnel;
	bool frozen;
	bool stunned;
	bool healing;
	bool immobile;
};

//----------------------------------------------------------------
//------------------ GUI -----------------------------------------
//----------------------------------------------------------------
class GUI {

public:

	GUI();
	~GUI();

	void update(float dt);
	void draw();
	void addAvailableAbility(int ability);
	bool isAbilityAvailable(int ability);
	int getSelectedAbility();

private:

	void changeAbility(int direction);

	int availableAbilities[3];

};

//----------------------------------------------------------------
//------------------ WORM ----------------------------------------
//----------------------------------------------------------------
struct WormNode {
	int x,y;
	int dir;
};

class Worm {
public:
	Worm(int gridX,int gridY);
	~Worm();

	void update();
	void draw();
	void reset();
	WormNode getNode(int nodeNumber);

private:
	//Methods
	void addWormTrail(); //adds a trail of nodes 1 pixel apart from the first node to smiley's position
	void addWormNode(int x,int y,int direction);

	//Variables
	std::list<WormNode> theWorms;
};

//----------------------------------------------------------------
//------------------ WORM ----------------------------------------
//----------------------------------------------------------------
class Tongue {

public:
	Tongue();
	~Tongue();

	//methods
	void update(float dt);
	void draw(float dt);
	void startAttack();
	bool testCollision(hgeRect *collisionBox);
	bool testCollision(CollisionCircle *collisionCircle);
	bool isAttacking();

private:

	int tongueState;
	bool attacking;
	bool hasActivatedSomething;
	float timeStartedAttack;
	float tongueOffsetAngle;
	hgeRect *collisionBox;

	//Used for calculating collision
	float pointX, pointY, seperation, numPoints, testAngle;

};

#endif