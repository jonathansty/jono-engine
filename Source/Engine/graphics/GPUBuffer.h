#pragma once
enum class BufferUsage
{
	Default,
	Dynamic,
	Staging,
	Immutable
};

interface IGPUBuffer
{
	virtual void* map(ID3D11DeviceContext* ctx) = 0;
	virtual void unmap(ID3D11DeviceContext* ctx) = 0;

	virtual ID3D11Buffer* get_buffer() = 0;

	virtual ComPtr<ID3D11ShaderResourceView> const& get_srv() const
	{
		FAILMSG("GPU Buffer does not support SRVs");
		static ComPtr<ID3D11ShaderResourceView> s_null;
		return s_null;
	}

	virtual ComPtr<ID3D11UnorderedAccessView> const& get_uav() const 
	{
		FAILMSG("GPU Buffer does not support UAVs");
		static ComPtr<ID3D11UnorderedAccessView> s_null;
		return s_null;
	}
};
