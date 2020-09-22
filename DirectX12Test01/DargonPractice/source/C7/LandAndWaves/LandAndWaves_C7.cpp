#include<d3dApp.h>
#include<MathHelper.h>
#include<UploadBuffer.h>
#include<GeometryGenerator.h>

#include <../header/C7/C7_LandAndWave/FrameResource1.hpp>
#include <../header/C7/C7_LandAndWave/LandAndWaves_C7.hpp>

namespace C7_LandAndWaves
{
	const int gNumFrameResource = 3;

	using namespace DirectX;
	using mat = DirectX::XMMATRIX;

#pragma region Ctor
	LandAndWavesApp::LandAndWavesApp(HINSTANCE hInstance) : D3DApp(hInstance) {}

	LandAndWavesApp::~LandAndWavesApp()
	{
		if (md3dDevice != nullptr)
		{
			FlushCommandQueue();
		}
	}
#pragma endregion Ctor

	bool LandAndWavesApp::Initialize()
	{
		if (!D3DApp::Initialize())
		{
			return false;
		}

		ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

		mWaves = std::make_unique<Geometry::Waves>(128, 128, 1.0f, 0.03f, 4.0f, 0.2f);

		BuildRootSignature();
		BuildShadersAndInputLayout();
		BuildLandGeometry();
		BuildWavesGeometryBuffers();
		BuildRenderItems();
		BuildFrameResources();
		BuildPSOs();

		ThrowIfFailed(mCommandList->Close());
		ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
		mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

		FlushCommandQueue();
		return true;
	}

#pragma region Core Prepar
	void LandAndWavesApp::BuildRootSignature()
	{
		CD3DX12_ROOT_PARAMETER slotRootParameter[2];

		slotRootParameter[0].InitAsConstantBufferView(0);
		slotRootParameter[1].InitAsConstantBufferView(1);

		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2, slotRootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> serializedRootSig = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;

		HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
			serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

		if (errorBlob != nullptr)
		{
			::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}
		ThrowIfFailed(hr);

		ThrowIfFailed(md3dDevice->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(mRootSignature.GetAddressOf())
		));
	}

	void LandAndWavesApp::BuildShadersAndInputLayout()
	{
		mShaders["standardVS"] = d3dUtil::CompileShader(L"shader/C7_LandAndWaves.hlsl", nullptr, "VS", "vs_5_0");
		mShaders["opaquePS"] = d3dUtil::CompileShader(L"shader/C7_LandAndWaves.hlsl", nullptr, "PS", "ps_5_0");

		mInputLayout =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
		};
	}

	void LandAndWavesApp::BuildPSOs()
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

		ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		opaquePsoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
		opaquePsoDesc.pRootSignature = mRootSignature.Get();
		opaquePsoDesc.VS =
		{
			reinterpret_cast<BYTE*>(mShaders["standardVS"]->GetBufferPointer()),
			mShaders["standardVS"]->GetBufferSize()
		};
		opaquePsoDesc.PS =
		{
			reinterpret_cast<BYTE*>(mShaders["opaquePS"]->GetBufferPointer()),
			mShaders["opaquePS"]->GetBufferSize()
		};
		opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		auto& pso = opaquePsoDesc;
		pso.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		pso.SampleMask = UINT_MAX;
		pso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		pso.NumRenderTargets = 1;
		pso.RTVFormats[0] = mBackBufferFormat;
		pso.SampleDesc.Count = m4xMsaaState ? 4 : 1;
		pso.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
		pso.DSVFormat = mDepthStencilFormat;
		ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&mPSOs["opaque"])));

		auto wpso = pso;
		wpso.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
		ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&wpso, IID_PPV_ARGS(&mPSOs["opaque_wireframe"])));
	}

	void LandAndWavesApp::BuildFrameResources()
	{
		for (int i = 0; i < gNumFrameResource; ++i)
		{
			mFrameResources.push_back(
				std::make_unique<FrameResource>(
					md3dDevice.Get(),
					1,
					(UINT)mAllRitems.size(),
					mWaves->VertexCount()));
		}
	}

	void LandAndWavesApp::BuildRenderItems()
	{
		auto wavesRitem = std::make_unique<RenderItem>();
		wavesRitem->World = MathHelper::Identity4x4();
		wavesRitem->ObjCBIndex = 0;
		wavesRitem->Geo = mGeometries["waterGeo"].get();
		wavesRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		wavesRitem->IndexCount = wavesRitem->Geo->DrawArgs["grid"].IndexCount;
		wavesRitem->StartIndexLocation = wavesRitem->Geo->DrawArgs["grid"].StartIndexLocation;
		wavesRitem->BaseVertexLocation = wavesRitem->Geo->DrawArgs["grid"].BaseVertexLocation;

		mWavesRitem = wavesRitem.get();
		mRitemLayer[(int)RenderLayer::Opaque].push_back(wavesRitem.get());


		auto gridRitem = std::make_unique<RenderItem>();
		gridRitem->World = MathHelper::Identity4x4();
		gridRitem->ObjCBIndex = 1;
		gridRitem->Geo = mGeometries["landGeo"].get();
		gridRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		gridRitem->IndexCount = gridRitem->Geo->DrawArgs["grid"].IndexCount;
		gridRitem->StartIndexLocation = gridRitem->Geo->DrawArgs["grid"].StartIndexLocation;
		gridRitem->BaseVertexLocation = gridRitem->Geo->DrawArgs["grid"].BaseVertexLocation;

		mRitemLayer[(int)RenderLayer::Opaque].push_back(gridRitem.get());

		mAllRitems.push_back(std::move(wavesRitem));
		mAllRitems.push_back(std::move(gridRitem));
	}

	void LandAndWavesApp::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
	{
		UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

		auto objectCB = mCurrFrameResource->ObjectCB->Resource();

		for (size_t i = 0; i < ritems.size(); i++)
		{
			auto ri = ritems[i];

			cmdList->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
			cmdList->IASetIndexBuffer(&ri->Geo->IndexBufferView());
			cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

			D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress();
			objCBAddress += ri->ObjCBIndex * objCBByteSize;

			cmdList->SetGraphicsRootConstantBufferView(0, objCBAddress);
			cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
		}
	}
#pragma endregion Core Prepar

#pragma region Core
	void LandAndWavesApp::Update(const GameTimer& gt)
	{
		OnKeyboardInput(gt);
		UpdateCamera(gt);

		mCurrFrameResourceIndex = (mCurrFrameResourceIndex + 1) % gNumFrameResource;
		mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();

		if (mCurrFrameResource->Fence != 0 && mFence->GetCompletedValue() < mCurrFrameResource->Fence)
		{
			HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
			ThrowIfFailed(mFence->SetEventOnCompletion(mCurrFrameResource->Fence, eventHandle));
			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}
		UpdateObjectCBs(gt);
		UpdateMainPassCBs(gt);
		UpdateWaves(gt);
	}

	void LandAndWavesApp::UpdateCamera(const GameTimer& gt)
	{
		mEyePos.x = mRadius * sinf(mPhi) * cosf(mTheta);
		mEyePos.z = mRadius * sinf(mPhi) * sinf(mTheta);
		mEyePos.y = mRadius * cosf(mPhi);

		// Build the view matrix.
		XMVECTOR pos = XMVectorSet(mEyePos.x, mEyePos.y, mEyePos.z, 1.0f);
		XMVECTOR target = XMVectorZero();
		XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
		XMStoreFloat4x4(&mView, view);
	}

	void LandAndWavesApp::UpdateObjectCBs(const GameTimer& gt)
	{
		auto currObjectCB = mCurrFrameResource->ObjectCB.get();
		for (auto& e : mAllRitems)
		{
			if (e->NumFrameDirty > 0)
			{
				DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&e->World);
				ObjectConstants objConstants;
				DirectX::XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));

				currObjectCB->CopyData(e->ObjCBIndex, objConstants);

				e->NumFrameDirty--;
			}
		}
	}

	void LandAndWavesApp::UpdateMainPassCBs(const GameTimer& gt)
	{
		mat view = XMLoadFloat4x4(&mView);
		mat proj = XMLoadFloat4x4(&mProj);

		mat viewProj = XMMatrixMultiply(view, proj);
		mat invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
		mat invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
		mat invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

		XMStoreFloat4x4(&mMainPassCB.View, XMMatrixTranspose(view));
		XMStoreFloat4x4(&mMainPassCB.InvView, XMMatrixTranspose(invView));
		XMStoreFloat4x4(&mMainPassCB.Proj, XMMatrixTranspose(proj));
		XMStoreFloat4x4(&mMainPassCB.InvProj, XMMatrixTranspose(invProj));
		XMStoreFloat4x4(&mMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
		XMStoreFloat4x4(&mMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
		mMainPassCB.EyePosW = mEyePos;
		mMainPassCB.RenderTargetSize = XMFLOAT2((float)mClientWidth, (float)mClientHeight);
		mMainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
		mMainPassCB.NearZ = 1.0f;
		mMainPassCB.FarZ = 1000.0f;
		mMainPassCB.TotalTime = gt.TotalTime();
		mMainPassCB.DeltaTime = gt.DeltaTime();

		auto currPassCB = mCurrFrameResource->PassCB.get();
		currPassCB->CopyData(0, mMainPassCB);
	}

	void LandAndWavesApp::UpdateWaves(const GameTimer& gt)
	{
		static float t_base = 0.0f;
		if ((mTimer.TotalTime() - t_base) >= 0.25f)
		{
			t_base += 0.25f;

			int i = MathHelper::Rand(4, mWaves->RowCount() - 5);
			int j = MathHelper::Rand(4, mWaves->ColumnCount() - 5);

			float r = MathHelper::RandF(0.2f, 0.5f);

			mWaves->Disturb(i, j, r);
		}

		mWaves->Update(gt.DeltaTime());

		auto currWavesVB = mCurrFrameResource->WavesVB.get();
		for (int i = 0; i < mWaves->VertexCount(); i++)
		{
			Vertex v;
			v.Pos = mWaves->Position(i);
			v.Color = XMFLOAT4(DirectX::Colors::Blue);

			currWavesVB->CopyData(i, v);
		}
		mWavesRitem->Geo->VertexBufferGPU = currWavesVB->Resource();
	}

	void LandAndWavesApp::Draw(const GameTimer& gt)
	{
		auto cmdListAlloc = mCurrFrameResource->CmdListAlloc;

		ThrowIfFailed(cmdListAlloc->Reset());

		if (mIsWireframe)
		{
			ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), mPSOs["opaque_wireframe"].Get()));
		}
		else
		{
			ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), mPSOs["opaque"].Get()));
		}

		mCommandList->RSSetViewports(1, &mScreenViewport);
		mCommandList->RSSetScissorRects(1, &mScissorRect);


		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

		mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
		mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

		mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, nullptr);
		mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());
		mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

		auto passCB = mCurrFrameResource->PassCB->Resource();
		mCommandList->SetGraphicsRootConstantBufferView(1, passCB->GetGPUVirtualAddress());

		DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Opaque]);

		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

		ThrowIfFailed(mCommandList->Close());

		ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
		mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

		ThrowIfFailed(mSwapChain->Present(0, 0));
		mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;
		mCurrFrameResource->Fence = ++mCurrentFence;

		mCommandQueue->Signal(mFence.Get(), mCurrentFence);
	}
#pragma endregion Core


#pragma region Resource Build
	void LandAndWavesApp::BuildWavesGeometryBuffers()
	{
		std::vector<std::uint16_t> indices(3 * mWaves->TriangleCount());
		assert(mWaves->VertexCount() < 0x0000ffff);

		int m = mWaves->RowCount();
		int n = mWaves->ColumnCount();
		int k = 0;

		for (int i = 0; i < m - 1; i++)
		{
			for (int j = 0; j < n - 1; j++)
			{
				indices[k] = i * n + j;
				indices[k + 1] = i * n + j + 1;
				indices[k + 2] = (i + 1) * n + j;

				indices[k + 3] = (i + 1) * n + j;
				indices[k + 4] = i * n + j + 1;
				indices[k + 5] = (i + 1) * n + j + 1;

				k += 6;
			}
		}

		UINT vbByteSize = mWaves->VertexCount() * sizeof(Vertex);
		UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

		auto geo = std::make_unique<MeshGeometry>();
		geo->Name = "waterGeo";

		geo->VertexBufferCPU = nullptr;
		geo->VertexBufferGPU = nullptr;

		ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
		CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

		geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
			mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

		geo->VertexByteStride = sizeof(Vertex);
		geo->VertexBufferByteSize = vbByteSize;
		geo->IndexFormat = DXGI_FORMAT_R16_UINT;
		geo->IndexBufferByteSize = ibByteSize;

		SubmeshGeometry submesh;
		submesh.IndexCount = (UINT)indices.size();
		submesh.StartIndexLocation = 0;
		submesh.BaseVertexLocation = 0;

		geo->DrawArgs["grid"] = submesh;

		mGeometries["waterGeo"] = std::move(geo);
	}

	void LandAndWavesApp::BuildLandGeometry()
	{
		GeometryGenerator geoGen;
		GeometryGenerator::MeshData grid = geoGen.CreateGrid(160.0f, 160.0f, 50, 50);

		std::vector<Vertex> vertices(grid.Vertices.size());
		for (size_t i = 0; i < grid.Vertices.size(); i++)
		{
			auto& p = grid.Vertices[i].Position;
			vertices[i].Pos = p;
			vertices[i].Pos.y = GetHillsHeight(p.x, p.z);

			if (vertices[i].Pos.y < -10.0f)
			{
				vertices[i].Color = XMFLOAT4(1.0f, 0.96f, 0.62f, 1.0f);
			}
			else if (vertices[i].Pos.y < 5.0f)
			{
				vertices[i].Color = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
			}
			else if (vertices[i].Pos.y < 12.0f)
			{
				vertices[i].Color = XMFLOAT4(0.1f, 0.48f, 0.19f, 1.0f);
			}
			else if (vertices[i].Pos.y < 20.0f)
			{
				vertices[i].Color = XMFLOAT4(0.45f, 0.39f, 0.34f, 1.0f);
			}
			else
			{
				vertices[i].Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
			}
		}

		const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

		std::vector<std::uint16_t> indices = grid.GetIndices16();
		const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

		auto geo = std::make_unique<MeshGeometry>();
		geo->Name = "landGeo";

		ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
		CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

		ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
		CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

		geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
			mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

		geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
			mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

		geo->VertexByteStride = sizeof(Vertex);
		geo->VertexBufferByteSize = vbByteSize;

		geo->IndexFormat = DXGI_FORMAT_R16_UINT;
		geo->IndexBufferByteSize = ibByteSize;

		SubmeshGeometry submesh;
		submesh.IndexCount = (UINT)indices.size();
		submesh.StartIndexLocation = 0;
		submesh.BaseVertexLocation = 0;

		geo->DrawArgs["grid"] = submesh;

		mGeometries["landGeo"] = std::move(geo);

	}

	float LandAndWavesApp::GetHillsHeight(float x, float z) const
	{
		return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
	}
#pragma endregion Resource Build

#pragma region Control
	void LandAndWavesApp::OnMouseDown(WPARAM btnState, int x, int y)
	{
		mLastMousePos.x = x;
		mLastMousePos.y = y;

		SetCapture(mhMainWnd);
	}

	void LandAndWavesApp::OnMouseUp(WPARAM btnState, int x, int y)
	{
		ReleaseCapture();
	}

	void LandAndWavesApp::OnMouseMove(WPARAM btnState, int x, int y)
	{
		if ((btnState & MK_LBUTTON) != 0)
		{
			// Make each pixel correspond to a quarter of a degree.
			float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
			float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

			// Update angles based on input to orbit camera around box.
			mTheta += dx;
			mPhi += dy;

			// Restrict the angle mPhi.
			mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
		}
		else if ((btnState & MK_RBUTTON) != 0)
		{
			// Make each pixel correspond to 0.2 unit in the scene.
			float dx = 0.2f * static_cast<float>(x - mLastMousePos.x);
			float dy = 0.2f * static_cast<float>(y - mLastMousePos.y);

			// Update the camera radius based on input.
			mRadius += dx - dy;

			// Restrict the radius.
			mRadius = MathHelper::Clamp(mRadius, 5.0f, 150.0f);
		}

		mLastMousePos.x = x;
		mLastMousePos.y = y;
	}

	void LandAndWavesApp::OnKeyboardInput(const GameTimer& gt)
	{
		if (GetAsyncKeyState('1') & 0x8000)
			mIsWireframe = true;
		else
			mIsWireframe = false;
	}

	void LandAndWavesApp::OnResize()
	{
		D3DApp::OnResize();
		XMMATRIX p = XMMatrixPerspectiveFovLH(0.25 * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
		XMStoreFloat4x4(&mProj, p);
	}
#pragma endregion Control


	int Main(HINSTANCE hInstance)
	{
#if defined(DEBUG) | defined(_DEBUG)
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

		try
		{
			LandAndWavesApp theApp(hInstance);
			if (!theApp.Initialize())
			{
				return 0;
			}
			return theApp.Run();
		}
		catch (DxException& e)
		{
			MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
			return 0;
		}
	}

}