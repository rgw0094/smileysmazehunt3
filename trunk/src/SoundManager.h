#ifndef SOUNDMANAGER_H_
#define SOUNDMANAGER_H_

#include <string>

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
	void update(float dt);
	void draw(float dt);

private:

	HCHANNEL musicChannel;
	std::string currentMusic;
	std::string previousMusic;
	int previousMusicPosition;
	int musicVolume;
	float timeChangedMusicVolume;
	bool lockMusicVolume;

};

#endif