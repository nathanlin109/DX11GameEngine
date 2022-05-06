cbuffer ExternalData : register(b0)
{
    float pixelWidth;
    float pixelHeight;
    int blurAmount;
}

// Defines input to this pixel shader
struct VertexToPixel
{
    float4 position : SV_Position;
    float2 uv       : TEXCOORD0;
};

// Textures
Texture2D pixels            : register(t0);
SamplerState samplerOptions : register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
    float4 totalColor = float4(0, 0, 0, 0);
    int totalSamples = 0;
    
    for (int y = -blurAmount; y <= blurAmount; y++)
    {
        for (int x = -blurAmount; x <= blurAmount; x++)
        {
            int3 pixelPos = int3(
            max(0, input.position.x + x),
            max(0, input.position.y + y),
            0);
            totalColor += pixels.Load(pixelPos);
            totalSamples++;
        }
    }
    
    return totalColor / totalSamples;
}