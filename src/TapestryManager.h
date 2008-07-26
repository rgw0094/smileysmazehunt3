#ifndef TAPESTRYMANAGER_H
#define TAPESTRYMANAGER_H

#include <list>
#include "hge.h"

class hgeRect;
class hgeDistortionMesh;

struct Tapestry {
	hgeDistortionMesh *distortion;
	float x, y;
};

class TapestryManager {

public:
	TapestryManager();
	~TapestryManager();

	//methods
	void draw(float dt);
	void update(float dt);
	void addTapestry(int gridX, int gridY, int id);
	void reset();

	//Variables
	std::list<Tapestry> tapestryList;

};

#endif