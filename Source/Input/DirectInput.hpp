/*
	Created 	Elinor Townsend 2012
*/

/*	
	Name		Direct Input
	Brief		Definition of Direct Input class used to handle input from the keyboard and mouse
*/

#ifndef DIRECTINPUT_H
#define DIRECTINPUT_H

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h> 

class DirectInput 
{
public:
	DirectInput();

	virtual ~DirectInput();
	virtual void update();
	bool isKeyDown(int scanCode);
	bool isKeyPressed(int scanCode);
	float getMouseX() const { return mouseX_; };
	float getMouseY() const { return mouseY_; };
	float getMouseZ() const { return mouseZ_; };
	bool getMouseLeftDown() const { return mouseLeftDown_; };
	
private:

	bool getDevices();
	void shutDown();

	LPDIRECTINPUT8 diObject_;				// DirectInput main object
	LPDIRECTINPUTDEVICE8 keyboardDevice_;	// Keyboard device
	LPDIRECTINPUTDEVICE8 mouseDevice_;		// Mouse device
	DIMOUSESTATE2 mouseState_;				// contains state of mouse

	char keyBuffer_[256];					// Buffer that holds the key input state
	bool heldKeys_[256];					// Array to hold key down events
	bool pressedKeys_[256];					// Array to hold key pressed events

	float mouseX_;							// X position of the mouse
	float mouseY_;							// Y position of the mouse
	float mouseZ_;

	bool mouseLeftUp_;						// Mouse button up/down flags
	bool mouseLeftDown_;
	bool mouseRightUp_;
	bool mouseRightDown_;
	bool mouseLeftPressed_;					// Mouse buttom pressed flags
	bool mouseRightPressed_;

	const int KEYBOARD_BUFFER_SIZE;
	
};


#endif
