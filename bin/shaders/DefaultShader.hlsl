#include "InputStructures.hlsli"

struct VertexInput {
    float3 position : POSITION;
    float3 color : COLOR0;
};

struct VertexToPixel {
	float4 Position : SV_Position;
	float3 Color : COLOR0;
};

struct ColorInput {
	float3 Color : COLOR0;
};

struct ColorOutput {
	float4 Color : SV_Target0;
};

VertexToPixel VSMain(VertexInput input) {
	VertexToPixel output;
	float4 worldPos = mul(model, float4(input.position, 1.0f));
	output.Position = mul(viewProj, worldPos);
	output.Color = input.color;
    return output;
}

ColorOutput FSMain(ColorInput input) {
	ColorOutput output;
	output.Color = float4(input.Color.r, input.Color.g, input.Color.b, 1.0f);
	return output;
}
