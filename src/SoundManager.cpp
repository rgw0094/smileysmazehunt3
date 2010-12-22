#include "SmileyEngine.h"
#include "Player.h"

extern SMH *smh;

//The minimum time that must pass between switch sounds so that it doesn't sound like hell
#define SWITCH_SOUND_DELAY 0.5

SoundManager::SoundManager() 
{
	//Music volume starts at what it was when app was closed last
	setMusicVolume(smh->hge->Ini_GetInt("Options", "musicVolume", 100));
	setSoundVolume(smh->hge->Ini_GetInt("Options", "soundVolume", 100));

	abilityChannelActive = false;
	environmentChannelActive = false;
	iceChannelActive = false;
}


SoundManager::~SoundManager() 
{
	//Should never be deleted
}

void SoundManager::ResetLoopingChannels() {
	stopAbilityChannel();
	stopIceChannel();
	stopEnvironmentChannel();
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

std::string SoundManager::getCurrentSongName()
{
	return currentMusic;
}

/**
 * Plays the music for the specified area.
 */
void SoundManager::playAreaMusic(int id) {
	if (id == FOUNTAIN_AREA) {
		playMusic("townMusic");
	} else if (id == OLDE_TOWNE) {
		playMusic("oldeTowneMusic");
	} else if (id == SMOLDER_HOLLOW) {
		playMusic("smolderHollowMusic");
	} else if (id == FOREST_OF_FUNGORIA) {
		playMusic("forestMusic");
	} else if (id == SESSARIA_SNOWPLAINS) {
		playMusic("iceMusic");
	} else if (id == TUTS_TOMB) {
		playMusic("kingTutMusic");
	} else if (id == WORLD_OF_DESPAIR) {
		playMusic("realmOfDespairMusic");
	} else if (id == SERPENTINE_PATH) {
		playMusic("serpentinePathMusic");
	} else if (id == CASTLE_OF_EVIL) {
		playMusic("castleOfEvilMusic");
	} else if (id == CONSERVATORY) {
		playMusic("conservatoryMusic");
	} else if (id == DEBUG_AREA) {
		smh->soundManager->stopMusic();
	}
}

/**
 * Stops the music immediately.
 */
void SoundManager::stopMusic() 
{
	smh->hge->Channel_Stop(musicChannel);
}

/**
 * Stops the music by fading out.
 */
void SoundManager::fadeOutMusic() 
{
	smh->hge->Channel_SlideTo(musicChannel,3.0f,0,-101,-1.0f);
}

/** 
 * Sets a new music volume
 */
void SoundManager::setMusicVolume(int newVolume) 
{
	musicVolume = newVolume;
	if (musicVolume < 0) musicVolume = 0;
	if (musicVolume > 100) musicVolume = 100;
	smh->hge->Channel_SetVolume(musicChannel, musicVolume);
	smh->hge->Ini_SetInt("Options", "musicVolume", musicVolume);
}

/**
 * Sets a new sound volume.
 */
void SoundManager::setSoundVolume(int newVolume) 
{
	soundVolume = newVolume;
	if (soundVolume < 0) soundVolume = 0;
	if (soundVolume > 100) soundVolume = 100;
	smh->hge->System_SetState(HGE_FXVOLUME, soundVolume);
	smh->hge->Ini_SetInt("Options", "soundVolume", soundVolume);
}

/**
 * Returns to the last song played at the position where it stopped.
 */
void SoundManager::playPreviousMusic() 
{
	smh->hge->Channel_Stop(musicChannel);
	musicChannel = smh->hge->Music_Play(smh->resources->GetMusic(previousMusic.c_str()),true,musicVolume);
	smh->hge->Channel_SetPos(musicChannel, previousMusicPosition);
	currentMusic = previousMusic;
}

/**
 * Plays the switch sound effect originating from point (gridX, gridY). The sound will only be played if
 * alwaysPlaySound is true or the sound is originating from a point close to Smiley. 
 * 
 * This method can be called as much as you want but it will only play a sound at maximum every SWITCH_SOUND_DELAY
 * seconds to avoid it sounding like the dickens if a ton of switches are toggled at once.
 */
void SoundManager::playSwitchSound(int gridX, int gridY, bool alwaysPlaySound) 
{
	if (smh->timePassedSince(lastSwitchSoundTime) > SWITCH_SOUND_DELAY) {
		bool inRange = abs(gridX - smh->player->gridX) <= 8 && abs(gridY - smh->player->gridY) <= 6;
		if (alwaysPlaySound || inRange) {
			lastSwitchSoundTime = smh->getGameTime();
			playSound("snd_switch");
		}
	}
}

void SoundManager::playEnvironmentEffect(char *effect, bool loop) 
{
	if (environmentChannelActive) return;
	if (smh->player->getHealth() <= 0.0) return;

	environmentChannel = smh->hge->Effect_PlayEx(smh->resources->GetEffect(effect),100,0,1.0f,loop);
	environmentChannelActive = true;
}

void SoundManager::stopEnvironmentChannel() 
{
	smh->hge->Channel_Stop(environmentChannel);
	environmentChannelActive = false;
}

void SoundManager::playAbilityEffect(char *effect, bool loop) 
{
	if (abilityChannelActive) return;

	abilityChannel = smh->hge->Effect_PlayEx(smh->resources->GetEffect(effect),100,0,1.0f,loop);
	abilityChannelActive = true;
}

void SoundManager::stopAbilityChannel() 
{
	smh->hge->Channel_Stop(abilityChannel);
	abilityChannelActive = false;
}

void SoundManager::playIceEffect(char *effect, bool loop) 
{
	if (iceChannelActive) return;
	
	iceChannel = smh->hge->Effect_PlayEx(smh->resources->GetEffect(effect),100,0,1.0f,loop);
	iceChannelActive = true;
}

void SoundManager::stopIceChannel() 
{
	smh->hge->Channel_Stop(iceChannel);
	iceChannelActive = false;
}

void SoundManager::playSound(const char* sound) 
{
	playSound(sound, 0.0);
}

void SoundManager::playSound(const char* sound, float delay) {

	bool play = false;

	if (delay == 0.0) {
		play = true;
	} else {
		bool soundFound = false;
		//Search the last played list and find the last time that the sound was played to check if the delay has passed yet
		for (std::list<Sound>::iterator i = lastPlayTimes.begin(); i != lastPlayTimes.end(); i++) {
			if (strcmp(i->name.c_str(), sound) == 0) {
				soundFound = true;
				if (smh->timePassedSince(i->lastTimePlayed) >= delay) {
					play = true;
					i->lastTimePlayed = smh->getGameTime();
				}
			}
		}
		//Handle the case where the sound hasn't been played yet
		if (!soundFound) {
			play = true;
			Sound newSound;
			newSound.name = sound;
			newSound.lastTimePlayed = smh->getGameTime();
			lastPlayTimes.push_back(newSound);
		}
	}

	if (play) {
		smh->hge->Effect_Play(smh->resources->GetEffect(sound));
	}
}

int SoundManager::getMusicVolume() {
	return musicVolume;
}

int SoundManager::getSoundVolume() {
	return soundVolume;
}