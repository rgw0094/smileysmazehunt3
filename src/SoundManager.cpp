#include "SmileyEngine.h"
#include "Player.h"

extern SMH *smh;

//The minimum time that must pass between switch sounds so that it doesn't sound like hell
#define SWITCH_SOUND_DELAY 0.5

/** 
 * Constructor
 */
SoundManager::SoundManager() {

	//Music volume starts at what it was when app was closed last
	setMusicVolume(smh->hge->Ini_GetInt("Options", "musicVolume", 100));
	setSoundVolume(smh->hge->Ini_GetInt("Options", "soundVolume", 100));

	lastSwitchSoundTime = 0.0;
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
	previousMusicPosition = smh->hge->Channel_GetPos(musicChannel);
	currentMusic = music;

	smh->hge->Channel_Stop(musicChannel);
	smh->hge->Music_SetPos(smh->resources->GetMusic(music),0,0);
	musicChannel = smh->hge->Music_Play(smh->resources->GetMusic(music),true,musicVolume);

}

/**
 * Stops the music immediately.
 */
void SoundManager::stopMusic() {
	smh->hge->Channel_Stop(musicChannel);
}

/**
 * Stops the music by fading out.
 */
void SoundManager::fadeOutMusic() {
	smh->hge->Channel_SlideTo(musicChannel,3.0f,0,-101,-1.0f);
}

/** 
 * Sets a new music volume
 */
void SoundManager::setMusicVolume(int newVolume) {
	musicVolume = newVolume;
	if (musicVolume < 0) musicVolume = 0;
	if (musicVolume > 100) musicVolume = 100;
	smh->hge->Channel_SetVolume(musicChannel, musicVolume);
	smh->hge->Ini_SetInt("Options", "musicVolume", musicVolume);
}

/**
 * Sets a new sound volume.
 */
void SoundManager::setSoundVolume(int newVolume) {
	soundVolume = newVolume;
	if (soundVolume < 0) soundVolume = 0;
	if (soundVolume > 100) soundVolume = 100;
	smh->hge->System_SetState(HGE_FXVOLUME, soundVolume);
	smh->hge->Ini_SetInt("Options", "soundVolume", soundVolume);
}

/**
 * Returns to the last song played at the position where it stopped.
 */
void SoundManager::playPreviousMusic() {
	smh->hge->Channel_Stop(musicChannel);
	musicChannel = smh->hge->Music_Play(smh->resources->GetMusic(previousMusic.c_str()),true,musicVolume);
	smh->hge->Channel_SetPos(musicChannel, previousMusicPosition);
	currentMusic = previousMusic;
}

void SoundManager::playEnvironmentEffect(char *effect, bool loop) {
	environmentChannel = smh->hge->Effect_PlayEx(smh->resources->GetEffect(effect),100,0,1.0f,loop);
}

void SoundManager::stopEnvironmentChannel() {
	smh->hge->Channel_Stop(environmentChannel);
}

/**
 * Plays the switch sound effect originating from point (gridX, gridY). The sound will only be played if
 * alwaysPlaySound is true or the sound is originating from a point close to Smiley. 
 * 
 * This method can be called as much as you want but it will only play a sound at maximum every SWITCH_SOUND_DELAY
 * seconds to avoid it sounding like the dickens if a ton of switches are toggled at once.
 */
void SoundManager::playSwitchSound(int gridX, int gridY, bool alwaysPlaySound) {
	if (smh->timePassedSince(lastSwitchSoundTime) > SWITCH_SOUND_DELAY) {
		bool inRange = abs(gridX - smh->player->gridX) <= 8 && abs(gridY - smh->player->gridY) <= 6;
		if (alwaysPlaySound || inRange) {
			lastSwitchSoundTime = smh->getGameTime();
			playSound("snd_switch");
		}
	}
}

void SoundManager::playAbilityEffect(char *effect, bool loop) {
	abilityChannel = smh->hge->Effect_PlayEx(smh->resources->GetEffect(effect),100,0,1.0f,loop);
}

void SoundManager::playSound(const char* sound) {
	smh->hge->Effect_Play(smh->resources->GetEffect(sound));
}

void SoundManager::stopAbilityChannel() {
	smh->hge->Channel_Stop(abilityChannel);
}

int SoundManager::getMusicVolume() {
	return musicVolume;
}

int SoundManager::getSoundVolume() {
	return soundVolume;
}