/*
	Created 	Elinor Townsend 2012
*/

/*	
	Name		Direct Input
	Brief		Definition of Direct Input class used to handle input from the keyboard and mouse
*/

#include <exception>
#include <cassert>
#include "Input/DirectInput.hpp"

/*
	Name		DirectInput::DirectInput
	Syntax		DirectInput()
	Brief		DirectInput constructor gets the input devices and initialises input variables
*/
DirectInput::DirectInput()
: diObject_(0), keyboardDevice_(0), mouseDevice_(0), mouseX_(0), mouseY_(0), mouseZ_(0),
  mouseLeftUp_(true), mouseLeftDown_(false), mouseRightUp_(true), mouseRightDown_(false),
  mouseLeftPressed_(false), mouseRightPressed_(false), KEYBOARD_BUFFER_SIZE(16)
{ 
    if (!getDevices())
    {
        throw new std::exception("getDevices failed.");
    }

    // Initialise the key events arrays
    for (int i = 0; i < 256; ++i)
    {
        heldKeys_[i] = false;
    }

	for (int i = 0; i < 256; ++i)
    {
        pressedKeys_[i] = false;
    }

	// Initialise the mouse button flags
	mouseLeftUp_ = true;
	mouseLeftDown_ = false;
	mouseRightUp_ = true;
	mouseRightDown_ = false;
	mouseLeftPressed_ = false;
	mouseRightPressed_ = false;
}

/*
	Name		DirectInput::~DirectInput
	Syntax		~DirectInput()
	Brief		DirectInput destructor calls shutDown
*/   
DirectInput::~DirectInput()
{	
    shutDown();
}

/*
	Name		DirectInput::update
	Syntax		directInputObj.update()
	Brief		Reads the active input devices and maps into the key events arrays and mouse button flags
*/
void DirectInput::update()
{	
    // Obtain the current keyboard state and pack it into the keyBuffer_ 
    bool deviceState = SUCCEEDED(keyboardDevice_->GetDeviceState(sizeof(keyBuffer_),
                                keyBuffer_));
    assert(deviceState);

    // Set key states
	for (int i = 0; i < 256; ++i)
	{
		pressedKeys_[i] = false;

		// If the key is pressed
		if (keyBuffer_[i] & 0x80)
		{
			// If the key has just been pressed
			if (!heldKeys_[i])
			{
				pressedKeys_[i] = true;
			}
			heldKeys_[i] = true;
		} 
		else 
		{
			heldKeys_[i] = false;
		}
	}

	bool MouseDeviceState = SUCCEEDED(mouseDevice_->GetDeviceState(sizeof(mouseState_),
                                &mouseState_));
    assert(MouseDeviceState);

	if (mouseState_.rgbButtons[0] & 0x80)
	{
		// Pressed
		mouseLeftDown_ = true;
		mouseLeftUp_ = false;
	}
	else
	{
		/// Released if down before
		if (mouseLeftDown_)
		{
			mouseLeftUp_ = true;
			mouseLeftDown_ = false;
		}
	}

	if (mouseState_.rgbButtons[1] & 0x80)
	{
		// Pressed
		mouseRightDown_ = true;
		mouseRightUp_ = false;
	}
	else
	{
		/// Released if down before
		if (mouseRightDown_)
		{
			mouseRightUp_ = true;
			mouseRightDown_ = false;
		}
	}
	mouseX_ = static_cast<float>(mouseState_.lX);
	mouseY_ = static_cast<float>(mouseState_.lY);
	mouseZ_ = static_cast<float>(mouseState_.lZ);
}

/*
	Name		DirectInput::getDevices
	Syntax		directInputObj.getDevices()
	Return		Bool - Returns false is fails to get input device
	Brief		Creates the keyboard and mouse input devices
*/
bool DirectInput::getDevices()
{	
    shutDown();

    if (FAILED(DirectInput8Create(GetModuleHandle(0), 0x0800, IID_IDirectInput8, 
								  (void**)&diObject_, 0)))
    {
        return false;
    }

	////////////////////////
	// Keyboard
	////////////////////////

    // Create keyboard device
    if (FAILED(diObject_->CreateDevice(GUID_SysKeyboard, &keyboardDevice_, 0)))
    {
        return false;
    }
   
    // Set the Data format (c_dfDIKeyboard is standard Dirx Global)
    if (FAILED(keyboardDevice_->SetDataFormat(&c_dfDIKeyboard)))
    {
        shutDown();
        return false;
    }
   
    // How the app handles control of keyboard when switching window etc. 
    if (FAILED(keyboardDevice_->SetCooperativeLevel(0, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE)))
    {	
        shutDown();
        return false;
    }

	// The header
	DIPROPDWORD dipdw;
    dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
    dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    dipdw.diph.dwObj        = 0;
    dipdw.diph.dwHow        = DIPH_DEVICE;
    // The data
    dipdw.dwData            = KEYBOARD_BUFFER_SIZE;

    // Aquiring the keyboard now everything is set up
    if (FAILED(keyboardDevice_->Acquire()))
    {	
        shutDown();
        return false;
    }
	
	////////////////////////
	// Mouse
	////////////////////////

	// Create mouse device
	if (FAILED(diObject_->CreateDevice(GUID_SysMouse, &mouseDevice_, 0)))
    {
        return false;
    }

	if (FAILED(mouseDevice_->SetCooperativeLevel(0, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE)))
    {	
        shutDown();
        return false;
    }

	// c_dfDIMouse is a standard Dirx Global
	if (FAILED(mouseDevice_->SetDataFormat(&c_dfDIMouse2)))
    {
        shutDown();
        return false;
    }

	// Aquiring the mouse now everything is set up
    if (FAILED(mouseDevice_->Acquire()))
    {	
        shutDown();
        return false;
    }

	return true;
}

/*
	Name		DirectInput::shutDown
	Syntax		directInputObj.shutDown()
	Brief		Releases the input devices
*/
void DirectInput::shutDown()
{
    if (diObject_ != 0)
    {
        diObject_->Release(); 
        diObject_ = 0;
    }

    if (keyboardDevice_ != 0)
    {
        keyboardDevice_->Unacquire();	
        keyboardDevice_->Release();   
        keyboardDevice_ = 0;		
    }

	if (mouseDevice_ != 0)
    {
        mouseDevice_->Unacquire();	
        mouseDevice_->Release();   
        mouseDevice_ = 0;		
    }
}

/*
	Name		DirectInput::isKeyDown
	Syntax		directInputObj.isKeyDown(int scanCode)
	Param		scanCode - The scan code for the key to be checked
	Return		bool - True if key is held down
	Brief		Checks to find if key is held down
*/
bool DirectInput::isKeyDown(int scanCode)
{
	return heldKeys_[scanCode];
}

/*
	Name		DirectInput::isKeyPressed
	Syntax		directInputObj.isKeyPressed(int scanCode)
	Param		scanCode - The scan code for the key to be checked
	Return		bool - True if key has just been pressed
	Brief		Checks to find if key has just been pressed
*/
bool DirectInput::isKeyPressed(int scanCode)
{
	return pressedKeys_[scanCode];
}
