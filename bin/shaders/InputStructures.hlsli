cbuffer CameraUBO : register(b0, space1)
{
    float4x4 viewProj;
    float4x4 model;
};