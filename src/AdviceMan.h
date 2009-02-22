#ifndef _ADVICEMAN_H_
#define _ADVICEMAN_H_

class AdviceMan {

public:

	AdviceMan(int gridX, int gridY);
	~AdviceMan();

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