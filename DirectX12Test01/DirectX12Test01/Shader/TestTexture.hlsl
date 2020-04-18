struct PSInput
{
	float4 position : SV_Position;
	float2 uv : TEXCOORD;
};

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);


PSInput main(float4 pos : POSITION, float4 uv : TEXCOORD)
{
	PSInput result;

	result.position = pos;
	result.uv = uv;

	return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
	return g_texture.Sample(g_sampler, input.uv);
}