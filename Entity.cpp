#include "Entity.h"

Entity::Entity(Transform transform, std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material)
{
	this->transform = transform;
	this->mesh = mesh;
	this->material = material;
}

void Entity::Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
	std::shared_ptr<Camera> camera, float totalTime,
	DirectX::XMFLOAT3 ambientColor, std::vector<Light> lights)
{
	// Sets the appropriate shaders
	material->GetVertexShader()->SetShader();
	material->GetPixelShader()->SetShader();

	// Creates a struct to represent the data to put in the vertex constant buffer
	std::shared_ptr<SimpleVertexShader> vs = material->GetVertexShader();
	vs->SetMatrix4x4("worldMatrix", transform.GetWorldMatrix());
	vs->SetMatrix4x4("worldInvMatrix", transform.GetworldInverseTransposeMatrix());
	vs->SetMatrix4x4("viewMatrix", camera->GetViewMatrix());
	vs->SetMatrix4x4("projectionMatrix", camera->GetProjectionMatrix()); 

	// Creates a struct to represent the data to put in the pixel constant buffer
	std::shared_ptr<SimplePixelShader> ps = material->GetPixelShader();
	material->PrepareMaterial(ps);
	ps->SetFloat4("colorTint", material->GetColorTint());
	ps->SetFloat("totalTime", totalTime);
	ps->SetFloat("roughness", material->GetRoughness());
	ps->SetFloat3("cameraPos", camera->GetTransform()->GetPosition());
	ps->SetFloat3("ambient", ambientColor);
	ps->SetData("lights", &lights[0], sizeof(Light) * (int)lights.size());

	// Copies the data to the constant buffer
	vs->CopyAllBufferData();
	ps->CopyAllBufferData();

	// Draws mesh
	mesh->Draw(context);
}

std::shared_ptr<Mesh> Entity::GetMesh() { return mesh; }

Transform* Entity::GetTransform(){ return &transform; }

std::shared_ptr<Material> Entity::GetMaterial(){ return material; }

void Entity::SetMaterial(std::shared_ptr<Material> material) { this->material = material; }
