﻿#include <stdafx.h>
#include <ConstBuffer.h>

D3D12HelloConstBuffer::D3D12HelloConstBuffer(UINT width, UINT height, std::wstring title) :
	DXSample(width, height, title),
	m_frameIndex(0),
	m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
	m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
	m_rtvDescriptorSize(0),
	m_pCbvDataBegin(nullptr),
	m_constantBufferData{}
{}

void D3D12HelloConstBuffer::OnInit()
{
	LoadPipeline();
	LoadAssets();
}

void D3D12HelloConstBuffer::LoadPipeline()
{
	UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG) || defined(DBG)
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();

			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif
	//Factory
	ComPtr<IDXGIFactory4> factory;
	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

	//adapter and device
	if (m_useWarpDevice)
	{
		ComPtr<IDXGIAdapter> warpAdapter;
		ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

		ThrowIfFailed(D3D12CreateDevice(
			warpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&m_device)
		));
	}
	else
	{
		ComPtr<IDXGIAdapter1> hardwareAdapter;
		GetHardwareAdapter(factory.Get(), &hardwareAdapter);

		ThrowIfFailed(D3D12CreateDevice(
			hardwareAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&m_device)
		));
	}

	//Describe and create the command queue;
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

	//Describe and create the swap chain;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	//SwapChain 里面包含了有多少交换RT,device使用RTV Heap 引用了交换链上的RT
	swapChainDesc.BufferCount = FrameCount;
	swapChainDesc.Width = m_width;
	swapChainDesc.Height = m_height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	ComPtr<IDXGISwapChain1> swapChain;
	ThrowIfFailed(factory->CreateSwapChainForHwnd(
		m_commandQueue.Get(),
		Win32Application::GetHwnd(),
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	));

	ThrowIfFailed(factory->MakeWindowAssociation(Win32Application::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));
	//用as赋值
	ThrowIfFailed(swapChain.As(&m_swapChain));
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	//Create descriptor heaps;
	{
		//rtv的描述符堆，描述rtv的信息
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = FrameCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		// device创建 rtv描述符。
		ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
		// 每个描述符的大小
		m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		//cbv的描述符堆，描述了CBV信息
		D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
		cbvHeapDesc.NumDescriptors = 1;
		cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		ThrowIfFailed(m_device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_cbvHeap)));
	}

	//Create a RTV for each frame
	{
		// rtvHandle是从m_rtvHeap中获取到的CPU Handle地址，
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
		for (UINT i = 0; i < FrameCount; i++)
		{
			// RT本身属于swapChain, 而非device
			ThrowIfFailed(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i])));
			// device给自己的rtvHeap赋值，值为交换链的RTbuffer.
			m_device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, m_rtvDescriptorSize);
		}
	}

	ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));
}

void D3D12HelloConstBuffer::LoadAssets()
{
	//Create a root signature consisting of a descriptor table with a single CBV.
	{
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

		if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}

		CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
		CD3DX12_ROOT_PARAMETER1 rootParameters[1];

		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
		//初始化根签名表，这里已经不是直接写进根签名了
		rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_VERTEX);

		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		// dx12 根签名必须先被序列化才能创建
		ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
		ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
	}

	////Create an empty root signature
	//{
	//	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	//	rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	//	ComPtr<ID3DBlob> signature;
	//	ComPtr<ID3DBlob> error;
	//	ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
	//	ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
	//}

	{
		ComPtr<ID3DBlob> vertexShader;
		ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG) || defined(DBG)
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif

		ThrowIfFailed(D3DCompileFromFile(LR"(Shader/CBufferTest.hlsl)", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
		ThrowIfFailed(D3DCompileFromFile(LR"(Shader/CBufferTest.hlsl)", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		};

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
		psoDesc.pRootSignature = m_rootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState.DepthEnable = FALSE;
		psoDesc.DepthStencilState.StencilEnable = FALSE;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = 1;
		ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
	}

	ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)));

	ThrowIfFailed(m_commandList->Close());

	//Create the vertex buffer
	{
		Vertex triangleVertices[] =
		{
			{{0.0f, 0.25f * m_aspectRatio, 0.0f}, {1.0f, 0.0f,0.0f,1.0f}},
			{{0.25f, -0.25f * m_aspectRatio, 0.0f}, {0.0f, 1.0f, 0.0f,1.0f}},
			{{-0.25f, -0.25f * m_aspectRatio, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},

		};

		const UINT vertexBufferSize = sizeof(triangleVertices);
		// device创建一段顶点大小的buffer，类型为上传只读
		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_vertexBuffer)
		));

		// 通过映射至RAM，直接复制到RAM会同步至VRAM。
		UINT8* pVertexDataBegin;
		CD3DX12_RANGE rendRange(0, 0);
		ThrowIfFailed(m_vertexBuffer->Map(0, &rendRange, reinterpret_cast<void**>(&pVertexDataBegin)));
		memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
		m_vertexBuffer->Unmap(0, nullptr);

		//Initialize the vertex buffer view.
		// 渲染数据被Unmap之后，这些缓冲区需要直接拿到GPU地址提供给cmd去渲染。？
		m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
		m_vertexBufferView.StrideInBytes = sizeof(Vertex);
		m_vertexBufferView.SizeInBytes = vertexBufferSize;
	}

	//Create the constant buffer.
	//使用上传堆上传CB
	{
		// device 创建一个大小未1024*64大小的buffer 为GPU只读上传类型
		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(1024 * 64),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_constantBuffer)));

		//Describe and create a constant buffer view.
		// device使用cbvDesc来关联创建的buffer和cbv 描述符，使得描述符描述了这个buffer
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		// 直接存了GPU VirtualAddress,设到RAM？
		cbvDesc.BufferLocation = m_constantBuffer->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = (sizeof(SceneConstantBuffer) + 255) & ~255; //CB size is required to be 256-byte aligned.
		// 描述符有CPU端和GPU端？？？Why？
		m_device->CreateConstantBufferView(&cbvDesc, m_cbvHeap->GetCPUDescriptorHandleForHeapStart());

		//Map and initialize the constant buffer. We don't unmap this until the app closes.
		//Update to Upload datas.
		CD3DX12_RANGE readRange(0, 0);
		ThrowIfFailed(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pCbvDataBegin)));
		memcpy(m_pCbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));
	}

	//Create synchronization objects and wait until assets have been upload to The GPU.
	{
		ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
		m_fenceValue = 1;

		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_fenceEvent == nullptr)
		{
			ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
		}

		WaitForPreviousFrame();
	}
}

void D3D12HelloConstBuffer::OnUpdate()
{
	const float translationSpeed = 0.005f;
	const float offsetBounds = 1.25f;

	m_constantBufferData.offset.x += translationSpeed;
	if (m_constantBufferData.offset.x > offsetBounds)
	{
		m_constantBufferData.offset.x = -offsetBounds;
	}
	//没UnMap.持续性往里面写数据
	memcpy(m_pCbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));
}

void D3D12HelloConstBuffer::OnRender()
{
	PopulateCommandList();

	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	ThrowIfFailed(m_swapChain->Present(1, 0));

	WaitForPreviousFrame();
}

void D3D12HelloConstBuffer::OnDestroy()
{
	//好像就不用管UnMap了？？？
	WaitForPreviousFrame();

	CloseHandle(m_fenceEvent);
}

void D3D12HelloConstBuffer::PopulateCommandList()
{
	ThrowIfFailed(m_commandAllocator->Reset());

	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get()));

	//Set necessary state.
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	ID3D12DescriptorHeap* ppHeaps[] = { m_cbvHeap.Get() };
	m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	// cbv直接引用GPU的描述符
	m_commandList->SetGraphicsRootDescriptorTable(0, m_cbvHeap->GetGPUDescriptorHandleForHeapStart());
	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	//Indicate that the back buffer will be used as a render target.
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	//rtvHeap引用的是SwapChain的资源，可是这必须是CPU所描述的？？？
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	//设完了就可以开始画了。。。
	const float clearColor[] = { 0.0f,0.0f,0.0f,1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	m_commandList->DrawInstanced(3, 1, 0, 0);


	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	ThrowIfFailed(m_commandList->Close());
}

void D3D12HelloConstBuffer::WaitForPreviousFrame()
{
	const UINT64 fence = m_fenceValue;

	ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fence));
	++m_fenceValue;

	if (m_fence->GetCompletedValue() < fence)
	{
		ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}