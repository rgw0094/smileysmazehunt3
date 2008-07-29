#include "hge.h"
#include "hgeresource.h"
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
	setMusicVolume(hge->Ini_GetInt("Options", "musicVolume", 100));
	setMusicVolume(hge->Ini_GetInt("Options", "soundVolume", 100));
	timeChangedVolume = -10.0f;

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
	timeChangedVolume = hge->Timer_GetTime();
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
	timeChangedVolume = hge->Timer_GetTime();
	hge->Ini_SetInt("Options", "soundVolume", soundVolume);
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

void SoundManager::playEnvironmentEffect(char *effect, bool loop) {
	environmentChannel = hge->Effect_PlayEx(resources->GetEffect(effect),100,0,1.0f,loop);
}

void SoundManager::stopEnvironmentChannel() {
	hge->Channel_Stop(environmentChannel);
}
	

void SoundManager::playAbilityEffect(char *effect, bool loop) {
	abilityChannel = hge->Effect_PlayEx(resources->GetEffect(effect),100,0,1.0f,loop);
}

void SoundManager::stopAbilityChannel() {
	hge->Channel_Stop(abilityChannel);
}

/**
 * Listens for input to raise/lower music volume
 */
void SoundManager::update(float dt) {
	//Input to change music volume
	if (input->keyPressed(INPUT_VOLUME_UP)) {
		setMusicVolume(musicVolume + 10);
		setSoundVolume(soundVolume + 10);
	} 
	if (input->keyPressed(INPUT_VOLUME_DOWN)) {
		setMusicVolume(musicVolume - 10);
		setSoundVolume(soundVolume - 10);
	}
}

/**
 * Draws the music volume on the screen after it has been changed.
 */
void SoundManager::draw(float dt) {
	//Display music volume after it was changed
	if (timeChangedVolume > hge->Timer_GetTime() - 2.0f) {
		resources->GetFont("curlz")->SetColor(ARGB(255,0,255,0));
		resources->GetFont("curlz")->printf(512,10,HGETEXT_CENTER,"Music Volume: %d", musicVolume);
		resources->GetFont("curlz")->SetColor(ARGB(255,255,255,255));
	}
}