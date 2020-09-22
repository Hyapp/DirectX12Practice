#include <../header/C7/C7_LandAndWave/FrameResource1.hpp>

namespace C7_LandAndWaves
{
	FrameResource::FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount, UINT waveVertCount)
	{
		ThrowIfFailed(device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(CmdListAlloc.GetAddressOf())));

		PassCB = std::make_unique<UploadBuffer<PassConstants>>(device, passCount, true);
		ObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(device, objectCount, true);
		WavesVB = std::make_unique<UploadBuffer<Vertex>>(device, waveVertCount, false);
	}

	FrameResource::~FrameResource() {}
}

