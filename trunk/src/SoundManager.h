#ifndef SOUNDMANAGER_H_
#define SOUNDMANAGER_H_

#include <string>
#include "hge.h"

class SoundManager {

public:

	SoundManager();
	~SoundManager();

	//Methods
	void playMusic(char *musicName);
	void stopMusic();
	void fadeOutMusic();
	void playPreviousMusic();
	void setMusicVolume(int newVolume);
	void setSoundVolume(int soundVolume);
	void update(float dt);
	void draw(float dt);
	void playEnvironmentEffect(char *effect, bool loop);
	void stopEnvironmentChannel();
	void playAbilityEffect(char *effect, bool loop);
	void stopAbilityChannel();

private:

	HCHANNEL musicChannel;
	HCHANNEL abilityChannel;		//Audio channel for player ability sound effects
	HCHANNEL environmentChannel;	//Audio channel for environment sound effects
	
	std::string currentMusic;
	std::string previousMusic;
	int previousMusicPosition;
	int musicVolume;
	int soundVolume;
	float timeChangedVolume;

};

#endif