#ifndef PLAYER_H_
#define PLAYER_H_

#define KNOCKBACK_DURATION 0.2
#define INITIAL_MANA 50.0
#define MANA_REGENERATE_RATE 6.0
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
	void drawJesusBeam();
	void update(float dt);
	void updateGUI(float dt);
	void moveTo(int gridX, int gridY);
	bool canPass(int collision);
	bool canPass(int collision, bool applyCurrentAbilities);
	void dealDamage(float damage, bool makesFlash);
	void dealDamageAndKnockback(float damage, bool makesFlash, float knockbackDist, float knockbackerX, float knockbackerY);
	void dealDamageAndKnockback(float damage, bool makesFlash, bool alwaysKnockback, float knockbackDist, float knockbackerX, float knockbackerY);
	void freeze(float duration);
	void stun(float duration);
	void heal(float amount);
	void slime(float duration);
	void immobilize(float duration);
	void reset();
	void stopFireBreath();
	void stopMovement();
	void graduallyMoveTo(float x, float y, float speed);
	void resetTongue();

	//Accessors/mutators
	bool isInvisible();
	bool isReflectingProjectiles();
	bool isOnIce();
	bool isOnArrow();
	bool isShrunk();
	bool isFlashing();
	bool isSpringing();
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
	void resetSliding();

	WormNode getWormNode(int num);
	Tongue *getTongue();

	//variables that might be directly accessed by other objects
	float scale,hoverScale,shrinkScale;		//Scales to draw smiley
	float rotation;							//Smiley's sprite rotation angle in radians
	float radius;
	int facing;								//Direction Smiley is facing
	float x, y;								//Global coordinates
	int baseX, baseY;						//The coordinate of the center of smiley's shadow
	int baseGridX, baseGridY;				//The grid coordinate of the center of smiley's shadow
	int gridX,gridY;						//Global grid coordinates
	int lastGridX, lastGridY;
	int lastNonIceGridX, lastNonIceGridY;
	float springOffset;
	float hoveringYOffset;
	bool shrinkActive;
	int startedFallingX;					//X position where the player started falling
	int startedFallingY;					//Y position where the player started falling

	bool needToIceCenter;

	float startedSlidingX, startedSlidingY;
	float finishSlidingX, finishSlidingY;
	int slideDir;
	float timeToSlideOnArrow;
	bool slidingOntoIce;

	//Constants
	float angles[8];
	float mouthXOffset[8];
	float mouthYOffset[8];

	bool dontUpdate;
	bool invincible;
	bool uber;
	bool abilitiesLocked;
	bool tongueLocked;

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
	void checkForIceGlitch();
	void updateJesusSound();

	Tongue *tongue;
	Worm *worm;
	
	float health, mana;
	float springVelocity;
	int enteredWaterX, enteredWaterY;		//Grid position the player was on before entering water
	int enteredSpringX, enteredSpringY;
	bool usingManaItem;
	float fallingDx, fallingDy;
	float frisbeePower;
	float dx, dy;
	float graduallyMoveTargetX, graduallyMoveTargetY;

	//Time variables
	float sprintDuration;			//Remaining time that smiley can sprint. Slowly recharges.
	float startedFlashing;
	float startedKnockBack;
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
	float timeSlimed;
	float slimeDuration;
	float timeStartedGraduallyMoving;
	float timeToGraduallyMove;
	float timeStoppedBreathingFire;
	float timeLastUsedMana;
	
	//Stuff needed for the centering of Smiley during the "hop" onto ice
	float startIceTime;
	float startIceX;
	float startIceY;
	float endIceX;
	float endIceY;

	//Stuff for the "hop" onto ice
	bool needToIceHop;
	float timeStartedIceHop;
	float iceHopOffset;

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
	bool inShrinkTunnel;
	bool frozen;
	bool stunned;
	bool healing;
	bool immobile;
	bool slimed;
	bool chargingFrisbee;
	bool graduallyMoving;
	bool jesusSoundPlaying;
};

//----------------------------------------------------------------
//------------------ GUI -----------------------------------------
//----------------------------------------------------------------
class GUI 
{
public:

	GUI();
	~GUI();

	void update(float dt);
	void draw();
	void toggleAvailableAbility(int ability);
	bool isAbilityAvailable(int ability);
	bool areAbilitySlotsEmpty();
	void abilityKeyPressedInInventoryScreen(int abilityNum, int ability);
	int getUsedAbility();
	int getAbilityInSlot(int slot);
	void setAbilityInSlot(int ability, int slot);
	void resetAbilities();

private:

	int activeAbilities[3];
};

//----------------------------------------------------------------
//------------------ WORM ----------------------------------------
//----------------------------------------------------------------
struct WormNode 
{
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
//------------------ TONGUE --------------------------------------
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
	void resetAttack();

	bool dontPlaySound;

private:

	void playSound();

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