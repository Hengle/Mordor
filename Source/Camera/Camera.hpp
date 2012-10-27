/*
	Created 	Elinor Townsend 2012
*/

/*	
	Name		Camera
	Brief		Definition of Camera class 
*/

#ifndef CAMERA_H
#define CAMERA_H

#include <d3dx10.h>

enum CameraType 
{
	CAMERA_AUTO,
	CAMERA_FREE,	
};

class Camera
{
public:
	static Camera* getCamera(CameraType type);

	virtual void initialise() = 0;
	virtual void rotate(float yaw, float pitch, float roll) = 0;
	virtual void move(float moveX, float moveY, float moveZ) = 0;
	virtual void update() = 0;
	virtual void reset() = 0;
	void setDeltaTime(float dt) { deltaTime_ = dt; };
	D3DXVECTOR3 getPosition() const { return position_; };
	D3DXVECTOR3 getRotation() const { return D3DXVECTOR3(pitch_, yaw_, roll_); };
	virtual void zoom(float direction) = 0;
	virtual void setZoomFactor(float zoom) = 0;

protected:
	Camera();
	D3DXVECTOR3 position_;
	float yaw_, pitch_, roll_;
	float zoomFactor_;
	float deltaTime_;
};

#endif