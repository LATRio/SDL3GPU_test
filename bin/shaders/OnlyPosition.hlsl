struct VertexInput {
    //float3 position : POSITION;
	float x : TEXCOORD0;
	float y : TEXCOORD1;
	float z : TEXCOORD2;
};

struct VertexToPixel {
	float4 Position : SV_Position;
};

struct ColorInput {
};

struct ColorOutput {
	float4 Color : SV_Target0;
};

VertexToPixel VSMain(VertexInput input) {
	VertexToPixel output;
	output.Position = float4(input.x, input.y, input.z, 1.0f);
    return output;
}

ColorOutput FSMain(ColorInput input) {
	ColorOutput output;
	output.Color = float4(1.0f, 0.0f, 0.0f, 1.0f);
	return output;
}
