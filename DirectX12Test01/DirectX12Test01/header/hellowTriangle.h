#pragma once

#include <DXSample.h>

using namespace DirectX;

using Microsoft::WRL::ComPtr;

class D3D12HellowTriangle : public DXSample
{
public:
	D3D12HellowTriangle(UINT width, UINT height, std::wstring title);

	virtual void OnInit() override;
	virtual void OnUpdate() override;
	virtual void OnRender() override;
	virtual void OnDestroy() override;

	virtual void OnKeyDown(UINT8) override;

private:
	static const UINT FrameCount = 2;

	struct  Vertex
	{
		XMFLOAT3 position;
		XMFLOAT4 color;
	};

	CD3DX12_VIEWPORT m_viewport;
	CD3DX12_RECT m_scissorRect;
	ComPtr<IDXGISwapChain3> m_swapChain;
	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	UINT m_rtvDescriptorSize;

	//App resoureces
	ComPtr<ID3D12Resource> m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

	//Synchronization objects
	UINT m_frameIndex;
	HANDLE m_fenceEvent;
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValue;


	void LoadPipeline();
	void LoadAssets();
	void PopulateCommandList();
	void WaitForPreviousFrame(); 

	std::ofstream m_log_os;
	std::chrono::high_resolution_clock::time_point m_start_time;
	unsigned int m_fps_counter = 0;
};