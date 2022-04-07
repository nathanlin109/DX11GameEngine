#include "ShaderIncludes.hlsli"
cbuffer ExternalData : register(b0) 
{ 
	matrix worldMatrix; 
	matrix worldInvMatrix; 
	matrix viewMatrix;
	matrix projectionMatrix;
}

// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// 
// - Input is exactly one vertex worth of data (defined by a struct)
// - Output is a single struct of data to pass down the pipeline
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
VertexToPixel main( VertexShaderInput input )
{
	// Set up output struct
	VertexToPixel output;

	// Multiplies the projection, view, and world matrices
	matrix wvp = mul(mul(projectionMatrix, viewMatrix), worldMatrix);
	output.screenPosition = mul(wvp, float4(input.localPosition, 1.0f));

	// Sets the UV coordinates
	output.uv = input.uv;
	
	// Sets the tangents
    output.tangent = mul((float3x3) worldInvMatrix, input.tangent);;

	// Sets the normals
	output.normal = mul((float3x3)worldInvMatrix, input.normal);

	// Sets the world position
	output.worldPosition = mul(worldMatrix, float4(input.localPosition, 1)).xyz;

	// Whatever we return will make its way through the pipeline to the
	// next programmable stage we're using (the pixel shader for now)
	return output;
}