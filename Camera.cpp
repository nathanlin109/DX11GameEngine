#include "Camera.h"

using namespace DirectX;

Camera::Camera(float x, float y, float z,
	float aspectRatio, float moveSpeed,
	float speedUpMultiplier, float mouseLookSpeed)
{
	transform.SetPosition(x, y, z);
	UpdateViewMatrix();
	UpdateProjectionMatrix(aspectRatio);
	this->moveSpeed = moveSpeed;
	this->currentMoveSpeed = moveSpeed;
	this->speedUpMultiplier = speedUpMultiplier;
	this->mouseLookSpeed = mouseLookSpeed;
}

Camera::~Camera()
{

}

void Camera::Update(float dt)
{
	// Gets input singleton
	Input& input = Input::GetInstance();

	moving = false;

	// Handles camera movement from input
	// ----------------Keyboard Input----------------
	// Speeds up movement if shift is held
	if (input.KeyDown(VK_SHIFT))
	{
		currentMoveSpeed += speedUpMultiplier * dt;

		// Clamps move speed
		currentMoveSpeed = min(max(currentMoveSpeed, 0), 15); // Clamp between 0 and 15
	}
	else 
	{
		currentMoveSpeed = moveSpeed;
	}

	// Forward
	if (input.KeyDown('W')) 
	{
		transform.MoveRelative(0, 0, currentMoveSpeed * dt);
		moving = true;
	}
	// Backwards
	if (input.KeyDown('S'))
	{
		transform.MoveRelative(0, 0, -currentMoveSpeed * dt);
		moving = true;
	}
	// Right
	if (input.KeyDown('D'))
	{
		transform.MoveRelative(currentMoveSpeed * dt, 0, 0);
		moving = true;
	}
	// Left
	if (input.KeyDown('A'))
	{
		transform.MoveRelative(-currentMoveSpeed * dt, 0, 0);
		moving = true;
	}
	// Up
	if (input.KeyDown(VK_SPACE))
	{
		transform.MoveRelative(0, currentMoveSpeed * dt, 0);
		moving = true;
	}
	// Down
	if (input.KeyDown('X'))
	{
		transform.MoveRelative(0, -currentMoveSpeed * dt, 0);
		moving = true;
	}

	if (moving == false)
	{
		currentMoveSpeed = 0;
	}

	// ----------------Mouse Input----------------
	// Camera rotation
	if (input.MouseLeftDown())
	{
		// Rotates around the y axis
		float cursorMovementX = input.GetMouseXDelta() * mouseLookSpeed * dt;
		transform.Rotate(0, cursorMovementX, 0);

		// Rotates around the x axis
		float cursorMovementY = input.GetMouseYDelta() * mouseLookSpeed * dt;
		transform.Rotate(cursorMovementY, 0, 0);

		// Clamps the x axis rotation, so the camera doesn't flip over
		if (transform.GetPitchYawRoll().x > XM_PIDIV2 - .01)
		{
			transform.SetRotation(XM_PIDIV2 - .01,
				transform.GetPitchYawRoll().y,
				transform.GetPitchYawRoll().z);
		}
		else if (transform.GetPitchYawRoll().x < -XM_PIDIV2 + .01)
		{
			transform.SetRotation(-XM_PIDIV2 + .01,
				transform.GetPitchYawRoll().y,
				transform.GetPitchYawRoll().z);
		}
	}

	// Updates the view matrix
	UpdateViewMatrix();
}

void Camera::UpdateViewMatrix()
{
	// I need the position, direction and "world up" vectors
	XMFLOAT3 pos = transform.GetPosition();
	XMFLOAT3 up(0, 1, 0);
	XMFLOAT3 rot = transform.GetPitchYawRoll();

	// This is the "math" type
	XMVECTOR forward = XMVector3Rotate(
	XMVectorSet(0, 0, 1, 0), // The world "forward" is (0, 0, 1)
	XMQuaternionRotationRollPitchYaw(rot.x, rot.y, rot.z));

	// Build the view matrix from our position, our local foward vector
	// and the world's up axis
	XMMATRIX view = XMMatrixLookToLH(
		XMLoadFloat3(&pos),
		forward,
		XMLoadFloat3(&up));
	XMStoreFloat4x4(&viewMatrix, view);
}

void Camera::UpdateProjectionMatrix(float aspectRatio)
{
	XMMATRIX proj = XMMatrixPerspectiveFovLH(
		XM_PIDIV4,		// 45 degree field of view
		aspectRatio,	// Aspect ratio of window (hopefully)
		0.01f,			// Near plane (close to 0 but NOT 0)
		100.0f);		// Far plane (ideally not more than 1000 or so)
	XMStoreFloat4x4(&projectionMatrix, proj);
}

Transform* Camera::GetTransform(){ return &transform; }

float Camera::getCurrentMoveSpeed(){ return currentMoveSpeed; }

XMFLOAT4X4 Camera::GetViewMatrix(){ return viewMatrix; }

XMFLOAT4X4 Camera::GetProjectionMatrix(){ return projectionMatrix; }
