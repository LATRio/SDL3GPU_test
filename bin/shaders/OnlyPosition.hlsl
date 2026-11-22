struct VertexInput {
    float3 position : POSITION;
    float3 color : COLOR0;
// 	float x : TEXCOORD0;
// 	float y : TEXCOORD1;
// 	float z : TEXCOORD2;
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
	output.Position = float4(input.position.x, input.position.y, input.position.z, 1.0f);
	output.Color = input.color;
    return output;
}

ColorOutput FSMain(ColorInput input) {
	ColorOutput output;
	output.Color = float4(input.Color.r, input.Color.g, input.Color.b, 1.0f);
	return output;
}
