#ifndef _FOUNTAIN_H_
#define _FOUNTAIN_H_

class Fountain {

public:
	Fountain(int gridX, int gridY);
	~Fountain();

	//methods
	void update(float dt);
	void draw(float dt);
	bool isAboveSmiley();

private:

	float x, y;

};

#endif