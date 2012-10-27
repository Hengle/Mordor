/*
	Created 	Elinor Townsend 2012
*/

/*	
	Name		CameraAuto
	Brief		Definition of CameraAuto class
*/

#include "Camera\CameraAuto.hpp"
#include "Scene\Scene.hpp"
#include "Global\Global.hpp"

/*
	Name		CameraAuto::CameraAuto
	Syntax		CameraAuto()
	Brief		CameraAuto constructor initialises member variables
*/
CameraAuto::CameraAuto()
	: orbitalAngle_(0), CENTRE_OF_ORBIT(0.0f, -50.0f, 275.0f), ORBITAL_RADIUS(375.0f),
	  RADIANS_CONVERSION(0.0174532925f)
{
	position_	= D3DXVECTOR3(0.0f, 150.0f, ORBITAL_RADIUS);
	yaw_ = -90.0f * RADIANS_CONVERSION;
	roll_ = 0;
	pitch_ = 25.0f * RADIANS_CONVERSION;
	zoomFactor_	= 1;
}

/*
	Name		CameraAuto::~CameraAuto
	Syntax		~CameraAuto()
	Brief		CameraAuto destructor
*/
CameraAuto::~CameraAuto()
{
	// Nothing to clean up
}

/*
	Name		CameraAuto::initialise
	Syntax		CameraAuto::initialise()
	Brief		Initialises the camera
*/
void CameraAuto::initialise()
{
	update();
}

/*
	Name		CameraAuto::update
	Syntax		CameraAuto::update()
	Brief		Updates the camera
*/
void CameraAuto::update()
{
	// Update position and rotation of camera.
	position_.x =  CENTRE_OF_ORBIT.x + ORBITAL_RADIUS * cos(orbitalAngle_);
	position_.z = CENTRE_OF_ORBIT.z + ORBITAL_RADIUS * sin(orbitalAngle_);

	yaw_ += 5.0f * deltaTime_ * RADIANS_CONVERSION;
	if (yaw_ >= 2 * D3DX_PI)
	{
		yaw_ -= 2 * D3DX_PI;
	}

	orbitalAngle_ -= 5.0f * deltaTime_ * RADIANS_CONVERSION;
	if (orbitalAngle_ < 0)
	{
		orbitalAngle_ += 2 * D3DX_PI;
	}

	// Create the rotation matrix.
	D3DXMATRIX rotation;
	D3DXMatrixRotationYawPitchRoll(&rotation, yaw_, pitch_, roll_);

	// Transform the default target and up vectors by the rotation matrix.
	D3DXVECTOR3 target = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
	D3DXVECTOR3 up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	D3DXVec3TransformCoord(&target, &target, &rotation);
	D3DXVec3TransformCoord(&up, &up, &rotation);

	// Translate to the camera's position.
	target += position_;

	// Create the view matrix.
	D3DXMATRIX view;
	D3DXMatrixLookAtLH(&view, &position_, &target, &up);

	// Set the view matrix.
	Scene::instance()->setView(view);
}

/*
	Name		CameraAuto::setCameraProjectionMatrix
	Syntax		CameraAuto::setCameraProjectionMatrix()
	Brief		Sets the camera projection matrix
*/
void CameraAuto::setCameraProjectionMatrix()
{
	// Create the projection matrix
	D3DXMATRIX projection;
	D3DXMatrixPerspectiveFovLH(&projection, (float)D3DX_PI * 0.25f * zoomFactor_, Scene::instance()->getAspect(), SCREEN_NEAR, SCREEN_DEPTH);

	// Set the projection matrix
	Scene::instance()->setProjection(projection);
}

/*
	Name		CameraAuto::reset
	Syntax		CameraAuto::reset()
	Brief		Resets the camera's position
*/
void CameraAuto::reset()
{
	position_	= D3DXVECTOR3(0.0f, 150.0f, ORBITAL_RADIUS);
	yaw_ = -90.0f * RADIANS_CONVERSION;
	roll_ = 0;
	pitch_ = 25.0f * RADIANS_CONVERSION;
	orbitalAngle_ = 0.0f;
}

/*
	Name		CameraAuto::zoom
	Syntax		CameraAuto::zoom(float direction)
	Brief		Adjusts the camera's zoom factor
*/
void CameraAuto::zoom(float direction)
{
	if((zoomFactor_ < 2.0) && (direction < 0))
	{
		zoomFactor_ += 0.1;
	}
	if((zoomFactor_ > 0.1) && (direction > 0))
	{
		zoomFactor_ -= 0.1;
	}

	setCameraProjectionMatrix();
}

/*
	Name		CameraAuto::setZoomFactor
	Syntax		CameraAuto::setZoomFactor(float zoom)
	Brief		Sets the camera's zoom factor
*/
void CameraAuto::setZoomFactor(float zoom)
{
	zoomFactor_ = zoom;
	setCameraProjectionMatrix();
}

/*
	Name		CameraAuto::rotate
	Syntax		CameraAuto::rotate(float yaw, float pitch, float roll)
	Param		float yaw - Change in rotation around the y-axis
	Param		float pitch - Change in rotation around the x-axis
	Param		float roll - Change in rotation around the z-axis
	Brief		Updates the camera rotation
*/
void CameraAuto::rotate(float yaw, float pitch, float roll)
{
	// Not used in this class
}

/*
	Name		CameraAuto::move
	Syntax		CameraAuto::move(float moveX, float moveY, float moveZ)
	Param		float moveX - Distance to move the camera along its x-axis
	Param		float moveY - Distance to move the camera along its y-axis
	Param		float moveZ - Distance to move the camera along its z-axis
	Brief		Updates the camera position
*/
void CameraAuto::move(float moveX, float moveY, float moveZ)
{
	// Not used in this class
}