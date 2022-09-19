#pragma once

class StructuredBuffer final
{
public:
	enum class BufferUsage
	{
		Default,
		Dynamic,
		Staging,
		Immutable
	};

	StructuredBuffer(){};
	~StructuredBuffer() {}

	static std::unique_ptr<StructuredBuffer> create(ID3D11Device* device, size_t struct_size_bytes, size_t element_count, bool cpu_write = false, BufferUsage usage = BufferUsage::Default);

	ComPtr<ID3D11ShaderResourceView> const& get_srv() const { return _srv; }

private:
	ComPtr<ID3D11Buffer> _buffer;
	ComPtr<ID3D11ShaderResourceView>  _srv;
	ComPtr<ID3D11UnorderedAccessView> _uav;

	size_t _size_bytes;
	size_t _struct_size_bytes;

};