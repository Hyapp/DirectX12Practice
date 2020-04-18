struct PSInput
{
	float4 position : SV_Position;
	float4 color : COLOR;
};

PSInput main(float4 pos : POSITION, float4 color : COLOR)
{
	PSInput result;
	result.position = pos;
	result.color = color;
	return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
	return input.color;
}