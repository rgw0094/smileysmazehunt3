#ifndef PLAYER_H_
#define PLAYER_H_

#define KNOCKBACK_DURATION 0.2

class CollisionCircle;
class Tongue;
class WeaponParticleSystem;
class hgeRect;

struct velocity {
	double x,y;
	float timeBegan,duration;
};

class Player {

public:
	Player(int x, int y);
	~Player();

	//methods
	void draw(float dt);
	void drawGUI(float dt);
	void update(float dt);
	void moveTo(int gridX, int gridY);
	void move(float x, float y, float dt);
	bool canPass(int collision);
	void changeAbility(int direction);
	void doAbility(float dt);
	void doWarps();
	void doFalling(float dt);
	void doSprings(float dt);
	void doArrowPads(float dt);
	void doItems();
	void doWater();
	void setFacingStraight();
	void doMovement(float dt);
	void doIce(float dt);
	void dealDamage(float damage, bool makesFlash);
	void dealDamageAndKnockback(float damage, bool makesFlash, float knockbackDist, float knockbackerX, float knockbackerY);
	void dealDamageAndKnockback(float damage, bool makesFlash, bool alwaysKnockback, float knockbackDist, float knockbackerX, float knockbackerY);
	void freeze(float duration);
	void doShrinkTunnels(float dt);

	//Accessors/mutators
	bool isInvisible();
	bool isReflectingProjectiles();
	bool isOnIce();
	bool isShrunk();
	bool isInShrinkTunnel();
	void setHealth(float amount);
	float getHealth();
	float getMaxHealth();
	float getMaxMana();
	float getDamage();
	float getFireBreathDamage();
	float getLightningOrbDamage();
	void modifyVelocity(double xVel,double yVel);
	Tongue *getTongue();

	//variables
	float scale,hoverScale,shrinkScale;	//Scales to draw smiley
	float rotation;							//Smiley's sprite rotation angle in radians
	float radius;
	int facing;								//Direction Smiley is facing
	float x, y;								//Global coordinates
	float lastX, lastY;						//Last frame's x and y
	int baseX, baseY;						//The coordinate of the center of smiley's shadow
	int baseGridX, baseGridY;				//The grid coordinate of the center of smiley's shadow
	int gridX,gridY;						//Global grid coordinates
	int lastGridX, lastGridY;
	float springOffset;
	float screenX, screenY;					//Screen coordinates
	float speed;
	float dx, dy;
	float alpha;
	float mana;
	CollisionCircle *collisionCircle;
	int selectedAbility;
	float speedModifier;
	int startedFallingX;					//X position where the player started falling
	int startedFallingY;					//Y position where the player started falling
	int enteredWaterX, int enteredWaterY;	//Grid position the player was on before entering water
	int enteredSpringX, enteredSpringY;
	float hoveringYOffset;					//Y Offset for player sprite while hovering
	int startSpringX, startSpringY;
	float fallingDx, fallingDy;

	//Constants
	float angles[8];
	float mouthXOffset[8];
	float mouthYOffset[8];

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
	float lastMove;					//Last time a movement key was pressed
	float lastOrb;
	float timeToSlide;
	float startedCane;
	float springTime;				//How long to be in the air after touching a spring pad
	float lastHit;					//Last time smiley was hit by something
	float timeEnteredShrinkTunnel;  //Time smiley entered the shrink tunnel
	float timeInShrinkTunnel;		//Time to take to go through the shrink tunnel
	float timeStartedHovering;

	//Graphics
	WeaponParticleSystem *fireBreathParticle;
	WeaponParticleSystem *iceBreathParticle;

private:

	float health;
	
	float timeFrozen, freezeDuration;
	float springVelocity;
	Tongue *tongue;
	
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
	bool shrinkActive;
	bool sprinting;					//If sprint boots are being used
	bool isHovering;
	bool cloaked;
	bool usingCane;
	bool inShrinkTunnel;
	bool frozen;

};

#endif