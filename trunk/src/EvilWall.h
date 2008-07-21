#ifndef _EVIL_WALL_H_
#define _EVIL_WALL_H_

class hgeRect;

class EvilWall {
public:

	EvilWall();
	~EvilWall();

	//Methods
	void activate();
	void deactivate();
	void update(float dt);
	void draw(float dt);
	void drawEvilWall();
	void doCollision();

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