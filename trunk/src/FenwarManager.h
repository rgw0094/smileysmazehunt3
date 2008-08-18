#ifndef _FENWARMANAGER_H_
#define _FENWARMANAGER_H_

#include <list>

class hgeParticleManager;

struct FenwarEncounter {
	int gridX, gridY, id;
	float x, y;
	float timeEnteredState, timeTextBoxClosed;
	int state;
	bool fenwarVisible, textBoxClosed;
	bool triggered;
};

class FenwarManager {
public:

	FenwarManager();
	~FenwarManager();

	void update(float dt);
	void draw(float dt);
	void addFenwarEncounter(int gridX, int gridY, int id);
	bool isEncounterActive();
	void reset();

private:

	hgeParticleManager *particles;
	std::list<FenwarEncounter> fenwarEncounterList;


};


#endif