/*
	Created 	Elinor Townsend 2012
*/

/*	
	Name		CameraFree
	Brief		Definition of CameraFree class
*/

#include "Camera\CameraFree.hpp"
#include "Scene\Scene.hpp"
#include "Global\Global.hpp"

/*
	Name		CameraFree::CameraFree
	Syntax		CameraFree()
	Brief		CameraFree constructor initialises member variables
*/
CameraFree::CameraFree()
	: RADIANS_CONVERSION(0.0174532925f)
{
	position_	= D3DXVECTOR3(0, 0, 0);
	yaw_ = pitch_ = roll_ = 0;
	zoomFactor_	= 1;
}

/*
	Name		CameraFree::~CameraFree
	Syntax		~CameraFree()
	Brief		CameraFree destructor
*/
CameraFree::~CameraFree()
{
	// Nothing to clean up
}

/*
	Name		CameraFree::initialise
	Syntax		CameraFree::initialise()
	Brief		Initialises the camera
*/
void CameraFree::initialise()
{
	update();
}

/*
	Name		CameraFree::rotate
	Syntax		CameraFree::rotate(float yaw, float pitch, float roll)
	Param		float yaw - Change in rotation around the y-axis
	Param		float pitch - Change in rotation around the x-axis
	Param		float roll - Change in rotation around the z-axis
	Brief		Updates the camera rotation
*/
void CameraFree::rotate(float yaw, float pitch, float roll)
{
	yaw_ += yaw  * RADIANS_CONVERSION;
	if (yaw_ >= 2 * D3DX_PI)
	{
		yaw_ -= 2 * D3DX_PI;
	}
	else if (yaw_ < 0)
	{
		yaw_ += 2 * D3DX_PI;
	}

	pitch_ += pitch * RADIANS_CONVERSION;
	if (pitch_ >= 2 * D3DX_PI)
	{
		pitch_ -= 2 * D3DX_PI;
	}
	else if (pitch_ < 0)
	{
		pitch_ += 2 * D3DX_PI;
	}

	roll_ += roll * RADIANS_CONVERSION;
	if (roll_ >= 2 * D3DX_PI)
	{
		roll_ -= 2 * D3DX_PI;
	}
	else if (roll_ < 0)
	{
		roll_ += 2 * D3DX_PI;
	}
}

/*
	Name		CameraFree::move
	Syntax		CameraFree::move(float moveX, float moveY, float moveZ)
	Param		float moveX - Distance to move the camera along its x-axis
	Param		float moveY - Distance to move the camera along its y-axis
	Param		float moveZ - Distance to move the camera along its z-axis
	Brief		Updates the camera position
*/
void CameraFree::move(float moveX, float moveY, float moveZ)
{
	position_.x += moveX;
	position_.y += moveY;
	position_.z += moveZ;
}

/*
	Name		CameraFree::update
	Syntax		CameraFree::update()
	Brief		Updates the camera
*/
void CameraFree::update()
{
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
	Name		CameraFree::setCameraProjectionMatrix
	Syntax		CameraFree::setCameraProjectionMatrix()
	Brief		Sets the camera projection matrix
*/
void CameraFree::setCameraProjectionMatrix()
{
	// Create the projection matrix
	D3DXMATRIX projection;
	D3DXMatrixPerspectiveFovLH(&projection, (float)D3DX_PI * 0.25f * zoomFactor_, Scene::instance()->getAspect(), SCREEN_NEAR, SCREEN_DEPTH);

	// Set the projection matrix
	Scene::instance()->setProjection(projection);
}

/*
	Name		CameraFree::reset
	Syntax		CameraFree::reset()
	Brief		Resets the camera's position
*/
void CameraFree::reset()
{
	position_.x = 0.0f;
	position_.y = 0.0f;
	position_.z = 0.0f;
	yaw_ = 0.0f;
	pitch_ = 0.0f;
	roll_ = 0.0f;
	zoomFactor_	= 1;
}

/*
	Name		CameraFree::zoom
	Syntax		CameraFree::zoom(float direction)
	Brief		Adjusts the camera's zoom factor
*/
void CameraFree::zoom(float direction)
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
	Name		CameraFree::setZoomFactor
	Syntax		CameraFree::setZoomFactor(float zoom)
	Brief		Sets the camera's zoom factor
*/
void CameraFree::setZoomFactor(float zoom)
{
	zoomFactor_ = zoom;
	setCameraProjectionMatrix();
}