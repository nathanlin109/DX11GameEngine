#pragma once
#include "Transform.h"
#include "Mesh.h"
#include "Camera.h"
#include "Material.h"
#include "Lights.h"
#include <memory>

class Entity
{
private:
	Transform transform;
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> material;
public:
	// Ctor
	Entity(Transform transform, std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material);

	void Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
		std::shared_ptr<Camera> camera, float totalTime,
		DirectX::XMFLOAT3 ambientColor, std::vector<Light> lights);

	// Getters and setters
	std::shared_ptr<Mesh> GetMesh();

	Transform* GetTransform();

	std::shared_ptr<Material> GetMaterial();
	void SetMaterial(std::shared_ptr<Material> material);
};

