#include "engine.pch.h"
#include "ShaderCompiler.h"

#include "PlatformIO.h"
#include "Logging.h"

namespace ShaderCompiler {

bool compile(const char* shader, CompileParameters const& parameters, std::vector<u8>& bytecode) {

	auto io = IO::get();
	if(IO::IFileRef file = io->open(shader, IO::Mode::Read); file) {

		file->seek(0, IO::SeekMode::FromEnd);
		u64 size = file->tell();
		file->seek(0, IO::SeekMode::FromBeginning);

		char* data = (char*)malloc(size + 1);
		memset(data, 0, size + 1);
		file->read(data, (u32)size);

		std::vector<D3D_SHADER_MACRO> defines = {};

		ID3DBlob* shadercode_result;
		ID3DBlob* errors;

		std::vector<D3D_SHADER_MACRO> macros;
		std::transform(parameters.defines.begin(), parameters.defines.end(), std::back_inserter(macros), [](CompileParameters::Define const& define) {
			D3D_SHADER_MACRO macro{};
			if(define.value.has_value()) {
				macro.Definition = define.value.value().c_str();
			}
			macro.Name = define.name.c_str();
			return macro;
		});
		macros.push_back({});

		std::string entry_point = parameters.entry_point;
		std::string target = get_target(parameters.stage);
		HRESULT result = D3DCompile(data, size, shader, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entry_point.c_str(), target.c_str(), static_cast<u32>(parameters.flags), static_cast<u32>(parameters.effect_flags), &shadercode_result, &errors);
		free(data);

		if(FAILED(result)) {
		
			std::string messages{(char*)errors->GetBufferPointer()};
			LOG_ERROR(Graphics, "Shader compile failed with the following message: {}", messages);
			return false;
		}

		// Copy the byte code resut to the output buffer
		u8* d = static_cast<u8*>(shadercode_result->GetBufferPointer());
		u32 s = static_cast<u32>(shadercode_result->GetBufferSize());

		bytecode.resize(s);
		memcpy(bytecode.data(), d, s);
	}

	return true;
}

} // namespace ShaderCompiler
