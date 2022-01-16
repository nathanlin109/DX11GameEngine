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
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
	int indexCount;
public:
	// Constructor
	Mesh(Vertex* vertices, int vertexCount, unsigned int* indices, int indexCount,
		Microsoft::WRL::ComPtr<ID3D11Device> device,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context)
	{
		// Sets fields
		this->context = context;
		this->indexCount = indexCount;

		// Create the VERTEX BUFFER description -----------------------------------
		// - The description is created on the stack because we only need
		//    it to create the buffer.  The description is then useless.
		D3D11_BUFFER_DESC vbd = {};
		vbd.Usage = D3D11_USAGE_IMMUTABLE;
		vbd.ByteWidth = sizeof(Vertex) * vertexCount; // vertexCount = number of vertices in the buffer
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER; // Tells DirectX this is a vertex buffer
		vbd.CPUAccessFlags = 0;
		vbd.MiscFlags = 0;
		vbd.StructureByteStride = 0;

		// Create the proper struct to hold the initial vertex data
		// - This is how we put the initial data into the buffer
		D3D11_SUBRESOURCE_DATA initialVertexData = {};
		initialVertexData.pSysMem = vertices;

		// Actually create the buffer with the initial data
		// - Once we do this, we'll NEVER CHANGE THE BUFFER AGAIN
		device->CreateBuffer(&vbd, &initialVertexData, vertexBuffer.GetAddressOf());

		// Create the INDEX BUFFER description ------------------------------------
		// - The description is created on the stack because we only need
		//    it to create the buffer.  The description is then useless.
		D3D11_BUFFER_DESC ibd = {};
		ibd.Usage = D3D11_USAGE_IMMUTABLE;
		ibd.ByteWidth = sizeof(unsigned int) * indexCount;	// indexCount = number of indices in the buffer
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;	// Tells DirectX this is an index buffer
		ibd.CPUAccessFlags = 0;
		ibd.MiscFlags = 0;
		ibd.StructureByteStride = 0;

		// Create the proper struct to hold the initial index data
		// - This is how we put the initial data into the buffer
		D3D11_SUBRESOURCE_DATA initialIndexData = {};
		initialIndexData.pSysMem = indices;

		// Actually create the buffer with the initial data
		// - Once we do this, we'll NEVER CHANGE THE BUFFER AGAIN
		device->CreateBuffer(&ibd, &initialIndexData, indexBuffer.GetAddressOf());
	}

	~Mesh()
	{

	}

	// Returns vertex buffer ptr
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer()
	{
		return vertexBuffer;
	}

	// Returns index buffer ptr
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer()
	{
		return indexBuffer;
	}

	// Returns index count for mesh
	int GetIndexCount()
	{
		return indexCount;
	}

	// Sets buffers and tells DirectX to draw the correct number of indices
	void Draw()
	{
		// Set buffers in the input assembler
		//  - Do this ONCE PER OBJECT you're drawing, since each object might
		//    have different geometry.
		//  - for this demo, this step *could* simply be done once during Init(),
		//    but I'm doing it here because it's often done multiple times per frame
		//    in a larger application/game
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
		context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		// Finally do the actual drawing
		//  - Do this ONCE PER OBJECT you intend to draw
		//  - This will use all of the currently set DirectX "stuff" (shaders, buffers, etc)
		//  - DrawIndexed() uses the currently set INDEX BUFFER to look up corresponding
		//     vertices in the currently set VERTEX BUFFER
		context->DrawIndexed(
			indexCount,     // The number of indices to use (we could draw a subset if we wanted)
			0,     // Offset to the first index we want to use
			0);    // Offset to add to each index when looking up vertices
	}
};

