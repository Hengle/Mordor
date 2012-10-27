/*
	Created 	Elinor Townsend 2012
*/

/*	
	Name		Camera
	Brief		Definition of Camera class
*/

#include "Camera\Camera.hpp"
#include "Camera\CameraFree.hpp"
#include "Camera\CameraAuto.hpp"
#include "Scene\Scene.hpp"

/*
	Name		Camera::getCamera
	Syntax		Camera::getCamera(CameraType type)
	Param		CameraType type - The type of camera to create
	Return		Camera* - Returns to a pointer to a camera object
	Brief		Creates a camera of the type specified and returns a 
				pointer to it
*/
Camera* Camera::getCamera(CameraType type)
{

	switch (type)
	{
	case CAMERA_AUTO:
		return new CameraAuto;
		break;
	case CAMERA_FREE:
		return new CameraFree;
		break;
	default:
		return new CameraFree;
		break;
	}

}

/*
	Name		Camera::Camera
	Syntax		Camera()
	Brief		Camera constructor initialises member variables
*/
Camera::Camera()
: deltaTime_(0)
{

}