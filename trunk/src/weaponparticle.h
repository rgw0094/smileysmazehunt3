#ifndef WEAPONPARTICLE_H
#define WEAPONPARTICLE_H

#include "hge.h"
#include "hgecolor.h"
#include "hgerect.h"
#include "hgevector.h"
#include "hgesprite.h"

#define MAX_PARTICLES	750
#define PARTICLE_FIRE_BREATH 0
#define PARTICLE_ICE_BREATH 1
#define PARTICLE_FIRE_NOVA 2
#define PARTICLE_ICE_NOVA 3

#define ICE_NOVA_FREEZE_DURATION 2.0

struct weaponParticle {
	hgeVector	vecLocation;
	hgeVector	vecVelocity;
	float		fGravity;
	float		fRadialAccel;
	float		fTangentialAccel;
	float		fSpin;
	float		fSpinDelta;
	float		fSize;
	float		fSizeDelta;
	hgeColor	colColor;		// + alpha
	hgeColor	colColorDelta;
	float		fAge;
	float		fTerminalAge;
};

struct weaponParticleSystemInfo {
	hgeSprite*	sprite;    // texture + blend mode
	int			nEmission; // particles per sec
	float		fLifetime;
	float		fParticleLifeMin;
	float		fParticleLifeMax;
	float		fDirection;
	float		fSpread;
	bool		bRelative;
	float		fSpeedMin;
	float		fSpeedMax;
	float		fGravityMin;
	float		fGravityMax;
	float		fRadialAccelMin;
	float		fRadialAccelMax;
	float		fTangentialAccelMin;
	float		fTangentialAccelMax;
	float		fSizeStart;
	float		fSizeEnd;
	float		fSizeVar;
	float		fSpinStart;
	float		fSpinEnd;
	float		fSpinVar;
	hgeColor	colColorStart; // + alpha
	hgeColor	colColorEnd;
	float		fColorVar;
	float		fAlphaVar;
};


class WeaponParticleSystem {

public:
	weaponParticleSystemInfo info;
	
	WeaponParticleSystem(const char *filename, hgeSprite *sprite, int type);

	void Render();
	void FireAt(float x, float y);
	void Fire();
	void Stop(bool bKillParticles=false);
	void Update(float fDeltaTime);
	void MoveTo(float x, float y, bool bMoveParticles=false);
	bool testCollision(hgeRect *collisionBox);

	static HGE			*hge;
	float				fAge;
	float				fEmissionResidue;
	hgeVector			vecPrevLocation;
	hgeVector			vecLocation;
	float				fTx, fTy;
	int					nParticlesAlive;
	hgeRect				rectBoundingBox;
	bool				bUpdateBoundingBox;
	int					type;					//fire or ice
	weaponParticle		particles[MAX_PARTICLES];
	hgeRect				*collisionBox;

};


#endif
