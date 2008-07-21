#ifndef LOADEFFECTMANAGER_H
#define LOADEFFECTMANAGER_H

class LoadEffectManager {

public:

	LoadEffectManager();
	~LoadEffectManager();

	//methods
	void draw(float dt);
	void update(float dt);
	void startEffect(int destinationX, int destinationY, int destinationArea);
	bool isEffectActive();
	void displayNewAreaName();

private:

	int state;
	int destinationX;
	int destinationY;
	int destinationArea;

	float timeLevelLoaded;
	float zoneTextAlpha;
	float loadingEffectScale;

};

#endif