#ifndef _TUTORIALMAN_H_
#define _TUTORIALMAN_H_

class TutorialMan {

public:

	TutorialMan(int gridX, int gridY);
	~TutorialMan();

	void draw(float dt);
	void update(float dt);
	bool isActive();

private:

	void updateState(float dt);

	float x, y;
	int triggerGridX, triggerGridY;
	int state;

};

#endif