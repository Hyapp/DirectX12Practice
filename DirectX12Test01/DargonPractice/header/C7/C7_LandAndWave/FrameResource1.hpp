#pragma once
#include <d3dUtil.h>
#include <MathHelper.h>
#include <UploadBuffer.h>

namespace C7_LandAndWaves
{
	struct ObjectConstants
	{
		using mat4 = DirectX::XMFLOAT4X4;
		mat4 World = MathHelper::Identity4x4();
	};

	struct PassConstants
	{
		using mat4 = DirectX::XMFLOAT4X4;
		using float2 = DirectX::XMFLOAT2;
		using float3 = DirectX::XMFLOAT3;
		using float4 = DirectX::XMFLOAT4;

		mat4 View = MathHelper::Identity4x4();
		mat4 InvView = MathHelper::Identity4x4();
		mat4 Proj = MathHelper::Identity4x4();
		mat4 InvProj = MathHelper::Identity4x4();
		mat4 ViewProj = MathHelper::Identity4x4();
		mat4 InvViewProj = MathHelper::Identity4x4();

		float3 EyePosW = { 0.0f, 0.0f, 0.0f };
		float cbPerObjectPad1 = 0.0f;
		float2 RenderTargetSize = { 0.0f, 0.0f };
		float2 InvRenderTargetSize = { 0.0f, 0.0f };
		float NearZ = 0.0f;
		float FarZ = 0.0f;
		float TotalTime = 0.0f;
		float DeltaTime = 0.0f;
	};

	struct Vertex
	{
		using float3 = DirectX::XMFLOAT3;
		using float4 = DirectX::XMFLOAT4;

		float3 Pos;
		float4 Color;
	};

	struct FrameResource final
	{
		FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount, UINT waveVertCount);
		FrameResource(const FrameResource& rhs) = delete;
		FrameResource& operator=(const FrameResource& rhs) = delete;
		~FrameResource();

		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CmdListAlloc;

		std::unique_ptr<UploadBuffer<PassConstants>> PassCB = nullptr;
		std::unique_ptr<UploadBuffer<ObjectConstants>> ObjectCB = nullptr;
		std::unique_ptr<UploadBuffer<Vertex>> WavesVB = nullptr;

		UINT64 Fence = 0;
	};
}

