#pragma once

#include "Transform.h"
#include "Input.h"
#include <DirectXMath.h>

class Camera
{
public:
	Camera(float x, float y, float z,
		float aspectRatio, float moveSpeed,
		float speedUpMultiplier, float mouseLookSpeed);
	~Camera();

	// Update methods
	void Update(float dt);
	void UpdateViewMatrix();
	void UpdateProjectionMatrix(float aspectRatio);

	// Getter for view and projection matrices
	DirectX::XMFLOAT4X4 GetViewMatrix();
	DirectX::XMFLOAT4X4 GetProjectionMatrix();

	// Getter for the transform
	Transform* GetTransform();

	// Getter for current move speed
	float getCurrentMoveSpeed();

private:
	// Camera matrices
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;

	// Overall transformation info
	Transform transform;

	// Additional custom fields
	float moveSpeed;
	float currentMoveSpeed;
	float speedUpMultiplier;
	float mouseLookSpeed;
	bool moving;
};

