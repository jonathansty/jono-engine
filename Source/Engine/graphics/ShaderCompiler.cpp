#include "engine.pch.h"
#include "ShaderCompiler.h"

#include "Logging.h"
#include "PlatformIO.h"

namespace ShaderCompiler
{

bool compile(const char* shader, CompileParameters const& parameters, std::vector<u8>& bytecode)
{
	LOG_VERBOSE(Graphics, "[SHDRCMP] {}", shader);

	auto io = IO::get();
	if (IO::IFileRef file = io->OpenFile(shader, IO::Mode::Read); file)
	{
		file->seek(0, IO::SeekMode::FromEnd);
		u64 file_size = file->tell();
		file->seek(0, IO::SeekMode::FromBeginning);

		char* data = (char*)malloc(file_size + 1);
		memset(data, 0, file_size + 1);
		u32 bytes_read = file->read(data, (u32)file_size);

		std::vector<D3D_SHADER_MACRO> defines = {};

		ID3DBlob* shadercode_result;
		ID3DBlob* errors;

		std::vector<D3D_SHADER_MACRO> macros;
		std::transform(parameters.defines.begin(), parameters.defines.end(), std::back_inserter(macros), 
			[](CompileParameters::Define const& define)
			{
				D3D_SHADER_MACRO macro{};
				if(define.value.has_value()) 
				{
					macro.Definition = define.value.value().c_str();
				}
				macro.Name = define.name.c_str();
				return macro; 
			}
		);

		if(any(parameters.stage & ShaderStage::Vertex))
        {
            D3D_SHADER_MACRO macro;
            macro.Definition = "1";
            macro.Name = "_VERTEX";
            macros.push_back(macro);
		}

        if(any(parameters.stage & ShaderStage::Pixel))
        {
            D3D_SHADER_MACRO macro;
            macro.Definition = "1";
            macro.Name = "_PIXEL";
            macros.push_back(macro);
		}

		macros.push_back({});

		std::string entry_point = parameters.entry_point;
		std::string target = get_target(parameters.stage);


		ComPtr<ID3DBlob> preprocessedData;
		ComPtr<ID3DBlob> errorData;
		HRESULT result = D3DPreprocess(data, bytes_read, shader, macros.data(), D3D_COMPILE_STANDARD_FILE_INCLUDE, preprocessedData.ReleaseAndGetAddressOf(), errorData.ReleaseAndGetAddressOf());
		if(FAILED(result))
		{

			std::string messages{};
			if(errorData)
			{
				messages = { (char*)errorData->GetBufferPointer() };
			}

			LOG_ERROR(Graphics, "Shader preprocess failed with the following message: {}", messages);
			free(data);
			return false;
		}

		//std::string preprocessed_fn = fmt::format("Intermediate/{}.preprocessed", shader);
		//auto f = io->open(preprocessed_fn.c_str(), IO::Mode::Write, false);
		//if(f)
		//{
		//	f->write(preprocessedData->GetBufferPointer(), static_cast<u32>(preprocessedData->GetBufferSize()));
		//}

		result = D3DCompile(preprocessedData->GetBufferPointer(), preprocessedData->GetBufferSize(), shader, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entry_point.c_str(), target.c_str(), static_cast<u32>(parameters.flags), static_cast<u32>(parameters.effect_flags), &shadercode_result, &errors);
		free(data);

		if (FAILED(result))
		{
			std::string messages{ (char*)errors->GetBufferPointer() };
			LOG_ERROR(Graphics, "Shader compile failed with the following message: {}", messages);
			return false;
		}

		// Copy the byte code resut to the output buffer
		u8* d = static_cast<u8*>(shadercode_result->GetBufferPointer());
		u32 s = static_cast<u32>(shadercode_result->GetBufferSize());

		bytecode.resize(s);
		memcpy(bytecode.data(), d, s);
		return true;
	}

	LOG_ERROR(Graphics, "Shader compile failed with reading the input shader file: {}", shader);
	return false;
}

} // namespace ShaderCompiler
