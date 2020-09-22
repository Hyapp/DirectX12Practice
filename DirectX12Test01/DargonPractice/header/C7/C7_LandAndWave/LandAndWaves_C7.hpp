#pragma once
#include <d3dApp.h>
#include <MathHelper.h>
#include <UploadBuffer.h>
#include <GeometryGenerator.h>

#include <C7/C7_LandAndWave/FrameResource1.hpp>
#include <C7/C7_LandAndWave/Waves.hpp>

namespace C7_LandAndWaves
{
	extern const int gNumFrameResource;

	struct RenderItem
	{
		using mat4 = DirectX::XMFLOAT4X4;

		RenderItem() = default;
		mat4 World = MathHelper::Identity4x4();
		int NumFrameDirty = gNumFrameResource;

		UINT ObjCBIndex = -1;

		MeshGeometry* Geo = nullptr;

		D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		UINT IndexCount = 0;
		UINT StartIndexLocation = 0;
		int BaseVertexLocation = 0;
	};

	enum class RenderLayer : int
	{
		Opaque = 0,
		Count,
	};


	class LandAndWavesApp : public D3DApp
	{
		template<typename T>
		using ComPtr = Microsoft::WRL::ComPtr<T>;

		using float3 = DirectX::XMFLOAT3;
		using mat4 = DirectX::XMFLOAT4X4;
	public:
		LandAndWavesApp(HINSTANCE hInstance);
		LandAndWavesApp(const LandAndWavesApp& rhs) = delete;
		LandAndWavesApp& operator=(const LandAndWavesApp& rhs) = delete;
		~LandAndWavesApp();

		virtual bool Initialize() override;

	private:

#pragma region Core
		virtual void Update(const GameTimer& gt) override;
		virtual void Draw(const GameTimer& gt) override;
#pragma endregion Core

#pragma region Update
		void UpdateCamera(const GameTimer& gt);
		void UpdateObjectCBs(const GameTimer& gt);
		void UpdateMainPassCBs(const GameTimer& gt);
		void UpdateWaves(const GameTimer& gt);
#pragma endregion Update

#pragma region Core Prepar
		void BuildRootSignature();
		void BuildShadersAndInputLayout();
		void BuildLandGeometry();
		void BuildWavesGeometryBuffers();
		void BuildPSOs();
		void BuildFrameResources();
		void BuildRenderItems();
		void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);

		float GetHillsHeight(float x, float z) const;

		float3 GetHillsNormal(float x, float z) const;

#pragma endregion Core Prepar

#pragma region Control

		virtual void OnMouseDown(WPARAM btnState, int x, int y) override;
		virtual void OnMouseUp(WPARAM btnState, int x, int y) override;
		virtual void OnMouseMove(WPARAM btnState, int x, int y) override;

		void OnKeyboardInput(const GameTimer& gt);

		virtual void OnResize() override;
#pragma endregion Control

	private:
		std::vector<std::unique_ptr<FrameResource>> mFrameResources;
		FrameResource* mCurrFrameResource = nullptr;
		int mCurrFrameResourceIndex = 0;



		UINT mCbvSrvDescriptorSize = 0;
		ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
		std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
		std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
		std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;

		std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

		RenderItem* mWavesRitem = nullptr;

		std::vector<std::unique_ptr<RenderItem>> mAllRitems;

		std::vector<RenderItem*>mRitemLayer[(int)RenderLayer::Count];
		std::unique_ptr<Geometry::Waves> mWaves;

		PassConstants mMainPassCB;

		bool mIsWireframe = false;
		float3 mEyePos = { 0.0f, 0.0f,0.0f };
		mat4 mView = MathHelper::Identity4x4();
		mat4 mProj = MathHelper::Identity4x4();

		float mTheta = 1.5f * DirectX::XM_PI;
		float mPhi = DirectX::XM_PIDIV2 - 0.1f;
		float mRadius = 50.0f;

		float mSunTheta = 1.25f * DirectX::XM_PI;
		float mSunPhi = DirectX::XM_PIDIV4;

		POINT mLastMousePos;
	};

	int Main(HINSTANCE hInstance);
}