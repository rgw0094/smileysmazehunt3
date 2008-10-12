#include "SmileyEngine.h"
#include "hge.h"
#include "hgeresource.h"

extern SMH *smh;
extern HGE *hge;

/** 
 * Constructor
 */
SoundManager::SoundManager() {

	//Music volume starts at what it was when app was closed last
	setMusicVolume(hge->Ini_GetInt("Options", "musicVolume", 100));
	setSoundVolume(hge->Ini_GetInt("Options", "soundVolume", 100));

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

	previousMusic = currentMusic;
	previousMusicPosition = hge->Channel_GetPos(musicChannel);
	currentMusic = music;

	hge->Channel_Stop(musicChannel);
	hge->Music_SetPos(smh->resources->GetMusic(music),0,0);
	musicChannel = hge->Music_Play(smh->resources->GetMusic(music),true,musicVolume);

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
	hge->Channel_SlideTo(musicChannel,3.0f,0,-101,-1.0f);
}

/** 
 * Sets a new music volume
 */
void SoundManager::setMusicVolume(int newVolume) {
	musicVolume = newVolume;
	if (musicVolume < 0) musicVolume = 0;
	if (musicVolume > 100) musicVolume = 100;
	hge->System_SetState(HGE_MUSVOLUME, musicVolume);
	hge->Ini_SetInt("Options", "musicVolume", musicVolume);
}

/**
 * Sets a new sound volume.
 */
void SoundManager::setSoundVolume(int newVolume) {
	soundVolume = newVolume;
	if (soundVolume < 0) soundVolume = 0;
	if (soundVolume > 100) soundVolume = 100;
	hge->System_SetState(HGE_FXVOLUME, soundVolume);
	hge->Ini_SetInt("Options", "soundVolume", soundVolume);
}

/**
 * Returns to the last song played at the position where it stopped.
 */
void SoundManager::playPreviousMusic() {
	hge->Channel_Stop(musicChannel);
	musicChannel = hge->Music_Play(smh->resources->GetMusic(previousMusic.c_str()),true,musicVolume);
	hge->Channel_SetPos(musicChannel, previousMusicPosition);
	currentMusic = previousMusic;
}

void SoundManager::playEnvironmentEffect(char *effect, bool loop) {
	environmentChannel = hge->Effect_PlayEx(smh->resources->GetEffect(effect),100,0,1.0f,loop);
}

void SoundManager::stopEnvironmentChannel() {
	hge->Channel_Stop(environmentChannel);
}
	

void SoundManager::playAbilityEffect(char *effect, bool loop) {
	abilityChannel = hge->Effect_PlayEx(smh->resources->GetEffect(effect),100,0,1.0f,loop);
}

void SoundManager::stopAbilityChannel() {
	hge->Channel_Stop(abilityChannel);
}

int SoundManager::getMusicVolume() {
	return musicVolume;
}

int SoundManager::getSoundVolume() {
	return soundVolume;
}