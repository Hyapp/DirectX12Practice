#pragma once
#include <d3dUtil.h>
#include <MathHelper.h>
#include <UploadBuffer.h>

namespace C7_Geometry
{
    struct ObjectConstants_C7
    {
        DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();
    };

    struct PassConstants
    {
        DirectX::XMFLOAT4X4 View = MathHelper::Identity4x4();
        DirectX::XMFLOAT4X4 InvView = MathHelper::Identity4x4();
        DirectX::XMFLOAT4X4 Proj = MathHelper::Identity4x4();
        DirectX::XMFLOAT4X4 InvProj = MathHelper::Identity4x4();
        DirectX::XMFLOAT4X4 ViewProj = MathHelper::Identity4x4();
        DirectX::XMFLOAT4X4 InvViewProj = MathHelper::Identity4x4();

        DirectX::XMFLOAT3 EyePosW = { 0.0f,0.0f,0.0f };
        float cbPerObjectPad1 = 0.0f;
        DirectX::XMFLOAT2 RenderTargetSize = { 0.0f,0.0f };
        DirectX::XMFLOAT2 InvRenderTargetSize = { 0.0f,0.0f };
        float NearZ = 0.0f;
        float  FarZ = 0.0f;
        float TotalTime = 0.0f;
        float DeltaTime = 0.0f;
    };

    struct Vertex_C7
    {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT4 color;
    };

    struct FrameResource
    {
    public:
        FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount);
        FrameResource(const FrameResource& rhs) = delete;
        FrameResource& operator=(const FrameResource& rhs) = delete;
        ~FrameResource();

        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CmdListAlloc;

        std::unique_ptr<UploadBuffer<PassConstants>> PassCB = nullptr;
        std::unique_ptr<UploadBuffer<ObjectConstants_C7>> ObjectCB = nullptr;

        UINT64 Fence = 0;
    };
}

