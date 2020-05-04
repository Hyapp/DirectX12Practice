cbuffer cbPerObject : register(b0)
{
	float4x4 gWorldViewProj;
};

struct VertexIn
{
	float3 posL : POSITION;
	float4 color : COLOR;
};

struct VertexOut
{
	float4 posH : SV_Position;
	float4 color : COLOR;
};


VertexOut main(VertexIn appdata)
{
	VertexOut o;
	//float4 posL = float4(appdata.posL, 1.0f);
	o.posH = mul(float4(appdata.posL, 1.0f), gWorldViewProj);
	o.color = appdata.color;
	
	return o;
}

float4 PSMain(VertexOut i) : SV_Target
{
	return i.color;
}