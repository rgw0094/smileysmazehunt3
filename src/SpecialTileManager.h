#ifndef _SPECIALTILEMANAGER_H_
#define _SPECIALTILEMANAGER_H_

#include <list>

class CollisionCircle;
class hgeParticleManager;
class hgeParticleSystem;
class hgeRect;

struct Mushroom {
	int state;
	
	int gridX,gridY;
	double x,y;
	CollisionCircle *mushroomCollisionCircle;
	
	double beginGrowTime;
	double beginExplodeTime;

	int graphicsIndex; //This is needed since there are 2 different mushroom graphics
};

struct Flame {
	float x, y;
	hgeParticleSystem *particle;
	hgeRect *collisionBox;
	float timeFlamePutOut;
};

struct SillyPad {
	int gridX, int gridY;
	float timePlaced;
};

struct IceBlock {
	int gridX, int gridY;
	bool hasBeenMelted;
	float timeMelted;
};

struct TimedTile {
	int gridX, gridY, newTile, oldTile;
	float duration, timeCreated, alpha;
};

class SpecialTileManager {

public:

	//Methods
	SpecialTileManager();
	~SpecialTileManager();

	void draw(float dt);
	void update(float dt);
	void reset();

	void addTimedTile(int gridX, int gridY, int tile, float duration);
	void updateTimedTiles(float dt);
	void drawTimedTiles(float dt);
	void resetTimedTiles();
	bool isTimedTileAt(int gridX, int gridY);
	bool isTimedTileAt(int gridX, int gridY, int tile);

	void addMushroom(int _gridX,int _gridY, int _graphicsIndex);
	void updateMushrooms(float dt);
	void drawMushrooms(float dt);
	void resetMushrooms();

	void addFlame(int gridX, int gridY);
	void updateFlames(float dt);
	void drawFlames(float dt);
	void resetFlames();

	void addSillyPad(int gridX, int gridY);
	void updateSillyPads(float dt);
	void drawSillyPads(float dt);
	void resetSillyPads();
	bool isSillyPadAt(int gridX, int gridY);
	bool destroySillyPad(int gridX, int gridY);

	void addIceBlock(int gridX, int gridY);
	void updateIceBlocks(float dt);
	void drawIceBlocks(float dt);
	void resetIceBlocks();

	//Variables		
	std::list<Flame> flameList;
	std::list<SillyPad> sillyPadList;
	std::list<Mushroom> theMushrooms; //Linked list containing the mushrooms
	std::list<IceBlock> iceBlockList;
	std::list<TimedTile> timedTileList;

	hgeParticleManager *explosions; //This contains the particle effects (graphics) of the explosions	
	hgeRect *collisionBox;	//general purpose collision box

};

#endif