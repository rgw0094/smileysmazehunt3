#ifndef _WORM_H_
#define _WORM_H_

#include <list>

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


#endif