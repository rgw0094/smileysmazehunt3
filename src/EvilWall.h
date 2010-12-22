#ifndef _EVIL_WALL_H_
#define _EVIL_WALL_H_

class hgeRect;


#define EVIL_WALL_STATE_IDLE 0
#define EVIL_WALL_STATE_APPEARING 1
#define EVIL_WALL_STATE_MOVING 2
#define EVIL_WALL_STATE_FADING 3
#define EVIL_WALL_COLLISION_CIRCLE_IN 4
#define EVIL_WALL_COLLISION_CIRCLE_OUT 5
#define EVIL_WALL_APPEAR_TIME 1.0
#define EVIL_WALL_DAMAGE 0.0

class EvilWall {
public:

	EvilWall();
	~EvilWall();

	//Methods
	void activate();
	void deactivate();
	bool update(float dt); //returns true if smiley hit it
	void draw(float dt);
	void drawEvilWall();
	bool doCollision(); //returns true if smiley hit it

	//Mutators
	void setBeginWallPosition(int gridX, int gridY);
	void setSmileyRestartPosition(int gridX, int gridY);
	void setDir(int _dir);
	void setSpeed(int _speed);
	void setState(int _state);
	
	//Variables
	int evilWallID;
	int state;
	int dir;
	int xBeginWall, yBeginWall; //what grid spot the beginning is located at
	double xPosition,yPosition; //where the wall currently is
	int speed; //pixels/second
	int restartX,restartY; //where smiley restarts if he fails
	float beginAppearTime, beginFadeTime, beginWallMoveTime;

	hgeRect *collisionRect;
};

#endif