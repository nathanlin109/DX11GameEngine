#include "Sky.h"
#include "DDSTextureLoader.h"

using namespace DirectX;

Sky::Sky(std::shared_ptr<Mesh> mesh,
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerOptions,
	Microsoft::WRL::ComPtr<ID3D11Device> device,
	const wchar_t* szFileName,
	std::shared_ptr<SimpleVertexShader> vertexShader,
	std::shared_ptr<SimplePixelShader> pixelShader)
{
	// Sets member vars
	this->mesh = mesh;
	this->samplerOptions = samplerOptions;
	this->vertexShader = vertexShader;
	this->pixelShader = pixelShader;

	// Creates texture from DDS
	CreateDDSTextureFromFile(device.Get(), szFileName, nullptr, &this->cubeMapSRV);

	// Creates rasterizer render states
	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_FRONT;

	device->CreateRasterizerState(&rasterizerDesc, &rasterizerState);

	// Creates depth stencil render state
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	device->CreateDepthStencilState(&depthStencilDesc, &depthStencilState);
}

void Sky::Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, std::shared_ptr<Camera> camera)
{
	// Sets render states
	context->RSSetState(rasterizerState.Get());
	context->OMSetDepthStencilState(depthStencilState.Get(), 0);

	// Sets the appropriate shaders
	vertexShader->SetShader();
	pixelShader->SetShader();

	// Creates a struct to represent the data to put in the vertex constant buffer
	vertexShader->SetMatrix4x4("viewMatrix", camera->GetViewMatrix());
	vertexShader->SetMatrix4x4("projectionMatrix", camera->GetProjectionMatrix());

	// Creates a struct to represent the data to put in the pixel constant buffer
	pixelShader->SetShaderResourceView("CubeMap", cubeMapSRV);
	pixelShader->SetSamplerState("SamplerOptions", samplerOptions);

	// Copies the data to the constant buffer
	vertexShader->CopyAllBufferData();
	pixelShader->CopyAllBufferData();

	// Draws mesh
	mesh->Draw(context);
	
	// Resets render state
	context->RSSetState(nullptr);
	context->OMSetDepthStencilState(nullptr, 0);
}