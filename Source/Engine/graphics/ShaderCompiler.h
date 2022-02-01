#pragma once
namespace ShaderCompiler {

	enum class ShaderStage {
		Vertex,
		Pixel,
		Compute,
		Domain,
		Geometry,
		Hull,
	};

	inline const char* get_target(ShaderStage stage) {
		switch (stage) {
			case ShaderStage::Vertex:
				return "vs_5_0";
			case ShaderStage::Pixel:
				return "ps_5_0";
			case ShaderStage::Compute:
				return "cs_5_0";
			case ShaderStage::Domain:
				return "ds_5_0";
			case ShaderStage::Geometry:
				return "gs_5_0";
			case ShaderStage::Hull:
				return "hs_5_0";
		}
		return nullptr;
	}

	#define ENUM_BITFLAGS(enum_type) \
	inline enum_type operator&(enum_type const& lhs, enum_type const& rhs) { \
		return static_cast<enum_type>((u32)lhs & (u32)rhs);                   \
	} \
	inline enum_type operator|(enum_type const& lhs, enum_type const& rhs) { \
		return static_cast<enum_type>((u32)lhs | (u32)rhs);\
	} 

	enum class CompilerFlags : u32 {
		CompileDebug = 1 << 0,
		SkipValidation = 1 << 2,
		SkipOptimization = 1 << 3,
		OptimizationLevel0 = 1 << 14,
		OptimizationLevel1 = 0,
		OptimizationLevel2 = (1 << 14) | (1 << 15),
		OptimizationLevel3 = 1 << 15,
		WarningsAreErrors = 1 << 18,
	};
	ENUM_BITFLAGS(CompilerFlags);

	enum class CompilerEffectFlags : u32 {
		ChildEffect = 1 << 0,
		AllowSlowOps = 1 << 1
	};
	ENUM_BITFLAGS(CompilerEffectFlags)

	struct CompileParameters {
		struct Define {
			std::string name;
			std::optional<std::string> value;
		};
		ShaderStage stage;

		CompilerFlags flags;
		CompilerEffectFlags effect_flags;

		std::string entry_point;
		std::vector<Define> defines;
	};

	bool compile(const char* shader, CompileParameters const& parameters, std::vector<u8>& out_bytecode);


}

