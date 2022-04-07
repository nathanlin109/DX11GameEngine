#include "Material.h"
// Ctor
Material::Material(DirectX::XMFLOAT4 colorTint,
	std::shared_ptr<SimpleVertexShader> vertexShader,
	std::shared_ptr<SimplePixelShader> pixelShader)
{
	this->colorTint = colorTint;
	this->vertexShader = vertexShader;
	this->pixelShader = pixelShader;

}

// Getters and setters
DirectX::XMFLOAT4 Material::GetColorTint(){ return colorTint; }
void Material::SetColorTint(DirectX::XMFLOAT4 colorTint) { this->colorTint = colorTint; }

std::shared_ptr<SimpleVertexShader> Material::GetVertexShader() { return vertexShader; }
void Material::SetVertexShader(std::shared_ptr<SimpleVertexShader> vertexShader) { this->vertexShader = vertexShader; }

std::shared_ptr<SimplePixelShader> Material::GetPixelShader() { return pixelShader; }
void Material::SetPixelShader(std::shared_ptr<SimplePixelShader> pixelShader) { this->pixelShader = pixelShader; }

void Material::AddTextureSRV(std::string shaderName, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv)
{
	textureSRVs.insert({ shaderName, srv });
}

void Material::AddSampler(std::string samplerName, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler)
{
	samplers.insert({ samplerName, sampler });
}

void Material::PrepareMaterial(std::shared_ptr<SimplePixelShader> ps)
{
	for (auto& t : textureSRVs) { ps->SetShaderResourceView(t.first.c_str(), t.second); }
	for (auto& s : samplers) { ps->SetSamplerState(s.first.c_str(), s.second); }
}
