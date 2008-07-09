#include "hge include/hge.h"
#include "hge include/hgeresource.h"
#include "SoundManager.h"
#include "Input.h"

extern HGE *hge;
extern hgeResourceManager *resources;
extern Input *input;

/** 
 * Constructor
 */
SoundManager::SoundManager() {

	//Music volume starts at what it was when app was closed last
	setMusicVolume(hge->Ini_GetInt("Options", "volume", 100));
	timeChangedMusicVolume = -10.0f;
	lockMusicVolume = false;

}

/**
 * Destructor
 */
SoundManager::~SoundManager() {

}


/**
 * Changes the music channel to play the specified song.
 */
void SoundManager::playMusic(char *music) {

	lockMusicVolume = false;

	previousMusic = currentMusic;
	previousMusicPosition = hge->Channel_GetPos(musicChannel);
	currentMusic = music;

	hge->Channel_Stop(musicChannel);
	hge->Music_SetPos(resources->GetMusic(music),0,0);
	musicChannel = hge->Music_Play(resources->GetMusic(music),true,musicVolume);

}

/**
 * Stops the music immediately.
 */
void SoundManager::stopMusic() {
	hge->Channel_Stop(musicChannel);
}

/**
 * Stops the music by fading out.
 */
void SoundManager::fadeOutMusic() {
	lockMusicVolume = true;
	hge->Channel_SlideTo(musicChannel,3.0f,0,-101,-1.0f);
}

/** 
 * Sets a new music volume
 */
void SoundManager::setMusicVolume(int newVolume) {
	musicVolume = newVolume;
	if (musicVolume < 0) musicVolume = 0;
	if (musicVolume > 100) musicVolume = 100;
	hge->Channel_SetVolume(musicChannel, musicVolume);
	timeChangedMusicVolume = hge->Timer_GetTime();
	hge->Ini_SetInt("Options", "volume", musicVolume);
}

/**
 * Returns to the last song played at the position where it stopped.
 */
void SoundManager::playPreviousMusic() {
	hge->Channel_Stop(musicChannel);
	musicChannel = hge->Music_Play(resources->GetMusic(previousMusic.c_str()),true,musicVolume);
	hge->Channel_SetPos(musicChannel, previousMusicPosition);
	currentMusic = previousMusic;
}

/**
 * Listens for input to raise/lower music volume
 */
void SoundManager::update(float dt) {
	//Input to change music volume
	if (!lockMusicVolume) {
		if (input->keyPressed(INPUT_VOLUME_UP)) {
			setMusicVolume(musicVolume + 10);
		} 
		if (input->keyPressed(INPUT_VOLUME_DOWN)) {
			setMusicVolume(musicVolume - 10);
		}
	}
}

/**
 * Draws the music volume on the screen after it has been changed.
 */
void SoundManager::draw(float dt) {
	//Display music volume after it was changed
	if (timeChangedMusicVolume > hge->Timer_GetTime() - 2.0f) {
		resources->GetFont("curlz")->SetColor(ARGB(255,0,255,0));
		resources->GetFont("curlz")->printf(512,10,HGETEXT_CENTER,"Music Volume: %d", musicVolume);
		resources->GetFont("curlz")->SetColor(ARGB(255,255,255,255));
	}
}