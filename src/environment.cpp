#include "smiley.h"
#include "environment.h"
#include "EnemyManager.h"
#include "lootmanager.h"
#include "ProjectileManager.h"
#include "SaveManager.h"
#include "player.h"
#include "collisioncircle.h"
#include "npcmanager.h"
#include "minimenu.h"
#include "boss.h"
#include "EnemyGroupManager.h"
#include "WindowManager.h"
#include "Input.h"
#include "SoundManager.h"
#include "GameData.h"
#include "hgeparticle.h"
#include "Tongue.h"
#include "SpecialTileManager.h"
#include "EvilWallManager.h"
#include "LoadEffectManager.h"
#include "WeaponParticle.h"
#include "TapestryManager.h"
#include "ChangeManager.h"
#include "enemy.h"
#include "Smilelet.h"

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

//Variables
extern bool debugMode;
extern int fountainX, fountainY;
extern int musicVolume;
extern float gameTime;

//Objects
extern HGE *hge;
extern Player *thePlayer;
extern SaveManager *saveManager;
extern EnemyManager *enemyManager;
extern LootManager *lootManager;
extern ProjectileManager *projectileManager;
extern TextBox *theTextBox;
extern hgeStringTable *stringTable;
extern NPCManager *npcManager;
extern WindowManager *windowManager;
extern BossManager *bossManager;
extern hgeResourceManager *resources;
extern EnemyGroupManager *enemyGroupManager;
extern Input *input;
extern SoundManager *soundManager;
extern GameData *gameData;
extern LoadEffectManager *loadEffectManager;

//Sprites
extern hgeSprite *itemLayer[512];

/**
 * Constructor
 */
Environment::Environment() {
	
	//Load particles
	environmentParticles = new hgeParticleManager();
	resources->GetParticleSystem("fountain")->Fire();

	//Load animations
	silverCylinder = new hgeAnimation(resources->GetTexture("animations"),5,20,0,3*64,64,64);
	silverCylinder->SetMode(HGEANIM_REV);
	silverCylinder->Play();
	brownCylinder = new hgeAnimation(resources->GetTexture("animations"),5,20,0,4*64,64,64);
	brownCylinder->SetMode(HGEANIM_REV);
	brownCylinder->Play();
	blueCylinder = new hgeAnimation(resources->GetTexture("animations"),5,20,0,5*64,64,64);
	blueCylinder->SetMode(HGEANIM_REV);
	blueCylinder->Play();
	greenCylinder = new hgeAnimation(resources->GetTexture("animations"),5,20,0,6*64,64,64);
	greenCylinder->SetMode(HGEANIM_REV);
	greenCylinder->Play();
	yellowCylinder = new hgeAnimation(resources->GetTexture("animations"),5,20,0,7*64,64,64);
	yellowCylinder->SetMode(HGEANIM_REV);
	yellowCylinder->Play();
	whiteCylinder = new hgeAnimation(resources->GetTexture("animations"),5,20,0,8*64,64,64);
	whiteCylinder->SetMode(HGEANIM_REV);
	whiteCylinder->Play();
	silverCylinderRev = new hgeAnimation(resources->GetTexture("animations"),5,20,0,3*64,64,64);
	silverCylinderRev->SetMode(HGEANIM_FWD);
	brownCylinderRev = new hgeAnimation(resources->GetTexture("animations"),5,20,0,4*64,64,64);
	brownCylinderRev->SetMode(HGEANIM_FWD);
	blueCylinderRev = new hgeAnimation(resources->GetTexture("animations"),5,20,0,5*64,64,64);
	blueCylinderRev->SetMode(HGEANIM_FWD);
	greenCylinderRev = new hgeAnimation(resources->GetTexture("animations"),5,20,0,6*64,64,64);
	greenCylinderRev->SetMode(HGEANIM_FWD);
	yellowCylinderRev = new hgeAnimation(resources->GetTexture("animations"),5,20,0,7*64,64,64);
	yellowCylinderRev->SetMode(HGEANIM_FWD);
	whiteCylinderRev = new hgeAnimation(resources->GetTexture("animations"),5,20,0,8*64,64,64);
	whiteCylinderRev->SetMode(HGEANIM_FWD);

	//Manager for special tiles
	hge->System_Log("Creating Environment.SpecialTileManager");
	specialTileManager = new SpecialTileManager();

	//Evil walls
	hge->System_Log("Creating Environment.EvilWallManager");
	evilWallManager = new EvilWallManager();

	//Tapestries
	hge->System_Log("Creating Environment.TapestryManager");
	tapestryManager = new TapestryManager();

	//Smilelets (guys that follow Smiley)
	smilelet = new Smilelet();

	resources->GetAnimation("savePoint")->Play();
	collisionBox = new hgeRect();

}


/**
 * Destructor
 */
Environment::~Environment() {

	delete environmentParticles;
	delete evilWallManager;
	delete specialTileManager;
	delete tapestryManager;
	delete smilelet;

	delete collisionBox;
	delete silverCylinder;
	delete brownCylinder;
	delete blueCylinder;
	delete greenCylinder;
	delete yellowCylinder;
	delete whiteCylinder;
	delete silverCylinderRev;
	delete brownCylinderRev;
	delete blueCylinderRev;
	delete greenCylinderRev;
	delete yellowCylinderRev;
	delete whiteCylinderRev;

}


/**
 * Loads an area.
 *
 *	id			id of the zone to load
 *	from		id of the zone Smiley is coming from
 *	playerX		x location to put Smiley, or 0 for default
 *  playerY		y location to put Smiley, or 0 for default
 */
void Environment::loadArea(int id, int from, int playerX, int playerY) {

	hge->System_Log("Loading area: %d, placing Player at (%d,%d)", id, playerX, playerY);

	std::ifstream areaFile;
	char threeBuffer[3];

	saveManager->currentArea = id;
	hasFountain = false;
	fountainOnScreen = false;

	//Delete all objects from the previous area.
	bossManager->reset();
	enemyManager->reset();
	projectileManager->reset();
	lootManager->reset();
	npcManager->reset();
	enemyGroupManager->resetGroups();
	tapestryManager->reset();
	specialTileManager->reset();
	evilWallManager->reset();
	smilelet->reset();

	//Clear old level data
	for (int i = 0; i < 256; i++) {
		for (int j = 0; j < 256; j++) {
			terrain[i][j] = 0;
			collision[i][j] = 0;
			ids[i][j] = -1;
			item[i][j] = 0;
			activated[i][j] = -100.0f;
			variable[i][j] = 0;
			enemyLayer[i][j] = -1;
		}
	}

	//Set zone specific info
	if (areaFile.is_open()) areaFile.close();
	if (id == FOUNTAIN_AREA) {
		soundManager->playMusic("townMusic");
		areaFile.open("Data/Maps/fountain.smh");
	} else if (id == OLDE_TOWNE) {
		soundManager->playMusic("oldeTowneMusic");
		areaFile.open("Data/Maps/oldetowne.smh");
	} else if (id == SMOLDER_HOLLOW) {
		soundManager->playMusic("smolderHollowMusic");
		areaFile.open("Data/Maps/smhollow.smh");
	} else if (id == FOREST_OF_FUNGORIA) {
		soundManager->playMusic("forestMusic");
		areaFile.open("Data/Maps/forest.smh");
	} else if (id == SESSARIA_SNOWPLAINS) {
		soundManager->playMusic("iceMusic");
		areaFile.open("Data/Maps/snow.smh");
	} else if (id == TUTS_TOMB) {
		soundManager->playMusic("kingTutMusic");
		areaFile.open("Data/Maps/tutstomb.smh");
	} else if (id == WORLD_OF_DESPAIR) {
		soundManager->playMusic("realmOfDespairMusic");
		areaFile.open("Data/Maps/despair.smh");
	} else if (id == SERPENTINE_PATH) {
		soundManager->playMusic("serpentinePathMusic");
		areaFile.open("Data/Maps/path.smh");
	} else if (id == CASTLE_OF_EVIL) {
		soundManager->playMusic("castleOfEvilMusic");
		areaFile.open("Data/Maps/castle.smh");
	}

	//id range - ignore it
	areaFile.read(threeBuffer,2);

	//Read area width
	areaFile.read(threeBuffer,3);
	areaWidth = atoi(threeBuffer);
	areaFile.read(threeBuffer,1);	//read space
	//Read area height
	areaFile.read(threeBuffer,3);
	areaHeight = atoi(threeBuffer);
	areaFile.read(threeBuffer,1);	//read newline

	//Set up screen size (64 is normal size)
	screenWidth = SCREEN_WIDTH / 64.0;
	screenHeight = SCREEN_HEIGHT / 64.0;


	//Load ID Layer
	for (int row = 0; row < areaHeight; row++) {
		for (int col = 0; col < areaWidth; col++) {
			areaFile.read(threeBuffer,3);
			ids[col][row] = atoi(threeBuffer);
		}
		//Read the newline
		areaFile.read(threeBuffer,1);
	}
	areaFile.read(threeBuffer,3);	//width
	areaFile.read(threeBuffer,1);	//space
	areaFile.read(threeBuffer,3);	//height
	areaFile.read(threeBuffer,1);	//newline


	//Load Variable Layer
	for (int row = 0; row < areaHeight; row++) {
		for (int col = 0; col < areaWidth; col++) {
			areaFile.read(threeBuffer,3);
			variable[col][row] = atoi(threeBuffer);
		}
		//Read the newline
		areaFile.read(threeBuffer,1);
	}
	areaFile.read(threeBuffer,3);	//width
	areaFile.read(threeBuffer,1);	//space
	areaFile.read(threeBuffer,3);	//height
	areaFile.read(threeBuffer,1);	//newline


	//Load terrain data for the area
	for (int row = 0; row < areaHeight; row++) {
		for (int col = 0; col < areaWidth; col++) {
			areaFile.read(threeBuffer,3);
			terrain[col][row] = atoi(threeBuffer);
		}
		//Read the newline
		areaFile.read(threeBuffer,1);
	}
	areaFile.read(threeBuffer,3);	//width
	areaFile.read(threeBuffer,1);	//space
	areaFile.read(threeBuffer,3);	//height
	areaFile.read(threeBuffer,1);	//newline


	//Load collision detection data
	for (int row = 0; row < areaHeight; row++) {
		for (int col = 0; col < areaWidth; col++) {
			areaFile.read(threeBuffer,3);
			collision[col][row] = atoi(threeBuffer);

			//Big ass fountain location
			if (collision[col][row] == FOUNTAIN) {
				hasFountain = true;
				fountainX = -7;
				fountainY = -7;
			}
			
			//Flames
			if (collision[col][row] == FLAME) {
				specialTileManager->addFlame(col, row);
			}
			
			//Ice blocks
			if (collision[col][row] == FIRE_DESTROY) {
				specialTileManager->addIceBlock(col, row);
			}

			//Mushrooms
			if (collision[col][row] == DIZZY_MUSHROOM_1 || collision[col][row] == DIZZY_MUSHROOM_2) {
				specialTileManager->addMushroom(col,row,collision[col][row]);
			}

			//Smilelet
			if (collision[col][row] == SMILELET) {
				smilelet->addSmilelet(col,row,ids[col][row]);
				collision[col][row] = WALKABLE;
			}
			
			//Evil wall stuff
			if (collision[col][row] >= EVIL_WALL_POSITION && collision[col][row] <= EVIL_WALL_RESTART) {
				evilWallManager->addEvilWall(ids[col][row]);
				evilWallManager->setState(ids[col][row],0);
			}

            //Rob DO NOT put an else here
			if (collision[col][row] == EVIL_WALL_POSITION) {					
				evilWallManager->setBeginWallPosition(ids[col][row],col,row);
				evilWallManager->setSpeed(ids[col][row],variable[col][row]);
			} else if (collision[col][row] == EVIL_WALL_TRIGGER) {
				evilWallManager->setDir(ids[col][row],variable[col][row]);
			} else if (collision[col][row] == EVIL_WALL_RESTART) {
				evilWallManager->setSmileyRestartPosition(ids[col][row],col,row);
			}
		}
		//Read the newline
		areaFile.read(threeBuffer,1);
	}
	areaFile.read(threeBuffer,3);	//width
	areaFile.read(threeBuffer,1);	//space
	areaFile.read(threeBuffer,3);	//height
	areaFile.read(threeBuffer,1);	//newline

	//Load item data
	for (int row = 0; row < areaHeight; row++) {
		for (int col = 0; col < areaWidth; col++) {
			areaFile.read(threeBuffer,3);
			int newItem = atoi(threeBuffer);

			//Tapestries - second row of the item layer
			if (newItem >= 16 && newItem < 32) {
				tapestryManager->addTapestry(col, row, newItem);
			//Other item
			} else {
				item[col][row] = newItem;
			}

		}
		//Read the newline
		areaFile.read(threeBuffer,1);
	}
	areaFile.read(threeBuffer,3);	//width
	areaFile.read(threeBuffer,1);	//space
	areaFile.read(threeBuffer,3);	//height
	areaFile.read(threeBuffer,1);	//newline


	//Load enemy/NPC/Boss data
	int enemy;
	for (int row = 0; row < areaHeight; row++) {
		for (int col = 0; col < areaWidth; col++) {
			areaFile.read(threeBuffer,3);
			enemy = atoi(threeBuffer);
			enemyLayer[col][row] = enemy-1;

			//IDs 240-256 are bosses
			if (enemy >= 240) {

				//Spawn the boss if it has never been killed
				if (!saveManager->isBossKilled(enemy)) {
					bossManager->spawnBoss(enemy, variable[col][row], col, row);
				}

			//IDs 1-127 are enemies
			} else if (enemy > 0 && enemy < 128) {

				if (ids[col][row] == ENEMYGROUP_ENEMY) {
					//If this enemy is part of a group, notify the manager
					enemyGroupManager->addEnemy(variable[col][row]);
				}
				if (ids[col][row] != ENEMYGROUP_ENEMY_POPUP) {
					//Don't spawn popup enemies yet
					enemyManager->addEnemy(enemy-1,col,row, .2, .2, variable[col][row]);
				}

			//IDs 128 - 239 are NPCs
			} else if (enemy >= 128 && enemy < 240) {
				npcManager->addNPC(enemy-128,ids[col][row],col,row);
			} 
		}
		//Read the newline
		areaFile.read(threeBuffer,1);
	}
	areaFile.read(threeBuffer,3);	//width
	areaFile.read(threeBuffer,1);	//space
	areaFile.read(threeBuffer,3);	//height
	areaFile.read(threeBuffer,1);	//newline


	//Set up stuff
	bool playerPlaced = false;
	for (int i = 0; i < areaWidth; i++) {
		for (int j = 0; j < areaHeight; j++) {

			//Put the player in the correct starting location
			if (playerX > 0 && playerY > 0) {
				thePlayer->moveTo(playerX,playerY);
				startX = playerX;
				startY = playerY;
				playerPlaced = true;
			} else if (!playerPlaced && collision[i][j] == PLAYER_START && ids[i][j] == from) {
				thePlayer->moveTo(i,j);
				startX = i;
				startY = j;
				playerPlaced = true;
			}

			//If there is an item at this tile check to see if it has already
			//been collected
			if (item[i][j] > 0 && item[i][j] < 16) {
				if (saveManager->isTileChanged(i,j)) {
					item[i][j] = NONE;
				}
			}

			//If there is a door on this tile check to see if it has already
			//been opened
			if (collision[i][j] >= RED_KEYHOLE && collision[i][j] <= BLUE_KEYHOLE) {
				if (saveManager->isTileChanged(i,j)) {
					collision[i][j] = WALKABLE;
				}
			}

			//If there is a bombable wall at this square check to see if it has
			//already been bombed
			if (collision[i][j] == BOMBABLE_WALL) {
				if (saveManager->isTileChanged(i,j)) {
					collision[i][j] = WALKABLE;
				}
			}

			//Flip switches that have been marked as changed
			if (isCylinderSwitchLeft(collision[i][j]) || isCylinderSwitchRight(collision[i][j])) {
				if (saveManager->isTileChanged( i, j)) {
					int id = ids[i][j];
					//Scan the area for cylinders linked to this switch
					for (int k = 0; k < areaWidth; k++) {
						for (int l = 0; l < areaHeight; l++) {
							//If this id matches the switch we flipped
							if (ids[k][l] == id) {
								//Switch up cylinders down
								if (isCylinderUp(collision[k][l])) {
									collision[k][l] -= 16;
								//Switch down cylinders up
								} else if (isCylinderDown(collision[k][l])) {
									collision[k][l] += 16;
								}
							}
						}
					}
				}
			}
		}
	}
	
	//Update this and the enemies once to get shit set up
	update(0.0);
	enemyManager->update(0.0);
	thePlayer->update(0.0);

	//Tell the LoadEffectManager to display the new area name
	loadEffectManager->displayNewAreaName();

}


/**
 * Draws the environment
 */
void Environment::draw(float dt) {

	fountainOnScreen = false;
	int offscreenRange = (hasFountain ? 6 : 1);

	//Loop through each tile to draw shit
	for (int j = -offscreenRange; j <= screenHeight + offscreenRange; j++) {
		for (int i = -offscreenRange; i <= screenWidth + offscreenRange; i++) {
			
			drawX = int(float(i)*64.0f - xOffset);
			drawY = int(float(j)*64.0f - yOffset);

			if (inBounds(i+xGridOffset, j+yGridOffset)) {	

				//Terrain
				int theTerrain = terrain[i+xGridOffset][j+yGridOffset];
				if (theTerrain > 0 && theTerrain < 256) {
					resources->GetAnimation("mainLayer")->SetFrame(theTerrain);
					resources->GetAnimation("mainLayer")->Render(drawX,drawY);
				} else {
					resources->GetSprite("blackScreen")->RenderStretch(drawX, drawY, drawX+64, drawY+64);
				}

				//Collision
				int theCollision = collision[i+xGridOffset][j+yGridOffset];
				if (theCollision != WALKABLE && theCollision != UNWALKABLE && 
					theCollision != ENEMY_NO_WALK && theCollision != PLAYER_START && 
					theCollision != DIZZY_MUSHROOM_1 && theCollision != DIZZY_MUSHROOM_2 &&
					theCollision != PLAYER_END && theCollision != PIT && 
					theCollision != UNWALKABLE_PROJECTILE && 		
					theCollision != FLAME &&
					theCollision != FIRE_DESTROY &&
					!(isWarp(theCollision) && 
					variable[i + xGridOffset][j + yGridOffset] == 990)) {
					
					//Water animation
					if (theCollision == SHALLOW_WATER) {
						resources->GetAnimation("water")->SetColor(ARGB(125,255,255,255));
						resources->GetAnimation("water")->Render(drawX,drawY);
					} else if (theCollision == DEEP_WATER) {
						resources->GetAnimation("water")->SetColor(ARGB(255,255,255,255));
						resources->GetAnimation("water")->Render(drawX,drawY);
					} else if (theCollision == WALK_LAVA || theCollision == NO_WALK_LAVA) {
						resources->GetAnimation("lava")->Render(drawX,drawY);
					} else if (theCollision == GREEN_WATER) {
						resources->GetAnimation("greenWater")->SetColor(ARGB(255,255,255,255));
						resources->GetAnimation("greenWater")->Render(drawX,drawY);
					} else if (theCollision == SHALLOW_GREEN_WATER) {
						resources->GetAnimation("greenWater")->SetColor(ARGB(125,255,255,255));
						resources->GetAnimation("greenWater")->Render(drawX,drawY);
					} else if (theCollision == SPRING_PAD && gameTime - 0.5f < activated[i+xGridOffset][j+yGridOffset]) {
						resources->GetAnimation("spring")->Render(drawX,drawY);
					} else if (theCollision == SUPER_SPRING && gameTime - 0.5f < activated[i+xGridOffset][j+yGridOffset]) {
						resources->GetAnimation("superSpring")->Render(drawX, drawY);
					
					//Switch animations
					} else if ((theCollision == SILVER_SWITCH_LEFT || theCollision == SILVER_SWITCH_RIGHT) && gameTime - .25f < activated[i+xGridOffset][j+yGridOffset]) {
						resources->GetAnimation("silverSwitch")->Render(drawX,drawY);
					} else if ((theCollision == BROWN_SWITCH_LEFT || theCollision == BROWN_SWITCH_RIGHT) && gameTime - .25f < activated[i+xGridOffset][j+yGridOffset]) {
						resources->GetAnimation("brownSwitch")->Render(drawX,drawY);
					} else if ((theCollision == BLUE_SWITCH_LEFT || theCollision == BLUE_SWITCH_RIGHT) && gameTime - .25f < activated[i+xGridOffset][j+yGridOffset]) {
						resources->GetAnimation("blueSwitch")->Render(drawX,drawY);
					} else if ((theCollision == GREEN_SWITCH_LEFT || theCollision == GREEN_SWITCH_RIGHT) && gameTime - .25f < activated[i+xGridOffset][j+yGridOffset]) {
						resources->GetAnimation("greenSwitch")->Render(drawX,drawY);
					} else if ((theCollision == YELLOW_SWITCH_LEFT || theCollision == YELLOW_SWITCH_RIGHT) && gameTime - .25f < activated[i+xGridOffset][j+yGridOffset]) {
						resources->GetAnimation("yellowSwitch")->Render(drawX,drawY);
					} else if ((theCollision == WHITE_SWITCH_LEFT || theCollision == WHITE_SWITCH_RIGHT) && gameTime - .25f < activated[i+xGridOffset][j+yGridOffset]) {
						resources->GetAnimation("whiteSwitch")->Render(drawX,drawY);
					
					//Cylinder animations
					} else if ((theCollision == SILVER_CYLINDER_DOWN) && gameTime - .25f < activated[i+xGridOffset][j+yGridOffset]) {
						silverCylinder->Render(drawX,drawY);
					} else if ((theCollision == SILVER_CYLINDER_UP) && gameTime - .25f < activated[i+xGridOffset][j+yGridOffset]) {
						silverCylinderRev->Render(drawX,drawY);
					} else if ((theCollision == BROWN_CYLINDER_DOWN) && gameTime - .25f < activated[i+xGridOffset][j+yGridOffset]) {
						brownCylinder->Render(drawX,drawY);
					} else if ((theCollision == BROWN_CYLINDER_UP) && gameTime - .25f < activated[i+xGridOffset][j+yGridOffset]) {
						brownCylinderRev->Render(drawX,drawY);
					} else if ((theCollision == BLUE_CYLINDER_DOWN) && gameTime - .25f < activated[i+xGridOffset][j+yGridOffset]) {
						blueCylinder->Render(drawX,drawY);
					} else if ((theCollision == BLUE_CYLINDER_UP) && gameTime - .25f < activated[i+xGridOffset][j+yGridOffset]) {
						blueCylinderRev->Render(drawX,drawY);
					} else if ((theCollision == GREEN_CYLINDER_DOWN) && gameTime - .25f < activated[i+xGridOffset][j+yGridOffset]) {
						greenCylinder->Render(drawX,drawY);
					} else if ((theCollision == GREEN_CYLINDER_UP) && gameTime - .25f < activated[i+xGridOffset][j+yGridOffset]) {
						greenCylinderRev->Render(drawX,drawY);
					} else if ((theCollision == YELLOW_CYLINDER_DOWN) && gameTime - .25f < activated[i+xGridOffset][j+yGridOffset]) {
						yellowCylinder->Render(drawX,drawY);
					} else if ((theCollision == YELLOW_CYLINDER_UP) && gameTime - .25f < activated[i+xGridOffset][j+yGridOffset]) {
						yellowCylinderRev->Render(drawX,drawY);
					} else if ((theCollision == WHITE_CYLINDER_DOWN) && gameTime - .25f < activated[i+xGridOffset][j+yGridOffset]) {
						whiteCylinder->Render(drawX,drawY);
					} else if ((theCollision == WHITE_CYLINDER_UP) && gameTime - .25f < activated[i+xGridOffset][j+yGridOffset]) {
						whiteCylinderRev->Render(drawX,drawY);

					//Save thing
					} else if (theCollision == SAVE_SHRINE) {
						resources->GetAnimation("savePoint")->Update(dt);
						resources->GetAnimation("savePoint")->Render(drawX, drawY);

					//Remember where the fountain is, it is drawn later
					} else if (theCollision == FOUNTAIN) {
						fountainOnScreen = true;
						fountainX = i;
						fountainY = j;

					//Non-animated collision tiles
					} else {

						//Set to current tile
						resources->GetAnimation("walkLayer")->SetFrame(theCollision);

						//Set color values
						if (theCollision >= UP_ARROW && theCollision <= LEFT_ARROW) {
							if (ids[i+xGridOffset][j+yGridOffset] == -1 || ids[i+xGridOffset][j+yGridOffset] == 990) {
								//render normal, red arrow
								resources->GetAnimation("walkLayer")->SetColor(ARGB(255,255,0,255));							
							} else { 
								//it's a rotating arrow, make it green
								resources->GetAnimation("walkLayer")->SetColor(ARGB(255,0,255,255));					
							}
						} else if (theCollision == SHALLOW_WATER) {
							resources->GetAnimation("walkLayer")->SetColor(ARGB(125,255,255,255));
						} else if (theCollision == SLIME) {
							resources->GetAnimation("walkLayer")->SetColor(ARGB(200,255,255,255));
						}

						//Draw it and set the color back to normal
						resources->GetAnimation("walkLayer")->Render(drawX,drawY);
						resources->GetAnimation("walkLayer")->SetColor(ARGB(255,255,255,255));

					}
				}

				//Items
				int theItem = item[i+xGridOffset][j+yGridOffset];
				if (theItem != NONE && ids[i+xGridOffset][j+yGridOffset] != DRAW_AFTER_SMILEY) {
					if (theItem == ENEMYGROUP_BLOCKGRAPHIC) {
						//If this is an enemy block, draw it with the enemy group's
						//block alpha
						itemLayer[theItem]->SetColor(ARGB(
							enemyGroupManager->groups[variable[i+xGridOffset][j+yGridOffset]].blockAlpha, 255, 255, 255));
						itemLayer[theItem]->Render(drawX,drawY);
						itemLayer[theItem]->SetColor(ARGB(255,255,255,255));
					} else {
						itemLayer[theItem]->Render(drawX,drawY);
					}
				}
			
			} else {
				resources->GetSprite("blackScreen")->RenderStretch(drawX, drawY, drawX+64, drawY+64);
			}
		}
	}

	//If the area has a big ass fountain draw the top part after the player
	if (hasFountain && fountainOnScreen) {
		resources->GetSprite("bigFountainBottom")->Render(fountainX*64.0 - xOffset + 32, fountainY*64.0 - yOffset + 32);
	}

	//Draw environment particles
	environmentParticles->Update(dt);
	environmentParticles->Transpose(-1*(xGridOffset*64 + xOffset), -1*(yGridOffset*64 + yOffset));
	environmentParticles->Render();

	//Debug mode stuff
	if (debugMode) {

		//Column lines
		for (int i = 0; i <= screenWidth; i++) {
			hge->Gfx_RenderLine(i*64.0 - xOffset,0,i*64.0 - xOffset,SCREEN_HEIGHT);
		}
		//Row lines
		for (int i = 0; i <= screenHeight; i++) {
			hge->Gfx_RenderLine(0,i*64.0 - yOffset,SCREEN_WIDTH,i*64.0 - yOffset);
		}
		//Draw Terrain collision boxes
		for (int i = thePlayer->gridX - 2; i <= thePlayer->gridX + 2; i++) {
			for (int j = thePlayer->gridY - 2; j <= thePlayer->gridY + 2; j++) {
				if (inBounds(i,j)) {
					//Set collision box depending on collision type
					if (hasSillyPad(i,j)) {
						collisionBox->SetRadius(i*64+32,j*64+32,24);
					} else {
						setTerrainCollisionBox(collisionBox, collision[i][j], i, j);
					}
					if (!thePlayer->canPass(collision[i][j]) || hasSillyPad(i,j)) drawCollisionBox(collisionBox, GREEN);
				}
			}
		}
	}

	//Draw the evil walls
	evilWallManager->draw(dt);

	//Draw tapestries
	tapestryManager->draw(dt);

	//Draw smilelets
	smilelet->drawBeforeSmiley();
	smilelet->drawAfterSmiley();

	//Draw the mushrooms
	specialTileManager->draw(dt);

}


/**
 * Draws stuff on the item layer that was marked to be drawn after
 * Smiley (indicated by ID 990) as well as shrink tunnels.
 */
void Environment::drawAfterSmiley(float dt) {

	//Loop through each tile to draw shit
	for (int gridY = yGridOffset-1; gridY <= yGridOffset + screenHeight + 1; gridY++) {
		for (int gridX = xGridOffset-1; gridX <= xGridOffset + screenWidth + 1; gridX++) {
			if (inBounds(gridX, gridY)) {

				drawX = getScreenX(gridX * 64);
				drawY = getScreenY(gridY * 64);

				//Marked as draw after smiley or the thing above Smiley is marked as draw
				//above Smiley and Smiley is behind it. That way you can't walk behind a tree
				//and lick through it.
				if (ids[gridX][gridY] == DRAW_AFTER_SMILEY || 
						(ids[gridX][gridY-1] == DRAW_AFTER_SMILEY && 
						thePlayer->gridX == gridX && 
						thePlayer->gridY < gridY)) {
					itemLayer[item[gridX][gridY]]->Render(drawX,drawY);
				}

				//Shrink tunnels unless Smiley is directly underneath it and not 
				//yet in the shrink tunnel
				if ((collision[gridX][gridY] == SHRINK_TUNNEL_HORIZONTAL || 
						collision[gridX][gridY] == SHRINK_TUNNEL_VERTICAL) &&
						!(thePlayer->gridY == gridY+1 && !thePlayer->isInShrinkTunnel())) {
					resources->GetAnimation("walkLayer")->SetFrame(collision[gridX][gridY]);
					resources->GetAnimation("walkLayer")->Render(drawX, drawY);
				}

			}
		}
	}
}


/**
 * Draws a big ass fountain. This is called after the main draw() function is called for
 * both the Environment and the Player so that it is drawn over top of the Player. It only
 * draws the top part of the fountain which the player can walk behind
 */
void Environment::drawFountain() {
	if (!hasFountain || !fountainOnScreen) return;
	if (-400 < getScreenX(fountainX) < 1700 && -400 < getScreenY(fountainY) < 1600) {
		resources->GetSprite("bigFountainTop")->Render(fountainX*64.0 - xOffset + 32, fountainY*64.0 - yOffset + 32);
		resources->GetAnimation("fountainRipple")->Render(fountainX*64.0 - xOffset + 32, fountainY*64.0 - yOffset - 40);
		resources->GetSprite("smallFountain")->Render(fountainX*64.0 - xOffset + 32, fountainY*64.0 - yOffset - 83);
		resources->GetAnimation("fountainRipple")->RenderEx(fountainX*64.0 - xOffset + 32, fountainY*64.0 - yOffset - 183,0.0f,.35f,.4f);
		resources->GetParticleSystem("fountain")->MoveTo(fountainX*64.0 - xOffset + 32, fountainY*64.0 - yOffset - 180, true);
		resources->GetParticleSystem("fountain")->Render();
	}
}


/**
 * Update the environment variables
 */
void Environment::update(float dt) {

	//Update animations and shit
	resources->GetAnimation("water")->Update(dt);
	resources->GetAnimation("greenWater")->Update(dt);
	resources->GetAnimation("lava")->Update(dt);
	resources->GetAnimation("spring")->Update(dt);
	resources->GetAnimation("superSpring")->Update(dt);
	resources->GetAnimation("silverSwitch")->Update(dt);
	resources->GetAnimation("brownSwitch")->Update(dt);
	resources->GetAnimation("blueSwitch")->Update(dt);
	resources->GetAnimation("greenSwitch")->Update(dt);
	resources->GetAnimation("yellowSwitch")->Update(dt);
	resources->GetAnimation("whiteSwitch")->Update(dt);
	silverCylinder->Update(dt);
	brownCylinder->Update(dt);
	blueCylinder->Update(dt);
	greenCylinder->Update(dt);
	yellowCylinder->Update(dt);
	whiteCylinder->Update(dt);
	silverCylinderRev->Update(dt);
	brownCylinderRev->Update(dt);
	blueCylinderRev->Update(dt);
	greenCylinderRev->Update(dt);
	yellowCylinderRev->Update(dt);
	whiteCylinderRev->Update(dt);
	resources->GetAnimation("fountainRipple")->Update(dt);
	resources->GetParticleSystem("fountain")->Update(dt);

	//Determine the grid offset to figure out which tiles to draw
	xGridOffset = thePlayer->gridX - screenWidth/2;
	yGridOffset = thePlayer->gridY - screenHeight/2;

	//Determine the tile offset for smooth movement
	xOffset = thePlayer->x - float(thePlayer->gridX) * float(64.0);
	yOffset = thePlayer->y - float(thePlayer->gridY) * float(64.0);

	//Update each grid square
	for (int i = 0; i < areaWidth; i++) {
		for (int j = 0; j < areaHeight; j++) {

			//Update timed switches
			if (isCylinderSwitchLeft(collision[i][j]) || isCylinderSwitchRight(collision[i][j])) {
				if (variable[i][j] != -1 && activated[i][j] + (float)variable[i][j] < gameTime && 
						saveManager->isTileChanged( i, j)) {
					
					//Make sure the player isn't on top of any of the cylinders that will pop up
					if (!playerOnCylinder(i,j)) {
						flipCylinderSwitch(i, j);
					}	
	
				}
			}
		}
	}

	//update the mushrooms
	specialTileManager->update(dt);

	//update the evil walls
	evilWallManager->update(dt);

	//Update tapestries
	tapestryManager->update(dt);

	
	//Update smilelets
	smilelet->update();

}

/**
 * Returns what type of collision there is at point (x,y)
 */
int Environment::collisionAt(float x, float y) {
	
	//Determine grid coords of the object
	int gridX = x / (float)64.0;
	int gridY = y / (float)64.0;

	//Handle out of bounds input
	if (!inBounds(gridX,gridY)) {
		return UNWALKABLE;
	}

	//Return the collision type
	return collision[gridX][gridY];

}


void Environment::unlockDoor(int gridX, int gridY) {
	bool doorOpened = false;

	//If this square is a door and the player has the key, unlock it
	if (collision[gridX][gridY] == RED_KEYHOLE && saveManager->numKeys[getKeyIndex(saveManager->currentArea)][RED_KEY-1] > 0) {
		collision[gridX][gridY] = WALKABLE;
		saveManager->numKeys[getKeyIndex(saveManager->currentArea)][RED_KEY-1]--;
		doorOpened = true;
	} else  if (collision[gridX][gridY] == BLUE_KEYHOLE && saveManager->numKeys[getKeyIndex(saveManager->currentArea)][BLUE_KEY-1] > 0) {
		collision[gridX][gridY] = WALKABLE;
		saveManager->numKeys[getKeyIndex(saveManager->currentArea)][BLUE_KEY-1]--;
		doorOpened = true;
	} else if (collision[gridX][gridY] == YELLOW_KEYHOLE && saveManager->numKeys[getKeyIndex(saveManager->currentArea)][YELLOW_KEY-1] > 0) {
		collision[gridX][gridY] = WALKABLE;
		saveManager->numKeys[getKeyIndex(saveManager->currentArea)][YELLOW_KEY-1]--;
		doorOpened = true;
	} else if (collision[gridX][gridY] == GREEN_KEYHOLE && saveManager->numKeys[getKeyIndex(saveManager->currentArea)][GREEN_KEY-1] > 0) {
		collision[gridX][gridY] = WALKABLE;
		saveManager->numKeys[getKeyIndex(saveManager->currentArea)][GREEN_KEY-1]--;
		doorOpened = true;
	}

	//Remember that this door was opened!
	if (doorOpened) {
		saveManager->change( gridX, gridY);
	}

}

/**
 * Toggles any switches hit by a collision box. Returns whether or not a
 * switch was toggled.
 */
bool Environment::toggleSwitches(hgeRect *box) {
	return toggleSwitches(box, true);
}

/**
 * Toggles switches hit by a collision box. Returns whether or not a switch
 * was toggled.
 *
 * @param playSoundFarAway  If this is true the switch sound will always play even if Smiley is
 *							really far away
 */
bool Environment::toggleSwitches(hgeRect *box, bool playSoundFarAway) {

	//Determine what grid square the collision box is in
	int boxGridX = (box->x1 + ((box->x2-box->x1)/2)) / 64;
	int boxGridY = (box->y1 + ((box->y2-box->y1)/2)) / 64;

	//Loop through all the adjacent grid squares
	for (int gridX = boxGridX - 2; gridX <= boxGridX + 2; gridX++) {
		for (int gridY = boxGridY - 2; gridY <= boxGridY + 2; gridY++) {

			//Make sure the square is in bounds
			if (inBounds(gridX,gridY)) {

				//Set collision box for this square
				setTerrainCollisionBox(collisionBox,collision[gridX][gridY],gridX,gridY);
				
				//Check collision with any switches
				if (timePassedSince(activated[gridX][gridY]) > .5 && collisionBox->Intersect(box)) {
					if (toggleSwitchAt(gridX, gridY, playSoundFarAway)) return true;
				}
			}
		}
	}

	//No switches were toggled so return false;
	return false;
}


/**
 * Toggles any switches hit by smiley's tongue. Returns whether or not a
 * switch was toggled.
 */
bool Environment::toggleSwitches(Tongue *tongue) {
	
	//Loop through all the squares adjacent to Smiley
	for (int gridX = thePlayer->gridX - 2; gridX <= thePlayer->gridX + 2; gridX++) {
		for (int gridY = thePlayer->gridY - 2; gridY <= thePlayer->gridY + 2; gridY++) {

			//Make sure the square is in bounds
			if (inBounds(gridX,gridY)) {

				//Set collision box for this square
				setTerrainCollisionBox(collisionBox,collision[gridX][gridY],gridX,gridY);
				
				//Check collision with any switches
				if (timePassedSince(activated[gridX][gridY]) > .5 && tongue->testCollision(collisionBox)) {			
					if (toggleSwitchAt(gridX, gridY, true)) {
						return true;
					}
				}
			}
		}
	}

	//No switches were toggled so return false;
	return false;

}

/**
 * Toggles a switch.
 *
 * @param id	id of the switch to toggle.
 */
void Environment::toggleSwitch(int id) {

	if (id < 0) {
		hge->System_Log("ERROR: Environment.toggleSwitch() Attemping to toggle switch %d", id);
		return;
	}

	//Scan the area to find the switch then toggle it
	for (int i = 0; i < areaWidth; i++) {
		for (int j = 0; j < areaHeight; j++) {
			if (ids[i][j] == id && (isCylinderSwitchLeft(collision[i][j]) || isCylinderSwitchLeft(collision[i][j]))) {
				toggleSwitchAt(i,j,true);
				return;
			}
		}
	}

}

/** 
 * Attempts to toggle a switch at (gridX, gridY). Returns whether or not there is
 * a switch there to toggle.
 * 
 * @param playSoundFarAway  If this is true the switch sound will always play even if Smiley is
 *							really far away
 */
bool Environment::toggleSwitchAt(int gridX, int gridY, bool playSoundFarAway) {
	
	int switchID = ids[gridX][gridY];
	bool hasSwitch = false;
		
	//Flip cylinder switch
	if (isCylinderSwitchLeft(collision[gridX][gridY]) || isCylinderSwitchRight(collision[gridX][gridY])) {

		flipCylinderSwitch(gridX, gridY);
		hasSwitch = true;

	//Rotate Shrink tunnels
	} else if (collision[gridX][gridY] == SHRINK_TUNNEL_SWITCH) {

		hasSwitch = true;
		activated[gridX][gridY] = gameTime;
		//Loop through the grid and look for shrink tunnels with the same id as the switch
		for (int i = 0; i < areaWidth; i++) {
			for (int j = 0; j < areaHeight; j++) {
				if (ids[i][j] == switchID) {
					//When found, rotate clockwise.
					if (collision[i][j] == SHRINK_TUNNEL_HORIZONTAL) 
						collision[i][j] = SHRINK_TUNNEL_VERTICAL;
					else if (collision[i][j] == SHRINK_TUNNEL_VERTICAL) 
						collision[i][j] = SHRINK_TUNNEL_HORIZONTAL;
				}
			}
		}

	//Rotate arrows switch
	} else if (collision[gridX][gridY] == SPIN_ARROW_SWITCH) {

		hasSwitch = true;
		activated[gridX][gridY] = gameTime;

		//Loop through the grid and look for arrows with the same id as the switch
		for (int i = 0; i < areaWidth; i++) {
			for (int j = 0; j < areaHeight; j++) {
				if (ids[i][j] == switchID) {
					//When found, rotate clockwise.
					if (collision[i][j] == UP_ARROW) 
						collision[i][j] = RIGHT_ARROW;
					else if (collision[i][j] == RIGHT_ARROW) 
						collision[i][j] = DOWN_ARROW;
					else if (collision[i][j] == DOWN_ARROW) 
						collision[i][j] = LEFT_ARROW;
					else if (collision[i][j] == LEFT_ARROW) 
						collision[i][j] = UP_ARROW;
				}
			}
		}

	//Rotate mirrors switch
	} else if (collision[gridX][gridY] == MIRROR_SWITCH) {

		hasSwitch = true;
		activated[gridX][gridY] = gameTime;
		
		//Switch up and down cylinders
		for (int i = 0; i < areaWidth; i++) {
			for (int j = 0; j < areaHeight; j++) {
				if (ids[i][j] == switchID) {
					if (collision[i][j] == MIRROR_UP_LEFT) collision[i][j] = MIRROR_UP_RIGHT;
					else if (collision[i][j] == MIRROR_UP_RIGHT) collision[i][j] = MIRROR_DOWN_RIGHT;
					else if (collision[i][j] == MIRROR_DOWN_RIGHT) collision[i][j] = MIRROR_DOWN_LEFT;
					else if (collision[i][j] == MIRROR_DOWN_LEFT) collision[i][j] = MIRROR_UP_LEFT;
				}
			}
		}
	}

	//Play switch sound if the switch is somewhat close to Smiley
	if (hasSwitch && (playSoundFarAway || distance(gridX, gridY, thePlayer->gridX, thePlayer->gridY) < 15)) {
		hge->Effect_Play(resources->GetEffect("snd_switch"));
	}

	return hasSwitch;
}

/**
 * Flips the cylinder switch at gridX, gridY
 */  
void Environment::flipCylinderSwitch(int gridX, int gridY) {

	activated[gridX][gridY] = gameTime;

	//Flip switch in collision layer
	if (isCylinderSwitchLeft(collision[gridX][gridY])) {
		collision[gridX][gridY] += 16;
		resources->GetAnimation("silverSwitch")->SetMode(HGEANIM_FWD);
		resources->GetAnimation("brownSwitch")->SetMode(HGEANIM_FWD);
		resources->GetAnimation("blueSwitch")->SetMode(HGEANIM_FWD);
		resources->GetAnimation("greenSwitch")->SetMode(HGEANIM_FWD);
		resources->GetAnimation("yellowSwitch")->SetMode(HGEANIM_FWD);
		resources->GetAnimation("whiteSwitch")->SetMode(HGEANIM_FWD);
		saveManager->change(gridX, gridY);
	} else if (isCylinderSwitchRight(collision[gridX][gridY])) {
		collision[gridX][gridY] -= 16;
		resources->GetAnimation("silverSwitch")->SetMode(HGEANIM_REV);
		resources->GetAnimation("brownSwitch")->SetMode(HGEANIM_REV);
		resources->GetAnimation("blueSwitch")->SetMode(HGEANIM_REV);
		resources->GetAnimation("greenSwitch")->SetMode(HGEANIM_REV);
		resources->GetAnimation("yellowSwitch")->SetMode(HGEANIM_REV);
		resources->GetAnimation("whiteSwitch")->SetMode(HGEANIM_REV);
		saveManager->change(gridX, gridY);
	}

	//Play animation
	resources->GetAnimation("silverSwitch")->Play();
	resources->GetAnimation("brownSwitch")->Play();
	resources->GetAnimation("blueSwitch")->Play();
	resources->GetAnimation("greenSwitch")->Play();
	resources->GetAnimation("yellowSwitch")->Play();
	resources->GetAnimation("whiteSwitch")->Play();
	activated[gridX][gridY] = gameTime;

	//Switch up and down cylinders if the player isn't on top of any down cylindersw
	if (!playerOnCylinder(gridX,gridY)) {
		switchCylinders(ids[gridX][gridY]);
	}

}

/**
 * Switches all cylinders with the specified ID.
 */
void Environment::switchCylinders(int switchID) {

	//Switch up and down cylinders if the player isn't on top of any down cylindersw
	for (int i = 0; i < areaWidth; i++) {
		for (int j = 0; j < areaHeight; j++) {
			if (ids[i][j] == switchID) {
				if (isCylinderUp(collision[i][j])) {
					collision[i][j] -= 16;
					activated[i][j] = gameTime;
				} else if (isCylinderDown(collision[i][j])) {
					collision[i][j] += 16;
					activated[i][j] = gameTime;
				}
				silverCylinder->Play();
				brownCylinder->Play();
				blueCylinder->Play();
				greenCylinder->Play();
				yellowCylinder->Play();
				whiteCylinder->Play();
				silverCylinderRev->Play();
				brownCylinderRev->Play();
				blueCylinderRev->Play();
				greenCylinderRev->Play();
				yellowCylinderRev->Play();
				whiteCylinderRev->Play();
			}
		}
	}
}


/**
 * Returns the item in a square if any and removes it.
 */
int Environment::gatherItem(int x, int y) {
	int retVal = item[x][y];
	if (retVal > 0 && retVal < 16) {
		item[x][y] = NONE;
		saveManager->change( x, y);
		return retVal;
	} else {
		return NONE;
	}
}

/**
 * Returns whether or not there is an unobstructed straight line from 
 * pixel position (x1, y1) to (x2, y2).
 *
 *	x1, y1		point at the start of the path
 *	x2, y2		point at the end of the path
 *	radius		radius of the object taking the path
 *
 */
bool Environment::validPath(int x1, int y1, int x2, int y2, int radius, bool canPath[256]) {
	
	//First get the velocities of the path
	float angle = getAngleBetween(x1,y1,x2,y2);
	float dx = 10.0*cos(angle);
	float dy = 10.0*sin(angle);

	//Now trace the path using dx and dy and see if you run into any SHIT
	float xTravelled = 0;
	float yTravelled = 0;
	float curX = x1;
	float curY = y1;

	//This can throw an exception if the enemy is perfectly on top of the player
	try {
		while (abs(xTravelled) < abs(x2 - x1) && abs(yTravelled) < abs(y2 - y1)) {
			//Top left of the object
			if (!canPath[collisionAt(curX-radius, curY-radius)] || hasSillyPad(int(curX-radius)/64,int(curY-radius)/64)) return false;
			//Top right of the object
			if (!canPath[collisionAt(curX+radius, curY-radius)] || hasSillyPad(int(curX+radius)/64,int(curY-radius)/64)) return false;
			//Bottom left of the object
			if (!canPath[collisionAt(curX-radius, curY+radius)] || hasSillyPad(int(curX-radius)/64,int(curY+radius)/64)) return false;
			//Bottom right of the object
			if (!canPath[collisionAt(curX+radius, curY+radius)] || hasSillyPad(int(curX+radius)/64,int(curY+radius)/64)) return false;
			curX += dx;
			curY += dy;
			xTravelled += dx;
			yTravelled += dy;
		}
	} catch(int type) {
		return true;
	}

	//You didnt hit any SHIT so return true
	return true;
}


/**
 * Returns whether or not player, when centered at (x,y), collides with any terrain. 
 * Also autoadjusts the player's position to navigate corners.
 * 
 * This should be moved to the Player class.
 *
 * @arg x		x-coord of the player
 * @arg y		y-coord of the player
 * @arg dt
 */
bool Environment::playerCollision(int x, int y, float dt) {
	
	//Determine the location of the collision box
	int gridX = x / 64;
	int gridY = y / 64;

    bool onIce = collision[thePlayer->gridX][thePlayer->gridY] == ICE;

	//Check all neighbor squares
	for (int i = gridX - 2; i <= gridX + 2; i++) {
		for (int j = gridY - 2; j <= gridY + 2; j++) {

			//Special logic for shrink tunnels
			bool canPass;
			if (collision[i][j] == SHRINK_TUNNEL_HORIZONTAL) {
				canPass = thePlayer->isShrunk() && j == thePlayer->gridY;
			} else if (collision[i][j] == SHRINK_TUNNEL_VERTICAL) {
				canPass = thePlayer->isShrunk() && i == thePlayer->gridX;
			} else {
				canPass = thePlayer->canPass(collision[i][j]);
			}

			//Ignore squares off the map
			if (inBounds(i,j) && !canPass) {
		
				//Set collision box depending on collision type
				setTerrainCollisionBox(collisionBox, collision[i][j], i, j);

				//Test top and bottom of box
				if (x > collisionBox->x1 && x < collisionBox->x2) {
					if (abs(collisionBox->y2 - y) < thePlayer->radius) return true;
					if (abs(collisionBox->y1 - y) < thePlayer->radius) return true;
				}

				//Test left and right side of box
				if (y > collisionBox->y1 && y < collisionBox->y2) {
					if (abs(collisionBox->x2 - x) < thePlayer->radius) return true;
					if (abs(collisionBox->x1 - x) < thePlayer->radius) return true;
				}

				bool onlyDownPressed = input->keyDown(INPUT_DOWN) && !input->keyDown(INPUT_UP) && !input->keyDown(INPUT_LEFT) && !input->keyDown(INPUT_RIGHT);
				bool onlyUpPressed = !input->keyDown(INPUT_DOWN) && input->keyDown(INPUT_UP) && !input->keyDown(INPUT_LEFT) && !input->keyDown(INPUT_RIGHT);
				bool onlyLeftPressed = !input->keyDown(INPUT_DOWN) && !input->keyDown(INPUT_UP) && input->keyDown(INPUT_LEFT) && !input->keyDown(INPUT_RIGHT);
				bool onlyRightPressed = !input->keyDown(INPUT_DOWN) && !input->keyDown(INPUT_UP) && !input->keyDown(INPUT_LEFT) && input->keyDown(INPUT_RIGHT);
				float angle;

				//Top left corner
				if (distance(collisionBox->x1, collisionBox->y1, x, y) < thePlayer->radius) {
					if (thePlayer->isOnIce()) return true;
					angle = getAngleBetween(collisionBox->x1, collisionBox->y1, thePlayer->x, thePlayer->y);
					if (onlyDownPressed && thePlayer->facing == DOWN && x < collisionBox->x1 && thePlayer->canPass(collision[i-1][j]) && !hasSillyPad(i-1,j) && !onIce) {
						angle -= 4.0 * PI * dt;
					} else if (onlyRightPressed && thePlayer->facing == RIGHT && y < collisionBox->y1 && thePlayer->canPass(collision[i][j-1]) && !hasSillyPad(i,j-1) && !onIce) {
						angle += 4.0 * PI * dt;
					} else return true;
					thePlayer->x = collisionBox->x1 + (thePlayer->radius+1) * cos(angle);
					thePlayer->y = collisionBox->y1 + (thePlayer->radius+1) * sin(angle);
					return true;
				}

				//Top right corner
				if (distance(collisionBox->x2, collisionBox->y1, x, y) < thePlayer->radius) {
					if (thePlayer->isOnIce()) return true;
					angle = getAngleBetween(collisionBox->x2, collisionBox->y1, thePlayer->x, thePlayer->y);
					if (onlyDownPressed && thePlayer->facing == DOWN && x > collisionBox->x2 && thePlayer->canPass(collision[i+1][j]) && !hasSillyPad(i+1,j) && !onIce) {
						angle += 4.0 * PI * dt;
					} else if (onlyLeftPressed && thePlayer->facing == LEFT && y < collisionBox->y1 && thePlayer->canPass(collision[i][j-1]) && !hasSillyPad(i,j-1) && !onIce) {
						angle -= 4.0 * PI * dt;
					} else return true;
					thePlayer->x = collisionBox->x2 + (thePlayer->radius+1) * cos(angle);
					thePlayer->y = collisionBox->y1 + (thePlayer->radius+1) * sin(angle);
					return true;
				}

				//Bottom right corner
				if (distance(collisionBox->x2, collisionBox->y2, x, y) < thePlayer->radius) {
					if (thePlayer->isOnIce()) return true;
					angle = getAngleBetween(collisionBox->x2, collisionBox->y2, thePlayer->x, thePlayer->y);
					if (onlyUpPressed && thePlayer->facing == UP && x > collisionBox->x2 && thePlayer->canPass(collision[i+1][j]) && !hasSillyPad(i+1,j) && !onIce) {
						angle -= 4.0 * PI * dt;
					} else if (onlyLeftPressed && thePlayer->facing == LEFT && y > collisionBox->y2 && thePlayer->canPass(collision[i][j+1]) && !hasSillyPad(i,j+1) && !onIce) {
						angle += 4.0 * PI * dt;
					} else return true;
					thePlayer->x = collisionBox->x2 + (thePlayer->radius+1) * cos(angle);
					thePlayer->y = collisionBox->y2 + (thePlayer->radius+1) * sin(angle);
					return true;
				}
				
				//Bottom left corner
				if (distance(collisionBox->x1, collisionBox->y2, x, y) < thePlayer->radius) {
					if (thePlayer->isOnIce()) return true;
					angle = getAngleBetween(collisionBox->x1, collisionBox->y2, thePlayer->x, thePlayer->y);
					if (onlyUpPressed && thePlayer->facing == UP && x < collisionBox->x1 && thePlayer->canPass(collision[i-1][j]) && !hasSillyPad(i-1,j) && !onIce) {
						angle += 4.0 * PI * dt;
					} else if (onlyRightPressed && thePlayer->facing == RIGHT && y > collisionBox->y2 && thePlayer->canPass(collision[i][j+1]) && !hasSillyPad(i, j+1) && !onIce) {
						angle -= 4.0 * PI * dt;
					} else return true;
					thePlayer->x = collisionBox->x1 + (thePlayer->radius+1) * cos(angle);
					thePlayer->y = collisionBox->y2 + (thePlayer->radius+1) * sin(angle);
					return true;
				}
				
			}
		}
	}

	return false;
}


/**
 * Bombs any bombable walls at the given coordinates
 */
void Environment::bombWall(int x,int y) {
	if (inBounds(x,y) && collision[x][y] == BOMBABLE_WALL) {
		collision[x][y]=WALKABLE;
		saveManager->change( x, y);
	}
}

/**
 * Returns whether or not an enemy bounded by *box box collides with any terrain. Also
 * adjusts the enemy's position to help it round corners
 */
bool Environment::enemyCollision(hgeRect *box, BaseEnemy *enemy, float dt) {
	
	//Determine the location of the collision box
	int gridX = (box->x1 + (box->x2 - box->x1)/2) / 64;
	int gridY = (box->y1 + (box->y2 - box->y1)/2) / 64;

	//Check all neighbor squares
	for (int i = gridX - 2; i <= gridX + 2; i++) {
		for (int j = gridY - 2; j <= gridY + 2; j++) {
			//Ignore squares off the map
			if (inBounds(i,j) && (!enemy->canPass[collision[i][j]] || hasSillyPad(i,j))) {
				//Test collision
				setTerrainCollisionBox(collisionBox, hasSillyPad(i,j) ? UNWALKABLE : collision[i][j], i, j);
				if (box->Intersect(collisionBox)) {

					//Help the enemy round corners
					if ((int)enemy->dx == 0 && enemy->dy > 0) {
						//Moving down
						if (enemy->x < collisionBox->x1) {
							enemy->x -= enemy->speed * dt;
						} else if (enemy->x > collisionBox->x2) {
							enemy->x += enemy->speed * dt;
						}
					} else if ((int)enemy->dx == 0 && enemy->dy < 0) {
						//Moving up
						if (enemy->x < collisionBox->x1) {
							enemy->x -= enemy->speed * dt;
						} else if (enemy->x > collisionBox->x2) {
							enemy->x += enemy->speed * dt;
						}
					} else if (enemy->dx < 0 && (int)enemy->dy == 0) {
						//Moving left
						if (enemy->y < collisionBox->y1) {
							enemy->y -= enemy->speed * dt;
						} else if (enemy->y > collisionBox->y2) {
							enemy->y += enemy->speed * dt;
						}
					} else if (enemy->dx > 0 && (int)enemy->dy == 0) {
						//Moving right
						if (enemy->y < collisionBox->y1) {
							enemy->y -= enemy->speed * dt;
						} else if (enemy->y > collisionBox->y2) {
							enemy->y += enemy->speed * dt;
						}
					}

					return true;
				}
			} else if (!inBounds(i,j)) {
				return true;
			}
		}
	}

	//No collision occured, so return false
	return false;
}

/**
 * Reads any sign within *box.
 */
bool Environment::hitSigns(Tongue *tongue) {
	
	std::string paramString;

	for (int i = thePlayer->gridX - 2; i <= thePlayer->gridX + 2; i++) {
		for (int j = thePlayer->gridY - 2; j <= thePlayer->gridY + 2; j++) {
			if (inBounds(i,j) && collision[i][j] == SIGN) {
				collisionBox->SetRadius(i*64+32,j*64+32,24);
				if (tongue->testCollision(collisionBox)) {
					windowManager->openSignTextBox(ids[i][j]);
					return true;
				}
			}
		}
	}
	return false;
}


/**
 * Opens the save menu if box collides with a save shrine
 */
bool Environment::hitSaveShrine(Tongue *tongue) {
	for (int i = thePlayer->gridX - 2; i <= thePlayer->gridX + 2; i++) {
		for (int j = thePlayer->gridY - 2; j <= thePlayer->gridY + 2; j++) {
			if (inBounds(i,j) && collision[i][j] == SAVE_SHRINE) {
				collisionBox->SetRadius(i*64+32,j*64+32,24);
				if (tongue->testCollision(collisionBox)) {
					windowManager->openWindow(new MiniMenu(MINIMENU_SAVEGAME));
					return true;
				}
			}
		}
	}
	return false;
}

/**
 * Returns whether or not box collides with any silly pads or terrain 
 * as specified by canPass.
 */
bool Environment::testCollision(hgeRect *box, bool canPass[256]) {
	return testCollision(box, canPass, false);
}

/**
 * Returns whether or not box collides with terrain as specified by canPass.
 * 
 * @param ignoreSillyPads	If true, hitting silly pads won't be counted as collision
 */
bool Environment::testCollision(hgeRect *box, bool canPass[256], bool ignoreSillyPads) {

	//Determine the location of the collision box
	int gridX = (box->x1 + (box->x2 - box->x1)/2) / 64;
	int gridY = (box->y1 + (box->y2 - box->y1)/2) / 64;

	//Check all neighbor squares
	for (int i = gridX - 2; i <= gridX + 2; i++) {
		for (int j = gridY - 2; j <= gridY + 2; j++) {
			//Ignore squares off the map
			if (inBounds(i,j) && (!canPass[collision[i][j]] || 
					(!ignoreSillyPads && hasSillyPad(i,j)))) {
				
				//Test collision
				setTerrainCollisionBox(collisionBox, (!ignoreSillyPads && hasSillyPad(i,j)) 
						? UNWALKABLE : collision[i][j], i, j);

				if (box->Intersect(collisionBox)) {
					return true;
				}
			}
		}
	}

	//No collision occured, so return false
	return false;

}


/**
 * Returns whether or not the player is on top of any cylinders that will pop up
 * when the switch at grid position (x,y) is toggled.
 */
bool Environment::playerOnCylinder(int x, int y) {
	bool retVal = false;
	//Make sure the player isn't on top of any of the cylinders that will pop up
	for (int k = thePlayer->gridX-1; k <= thePlayer->gridX+1; k++) {
		for (int l = thePlayer->gridY-1; l <= thePlayer->gridY+1; l++) {
			if (ids[k][l] == ids[x][y] && isCylinderDown(collision[k][l])) {
				collisionBox->SetRadius(k*64+32,l*64+32,32);
				//Player collides with a cylinder
				if (thePlayer->collisionCircle->testBox(collisionBox)) {
					retVal = true;
				}
			}
		}
	}
	return retVal;
}

/**
 * Returns whether or not there is deep water of any kind at grid (x,y)
 */
bool Environment::isDeepWaterAt(int x, int y) {
	return (collision[x][y] == DEEP_WATER || collision[x][y] == GREEN_WATER);
}

/**
 * Returns whether or not there is a SMILELET FLOWER at grid (x,y)
 */
bool Environment::isSadSmileletFlowerAt(int x,int y) {
	return (collision[x][y] == SMILELET_FLOWER_SAD);
}

/**
 * Returns whether or not there is shallow water of any kind at grid (x,y)
 */ 
bool Environment::isShallowWaterAt(int x, int y) {
	return (collision[x][y] == SHALLOW_WATER || collision[x][y] == SHALLOW_GREEN_WATER);
}

bool Environment::isArrowAt(int x, int y) {
	return (collision[x][y] >= UP_ARROW && collision[x][y] <= LEFT_ARROW);
}

/**
 * Set a collision box for the speicifed collision type decalared in smiley.
 * This allows different things to have different shaped collision boxes.
 */
void Environment::setTerrainCollisionBox(hgeRect *box, int whatFor, int gridX, int gridY) {
	if (whatFor == FOUNTAIN) {
		//Fountain
		box->Set((gridX-1)*64,gridY*64 + 35,(gridX+2)*64,(gridY+1)*64 + 10);
	} else {
		box->SetRadius(gridX*64+32,gridY*64+31,31);
	}
}

/**
 * Places a silly pad at the specified grid location.
 */
void Environment::placeSillyPad(int gridX, int gridY) {
	specialTileManager->addSillyPad(gridX, gridY);

	//Play sound effect
	hge->Effect_Play(resources->GetEffect("snd_sillyPad"));
}

bool Environment::hasSillyPad(int gridX, int gridY) {
	return specialTileManager->isSillyPadAt(gridX, gridY);
}

void Environment::addTimedTile(int gridX, int gridY, int tile, float duration) {
	specialTileManager->addTimedTile(gridX, gridY, tile, duration);
}

bool Environment::isTimedTileAt(int gridX, int gridY) {
	return specialTileManager->isTimedTileAt(gridX, gridY);
}

bool Environment::isTimedTileAt(int gridX, int gridY, int tile) {
	return specialTileManager->isTimedTileAt(gridX, gridY, tile);
}