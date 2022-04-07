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
	vsync(false)
{
#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.\n");
#endif

	// Creates camera
	camera = std::make_shared<Camera>(12, 0, -25,
		(float)width / height, 3, 1.5f, 2);
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

	// Loads textures
	// Cobblestone albedo
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestoneSRVPtr;
	CreateWICTextureFromFile(device.Get(), context.Get(),
		GetFullPathTo_Wide(L"../../Assets/Textures/cobblestone/cobblestone.png").c_str(),
		nullptr, &cobblestoneSRVPtr);
	// Cobblestone normal
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestoneNormalsSRVPtr;
	CreateWICTextureFromFile(device.Get(), context.Get(),
		GetFullPathTo_Wide(L"../../Assets/Textures/cobblestone/cobblestone_normals.png").c_str(),
		nullptr, &cobblestoneNormalsSRVPtr);
	// Cushion albedo
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cushionSRVPtr;
	CreateWICTextureFromFile(device.Get(), context.Get(),
		GetFullPathTo_Wide(L"../../Assets/Textures/cushion/cushion.png").c_str(),
		nullptr, &cushionSRVPtr);
	// Cushion normal
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cushionNormalsSRVPtr;
	CreateWICTextureFromFile(device.Get(), context.Get(),
		GetFullPathTo_Wide(L"../../Assets/Textures/cushion/cushion_normals.png").c_str(),
		nullptr, &cushionNormalsSRVPtr);
	// Rock albedo
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> rockSRVPtr;
	CreateWICTextureFromFile(device.Get(), context.Get(),
		GetFullPathTo_Wide(L"../../Assets/Textures/rock/rock.png").c_str(),
		nullptr, &rockSRVPtr);
	// Rock normal
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> rockNormalsSRVPtr;
	CreateWICTextureFromFile(device.Get(), context.Get(),
		GetFullPathTo_Wide(L"../../Assets/Textures/rock/rock_normals.png").c_str(),
		nullptr, &rockNormalsSRVPtr);
	// Specular texture to use for everything (black spec)
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> generalSpecularSRVPtr;
	CreateWICTextureFromFile(device.Get(), context.Get(),
		GetFullPathTo_Wide(L"../../Assets/Textures/general_metallic_white.png").c_str(),
		nullptr, &generalSpecularSRVPtr);

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
	materials.push_back(std::make_shared<Material>(
		XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f), vertexShader, pixelShader, 0.15f));
	// Yellow
	materials.push_back(std::make_shared<Material>(
		XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), vertexShader, pixelShader, 0.15f));
	// Cyan
	materials.push_back(std::make_shared<Material>(
		XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), vertexShader, pixelShader, 0.15f));
	// White/cobblestone
	materials.push_back(std::make_shared<Material>(
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), vertexShader, pixelShader, 0.15f));
	// White/cushion
	materials.push_back(std::make_shared<Material>(
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), vertexShader, pixelShader, 0.15f));
	// White/rock
	materials.push_back(std::make_shared<Material>(
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), vertexShader, pixelShader, 0.15f));

	// Adds SRV's and samplers to materials
	materials[3]->AddTextureSRV("SurfaceTexture", cobblestoneSRVPtr);
	materials[3]->AddTextureSRV("NormalTexture", cobblestoneNormalsSRVPtr);
	materials[3]->AddTextureSRV("SpecularTexture", generalSpecularSRVPtr);
	materials[3]->AddSampler("BasicSampler", samplerState);
	materials[4]->AddTextureSRV("SurfaceTexture", cushionSRVPtr);
	materials[4]->AddTextureSRV("NormalTexture", cushionNormalsSRVPtr);
	materials[4]->AddTextureSRV("SpecularTexture", generalSpecularSRVPtr);
	materials[4]->AddSampler("BasicSampler", samplerState);
	materials[5]->AddTextureSRV("SurfaceTexture", rockSRVPtr);
	materials[5]->AddTextureSRV("NormalTexture", rockNormalsSRVPtr);
	materials[5]->AddTextureSRV("SpecularTexture", generalSpecularSRVPtr);
	materials[5]->AddSampler("BasicSampler", samplerState);

	// Sets lighting variables
	// Ambient
	ambientColor = XMFLOAT3(0.1f, 0.1f, 0.15f);
	//ambientColor = XMFLOAT3(0.0f, 0.0f, 0.0f);

	// Lights
	// Red directional right
	lights.push_back({});
	lights[0].Type = LIGHT_TYPE_DIRECTIONAL;
	lights[0].Direction = XMFLOAT3(1, 0, 0);
	lights[0].Color = XMFLOAT3(0, 0, 0);
	lights[0].Intensity = 0.4f;

	// Green directional down
	lights.push_back({});
	lights[1].Type = LIGHT_TYPE_DIRECTIONAL;
	lights[1].Direction = XMFLOAT3(0, -1, 0);
	lights[1].Color = XMFLOAT3(1, 1, 1);
	lights[1].Intensity = 0.4f;

	// Blue directional up at an angle
	lights.push_back({});
	lights[2].Type = LIGHT_TYPE_DIRECTIONAL;
	lights[2].Direction = XMFLOAT3(-1, 1, -0.5f);
	lights[2].Color = XMFLOAT3(1, 1, 1);
	lights[2].Intensity = 0.4f;

	// White point light with range 10 positioned between sphere and helix
	lights.push_back({});
	lights[3].Type = LIGHT_TYPE_POINT;
	lights[3].Range = 10;
	lights[3].Position = XMFLOAT3(10, 0, 0);
	lights[3].Color = XMFLOAT3(1, 1, 1);
	lights[3].Intensity = 0.4f;

	// Dim white point light with range 10 positioned between sphere and torus
	lights.push_back({});
	lights[4].Type = LIGHT_TYPE_POINT;
	lights[4].Range = 10;
	lights[4].Position = XMFLOAT3(14, 0, 0);
	lights[4].Color = XMFLOAT3(1, 1, 1);
	lights[4].Intensity = 0.4f;

	CreateBasicGeometry();

	// Creates sky box
	skyBox = std::make_shared<Sky>(meshes[0], samplerState, device, 
		GetFullPathTo_Wide(L"../../Assets/Textures/skies/SunnyCubeMap.dds").c_str(),
		skyVertexShader, skyPixelShader);

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

	skyVertexShader = std::make_shared<SimpleVertexShader>(device, context,
		GetFullPathTo_Wide(L"SkyVertexShader.cso").c_str());

	skyPixelShader = std::make_shared<SimplePixelShader>(device, context,
		GetFullPathTo_Wide(L"SkyPixelShader.cso").c_str());

	/*customPixelShader = std::make_shared<SimplePixelShader>(device, context,
		GetFullPathTo_Wide(L"CustomPS.cso").c_str());*/
}



// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateBasicGeometry()
{
	// Creates the meshes
	meshes.push_back(std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/cube.obj").c_str(), device));
	meshes.push_back(std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/cylinder.obj").c_str(), device));
	meshes.push_back(std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/helix.obj").c_str(), device));
	meshes.push_back(std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/sphere.obj").c_str(), device));
	meshes.push_back(std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/torus.obj").c_str(), device));
	meshes.push_back(std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/quad.obj").c_str(), device));
	meshes.push_back(std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/quad_double_sided.obj").c_str(), device));

	// Creates the entities from the meshes
	for (int i = 0; i < meshes.size(); i++)
	{
		// Adds entity to vector
		if (i % 3 == 0) 
		{
			entities.push_back(std::make_shared<Entity>(Transform(), meshes[i], materials[3]));
		}
		else if (i % 3 == 1)
		{
			entities.push_back(std::make_shared<Entity>(Transform(), meshes[i], materials[4]));
		}
		else if (i % 3 == 2)
		{
			entities.push_back(std::make_shared<Entity>(Transform(), meshes[i], materials[5]));
		}
		entities[i]->GetTransform()->SetPosition((float)i * 4, 0, 0);

		// Rotates quads
		if (i >= meshes.size() - 2)
		{
			entities[i]->GetTransform()->SetRotation(-45, -1, 0);
		}
		// 1/3 of meshes w/ purple tint
		/*if (i < meshes.size() / 3)
		{
			// Adds entity to vector
			entities.push_back(std::make_shared<Entity>(Transform(), meshes[i], materials[0]));
		}
		// 1/3 of meshes w/ yellow tint
		else if (i > meshes.size() / 3 + meshes.size() / 3)
		{
			// Adds entity to vector
			entities.push_back(std::make_shared<Entity>(Transform(), meshes[i], materials[1]));
		}
		// 1/3 of meshes w/ cyan tint
		else
		{
			// Adds entity to vector
			entities.push_back(std::make_shared<Entity>(Transform(), meshes[i], materials[2]));
		}*/
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
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Example input checking: Quit if the escape key is pressed
	if (Input::GetInstance().KeyDown(VK_ESCAPE))
		Quit();

	// Makes entities change their transforms with sin
	for (int i = 0; i < entities.size(); i++)
	{
		//entities[i]->GetTransform()->SetPosition(sin(totalTime) * (float)i / 1.5f, 0, 0);
		//entities[i]->GetTransform()->SetScale(sin(totalTime) / 4, sin(totalTime) / 4, sin(totalTime) / 4);
		//entities[i]->GetTransform()->Rotate(0, sin(deltaTime), 0);
	}

	// Updates camera
	camera->Update(deltaTime);
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Background color (Cornflower Blue in this case) for clearing
	const float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };
	//const float color[4] = { 0, 0, 0, 0.0f };

	// Clear the render target and depth buffer (erases what's on the screen)
	//  - Do this ONCE PER FRAME
	//  - At the beginning of Draw (before drawing *anything*)
	context->ClearRenderTargetView(backBufferRTV.Get(), color);
	context->ClearDepthStencilView(
		depthStencilView.Get(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);

	// Draws entities
	for (std::shared_ptr<Entity> entity : entities)
	{
		entity->Draw(context, camera, totalTime, ambientColor, lights);
	}

	// Draws sky box after entities
	skyBox->Draw(context, camera);

	// Present the back buffer to the user
	//  - Puts the final frame we're drawing into the window so the user can see it
	//  - Do this exactly ONCE PER FRAME (always at the very end of the frame)
	swapChain->Present(vsync ? 1 : 0, 0);

	// Due to the usage of a more sophisticated swap chain,
	// the render target must be re-bound after every call to Present()
	context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthStencilView.Get());
}