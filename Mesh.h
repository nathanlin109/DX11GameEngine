#pragma once
#include <d3d11.h>
#include "Vertex.h"
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects

using namespace DirectX;

class Mesh
{
private:
	// ComPtr's to the vertex and index buffers and device
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	int indexCount;
public:
	// Constructor
	Mesh(Vertex* vertices, int vertexCount, unsigned int* indices, int indexCount,
		Microsoft::WRL::ComPtr<ID3D11Device> device);

	// Ctor for loading mesh from file
	Mesh(const char* objFile, Microsoft::WRL::ComPtr<ID3D11Device> device);

	~Mesh();

	// Returns vertex buffer ptr
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();

	// Returns index buffer ptr
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();

	// Returns index count for mesh
	int GetIndexCount();

	// Sets buffers and tells DirectX to draw the correct number of indices
	void Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context);

	// Creates meshes. Used for both CTORS
	void CreateMesh(Vertex* vertices, int vertexCount, unsigned int* indices, int indexCount,
		Microsoft::WRL::ComPtr<ID3D11Device> device);

	// Calculates tangents
	void CalculateTangents(Vertex* verts, int numVerts, unsigned int* indices, int numIndices);
};

