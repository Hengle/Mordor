/*
	Created		Elinor Townsend
*/

/*
	Name		Particle
	Brief		Enumerated type of Particle used by the particle system shader 
				to set the correct effect file
	Details		New particles should be added to the end of the list
*/

#ifndef _PARTICLE_H
#define _PARTICLE_H

enum Particle
{
        PARTICLE_FIRE,
		PARTICLE_SMOKE,
        PARTICLE_ASH,
};

#endif // _PARTICLE_H