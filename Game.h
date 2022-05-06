#pragma once

#include "DXCore.h"
#include "Mesh.h"
#include "Transform.h"
#include "Entity.h"
#include "camera.h"
#include "SimpleShader.h"
#include "Material.h"
#include "Lights.h"
#include "Sky.h"

#include <DirectXMath.h>
#include <memory>
#include <vector>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects

class Game
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);

private:

	// Should we use vsync to limit the frame rate?
	bool vsync;

	// Initialization helper methods - feel free to customize, combine, etc.
	void LoadShaders();
	void LoadTextures(std::wstring fileName);
	void LoadAssetsAndCreateEntities();
	void GenerateLights();

	// Camera
	std::shared_ptr<Camera> camera;

	// A vector to hold any number of meshes
	// This makes things easy to draw and clean up, too!
	std::vector<std::shared_ptr<Mesh>> meshes;
	std::vector<std::shared_ptr<Entity>> entities;
	std::vector<std::shared_ptr<Material>> materials;

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//    Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Shaders and shader-related constructs
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimplePixelShader> pixelShader;
	std::shared_ptr<SimplePixelShader> customPixelShader;
	std::shared_ptr<SimpleVertexShader> skyVertexShader;
	std::shared_ptr<SimplePixelShader> skyPixelShader;

	// SVPtrs
	std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> albedoSVPtrs;
	std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> metallicSVPtrs;
	std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> normalSVPtrs;
	std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> roughnessSVPtrs;

	// Lighting
	DirectX::XMFLOAT3 ambientColor;
	std::vector<Light> lights;

	// Sky box
	std::shared_ptr<Sky> skyBox;

	// Post processing resources
	// ----------------BLUR--------------
	int blurAmount;
	int additionalBlurAmount;
	float blurMultiplier;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> ppRTV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ppSRV;
	std::shared_ptr<SimpleVertexShader> ppVS;
	std::shared_ptr<SimplePixelShader> fullScreenBlurPS;
	void fullScreenBlur();

	// --------------BLOOM---------------
	static const int MaxBloomLevels = 5;

	bool drawBloomTextures;
	int bloomLevels;
	float bloomThreshold;
	float bloomLevelIntensities[MaxBloomLevels];

	Microsoft::WRL::ComPtr<ID3D11SamplerState> ppSampler; // Clamp sampler for post processing

	// Render targets and shader resources
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> bloomExtractRTV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bloomExtractSRV;
	
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> bloomCombineRTV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bloomCombineSRV;

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> blurHorizontalRTV[MaxBloomLevels];
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> blurHorizontalSRV[MaxBloomLevels];

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> blurVerticalRTV[MaxBloomLevels];
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> blurVerticalSRV[MaxBloomLevels];

	// Pixel shaders
	std::shared_ptr<SimplePixelShader> bloomExtractPS;
	std::shared_ptr<SimplePixelShader> gaussianBlurPS;
	std::shared_ptr<SimplePixelShader> bloomCombinePS;

	// Bloom helpers
	void BloomExtract();
	void SingleDirectionBlur(
		float renderTargetScale,
		DirectX::XMFLOAT2 blurDirection,
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> target,
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> sourceTexture);
	void BloomCombine();

	// Post processing resizing
	void ResizeAllPostProcessResources();
	void ResizeOnePostProcessResource(
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>& rtv,
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& srv,
		float renderTargetScale,
		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);
};