#ifndef _ENVIRONMENT_H_
#define _ENVIRONMENT_H_

class Tongue;
class hgeParticleSystem;
class CollisionCircle;
class hgeParticleManager;
class hgeRect;
class hgeFont;
class hgeAnimation;
class BaseEnemy;
class SpecialTileManager;
class EvilWallManager;
class TapestryManager;

//-------- Collision Layer ---------//
#define NUM_COLLISION 36
#define WALKABLE 0
#define UNWALKABLE 1
#define RED_KEYHOLE 2
#define YELLOW_KEYHOLE 3
#define GREEN_KEYHOLE 4
#define BLUE_KEYHOLE 5
#define EVIL_DOOR 6
#define SHALLOW_WATER 7
#define DEEP_WATER 8
#define UP_ARROW 9
#define RIGHT_ARROW 10
#define DOWN_ARROW 11
#define LEFT_ARROW 12
#define SLIME 13
#define SPRING_PAD 14
#define FIRE_DESTROY 15
#define WALK_LAVA 16
#define NO_WALK_LAVA 17
#define UNWALKABLE_PROJECTILE 18
#define RED_WARP 19
#define BLUE_WARP 20
#define YELLOW_WARP 21
#define GREEN_WARP 22
#define SPIN_ARROW_SWITCH 23
#define PIT 24
#define FOUNTAIN 25
#define SAVE_SHRINE 26
#define SIGN 27
#define ICE 28
#define MIRROR_UP_LEFT 29
#define MIRROR_UP_RIGHT 30
#define MIRROR_DOWN_RIGHT 31
#define MIRROR_DOWN_LEFT 32
#define MIRROR_SWITCH 33
#define ENEMY_NO_WALK 34
#define GREEN_WATER 35
#define DIZZY_MUSHROOM_1 36
#define DIZZY_MUSHROOM_2 37
#define BOMB_PAD_UP 38
#define BOMB_PAD_DOWN 39
#define BOMBABLE_WALL 40
#define HOVER_PAD 41
#define WHITE_CYLINDER_DOWN 42
#define YELLOW_CYLINDER_DOWN 43
#define GREEN_CYLINDER_DOWN 44
#define BLUE_CYLINDER_DOWN 45
#define BROWN_CYLINDER_DOWN 46
#define SILVER_CYLINDER_DOWN 47
#define SHRINK_TUNNEL_SWITCH 48
#define SHRINK_TUNNEL_HORIZONTAL 49
#define SHRINK_TUNNEL_VERTICAL 50
#define SHALLOW_GREEN_WATER 51
#define EVIL_WALL_POSITION 52
#define EVIL_WALL_TRIGGER 53
#define EVIL_WALL_DEACTIVATOR 54
#define EVIL_WALL_RESTART 55
#define FLAME 56
//...
#define WHITE_CYLINDER_UP 58
#define YELLOW_CYLINDER_UP 59
#define GREEN_CYLINDER_UP 60
#define BLUE_CYLINDER_UP 61
#define BROWN_CYLINDER_UP 62
#define SILVER_CYLINDER_UP 63
//...
#define WHITE_SWITCH_LEFT 74
#define YELLOW_SWITCH_LEFT 75
#define GREEN_SWITCH_LEFT 76
#define BLUE_SWITCH_LEFT 77
#define BROWN_SWITCH_LEFT 78
#define SILVER_SWITCH_LEFT 79
//...
#define WHITE_SWITCH_RIGHT 90
#define YELLOW_SWITCH_RIGHT 91
#define GREEN_SWITCH_RIGHT 92
#define BLUE_SWITCH_RIGHT 93
#define BROWN_SWITCH_RIGHT 94
#define SILVER_SWITCH_RIGHT 95
//...
#define PLAYER_START 224
#define PLAYER_END 225

class Environment {

public:
	Environment();
	~Environment();

	//methods
	void draw(float dt);
	void drawAfterSmiley(float dt);
	void drawGrid(float dt);
	void update(float dt);
	void loadArea(int id, int from, int playerX, int playerY);
	int gatherItem(int x, int y);
	int collisionAt(float x, float y);
	bool playerCollision(int x, int y,float dt);
	bool enemyCollision(hgeRect *box, BaseEnemy *enemy, float dt);
	void unlockDoor(int gridX, int gridY);
	bool toggleSwitches(hgeRect* box);
	bool toggleSwitches(hgeRect* box, bool playSoundFarAway);
	bool toggleSwitches(Tongue *tongue);
	bool toggleSwitchAt(int gridX, int gridY, bool playSoundFarAway);
	void toggleSwitch(int id);
	bool hitSigns(Tongue *tongue);
	bool hitSaveShrine(Tongue *tongue);
	bool validPath(int x1, int y1, int x2, int y2, int radius, bool canPass[256]);
	void drawFountain();
	bool testCollision(hgeRect *box, bool canPass[256], bool ignoreSillyPads);
	bool testCollision(hgeRect *box, bool canPass[256]);
	bool playerOnCylinder(int x, int y);
	void switchCylinders(int id);
	void flipCylinderSwitch(int gridX, int gridY);
	bool isDeepWaterAt(int x, int y);
	bool isShallowWaterAt(int x, int y);
	bool isArrowAt(int x,int y);
	void bombWall(int x,int y);
	void setTerrainCollisionBox(hgeRect *box, int whatFor, int gridX, int gridY);
	void placeSillyPad(int gridX, int gridY);
	bool hasSillyPad(int gridX, int gridY);

	//Objects
	hgeRect *collisionBox;
    
	//variables
	int areaWidth,areaHeight;		//Width and height of the area in squares
	int ids[256][256];				//ID Layer	
	int variable[256][256];			//Variable Layer
	int terrain[256][256];			//Main Layer
	int collision[256][256];		//Walk Layer
	int item[256][256];				//Item Layer
	int enemyLayer[256][256];		//Enemy Layer
	float activated[256][256];		//What time things were activated on each square
	int screenWidth, screenHeight;	//Number of squares that fit on the screen
	int xGridOffset,yGridOffset;	//Number of squares the top left corner is from (0,0)
	float xOffset,yOffset;			//Number of pixels the player is off alignment with the grid
	int startX, int startY;			//Level entrance
	bool fountainOnScreen;
	float drawX,drawY;				//Location to draw a tile
	int offScreenRange;				//Number of tiles offscreen to draw
	bool hasFountain;				//Whether or not this area has a fountain

	SpecialTileManager *specialTileManager;
	EvilWallManager *evilWallManager; //Evil walls which move and try to kill smiley
	TapestryManager *tapestryManager;
	
	hgeAnimation *silverCylinder, *brownCylinder, *blueCylinder, *greenCylinder, *yellowCylinder, *whiteCylinder;
	hgeAnimation *silverCylinderRev, *brownCylinderRev, *blueCylinderRev, *greenCylinderRev, *yellowCylinderRev, *whiteCylinderRev;
	hgeParticleManager *environmentParticles;

};

#endif