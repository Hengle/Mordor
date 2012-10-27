/*
	Created 	Elinor Townsend 2012
*/

/*	
	Name		State
	Brief		Definition of abstract base state class
*/

#ifndef STATE_H
#define STATE_H

#include "Camera\Camera.hpp"
#include "Input\DirectInput.hpp"
#include <d3dx10.h>

class State
{
public:
	// Pure virtual function
    virtual State * getNextState() = 0;
	virtual bool initialise() = 0;
	virtual bool deinitialise() = 0;
	virtual bool update(float dt) = 0;
	virtual void render() = 0;

protected:
	Camera* cameraOne_;
	Camera* cameraTwo_;
	DirectInput * input_;
};

#endif