#include "Game.h"
#include "Vertex.h"
#include "Input.h"
#include "WICTextureLoader.h"

// Needed for a helper function to read compiled shader files from the hard drive
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// DirectX itself, and our window, are not ready yet!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,		   // The application's handle
		"DirectX Game",	   // Text for the window's title bar
		1280,			   // Width of the window's client area
		720,			   // Height of the window's client area
		true),			   // Show extra stats (fps) in title bar?
	vsync(false),
	blurAmount(0),
	blurMultiplier(.6f),
	additionalBlurAmount(0),
	ambientColor(0.1f, 0.1f, 0.15f), 
	bloomLevels(5),
	bloomThreshold(1.0f),
	bloomLevelIntensities{ 1,1,1,1,1 },
	drawBloomTextures(true)
{
#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.\n");
#endif

	// Creates camera
	camera = std::make_shared<Camera>(12, 0, -25,
		(float)width / height, 3, 5, 4);
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Release all DirectX objects created here
//  - Delete any objects to prevent memory leaks
// --------------------------------------------------------
Game::~Game()
{
	// Note: Since we're using smart pointers (ComPtr),
	// we don't need to explicitly clean up those DirectX objects
	// - If we weren't using smart pointers, we'd need
	//   to call Release() on each DirectX object created in Game

}

// --------------------------------------------------------
// Called once per program, after DirectX and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();

	LoadAssetsAndCreateEntities();

	GenerateLights();

	// Create post process resources
	ResizeAllPostProcessResources();

	// Sampler state for post processing
	D3D11_SAMPLER_DESC ppSampDesc = {};
	ppSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	ppSampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&ppSampDesc, ppSampler.GetAddressOf());

	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	// Simple shader code
	vertexShader = std::make_shared<SimpleVertexShader>(device, context,
		GetFullPathTo_Wide(L"VertexShader.cso").c_str());

	pixelShader = std::make_shared<SimplePixelShader>(device, context,
		GetFullPathTo_Wide(L"PixelShader.cso").c_str());

	// Sky box shaders
	skyVertexShader = std::make_shared<SimpleVertexShader>(device, context,
		GetFullPathTo_Wide(L"SkyVertexShader.cso").c_str());

	skyPixelShader = std::make_shared<SimplePixelShader>(device, context,
		GetFullPathTo_Wide(L"SkyPixelShader.cso").c_str());

	// Post process shaders
	// Blur
	ppVS = std::make_shared<SimpleVertexShader>(device, context,
		GetFullPathTo_Wide(L"PostProcessVS.cso").c_str());;

	fullScreenBlurPS = std::make_shared<SimplePixelShader>(device, context,
		GetFullPathTo_Wide(L"FullScreenBlurPS.cso").c_str());

	// Bloom
	bloomExtractPS = std::make_shared<SimplePixelShader>(device, context,
		GetFullPathTo_Wide(L"BloomExtractPS.cso").c_str());

	gaussianBlurPS = std::make_shared<SimplePixelShader>(device, context,
		GetFullPathTo_Wide(L"GaussianBlurPS.cso").c_str());

	bloomCombinePS = std::make_shared<SimplePixelShader>(device, context,
		GetFullPathTo_Wide(L"BloomCombinePS.cso").c_str());

	/*customPixelShader = std::make_shared<SimplePixelShader>(device, context,
		GetFullPathTo_Wide(L"CustomPS.cso").c_str());*/
}

// Loads textures
void Game::LoadTextures(std::wstring fileName)
{
	// Albedo
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> albedoSRVPtr;
	CreateWICTextureFromFile(device.Get(), context.Get(),
		GetFullPathTo_Wide(L"../../Assets/Textures/" + fileName + L"/" + fileName + L"_albedo.png").c_str(),
		nullptr, &albedoSRVPtr);
	// Metallic
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> metallicSRVPtr;
	CreateWICTextureFromFile(device.Get(), context.Get(),
		GetFullPathTo_Wide(L"../../Assets/Textures/" + fileName + L"/" + fileName + L"_metal.png").c_str(),
		nullptr, &metallicSRVPtr);
	// Normal
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> normalsSRVPtr;
	CreateWICTextureFromFile(device.Get(), context.Get(),
		GetFullPathTo_Wide(L"../../Assets/Textures/" + fileName + L"/" + fileName + L"_normals.png").c_str(),
		nullptr, &normalsSRVPtr);
	// Roughness
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> roughnessSRVPtr;
	CreateWICTextureFromFile(device.Get(), context.Get(),
		GetFullPathTo_Wide(L"../../Assets/Textures/" + fileName + L"/" + fileName + L"_roughness.png").c_str(),
		nullptr, &roughnessSRVPtr);
	albedoSVPtrs.push_back(albedoSRVPtr);
	metallicSVPtrs.push_back(metallicSRVPtr);
	normalSVPtrs.push_back(normalsSRVPtr);
	roughnessSVPtrs.push_back(roughnessSRVPtr);
}


// --------------------------------------------------------
// Loads all necessary assets and creates various entities
// --------------------------------------------------------
void Game::LoadAssetsAndCreateEntities()
{
	// Loads textures
	// Bronze
	LoadTextures(L"cobblestone");

	// Wood
	LoadTextures(L"wood");

	// Cobblestone
	LoadTextures(L"bronze");

	// Floor
	LoadTextures(L"floor");

	// Paint
	LoadTextures(L"rough");

	// Scratched metal
	LoadTextures(L"scratched");

	// Rough
	LoadTextures(L"paint");

	// Arcade room
	LoadTextures(L"arcade_room");

	// Counter
	LoadTextures(L"counter");

	// Skeeball
	LoadTextures(L"skeeball_1");
	LoadTextures(L"skeeball_2");
	LoadTextures(L"skeeball_3");

	// Arcade machine
	LoadTextures(L"arcade_machine_1");
	LoadTextures(L"arcade_machine_2");
	LoadTextures(L"arcade_machine_3");

	// DDR
	LoadTextures(L"ddr");

	// Ticket Machine
	LoadTextures(L"ticket_machine");

	// Creates sampler state
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&samplerDesc, &samplerState);

	// Creates some materials
	// Purple
	/*materials.push_back(std::make_shared<Material>(
		XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f), vertexShader, pixelShader));
	// Yellow
	materials.push_back(std::make_shared<Material>(
		XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), vertexShader, pixelShader));
	// Cyan
	materials.push_back(std::make_shared<Material>(
		XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), vertexShader, pixelShader));*/
		// White material
	for (int i = 0; i < albedoSVPtrs.size(); i++)
	{
		materials.push_back(std::make_shared<Material>(
			XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), vertexShader, pixelShader));
	}

	// Adds SRV's and samplers to materials
	for (int i = 0; i < materials.size(); i++)
	{
		materials[i]->AddTextureSRV("Albedo", albedoSVPtrs[i]);
		materials[i]->AddTextureSRV("MetallicMap", metallicSVPtrs[i]);
		materials[i]->AddTextureSRV("NormalMap", normalSVPtrs[i]);
		materials[i]->AddTextureSRV("RoughnessMap", roughnessSVPtrs[i]);
		materials[i]->AddSampler("BasicSampler", samplerState);
	}

	// Loads meshes and creates some geometry
	// Creates basic meshes
	meshes.push_back(std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/quad.obj").c_str(), device));
	meshes.push_back(std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/quad_double_sided.obj").c_str(), device));
	meshes.push_back(std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/torus.obj").c_str(), device));
	meshes.push_back(std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/sphere.obj").c_str(), device));
	meshes.push_back(std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/cylinder.obj").c_str(), device));
	meshes.push_back(std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/cube.obj").c_str(), device));
	meshes.push_back(std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/helix.obj").c_str(), device));
	// Creates extra meshes
	meshes.push_back(std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/arcade_room.obj").c_str(), device));
	meshes.push_back(std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/counter.obj").c_str(), device));
	meshes.push_back(std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/skeeball.obj").c_str(), device));
	meshes.push_back(std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/arcade_machine.obj").c_str(), device));
	meshes.push_back(std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/ddr.obj").c_str(), device));
	meshes.push_back(std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/ticket_machine.obj").c_str(), device));

	// Creates the entities from the meshes
	for (int i = 0; i < meshes.size(); i++)
	{
		// Adds entity to vector
		int matIndex = i;
		if (matIndex > materials.size() - 1)
		{
			matIndex = 0;
		}

		// Adds multiple entities for machines
		switch (i)
		{
		// 4 sleeball machines
		case 9:
			for (int x = 0; x < 4; x++)
			{
				int offset = x % 3;
				entities.push_back(std::make_shared<Entity>(Transform(), meshes[i], materials[(matIndex + offset)]));
			}
			break;
		// 12 arcade machines
		case 10:
			for (int x = 0; x < 12; x++)
			{
				int offset = x % 3;
				entities.push_back(std::make_shared<Entity>(Transform(), meshes[i], materials[(matIndex + 2 + offset)]));
			}
			break;
		// 1 DDR machine
		case 11:
			entities.push_back(std::make_shared<Entity>(Transform(), meshes[i], materials[(matIndex + 4)]));
			break;
		// 4 ticket machines
		case 12:
			for (int x = 0; x < 4; x++)
			{
				entities.push_back(std::make_shared<Entity>(Transform(), meshes[i], materials[(matIndex + 4)]));
			}			
			break;
		default:
			entities.push_back(std::make_shared<Entity>(Transform(), meshes[i], materials[matIndex]));
			break;
		}
	}

	// Sets positions for basic meshes
	for (int i = 0; i <= 6; i++)
	{
		entities[i]->GetTransform()->SetPosition((float)i * 4, -5, 0);
	}

	// Rotates quads
	entities[0]->GetTransform()->SetRotation(-45, -1, 45);
	entities[1]->GetTransform()->SetRotation(-45, -1, 45);

	// Rotates torus
	entities[2]->GetTransform()->SetRotation(0, 45, 45);

	// Sets appropriate transform for arcade room
	entities[7]->GetTransform()->SetRotation(XM_PIDIV2, 0, 0);
	entities[7]->GetTransform()->SetScale(.05f, .05f, .05f);
	entities[7]->GetTransform()->SetPosition(12, -5, 20);

	// Sets appropriate transform for counter
	entities[8]->GetTransform()->SetRotation(XM_PIDIV2, -XM_PIDIV2, 0);
	entities[8]->GetTransform()->SetScale(.01f, .01f, .01f);
	entities[8]->GetTransform()->SetPosition(20, -5, 10);

	// Sets appropriate transform for skeeball machines
	for (int i = 9; i < 11; i++)
	{
		entities[i]->GetTransform()->SetRotation(0, XM_PIDIV2, 0);
		entities[i]->GetTransform()->SetScale(.75f, .75f, .75f);
		entities[i]->GetTransform()->SetPosition(23.5f, -4.5f, 33.5 - (i - 9) * 6.5f);
	}
	for (int i = 11; i < 13; i++)
	{
		entities[i]->GetTransform()->SetRotation(0, -XM_PIDIV2, 0);
		entities[i]->GetTransform()->SetScale(.75f, .75f, .75f);
		entities[i]->GetTransform()->SetPosition(0.5f, -4.5f, 9.5f - (i - 11) * 3);
	}

	// Sets appropriate transform for arcade machines
	for (int i = 13; i < 15; i++)
	{
		entities[i]->GetTransform()->SetPosition(19 - (i - 13) * 3.5f, -3.2f, 34);
	}
	for (int i = 15; i < 18; i++)
	{
		entities[i]->GetTransform()->SetPosition(7 - (i - 15) * 3.5f, -3.2f, 34);
	}
	for (int i = 18; i < 20; i++)
	{
		entities[i]->GetTransform()->SetRotation(0, XM_PI, 0);
		entities[i]->GetTransform()->SetPosition(5 - (i - 18) * 3.5f, -3.2f, 26);
	}
	for (int i = 20; i < 22; i++)
	{
		entities[i]->GetTransform()->SetPosition(5 - (i - 20) * 3.5f, -3.2f, 14);
	}
	for (int i = 22; i < 24; i++)
	{
		entities[i]->GetTransform()->SetRotation(0, -XM_PIDIV2, 0);
		entities[i]->GetTransform()->SetPosition(-2, -3.2f, 31 - (i - 22) * 3.5f);
	}
	entities[24]->GetTransform()->SetRotation(0, -XM_PIDIV2, 0);
	entities[24]->GetTransform()->SetPosition(-2, -3.2f, 13);

	// Sets appropriate transform for DDR machine
	entities[25]->GetTransform()->SetScale(.4f, .4f, .4f);
	entities[25]->GetTransform()->SetPosition(11.5f, -4.65f, 31.5f);

	// Sets appropriate transform for ticket machines
	for (int i = 26; i < 28; i++)
	{
		entities[i]->GetTransform()->SetRotation(0, XM_PI, 0);
		entities[i]->GetTransform()->SetScale(.25f, .25f, .25f);
		entities[i]->GetTransform()->SetPosition(8.5f - (i - 26) * 2.5f, -3.1f, 5.5f);
	}
	for (int i = 28; i < 30; i++)
	{
		entities[i]->GetTransform()->SetRotation(0, XM_PI, 0);
		entities[i]->GetTransform()->SetScale(.25f, .25f, .25f);
		entities[i]->GetTransform()->SetPosition(18 - (i - 28) * 2.5f, -3.1f, 5.5f);
	}

	// Creates sky box
	skyBox = std::make_shared<Sky>(meshes[5], samplerState, device,
		GetFullPathTo_Wide(L"../../Assets/Textures/skies/SunnyCubeMap.dds").c_str(),
		skyVertexShader, skyPixelShader);
}


// ------------------------------------------------------------
// Resizes (by releasing and re-creating) the resources
// required for post processing.
// 
// We only need to do this at start-up and whenever the 
// window is resized.
// ------------------------------------------------------------
void Game::ResizeAllPostProcessResources()
{
	ResizeOnePostProcessResource(ppRTV, ppSRV, 1.0f, DXGI_FORMAT_R16G16B16A16_FLOAT);
	ResizeOnePostProcessResource(bloomExtractRTV, bloomExtractSRV, 0.5f, DXGI_FORMAT_R16G16B16A16_FLOAT);
	ResizeOnePostProcessResource(bloomCombineRTV, bloomCombineSRV, 1, DXGI_FORMAT_R16G16B16A16_FLOAT);

	float rtScale = 0.5f;
	for (int i = 0; i < MaxBloomLevels; i++)
	{
		ResizeOnePostProcessResource(blurHorizontalRTV[i], blurHorizontalSRV[i], rtScale);
		ResizeOnePostProcessResource(blurVerticalRTV[i], blurVerticalSRV[i], rtScale);

		// Each successive bloom level is half the resolution
		rtScale *= 0.5f;
	}
}

void Game::ResizeOnePostProcessResource(
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>& rtv,
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& srv,
	float renderTargetScale,
	DXGI_FORMAT format)
{
	// Reset if they already exist
	rtv.Reset();
	srv.Reset();

	// Describe the render target
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = (unsigned int)(width * renderTargetScale);
	textureDesc.Height = (unsigned int)(height * renderTargetScale);
	textureDesc.ArraySize = 1;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE; // Will render to it and sample from it!
	textureDesc.CPUAccessFlags = 0;
	textureDesc.Format = format;
	textureDesc.MipLevels = 1;
	textureDesc.MiscFlags = 0;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> ppTexture;
	device->CreateTexture2D(&textureDesc, 0, ppTexture.GetAddressOf());

	// Create the Render Target View
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	device->CreateRenderTargetView(ppTexture.Get(), &rtvDesc, rtv.ReleaseAndGetAddressOf());

	// Create the Shader Resource View using a null description
	// which gives a default SRV with access to the whole resource
	device->CreateShaderResourceView(ppTexture.Get(), 0, srv.ReleaseAndGetAddressOf());
}


// Generates lights in scene
void Game::GenerateLights()
{
	// Lights
	// Red directional right
	lights.push_back({});
	lights[0].Type = LIGHT_TYPE_DIRECTIONAL;
	lights[0].Direction = XMFLOAT3(1, 0, 0);
	lights[0].Color = XMFLOAT3(1, 0, 0);
	//lights[0].Intensity = 0.2f;
	lights[0].Intensity = 0;

	// Green directional down
	lights.push_back({});
	lights[1].Type = LIGHT_TYPE_DIRECTIONAL;
	lights[1].Direction = XMFLOAT3(0, -1, 0);
	lights[1].Color = XMFLOAT3(1, 1, 1);
	//lights[1].Intensity = 0.2f;
	lights[1].Intensity = 0;

	// Blue directional up at an angle
	lights.push_back({});
	lights[2].Type = LIGHT_TYPE_DIRECTIONAL;
	lights[2].Direction = XMFLOAT3(-1, 1, -0.5f);
	lights[2].Color = XMFLOAT3(1, 1, 1);
	//lights[2].Intensity = 0.2f;
	lights[2].Intensity = 0;

	// White blue light with range 10 positioned between sphere and helix
	lights.push_back({});
	lights[3].Type = LIGHT_TYPE_POINT;
	lights[3].Range = 10;
	lights[3].Position = XMFLOAT3(10, 0, -2);
	lights[3].Color = XMFLOAT3(0, 0, 1);
	lights[3].Intensity = 1;

	// Dim green point light with range 10 positioned between sphere and torus
	lights.push_back({});
	lights[4].Type = LIGHT_TYPE_POINT;
	lights[4].Range = 10;
	lights[4].Position = XMFLOAT3(14, 0, -2);
	lights[4].Color = XMFLOAT3(0, 1, 0);
	lights[4].Intensity = 1;

	// White directional forward
	lights.push_back({});
	lights[5].Type = LIGHT_TYPE_DIRECTIONAL;
	lights[5].Direction = XMFLOAT3(0, 0, 1);
	lights[5].Color = XMFLOAT3(1, 1, 1);
	//lights[5].Intensity = 0.2f;
	lights[5].Intensity = 0;

	// Creates 20 white point lights on ceiling
	for (int i = 6; i < 26; i++)
	{
		int zPos = 7;
		if (i >= 10 && i < 14)
		{
			zPos = 13;
		}
		else if (i >= 14 && i < 18)
		{
			zPos = 28;
		}
		else if (i >= 17 && i < 22)
		{
			zPos = 32;
		}

		lights.push_back({});
		lights[i].Type = LIGHT_TYPE_POINT;
		lights[i].Range = 10;
		lights[i].Position = XMFLOAT3(2 + ((i - 6) % 4) * 6.75f, 2, zPos);
		lights[i].Color = XMFLOAT3(.6f, .2f, 1);
		lights[i].Intensity = 10;

		// Middle lights
		if (i >= 22 && i < 24)
		{
			lights[i].Position = XMFLOAT3(9 + ((i - 6) % 2) * 6, 2, 17);
		}
		else if (i >= 24)
		{
			lights[i].Position = XMFLOAT3(9 + ((i - 6) % 2) * 6, 2, 23);
		}
	}
}

// --------------------------------------------------------
// Handle resizing DirectX "stuff" to match the new window size.
// For instance, updating our projection matrix's aspect ratio.
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();

	// Updates the projection matrix
	camera->UpdateProjectionMatrix((float)width / height);

	// Resize post process resources
	ResizeAllPostProcessResources();
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	Input& input = Input::GetInstance();

	// Example input checking: Quit if the escape key is pressed
	if (input.KeyDown(VK_ESCAPE))
		Quit();

	// Makes entities change their transforms with sin
	for (int i = 0; i < entities.size(); i++)
	{
		if (i < entities.size() - 1)
		{
			//entities[i]->GetTransform()->SetPosition(sin(totalTime) * (float)i / 1.5f, 0, 0);
			//entities[i]->GetTransform()->SetScale(sin(totalTime) / 4, sin(totalTime) / 4, sin(totalTime) / 4);
			//entities[i]->GetTransform()->Rotate(0, sin(deltaTime), 0);
		}
	}

	// Adjusts blur amount w/ arrow keys
	if (input.KeyPress(VK_UP)) { additionalBlurAmount++; }
	if (input.KeyPress(VK_DOWN)) { additionalBlurAmount--; }
	additionalBlurAmount = min(max(additionalBlurAmount, 0), 15); // Clamp between 0 and 15
	// Toggles bluring
	if (input.KeyPress('Q')) { blurMultiplier > 0 ? blurMultiplier = 0 : blurMultiplier = .6f; }

	// Updates camera
	camera->Update(deltaTime);

	// Adjusts blur amount based on camera speed
	blurAmount = camera->getCurrentMoveSpeed() * blurMultiplier + additionalBlurAmount;
	blurAmount = min(max(blurAmount, 0), 15); // Clamp between 0 and 15

	// Handle bloom input
	if (input.KeyDown(VK_LEFT)) { bloomThreshold -= 0.1f * deltaTime; }
	if (input.KeyDown(VK_RIGHT)) { bloomThreshold += 0.1f * deltaTime; }
	bloomThreshold = max(bloomThreshold, 0);

	if (input.KeyPress(VK_OEM_MINUS)) { bloomLevels--; }
	if (input.KeyPress(VK_OEM_PLUS)) { bloomLevels++; }
	bloomLevels = max(min(bloomLevels, MaxBloomLevels), 0);

	if (input.KeyPress('E')) { drawBloomTextures = !drawBloomTextures; }
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Background color (Cornflower Blue in this case) for clearing
	const float color[4] = { 0, 0, 0, 0.0f };

	// Clear the render target and depth buffer (erases what's on the screen)
	//  - Do this ONCE PER FRAME
	//  - At the beginning of Draw (before drawing *anything*)
	context->ClearRenderTargetView(backBufferRTV.Get(), color);
	context->ClearDepthStencilView(
		depthStencilView.Get(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);

	// -----------------------------POST PROCESS PRE DRAW-------------------------
	// For post processing, swap to post process RTV
	// Clear render targets
	context->ClearRenderTargetView(ppRTV.Get(), color);
	context->ClearRenderTargetView(bloomExtractRTV.Get(), color);
	context->ClearRenderTargetView(bloomCombineRTV.Get(), color);

	for (int i = 0; i < MaxBloomLevels; i++)
	{
		context->ClearRenderTargetView(blurHorizontalRTV[i].Get(), color);
		context->ClearRenderTargetView(blurVerticalRTV[i].Get(), color);
	}

	// Sets render target for bloom
	context->OMSetRenderTargets(1, ppRTV.GetAddressOf(), depthStencilView.Get());

	// -----------------------DRAWS ENTITIES-------------------------
	for (std::shared_ptr<Entity> entity : entities)
	{
		entity->Draw(context, camera, totalTime, ambientColor, lights);
	}

	// Draws sky box after entities
	skyBox->Draw(context, camera);

	// ----------------------------POST PROCESS POST DRAW----------------------
	// Post process drawing - need to swap output back to back buffer
	// Unbind vertex and index buffer
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	ID3D11Buffer* nullBuffer = 0;
	context->IASetVertexBuffers(0, 1, &nullBuffer, &stride, &offset);
	context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

	// This is the same vertex shader used for all post processing, so set it once
	ppVS->SetShader();

	// Assuming all of the post process steps have a single sampler at register 0
	context->PSSetSamplers(0, 1, ppSampler.GetAddressOf());

	// Handle the bloom extraction
	BloomExtract();
	
	// Any bloom actually happening?
	if (bloomLevels >= 1 && drawBloomTextures)
	{
		float levelScale = 0.5f;
		SingleDirectionBlur(levelScale, XMFLOAT2(1, 0), blurHorizontalRTV[0], bloomExtractSRV); // Bloom extract is the source
		SingleDirectionBlur(levelScale, XMFLOAT2(0, 1), blurVerticalRTV[0], blurHorizontalSRV[0]);
	
		// Any other levels?
		for (int i = 1; i < bloomLevels; i++)
		{
			levelScale *= 0.5f; // Half the size of the previous
			SingleDirectionBlur(levelScale, XMFLOAT2(1, 0), blurHorizontalRTV[i], blurVerticalSRV[i - 1]); // Previous blur is the source
			SingleDirectionBlur(levelScale, XMFLOAT2(0, 1), blurVerticalRTV[i], blurHorizontalSRV[i]);
		}
	}
	
	// Final combine
	BloomCombine(); // This step should reset viewport and write to the bloom extract

	// Blurs full screen
	fullScreenBlur();

	// Unbind shader resource views at the end of the frame,
	// since we'll be rendering into one of those textures
	// at the start of the next
	ID3D11ShaderResourceView* nullSRVs[16] = {};
	context->PSSetShaderResources(0, 16, nullSRVs);

	// Present the back buffer to the user
	//  - Puts the final frame we're drawing into the window so the user can see it
	//  - Do this exactly ONCE PER FRAME (always at the very end of the frame)
	swapChain->Present(vsync ? 1 : 0, 0);

	// Due to the usage of a more sophisticated swap chain,
	// the render target must be re-bound after every call to Present()
	context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthStencilView.Get());
}

void Game::fullScreenBlur()
{
	// Turn on special shaders and draw single triangle to fill screen
	// Render to the BACK BUFFER (since this is the last step!)
	context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), 0);

	// Sets blur pixel shader
	fullScreenBlurPS->SetShader();

	// Sets shader values
	fullScreenBlurPS->SetShaderResourceView("pixels", bloomCombineSRV.Get()); // IMPORTANT: This step takes the BLOOM COMBINE post process texture
	fullScreenBlurPS->SetInt("blurAmount", blurAmount);
	fullScreenBlurPS->CopyAllBufferData();

	// Draw exactly 3 vertices for our "full screen triangle"
	context->Draw(3, 0);
}

// Handles extracting the "bright" pixels to a second render target
void Game::BloomExtract()
{
	// We're using a half-sized texture for bloom extract, so adjust the viewport
	D3D11_VIEWPORT vp = {};
	vp.Width = width * 0.5f;
	vp.Height = height * 0.5f;
	vp.MaxDepth = 1.0f;
	context->RSSetViewports(1, &vp);

	// Render to the BLOOM EXTRACT texture
	context->OMSetRenderTargets(1, bloomExtractRTV.GetAddressOf(), 0);

	// Activate the shader and set resources
	bloomExtractPS->SetShader();
	bloomExtractPS->SetShaderResourceView("pixels", ppSRV.Get()); // IMPORTANT: This step takes the original post process texture!
	// Note: Sampler set already!

	// Set post process specific data
	bloomExtractPS->SetFloat("bloomThreshold", bloomThreshold);
	bloomExtractPS->CopyAllBufferData();

	// Draw exactly 3 vertices for our "full screen triangle"
	context->Draw(3, 0);
}


// Blurs in a single direction, based on the "blurDirection" parameter
// This allows us to use a single shader for both horizontal and vertical
// blurring, rather than having to write two nearly-identical shaders
void Game::SingleDirectionBlur(float renderTargetScale, DirectX::XMFLOAT2 blurDirection, Microsoft::WRL::ComPtr<ID3D11RenderTargetView> target, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> sourceTexture)
{
	// Ensure our viewport matches our render target
	D3D11_VIEWPORT vp = {};
	vp.Width = width * renderTargetScale;
	vp.Height = height * renderTargetScale;
	vp.MaxDepth = 1.0f;
	context->RSSetViewports(1, &vp);

	// Target to which we're rendering
	context->OMSetRenderTargets(1, target.GetAddressOf(), 0);

	// Activate the shader and set resources
	gaussianBlurPS->SetShader();
	gaussianBlurPS->SetShaderResourceView("pixels", sourceTexture.Get()); // The texture from the previous step
	// Note: Sampler set already!

	// Set post process specific data
	gaussianBlurPS->SetFloat2("pixelUVSize", XMFLOAT2(1.0f / (width * renderTargetScale), 1.0f / (height * renderTargetScale)));
	gaussianBlurPS->SetFloat2("blurDirection", blurDirection);
	gaussianBlurPS->CopyAllBufferData();

	// Draw exactly 3 vertices for our "full screen triangle"
	context->Draw(3, 0);
}

// Combines all bloom levels with the original post process target
// Note: If a level isn't being used, it's still cleared to black
//       so it won't have any impact on the final result
void Game::BloomCombine()
{
	// Back to the full window viewport
	D3D11_VIEWPORT vp = {};
	vp.Width = (float)width;
	vp.Height = (float)height;
	vp.MaxDepth = 1.0f;
	context->RSSetViewports(1, &vp);

	// Render to the BLOOM COMBINE texture
	context->OMSetRenderTargets(1, bloomCombineRTV.GetAddressOf(), 0);

	// Activate the shader and set resources
	bloomCombinePS->SetShader();
	bloomCombinePS->SetShaderResourceView("originalPixels", ppSRV.Get()); // Set the original render
	bloomCombinePS->SetShaderResourceView("bloomedPixels0", blurVerticalSRV[0].Get()); // And all other bloom levels
	bloomCombinePS->SetShaderResourceView("bloomedPixels1", blurVerticalSRV[1].Get()); // And all other bloom levels
	bloomCombinePS->SetShaderResourceView("bloomedPixels2", blurVerticalSRV[2].Get()); // And all other bloom levels
	bloomCombinePS->SetShaderResourceView("bloomedPixels3", blurVerticalSRV[3].Get()); // And all other bloom levels
	bloomCombinePS->SetShaderResourceView("bloomedPixels4", blurVerticalSRV[4].Get()); // And all other bloom levels

	// Note: Sampler set already!

	// Set post process specific data
	bloomCombinePS->SetFloat("intensityLevel0", bloomLevelIntensities[0]);
	bloomCombinePS->SetFloat("intensityLevel1", bloomLevelIntensities[1]);
	bloomCombinePS->SetFloat("intensityLevel2", bloomLevelIntensities[2]);
	bloomCombinePS->SetFloat("intensityLevel3", bloomLevelIntensities[3]);
	bloomCombinePS->SetFloat("intensityLevel4", bloomLevelIntensities[4]);
	bloomCombinePS->CopyAllBufferData();

	// Draw exactly 3 vertices for our "full screen triangle"
	context->Draw(3, 0);
}